#!/usr/bin/env python3

# This file generates the .gitlab-ci.yml file that defines the pipeline.

import argparse
import jinja2
import os
import sys
import yaml

from pathlib import Path

# The various sources for templating
SOURCE_DIR = Path('.gitlab-ci')
CONFIG_FILE = 'gitlab-ci-config.yaml'
TEMPLATE_FILE = 'gitlab-ci.tmpl'

BASE_DIR = Path('.')
OUTPUT_FILE = '.gitlab-ci.yml'


def generate_template():
    with open(SOURCE_DIR / CONFIG_FILE) as fd:
        config = yaml.safe_load(fd)

    env = jinja2.Environment(loader=jinja2.FileSystemLoader(os.fspath(SOURCE_DIR)),
                             trim_blocks=True, lstrip_blocks=True)

    template = env.get_template(TEMPLATE_FILE)
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
