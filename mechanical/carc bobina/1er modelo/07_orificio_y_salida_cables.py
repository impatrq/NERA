# ETAPA 7 - ORIFICIO DE SALIDA DE CABLES
# Blender 5.2
# Realiza el agujero inferior en la media carcasa.
# Se puede ajustar el radio y posicion al principio.

import bpy

CABLE_PORT_R = 3.8
CABLE_PORT_X = 18.0
CABLE_PORT_Y = 6.5

X_POS = 55.0
H = 70.0

shell = bpy.data.objects.get("Carcasa_Corte_Mitad")

if shell is None:
    raise RuntimeError("No se encontro 'Carcasa_Corte_Mitad'. Ejecutar antes la etapa 3.")

# Cilindro vertical atravesando la pared inferior
bpy.ops.mesh.primitive_cylinder_add(
    vertices=64,
    radius=CABLE_PORT_R,
    depth=12.0,
    location=(X_POS + CABLE_PORT_X, CABLE_PORT_Y, -H/2.0)
)

cutter = bpy.context.object
cutter.name = "Cortador_Salida_Cables"

mod = shell.modifiers.new("Salida_Cables", 'BOOLEAN')
mod.operation = 'DIFFERENCE'
mod.solver = 'EXACT'
mod.object = cutter

bpy.ops.object.select_all(action='DESELECT')
shell.select_set(True)
bpy.context.view_layer.objects.active = shell
bpy.ops.object.modifier_apply(modifier=mod.name)

bpy.data.objects.remove(cutter, do_unlink=True)

print("ETAPA 7 COMPLETA: orificio para cables realizado.")
