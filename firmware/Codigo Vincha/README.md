# Vincha NERA — ESP32-C3

Proyecto ESP-IDF independiente, ubicado en firmware/Codigo Vincha/.
El reloj y la vincha usan los archivos de ../Protocolo Reloj Vincha/.

## Flujo de alertas v3

1. El reloj detecta una condición sostenida y comunica el evento a la app por BLE.
2. La app confirma la recepción mientras está abierta y en primer plano.
3. El reloj envía una orden vigente a la vincha por ESP-NOW.
4. La vincha aplica GPIO5 y después envía la confirmación al reloj.
5. El reloj informa a la app del estado confirmado o del fallo.

GPIO5 inicia en alto, con la salida de prueba apagada. Una orden activa habilita
la señal de 15 Hz y aproximadamente 2,5 ms en bajo del driver existente.
La salida se apaga cuando finalizan las alertas o vence la renovación de cinco
segundos, más el intervalo de sondeo. Las dos métricas se combinan mediante OR.
Los paquetes anteriores y los duplicados de una activación ya vencida no la reactivan.

El ACK confirma que el firmware aplicó una orden al periférico GPIO/LEDC.
No mide corriente, campo magnético ni funcionamiento de las bobinas.

## Compilar y comprobar

Usar ESP-IDF 6.0.2 con destino esp32c3 y abrir esta carpeta como proyecto independiente.
Ejecutar idf.py build para compilar. Elegir el puerto real de la placa al cargar.
El reloj debe usar el firmware v3 correspondiente y tener habilitado
CONFIG_NERA_ESPNOW_DIAGNOSTIC. Las MAC configuradas deben corresponder a las dos placas.

La primera comprobación se realiza por USB, con las bobinas y la batería de potencia
desconectadas, como se indica en la documentación de laboratorio.
Compilar no valida la etapa eléctrica ni la forma de onda en la placa.

## Documentación

- [Flujo y confirmaciones](../Codigo%20Reloj/docs/ALERTAS_LAB.md).
- [Señal GPIO5 y registros de laboratorio](docs/PRUEBA_GPIO5_15HZ.md).
- [Estado de la etapa de potencia](docs/ETAPA5_ACTUADOR_PENDIENTE.md).
- [Registro anterior de la prueba de radio](docs/ETAPA5_RADIO_2026_11_08.md).

ESP-NOW conserva el filtro por MAC del montaje y no incorpora autenticación
criptográfica en esta entrega. BLE y ESP-NOW son enlaces distintos; el reloj es
el puente entre el teléfono y la vincha.
