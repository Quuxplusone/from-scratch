#pragma once

#include <iterator>
#include <string>

namespace scratch {

using code_unit_iterator = const char *;

class code_unit_view {
    const char *m_begin;
    const char *m_end;
public:
    using iterator = code_unit_iterator;
    constexpr explicit code_unit_view(const char *a, const char *b) noexcept : m_begin(a), m_end(b) {}
    constexpr iterator begin() const noexcept { return m_begin; }
    constexpr iterator end() const noexcept { return m_end; }
};


class code_point_iterator {
    const char *m_pos = nullptr;
    mutable const char *m_next_pos = nullptr;
    const char *m_end_pos = nullptr;  // for bounds-checking purposes
    mutable char32_t m_cached = 0;
    constexpr explicit code_point_iterator(const char *p, const char *q) : m_pos(p), m_next_pos(p), m_end_pos(q) {}
    void decode_and_cache() const;
    friend class code_point_view;
    friend class grapheme_iterator;

    static constexpr bool should_check_malformed() { return true; }
    static constexpr bool should_check_bounds() { return true; }
    void encounter_malformed_sequence(int) const { m_cached = 0xFFFD; m_next_pos += 1; }
public:
    using value_type = char32_t;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = const char32_t*;
    using reference = const char32_t&;
    code_point_iterator() noexcept = default;
    code_point_iterator& operator++() { decode_and_cache(); m_pos = m_next_pos; return *this; }
    code_point_iterator operator++(int) { auto x(*this); ++*this; return x; }
    const char32_t& operator*() const { decode_and_cache(); return m_cached; }
    constexpr explicit operator code_unit_iterator() const noexcept { return code_unit_iterator(m_pos); }
};

inline constexpr bool operator==(code_point_iterator a, code_point_iterator b) noexcept {
    return static_cast<code_unit_iterator>(a) == static_cast<code_unit_iterator>(b);
}

inline constexpr bool operator!=(code_point_iterator a, code_point_iterator b) noexcept {
    return static_cast<code_unit_iterator>(a) != static_cast<code_unit_iterator>(b);
}

void code_point_iterator::decode_and_cache() const
{
    if (m_next_pos != m_pos) {
        // Nothing to do.
    } else {
#ifdef JUST_USE_ASCII_CODEPOINTS
        m_cached = uint8_t(*m_pos);
        m_next_pos += 1;
#else
        // UTF-8 decoding.
        auto is_continuation_byte = [](uint8_t ch) { return (0x80 <= ch && ch <= 0xBF); };
        uint8_t firstbyte = m_pos[0];
        if (firstbyte <= 0x7F) {
            m_cached = firstbyte;
            m_next_pos += 1;
        } else if (should_check_malformed() && firstbyte <= 0xC1) {
            encounter_malformed_sequence(1);
        } else if (firstbyte <= 0xDF) {
            if (should_check_bounds() && m_end_pos - m_pos < 2) {
                encounter_malformed_sequence(m_end_pos - m_pos);
            } else if (should_check_malformed() && !is_continuation_byte(m_pos[1])) {
                encounter_malformed_sequence(1);
            } else {
                m_cached = ((firstbyte & 0x3F) << 6) | (m_pos[1] & 0x3F);
                m_next_pos += 2;
            }
        } else if (firstbyte <= 0xEF) {
            if (should_check_bounds() && m_end_pos - m_pos < 3) {
                encounter_malformed_sequence(m_end_pos - m_pos);
            } else if (should_check_malformed() && !is_continuation_byte(m_pos[1])) {
                encounter_malformed_sequence(1);
            } else if (should_check_malformed() && !is_continuation_byte(m_pos[2])) {
                encounter_malformed_sequence(2);
            } else {
                m_cached = ((firstbyte & 0x1F) << 12) | ((m_pos[1] & 0x3F) << 6) | (m_pos[2] & 0x3F);
                m_next_pos += 3;
            }
        } else if (firstbyte <= 0xF7) {
            if (should_check_bounds() && m_end_pos - m_pos < 4) {
                encounter_malformed_sequence(m_end_pos - m_pos);
            } else if (should_check_malformed() && !is_continuation_byte(m_pos[1])) {
                encounter_malformed_sequence(1);
            } else if (should_check_malformed() && !is_continuation_byte(m_pos[2])) {
                encounter_malformed_sequence(2);
            } else if (should_check_malformed() && !is_continuation_byte(m_pos[3])) {
                encounter_malformed_sequence(3);
            } else {
                m_cached = ((firstbyte & 0x0F) << 18) | ((m_pos[1] & 0x3F) << 12) | ((m_pos[2] & 0x3F) << 6) | (m_pos[3] & 0x3F);
                m_next_pos += 4;
            }
        } else {
            encounter_malformed_sequence(1);
        }
#endif
    }
}

class code_point_view {
    const char *m_begin;
    const char *m_end;
public:
    using iterator = code_point_iterator;
    constexpr explicit code_point_view(const char *a, const char *b) noexcept : m_begin(a), m_end(b) {}
    constexpr iterator begin() const noexcept { return iterator(m_begin, m_end); }
    constexpr iterator end() const noexcept { return iterator(m_end, m_end); }
    constexpr code_unit_view code_units() const noexcept { return code_unit_view(m_begin, m_end); }
};

class grapheme {
    const char *m_begin = nullptr;
    const char *m_end = nullptr;
    constexpr explicit grapheme() noexcept = default;
    constexpr explicit grapheme(const char *a, const char *b) noexcept : m_begin(a), m_end(b) {}
    friend class grapheme_iterator;
public:
    constexpr code_unit_view code_units() const noexcept { return code_unit_view(m_begin, m_end); }
    constexpr code_point_view code_points() const noexcept { return code_point_view(m_begin, m_end); }
};

class grapheme_iterator {
    const char *m_pos = nullptr;
    mutable const char *m_next_pos = nullptr;
    mutable const char *m_end_pos = nullptr;
    mutable grapheme m_cached;
    constexpr explicit grapheme_iterator(const char *p, const char *q) : m_pos(p), m_next_pos(p), m_end_pos(q) {}
    void decode_and_cache() const;
    friend class grapheme_view;
public:
    using value_type = grapheme;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = const grapheme*;
    using reference = const grapheme&;
    constexpr grapheme_iterator() = default;
    grapheme_iterator& operator++() { decode_and_cache(); m_pos = m_next_pos; return *this; }
    grapheme_iterator operator++(int) { auto x(*this); ++*this; return x; }
    const grapheme& operator*() const { decode_and_cache(); return m_cached; }
    constexpr explicit operator code_unit_iterator() const noexcept { return code_unit_iterator(m_pos); }
    constexpr explicit operator code_point_iterator() const noexcept { return code_point_iterator(m_pos, m_end_pos); }
};

inline constexpr bool operator==(grapheme_iterator a, grapheme_iterator b) noexcept {
    return static_cast<code_unit_iterator>(a) == static_cast<code_unit_iterator>(b);
}

inline constexpr bool operator!=(grapheme_iterator a, grapheme_iterator b) noexcept {
    return static_cast<code_unit_iterator>(a) != static_cast<code_unit_iterator>(b);
}

class grapheme_view {
    const char *m_begin;
    const char *m_end;
public:
    using iterator = grapheme_iterator;
    constexpr explicit grapheme_view(const char *a, const char *b) noexcept : m_begin(a), m_end(b) {}
    constexpr iterator begin() const noexcept { return iterator(m_begin, m_end); }
    constexpr iterator end() const noexcept { return iterator(m_end, m_end); }
    constexpr code_unit_view code_units() const noexcept { return code_unit_view(m_begin, m_end); }
    constexpr code_point_view code_points() const noexcept { return code_point_view(m_begin, m_end); }
};

void grapheme_iterator::decode_and_cache() const
{
    if (m_next_pos != m_pos) {
        // Nothing to do.
    } else {
        auto is_grapheme_break = [](char32_t a, char32_t) {
            // TODO: here is where most of the Unicode grapheme-cluster segmentation algorithm would go.
            if (U'\u0300' <= a && a <= U'\u036F') return false;
            return true;
        };
        // This actually requires knowing where the end of the current sequence is, because otherwise
        // we risk running off the end of the sequence. Hence our `m_end_pos`.
        code_point_iterator it(*this);
        char32_t prevpoint = *it;
        while (true) {
            ++it;
            if (it.m_pos == this->m_end_pos) break;
            char32_t nextpoint = *it;
            if (is_grapheme_break(prevpoint, nextpoint)) break;
            prevpoint = nextpoint;
        }
        m_next_pos = it.m_pos;
        m_cached = grapheme(m_pos, m_next_pos);
    }
}


class text {
    std::string m_code_units;
    const char *cubegin() const noexcept { return m_code_units.data(); }
    const char *cuend() const noexcept { return m_code_units.data() + m_code_units.size(); }
public:
    explicit text() = default;
    explicit text(std::string s) : m_code_units(std::move(s)) {}

    std::string str() const { return m_code_units; }
    code_unit_view code_units() const noexcept { return code_unit_view(cubegin(), cuend()); }
    code_point_view code_points() const noexcept { return code_point_view(cubegin(), cuend()); }
    grapheme_view graphemes() const noexcept { return grapheme_view(cubegin(), cuend()); }
};

} // namespace scratch
