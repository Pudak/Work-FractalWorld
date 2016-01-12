import math
import cmath
import cairo
from random import randint


IMAGE_SIZE = (5000, 5000)
NUM_SUBDIVISIONS = 6

def chooseRatio():
	return randint(0, 6)

colors = [(1.0, 0.35, 0.35), (0.4, 0.4, 1.0), (0.1, 1.0, 0.2), (0.7, 0.1, 0.8)]

def chooseColor():
	return colors[randint(0, len(colors) - 1)] 

Ratio = chooseRatio()*0.1 + 1.4

def subdivide(triangles):
    result = []
    for color, A, B, C in triangles:
        if color == 0:
            # Subdivide red triangle
            P = A + (B - A) / Ratio
            result += [(0, C, P, B), (1, P, C, A)]
        else:
            # Subdivide blue triangle
            Q = B + (A - B) / Ratio
            R = B + (C - B) / Ratio
            result += [(1, R, C, A), (1, Q, R, B), (0, R, Q, A)]
    return result

# Create wheel of red triangles around the origin
triangles = []
for i in xrange(10):
    B = cmath.rect(1, (2*i - 1) * math.pi / 10)
    C = cmath.rect(1, (2*i + 1) * math.pi / 10)
    if i % 2 == 0:
        B, C = C, B  # Make sure to mirror every second triangle
    triangles.append((0, 0j, B, C))

# Perform subdivisions
for i in xrange(NUM_SUBDIVISIONS):
    triangles = subdivide(triangles)

# Prepare cairo surface
surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, IMAGE_SIZE[0], IMAGE_SIZE[1])
cr = cairo.Context(surface)
cr.translate(IMAGE_SIZE[0] / 2.0, IMAGE_SIZE[1] / 2.0)
wheelRadius = 1.2 * math.sqrt((IMAGE_SIZE[0] / 2.0) ** 2 + (IMAGE_SIZE[1] / 2.0) ** 2)
cr.scale(wheelRadius, wheelRadius)

# Draw first triangles
for color, A, B, C in triangles:
    if color == 0:
        cr.move_to(A.real, A.imag)
        cr.line_to(B.real, B.imag)
        cr.line_to(C.real, C.imag)
        cr.close_path()
chosen = chooseColor()
colors.remove(chosen)
cr.set_source_rgb(*chosen)
cr.fill()    

# Draw second triangles
for color, A, B, C in triangles:
    if color == 1:
        cr.move_to(A.real, A.imag)
        cr.line_to(B.real, B.imag)
        cr.line_to(C.real, C.imag)
        cr.close_path()
chosen = chooseColor()
colors.remove(chosen)
cr.set_source_rgb(*chosen)
cr.fill()

# Determine line width from size of first triangle
color, A, B, C = triangles[0]
cr.set_line_width(abs(B - A) / 10.0)
cr.set_line_join(cairo.LINE_JOIN_ROUND)

# Draw outlines
for color, A, B, C in triangles:
    cr.move_to(C.real, C.imag)
    cr.line_to(A.real, A.imag)
    cr.line_to(B.real, B.imag)
cr.set_source_rgb(0.2, 0.2, 0.2)
cr.stroke()

# Save to PNG
surface.write_to_png('penrose.png')
