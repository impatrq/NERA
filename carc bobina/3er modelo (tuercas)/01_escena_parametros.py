# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 1 / 8
# ESCENA + PARAMETROS CENTRALES
# Blender 5.2
# ============================================================
# Ejes:
#   X = ancho
#   Y = profundidad / espesor
#   Z = alto
#
# CONFIRMADO:
# - Carcasa exterior: 93 x 35 x 100 mm
# - Pared: 2.2 mm
# - Radio exterior: 10 mm
# - Radio interior: 7.8 mm
# - 2 CoilBoards fisicas: 82 x BOBINA_Y x 89 mm
# - Cada CoilBoard es la placa rectangular REAL que ya contiene
#   la bobina pancake redonda. Por eso las esquinas quedan libres.
# - Las CoilBoards NO se imprimen.
# - Las dos CoilBoards se pegan entre si: NO hay separador fisico.
# - Cierre / sujecion visual con M3.
#
# PENDIENTE / EDITABLE:
# - BOBINA_Y (espesor real de cada CoilBoard)
# - posicion exacta de agujeros de esquina en la CoilBoard
# ============================================================

import bpy

# Solo esta etapa limpia toda la escena.
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

scene = bpy.context.scene
scene.unit_settings.system = 'METRIC'
scene.unit_settings.length_unit = 'MILLIMETERS'
scene.unit_settings.scale_length = 0.001

# --------------------------
# CARCASA - CONFIRMADO
# --------------------------
scene['NERA_CARCASA_X'] = 93.0
scene['NERA_CARCASA_Y'] = 35.0
scene['NERA_CARCASA_Z'] = 100.0
scene['NERA_PARED'] = 2.2
scene['NERA_RADIO_EXT'] = 10.0
scene['NERA_RADIO_INT'] = 7.8

# --------------------------
# COILBOARDS - CONFIRMADO X/Z
# --------------------------
scene['NERA_COILBOARD_X'] = 82.0
scene['NERA_COILBOARD_Y'] = 1.0  # PROVISIONAL: medir espesor real.
scene['NERA_COILBOARD_Z'] = 89.0

# Las CoilBoards se pegan entre si. Por defecto se representan TOCANDOSE.
# Si se desea una minima separacion solo para visualizar, cambiar este valor.
scene['NERA_COIL_GLUE_GAP'] = 0.0

# Esquinas libres de la placa: agujeros M3 de referencia.
# PARAMETRICO: ajustar con la placa fisica antes de perforar.
scene['NERA_COIL_HOLE_INSET_X'] = 5.0
scene['NERA_COIL_HOLE_INSET_Z'] = 5.0
scene['NERA_COIL_HOLE_D'] = 3.4

# --------------------------
# HARDWARE M3 CONFIRMADO
# --------------------------
scene['NERA_M3_NUT_AF'] = 5.5
scene['NERA_M3_NUT_THICK'] = 2.4
scene['NERA_M3_CLEARANCE_D'] = 3.4
scene['NERA_M3_NUT_POCKET_AF'] = 5.9  # incluye holgura FDM inicial
scene['NERA_M3_NUT_POCKET_DEPTH'] = 2.7
scene['NERA_M3_BOSS_D'] = 10.0
scene['NERA_M3_BOSS_DEPTH'] = 4.0
scene['NERA_M3_SCREW_LENGTH'] = 20.0  # referencia visual del kit disponible
scene['NERA_M3_HEAD_D'] = 5.8
scene['NERA_M3_HEAD_H'] = 2.5

# --------------------------
# SALIDA DE CABLE
# --------------------------
scene['NERA_CABLE_D'] = 7.6
scene['NERA_CABLE_X'] = 18.0
scene['NERA_CABLE_Z'] = -42.0

# --------------------------
# VISTA TECNICA
# --------------------------
scene['NERA_VIEW_OFFSET_X'] = 115.0
# El corte de inspeccion elimina exactamente el 50% FRONTAL de la carcasa en Y.
scene['NERA_CUT_PERCENT'] = 0.50

# --------------------------
# COLECCIONES
# --------------------------
def ensure_collection(name):
    col = bpy.data.collections.get(name)
    if col is None:
        col = bpy.data.collections.new(name)
        scene.collection.children.link(col)
    return col

for name in (
    'NERA_CARCASA',
    'NERA_CORTE',
    'NERA_COILBOARDS',
    'NERA_SOPORTES',
    'NERA_HARDWARE_REF',
    'NERA_ORIFICIOS',
    'NERA_REFERENCIAS',
):
    ensure_collection(name)

# --------------------------
# MATERIALES
# --------------------------
def ensure_material(name, rgba, metallic=0.0, roughness=0.5):
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
    mat.diffuse_color = rgba
    mat.metallic = metallic
    mat.roughness = roughness
    return mat

# Carcasa: gris/crema neutro.
ensure_material('NERA_MAT_CARCASA', (0.68, 0.66, 0.61, 1.0), 0.0, 0.48)
ensure_material('NERA_MAT_CORTE', (0.76, 0.74, 0.69, 1.0), 0.0, 0.45)
# CoilBoards: dos tonos naranjas para distinguir FRONT/BACK sin cambiar su funcion.
ensure_material('NERA_MAT_COILBOARD_FRONT', (0.95, 0.34, 0.06, 1.0), 0.0, 0.42)
ensure_material('NERA_MAT_COILBOARD_BACK', (0.78, 0.22, 0.03, 1.0), 0.0, 0.45)
ensure_material('NERA_MAT_SUPPORT', (0.34, 0.36, 0.39, 1.0), 0.1, 0.42)
ensure_material('NERA_MAT_HARDWARE', (0.56, 0.58, 0.62, 1.0), 0.75, 0.24)

# --------------------------
# VALIDACION BASICA
# --------------------------
IX = scene['NERA_CARCASA_X'] - 2.0 * scene['NERA_PARED']
IY = scene['NERA_CARCASA_Y'] - 2.0 * scene['NERA_PARED']
IZ = scene['NERA_CARCASA_Z'] - 2.0 * scene['NERA_PARED']
scene['NERA_INTERIOR_X'] = IX
scene['NERA_INTERIOR_Y'] = IY
scene['NERA_INTERIOR_Z'] = IZ

if IX <= scene['NERA_COILBOARD_X'] or IZ <= scene['NERA_COILBOARD_Z']:
    raise ValueError('NERA: la CoilBoard no entra en la cavidad rectangular basica.')

# Posiciones Y automaticas: ambas CoilBoards quedan centradas y pegadas.
by = scene['NERA_COILBOARD_Y']
gap = scene['NERA_COIL_GLUE_GAP']
coil_center = gap / 2.0 + by / 2.0
scene['NERA_COIL_FRONT_Y'] = -coil_center
scene['NERA_COIL_BACK_Y'] = +coil_center
# Alias de compatibilidad para scripts/controles anteriores.
scene['NERA_COIL_GAP'] = gap

print('============================================================')
print('NERA MODELO 3 - ETAPA 1 COMPLETA')
print(f'Carcasa: 93 x 35 x 100 mm | interior aprox: {IX:.1f} x {IY:.1f} x {IZ:.1f} mm')
print(f'CoilBoards: 82 x {by:.2f} x 89 mm | espesor Y PROVISIONAL')
print(f'Gap de pegado/visualizacion: {gap:.2f} mm (0 = CoilBoards tocandose)')
print('NO hay separador fisico entre CoilBoards.')
print('M3 activo. M4 NO se modela hasta definir su funcion.')
print('============================================================')
