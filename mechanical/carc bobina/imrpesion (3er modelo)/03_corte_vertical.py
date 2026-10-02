# ============================================================
# N.E.R.A. - VERSION DE FABRICACION - ETAPA 3 / 8
# CARCASA_FRONT + CARCASA_BACK + ENCASTRE MACHO/HEMBRA
# Blender 5.2
# ============================================================
# Divide la carcasa en Y=0 en dos piezas reales imprimibles.
# FRONT = mitad frontal (-Y), con ranura hembra.
# BACK  = mitad trasera (+Y), con labio macho.
# ============================================================
import bpy, math
scene=bpy.context.scene

def P(n):
    k='NERA_'+n
    if k not in scene: raise RuntimeError('Ejecutar 01_escena_parametros.py')
    return float(scene[k])
def activate(o):
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
def move(o,c):
    col=bpy.data.collections[c]
    for x in list(o.users_collection): x.objects.unlink(o)
    col.objects.link(o)
def delete(name):
    o=bpy.data.objects.get(name)
    if o:bpy.data.objects.remove(o,do_unlink=True)
def rounded_box(name,sx,sy,sz,r,loc):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc)
    o=bpy.context.object;o.name=name;o.dimensions=(sx,sy,sz)
    activate(o);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    b=o.modifiers.new('Redondeo','BEVEL');b.width=r;b.segments=10;b.limit_method='ANGLE'
    activate(o);bpy.ops.object.modifier_apply(modifier=b.name)
    return o
def boolean(target,cutter,op,name,delete_cutter=True):
    m=target.modifiers.new(name,'BOOLEAN');m.operation=op;m.solver='EXACT';m.object=cutter
    activate(target);bpy.ops.object.modifier_apply(modifier=m.name)
    if delete_cutter: bpy.data.objects.remove(cutter,do_unlink=True)
def make_shell(name):
    W,D,H=P('CARCASA_X'),P('CARCASA_Y'),P('CARCASA_Z');wall=P('PARED')
    outer=rounded_box(name,W,D,H,P('RADIO_EXT'),(0,0,0))
    inner=rounded_box('TMP_INNER_'+name,W-2*wall,D-2*wall,H-2*wall,P('RADIO_INT'),(0,0,0))
    boolean(outer,inner,'DIFFERENCE','Hueco')
    return outer
def ring(name,outer_x,outer_z,inner_x,inner_z,depth,y,outer_r,inner_r):
    # IMPORTANTE: el bevel de radio grande NO funciona en un cuerpo delgado (Blender
    # reduce el radio a ~la mitad del espesor y las esquinas quedan casi cuadradas).
    # Por eso el anillo se construye con cuerpos ALTOS en Y (donde el radio si se
    # respeta) y despues se recorta al espesor real con una plancha.
    big=40.0
    out=rounded_box(name,outer_x,big,outer_z,outer_r,(0,y,0))
    inn=rounded_box('TMP_'+name+'_IN',inner_x,big+0.6,inner_z,inner_r,(0,y,0))
    boolean(out,inn,'DIFFERENCE','Ring')
    bpy.ops.mesh.primitive_cube_add(size=1,location=(0,y,0))
    slab=bpy.context.object;slab.dimensions=(outer_x+4,depth,outer_z+4)
    activate(slab);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    boolean(out,slab,'INTERSECT','Espesor')
    return out

for n in ('CARCASA_FRONT','CARCASA_BACK'): delete(n)
W,D,H=P('CARCASA_X'),P('CARCASA_Y'),P('CARCASA_Z'); wall=P('PARED')
# Cortadores que conservan EXACTAMENTE cada mitad en Y.
# IMPORTANTE: el margen se aplica solo en X/Z.
# En la version anterior tambien se sumaba al eje Y, por lo que FRONT y BACK
# se solapaban 6 mm alrededor de Y=0 y aparecia una especie de rectangulo/bloque
# sobresaliendo dentro de ambas carcasas.
front=make_shell('CARCASA_FRONT')
back=make_shell('CARCASA_BACK')
margin_xz=6.0
half_depth=D/2

bpy.ops.mesh.primitive_cube_add(size=1,location=(0,-D/4,0))
cf=bpy.context.object
cf.dimensions=(W+margin_xz,half_depth,H+margin_xz)
activate(cf);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
boolean(front,cf,'INTERSECT','Mitad_FRONT')

bpy.ops.mesh.primitive_cube_add(size=1,location=(0,+D/4,0))
cb=bpy.context.object
cb.dimensions=(W+margin_xz,half_depth,H+margin_xz)
activate(cb);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
boolean(back,cb,'INTERSECT','Mitad_BACK')

# Labio macho sobre BACK: anillo dentro del espesor de pared, entrando en FRONT.
lip_d=P('LIP_DEPTH'); overlap=P('LIP_OVERLAP'); inset=P('LIP_OUTER_INSET'); t=P('LIP_RADIAL_THICKNESS'); clr=P('LIP_CLEARANCE')
lox=W-2*inset; loz=H-2*inset; lix=lox-2*t; liz=loz-2*t
lip=ring('TMP_LIP_MACHO',lox,loz,lix,liz,lip_d+overlap,-lip_d/2+overlap/2,max(0.5,P('RADIO_EXT')-inset),max(0.3,P('RADIO_EXT')-inset-t))
boolean(back,lip,'UNION','Labio_Macho')

# Ranura hembra en FRONT ligeramente mas grande que el macho.
gox=lox+clr; goz=loz+clr; gix=lix-clr; giz=liz-clr
groove=ring('TMP_GROOVE',gox,goz,gix,giz,lip_d+0.5,-lip_d/2,max(0.5,P('RADIO_EXT')-inset+clr/2),max(0.3,P('RADIO_EXT')-inset-t-clr/2))
boolean(front,groove,'DIFFERENCE','Ranura_Hembra')

# Verificacion: ningun vertice debe salirse del contorno exterior redondeado de la carcasa.
def exceso_contorno(o):
    hx=W/2-P('RADIO_EXT');hz=H/2-P('RADIO_EXT');peor=-99.0
    for v in o.data.vertices:
        dx=max(abs(v.co.x)-hx,0.0);dz=max(abs(v.co.z)-hz,0.0)
        peor=max(peor,math.hypot(dx,dz)-P('RADIO_EXT'))
    return peor
for o in (front,back):
    e=exceso_contorno(o)
    if e>0.05: print(f'AVISO {o.name}: hay geometria que sobresale {e:.2f} mm del contorno exterior.')
    else: print(f'{o.name}: contorno exterior OK (exceso {max(e,0):.2f} mm).')

move(front,'NERA_IMPRESION');move(back,'NERA_IMPRESION')
for o,mn in ((front,'NERA_MAT_FRONT'),(back,'NERA_MAT_BACK')):
    mat=bpy.data.materials.get(mn)
    if mat:o.data.materials.append(mat)
    o['NERA_PRINTABLE']=True;o['NERA_REFERENCE_ONLY']=False
print('ETAPA 3: CARCASA_FRONT y CARCASA_BACK creadas con encastre macho/hembra.')
