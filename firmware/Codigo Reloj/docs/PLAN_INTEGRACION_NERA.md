# Plan de integración NERA

> Documento histórico del plan de integración. La organización activa y las
> etapas 0/1 aprobadas el 7/11/2026 se describen en `../LEER_PRIMERO.md` y
> `ETAPAS_0_1_ORGANIZACION.md`. Las rutas y estados de abajo pueden corresponder
> a versiones anteriores; no usarlos como inventario del firmware actual.

Estado: revisión estática del código al 31/10/2026. Esta guía no reemplaza pruebas en las placas.
Repositorio del reloj: Codigo Reloj, rama codex/reloj-desarrollo. No usar NERA_backup para flashear este plan.
Aplicación: C:\Users\boca_\OneDrive\Escritorio\ESCUELA\aplicacion_nera.
Firmware de vincha: proyecto de diagnóstico nuevo en firmware/Codigo Vincha.
El archivo ESCUELA\codigo vincha estaba vacío; el actuador sigue sin configurar.

## Regla de trabajo

Hacer una etapa por vez. Antes de pasar a la siguiente, compilar, flashear cuando corresponda, probar físicamente y guardar el log. Registrar archivos y líneas cambiadas. No mezclar ajustes de hardware con rediseños de UI. No publicar datos simulados como mediciones reales.

## Etapa 1 — Línea base y sensores (prioridad inmediata)

Estado: parcialmente implementada; pendiente validación física estable.

- Confirmar que VS Code compila y flashea este repositorio y que el monitor muestra la versión esperada.
- Bus externo vigente en main/nera_config.h: SDA GPIO21, SCL GPIO2, I2C1; touch separado en SDA48/SCL47, I2C0.
- Corregir la documentación vieja que aún menciona SDA8/SCL9; no cambiar los GPIO activos sin medir el cableado.
- Probar MLX90614 0x5A, MPU6050 0x68/0x69 y MAX30102 0x57 de uno en uno y después juntos.
- Para MLX, comprobar ACK, PEC, lectura estable y recuperación tras desconexión. La prueba anterior dio lecturas reales intermitentes.
- El MAX30102 se calentó en pruebas previas. Mantenerlo desconectado hasta revisar alimentación, polaridad, soldadura y módulo; no usar software para ocultar ese síntoma.
- Confirmar que ningún sensor ausente produce un valor inventado ni bloquea UI/touch.
- Desactivar el diagnóstico prolongado de arranque de real_sensors.cpp solo después de conservar un modo de diagnóstico reproducible.

Criterio de salida: logs y pantalla muestran lecturas válidas repetibles de cada módulo probado y estados "sin lectura" al desconectarlo. No se exige que los tres funcionen si hay una falla física sin resolver; documentar el bloqueo exacto.

## Etapa 2 — Contrato de datos reloj ↔ aplicación

Estado: contrato BLE v1 de pulso y temperatura IR documentado en
`CONTRATO_DATOS_RELOJ_APP_V1.md` y probado en el parser/controlador de la app
el 8/11/2026. Sin reloj de tiempo real sincronizado. La emisión física queda
para la etapa 3.

- Definir un documento único con versión de protocolo, UUID BLE, unidades, validez por métrica, marca de tiempo y ejemplos.
- La app hoy espera nombre "NERA Watch", servicio 7e400001-b5a3-f393-e0a9-e50e24dcca9e y característica 7e400003-b5a3-f393-e0a9-e50e24dcca9e; confirmar esos valores antes de programar el servidor.
- El parser espera hr, spo2, temp, ax, ay, az. La extensión posterior calcula una
  SpO2 experimental desde rojo e IR, sin valor fijo. Los campos ausentes no deben
  convertirse en cero válido.
- Diferenciar temperatura de objeto IR de temperatura corporal y movimiento medido de sueño inferido.

Criterio de salida: un mensaje documentado se interpreta igual en ambos proyectos, con pruebas para campos válidos, ausentes y datos antiguos.

## Etapa 3 — BLE real del reloj y recepción en la app

Estado: servidor GATT cifrado de temperatura y pulso válido reintegrado al
firmware activo, compilado y cargado en COM16 el 8/11/2026. El monitor confirmó
el anuncio BLE, conexión cifrada, notificaciones, reconexión y los tres sensores;
la app mostró lecturas y las borró al desconectar. La persona confirmó que la
temperatura IR cambió en el reloj y el teléfono al variar la superficie apuntada.
Etapa 3 completada para el criterio de salida definido. Ver
`BLE_ETAPA3_2026_11_08.md` y docs/BLE_TEMPERATURA.md.

- Implementar advertising, servicio GATT, característica de lectura/notificación, conexión, desconexión y estado BLE real en connectivity/ble_manager.cpp.
- Alimentar notificaciones desde AppState/HealthSnapshot sin leer sensores desde la tarea BLE.
- En la app, conectar, suscribirse, validar mensajes, mostrar estado de cada sensor y caducidad de lecturas.
- App: números aleatorios retirados del botón de medición el 31/10/2026; sin lectura muestra "--" y caduca tras 5 s. Validar en teléfono cuando exista servidor BLE.
- Probar reconexión, teléfono fuera de alcance y reloj reiniciado.

Criterio de salida: una variación real del MLX se ve en pantalla del reloj y teléfono; una desconexión deja de mostrarla como lectura actual.

## Etapa 4 — Historial y sincronización

Estado al 8/11/2026: el reloj guarda hasta 10.080 muestras válidas, una por
minuto, en SPIFFS. La app persiste el historial local, recupera muestras tras
desconexión y sincroniza la hora del reloj por BLE. La persona confirmó la
prueba física de esta etapa. Ver `ETAPA4_HORA_HISTORIAL_2026_11_08.md`.

- Guardar lecturas válidas en almacenamiento local de la app con fecha, origen y calidad.
- Definir si se requiere recuperar lecturas tomadas mientras el teléfono estaba desconectado; las notificaciones en vivo por sí solas no recuperan el pasado.
- Sincronizar hora del reloj desde una fuente real y marcar si no está sincronizada.
- Limitar crecimiento de eventos y permitir revisar/exportar historial solo cuando sea necesario.

Criterio de salida: cerrar y abrir la app conserva el historial; lecturas duplicadas o viejas no aparecen como actuales.

## Etapa 5 — Vincha ESP32-C3 y ESP-NOW

Estado al 8/11/2026: receptor ESP-NOW cargado en la C3 y comunicación física
comprobada con el reloj. Se verificaron balizas, evento nuevo, ACK,
restablecimiento, tres intentos sin C3 y coexistencia con BLE. Las MAC y el
canal 1 coincidieron. El actuador permanece desconectado y sin GPIO de salida;
faltan las especificaciones de bobinas, la validación eléctrica del driver y
el diseño de actuación física. Ver `ETAPA5_RADIO_2026_11_08.md`.

- Ubicar o crear el proyecto ESP-IDF de la vincha, separado del reloj, según el esquema eléctrico real.
- Definir mensaje, destinatario, umbrales, confirmación, reintentos y comportamiento ante pérdida de enlace.
- Integrar ESP-NOW en el reloj y en la vincha; probar coexistencia con BLE en el ESP32-S3.
- El actuador de la vincha debe permanecer inactivo si faltan datos válidos o se pierde comunicación.

Criterio de salida: con datos de prueba controlados, la vincha responde una sola vez al evento previsto y queda inactiva ante falla o desconexión.

## Etapa 6 — Alertas, sueño, batería y energía

Estado: mayormente pendiente.

- Definir umbrales con criterios del proyecto, duración mínima, histéresis y confirmación. No presentar estimaciones como diagnóstico.
- App: el mensaje falso de "alerta enviada" y "ubicación compartida" se retiró el 31/10/2026; el envío real sigue pendiente.
- Sueño: el servicio actual solo entrega demo o ausencia de datos; desarrollar un algoritmo y validarlo por separado.
- Batería: sustituir porcentaje simulado por una lectura del circuito real, si el hardware la permite.
- Energía: medir consumo; hoy el ahorro solo atenúa backlight, no duerme el ESP32 ni apaga sensores.
- Priorizar seguridad y estabilidad cuando BLE y ESP-NOW estén activos juntos.

Criterio de salida: cada función muestra su estado real y tiene prueba física y registro de resultado.

## Mapa rápido del reloj

- main/app_main.cpp: arranque, inicialización y tareas.
- main/nera_config.h: pines, direcciones I2C, tamaños y frecuencias.
- main/sensors/real_sensors.cpp: bus I2C1, diagnóstico, MAX30102, MLX90614, MPU6050.
- main/core/app_state.h y .cpp: datos compartidos y banderas de validez.
- main/services/health_service.cpp: snapshot y caducidad de mediciones.
- main/services/sleep_service.cpp: estado de sueño.
- main/connectivity/ble_manager.cpp: GATT BLE cifrado y lecturas válidas.
- main/connectivity/espnow_diagnostic.cpp: balizas de radio de laboratorio.
- ../Codigo Vincha/main/main.c: receptor ESP-NOW sin salida hacia actuador.
- main/storage/storage_manager.cpp: ajustes e historial breve en NVS.
- main/power/power_manager.cpp: brillo y atenuación.
- main/ui/screens/watchface.cpp: aspecto y datos de Inicio.
- main/ui/ui_manager.cpp: navegación y actualización de pantallas.

## Mapa rápido de la aplicación

- lib/watch_ble_service.dart: escaneo, conexión, UUID y parser JSON.
- lib/nera_controller.dart: estado mostrado, simulaciones y eventos en memoria.
- lib/main.dart: navegación, pantallas y alerta de emergencia actualmente local.
- android/app/src/main/AndroidManifest.xml: permisos BLE Android.

## Refactorizaciones propuestas, todavía no aplicadas

1. Corregir comentarios y documentos desactualizados antes de tocar comportamiento.
2. Separar pruebas eléctricas y escaneo del bucle normal de sensores, manteniendo una forma simple de activarlos.
3. Cuando los sensores estén estables, dividir real_sensors.cpp por dispositivo sin cambiar la API pública de AppState.
4. Mantener un solo contrato de mensajes y pruebas de validez entre reloj y app.
5. Separar en la app presentación, persistencia y reglas de alerta sin rehacer su diseño.
