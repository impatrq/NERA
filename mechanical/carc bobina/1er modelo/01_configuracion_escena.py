# ETAPA 1 - CONFIGURACION GENERAL DE LA ESCENA
# Blender 5.2
# Esta etapa deja preparadas las unidades, colecciones y materiales.
# Ejecutar primero.

import bpy

# -----------------------------
# MEDIDAS PRINCIPALES
# -----------------------------
W = 65.0   # ancho total (mm)
H = 70.0   # alto total (mm)
D = 35.0   # espesor total (mm)

# Limpia la escena
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

# Configura unidades en milimetros
scene = bpy.context.scene
scene.unit_settings.system = 'METRIC'
scene.unit_settings.length_unit = 'MILLIMETERS'
scene.unit_settings.scale_length = 0.001

# Crea colecciones
def ensure_collection(name):
    col = bpy.data.collections.get(name)
    if col is None:
        col = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(col)
    return col

ensure_collection("01_CARCASA_EXTERIOR")
ensure_collection("02_CORTE_ESTRUCTURAL")
ensure_collection("03_PLACAS")
ensure_collection("04_BOBINAS")
ensure_collection("05_SEPARADOR_SOPORTES")
ensure_collection("06_CABLEADO")

# Crea materiales basicos
def make_material(name, rgba):
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name=name)
    mat.diffuse_color = rgba
    return mat

make_material("MAT_Carcasa", (0.48, 0.45, 0.38, 1.0))
make_material("MAT_Placas", (0.20, 0.22, 0.24, 1.0))
make_material("MAT_Cobre", (0.55, 0.19, 0.07, 1.0))
make_material("MAT_Separador", (0.60, 0.62, 0.64, 1.0))
make_material("MAT_Soportes", (0.28, 0.30, 0.32, 1.0))

print("ETAPA 1 COMPLETA: escena configurada en milimetros.")
