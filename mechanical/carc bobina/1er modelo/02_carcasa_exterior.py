# ETAPA 2 - CARCASA EXTERIOR COMPLETA
# Blender 5.2
# Crea la carcasa cerrada de 65 x 70 x 35 mm.
# Ejecutar despues de 01_configuracion_escena.py

import bpy

W = 65.0
H = 70.0
D = 35.0

CORNER_R = 10.0
WALL = 2.2
INNER_R = CORNER_R - WALL
X_POS = -55.0

def activate(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

def move_to_collection(obj, name):
    col = bpy.data.collections.get(name)
    for c in list(obj.users_collection):
        c.objects.unlink(obj)
    col.objects.link(obj)

def rounded_box(name, sx, sy, sz, radius, location):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = (sx, sy, sz)

    activate(obj)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    bevel = obj.modifiers.new("Redondeo", 'BEVEL')
    bevel.width = radius
    bevel.segments = 8
    bevel.limit_method = 'ANGLE'
    activate(obj)
    bpy.ops.object.modifier_apply(modifier=bevel.name)

    return obj

def boolean_difference(target, cutter):
    mod = target.modifiers.new("Vaciado_Interior", 'BOOLEAN')
    mod.operation = 'DIFFERENCE'
    mod.solver = 'EXACT'
    mod.object = cutter
    activate(target)
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.data.objects.remove(cutter, do_unlink=True)

outer = rounded_box(
    "Carcasa_Exterior_Completa",
    W, D, H, CORNER_R,
    (X_POS, 0, 0)
)

inner = rounded_box(
    "Volumen_Interior",
    W - 2*WALL,
    D - 2*WALL,
    H - 2*WALL,
    INNER_R,
    (X_POS, 0, 0)
)

boolean_difference(outer, inner)

move_to_collection(outer, "01_CARCASA_EXTERIOR")

mat = bpy.data.materials.get("MAT_Carcasa")
if mat:
    outer.data.materials.append(mat)

print("ETAPA 2 COMPLETA: carcasa exterior creada.")
