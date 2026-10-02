# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 2 / 8
# CARCASA COMPLETA DE REFERENCIA
# Blender 5.2
# ============================================================
# Crea la carcasa cerrada solo como referencia visual a la izquierda.
# NO es la pieza que se exporta a STL.
# ============================================================
import bpy
scene=bpy.context.scene

def P(n):
    k='NERA_'+n
    if k not in scene: raise RuntimeError('Ejecutar 01_escena_parametros.py')
    return float(scene[k])
def activate(o):
    bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o
def move(o,c):
    col=bpy.data.collections[c]
    for x in list(o.users_collection): x.objects.unlink(o)
    col.objects.link(o)
def delete(name):
    o=bpy.data.objects.get(name)
    if o: bpy.data.objects.remove(o,do_unlink=True)
def rounded_box(name,sx,sy,sz,r,loc):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc)
    o=bpy.context.object; o.name=name; o.dimensions=(sx,sy,sz)
    activate(o); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    b=o.modifiers.new('Redondeo','BEVEL'); b.width=r; b.segments=10; b.limit_method='ANGLE'
    activate(o); bpy.ops.object.modifier_apply(modifier=b.name)
    return o
def diff(a,b,name):
    m=a.modifiers.new(name,'BOOLEAN');m.operation='DIFFERENCE';m.solver='EXACT';m.object=b
    activate(a);bpy.ops.object.modifier_apply(modifier=m.name);bpy.data.objects.remove(b,do_unlink=True)

W,D,H=P('CARCASA_X'),P('CARCASA_Y'),P('CARCASA_Z'); wall=P('PARED')
ro,ri=P('RADIO_EXT'),P('RADIO_INT'); x0=P('REFERENCE_OFFSET_X')
delete('CARCASA_COMPLETA_REF')
outer=rounded_box('CARCASA_COMPLETA_REF',W,D,H,ro,(x0,0,0))
inner=rounded_box('TMP_INNER',W-2*wall,D-2*wall,H-2*wall,ri,(x0,0,0))
diff(outer,inner,'Hueco')
move(outer,'NERA_REFERENCIA')
mat=bpy.data.materials.get('NERA_MAT_CARCASA')
if mat: outer.data.materials.append(mat)
outer['NERA_PRINTABLE']=False;outer['NERA_REFERENCE_ONLY']=True
print('ETAPA 2: carcasa completa de referencia creada.')
