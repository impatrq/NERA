# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 7 / 8
# TORNILLOS + TUERCAS M3 DE REFERENCIA VISUAL
# Blender 5.2
# ============================================================
# NO se imprimen. Solo sirven para visualizar montaje y espacio.
# ============================================================

import bpy, math
scene=bpy.context.scene

def P(n):
    k='NERA_'+n
    if k not in scene: raise RuntimeError('Ejecutar primero 01_escena_parametros.py')
    return float(scene[k])

def move(o,cname):
    col=bpy.data.collections[cname]
    for c in list(o.users_collection): c.objects.unlink(o)
    col.objects.link(o)

def delete_prefix(prefix):
    for o in list(bpy.data.objects):
        if o.name.startswith(prefix): bpy.data.objects.remove(o,do_unlink=True)

def join(obs,name):
    bpy.ops.object.select_all(action='DESELECT')
    for o in obs:o.select_set(True)
    bpy.context.view_layer.objects.active=obs[0]
    bpy.ops.object.join(); obs[0].name=name; return obs[0]

def bool_diff(target,cutter,name):
    bpy.ops.object.select_all(action='DESELECT'); target.select_set(True); bpy.context.view_layer.objects.active=target
    m=target.modifiers.new(name,'BOOLEAN');m.operation='DIFFERENCE';m.solver='EXACT';m.object=cutter
    bpy.ops.object.modifier_apply(modifier=m.name); bpy.data.objects.remove(cutter,do_unlink=True)

BX,BZ=P('COILBOARD_X'),P('COILBOARD_Z')
IX,IZ=P('COIL_HOLE_INSET_X'),P('COIL_HOLE_INSET_Z')
XOFF=P('VIEW_OFFSET_X')
SL=P('M3_SCREW_LENGTH'); SD=3.0; HD=P('M3_HEAD_D'); HH=P('M3_HEAD_H')
NAF=P('M3_NUT_AF'); NT=P('M3_NUT_THICK')
YF=P('COIL_FRONT_Y'); BY=P('COILBOARD_Y'); DEP=P('M3_BOSS_DEPTH')
HX=BX/2-IX; HZ=BZ/2-IZ
positions=[(-HX,-HZ),(HX,-HZ),(-HX,HZ),(HX,HZ)]

for p in ('TORNILLO_M3_REF_','TUERCA_M3_REF_'): delete_prefix(p)
mat=bpy.data.materials.get('NERA_MAT_HARDWARE')

# Tornillo se centra sobre el stack; la longitud es la del kit elegida como referencia.
for i,(x,z) in enumerate(positions,1):
    bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=SD/2,depth=SL,
        location=(XOFF+x,0,z),rotation=(math.radians(90),0,0))
    shaft=bpy.context.object
    # Cabeza Phillips aproximada solo por volumen (sin ranura).
    head_y=-SL/2-HH/2
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=HD/2,depth=HH,
        location=(XOFF+x,head_y,z),rotation=(math.radians(90),0,0))
    head=bpy.context.object
    screw=join([shaft,head],f'TORNILLO_M3_REF_{i:02d}')
    move(screw,'NERA_HARDWARE_REF')
    if mat:screw.data.materials.append(mat)
    screw['NERA_PRINTABLE']=False; screw['NERA_REFERENCE_ONLY']=True

    # Tuerca hexagonal de referencia del lado BACK.
    nr=NAF/math.sqrt(3.0)
    nut_y=+SL/2-NT/2
    bpy.ops.mesh.primitive_cylinder_add(vertices=6,radius=nr,depth=NT,
        location=(XOFF+x,nut_y,z),rotation=(math.radians(90),0,0))
    nut=bpy.context.object; nut.name=f'TUERCA_M3_REF_{i:02d}'
    bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=1.5,depth=NT+1,
        location=(XOFF+x,nut_y,z),rotation=(math.radians(90),0,0))
    bool_diff(nut,bpy.context.object,'Agujero_Referencia')
    move(nut,'NERA_HARDWARE_REF')
    if mat:nut.data.materials.append(mat)
    nut['NERA_PRINTABLE']=False; nut['NERA_REFERENCE_ONLY']=True

print(f'ETAPA 7: tornillos M3 x {SL:.0f} y tuercas M3 creados SOLO como referencia visual.')
