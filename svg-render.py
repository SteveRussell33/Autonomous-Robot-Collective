#!/usr/bin/env python3

import os
import subprocess

files = [
    'ATV',
    'FM',
    'hswitch-0',
    'hswitch-1',
    'knob18',
    'knob24',
    'knob45',
    'MIX2',
    'poly-port',
    'toggle-0',
    'toggle-1'
]

for f in files:
    fin = 'res-src/' + f + '-src.svg'
    fout = 'res/' + f + '.svg'

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
