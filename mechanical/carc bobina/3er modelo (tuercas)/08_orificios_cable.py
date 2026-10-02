# ============================================================
# N.E.R.A. - MODELO 3 - ETAPA 8 / 8
# SALIDA DE CABLE + REPORTE DE INTERFERENCIAS
# Blender 5.2
# ============================================================

import bpy, math
scene=bpy.context.scene

def P(n):
    k='NERA_'+n
    if k not in scene: raise RuntimeError('Ejecutar primero 01_escena_parametros.py')
    return float(scene[k])

def activate(o):
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o

def bool_diff(target,cutter,name):
    m=target.modifiers.new(name,'BOOLEAN');m.operation='DIFFERENCE';m.solver='EXACT';m.object=cutter
    activate(target);bpy.ops.object.modifier_apply(modifier=m.name);bpy.data.objects.remove(cutter,do_unlink=True)

W,D,H=P('CARCASA_X'),P('CARCASA_Y'),P('CARCASA_Z')
WALL=P('PARED'); R=P('RADIO_INT')
BX,BY,BZ=P('COILBOARD_X'),P('COILBOARD_Y'),P('COILBOARD_Z')
OFF=P('VIEW_OFFSET_X')
CD=P('CABLE_D'); CX=P('CABLE_X'); CZ=P('CABLE_Z')
IX=W-2*WALL; IY=D-2*WALL; IZ=H-2*WALL

# Salida de cable: agujero a traves de Y.
def cable_cut(name,xcenter):
    shell=bpy.data.objects.get(name)
    if shell is None: raise RuntimeError(f'Falta {name}')
    bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=CD/2,depth=D+4,
        location=(xcenter+CX,0,CZ),rotation=(math.radians(90),0,0))
    bool_diff(shell,bpy.context.object,'Salida_Cable')

cable_cut('CARCASA_COMPLETA',-OFF)
cable_cut('CARCASA_CORTE_VERTICAL',+OFF)

# Verificacion de bobina rectangular contra rounded-rectangle interior.
straight_x=(IX-BX)/2
straight_z=(IZ-BZ)/2
arc_cx=IX/2-R; arc_cz=IZ/2-R
coil_cx=BX/2; coil_cz=BZ/2
dx=max(0.0,coil_cx-arc_cx); dz=max(0.0,coil_cz-arc_cz)
corner_margin=R-math.hypot(dx,dz)

# Agujeros de esquina: deben permanecer dentro de la CoilBoard.
inset_x=P('COIL_HOLE_INSET_X'); inset_z=P('COIL_HOLE_INSET_Z'); hole_r=P('COIL_HOLE_D')/2
hole_edge_margin=min(inset_x-hole_r,inset_z-hole_r)

# Stack Y
front_y=P('COIL_FRONT_Y'); back_y=P('COIL_BACK_Y'); gap=P('COIL_GAP')
stack_outer=max(abs(front_y-BY/2),abs(back_y+BY/2))
wall_y=IY/2
wall_gap=wall_y-stack_outer

print('============================================================')
print('NERA MODELO 3 - REPORTE FINAL')
print('============================================================')
print(f'Carcasa exterior: {W:.1f} x {D:.1f} x {H:.1f} mm')
print(f'Interior: {IX:.1f} x {IY:.1f} x {IZ:.1f} mm')
print(f'CoilBoards: {BX:.1f} x {BY:.2f} x {BZ:.1f} mm')
print(f'Holgura recta X: {straight_x:.2f} mm/lado')
print(f'Holgura recta Z: {straight_z:.2f} mm/lado')
if corner_margin>=0:
    print(f'OK esquinas redondeadas: margen geometrico aprox. {corner_margin:.2f} mm')
else:
    print(f'ERROR: CoilBoard invade esquinas redondeadas por {-corner_margin:.2f} mm')
if hole_edge_margin>0:
    print(f'OK agujeros M3 de esquina: margen minimo al borde de placa {hole_edge_margin:.2f} mm')
else:
    print('ERROR: agujero de esquina sale de la placa; aumentar inset.')
print(f'Gap entre CoilBoards: {gap:.2f} mm [0 = pegado directo / contacto]')
print(f'Margen Y entre stack de CoilBoards y pared interior: {wall_gap:.2f} mm por lado aprox.')
print('ADVERTENCIA: no se conoce todavia el diametro/huella exacta de la zona activa de la bobina.')
print('Por eso la posicion definitiva de los agujeros de esquina debe corroborarse con la placa fisica.')
print('NO hay separador fisico: las CoilBoards se pegan entre si.')
print('M4 NO se genero: su funcion sigue pendiente.')
print('============================================================')
