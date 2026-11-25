# Vincha ESP32-C3 — prueba GPIO5 y diagnóstico ESP-NOW

## Firmware actual: prueba de señal SIN bobinas (18/11/2026)

Al arrancar, GPIO5 genera continuamente 15 Hz: aproximadamente 2,5 ms en BAJO
y 64,17 ms en ALTO. El driver 2N2222 + IRFZ44N es inversor: bajo corresponde
a la activación. Esta prueba se carga y mide exclusivamente con las dos
bobinas desconectadas y sin batería. Comienza sin necesitar reloj ni alertas;
ESP-NOW sigue registrando eventos y enviando ACK, sin modificar el PWM.

Instrucciones de osciloscopio y limitaciones:
[`docs/PRUEBA_GPIO5_15HZ.md`](docs/PRUEBA_GPIO5_15HZ.md).

Implementación: `main/gpio5_signal_test.c`, usando LEDC por hardware.

Proyecto ESP-IDF separado del firmware del reloj. Recibe una baliza de 8 bytes
cada cinco segundos y muestra en el monitor la MAC del reloj y la secuencia.
También registra alertas de laboratorio y envía un ACK de aplicación al reloj.
No lee sensores ni toma decisiones clínicas. GPIO5 sí es una salida de prueba.

El 8/11/2026 se cargó en la C3 de MAC `84:C7:BB:FB:E9:FC` y se probó con el
reloj de MAC `28:84:85:47:BC:98`. Ver `docs/ETAPA5_RADIO_2026_11_08.md`.
El reemplazo C3 confirmado el 17/11/2026 tiene MAC `30:ED:A0:65:F9:68`
y fue detectado en COM20; comprobar el puerto antes de cargar.

## Probar

1. Abrir esta carpeta `Codigo Vincha` como proyecto ESP-IDF independiente.
   Si `idf.py` no se reconoce en PowerShell, ejecutar antes
   `. 'C:\esp\v6.0.2\esp-idf\export.ps1'`.
2. Con la vincha alimentada únicamente por USB, sin batería y con ambas bobinas desconectadas,
   ejecutar `idf.py set-target esp32c3`, `idf.py build` y luego `idf.py -p COMx flash monitor`.
3. En el reloj, confirmar que `CONFIG_NERA_ESPNOW_DIAGNOSTIC=y` está activo en
   la configuración y cargar el firmware ESP32-S3 actual.
4. Ambos equipos deben usar canal Wi-Fi 1. La vincha debe mostrar
   `Baliza recibida ... sec=N` aproximadamente cada 5 segundos.
5. Registrar la MAC de ambos equipos y probar BLE del reloj en paralelo.

La baliza usa broadcast sin cifrar y no lleva métricas ni órdenes. Las alertas
unicast de laboratorio tampoco están autenticadas criptográficamente; la MAC
del emisor solo se usa como filtro.
Antes de conectar las bobinas faltan validar corriente, driver, estado de
apagado durante reset y protección eléctrica. La prueba de señal no valida
esa etapa de potencia. La futura actuación por alertas requiere también
autenticación ESP-NOW y límites de actuación; el PWM de este firmware es
continuo e independiente de las alertas.
