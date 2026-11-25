# GPIO5 fijo en bajo, sin bobinas

HISTÓRICO: finalizó esta prueba temporal. El firmware actual volvió al
[PWM de 15 Hz y 5 ms activos en bajo](PRUEBA_GPIO5_15HZ.md).
El comportamiento que sigue corresponde a la prueba anterior.

El firmware de esta prueba de `Codigo Vincha` configura GPIO5 como salida normal y lo
mantiene en 0 desde `app_main`. Reemplaza la prueba PWM de 15 Hz.
El nivel es continuo, no cambia con el reloj ni las alertas ESP-NOW.
ROM/bootloader/reset quedan fuera del control de esta función.

IMPORTANTE: con el driver 2N2222 + IRFZ44N del esquema aportado, GPIO5 bajo
corresponde a activación continua. Mantener ambas bobinas y la batería
desconectadas. Esta prueba no habilita el funcionamiento con carga.

## Medición

1. Con la alimentación desconectada, conectar masa del osciloscopio a GND
   del C3 y punta a GPIO5. Alimentar por USB; usar acoplamiento DC.
2. En GPIO5 debe aparecer una línea estable cerca de 0 V, sin pulsos.
   Usar modo Auto: en modo Normal/Single podría quedar en pantalla una
   captura anterior porque ya no hay flancos para disparar.
3. En la placa de la vincha, la compuerta G del MOSFET debería subir hacia
   la alimentación de R1 si ese bus está alimentado y el montaje coincide
   con el esquema. Mantener la pinza de masa en GND, que es la fuente S
   según el esquema. GPIO5 en bajo no significa compuerta del MOSFET en bajo.

Compilar y cargar desde `Codigo Vincha` al C3 (último puerto COM20, carga
configurada a 115200). El monitor debe indicar:

```text
PRUEBA SIN BOBINAS: GPIO5 FIJO EN BAJO (0 V), SIN PWM
```

Verificación: `idf.py build` completado con ESP-IDF 6.0.2 para ESP32-C3;
imagen de aplicación 0xbf360 bytes y 25 % libre en la partición.
No se flasheó desde esta sesión. La prueba física y los niveles medidos
en la placa quedan pendientes.
