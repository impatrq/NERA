# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 6 / 8
# BOSSES M3 INTEGRADOS EN FRONT/BACK
# ============================================================
# Los 4 ejes M3 aprovechan las esquinas libres de la CoilBoard.
# FRONT: boss + agujero pasante.
# BACK: boss + agujero + alojamiento hexagonal para tuerca cautiva.
# ============================================================
import bpy, math
s=bpy.context.scene

def P(n):
    k='NERA_'+n
    if k not in s:raise RuntimeError('Ejecutar 01_escena_parametros.py')
    return float(s[k])
def act(o):
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
def boolean(a,b,op,name):
    m=a.modifiers.new(name,'BOOLEAN');m.operation=op;m.solver='EXACT';m.object=b;act(a);bpy.ops.object.modifier_apply(modifier=m.name);bpy.data.objects.remove(b,do_unlink=True)

front=bpy.data.objects.get('CARCASA_FRONT');back=bpy.data.objects.get('CARCASA_BACK')
if not front or not back:raise RuntimeError('Ejecutar 03_corte_vertical.py')
BX,BZ=P('COILBOARD_X'),P('COILBOARD_Z');ix,iz=P('COIL_HOLE_INSET_X'),P('COIL_HOLE_INSET_Z')
hx=BX/2-ix;hz=BZ/2-iz;poss=[(-hx,-hz),(hx,-hz),(-hx,hz),(hx,hz)]
D=P('M3_BOSS_D');dep=P('M3_BOSS_DEPTH');hole=P('M3_CLEARANCE_D');wall=P('PARED');caseD=P('CARCASA_Y')
# bosses crecen desde cada pared interior hacia el centro.
y_front=-caseD/2+wall+dep/2
y_back=+caseD/2-wall-dep/2
for i,(x,z) in enumerate(poss,1):
    # FRONT boss integrado
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=D/2,depth=dep,location=(x,y_front,z),rotation=(math.radians(90),0,0));boss=bpy.context.object
    boolean(front,boss,'UNION',f'Union_Boss_Front_{i}')
    bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=hole/2,depth=caseD+4,location=(x,0,z),rotation=(math.radians(90),0,0));cut=bpy.context.object
    boolean(front,cut,'DIFFERENCE',f'Paso_M3_Front_{i}')
    # BACK boss integrado
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=D/2,depth=dep,location=(x,y_back,z),rotation=(math.radians(90),0,0));boss=bpy.context.object
    boolean(back,boss,'UNION',f'Union_Boss_Back_{i}')
    bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=hole/2,depth=caseD+4,location=(x,0,z),rotation=(math.radians(90),0,0));cut=bpy.context.object
    boolean(back,cut,'DIFFERENCE',f'Paso_M3_Back_{i}')
    # pocket hexagonal desde la cara interior del BACK
    af=P('M3_NUT_POCKET_AF');pd=P('M3_NUT_POCKET_DEPTH');r=af/math.sqrt(3)
    py=+caseD/2-wall-dep+pd/2+0.05
    bpy.ops.mesh.primitive_cylinder_add(vertices=6,radius=r,depth=pd+0.1,location=(x,py,z),rotation=(math.radians(90),0,0));hexcut=bpy.context.object
    boolean(back,hexcut,'DIFFERENCE',f'Pocket_Tuerca_{i}')
print('ETAPA 6: bosses M3 integrados a ambas mitades.')
