# ETAPA 4 - PLACAS DE SOPORTE FRONTAL Y TRASERA
import bpy, math
X=55.0; PW,PH,PT=66.5,71.5,1.2; HX,HZ=30.5,33.0; HR=3.3/2
# Eje Y = profundidad. Ensamble parado en X-Z.
Y_BACK=3.7; Y_FRONT=-3.7

def active(o): bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o

def move(o):
 c=bpy.data.collections['03_PLACAS']; [x.objects.unlink(o) for x in list(o.users_collection)]; c.objects.link(o)

def hole(plate,x,y,z,r,depth):
 bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=r,depth=depth,location=(x,y,z),rotation=(math.pi/2,0,0)); c=bpy.context.object
 m=plate.modifiers.new('Agujero','BOOLEAN'); m.operation='DIFFERENCE'; m.solver='EXACT'; m.object=c; active(plate); bpy.ops.object.modifier_apply(modifier=m.name); bpy.data.objects.remove(c,do_unlink=True)

def plate(name,y,countersink=False):
 bpy.ops.mesh.primitive_cube_add(size=1,location=(X,y,0)); p=bpy.context.object; p.name=name; p.dimensions=(PW,PT,PH); active(p); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
 b=p.modifiers.new('Bevel','BEVEL'); b.width=1.5; b.segments=5; active(p); bpy.ops.object.modifier_apply(modifier=b.name)
 for dx in (-HX,HX):
  for dz in (-HZ,HZ):
   hole(p,X+dx,y,dz,HR,PT+4)
   if countersink: hole(p,X+dx,y-PT/2+0.25,dz,3.1,0.6)
 move(p); p.data.materials.append(bpy.data.materials['MAT_Placas']); return p
plate('Placa_Soporte_Trasera',Y_BACK,False); plate('Placa_Soporte_Delantera',Y_FRONT,True)
print('ETAPA 4 COMPLETA: soportes 66.5 x 71.5 mm completos.')
