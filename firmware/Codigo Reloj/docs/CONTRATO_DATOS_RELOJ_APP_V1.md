# Contrato de datos reloj ↔ app, versión 1

Este es el formato vigente para las **lecturas** entre el reloj NERA y la app.
La app ya lo recibe y valida. El emisor BLE está integrado y la comunicación
física con el teléfono fue confirmada en la etapa 3.

## Transporte e identificación

| Elemento | Valor |
| --- | --- |
| Nombre anunciado | `NERA Watch` |
| Servicio GATT | `7e400001-b5a3-f393-e0a9-e50e24dcca9e` |
| Característica de datos | `7e400003-b5a3-f393-e0a9-e50e24dcca9e` |
| Dirección | Notificaciones cifradas, reloj → app |
| Codificación | Un objeto JSON UTF-8 completo por notificación, hasta 20 bytes |

Esta versión se identifica por el servicio y la característica; el mensaje no
incluye un campo de versión. Un formato incompatible requiere una nueva versión
de servicio o característica. La configuración personal usa otra característica,
documentada en `CONFIGURACION_PERSONAL.md`.

## Campos que envía el reloj

| Campo | Tipo y unidad | Fuente | Válido cuando |
| --- | --- | --- | --- |
| `hr` | Entero, latidos por minuto; la app acepta 30–240 | MAX30102 | Hay una estimación de pulso reciente y válida |
| `spo2` | Entero, porcentaje estimado; el reloj emite 70–100 | Canales rojo e IR del MAX30102 | Hay contacto y dos ventanas ópticas concordantes |
| `temp` | Número finito, °C; la app acepta −70 a 380 | Temperatura **de objeto IR** del MLX90614 | La lectura reciente pasó la validación del sensor |

Cada notificación contiene una sola métrica. `spo2` es una estimación experimental
sin calibración del conjunto; no se debe usar para decisiones médicas. El reloj no
envía sueño, batería ni ejes del MPU en esta versión. La app conserva compatibilidad de
lectura con campos anteriores, pero eso no habilita al reloj a inventar esas
mediciones. `temp` no representa temperatura corporal central.

## Validez y tiempo

| Mensaje | Efecto en la app |
| --- | --- |
| `{"hr":76}` | Actualiza solo el pulso. |
| `{"spo2":97}` | Actualiza solo la SpO₂ estimada. |
| `{"temp":27.35}` | Actualiza solo la temperatura de objeto IR. |
| `{"hr":null}`, `{"spo2":null}` o `{"temp":null}` | Invalida inmediatamente esa métrica. |
| Campo ausente | Conserva la otra métrica hasta que venza. Nunca la convierte en cero. |
| JSON inválido, campo desconocido o valor fuera de rango | Se ignora el mensaje completo. |

La app marca cada métrica con la hora de **recepción en el teléfono**. Si pasan
más de cinco segundos sin una nueva lectura válida de esa métrica, la borra.
Al desconectarse, borra las lecturas actuales. Este mensaje en vivo no contiene
marca temporal; la hora y el historial usan características adicionales.

## Hora e historial, extensión de etapa 4

Las tres características nuevas pertenecen al mismo servicio GATT y exigen
enlace cifrado. Se usan sin alterar el JSON de datos en vivo.

| Característica | UUID (sufijo) | Operación y formato |
| --- | --- | --- |
| Hora | `0005` | La app escribe 7 bytes: versión `1`, segundos UTC Unix `uint32 LE`, desplazamiento local respecto de UTC en minutos `int16 LE`. Lee el mismo formato para confirmar. |
| Historial | `0006` | Notificación de un registro binario de 20 bytes. |
| Control de historial | `0007` | Lectura: última secuencia y generación, ambos `uint32 LE`. Escritura: última secuencia recibida `uint32 LE`; cero pide todo lo disponible. |

Registro de historial, en orden: secuencia `uint32`, segundos UTC Unix
`uint32`, segundos desde el arranque `uint32`, pulso `int16`, temperatura de
objeto IR en décimas de °C `int16` y checksum FNV-1a `uint32` de los primeros
16 bytes; todos los enteros son little-endian. Pulso cero y temperatura
`-32768` indican ausencia de esa medición. UTC cero significa que no hay hora
confiable. La app conserva estos registros en almacenamiento interno.

El reloj guarda una muestra válida por minuto en memoria circular, hasta
10.080 muestras (7 días). La app sincroniza la hora del teléfono en cada
conexión y pide los registros posteriores a su último número de secuencia.
La generación permite detectar un historial reiniciado. Si se corta la
conexión, la siguiente transferencia retoma desde lo recibido. Las muestras
tomadas antes de recibir la hora se fechan al conectar solo si pertenecen al
mismo arranque; tras apagar y volver a encender sin teléfono, su hora queda
desconocida hasta incorporar un RTC con pila.

## Comprobación

`test/watch_ble_service_test.dart` prueba valores válidos, límites, `null`,
campos ausentes y mensajes inválidos. `test/ble_contract_controller_test.dart`
prueba que cada métrica vence de forma independiente y que `null` la invalida.
La prueba del emisor real y de la reconexión con un teléfono se completó en la
etapa 3. `test/watch_history_test.dart` valida el paquete de hora y los
registros de historial; `test/history_persistence_test.dart` valida la
conservación del historial al volver a abrir la app.
