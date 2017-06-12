#!/usr/bin/env python

import argparse
import os
import subprocess

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--seed', type=int, default=1, help='Initial seed (and we count upward from there)')
    options = parser.parse_args()

    dev_null = open('/dev/null', 'w')
    os.environ['CXXFLAGS'] = '-DFREE_USE_OF_CXX17'
    for i in xrange(options.seed, long(1e12)):
        try:
            subprocess.check_call([
                'python', 'generate_harness.py', '--seed', str(i)
            ], stdout=dev_null)
        except subprocess.CalledProcessError:
            print 'generator:', i
            continue
        try:
            subprocess.check_call([
                '../dependency-graph/unity-dump.py',
                'things.gen.cc', 'dynamicast.cc', 'harness.gen.cc',
                '--g++'
            ], stdout=dev_null, stderr=dev_null)
            failed_gcc = False
        except subprocess.CalledProcessError:
            failed_gcc = True
        try:
            subprocess.check_call([
                '../dependency-graph/unity-dump.py',
                'things.gen.cc', 'dynamicast.cc', 'harness.gen.cc',
                '--clang'
            ], stdout=dev_null, stderr=dev_null)
            failed_clang = False
        except subprocess.CalledProcessError:
            failed_clang = True
        if failed_gcc and failed_clang:
            print 'both:', i
        elif failed_gcc:
            print 'gcc:', i
        elif failed_clang:
            print 'clang:', i
