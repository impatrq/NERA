# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 4 / 8
# COILBOARDS FRONT / BACK - REFERENCIA NO IMPRIMIBLE
# ============================================================
import bpy, math
scene=bpy.context.scene

def P(n):
    k='NERA_'+n
    if k not in scene:raise RuntimeError('Ejecutar 01_escena_parametros.py')
    return float(scene[k])
def activate(o):
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
def move(o,c):
    col=bpy.data.collections[c]
    for x in list(o.users_collection):x.objects.unlink(o)
    col.objects.link(o)
def delete(name):
    o=bpy.data.objects.get(name)
    if o:bpy.data.objects.remove(o,do_unlink=True)
def diff(a,b,name):
    m=a.modifiers.new(name,'BOOLEAN');m.operation='DIFFERENCE';m.solver='EXACT';m.object=b
    activate(a);bpy.ops.object.modifier_apply(modifier=m.name);bpy.data.objects.remove(b,do_unlink=True)

BX,BY,BZ=P('COILBOARD_X'),P('COILBOARD_Y'),P('COILBOARD_Z')
YF,YB=P('COIL_FRONT_Y'),P('COIL_BACK_Y');ix,iz=P('COIL_HOLE_INSET_X'),P('COIL_HOLE_INSET_Z');hd=P('COIL_HOLE_D')
hx=BX/2-ix;hz=BZ/2-iz;poss=[(-hx,-hz),(hx,-hz),(-hx,hz),(hx,hz)]

def make(name,y,matname):
    delete(name);bpy.ops.mesh.primitive_cube_add(size=1,location=(0,y,0));o=bpy.context.object;o.name=name;o.dimensions=(BX,BY,BZ)
    activate(o);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    for i,(x,z) in enumerate(poss,1):
        bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=hd/2,depth=BY+4,location=(x,y,z),rotation=(math.radians(90),0,0))
        diff(o,bpy.context.object,f'Agujero_M3_{i}')
    move(o,'NERA_COILBOARDS');mat=bpy.data.materials.get(matname)
    if mat:o.data.materials.append(mat)
    o['NERA_PRINTABLE']=False;o['NERA_REFERENCE_ONLY']=True
    return o
make('COILBOARD_FRONT',YF,'NERA_MAT_COILBOARD_FRONT');make('COILBOARD_BACK',YB,'NERA_MAT_COILBOARD_BACK')
print('ETAPA 4: CoilBoards 82x89 creadas como referencias.')
