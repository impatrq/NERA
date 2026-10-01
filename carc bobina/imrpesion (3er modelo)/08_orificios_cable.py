# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 8 / 8
# SALIDA DE CABLE + VALIDACION + EXPORTACION STL
# Blender 5.2
# ============================================================
import bpy, math, os
s=bpy.context.scene

def P(n):return float(s['NERA_'+n])
def act(o):
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
def diff(a,b,name):
    m=a.modifiers.new(name,'BOOLEAN');m.operation='DIFFERENCE';m.solver='EXACT';m.object=b;act(a);bpy.ops.object.modifier_apply(modifier=m.name);bpy.data.objects.remove(b,do_unlink=True)
front=bpy.data.objects.get('CARCASA_FRONT');back=bpy.data.objects.get('CARCASA_BACK')
if not front or not back:raise RuntimeError('Faltan CARCASA_FRONT/BACK')
D=P('CARCASA_Y');cd=P('CABLE_D');cx=P('CABLE_X');cz=P('CABLE_Z')
# El agujero atraviesa ambas mitades para que al cerrar forme una salida alineada.
for obj,label in ((front,'FRONT'),(back,'BACK')):
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=cd/2,depth=D+4,location=(cx,0,cz),rotation=(math.radians(90),0,0));c=bpy.context.object
    diff(obj,c,f'Salida_Cable_{label}')

# Validaciones basicas
W,H=P('CARCASA_X'),P('CARCASA_Z');wall=P('PARED');R=P('RADIO_INT');BX,BZ=P('COILBOARD_X'),P('COILBOARD_Z')
IX=W-2*wall;IZ=H-2*wall
straight_x=(IX-BX)/2;straight_z=(IZ-BZ)/2
arc_cx=IX/2-R;arc_cz=IZ/2-R;dx=max(0,BX/2-arc_cx);dz=max(0,BZ/2-arc_cz);corner=R-math.hypot(dx,dz)
print('============================================================')
print('NERA - VERSION DE FABRICACION')
print(f'Holgura X: {straight_x:.2f} mm/lado | Z: {straight_z:.2f} mm/lado | esquina: {corner:.2f} mm')
print(f'Encastre: profundidad {P("LIP_DEPTH"):.2f} mm | clearance {P("LIP_CLEARANCE"):.2f} mm')
print('IMPORTANTE: verificar fisicamente la zona libre de esquina antes de perforar las CoilBoards.')

# Exportar FRONT/BACK a STL si Blender dispone del operador moderno.
base=os.path.dirname(bpy.data.filepath) if bpy.data.filepath else os.path.expanduser('~')
out=os.path.join(base,'NERA_STL_EXPORT');os.makedirs(out,exist_ok=True)

def export_one(obj,filename):
    bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);bpy.context.view_layer.objects.active=obj
    path=os.path.join(out,filename)
    try:
        bpy.ops.wm.stl_export(filepath=path,export_selected_objects=True)
    except Exception:
        try:
            bpy.ops.export_mesh.stl(filepath=path,use_selection=True)
        except Exception as e:
            print('No se pudo exportar STL automaticamente:',e);return
    print('STL:',path)
export_one(front,'NERA_Carcasa_FRONT.stl')
export_one(back,'NERA_Carcasa_BACK.stl')
print('============================================================')
