#!/usr/bin/env python

import argparse
import os
import re
import subprocess


def allow_std_identifier_from_header(ident, std_header):
    allowed = {
        '<atomic>': ['std::atomic'],
        '<cstddef>': ['ptrdiff_t', 'size_t', 'std::max_align_t'],
        '<cstdint>': ['int8_t', 'int16_t', 'int32_t', 'int64_t', 'intptr_t', 'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t', 'uintptr_t'],
        '<exception>': ['std::current_exception', 'std::exception_ptr', 'std::rethrow_exception'],
        '<initializer_list>': ['std::initializer_list'],
        '<new>': ['::new', '::operator new', '::operator delete', 'std::align_val_t', 'std::bad_alloc'],
        '<typeinfo>': ['std::type_info', 'typeid'],
        '<utility>': ['std::exchange', 'std::move', 'std::forward', 'std::swap'],
    }
    allowed_idents_from_header = allowed.get('<' + std_header + '>', [])
    return (ident in allowed_idents_from_header)


def list_all_h_files_under(root):
    def collector(result, dirname, fnames):
        for fname in fnames:
            fullname = dirname + '/' + fname
            if os.path.isdir(fullname):
                result += list_all_h_files_under(fullname)
            elif fullname.endswith('.h'):
                result.append(fullname)
    result = []
    os.path.walk(root, collector, result)
    return result


def locate_header_file(fname, local_include_paths):
    for p in local_include_paths:
        fullname = p + '/' + fname
        if os.path.exists(fullname):
            return fullname
    raise RuntimeError('File not found: %s' % fname)


def list_all_files_included_by(fname, options):
    local_headers = []
    std_headers = []
    std_identifiers = []
    local_include_paths = options.include_dir + [os.path.dirname(fname)]
    try:
        with open(fname, 'r') as f:
            for line in f.readlines():
                m = re.match(r'\s*#\s*include\s+"(.*)"', line)
                if m is not None:
                    local_headers.append(locate_header_file(m.group(1), local_include_paths))
                m = re.match(r'\s*#\s*include\s+<(.*)>', line)
                if m is not None:
                    std_headers.append(m.group(1))
                std_identifiers += re.findall(r'std::\w+', line)
                std_identifiers += re.findall(r'::new', line)
                std_identifiers += re.findall(r'::operator new', line)
                std_identifiers += re.findall(r'::operator delete', line)
                std_identifiers += re.findall(r'\b(ptrdiff_t)\b', line)
                std_identifiers += re.findall(r'\b(u?intptr_t)\b', line)
                std_identifiers += re.findall(r'\b(u?int\d+_t)\b', line)
                std_identifiers += re.findall(r'\b(size_t)\b', line)
                std_identifiers += re.findall(r'\b(typeid)\b', line)
    except RuntimeError as e:
        raise RuntimeError(str(e) + ' in ' + fname)
    return (local_headers, std_headers, sorted(set(std_identifiers)))


def build_graph(roots, options):
    results = {}
    while roots:
        frontier = roots
        roots = []
        for fname in frontier:
            if fname in results:
                continue
            results[fname] = list_all_files_included_by(fname, options)
            roots += results[fname][0]
    return results


def is_under_source_control(fname):
    try:
        x = subprocess.check_output(['git', 'log', '-l', '1', '--format=oneline', fname])
        return (x != '')
    except subprocess.CalledProcessError:
        return False


def get_graphviz(inclusions, options):

    def get_friendly_name(fname):
        return '"%s"' % os.path.basename(fname)

    def get_decorators(fname):
        if not is_under_source_control(fname):
            return ' [style=dashed]'
        return ''

    def transitively_includes(h1, h2):
        if h1 == h2:
            return True
        next_headers, _, _ = inclusions[h1]
        return any(transitively_includes(i, h2) for i in next_headers)

    result = ''
    result += 'strict digraph {\n'
    result += '    size="10,8";\n'
    result += '    rankdir=LR;\n'
    result += '    layout=dot;\n\n'
    for fname, value in inclusions.iteritems():
        local_headers, std_headers, std_identifiers = value
        result += '    %s%s;\n' % (get_friendly_name(fname), get_decorators(fname))
        for h in local_headers:
            if not options.show_transitive_edges:
                if any(transitively_includes(i, h) for i in local_headers if i != h):
                    continue
            result += '        %s -> %s;\n' % (get_friendly_name(fname), get_friendly_name(h))
    result += '}\n'
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', default='.', metavar='FILE', help='Root of the dependency graph')
    parser.add_argument('-I', '--include-dir', action='append', default=['.'], metavar='DIR', help='Path(s) to search for local includes')
    parser.add_argument('--dot', action='store_true', help='Generate a .dot file for GraphViz')
    parser.add_argument('--show-transitive-edges', action='store_true', help='Show edges to headers that are transitively included anyway')
    options = parser.parse_args()

    options.root = os.path.abspath(options.root)
    options.include_dir = [os.path.abspath(p) for p in options.include_dir]

    if os.path.isdir(options.root):
        roots = list_all_h_files_under(options.root)
    elif os.path.exists(options.root):
        roots = [options.root]
    else:
        raise RuntimeError('--root seems to be invalid')

    inclusions = build_graph(roots, options)

    if options.dot:
        print get_graphviz(inclusions, options)
    else:
        for fname, value in inclusions.iteritems():
            local_headers, std_headers, std_identifiers = value
            print '%s => %s' % (fname, ' '.join(local_headers))

        for fname, value in inclusions.iteritems():
            local_headers, std_headers, std_identifiers = value
            for ident in std_identifiers:
                if not any(allow_std_identifier_from_header(ident, h) for h in std_headers):
                    raise RuntimeError('%s => %s is not allowed' % (fname, ident))
            for h in std_headers:
                if not any(allow_std_identifier_from_header(ident, h) for ident in std_identifiers):
                    if not any(fname.endswith(h) for h in ['linux-futex.h']):
                        raise RuntimeError('%s => <%s> is not needed for anything' % (fname, h))
