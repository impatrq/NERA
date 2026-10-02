# ETAPA 1 - CONFIGURACION GENERAL DE LA ESCENA
# Blender 5.2 - Proyecto N.E.R.A.
# Ejecutar primero. Solo esta etapa limpia la escena.
import bpy

# Cotas maestras (mm)
HOUSING_W, HOUSING_H, HOUSING_D = 73.0, 78.0, 14.0
CAVITY_W, CAVITY_H = 67.0, 72.0
WALL_DEPTH = 2.0
CORNER_R = 6.0
PCB_W, PCB_H, PCB_T = 65.0, 70.0, 1.6
SUPPORT_W, SUPPORT_H, SUPPORT_T = 66.5, 71.5, 1.2
M3_X, M3_Z = 30.5, 33.0

# Dos vistas: carcasa completa a la izquierda y cutaway a la derecha.
FULL_X = -55.0
CUT_X = 55.0

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
scene=bpy.context.scene
scene.unit_settings.system='METRIC'
scene.unit_settings.length_unit='MILLIMETERS'
scene.unit_settings.scale_length=0.001

def ensure_collection(name):
    c=bpy.data.collections.get(name)
    if c is None:
        c=bpy.data.collections.new(name); scene.collection.children.link(c)
    return c
for n in ['01_CARCASA_EXTERIOR','02_CORTE_ESTRUCTURAL','03_PLACAS','04_BOBINAS','05_SEPARADOR_SOPORTES','06_CABLEADO']:
    ensure_collection(n)

def mat(name, rgba):
    m=bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.diffuse_color=rgba; return m
mat('MAT_Carcasa',(0.48,0.45,0.38,1)); mat('MAT_Placas',(0.20,0.22,0.24,1))
mat('MAT_PCB',(0.13,0.25,0.13,1)); mat('MAT_Cobre',(0.55,0.19,0.07,1))
mat('MAT_Separador',(0.60,0.62,0.64,1)); mat('MAT_Soportes',(0.28,0.30,0.32,1))
print('ETAPA 1 COMPLETA: escena vertical configurada.')
