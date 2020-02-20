#!/usr/bin/env python3
# vim: set expandtab shiftwidth=4:

# This file generates the .gitlab-ci.yml file that defines the pipeline.

import argparse
import jinja2
import os
import sys

from pathlib import Path


distributions = [
    {'name': 'fedora', 'version': '30'},
    {'name': 'fedora', 'version': '31'},
    {'name': 'ubuntu', 'version': '19.10'},
    {'name': 'ubuntu', 'version': '19.04'},
    {'name': 'debian', 'version': 'stable'},
    {'name': 'debian', 'version': 'sid'},
    {
        'name': 'centos', 'version': '7',
        'build': {
            'extra_variables': {
                'MAKE_ARGS': ('\'\'  # disable distcheck, requires doxygen'),
            }
        },
        'meson': False
    },
    {
        'name': 'centos', 'version': '8',
        'build': {
            'extra_variables': {
                'MAKE_ARGS': ('\'\'  # disable distcheck, requires doxygen'),
            }
        },
        'meson': False
    },
    {'name': 'arch', 'version': 'rolling',
     'flavor': 'archlinux' },  # see https://gitlab.freedesktop.org/wayland/ci-templates/merge_requests/19
    {'name': 'alpine', 'version': 'latest'},
]

# The various sources for templating
SOURCE_DIR = Path('.gitlab-ci')
TEMPLATE_FILE = 'gitlab-ci.tmpl'

BASE_DIR = Path('.')
OUTPUT_FILE = '.gitlab-ci.yml'

def generate_template():
    env = jinja2.Environment(loader=jinja2.FileSystemLoader(os.fspath(SOURCE_DIR)),
                             trim_blocks=True, lstrip_blocks=True)

    template = env.get_template(TEMPLATE_FILE)
    config = {'distributions': distributions}
    with open(BASE_DIR / OUTPUT_FILE, 'w') as fd:
        template.stream(config).dump(fd)


if __name__ == '__main__':
    description = ('''
    This script generates the .gitlab-ci.yml file.

    It must be run from the git repository root directory and will overwrite
    the existing .gitlab-ci.yml file.
    ''')
    parser = argparse.ArgumentParser(description=description)
    parser.parse_args()

    if not SOURCE_DIR.exists():
        print('Error: run me from the top-level tree')
        sys.exit(1)
    generate_template()
