#!/usr/bin/env python

import argparse
import os
import re


def locate_header_file(fname, include_paths):
    for p in include_paths:
        fullname = p + '/' + fname
        if os.path.exists(fullname):
            return fullname
    raise RuntimeError('File not found: %s' % fname)


def preprocess_file(fname, include_paths, already_included):
    if fname in already_included:
        return
    already_included.add(fname)
    local_include_paths = include_paths + [os.path.dirname(fname)]
    try:
        with open(fname, 'r') as f:
            for line in f.readlines():
                m = re.match(r'\s*#\s*include\s+"(.*)"', line)
                if m is not None:
                    hname = locate_header_file(m.group(1), local_include_paths)
                    preprocess_file(hname, local_include_paths, already_included)
                elif re.match(r'#pragma once', line):
                    pass
                else:
                    print line.rstrip()
    except RuntimeError as e:
        raise RuntimeError(str(e) + ' in ' + fname)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('fname', metavar='FILE', help='File to "preprocess" and dump to stdout')
    parser.add_argument('-I', '--include-dir', action='append', default=['.'], metavar='DIR', help='Path(s) to search for local includes')
    options = parser.parse_args()

    options.fname = os.path.abspath(options.fname)
    options.include_dir = [os.path.abspath(p) for p in options.include_dir]

    preprocess_file(options.fname, options.include_dir, set())
