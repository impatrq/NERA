# ETAPA 6 - SEPARADOR DIELECTRICO + 4 POSTES M3 DE LA CARCASA CUTAWAY
import bpy, math
X=55.0; SW,SH,ST=66.5,71.5,1.0; HX,HZ=30.5,33.0

def active(o): bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o

def move(o):
 c=bpy.data.collections['05_SEPARADOR_SOPORTES']; [x.objects.unlink(o) for x in list(o.users_collection)]; c.objects.link(o)

def diff(t,c):
 m=t.modifiers.new('Boolean','BOOLEAN'); m.operation='DIFFERENCE'; m.solver='EXACT'; m.object=c; active(t); bpy.ops.object.modifier_apply(modifier=m.name); bpy.data.objects.remove(c,do_unlink=True)
# Separador plano vertical
bpy.ops.mesh.primitive_cube_add(size=1,location=(X,0,0)); s=bpy.context.object; s.name='Separador_Central'; s.dimensions=(SW,ST,SH); active(s); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=3.0,depth=ST+4,location=(X,0,0),rotation=(math.pi/2,0,0)); diff(s,bpy.context.object); move(s); s.data.materials.append(bpy.data.materials['MAT_Separador'])
# Postes: Ø7, eje Y, con alojamiento ciego Ø4.2 x 5 mm desde el frente.
for i,(dx,dz) in enumerate([(-HX,-HZ),(HX,-HZ),(-HX,HZ),(HX,HZ)],1):
 bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=3.5,depth=10.0,location=(X+dx,2.0,dz),rotation=(math.pi/2,0,0)); p=bpy.context.object; p.name=f'Poste_M3_{i}'
 # agujero ciego: entra 5 mm desde cara frontal del poste
 bpy.ops.mesh.primitive_cylinder_add(vertices=48,radius=2.1,depth=5.0,location=(X+dx,-0.5,dz),rotation=(math.pi/2,0,0)); diff(p,bpy.context.object); move(p); p.data.materials.append(bpy.data.materials['MAT_Soportes'])
print('ETAPA 6 COMPLETA: separador y postes M3 completos.')
