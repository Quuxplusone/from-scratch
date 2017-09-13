#include <memory>
#include <typeinfo>
#include <type_traits>
#include <utility>

class any;
struct ContainerBase;
template<class Wrapped> struct Container;

struct ContainerBase {
    virtual void copy_to(any&) const = 0;
    virtual const std::type_info& type() const = 0;
    virtual void *data() const = 0;
    virtual ~ContainerBase() = default;
};

template<class Wrapped>
struct Container : ContainerBase {
    void copy_to(any&) const override;
    const std::type_info& type() const override { return typeid(Wrapped); }
    void *data() const override { return (void *)(&m_data); }

    template<typename... Args>
    Container(Args&&... args) : m_data(std::forward<Args>(args)...) {}
private:
    Wrapped m_data;
};

class any {
    std::unique_ptr<ContainerBase> m_ptr;
public:
    // The special member functions.

    constexpr any() noexcept = default;

    any(const any& rhs) {
        if (rhs.has_value()) {
            rhs.m_ptr->copy_to(*this);
        }
    }
    any(any&& rhs) {
        m_ptr = std::move(rhs.m_ptr);
    }

    any& operator=(const any& rhs) {
        any(rhs).swap(*this);
        return *this;
    }
    any& operator=(any&& rhs) {
        any(std::move(rhs)).swap(*this);
        return *this;
    }

    template<class T, class DT = std::decay_t<T>,
        class = std::enable_if_t<!std::is_same_v<DT, any>>>
    any(T&& value) {
        this->emplace<DT>(std::forward<T>(value));
    }

    template<class T, class DT = std::decay_t<T>,
        class = std::enable_if_t<!std::is_same_v<DT, any>>>
    any& operator=(T&& value) {
        any(std::forward<T>(value)).swap(*this);
        return *this;
    }

    ~any() = default;

    // The primitive operations.

    template<class Wrapped, class... Args>
    Wrapped& emplace(Args&&... args) {
        m_ptr = std::make_unique<Container<Wrapped>>(std::forward<Args>(args)...);
        return *static_cast<Wrapped*>(m_ptr->data());
    }
    void reset() noexcept {
        m_ptr = nullptr;
    }
    void swap(any& rhs) noexcept {
        m_ptr.swap(rhs.m_ptr);
    }
    bool has_value() const noexcept {
        return (m_ptr != nullptr);
    }

    // Type-unerasure.

    const std::type_info& type() const noexcept {
        return m_ptr ? m_ptr->type() : typeid(void);
    }
    template<class T>
    T* target() noexcept {
        return (m_ptr && m_ptr->type() == typeid(T)) ? static_cast<T*>(m_ptr->data()) : nullptr;
    }
    template<class T>
    const T* target() const noexcept {
        return (m_ptr && m_ptr->type() == typeid(T)) ? static_cast<const T*>(m_ptr->data()) : nullptr;
    }
};

// This is down here only so that the definition of "emplace" will have already been seen.
template<class Wrapped>
void Container<Wrapped>::copy_to(any& destination) const
{
    destination.emplace<Wrapped>(m_data);
}
