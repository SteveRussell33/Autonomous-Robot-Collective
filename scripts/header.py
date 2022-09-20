#!/usr/bin/env python3

import glob
import os
import subprocess

#------------------------------------

def find_header(lines):

    found = False
    begin = -1
    end = -1

    for i in range(0, len(lines)):
        if 'header begin' in lines[i]:
            begin = i
        elif 'header end' in lines[i]:
            end = i

    if begin != -1 and end != -1:
        found = True

    return (found, begin, end)

#------------------------------------

header = open('res-src/header.svg', 'r').readlines()

files = glob.glob('res-src/*-src.svg')

for f in files:

    lin = open(f, 'r').readlines()

    (found, begin, end) = find_header(lin)
    if not found:
        continue
    print('Updating ' + f)

    before = lin[0:begin]
    after = lin[end+1:len(lin)]

    lout = []
    lout.extend(before)
    lout.extend(header)
    lout.extend(after)

    open(f, 'w').writelines(lout)
    
