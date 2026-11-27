# Alertas reloj -> app -> reloj -> vincha, protocolo v3

Actualización: 26/11/2026. Integración de la señal GPIO5 del prototipo.

## Secuencia

1. El reloj evalúa muestras reales y vigentes de los sensores.
2. Superado un umbral durante el tiempo configurado, publica un evento BLE.
3. La app recibe el evento, lo muestra y confirma su identidad al reloj.
4. Solo entonces el reloj envía la orden por ESP-NOW a la vincha.
5. El C3 aplica GPIO5 y después envía el ACK al reloj.
6. El reloj comunica ese ACK a la app: «Vincha confirmó GPIO5 activado».

El celular no tiene ESP-NOW: el reloj es el puente entre Bluetooth y el C3.
El éxito de esp_now_send no se interpreta como recepción; se exige el ACK C3.
El ACK confirma que el firmware aplicó la orden al periférico GPIO/LEDC;
no mide corriente, campo magnético ni funcionamiento de las bobinas.

## Parámetros conservados

| Métrica | Activación | Restablecimiento |
| --- | --- | --- |
| Pulso | >100 BPM | <90 BPM |
| Temperatura de objeto IR | >33 °C | <31 °C |

Tiempo sostenido: alta 5 s, media 10 s, baja 20 s, elegido en Perfil.
Son reglas independientes (OR); igualar el umbral de activación no dispara.
En Ejercicio se pausa pulso y se mantiene temperatura.
Son umbrales de laboratorio, no límites médicos. Objeto IR no es temperatura
corporal central. Se mantienen la histéresis y el rechazo de datos inválidos
u obsoletos (>3 s). Repetir una muestra no suma tiempo.

## Salida y pérdidas de comunicación

GPIO5 inicia alto, sin PWM. Una orden activa habilita 15 Hz, aproximadamente
2,5 ms bajos y 64,17 ms altos por ciclo. Se conserva la polaridad del driver
2N2222 existente. Se usa LEDC sin inversión y duty alto de 15770/16384.
Si hay dos alertas, finalizar una mantiene GPIO5 activo mientras siga la otra.

La app debe permanecer abierta, en primer plano, conectada y suscrita a las
alertas por BLE cifrado. Renueva su disponibilidad y recepción aproximadamente
cada segundo; el permiso vence a los 3 s. Pasar a segundo plano suspende esas
renovaciones: no hay servicio ni notificaciones del sistema en segundo plano.
Sin app disponible no se inicia una activación. Al volver se debe cumplir
otra vez el tiempo de calificación.

El reloj renueva la orden C3 cada segundo mientras la lectura, el perfil y la
recepción por la app sigan vigentes. Reintenta hasta 3 veces sin ACK; después
muestra «Sin confirmación». La C3 apaga la salida si no recibe renovación
durante 5 s (más el sondeo de unos 100 ms), aunque el reloj se apague.
Al normalizarse la lectura, invalidarse el sensor, desactivar avisos o perder
la app, el reloj envía la inactivación sin esperar permiso del teléfono.
Si esa inactivación se pierde, actúa el timeout local del C3. Desde un cierre
sin aviso de la app, el límite combinado es aproximadamente 3 s + 5 s, más
los intervalos de sondeo; el apagado normal llega antes por la orden explícita.

No se retransmiten activaciones históricas. Un estado nuevo reemplaza el
pendiente anterior. Tras expirar la concesión C3, repetir la misma activación
no vuelve a encender GPIO5. Un paquete anterior al último de esa métrica se
rechaza. Reiniciar el reloj crea otra sesión. Los avisos de la app no se
repiten por cada renovación y el estado visible vence si deja de actualizarse.

## Cómo comprobar que llega

Primera prueba: bobinas y batería de potencia desconectadas, C3 por USB.
Se conserva esta condición del ensayo eléctrico anterior: el circuito durante
reset/bootloader y la etapa de potencia aún requieren validación.

1. Cargar AMBOS firmwares nuevos (primero C3, luego S3) e instalar la app actualizada.
   El protocolo v3 rechaza el anterior. Mantener el canal 1 en ambas placas.
2. Abrir el monitor del C3: debe imprimir «GPIO5 APAGADO» al arrancar.
3. Conectar NERA Watch desde la app. En Perfil habilitar avisos, elegir
   sensibilidad Alta (5 s) y Uso cotidiano. Verificar ajustes confirmados.
4. Acercar un objeto templado al MLX para sostener >33 °C al menos 5 s de
   lecturas válidas. No acelerar el pulso de una persona para probar.
5. La app muestra el aviso y la tarjeta «Alertas y vincha» pasa por recepción,
   espera de confirmación y «Vincha confirmó GPIO5 activado».
6. En el C3 aparecen «GPIO5 ACTIVADO», «ALERTA ACTIVA» y
   «ACK GPIO5 ... salida=ACTIVA: ESP_OK». En el S3: «ACK C3 CONFIRMADO».
7. Para comprobar la señal eléctrica, con el montaje sin alimentación conectar
   la masa del osciloscopio a GND común y punta x10 a GPIO5. Al alimentar,
   antes de la alerta se espera alto cercano a 3,3 V. Durante la alerta:
   flancos descendentes cada ~66,67 ms y pulsos bajos de ~2,5 ms.
   Usar 10 ms/div para el período y 1 ms/div para el ancho bajo.
8. Retirar el objeto y esperar <31 °C: «GPIO5 APAGADO». Si pulso también
   sigue en alerta, primero debe finalizar esa condición.
9. Cortar alimentación del C3: la app debe dejar de confirmar y mostrar fallo.
   Cortar alimentación del reloj con el C3 activo: debe apagarse por timeout.
   Volver a alimentar el C3 no recupera automáticamente una activación vieja;
   restablecer la condición y generar otra alerta para repetir el ensayo.

La lista MAC es un filtro de laboratorio, no autenticación criptográfica.
No se agregaron claves ESP-NOW ni se validó la carga de las bobinas.
Los detalles eléctricos pendientes están en ETAPA5_ACTUADOR_PENDIENTE.md.

## Archivos y contrato

- Umbrales: main/connectivity/lab_alert_rules.h.
- Flujo BLE/radio: alert_delivery.{h,cpp}, ../Protocolo Reloj Vincha/nera_delivery_state.h.
- Estado y watchdog C3: ../Protocolo Reloj Vincha/nera_output_state.h.
- Trama: ../Protocolo Reloj Vincha/nera_lab_protocol.h; 20 bytes, magic NERA, versión 3,
  tipo, métrica, estado, sesión LE32, id LE32, valor firmado en centésimas LE32.
- ESP-NOW: tipo 2 orden, tipo 3 ACK aplicado. Canal 1.
- BLE: característica 7e400008-b5a3-f393-e0a9-e50e24dcca9e, NOTIFY/WRITE cifrada.
  Tipo 4 espera app, 5 recepción app, 6 envío, 7 confirmado C3, 8 fallo,
  9 disponibilidad del teléfono (sesión/id/valor 0, métrica 1, estado 0).
- MAC vincha: 30:ED:A0:65:F9:68; reloj aceptado por C3: 28:84:85:47:BC:98.
  Si cambia una placa, actualizar las constantes de ambos proyectos.
- Pruebas: tests/lab_alert_test.cpp (C++17, static_assert) y
  aplicacion_nera/test/watch_alert_test.dart, más la suite Flutter existente.

Compilar no comprueba recepción por radio ni forma de onda física. En esta
sesión se generan binarios; la carga y la prueba en las placas son un paso separado.

## Validación del 26/11/2026

- ESP-IDF 6.0.2: build correcto del S3 y del C3.
- Pruebas C++17: umbrales, protocolo v3, permiso de app, expiración,
  reconexión, OR de dos métricas y rechazo de mensajes viejos: static_assert OK.
- Flutter: 16 pruebas correctas; análisis sin errores ni warnings. Permanecen
  17 avisos informativos previos por withOpacity deprecado.
- APK de prueba generado en aplicacion_nera/build/app/outputs/flutter-apk/app-debug.apk.
- Binarios: build/NERA.bin y ../Codigo Vincha/build/nera_vincha_diagnostico.bin.
- No se flashearon placas ni se comprobó la salida con instrumentos en esta sesión.
