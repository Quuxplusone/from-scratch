#!/usr/bin/env python

import argparse
import os
import re
import requests
import sys


def locate_header_file(fname, include_paths):
    for p in include_paths:
        fullname = p + '/' + fname
        if os.path.exists(fullname):
            return fullname
    raise RuntimeError('File not found: %s' % fname)


def preprocess_file(fname, include_paths, already_included):
    if fname in already_included:
        return ''
    result = ''
    already_included.add(fname)
    local_include_paths = include_paths + [os.path.dirname(fname)]
    try:
        with open(fname, 'r') as f:
            for line in f.readlines():
                m = re.match(r'\s*#\s*include\s+"(.*)"', line)
                if m is not None:
                    hname = locate_header_file(m.group(1), local_include_paths)
                    result += preprocess_file(hname, local_include_paths, already_included)
                elif re.match(r'#pragma once', line):
                    pass
                else:
                    result += line.rstrip() + '\n'
        return result
    except RuntimeError as e:
        raise RuntimeError(str(e) + ' in ' + fname)


def run_on_wandbox(code, compiler, options):
    data = {
        'code': code,
        'compiler': compiler,
        'options': options,
    }
    if 'CXXFLAGS' in os.environ:
        data['compiler-option-raw'] = '\n'.join(os.environ['CXXFLAGS'].split())
    response = requests.post(
        'https://wandbox.org/api/compile.json',
        json=data,
    )
    result = response.json()
    if 'compiler_message' in result:
        print result['compiler_message']
    if 'program_output' in result:
        print result['program_output']
    if 'signal' in result:
        print result['signal']  # e.g. "Aborted"
    if 'program_error' in result:
        print result['program_error']
    return int(result.get('status', -1))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('fnames', nargs='*', metavar='FILE', help='File(s) to "preprocess" and dump to stdout')
    parser.add_argument('-I', '--include-dir', action='append', default=['.'], metavar='DIR', help='Path(s) to search for local includes')
    parser.add_argument('--run', action='store_true', help='Run on Wandbox and print the results')
    parser.add_argument('--g++', dest='gcc', action='store_true', help='Run on Wandbox using GCC only')
    parser.add_argument('--clang', action='store_true', help='Run on Wandbox using Clang only')
    options = parser.parse_args()

    options.include_dir = [os.path.abspath(p) for p in options.include_dir]

    already_included = set()
    result = ''
    for fname in options.fnames:
        result += preprocess_file(os.path.abspath(fname), options.include_dir, already_included)

    if options.run and not (options.gcc or options.clang):
        options.gcc = True
        options.clang = True

    if options.clang:
        print 'Running on Clang...'
        status = run_on_wandbox(result, 'clang-head', 'c++1z,warning')
        if status == 0 and options.gcc:
            print 'Running on GCC...'
            status = run_on_wandbox(result, 'gcc-head', 'c++1z,warning')
        sys.exit(status)
    elif options.gcc:
        print 'Running on GCC...'
        status = run_on_wandbox(result, 'gcc-head', 'c++1z,warning')
        sys.exit(status)
    else:
        print result
