#!/usr/bin/env python3

import glob
import os
import subprocess

files = glob.glob('res-src/*-src.svg')

for fin in files:
    fout = fin.replace('-src', '')

    if not os.path.exists(fout) or os.path.getmtime(fin) > os.path.getmtime(fout):
        print('Processing ' + fout + '...')

        # read input
        lin = open(fin, 'r').readlines()
        
        # pre-process output
        lout = []
        for ln in lin:
            # skip the blueprint elements
            if 'class="blueprint"' not in ln:
                lout.append(ln)

        # write output
        open(fout, 'w').writelines(lout)

        # Run it through inkscape
        # Adapted from github.com/bogaudio/BogaudioModules/scripts/svg_render.rb
        subprocess.run([
            '/Applications/Inkscape.app/Contents/MacOS/inkscape', 
            '--batch-process', 
            "--actions=EditSelectAll;SelectionUnGroup;EditSelectAll;EditUnlinkClone;EditSelectAll;ObjectToPath;FileSave",
            fout],
            check=True)

print('SVG rendering is done.')
