# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 1 / 8
# ESCENA + PARAMETROS CENTRALES
# Blender 5.2
# ============================================================
# Ejes: X=ancho, Y=profundidad, Z=alto. Unidades: milimetros.
#
# CONFIRMADO:
# - Carcasa exterior: 93 x 35 x 100 mm
# - Pared: 2.2 mm
# - R exterior 10 mm / R interior 7.8 mm
# - 2 CoilBoards fisicas: 82 x BOBINA_Y x 89 mm
# - CoilBoards pegadas entre si, sin separador fisico
# - Cierre desmontable M3: agujero pasante FRONT + tuerca cautiva BACK
# - Las esquinas libres de la CoilBoard se aprovechan para los 4 ejes M3
# - CARCASA_FRONT y CARCASA_BACK seran las piezas imprimibles finales
# ============================================================

import bpy

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

scene=bpy.context.scene
scene.unit_settings.system='METRIC'
scene.unit_settings.length_unit='MILLIMETERS'
scene.unit_settings.scale_length=0.001

# CARCASA
scene['NERA_CARCASA_X']=93.0
scene['NERA_CARCASA_Y']=35.0
scene['NERA_CARCASA_Z']=100.0
scene['NERA_PARED']=2.2
scene['NERA_RADIO_EXT']=10.0
scene['NERA_RADIO_INT']=7.8

# COILBOARDS
scene['NERA_COILBOARD_X']=82.0
scene['NERA_COILBOARD_Y']=1.0  # PROVISIONAL: medir espesor real.
scene['NERA_COILBOARD_Z']=89.0
scene['NERA_COIL_GLUE_GAP']=0.0
# Ejes M3 en las esquinas libres de la placa. AJUSTAR tras medir la CoilBoard real.
scene['NERA_COIL_HOLE_INSET_X']=5.0
scene['NERA_COIL_HOLE_INSET_Z']=5.0
scene['NERA_COIL_HOLE_D']=3.4

# CIERRE M3
scene['NERA_M3_CLEARANCE_D']=3.4
scene['NERA_M3_NUT_AF']=5.5
scene['NERA_M3_NUT_THICK']=2.4
scene['NERA_M3_NUT_POCKET_AF']=5.9
scene['NERA_M3_NUT_POCKET_DEPTH']=2.7
scene['NERA_M3_BOSS_D']=10.0
scene['NERA_M3_BOSS_DEPTH']=6.0
scene['NERA_M3_SCREW_LENGTH']=20.0
scene['NERA_M3_HEAD_D']=5.8
scene['NERA_M3_HEAD_H']=2.5

# ENCASTRE MACHO / HEMBRA ENTRE MITADES
# El macho esta en BACK y entra en FRONT.
scene['NERA_LIP_DEPTH']=1.5
scene['NERA_LIP_RADIAL_THICKNESS']=1.0
scene['NERA_LIP_OUTER_INSET']=0.6
scene['NERA_LIP_CLEARANCE']=0.4  # juego diametral/total inicial para FDM
scene['NERA_LIP_OVERLAP']=0.12   # solape para boolean UNION robusto

# SALIDA DE CABLE
scene['NERA_CABLE_D']=7.6
scene['NERA_CABLE_X']=18.0
scene['NERA_CABLE_Z']=-42.0

# VISTA / REFERENCIA
scene['NERA_REFERENCE_OFFSET_X']=-125.0
scene['NERA_ASSEMBLY_X']=0.0

# DERIVADOS
IX=scene['NERA_CARCASA_X']-2*scene['NERA_PARED']
IY=scene['NERA_CARCASA_Y']-2*scene['NERA_PARED']
IZ=scene['NERA_CARCASA_Z']-2*scene['NERA_PARED']
scene['NERA_INTERIOR_X']=IX; scene['NERA_INTERIOR_Y']=IY; scene['NERA_INTERIOR_Z']=IZ
by=scene['NERA_COILBOARD_Y']; gap=scene['NERA_COIL_GLUE_GAP']
center=gap/2+by/2
scene['NERA_COIL_FRONT_Y']=-center
scene['NERA_COIL_BACK_Y']=+center

# COLECCIONES
def ensure_collection(name):
    col=bpy.data.collections.get(name)
    if col is None:
        col=bpy.data.collections.new(name); scene.collection.children.link(col)
    return col
for n in ('NERA_REFERENCIA','NERA_IMPRESION','NERA_COILBOARDS','NERA_HARDWARE_REF','NERA_ORIFICIOS'):
    ensure_collection(n)

# MATERIALES
def ensure_material(name,rgba,metallic=0.0,roughness=0.5):
    m=bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.diffuse_color=rgba; m.metallic=metallic; m.roughness=roughness
    return m
ensure_material('NERA_MAT_CARCASA',(0.68,0.66,0.61,1),0,0.48)
ensure_material('NERA_MAT_FRONT',(0.74,0.72,0.67,1),0,0.46)
ensure_material('NERA_MAT_BACK',(0.62,0.61,0.57,1),0,0.48)
ensure_material('NERA_MAT_COILBOARD_FRONT',(0.95,0.34,0.06,1),0,0.42)
ensure_material('NERA_MAT_COILBOARD_BACK',(0.78,0.22,0.03,1),0,0.45)
ensure_material('NERA_MAT_HARDWARE',(0.56,0.58,0.62,1),0.75,0.24)

if IX<=scene['NERA_COILBOARD_X'] or IZ<=scene['NERA_COILBOARD_Z']:
    raise ValueError('NERA: la CoilBoard no entra en la cavidad X/Z.')

print('NERA ETAPA 1 OK')
print(f'Carcasa: 93 x 35 x 100 | interior {IX:.1f} x {IY:.1f} x {IZ:.1f} mm')
print('Version de fabricacion: FRONT/BACK + encastre macho/hembra + 4 cierres M3.')
