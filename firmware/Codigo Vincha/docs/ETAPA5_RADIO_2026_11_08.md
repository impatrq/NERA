# Etapa 5: prueba física de radio reloj ↔ vincha C3

Fecha: 8/11/2026. Se probaron ambas placas conectadas por USB, con las bobinas
desconectadas. El receptor C3 no configura GPIO de salida.

## Equipo y firmware

- Reloj ESP32-S3: firmware activo de `Codigo Reloj`, COM16 durante la prueba;
  MAC ESP-NOW `28:84:85:47:BC:98`.
- Vincha ESP32-C3: proyecto `Codigo Vincha`, compilado y cargado por COM17; esptool
  confirmó MAC `84:C7:BB:FB:E9:FC` y hash del firmware tras la escritura.
- Ambos usan canal 1 y el protocolo de laboratorio v2 descrito en
  `ALERTAS_LAB.md`. El ACK confirma recepción en el programa C3, no actuación.

## Resultado observado en monitores

1. La C3 recibió balizas consecutivas del reloj, aproximadamente cada cinco
   segundos, con secuencias 94, 95 y 96 al inicio de la prueba.
2. Al apuntar el MLX90614 a 33,49 °C, el reloj produjo el evento de temperatura
   `id=1`, estado activo. La C3 lo registró una vez, devolvió ACK y el reloj
   registró `ACK C3 CONFIRMADO`.
3. A 29,17 °C el reloj produjo `id=2`, estado restablecido. La C3 y el reloj
   registraron el ACK de ese evento.
4. Con la C3 desconectada, el evento activo `id=7`, 48,67 °C, tuvo tres envíos
   y terminó en `SIN CONFIRMACION C3`. También se registró la falta de
   confirmación del restablecimiento `id=8`.
5. Al reconectar la C3, el reloj creó una alerta nueva `id=9` por una nueva
   transición a 51,39 °C; la C3 la recibió y confirmó. No se observó reenvío
   de `id=7` ni de `id=8`.
6. Con la app NERA conectada por BLE, el reloj registró `Telefono conectado`,
   `Enlace cifrado: si` y `Notificaciones activadas`. La persona confirmó que
   la app mostraba lecturas y la C3 siguió recibiendo balizas 91, 92 y 93.

La comunicación de laboratorio cumple la prueba de evento, ACK, pérdida de
receptor y coexistencia BLE/ESP-NOW. Falta definir y validar la etapa de
actuación física. El esquema `vincha esquematico (1).kicad_sch` dibuja dos
bobinas `L1/L2` sin valores eléctricos y dos MOSFET `IRF3205`; no basta para
elegir una corriente o duración de pulso. No se habilitó ninguna salida.

## Extractos de los registros serie

```text
RELOJ EVENTO LAB id=1 metrica=2 estado=1 valor=33.49
C3    ALERTA LAB ACTIVA: TEMPERATURA OBJETO C valor=33.49 evento=1; SIN ACTUACION
C3    ACK LAB evento=1 nuevo: ESP_OK
RELOJ ACK C3 CONFIRMADO evento=1 metrica=2 estado=1
RELOJ EVENTO LAB id=2 metrica=2 estado=0 valor=29.17
C3    ALERTA LAB RESTABLECIDA: TEMPERATURA OBJETO C valor=29.17 evento=2; SIN ACTUACION
RELOJ ACK C3 CONFIRMADO evento=2 metrica=2 estado=0
RELOJ EVENTO LAB id=7 metrica=2 estado=1 valor=48.67
RELOJ ENVIO LAB id=7 intento=1: ESP_OK
RELOJ ENVIO LAB id=7 intento=2: ESP_OK
RELOJ ENVIO LAB id=7 intento=3: ESP_OK
RELOJ SIN CONFIRMACION C3 evento=7 tras 3 intentos
RELOJ Telefono conectado
RELOJ Enlace cifrado: si
RELOJ Notificaciones activadas
C3    Baliza recibida de 28:84:85:47:BC:98 sec=91
C3    Baliza recibida de 28:84:85:47:BC:98 sec=92
C3    Baliza recibida de 28:84:85:47:BC:98 sec=93
```
