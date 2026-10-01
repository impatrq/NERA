# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 7 / 8
# TORNILLOS + TUERCAS M3 DE REFERENCIA (NO STL)
# ============================================================
import bpy, math
s=bpy.context.scene

def P(n):return float(s['NERA_'+n])
def move(o,c):
    col=bpy.data.collections[c]
    for x in list(o.users_collection):x.objects.unlink(o)
    col.objects.link(o)
def delete_prefix(p):
    for o in list(bpy.data.objects):
        if o.name.startswith(p):bpy.data.objects.remove(o,do_unlink=True)
def join(obs,name):
    bpy.ops.object.select_all(action='DESELECT')
    for o in obs:o.select_set(True)
    bpy.context.view_layer.objects.active=obs[0];bpy.ops.object.join();obs[0].name=name;return obs[0]
BX,BZ=P('COILBOARD_X'),P('COILBOARD_Z');ix,iz=P('COIL_HOLE_INSET_X'),P('COIL_HOLE_INSET_Z')
hx=BX/2-ix;hz=BZ/2-iz;poss=[(-hx,-hz),(hx,-hz),(-hx,hz),(hx,hz)]
SL=P('M3_SCREW_LENGTH');HD=P('M3_HEAD_D');HH=P('M3_HEAD_H');AF=P('M3_NUT_AF');NT=P('M3_NUT_THICK')
for p in ('TORNILLO_M3_REF_','TUERCA_M3_REF_'):delete_prefix(p)
mat=bpy.data.materials.get('NERA_MAT_HARDWARE')
for i,(x,z) in enumerate(poss,1):
    bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=1.5,depth=SL,location=(x,0,z),rotation=(math.radians(90),0,0));sh=bpy.context.object
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=HD/2,depth=HH,location=(x,-SL/2-HH/2,z),rotation=(math.radians(90),0,0));he=bpy.context.object
    sc=join([sh,he],f'TORNILLO_M3_REF_{i:02d}');move(sc,'NERA_HARDWARE_REF');sc['NERA_PRINTABLE']=False
    nr=AF/math.sqrt(3);ny=SL/2-NT/2
    bpy.ops.mesh.primitive_cylinder_add(vertices=6,radius=nr,depth=NT,location=(x,ny,z),rotation=(math.radians(90),0,0));nu=bpy.context.object;nu.name=f'TUERCA_M3_REF_{i:02d}';move(nu,'NERA_HARDWARE_REF');nu['NERA_PRINTABLE']=False
    if mat:
        sc.data.materials.append(mat);nu.data.materials.append(mat)
print('ETAPA 7: hardware M3 visual creado.')
