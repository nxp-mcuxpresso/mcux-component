# Copyright 2024-2025 NXP
# SPDX-License-Identifier: Apache-2.0

import os
import re
import sys
import subprocess
from pathlib import Path
import argparse
import pickle
import shutil
from textwrap import dedent

# Set the GENHAL_BASE directory
GENHAL_BASE        = Path(__file__).resolve().parent.parent
GENHAL_ZEPHYR_BASE = GENHAL_BASE / 'zephyr'
BOARD_DIR          = GENHAL_BASE / '_boards'
HAL_NXP            = GENHAL_BASE / 'modules' / 'hal' / 'nxp'

sys.path.insert(0,  str(GENHAL_ZEPHYR_BASE / 'scripts' / 'dts' / 'python-devicetree' / 'src'))

from devicetree import edtlib

os.environ['GENHAL_BASE'] = str(GENHAL_BASE)

# TODO: Support Shield
SHIELD_DIR = None
APPLICATION_SOURCE_DIR = None

WORKSPACE_DIR = Path('.') / 'dts_build_dir'

PYTHON_EXECUTABLE = sys.executable

# The directory containing devicetree related scripts
DT_SCRIPTS                      = GENHAL_ZEPHYR_BASE / 'scripts/dts'
# This parses and collects the DT information
GEN_EDT_SCRIPT                  = DT_SCRIPTS / 'gen_edt.py'
# This generates DT information needed by the C macro APIs,
# along with a few other things.
GEN_DEFINES_SCRIPT              = DT_SCRIPTS / 'gen_defines.py'
# The edtlib.EDT object in pickle format.
EDT_PICKLE                      = WORKSPACE_DIR / 'edt.pickle'
# The generated file containing the final DTS, for debugging.
ZEPHYR_DTS                      = WORKSPACE_DIR / 'zephyr.dts'
# The generated C header needed by <zephyr/devicetree.h>
DEVICETREE_GENERATED_H          = WORKSPACE_DIR / 'devicetree_generated.h'
# Generated build system internals.
DTS_POST_CPP                    = WORKSPACE_DIR / 'zephyr.dts.pre'
DTS_DEPS                        = WORKSPACE_DIR / 'zephyr.dts.d'

# This generates DT information needed by the Kconfig APIs.
GEN_DRIVER_KCONFIG_SCRIPT       = DT_SCRIPTS / 'gen_driver_kconfig_dts.py'
# Generated Kconfig symbols go here.
DTS_KCONFIG                     = WORKSPACE_DIR / 'Kconfig.dts'

DTS_OKAY_KCONFIG                = WORKSPACE_DIR / 'Kconfig.dts_okay'

# The location of a file containing known vendor prefixes, relative to
# each element of DTS_ROOT. Users can define their own in their own
# modules.
VENDOR_PREFIXES                 = Path('dts/bindings/vendor-prefixes.txt')

# TODO: Only ARM is supported for now
ARCH = "arm"

LICENSE_STR = '''# Generated devicetree Kconfig
#
# SPDX-License-Identifier: Apache-2.0

'''

def run_cmd(cmd):
    cmd = [str(x) for x in cmd]

    print(f'Running: {" ".join(cmd)}')
    ret = subprocess.run(cmd)

    if ret.returncode != 0:
        raise Exception(f"Command failed with return code: {ret.returncode}")

def get_dts_files():

    dts_source = GENHAL_BASE / '_boards' / args.board / f"{args.board}.dts"
    dts_source = dts_source.resolve()

    # TODO: Add board_extension_dts_files shield_dts_files
    dts_files = [ dts_source ]

    dts_files = dts_files + args.dtc_overlay_file

    print("Found devicetree files:")
    for dts_file in dts_files:
        print(f"{dts_file}")

    return dts_files

def get_dts_roots():

    dts_roots = [APPLICATION_SOURCE_DIR, BOARD_DIR, SHIELD_DIR, GENHAL_ZEPHYR_BASE, HAL_NXP ]
    dts_roots = [path.resolve() for path in dts_roots if path is not None]

    return dts_roots

def get_system_include_dirs(dts_roots, dts_arch_include):
    dirs = []

    for dts_root in dts_roots:
        for dts_root_path in [
            "include",
            "include/zephyr",
            "dts/common",
            dts_arch_include,
            "dts"
        ]:
            full_path = (dts_root / dts_root_path).resolve()
            if full_path.exists():
                dirs.append(full_path)

    return dirs

def get_build_args(dts_roots):
    dts_root_bindings  = []
    extra_gen_edt_args = []

    for dts_root in dts_roots:
        bindings_path = dts_root / 'dts' / 'bindings'
        if bindings_path.exists():
            dts_root_bindings.append(bindings_path)

        vendor_prefixes = dts_root / VENDOR_PREFIXES
        if os.path.exists(vendor_prefixes):
            extra_gen_edt_args.extend(["--vendor-prefixes", vendor_prefixes])

    dts_root_bindings = ';'.join(map(str, list(set(dts_root_bindings))))

    return dts_root_bindings, extra_gen_edt_args

def get_preprocessor():
    # Check if there is ARMGCC_DIR, or ARMCLANG_DIR in the environment
    # variables. If so, use the corresponding preprocessor.
    if 'ARMGCC_DIR' in os.environ:
        return Path(os.environ['ARMGCC_DIR']) / 'bin' / 'arm-none-eabi-gcc'
    if 'ARMCLANG_DIR' in os.environ:
        return Path(os.environ['ARMCLANG_DIR']) / 'ARM' / 'ARMCLANG' / 'bin' / 'armclang'

    # Check if there is gcc or clang in the PATH
    candidates = ['arm-none-eabi-gcc', 'gcc', 'clang', 'armclang']
    for candidate in candidates:
        if shutil.which(candidate):
            return candidate

    return ''

def get_preprocessor_cmd(c_pre_processor, source_files, extra_cppflags, include_directories):

    cpp = ''

    output = subprocess.run(f'{c_pre_processor} --version', stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    if output.returncode != 0:
        raise Exception(f"Failed to get the version of the preprocessor: {c_pre_processor}")

    if re.search(r'clang', output.stdout.decode()):
        cpp = 'clang'
    elif re.search(r'gcc', output.stdout.decode()):
        cpp = 'gcc'
    else:
        raise Exception(f"Unsupported preprocessor: {c_pre_processor}")

    include_opts = ' '.join([f"-isystem {dir}" for dir in include_directories]).split()
    source_opts  = ' '.join([f"-include {file}" for file in source_files]).split()
    deps_opts    = ["-MD", '-MF', f'{DTS_DEPS}']

    cmd = [c_pre_processor]

    if cpp == 'clang':
        cmd.extend(['-target', 'arm-arm-none-eabi'])

    cmd.extend(['-x', 'assembler-with-cpp'])
    cmd.extend(['-nostdinc'])
    cmd.extend(include_opts)
    cmd.extend(source_opts)
    cmd.extend(['-D__DTS__'])
    cmd.extend(extra_cppflags.split())
    cmd.extend(['-E'])  # Stop after preprocessing
    cmd.extend(deps_opts)
    cmd.extend(['-o', f'{DTS_POST_CPP}'])
    cmd.extend([GENHAL_ZEPHYR_BASE / 'misc' / 'empty_file.c'])

    return cmd

def dt_preprocess(c_pre_processor, source_files, extra_cppflags, include_directories):

    print(f"Preprocess the device tree")

    if c_pre_processor == '':
        c_pre_processor = get_preprocessor()

    if c_pre_processor == '':
        raise Exception("No preprocessor found")
    else:
        print(f"Using preprocessor: {c_pre_processor}")

    cmd = get_preprocessor_cmd(c_pre_processor, source_files, extra_cppflags, include_directories)

    run_cmd(cmd)

def gen_edt(dts_root_bindings, extra_gen_edt_args):
    #
    # Generate the etd.pickle, and the devicetree source
    #
    CMD_GEN_EDT = [
        PYTHON_EXECUTABLE,
        f"{GEN_EDT_SCRIPT}",
        "--dts", f"{DTS_POST_CPP}",
        "--dtc-flags", args.extra_dtc_flags,
        "--bindings-dirs", f"{dts_root_bindings}",
        "--dts-out", f"{ZEPHYR_DTS}",  # for debugging and dtc
        "--edt-pickle-out", f"{EDT_PICKLE}"
    ] + extra_gen_edt_args

    run_cmd(CMD_GEN_EDT)

def gen_defines():
    #
    # Generate the devicetree_generated.h file, which contains the C macro
    #
    CMD_GEN_DEFINES = [
        PYTHON_EXECUTABLE,
        GEN_DEFINES_SCRIPT,
        "--header-out", f"{DEVICETREE_GENERATED_H}",
        "--edt-pickle", f"{EDT_PICKLE}"
    ] + args.extra_gen_defines_args.split()

    run_cmd(CMD_GEN_DEFINES)

    print(f"Generated devicetree_generated.h: {DEVICETREE_GENERATED_H}")

def gen_dts_kconfig(dts_root_bindings):
    #
    # Generate the Kconfig.dts file, which contains all the DT Kconfig symbols.
    #
    cmd = [
        PYTHON_EXECUTABLE,
        GEN_DRIVER_KCONFIG_SCRIPT,
        "--kconfig-out", DTS_KCONFIG,
        "--bindings-dirs", dts_root_bindings,
    ]

    run_cmd(cmd)

def get_dt_kconfig():
    # Get the DT Kconfig symbols from the generated Kconfig.dts file, and the
    # corresponding dt_compat symbols.

    with open(DTS_KCONFIG, 'r') as fd:
        data = fd.read()

    # Find all dt_compat symbols name, collect them in a dictionary, for example:
    #
    # DT_COMPAT_ADAFRUIT_FEATHER_HEADER := adafruit-feather-header
    # will be collected in a dictionary like:
    # { 'DT_COMPAT_ADAFRUIT_FEATHER_HEADER': 'adafruit-feather-header' }
    dt_compat = re.findall(r'^(DT_COMPAT_\S+)\s+:=\s+(\S+)$', data, flags=re.MULTILINE)
    dt_compat = {k: v for k, v in dt_compat}

    # Find all DT Kconfig variables, and get their corresponding dt_compact symbol name, for example:
    #
    # DT_COMPAT_ADAFRUIT_FEATHER_HEADER := adafruit-feather-header
    # config DT_HAS_ADAFRUIT_FEATHER_HEADER_ENABLED
    #     def_bool $(dt_compat_enabled,$(DT_COMPAT_ADAFRUIT_FEATHER_HEADER))
    #
    # will be collected in a dictionary like:
    # { 'DT_HAS_ADAFRUIT_FEATHER_HEADER_ENABLED': 'adafruit-feather-header' }
    dt_kconfig = re.findall(r'^config\s+(DT_HAS_\S+_ENABLED)\s*$\s+def_bool\s+\$\(dt_compat_enabled,\$\((\S+)\)\)$', data, flags=re.MULTILINE)
    dt_kconfig = {k: dt_compat.get(v, v) for k, v in dt_kconfig}

    return dt_kconfig

def gen_dts_kconfig_okay():

    with open(EDT_PICKLE, 'rb') as fd:
        edt = pickle.load(fd)

    dt_kconfig = get_dt_kconfig()
    # Filter the DT Kconfig which are marked as okay
    dt_kconfig_okay = [k for k, v in dt_kconfig.items() if v in edt.compat2okay]

    with open(DTS_OKAY_KCONFIG, 'w') as fd:
        fd.write(LICENSE_STR)
        for k in dt_kconfig_okay:
            fd.write(f"config {k}\n")
            fd.write(f"    bool\n")
            fd.write(f"    default y\n")
            fd.write(f"\n")

def copy_result():
    # Copy the generated files to board directory
    board_dir = BOARD_DIR / args.board
    if args.output_dir != "":
        output_dir = Path(args.output_dir)
    else:
        output_dir = board_dir

    for file in [DEVICETREE_GENERATED_H, DTS_OKAY_KCONFIG]:
        print(f"Copy {file} to {output_dir}")
        shutil.copy(file, output_dir)

def parse_args():
    global args

    parser = argparse.ArgumentParser(
        description = dedent('''\
        Generate devicetree files (devicetree_generated.h, Kconfig.dts) for a board.

        To use this script, you need to at least provide the board name, for example:
        `python dts_build.py -b frdmrw612`

        This script needs a C preprocess (e.g., gcc, clang) to process the DTS files.
        You can specify the C preprocessor by setting the `--dts-preprocessor` option.
        For example, to use the GCC as the C preprocessor, you can run:
        `python dts_build.py -b frdmrw612 --dts-preprocessor=gcc`
        If you don't specify the C preprocessor, the script will try to find one in the
        environment variables ARMGCC_DIR, or ARMCLANG_DIR or in the PATH.
        Currently only GCC and Clang are supported.

        This script will generate the files in folder 'dts_build_dir' in the current directory,
        and copy devicetree_generated.h and Kconfig.dts to the board directory in `_boards/<board_name>`
        '''),

        formatter_class=argparse.RawTextHelpFormatter
    )

    parser.add_argument('-b', "--board", required=True,
                        help="board name, e.g., frdmrw612")

    parser.add_argument("--dtc-overlay-file", nargs='*', default=[],
                        help="list of devicetree overlay files which will be used to modify or extend the base devicetree")

    parser.add_argument("--extra-dtc-flags", action='store', type=str, default="",
                        help="list of extra command line options to pass to dtc when using it to check for additional errors and warnings")

    parser.add_argument("--dts-extra-cppflags", action='store', type=str, default="",
                        help="extra command line options to pass to the C preprocessor when generating the devicetree")

    parser.add_argument("--extra-gen-defines-args", action='store', type=str, default="",
                        help="extra command line options to control gen_defines.py")

    parser.add_argument("--dts-preprocessor", default="",
                        help="C preprocessor to use for devicetree files")
    parser.add_argument("--output-dir", nargs="?", default="", help="output dir of devicetree_generated.h and Kconfig.dts_okay in default they are generated in same folder of first overlay file")

    args = parser.parse_args()


def main():

    parse_args()

    os.makedirs(WORKSPACE_DIR, exist_ok=True)

    dts_files = get_dts_files()
    dts_roots = get_dts_roots()

    dts_arch_include = f"dts/{ARCH}"
    system_include_dirs = get_system_include_dirs(dts_roots, dts_arch_include)

    dts_root_bindings, extra_gen_edt_args = get_build_args(dts_roots)

    # Run the preprocessor on the DTS input files.
    dt_preprocess(args.dts_preprocessor, dts_files, args.dts_extra_cppflags, system_include_dirs)

    gen_edt(dts_root_bindings, extra_gen_edt_args)

    gen_defines()

    gen_dts_kconfig(dts_root_bindings)

    gen_dts_kconfig_okay()

    copy_result()


if __name__ == "__main__":
    main()
