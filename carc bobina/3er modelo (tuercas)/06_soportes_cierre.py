# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 6 / 8
# BOSSES / SOPORTES M3 DE REFERENCIA MECANICA
# Blender 5.2
# ============================================================
# Usa los mismos 4 ejes de las esquinas libres del CoilBoard.
# Lado FRONT: boss con agujero pasante M3.
# Lado BACK: boss con alojamiento hexagonal para tuerca M3 cautiva.
# Son piezas imprimibles separadas para inspeccion y posterior ajuste.
# ============================================================

import bpy, math
scene=bpy.context.scene

def P(n):
    k='NERA_'+n
    if k not in scene: raise RuntimeError('Ejecutar primero 01_escena_parametros.py')
    return float(scene[k])

def activate(o):
    bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o

def move(o,cname):
    col=bpy.data.collections[cname]
    for c in list(o.users_collection): c.objects.unlink(o)
    col.objects.link(o)

def delete_prefix(prefix):
    for o in list(bpy.data.objects):
        if o.name.startswith(prefix): bpy.data.objects.remove(o,do_unlink=True)

def bool_diff(target,cutter,name):
    m=target.modifiers.new(name,'BOOLEAN'); m.operation='DIFFERENCE'; m.solver='EXACT'; m.object=cutter
    activate(target); bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.data.objects.remove(cutter,do_unlink=True)

BX,BZ=P('COILBOARD_X'),P('COILBOARD_Z')
IX,IZ=P('COIL_HOLE_INSET_X'),P('COIL_HOLE_INSET_Z')
XOFF=P('VIEW_OFFSET_X')
YF,YB=P('COIL_FRONT_Y'),P('COIL_BACK_Y')
BY=P('COILBOARD_Y')
BD=P('M3_BOSS_D'); DEP=P('M3_BOSS_DEPTH'); HD=P('M3_CLEARANCE_D')
AF=P('M3_NUT_POCKET_AF'); NDEP=P('M3_NUT_POCKET_DEPTH')
HX=BX/2-IX; HZ=BZ/2-IZ
positions=[(-HX,-HZ),(HX,-HZ),(-HX,HZ),(HX,HZ)]

for pref in ('BOSS_PASANTE_','BOSS_TUERCA_'):
    delete_prefix(pref)
mat=bpy.data.materials.get('NERA_MAT_SUPPORT')

# Bosses se ubican inmediatamente por fuera de cada CoilBoard, dejando el agujero alineado.
y_front=YF-BY/2-DEP/2
y_back=YB+BY/2+DEP/2

for i,(x,z) in enumerate(positions,1):
    # FRONT / pasante
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=BD/2,depth=DEP,
        location=(XOFF+x,y_front,z),rotation=(math.radians(90),0,0))
    a=bpy.context.object; a.name=f'BOSS_PASANTE_{i:02d}'
    bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=HD/2,depth=DEP+2,
        location=(XOFF+x,y_front,z),rotation=(math.radians(90),0,0))
    bool_diff(a,bpy.context.object,'Paso_M3')
    move(a,'NERA_SOPORTES');
    if mat: a.data.materials.append(mat)
    a['NERA_PRINTABLE']=True

    # BACK / tuerca cautiva
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=BD/2,depth=DEP,
        location=(XOFF+x,y_back,z),rotation=(math.radians(90),0,0))
    b=bpy.context.object; b.name=f'BOSS_TUERCA_{i:02d}'
    bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=HD/2,depth=DEP+2,
        location=(XOFF+x,y_back,z),rotation=(math.radians(90),0,0))
    bool_diff(b,bpy.context.object,'Paso_M3')
    # Pocket hexagonal desde la cara exterior de BACK.
    hex_r=AF/math.sqrt(3.0)
    pocket_y=y_back+DEP/2-NDEP/2+0.05
    bpy.ops.mesh.primitive_cylinder_add(vertices=6,radius=hex_r,depth=NDEP+0.1,
        location=(XOFF+x,pocket_y,z),rotation=(math.radians(90),0,0))
    bool_diff(b,bpy.context.object,'Pocket_Tuerca_M3')
    move(b,'NERA_SOPORTES')
    if mat: b.data.materials.append(mat)
    b['NERA_PRINTABLE']=True

print('ETAPA 6: 4 bosses pasantes + 4 bosses con pocket hexagonal M3 creados y alineados con las esquinas.')
