# ETAPA 3 - MEDIA CARCASA PARA VER EL INTERIOR
# Blender 5.2
# Crea una segunda carcasa y corta la mitad frontal.
# Ejecutar despues de las etapas 1 y 2.

import bpy

W = 65.0
H = 70.0
D = 35.0

CORNER_R = 10.0
WALL = 2.2
INNER_R = CORNER_R - WALL
X_POS = 55.0

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

def boolean_apply(target, cutter, operation):
    mod = target.modifiers.new("Boolean", 'BOOLEAN')
    mod.operation = operation
    mod.solver = 'EXACT'
    mod.object = cutter
    activate(target)
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.data.objects.remove(cutter, do_unlink=True)

shell = rounded_box(
    "Carcasa_Corte_Mitad",
    W, D, H, CORNER_R,
    (X_POS, 0, 0)
)

inner = rounded_box(
    "Volumen_Interior_Corte",
    W - 2*WALL,
    D - 2*WALL,
    H - 2*WALL,
    INNER_R,
    (X_POS, 0, 0)
)

boolean_apply(shell, inner, 'DIFFERENCE')

# Cubo que conserva solamente la mitad posterior
bpy.ops.mesh.primitive_cube_add(
    size=1.0,
    location=(X_POS, D/4.0, 0)
)
keeper = bpy.context.object
keeper.name = "Volumen_Mitad_Conservada"
keeper.dimensions = (W + 20, D/2.0, H + 20)

activate(keeper)
bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

boolean_apply(shell, keeper, 'INTERSECT')

move_to_collection(shell, "02_CORTE_ESTRUCTURAL")

mat = bpy.data.materials.get("MAT_Carcasa")
if mat:
    shell.data.materials.append(mat)

print("ETAPA 3 COMPLETA: media carcasa creada.")
