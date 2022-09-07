#!/usr/bin/env python3

base = 348
rows = 6
dy = 30
for i in range(0,rows):
    y = base - ((rows-i-1)*dy)
    print(y)
