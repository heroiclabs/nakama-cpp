#!/usr/bin/env python
import sys
import subprocess

abi_list = ['armeabi-v7a', 'arm64-v8a', 'x86']

def call(command):
    res = subprocess.call(command, shell=False)
    if res != 0:
        sys.exit(-1)

for abi in abi_list:
    print 'building for', abi
    call(['python', 'build_android.py', abi])
