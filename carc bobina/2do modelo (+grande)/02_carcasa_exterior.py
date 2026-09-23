# ETAPA 2 - CARCASA EXTERIOR COMPLETA Y CERRADA
# Vista tecnica izquierda. No realiza ningun corte de seccion.
import bpy, math
W,H,D=73.0,78.0,14.0; IW,IH=67.0,72.0; R=6.0; FLOOR=2.0; X=-55.0

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

def diff(t,c):
    m=t.modifiers.new('Boolean','BOOLEAN'); m.operation='DIFFERENCE'; m.solver='EXACT'; m.object=c; active(t); bpy.ops.object.modifier_apply(modifier=m.name); bpy.data.objects.remove(c,do_unlink=True)

# Cuerpo tipo bandeja: exterior completo, cavidad abierta por el frente (-Y).
body=box('Carcasa_Exterior_Completa',W,D,H,R,(X,0,0))
# Cavidad exacta 67x72; se desplaza hacia el frente para dejar fondo de 2 mm.
inner_depth=D-FLOOR+2.0
inner=box('Cavidad_Exterior',IW,inner_depth,IH,max(1,R-3),(X,-FLOOR/2-1.0,0))
diff(body,inner)
move(body,'01_CARCASA_EXTERIOR'); body.data.materials.append(bpy.data.materials['MAT_Carcasa'])
print('ETAPA 2 COMPLETA: carcasa exterior ENTERA, sin corte tecnico.')
