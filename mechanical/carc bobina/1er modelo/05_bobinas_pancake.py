# ETAPA 5 - BOBINAS PANCAKE
# Blender 5.2
# Crea dos bobinas reales en forma de espiral.

import bpy
import math

X_POS = 55.0

COIL_INNER_R = 5.0
COIL_OUTER_R = 22.0
COIL_TURNS = 12
COIL_WIRE_D = 1.0
COIL_SEGMENTS_PER_TURN = 42

FRONT_COIL_Y = -7.4
REAR_COIL_Y = 7.4

def create_spiral_coil(name, y_pos, reverse=False):
    total_points = COIL_TURNS * COIL_SEGMENTS_PER_TURN + 1

    curve_data = bpy.data.curves.new(name=name + "_Curve", type='CURVE')
    curve_data.dimensions = '3D'
    curve_data.bevel_depth = COIL_WIRE_D / 2.0
    curve_data.bevel_resolution = 3

    spline = curve_data.splines.new('POLY')
    spline.points.add(total_points - 1)

    theta_max = 2.0 * math.pi * COIL_TURNS

    for i in range(total_points):
        t = i / (total_points - 1)
        theta = theta_max * t

        if reverse:
            theta = -theta

        r = COIL_INNER_R + (COIL_OUTER_R - COIL_INNER_R) * t

        x = r * math.cos(theta)
        z = r * math.sin(theta)

        spline.points[i].co = (
            X_POS + x,
            y_pos,
            z,
            1.0
        )

    coil = bpy.data.objects.new(name, curve_data)

    col = bpy.data.collections.get("04_BOBINAS")
    col.objects.link(coil)

    mat = bpy.data.materials.get("MAT_Cobre")
    if mat:
        curve_data.materials.append(mat)

    return coil

create_spiral_coil(
    "Bobina_Pancake_Delantera",
    FRONT_COIL_Y,
    reverse=False
)

create_spiral_coil(
    "Bobina_Pancake_Trasera",
    REAR_COIL_Y,
    reverse=True
)

print("ETAPA 5 COMPLETA: bobinas pancake creadas.")
