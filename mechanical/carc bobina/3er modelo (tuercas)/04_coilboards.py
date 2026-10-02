# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 4 / 8
# COILBOARDS FRONT / BACK
# Blender 5.2
# ============================================================
# Cada objeto representa LA PLACA FISICA COMPLETA QUE YA CONTIENE LA BOBINA.
# Dimensiones confirmadas: 82 x BOBINA_Y x 89 mm.
# Se visualizan en naranja y NO son imprimibles.
# Se agregan 4 agujeros M3 de referencia en las esquinas libres de la placa.
# La posicion exacta sigue siendo PARAMETRICA / AJUSTABLE y debe verificarse con la placa fisica.
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

def delete_named(name):
    o=bpy.data.objects.get(name)
    if o: bpy.data.objects.remove(o,do_unlink=True)

def bool_diff(target,cutter,name):
    m=target.modifiers.new(name,'BOOLEAN'); m.operation='DIFFERENCE'; m.solver='EXACT'; m.object=cutter
    activate(target); bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.data.objects.remove(cutter,do_unlink=True)

BX,BY,BZ=P('COILBOARD_X'),P('COILBOARD_Y'),P('COILBOARD_Z')
YF,YB=P('COIL_FRONT_Y'),P('COIL_BACK_Y')
IX,IZ=P('COIL_HOLE_INSET_X'),P('COIL_HOLE_INSET_Z')
HD=P('COIL_HOLE_D')
XOFF=P('VIEW_OFFSET_X')
HX=BX/2-IX; HZ=BZ/2-IZ
positions=[(-HX,-HZ),(HX,-HZ),(-HX,HZ),(HX,HZ)]

def make_board(name,y,matname):
    delete_named(name)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(XOFF,y,0))
    o=bpy.context.object; o.name=name; o.dimensions=(BX,BY,BZ)
    activate(o); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    # Redondeo minimo solo para evitar aristas visuales filosas; no representa radio real de PCB.
    b=o.modifiers.new('Borde_Suave','BEVEL'); b.width=min(0.5,BY*0.35); b.segments=3; b.limit_method='ANGLE'
    activate(o); bpy.ops.object.modifier_apply(modifier=b.name)
    for i,(x,z) in enumerate(positions,1):
        bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=HD/2,depth=BY+4,
            location=(XOFF+x,y,z),rotation=(math.radians(90),0,0))
        bool_diff(o,bpy.context.object,f'Agujero_M3_Esquina_{i}')
    move(o,'NERA_COILBOARDS')
    mat=bpy.data.materials.get(matname)
    if mat: o.data.materials.append(mat)
    o['NERA_PRINTABLE']=False; o['NERA_REFERENCE_ONLY']=True
    return o

make_board('COILBOARD_FRONT',YF,'NERA_MAT_COILBOARD_FRONT')
make_board('COILBOARD_BACK',YB,'NERA_MAT_COILBOARD_BACK')
print(f'ETAPA 4: CoilBoards 82 x {BY:.2f} x 89 mm creadas; 4 agujeros M3 de esquina parametrizados.')
