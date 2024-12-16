#!/usr/bin/env python3
#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: Apache-2.0

# Generates z_impl wrapper invocations for NXP generic HAL
# This script calls Zephyr syscalls scripts to generate all
# generic HAL needed syscall files in one step.
# The generated files will be in the folder SYSCALLS_OUTPUT

import os
import sys
import subprocess
from pathlib import Path

PYTHON_EXECUTABLE = sys.executable
GENHAL_BASE       = Path('..')

SYSCALLS_JSON     = 'syscalls.json'
STRUCT_TAGS_JSON  = 'struct_tags.json'

SYSCALLS_OUTPUT   = GENHAL_BASE / 'zephyr' / 'misc' / 'generated' / 'zephyr' / 'syscalls'
SYSCALLS_LIST     = GENHAL_BASE / 'zephyr' / 'misc' / 'generated' / 'zephyr' / 'syscall_list.h'

def run_cmd(cmd):
    cmd = [str(x) for x in cmd]

    print(f'Running: {" ".join(cmd)}')
    ret = subprocess.run(cmd)

    if ret.returncode != 0:
        raise Exception(f"Command failed with return code: {ret.returncode}")

def main():
    print('Generate syscall wrapper for Generic HAL')

    # Parse syscalls
    cmd = [
        PYTHON_EXECUTABLE,
        GENHAL_BASE / 'zephyr' / 'scripts' / 'build' / 'parse_syscalls.py',
        '--include', GENHAL_BASE  / 'zephyr' / 'include',
        '--json-file', SYSCALLS_JSON,
        '--tag-struct-file', STRUCT_TAGS_JSON
    ]
    run_cmd(cmd)

    # Generate the syscall header files
    cmd = [
        PYTHON_EXECUTABLE,
        GENHAL_BASE / 'zephyr' / 'scripts' / 'build' / 'gen_sdk_syscalls.py',
        '--json-file', SYSCALLS_JSON,
        '--base-output', SYSCALLS_OUTPUT,
        '--syscall-list', SYSCALLS_LIST
    ]
    run_cmd(cmd)

    print('Generated successfully')

if __name__ == "__main__":
    main()
