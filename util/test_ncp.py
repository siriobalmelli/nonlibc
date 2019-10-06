#!/usr/bin/env python3

#   test_ncp.py
# test 'ncp' utility for correctness

import subprocess
import sys
import os
import filecmp
import collections as col
import signal

ncp = 'util/ncp'
test_dir = 'test_copy/'

input_files = [
    'copy.bin',
    'copy_one.bin',
    'copy_two.bin',
    'copy_three.bin',
    'copy_four.bin'
    ]
output_files = [
    'no_flags.copy',
    'verbose.copy',
    'test_copy/no_flags.copy',
    'test_copy/verbose.copy',
    'test_copy/copy_one.bin',
    'test_copy/copy_two.bin',
    'test_copy/copy_three.bin',
    'test_copy/copy_four.bin'
    ]

verbose_output = [
    "",  # No-output is valid
    "'copy.bin' -> 'verbose.copy'",
    "'no_flags.copy' -> 'test_copy/no_flags.copy'\n" +
    "'verbose.copy' -> 'test_copy/verbose.copy'",
    "'copy_one.bin' -> 'test_copy/copy_one.bin'\n" +
    "'copy_two.bin' -> 'test_copy/copy_two.bin'",
    "'copy_three.bin' -> 'test_copy/copy_three.bin'",
    "'copy_four.bin' -> 'test_copy/copy_four.bin'"
    ]


def delete_files():
    '''delete_files()
    Delete all files used in test, whether successful or not.
    '''
    try:
        for tgt in input_files + [test_dir]:
            sub = subprocess.run(['rm', '-rf', tgt],
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE,
                                 shell=False, check=True)
            print(sub.args)
    except subprocess.CalledProcessError as err:
        print(err.cmd, file=sys.stderr)
        print(err.stdout.decode('ascii'), file=sys.stderr)
        print(err.stderr.decode('ascii'), file=sys.stderr)
        exit(1)


def fail(error_message):
    '''fail()
    Clean up before exiting with a non-zero return code.
    '''
    print(error_message, file=sys.stderr)
    delete_files()
    exit(1)


signal.signal(signal.SIGINT, lambda sig, stack: fail(f"caught signal {sig}"))


def create_random_file(filename):
    '''create_random_file()
    Pipe from /dev/urandom to create a 1M file named 'filename'.
    '''
    try:
        subprocess.run(['dd', 'if=/dev/urandom', 'of='+filename,
                        'bs=1024', 'count=1024'],
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                       shell=False, check=True)
    except subprocess.CalledProcessError as err:
        fail(err.cmd)


def call_ncp(cmds, ncp):
    '''call_ncp()
    Call 'ncp' with 'cmds' arguments.
    '''
    try:
        cmds.insert(0, ncp)
        sub = subprocess.run(cmds,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             shell=False, check=True)
        print(sub.args)
        return sub.stdout.decode('ascii'), sub.stderr.decode('ascii')
    except subprocess.CalledProcessError as err:
        fail(f"{err.cmd}\n" +
             "{err.stdout.decode('ascii')}\n" +
             "{err.stderr.decode('ascii')}")


def check_output_files():
    for out_file in output_files:
        if os.path.isfile(out_file):
            # compare 'copy_XXXX.bin' files.
            if out_file.endswith('.bin'):
                for i, o in [(1, 4), (2, 5), (3, 6), (4, 7)]:
                    fin = input_files[i]
                    fout = output_files[o]
                    if not filecmp.cmp(fin, fout):
                        fail(f'file {fin} not equal to {fout}', stderr)
            elif not filecmp.cmp(input_files[0], out_file):
                fail(f'file {input_files[0]} not equal to {out_file}')
        else:
            fail(f'file {out_file} does not exist')

    # delete all files if all checks have succeeded
    for f in output_files:
        os.remove(f)
    for f in input_files:
        os.remove(f)
    delete_files()


# Use OrderedDict such that iterations are always in order,
# 'force' tests should always see a previously created file.
cmds = col.OrderedDict([
         # File to File
         ('clean', [input_files[0], output_files[0]]),
         ('force', ['-f', input_files[0], output_files[0]]),
         ('verbose', ['-v', input_files[0], output_files[1]]),
         ('force_verbose', ['-v', '-f', input_files[0], output_files[1]]),
         # File to Directory
         ('dir_clean', [input_files[3], test_dir]),
         ('dir_force', ['-f', input_files[3], test_dir]),
         ('dir_verbose', ['-v', input_files[4], test_dir]),
         ('dir_verbose_force', ['-v', '-f', input_files[4], test_dir]),
         # Files to Directory
         ('files_clean', [output_files[0], output_files[1], test_dir]),
         ('files_force', ['-f', output_files[0], output_files[1], test_dir]),
         ('files_verbose', ['-v', input_files[1], input_files[2], test_dir]),
         ('files_verbose_force',
             ['-v', '-f', input_files[1], input_files[2], test_dir])
        ])


#   main()
if __name__ == "__main__":
    if sys.argv[1] is not None:
        ncp = sys.argv[1]

    if not os.path.exists(test_dir):
        os.mkdir(test_dir)

    for f in input_files:
        create_random_file(f)

    for k, v in cmds.items():
        stdout, stderr = call_ncp(v, ncp)
        # Check output, if any
        if stdout.rstrip() not in verbose_output:
            fail(stdout)
        elif stderr:
            fail(stderr)

    check_output_files()
