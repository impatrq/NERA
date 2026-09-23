# ETAPA 7 - PASACABLES CONICO A 45 GRADOS
# Se agrega a la carcasa cutaway; orificio interno Ø4.5 mm.
import bpy, math
shell=bpy.data.objects.get('Carcasa_Corte_Tecnico')
if shell is None: raise RuntimeError("Ejecutar primero 03_corte_media_carcasa.py")
X=55.0; W,H,D=73.0,78.0,14.0

def active(o): bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o
# esquina inferior trasera: +Y y -Z, tubo diagonal en plano Y-Z
loc=(X+25.0,D/2-1.0,-H/2+6.0); rot=(math.radians(45),0,0)
bpy.ops.mesh.primitive_cone_add(vertices=64,radius1=4.6,radius2=3.6,depth=12.0,location=loc,rotation=rot); sleeve=bpy.context.object; sleeve.name='Manguito_Pasacables_45deg'
bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=2.25,depth=18.0,location=loc,rotation=rot); cut=bpy.context.object
m=sleeve.modifiers.new('Orificio_4_5mm','BOOLEAN'); m.operation='DIFFERENCE'; m.solver='EXACT'; m.object=cut; active(sleeve); bpy.ops.object.modifier_apply(modifier=m.name); bpy.data.objects.remove(cut,do_unlink=True)
# Mantener manguito como geometria visual independiente para no arriesgar el cutaway.
c=bpy.data.collections['06_CABLEADO']; [x.objects.unlink(sleeve) for x in list(sleeve.users_collection)]; c.objects.link(sleeve); sleeve.data.materials.append(bpy.data.materials['MAT_Carcasa'])
print('ETAPA 7 COMPLETA: manguito conico 45 grados, Ø interno 4.5 mm.')
