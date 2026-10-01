# ETAPA 5 - DOS PCB RIGIDOS CON BOBINAS PANCAKE
# Las placas son 65 x 70 x 1.6 mm y quedan verticales en X-Z.
import bpy, math
X=55.0; BW,BH,BT=65.0,70.0,1.6; YB=1.65; YF=-1.65

def active(o): bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o

def move(o):
 c=bpy.data.collections['04_BOBINAS']; [x.objects.unlink(o) for x in list(o.users_collection)]; c.objects.link(o)

def board(name,y):
 bpy.ops.mesh.primitive_cube_add(size=1,location=(X,y,0)); o=bpy.context.object; o.name=name; o.dimensions=(BW,BT,BH); active(o); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
 b=o.modifiers.new('Bevel','BEVEL'); b.width=1.2; b.segments=4; active(o); bpy.ops.object.modifier_apply(modifier=b.name); move(o); o.data.materials.append(bpy.data.materials['MAT_PCB']); return o

def coil(name,y,reverse=False):
 turns=12; seg=42; ri,ro,wire=6.0,25.0,1.0; n=turns*seg+1
 cu=bpy.data.curves.new(name+'_Curve','CURVE'); cu.dimensions='3D'; cu.bevel_depth=wire/2; cu.bevel_resolution=3
 sp=cu.splines.new('POLY'); sp.points.add(n-1)
 for i in range(n):
  t=i/(n-1); a=2*math.pi*turns*t*(-1 if reverse else 1); r=ri+(ro-ri)*t
  sp.points[i].co=(X+r*math.cos(a),y,r*math.sin(a),1)
 o=bpy.data.objects.new(name,cu); bpy.data.collections['04_BOBINAS'].objects.link(o); cu.materials.append(bpy.data.materials['MAT_Cobre'])
board('PCB_Bobina_Trasera',YB); board('PCB_Bobina_Delantera',YF)
# cobre orientado hacia afuera de cada PCB
coil('Bobina_Pancake_Trasera',YB+BT/2+0.55,True); coil('Bobina_Pancake_Delantera',YF-BT/2-0.55,False)
print('ETAPA 5 COMPLETA: dos placas de bobina completas, BACK y FRONT.')
