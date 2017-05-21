#!/usr/bin/env python

import argparse
import re
import subprocess


def get_list_of_commits(start):
    args = ['git', 'log', '--format=oneline']
    if start is not None:
        args += ['%s..master' % start]
    commits = []
    lines = subprocess.check_output(args).splitlines()
    for line in lines:
        m = re.match(r'^([0-9a-f]{7,}) (.*)$', line)
        assert m is not None
        if 'verify-history' in m.group(2):
            break
        commits.append(m.group(1))
    commits.reverse()
    return commits


def make_check_on_commit(sha):
    subprocess.check_output(['git', 'checkout', sha])
    subprocess.check_output(['make', 'check'])


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--start', default=None, help='Start checking at the given git commit')
    options = parser.parse_args()

    try:
        for sha in get_list_of_commits(options.start):
            make_check_on_commit(sha)
    finally:
        subprocess.check_output(['git', 'checkout', 'master'])
