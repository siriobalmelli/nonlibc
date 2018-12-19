#!/usr/bin/env python3
'''test_fnvsum.py
Test the 'fnvsum' utility by running with various inputs
and checking again expected output.
(c) 2018 Sirio Balmelli
'''

#   test_fnvsum.py
# test 'fnvsum' utility for correctness

import subprocess
import sys
import os

HASH64 = {
    b''                                              : 0xcbf29ce484222325,
    b'the quick brown fox jumped over the lazy dog'  : 0x4fb124b03ec8f8f8,
    b'/the/enemy/gate/is/down'                       : 0x7814fb571359f23e,
    b'\t{}[]*\&^%$#@!'                               : 0xa8d4c7c3b9738aef,  #pylint: disable=anomalous-backslash-in-string
    b'eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee'        : 0xb47617d43071893b,
    b'The player formerly known as mousecop'         : 0x400b51cb52c3929d,
    b'Dan Smith'                                     : 0x088a7d587bd339f3,
    b'blaar'                                         : 0x4b64e9abbc760b0d
}
HASH32 = {
    b''                                              : 0x811c9dc5,
    b'the quick brown fox jumped over the lazy dog'  : 0x406d1fd8,
    b'/the/enemy/gate/is/down'                       : 0x45d2df9e,
    b'\t{}[]*\&^%$#@!'                               : 0x7e928eaf,  #pylint: disable=anomalous-backslash-in-string
    b'eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee'        : 0xc83e6efb,
    b'The player formerly known as mousecop'         : 0x7b8e245d,
    b'Dan Smith'                                     : 0x0d2b7f73,
    b'blaar'                                         : 0x6f93f02d
}

FNVSUM = 'util/fnvsum'



def fnvsum_stdin(str_in, expect_fnv, args=[]):  # pylint: disable=dangerous-default-value
    '''run fnvsum with 'string' as stdinput; verify that the output matches 'expect_fnv'
    '''
    try:
        sub = subprocess.run([FNVSUM] + args + ['-'], input=str_in,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             shell=False, check=True)
    except subprocess.CalledProcessError as err:
        print(err.cmd)
        print(err.stdout.decode('ascii'))
        print(err.stderr.decode('ascii'))
        exit(1)

    res = int(sub.stdout.decode('ascii').split(' ')[0], 16)
    if res != expect_fnv:
        print('%xd != %xd;  %s' % (res, expect_fnv, str_in), sys.stderr)
        exit(1)



def fnvsum_file(str_in, expect_fnv, args=[]):  # pylint: disable=dangerous-default-value
    '''put 'string' in a file; run fnvsum against that file;
        verify that output matches 'expect_fnv'
    '''
    # write string to file
    with open('./temp', 'wb') as fil:
        fil.write(str_in)

    try:
        sub = subprocess.run([FNVSUM] + args + ['./temp'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             shell=False, check=True)
    except subprocess.CalledProcessError as err:
        print(err.cmd)
        print(err.stdout.decode('ascii'))
        print(err.stderr.decode('ascii'))
        exit(1)

    res = int(sub.stdout.decode('ascii').split(' ')[0], 16)
    if res != expect_fnv:
        print('%xd != %xd;  %s' % (res, expect_fnv, str_in), sys.stderr)
        exit(1)

    os.remove('./temp')



#   main()
if __name__ == "__main__":
    if sys.argv[1] is not None:
        FNVSUM = sys.argv[1]

    for string, fnv in HASH64.items():
        fnvsum_stdin(string, fnv)
        fnvsum_stdin(string, fnv, ['-l 64'])
        fnvsum_file(string, fnv)
        fnvsum_file(string, fnv, ['-l 64'])

    for string, fnv in HASH32.items():
        fnvsum_stdin(string, fnv, ['-l 32'])
        fnvsum_file(string, fnv, ['-l 32'])
