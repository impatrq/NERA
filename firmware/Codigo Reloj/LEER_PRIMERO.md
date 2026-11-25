# NERA: por dónde empezar a leer el código

Esta guía describe el firmware activo después de las etapas 0 y 1. Se ordenaron
archivos, sin cambiar las lecturas, la pantalla, los GPIO ni ESP-NOW.

## Mapa del reloj

| Quiero encontrar... | Archivo o carpeta activa |
| --- | --- |
| El arranque del reloj | `main/app_main.cpp` |
| Qué archivos se compilan | `main/CMakeLists.txt` y `components/sensores/CMakeLists.txt` |
| Las tres lecturas y sus estados de validez | `components/sensores/sensor_service.cpp` |
| Los datos que recibe la interfaz | `components/sensores/include/sensor_service.h` |
| SDA, SCL y el puerto I2C externo | `components/sensores/include/sensor_config.h` |
| Temperatura MLX90614 y comprobación PEC | `components/sensores/drivers/mlx90614.c` |
| Movimiento MPU6050 | `components/sensores/drivers/mpu6050.c` |
| Lectura óptica y LED del MAX30102 | `components/sensores/drivers/max30102.cpp` |
| Cálculo y conservación temporal del pulso | `components/sensores/processing/` |
| La pantalla que se muestra actualmente | `main/ui/ui_manager.cpp` |
| Control físico de pantalla y táctil | `main/display/`, `main/touch/`, `main/lvgl/` |
| Envío de alertas y recepción de confirmación | `main/connectivity/espnow_diagnostic.cpp` |
| Reglas actuales de alerta | `main/connectivity/lab_alert_rules.h` |
| Preferencias guardadas del usuario | `main/connectivity/user_alert_preferences.cpp` |
| Formato compartido de los mensajes ESP-NOW | `../Protocolo Reloj Vincha/nera_lab_protocol.h` |
| Programa independiente de la vincha | `../Codigo Vincha/main/` |

## Cómo circulan los datos

1. `app_main.cpp` inicia la pantalla y el servicio de sensores.
2. El servicio configura el bus externo y consulta los drivers.
3. El MAX entrega muestras IR; el procesamiento busca intervalos de latidos.
4. El servicio publica una copia protegida de las últimas lecturas y su validez
   (`SensorSnapshot`). La pantalla y las alertas consultan esa copia.
5. ESP-NOW envía mensajes a la vincha y comprueba su confirmación. La vincha es
   otro proyecto: se abre y compila desde `../Codigo Vincha`, con destino ESP32-C3.

La UI no debe leer I2C directamente. Así se mantiene separado el dibujo de la
adquisición de datos. La configuración de memoria gráfica en PSRAM se conserva.

## Antes de modificar

- **MODIFICABLE:** presentación y textos de `main/ui/ui_manager.cpp`, cuando se
  apruebe la etapa de diseño. Los archivos de sensores no controlan el diseño.
- **FUNCIONAL:** callbacks, adquisición, temporizadores, validez de datos y
  protocolos. Cambiarlos requiere probar el comportamiento, no solo compilar.
- **IMPORTANTE:** conservar GPIO, direcciones, configuración del display y memoria
  LVGL mientras se comprueba esta reorganización. No editar archivos en `build/`.

Las constantes del bus siguen en I2C1, SDA GPIO21 y SCL GPIO2. El táctil mantiene
su bus propio. El MAX conserva LED rojo + IR y el pulso se calcula con IR.

## Archivos históricos: evitar confusiones

En el proyecto quedan archivos de etapas anteriores en `main/core/`,
`main/services/`, `main/power/`, `main/storage/` y subcarpetas antiguas de `main/ui/`.
Su presencia no significa que se compilen. La lista efectiva está en
`main/CMakeLists.txt`. La pantalla activa de esta versión es `ui_manager.cpp`.
Estos archivos históricos se conservaron; su limpieza es una tarea posterior.

Las pruebas antiguas de la arquitectura previa se excluyeron de esta preparacion.
Las pruebas vigentes usadas aquí son `lab_alert_test.cpp`,
`max30102_red_ir_test.cpp` y `heart_rate_estimator_test.cpp`.

## Respaldo y registro de cambios

El respaldo completo previo está fuera del repositorio, en:

`C:\Users\boca_\OneDrive\Documentos\NERA_Respaldos\2026-11-07_etapa0_bf5bf1ee`

Contiene fuentes del reloj, vincha y app, binarios disponibles, estado de Git y
hashes SHA-256. Para recuperar, descomprimir en una carpeta nueva y consultar
`LEEME.txt`; no sobrescribir el trabajo actual sin compararlo.

El inventario de archivos y líneas de esta etapa está en
`docs/ETAPAS_0_1_ORGANIZACION.md`.
