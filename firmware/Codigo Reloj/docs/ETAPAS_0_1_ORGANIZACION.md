# Etapas 0 y 1: respaldo y organización

Fecha: 7/11/2026. Rama conservada: `codex/reloj-desarrollo`.

## Alcance y carpetas de trabajo

- Reloj: `C:\Users\boca_\OneDrive\Documentos\GitHub\NERA Github\Codigo Reloj`.
- App: `C:\Users\boca_\OneDrive\Escritorio\ESCUELA\aplicacion_nera`.
- La vincha continúa como proyecto independiente en `firmware/Codigo Vincha`.
- No se trabajó sobre la copia `C:\esp\NERA_backup`.

## Etapa 0: respaldo permanente verificado

Carpeta: `C:\Users\boca_\OneDrive\Documentos\NERA_Respaldos\2026-11-07_etapa0_bf5bf1ee`.

| Archivo | Contenido |
| --- | --- |
| `reloj_antes_de_ordenar.zip` | 1.308 archivos: fuentes del reloj/vincha, configuración y binarios disponibles |
| `app_antes_de_ordenar.zip` | 160 archivos: fuentes/configuración de la app y APK disponible |
| `MANIFIESTO_SHA256.json` | Hash SHA-256 y tamaño de cada archivo y de ambos ZIP |
| `reloj_estado_git.txt`, `app_estado_git.txt` | Estado de trabajo y rama antes de ordenar |
| `LEEME.txt` | Alcance y recuperación en una carpeta nueva |

Se comprobó el CRC del ZIP y el hash de cada archivo descomprimido. Se excluyeron
cachés, `.git` y las carpetas de compilación masivas; se incluyeron explícitamente
los binarios y archivos de flasheo disponibles para facilitar la recuperación.
Este respaldo conserva también los cambios locales que ya existían.

## Etapa 1: qué se reorganizó

1. Los sensores activos forman un solo componente: `components/sensores`.
2. El archivo conjunto de MPU/MLX se separó en bus I2C, MPU6050 y MLX90614.
3. MAX30102, servicio y procesamiento del pulso se trasladaron al mismo componente.
4. Se actualizaron los includes y la lista de compilación, manteniendo las APIs.
5. `lib/main.dart` se dividió en pantallas, widgets, diálogos y formato, con `part`.
   Los cuerpos de la biblioteca Dart reconstruida coinciden con el original.
6. Se añadieron guías para distinguir el código activo del histórico. Los archivos
   históricos del firmware y los cuatro archivos vacíos previos de pantallas de
   la app se conservaron.

No se ajustaron algoritmos, parámetros, direcciones, pines, interfaz visual,
temporización ni protocolos. No se implementaron etapas posteriores: sueño,
ansiedad, envío a familiares, SpO2 o nuevas acciones de la bobina.

## Archivos existentes modificados y líneas

Los números se refieren a los archivos finales. Una eliminación indica también
el rango original para poder localizarla al comparar con el respaldo.

### Firmware

| Archivo | Líneas o sección modificada |
| --- | --- |
| `main/CMakeLists.txt` | Eliminadas las líneas originales 17–18; contexto final 16; 19–19 |
| `main/app_main.cpp` | 6–6; 17–17 |
| `main/connectivity/espnow_diagnostic.cpp` | 10–10 |
| `main/ui/ui_manager.cpp` | 5–5 |
| `tests/heart_rate_estimator_test.cpp` | 2–2 |
| `tests/max30102_red_ir_test.cpp` | 1–2 |
| `docs/PLAN_INTEGRACION_NERA.md` | 2–6 |

### Aplicación

| Archivo | Líneas o sección modificada |
| --- | --- |
| `lib/main.dart` | 7–16; Eliminadas las líneas originales 166–1533; contexto final 175 |

## Archivos nuevos o trasladados: todas sus líneas

En los archivos trasladados, estos rangos identifican dónde quedó el código;
no significan que se haya escrito un algoritmo nuevo. Se conservaron licencias
y comentarios del procesamiento original.

### Firmware

| Archivo | Líneas finales |
| --- | --- |
| `LEER_PRIMERO.md` | 1–76 |
| `components/sensores/CMakeLists.txt` | 1–11 |
| `components/sensores/README.md` | 1–46 |
| `components/sensores/drivers/i2c_bus.c` | 1–32 |
| `components/sensores/drivers/max30102.cpp` | 1–117 |
| `components/sensores/drivers/max30102_fifo.h` | 1–11 |
| `components/sensores/drivers/mlx90614.c` | 1–45 |
| `components/sensores/drivers/mpu6050.c` | 1–68 |
| `components/sensores/include/i2c_bus.h` | 1–14 |
| `components/sensores/include/max30102.h` | 1–12 |
| `components/sensores/include/mlx90614.h` | 1–21 |
| `components/sensores/include/mpu6050.h` | 1–42 |
| `components/sensores/include/sensor_config.h` | 1–11 |
| `components/sensores/include/sensor_service.h` | 1–27 |
| `components/sensores/processing/heart_rate_estimator.h` | 1–70 |
| `components/sensores/processing/pba_detector.h` | 1–82 |
| `components/sensores/processing/pulse_intervals.h` | 1–45 |
| `components/sensores/sensor_service.cpp` | 1–216 |

### Aplicación

| Archivo | Líneas finales |
| --- | --- |
| `GUIA_CODIGO.md` | 1–44 |
| `lib/dialogs/profile_contact_dialogs.dart` | 1–149 |
| `lib/screens/health_view.dart` | 1–89 |
| `lib/screens/home_view.dart` | 1–86 |
| `lib/screens/profile_view.dart` | 1–130 |
| `lib/screens/support_view.dart` | 1–82 |
| `lib/utils/display_format.dart` | 1–21 |
| `lib/widgets/breathing_sheet.dart` | 1–95 |
| `lib/widgets/nera_widgets.dart` | 1–740 |

### Relación de ubicaciones anteriores con las nuevas

| Antes | Ahora |
| --- | --- |
| `components/mpumlx/mpumlx.c` | `components/sensores/drivers/i2c_bus.c`, `mpu6050.c` y `mlx90614.c` |
| `components/mpumlx/mpumlx.h` | `components/sensores/include/sensor_config.h`, `i2c_bus.h`, `mpu6050.h`, `mlx90614.h` |
| `components/mpumlx/CMakeLists.txt` | `components/sensores/CMakeLists.txt` |
| `main/sensors/sensor_service.cpp` | `components/sensores/sensor_service.cpp` |
| `main/sensors/sensor_service.h`, `max30102.h` | `components/sensores/include/` |
| `main/sensors/max30102.cpp`, `max30102_fifo.h` | `components/sensores/drivers/` |
| `main/sensors/heart_rate_estimator.h`, `pba_detector.h`, `pulse_intervals.h` | `components/sensores/processing/` |
| Pantallas y controles antes dentro de `lib/main.dart` | Partes listadas en `lib/main.dart:9–16`; ver `GUIA_CODIGO.md` de la app |

## Secciones para capturas y Trello

- Bus externo: `components/sensores/include/sensor_config.h:9–11`.
- Datos disponibles y banderas de validez: `components/sensores/include/sensor_service.h:7–24`.
- Muestreo/reintentos: `components/sensores/sensor_service.cpp:17–20`.
- Publicación de pulso, temperatura y movimiento: `sensor_service.cpp:25–66`.
- Tarea de adquisición: `sensor_service.cpp:68`; API pública: `191–216`.
- Configuración óptica del MAX: `components/sensores/drivers/max30102.cpp:8–16`.
- Organización de pantallas de la app: `lib/main.dart:8–16`.
- Mapa completo para principiantes: `LEER_PRIMERO.md`, `components/sensores/README.md`
  y `GUIA_CODIGO.md` en la app.

## Validación

- Firmware S3: compilación correcta; `NERA.bin` de 0x10f790 bytes, 73 % de la
  partición de aplicación libre.
- Tres archivos de pruebas C++ existentes: comprobación de sus `static_assert`
  aprobada para alertas, decodificación FIFO rojo/IR y estimador de pulso.
- Flutter: 9 pruebas existentes aprobadas.
- Análisis Flutter: 17 avisos informativos previos por `withOpacity`, sin errores
  ni advertencias nuevos. No se cambiaron estilos para ocultar esos avisos.
- Comparación con el respaldo: cuerpos originales de los tres drivers, servicio
  y procesamiento conservados (solo includes adaptados donde correspondía).
- `sdkconfig`, dependencias, LVGL, pantalla/táctil y fuentes de la vincha idénticos
  al respaldo. No se hicieron operaciones de publicación ni cambios de rama.
- APK debug compilado correctamente: `build/app/outputs/flutter-apk/app-debug.apk`
  dentro de la carpeta de la app.

No se flasheó automáticamente. Las pruebas de lógica no verifican calidad óptica,
conexiones físicas ni estabilidad de radio en las placas.

## Prueba física antes de avanzar

1. Desde la carpeta del reloj, compilar y flashear el S3 con su puerto actual.
   Comprobar pantalla, táctil y las tres lecturas como antes.
2. Con la vincha encendida y su firmware actual, comprobar recepción de baliza y
   confirmación de una alerta controlada, manteniendo deshabilitada la bobina.
3. Si se instala el APK, comprobar Inicio, Salud, Apoyo y Perfil y que se conserva
   la navegación. La disponibilidad BLE del firmware es la misma que antes de
   esta etapa; aquí no se reactivó ningún módulo excluido.

Registrar resultados y logs antes de continuar con cambios de comportamiento o
diseño. Si algo falla, comparar con el respaldo en una carpeta nueva.
