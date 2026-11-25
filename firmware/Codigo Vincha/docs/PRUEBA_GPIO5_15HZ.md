# Prueba GPIO5 sin bobinas — 18/11/2026

Prueba actual: PWM de 15 Hz restaurado tras la prueba temporal de nivel bajo
fijo, a pedido del usuario. El tiempo activo se redujo de 5 ms a 2,5 ms.

## Comportamiento cargado al compilar esta versión

El proyecto `Codigo Vincha` inicia un PWM continuo en GPIO5 al entrar en
`app_main`, antes de iniciar ESP-NOW. No espera al reloj ni a una alerta.
Las alertas recibidas solo se registran y confirman; no cambian este PWM.
Se detiene al quitar la alimentación. Durante reset/bootloader el programa
no controla GPIO5: no conectar bobinas para esta prueba.

| Parámetro | Valor nominal |
| --- | --- |
| Frecuencia | 15 Hz |
| Período | 66,667 ms |
| GPIO5 bajo (activación del driver) | 2,5 ms |
| GPIO5 alto (apagado del driver) | 64,167 ms |
| Fracción activa baja | 3,75 % |

Se usa LEDC, timer 0, canal 0, reloj XTAL y resolución de 14 bits con salida
invertida. El pulso ocupa 614 de 16384 ticks: unos 2,4984 ms a 15 Hz.
La frecuencia real depende de la tolerancia del reloj y del divisor LEDC;
los valores se comprueban con el osciloscopio. No se usan esperas FreeRTOS
para generar los flancos.

## Qué se mide sin bobinas

El osciloscopio mide tensión entre su punta y su referencia. El C3 puede
producir esa tensión aunque no haya una bobina conectada. La primera medición
comprueba frecuencia, duración y amplitud de la orden. La segunda comprueba
que la etapa 2N2222 conmuta la compuerta del MOSFET. Estas mediciones no
comprueban corriente de bobinas, campo magnético ni calentamiento bajo carga.

1. Desconectar físicamente ambas bobinas y la batería. Alimentar el C3 por USB.
2. Con la placa sin alimentación, fijar la pinza de masa del osciloscopio a
   **GND común**. No colocar esa pinza en 5V, compuerta ni drenador: puede
   cortocircuitar ese nodo a tierra a través del instrumento/USB.
3. Usar sonda x10 y seleccionar también x10 en el canal, acoplamiento DC.
   Colocar la punta en GPIO5 (J4 pin 9 en el esquema confirmado).
4. Alimentar y comenzar con 10 ms/div, 1 V/div y disparo por flanco descendente
   alrededor de 1,5 V. Se esperan unos 3,3 V durante 64,17 ms y unos 0 V
   durante 2,5 ms. Medir ancho del pulso NEGATIVO. El duty positivo será ~96,25 %.
5. Para ver un ciclo completo, usar 10 ms/div; para ampliar el tramo bajo,
   usar 1 ms/div. Confirmar ~66,67 ms entre dos flancos descendentes.
6. Para comprobar el driver, quitar alimentación antes de mover la punta
   a la compuerta G de Q2, manteniendo la masa en GND (fuente S de Q2 según
   el esquema). Volver a alimentar. Si el bus de R1 recibe ~5 V, se esperan
   pulsos positivos hacia esa tensión durante ~2,5 ms, y cerca de 0 V el resto.
   R1 y la capacitancia de compuerta afectan el tiempo de subida: medirlo.
   Si ese bus no está alimentado, no se espera ese nivel de compuerta.

Con las bobinas desconectadas, el drenador puede quedar flotante: empezar
por GPIO5 y compuerta; el drenador sin carga no confirma conmutación de potencia.

## Esquema revisado y límites

Archivo aportado: `vincha pcb y esquematico (1).kicad_sch`, KiCad 10.
J4.9 -> R2 (1 kΩ) -> base Q3 (2N2222); emisor Q3 a GND; colector Q3
a compuerta Q2 (IRFZ44N) y R1 (10 kΩ) hacia 5V5; fuente Q2 a GND.
Por ello GPIO5 alto apaga y GPIO5 bajo activa el driver.

R1 puede elevar la compuerta mientras GPIO5 no está controlado durante
arranque/reset. Precargar GPIO5 alto en `app_main` no elimina ese intervalo.
Esta prueba no corrige ese comportamiento del circuito ni valida el IRFZ44N
con excitación de compuerta de ~5 V. No conectar bobinas con este firmware
de prueba: todavía faltan validaciones eléctricas y límites para la carga.

## Compilar y cargar

Abrir `Codigo Vincha` como proyecto independiente en VS Code. Ejecutar Build y
Flash usando el puerto del C3 (último confirmado: COM20). No cambiar el
destino al S3. El monitor debe mostrar:

```text
NERA/GPIO5_TEST: PRUEBA CONTINUA SIN BOBINAS: GPIO5, 15 Hz, activo BAJO ~2.5 ms, alto ~64.17 ms
```

Verificación: `idf.py build` completado con ESP-IDF 6.0.2 para ESP32-C3.
Imagen de aplicación: 0xc2340 bytes, 24 % libre en la partición de 1 MiB.
No se flasheó la placa desde esta sesión. La medición física con osciloscopio
queda pendiente; un build correcto no confirma la forma de onda en la placa.
