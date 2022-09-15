#!/usr/bin/env python3

import matplotlib as mpl
import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv("out.csv")
t = data['T'].tolist()
a = data['A'].tolist()
b = data['B'].tolist()

fig, ax = plt.subplots()
ax.set_ylim([-1, 1])
#ax.set_xlim([0, 1])
ax.grid(True)
#ax.plot(t, a)
ax.plot(t, b)

plt.savefig('out.png')
