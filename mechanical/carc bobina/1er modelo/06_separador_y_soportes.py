# ETAPA 6 - SEPARADOR CENTRAL Y SOPORTES INTERNOS
# Blender 5.2
# Crea el separador central y cuatro postes de soporte/encastre.

import bpy
import math

X_POS = 55.0

SPACER_W = 6.0
SPACER_H = 52.0
SPACER_T = 1.8
SPACER_R = 2.0

POST_R = 2.2
POST_DEPTH = 8.0
MOUNT_X = 22.0
MOUNT_Z = 24.0
POST_Y = 12.0

def activate(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

def move_to_collection(obj, name):
    col = bpy.data.collections.get(name)
    for c in list(obj.users_collection):
        c.objects.unlink(obj)
    col.objects.link(obj)

# Separador
bpy.ops.mesh.primitive_cube_add(size=1.0, location=(X_POS, 0, 0))
sep = bpy.context.object
sep.name = "Separador_Central"
sep.dimensions = (SPACER_W, SPACER_T, SPACER_H)

activate(sep)
bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

bevel = sep.modifiers.new("Redondeo", 'BEVEL')
bevel.width = SPACER_R
bevel.segments = 6

activate(sep)
bpy.ops.object.modifier_apply(modifier=bevel.name)

# Agujero central del separador
bpy.ops.mesh.primitive_cylinder_add(
    vertices=64,
    radius=5.5,
    depth=SPACER_T + 8.0,
    location=(X_POS, 0, 0),
    rotation=(math.radians(90), 0, 0)
)

cutter = bpy.context.object

mod = sep.modifiers.new("Ventana_Central", 'BOOLEAN')
mod.operation = 'DIFFERENCE'
mod.solver = 'EXACT'
mod.object = cutter

activate(sep)
bpy.ops.object.modifier_apply(modifier=mod.name)
bpy.data.objects.remove(cutter, do_unlink=True)

move_to_collection(sep, "05_SEPARADOR_SOPORTES")

mat = bpy.data.materials.get("MAT_Separador")
if mat:
    sep.data.materials.append(mat)

# Postes de soporte / encastre
positions = [
    (-MOUNT_X, -MOUNT_Z),
    ( MOUNT_X, -MOUNT_Z),
    (-MOUNT_X,  MOUNT_Z),
    ( MOUNT_X,  MOUNT_Z),
]

for i, (px, pz) in enumerate(positions):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=48,
        radius=POST_R,
        depth=POST_DEPTH,
        location=(X_POS + px, POST_Y, pz),
        rotation=(math.radians(90), 0, 0)
    )

    post = bpy.context.object
    post.name = f"Soporte_Encastre_{i+1}"

    move_to_collection(post, "05_SEPARADOR_SOPORTES")

    mat_post = bpy.data.materials.get("MAT_Soportes")
    if mat_post:
        post.data.materials.append(mat_post)

print("ETAPA 6 COMPLETA: separador y soportes creados.")
