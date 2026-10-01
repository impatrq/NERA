# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 3 / 8
# CORTE DE INSPECCION 50% - MEDIA CARCASA ABIERTA
# Blender 5.2
# ============================================================
# OBJETIVO:
# - CARCASA_COMPLETA queda intacta.
# - Se crea CARCASA_CORTE_VERTICAL como una copia independiente.
# - Se elimina exactamente el 50% FRONTAL de la carcasa en el eje Y.
# - Queda visible la mitad trasera como una bandeja abierta, tal como
#   la referencia visual indicada por el usuario.
# - SOLO se corta la carcasa. CoilBoards, bosses, tornillos y tuercas
#   permanecen completos.
# ============================================================

import bpy

scene = bpy.context.scene


def P(name):
    k = 'NERA_' + name
    if k not in scene:
        raise RuntimeError('Ejecutar primero 01_escena_parametros.py')
    return float(scene[k])


def activate(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def delete_named(name):
    obj = bpy.data.objects.get(name)
    if obj:
        bpy.data.objects.remove(obj, do_unlink=True)

W, D, H = P('CARCASA_X'), P('CARCASA_Y'), P('CARCASA_Z')
OFF = P('VIEW_OFFSET_X')
CUT_PERCENT = P('CUT_PERCENT')
X0 = +OFF

if not (0.0 < CUT_PERCENT < 1.0):
    raise ValueError('NERA_CUT_PERCENT debe estar entre 0 y 1.')

src = bpy.data.objects.get('CARCASA_COMPLETA')
if src is None:
    raise RuntimeError('Ejecutar primero 02_carcasa_completa.py')

delete_named('CARCASA_CORTE_VERTICAL')
cut = src.copy()
cut.data = src.data.copy()
cut.name = 'CARCASA_CORTE_VERTICAL'
bpy.data.collections['NERA_CORTE'].objects.link(cut)
cut.location.x = X0

# Se retira la porcion FRONTAL (-Y). Para 50%, el plano de corte queda en Y=0.
# Se agrega un pequeño exceso en X/Z/Y para garantizar un booleano limpio.
eps = 1.0
cut_depth = D * CUT_PERCENT
front_y = -D / 2.0
cutter_center_y = front_y + cut_depth / 2.0

bpy.ops.mesh.primitive_cube_add(size=1.0, location=(X0, cutter_center_y, 0.0))
cutter = bpy.context.object
cutter.name = 'TMP_CORTE_FRONTAL_50'
cutter.dimensions = (W + 2.0 * eps, cut_depth + eps, H + 2.0 * eps)
activate(cutter)
bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

mod = cut.modifiers.new('Corte_Frontal_50', 'BOOLEAN')
mod.operation = 'DIFFERENCE'
mod.solver = 'EXACT'
mod.object = cutter
activate(cut)
bpy.ops.object.modifier_apply(modifier=mod.name)
bpy.data.objects.remove(cutter, do_unlink=True)

mat = bpy.data.materials.get('NERA_MAT_CORTE')
if mat:
    cut.data.materials.clear()
    cut.data.materials.append(mat)

cut['NERA_PRINTABLE'] = False
cut['NERA_REFERENCE_ONLY'] = True
cut['NERA_CUT_PERCENT'] = CUT_PERCENT
cut['NERA_DESCRIPTION'] = 'Media carcasa abierta para inspeccion: 50% frontal eliminado en Y.'

print('ETAPA 3: CARCASA_CORTE_VERTICAL creada.')
print(f'Se elimino el {CUT_PERCENT*100:.0f}% frontal de la carcasa en Y.')
print('CARCASA_COMPLETA permanece intacta; los componentes internos no se cortan.')
