# ETAPA 4 - PLACAS DE SOPORTE Y ORIFICIOS DE ENCASTRE
# Blender 5.2
# Crea las dos placas internas con 4 agujeros cada una.

import bpy
import math

X_POS = 55.0

PLATE_W = 54.0
PLATE_H = 58.0
PLATE_T = 1.6
PLATE_R = 3.0

FRONT_PLATE_Y = -8.5
REAR_PLATE_Y = 8.5

MOUNT_HOLE_R = 1.4
MOUNT_X = 22.0
MOUNT_Z = 24.0

def activate(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

def move_to_collection(obj, name):
    col = bpy.data.collections.get(name)
    for c in list(obj.users_collection):
        c.objects.unlink(obj)
    col.objects.link(obj)

def create_plate(name, y_pos):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(X_POS, y_pos, 0))
    plate = bpy.context.object
    plate.name = name
    plate.dimensions = (PLATE_W, PLATE_T, PLATE_H)

    activate(plate)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    bevel = plate.modifiers.new("Redondeo", 'BEVEL')
    bevel.width = PLATE_R
    bevel.segments = 6
    activate(plate)
    bpy.ops.object.modifier_apply(modifier=bevel.name)

    hole_positions = [
        (-MOUNT_X, -MOUNT_Z),
        ( MOUNT_X, -MOUNT_Z),
        (-MOUNT_X,  MOUNT_Z),
        ( MOUNT_X,  MOUNT_Z),
    ]

    for i, (hx, hz) in enumerate(hole_positions):
        bpy.ops.mesh.primitive_cylinder_add(
            vertices=48,
            radius=MOUNT_HOLE_R,
            depth=PLATE_T + 6.0,
            location=(X_POS + hx, y_pos, hz),
            rotation=(math.radians(90), 0, 0)
        )
        cutter = bpy.context.object
        cutter.name = f"{name}_Agujero_{i+1}"

        mod = plate.modifiers.new(f"Agujero_{i+1}", 'BOOLEAN')
        mod.operation = 'DIFFERENCE'
        mod.solver = 'EXACT'
        mod.object = cutter

        activate(plate)
        bpy.ops.object.modifier_apply(modifier=mod.name)
        bpy.data.objects.remove(cutter, do_unlink=True)

    move_to_collection(plate, "03_PLACAS")

    mat = bpy.data.materials.get("MAT_Placas")
    if mat:
        plate.data.materials.append(mat)

    return plate

create_plate("Placa_Soporte_Delantera", FRONT_PLATE_Y)
create_plate("Placa_Soporte_Trasera", REAR_PLATE_Y)

print("ETAPA 4 COMPLETA: placas y agujeros creados.")
