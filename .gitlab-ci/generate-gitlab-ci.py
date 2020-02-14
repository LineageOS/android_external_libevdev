#!/usr/bin/env python3
# vim: set expandtab shiftwidth=4:

# This file generates the .gitlab-ci.yml file that defines the pipeline.

import jinja2

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
    },
    {
        'name': 'centos', 'version': '8',
        'build': {
            'extra_variables': {
                'MAKE_ARGS': ('\'\'  # disable distcheck, requires doxygen'),
            }
        },
    },
    {'name': 'arch', 'version': 'rolling',
     'flavor': 'archlinux' },  # see https://gitlab.freedesktop.org/wayland/ci-templates/merge_requests/19
    {'name': 'alpine', 'version': 'latest'},
]


def generate_template():
    env = jinja2.Environment(loader=jinja2.FileSystemLoader('./.gitlab-ci'),
                             trim_blocks=True, lstrip_blocks=True)

    template = env.get_template('gitlab-ci.tmpl')
    config = {'distributions': distributions}
    with open('.gitlab-ci.yml', 'w') as fd:
        template.stream(config).dump(fd)


if __name__ == '__main__':
    generate_template()
