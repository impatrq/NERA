# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 2 / 8
# CARCASA COMPLETA, CERRADA Y HUECA
# Blender 5.2
# ============================================================

import bpy

scene = bpy.context.scene

def P(name):
    k='NERA_'+name
    if k not in scene:
        raise RuntimeError('Ejecutar primero 01_escena_parametros.py')
    return float(scene[k])

def activate(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active=obj

def move(obj, colname):
    col=bpy.data.collections.get(colname)
    for c in list(obj.users_collection): c.objects.unlink(obj)
    col.objects.link(obj)

def delete_named(name):
    o=bpy.data.objects.get(name)
    if o: bpy.data.objects.remove(o,do_unlink=True)

def rounded_box(name, dims, radius, loc):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc)
    o=bpy.context.object
    o.name=name
    o.dimensions=dims
    activate(o)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    b=o.modifiers.new('Redondeo','BEVEL')
    b.width=radius
    b.segments=12
    b.limit_method='ANGLE'
    activate(o)
    bpy.ops.object.modifier_apply(modifier=b.name)
    return o

def boolean_diff(target, cutter, name):
    m=target.modifiers.new(name,'BOOLEAN')
    m.operation='DIFFERENCE'
    m.solver='EXACT'
    m.object=cutter
    activate(target)
    bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.data.objects.remove(cutter,do_unlink=True)

W,D,H=P('CARCASA_X'),P('CARCASA_Y'),P('CARCASA_Z')
WALL=P('PARED')
RO,RI=P('RADIO_EXT'),P('RADIO_INT')
OFF=P('VIEW_OFFSET_X')
X0=-OFF

delete_named('CARCASA_COMPLETA')
outer=rounded_box('CARCASA_COMPLETA',(W,D,H),RO,(X0,0,0))
inner=rounded_box('TMP_CAVIDAD',(W-2*WALL,D-2*WALL,H-2*WALL),RI,(X0,0,0))
boolean_diff(outer,inner,'Vaciado_Interior')
move(outer,'NERA_CARCASA')
mat=bpy.data.materials.get('NERA_MAT_CARCASA')
if mat: outer.data.materials.append(mat)
outer['NERA_PRINTABLE']=True
outer['NERA_REFERENCE_ONLY']=False

print('ETAPA 2: CARCASA_COMPLETA creada. 93 x 35 x 100 mm, hueca y cerrada.')
