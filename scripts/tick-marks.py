#!/usr/bin/env python3

import math

def rescale(x, xMin, xMax, yMin, yMax):
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin)

def polar2cart(cx, cy, radius, angle):
  theta = math.radians(angle)
  return (
    round(cx + radius * math.cos(theta), 2),
    round(cy + radius * math.sin(theta), 2))

radius = 22.5
marks = 11

for x in range(marks):

    angle = rescale(x, 0, 11, 0, 330) + 120
    if angle > 360:
        angle = angle - 360

    (ax, ay) = polar2cart(0, 0, radius + 2, angle)
    (bx, by) = polar2cart(0, 0, radius + 6, angle)

    print(f'<polyline points="{ax} {ay} {bx} {by}" class="icon"/>')

for x in range(marks):

    angle = rescale(x, 0, 11, 0, 330) + 120
    if angle > 360:
        angle = angle - 360

    (ax, ay) = polar2cart(0, 0, radius + 8, angle)

    print(f'label {ax} {ay}')
