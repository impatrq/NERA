# ETAPA 3 - CUTAWAY / CORTE TECNICO DE LA CARCASA
# Crea una SEGUNDA carcasa completa y retira SOLO una ventana frontal/esquina.
# No corta placas, bobinas, separador ni postes creados en etapas posteriores.
import bpy
W,H,D=73.0,78.0,14.0; IW,IH=67.0,72.0; R=6.0; FLOOR=2.0; X=55.0

def active(o):
    bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o

def move(o,cname):
    c=bpy.data.collections[cname]
    for x in list(o.users_collection): x.objects.unlink(o)
    c.objects.link(o)

def box(name,sx,sy,sz,r,loc):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc); o=bpy.context.object; o.name=name; o.dimensions=(sx,sy,sz)
    active(o); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    b=o.modifiers.new('Bevel','BEVEL'); b.width=r; b.segments=8; b.limit_method='ANGLE'; active(o); bpy.ops.object.modifier_apply(modifier=b.name); return o

def boolean(t,c,op):
    m=t.modifiers.new('Boolean','BOOLEAN'); m.operation=op; m.solver='EXACT'; m.object=c; active(t); bpy.ops.object.modifier_apply(modifier=m.name); bpy.data.objects.remove(c,do_unlink=True)

shell=box('Carcasa_Corte_Tecnico',W,D,H,R,(X,0,0))
inner=box('Cavidad_Cutaway',IW,D-FLOOR+2.0,IH,max(1,R-3),(X,-FLOOR/2-1.0,0)); boolean(shell,inner,'DIFFERENCE')
# Ventana de observacion: quita parte de la pared frontal (-Y), pero NO media carcasa.
# Deja marco superior, inferior y lateral derecho visibles.
bpy.ops.mesh.primitive_cube_add(size=1,location=(X-7.0,-D/2.0,0))
cut=bpy.context.object; cut.name='Cortador_Ventana_Cutaway'; cut.dimensions=(W-18.0,8.0,H-18.0)
active(cut); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
boolean(shell,cut,'DIFFERENCE')
move(shell,'02_CORTE_ESTRUCTURAL'); shell.data.materials.append(bpy.data.materials['MAT_Carcasa'])
print('ETAPA 3 COMPLETA: cutaway parcial; la carcasa conserva su contorno completo.')
