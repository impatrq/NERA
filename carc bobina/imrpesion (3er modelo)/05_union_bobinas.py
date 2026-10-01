# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 5 / 8
# ALINEACION DE COILBOARDS PEGADAS - SIN SEPARADOR
# ============================================================
import bpy
s=bpy.context.scene

def P(n):return float(s['NERA_'+n])
a=bpy.data.objects.get('COILBOARD_FRONT');b=bpy.data.objects.get('COILBOARD_BACK')
if not a or not b:raise RuntimeError('Ejecutar 04_coilboards.py')
by=P('COILBOARD_Y');gap=P('COIL_GLUE_GAP');c=gap/2+by/2
a.location=(0,-c,0);b.location=(0,+c,0)
for o in (a,b):o['NERA_PRINTABLE']=False;o['NERA_JOIN_METHOD']='ADHESIVO_GOTA'
print('ETAPA 5: CoilBoards centradas y pegadas (gap parametrico).')
