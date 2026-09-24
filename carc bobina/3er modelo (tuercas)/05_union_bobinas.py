# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 5 / 8
# UNION / ALINEACION DE COILBOARDS - SIN SEPARADOR
# Blender 5.2
# ============================================================
# Las CoilBoards FRONT/BACK se pegaran con una gota de adhesivo en el montaje.
# Por eso NO se genera ningun separador fisico ni pieza intermedia.
# Esta etapa solo asegura que ambas referencias queden:
# - centradas en X/Z,
# - paralelas,
# - tocandose en Y cuando NERA_COIL_GLUE_GAP = 0.
# ============================================================

import bpy

scene = bpy.context.scene


def P(name):
    k = 'NERA_' + name
    if k not in scene:
        raise RuntimeError('Ejecutar primero 01_escena_parametros.py')
    return float(scene[k])

front = bpy.data.objects.get('COILBOARD_FRONT')
back = bpy.data.objects.get('COILBOARD_BACK')
if front is None or back is None:
    raise RuntimeError('Ejecutar primero 04_coilboards.py')

BY = P('COILBOARD_Y')
GAP = P('COIL_GLUE_GAP')
XOFF = P('VIEW_OFFSET_X')
center = GAP / 2.0 + BY / 2.0

front.location.x = XOFF
front.location.y = -center
front.location.z = 0.0

back.location.x = XOFF
back.location.y = +center
back.location.z = 0.0

front.rotation_euler = (0.0, 0.0, 0.0)
back.rotation_euler = (0.0, 0.0, 0.0)

front['NERA_JOIN_METHOD'] = 'ADHESIVO_GOTA'
back['NERA_JOIN_METHOD'] = 'ADHESIVO_GOTA'
front['NERA_PRINTABLE'] = False
back['NERA_PRINTABLE'] = False

# Limpieza defensiva de separadores de versiones anteriores, sin tocar otras piezas.
for obj in list(bpy.data.objects):
    if obj.name.startswith('SEPARADOR_'):
        bpy.data.objects.remove(obj, do_unlink=True)

print('ETAPA 5: CoilBoards alineadas para pegado directo.')
print(f'Gap visual/adhesivo: {GAP:.2f} mm. No se genero ningun separador.')
