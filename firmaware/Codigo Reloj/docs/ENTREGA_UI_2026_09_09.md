# NERA: interfaz y consolidacion del firmware
Fecha: 2026-09-09. Proyecto: C:/esp/NERA_backup.
Esta entrega continua el firmware existente. No es una certificacion de producto terminado.

## 1. Resumen general
Se mantuvieron ESP-IDF, LVGL 8, FreeRTOS, los drivers del proyecto y sus sensores mock. Se reorganizo la interfaz en tema, navegacion y vistas separadas: inicio NERA, caratula, salud, pulso, temperatura, sueno y ajustes.

La direccion visual utiliza negro neutro, texto claro, acentos diferenciados por metrica, cifras grandes y controles tactiles estables. Es una interpretacion propia para un LCD rectangular 240x320, no una copia de Apple Watch o Galaxy Watch.

El brillo y ahorro se conectaron al PWM existente. Los historiales se guardan fuera de la tarea grafica, tambien cuando llegan a 60 muestras. Las funciones sin hardware o protocolo implementado no se presentan como operativas.

## 2. Que encontre
- Ya habia una arquitectura modular, LVGL integrado, touch, splash, seis vistas, mocks de pulso/temperatura/bateria, servicios y persistencia NVS.
- Gran parte del contenido de las pantallas seguia concentrada en ui_manager.cpp.
- Ajustes mostraba informacion, pero no ofrecía controles completos para cambiar brillo/ahorro.
- El guardado dependia de que creciera el contador; al saturar las 60 muestras dejaba de actualizar el historial persistente.
- La finalizacion de dibujo se notificaba sin esperar el fin real de la transferencia SPI.
- Habia opciones NERA en Kconfig.projbuild raiz que no estaban incorporadas como configuracion de componente.
- BLE tenia una estructura preparatoria, no una conexion real con telefono.

### Integridad respecto de la referencia disponible
Git contiene el commit 9555977. La comprobacion de archivos versionados eliminados no encontro ninguno; el proyecto actual compila y enlaza. No se puede asegurar la recuperacion de archivos que nunca se hayan versionado, ni reconstruir una version anterior desconocida. No se hizo reset, checkout, limpieza de proyecto ni borrado de NVS.

### Hardware identificado en el codigo
| Elemento | Configuracion encontrada |
| --- | --- |
| MCU | ESP32-S3 |
| LCD | 240x320, vertical, controlador ST7789 |
| Bus LCD | SPI2, 40 MHz |
| LCD GPIO | MOSI 38, SCLK 39, DC 42, CS 45, reset -1 |
| Backlight | GPIO 1, LEDC 5 kHz, 8 bits |
| Touch | CST816D, I2C 0x15, SDA 48, SCL 47, 400 kHz |
| Touch reset/INT | -1 en el proyecto |
| IMU | Direccion 0x6B declarada; no equivale a driver implementado |
| Flash | Configurada en 16 MB; particion de aplicacion 4 MB |
| PSRAM | Octal a 80 MHz configurada; capacidad fisica no medida en esta sesion |
| Dibujo | RGB565; dos buffers de 20 lineas, 9600 bytes cada uno |
| Heap LVGL | 128 KiB; fuente Montserrat de 48 habilitada |

Son datos de configuracion y codigo, no una comprobacion electrica de la placa. Se conserva la orientacion existente; falta verificar los cuatro extremos del touch en el dispositivo.

## 3. Que mejore y por que
### Arquitectura grafica
Se conserva LVGL porque el proyecto ya tiene su puerto de display/input y porque sus widgets, invalidacion, fuentes, graficos, timers y eventos cubren las necesidades del reloj. Cambiar de motor sumaria una migracion de drivers y componentes sin resolver una limitacion demostrada.

UIManager se ocupa de cambiar de pagina. Cada vista crea sus objetos una vez y recibe datos de HealthService. El tema concentra colores y primitivas. Las flechas miden 44x44 px; los gestos horizontales usan espera de liberacion y bloqueo durante la transicion para evitar saltos multiples.

Solo se actualiza el contenido de la pagina seleccionada. Las etiquetas no se reasignan si el texto es igual, y los graficos comparan sus muestras antes de modificarse. El splash usa un timer, no una espera que detenga las tareas.

### Datos y persistencia
HealthService calcula minimo, promedio y maximo del historial de pulso; la tendencia de temperatura usa las dos ultimas muestras. AppState filtra valores no finitos y lecturas invalidas antes de incorporarlas al historial.

Se mantiene el formato anterior de los blobs de historial y las claves de demo. Los ajustes se serializan como cuatro bytes validados. La cola de ajustes conserva la solicitud mas reciente; la tarea de almacenamiento realiza el commit. Los historiales se comparan por contenido una vez por minuto.

El namespace nera conserva los historiales anteriores, generados por mocks. nera_real queda separado para futuros datos reales, evitando presentar la demo anterior como medicion. Ante NVS incompatible o sin espacio no se borra automaticamente la particion.

### Display, input y energia
El driver espera el semaforo de fin de transferencia antes de devolver el buffer a LVGL. Esto evita reutilizar memoria mientras DMA todavia la consume. Es una transferencia serializada, no una demostracion de double buffering completamente solapado. La espera cede CPU, pero depende de que llegue el callback del hardware.

Se prueba la presencia del touch y se descartan coordenadas fuera del LCD. El loop LVGL duerme al menos un tick y limita su espera a 20 ms. El brillo elegido se conserva separado del brillo efectivo atenuado; al recibir actividad vuelve al nivel correspondiente. El timeout configurado es 30 segundos.

## 4. Mapa de archivos
Todos los siguientes enlaces apuntan al proyecto local:
- [Tema](C:/esp/NERA_backup/main/ui/theme/nera_theme.h): paleta y primitivas LVGL.
- [Contrato de vistas](C:/esp/NERA_backup/main/ui/screens/watch_screens.h): paginas y referencias a objetos.
- [Navegacion](C:/esp/NERA_backup/main/ui/ui_manager.cpp): inicializacion, gestos, flechas, transiciones y refresco.
- [Splash](C:/esp/NERA_backup/main/ui/screens/splash_screen.cpp): inicio NERA.
- [Caratula](C:/esp/NERA_backup/main/ui/screens/watchface.cpp): hora, fecha y metricas.
- [Salud](C:/esp/NERA_backup/main/ui/screens/health_screen.cpp): resumen y ultima actualizacion.
- [Pulso y temperatura](C:/esp/NERA_backup/main/ui/screens/metric_screen.cpp): graficos y estadisticas.
- [Sueno](C:/esp/NERA_backup/main/ui/screens/sleep_screen.cpp): duraciones demo.
- [Ajustes](C:/esp/NERA_backup/main/ui/screens/settings_screen.cpp): slider, ahorro y opciones deshabilitadas.
- [HealthService](C:/esp/NERA_backup/main/services/health_service.cpp): snapshot y calculos.
- [SleepService](C:/esp/NERA_backup/main/services/sleep_service.cpp): resumen simulado; no analiza IMU.
- [Estado](C:/esp/NERA_backup/main/core/app_state.cpp): datos compartidos protegidos.
- [Almacenamiento](C:/esp/NERA_backup/main/storage/storage_manager.cpp): validacion, carga y guardado.
- [Energia](C:/esp/NERA_backup/main/power/power_manager.cpp): politica de brillo.
- [Display](C:/esp/NERA_backup/main/display/display_driver.cpp): transferencia SPI y finalizacion DMA.
- [Touch](C:/esp/NERA_backup/main/touch/touch_driver.cpp): adquisicion I2C.
- [Arranque](C:/esp/NERA_backup/main/app_main.cpp): inicializacion y tareas.
- [Opciones NERA](C:/esp/NERA_backup/main/Kconfig.projbuild): incorpora el Kconfig raiz.
- [Configuracion reproducible](C:/esp/NERA_backup/sdkconfig.defaults): fuentes y heap.
- [Prueba de textos](C:/esp/NERA_backup/tests/check_ui_text.mjs): anchos con fuentes del proyecto.
- [Lineas para capturas](C:/esp/NERA_backup/docs/LINEAS_UI_2026_09_09.md): bloques modificados y nuevos.
- [Avances para Trello](C:/esp/NERA_backup/docs/TRELLO_UI_2026_09_09.md): trabajo separado por avances.

## 5. Como compilar
En la terminal ESP-IDF de VS Code, ubicada en C:/esp/NERA_backup:
```powershell
idf.py reconfigure
idf.py build
```
La reconfiguracion es importante despues de incorporar main/Kconfig.projbuild. No hace falta reiniciar el proyecto ni borrar build.

En esta instalacion tambien se puede ejecutar:
```powershell
.\build.bat
```
El script existente utiliza ESP-IDF 6.0.2 y herramientas instaladas bajo C:/Espressif. No usar su opcion clean para esta actualizacion.

Verificar en menuconfig que esten habilitadas las fuentes Montserrat 20, 28 y 48 y el heap LVGL de 128 KiB. sdkconfig.defaults aporta defaults a configuraciones nuevas; no siempre reemplaza valores previamente elegidos en sdkconfig.

## 6. Como flashear
Con la placa conectada, seleccionar su puerto real en VS Code y usar ESP-IDF: Flash Your Project. Alternativamente, en la terminal ESP-IDF:
```powershell
idf.py -p COMx flash monitor
```
COMx es un marcador: reemplazar por el puerto que Windows asigne a la placa. Salir del monitor con Ctrl+]. No se ejecuto flash, erase-flash ni monitor sobre una placa en esta sesion.

El binario esta en [NERA.bin](C:/esp/NERA_backup/build/NERA.bin). No cargar solamente este archivo en una direccion improvisada: usar el comando de ESP-IDF, que incluye la tabla de particiones y bootloader correctos.

## 7. Como probar
### Verificado en esta sesion
- Compilacion y enlace ESP-IDF 6.0.2 para ESP32-S3: correctos.
- Binario: 0xa0fe0 bytes, aproximadamente 644 KiB; 84% libre en la particion de 4 MiB.
- Informe de enlace: DIRAM estatica 212044 bytes; remanente de region 129716 bytes. Esto NO es el heap libre despues de arrancar.
- 19 casos de ancho de texto aprobados con las tablas Montserrat del repositorio.
- git diff --check sin errores de whitespace; advertencias locales LF/CRLF.
- Ningun archivo versionado eliminado respecto de 9555977.

Prueba reproducible de fuentes, desde la raiz:
```powershell
node tests/check_ui_text.mjs
```
Esta prueba comprueba avances de glifos y kerning de textos seleccionados. No ejecuta LVGL, no verifica todos los estados posibles ni reemplaza una captura del LCD.

### Pendiente: lista para la placa
1. Arrancar: ver NERA y transicion a caratula; revisar logs de display, touch y memoria.
2. Recorrer las seis paginas con flechas y swipes. Repetir gestos rapidos; verificar que no salte dos paginas.
3. Verificar extremos tactiles, ausencia de recortes y que el slider no provoque navegacion.
4. En Ajustes, cambiar brillo, soltar y esperar el guardado. Reiniciar y comprobar restauracion.
5. Activar ahorro, modificar brillo, desactivar ahorro. Debe volver al brillo elegido; tras 30 s sin actividad debe atenuarse.
6. Mantener encendido mas de dos minutos con mocks. Reiniciar y comprobar que ambos historiales conservan muestras recientes aun con contador 60.
7. Confirmar que los valores se identifican como demo. Sueno muestra 6 h 24 min dormido; 48 min despierto se muestran por separado.
8. Deshabilitar mocks, recompilar y comprobar ausencia de datos reales inventados. Esta variante no se compilo ni ejecuto en esta sesion.
9. Probar NVS invalida mediante un entorno de prueba respaldado, nunca borrando datos del usuario para simular errores.
10. Medir FPS, latencia, RAM minima, stacks y consumo durante uso prolongado.

No se obtuvo una captura de LVGL ni se midieron FPS, consumo, estabilidad fisica o latencia tactil. El compilador local investigado no dispone de destino host/wasm para ejecutar el motor fuera de la placa.

## 8. Que es real y que es simulado
| Categoria | Estado |
| --- | --- |
| LCD, touch, PWM | Implementaciones para hardware real; nueva version pendiente de prueba fisica |
| Navegacion, graficos, ajustes | Codigo LVGL compilado; interaccion pendiente de placa |
| NVS | Persistencia real implementada; reinicio/corrupcion pendientes de prueba |
| Pulso, temperatura, bateria | Mocks existentes; no lecturas medicas ni bateria medida |
| Sueno | Resumen demo fijo de SleepService; no deteccion de fases |
| Hora y fecha | Vista preparada para datetime sincronizado; muestra --:-- sin sincronizacion |
| BLE | Interfaz preparatoria existente; sin GATT ni sincronizacion con smartphone |
| Vibracion y notificaciones | Controles deshabilitados; sin actuador ni canal implementado |
| Ahorro | Atenuacion del backlight; no deep sleep ni autonomia medida |

No hay una fuente de hora real conectada en esta entrega. Se prefirio mostrar su ausencia antes que presentar el tiempo de arranque o una fecha fija como hora correcta. Integrar RTC, ajuste manual o sincronizacion validada es un pendiente funcional importante.

## 9. Problemas reales encontrados
- Configuracion NERA ausente del build: genero errores de compilacion para mocks y timeout; resuelto incorporando el Kconfig del componente y reconfigurando.
- Guardado detenido al saturar el historial: corregida la condicion para comparar contenido.
- Fin de flush adelantado: agregado handshake de fin DMA.
- Cola de eventos sin consumidor util: la UI ahora la drena de forma acotada y consulta el estado consolidado.
- Versiones antiguas de comentarios/logs anunciaban capacidades o etapas que no correspondian: corregidos los relevantes para este avance.
- El SDK emite advertencias por I2C legacy EOL, valores de Kconfig y enumeraciones LVGL/C++26. La compilacion final no falla, pero se recomienda migrar I2C por separado y validar fisicamente.
- Git del SDK presenta advertencias de propietario bajo el sandbox; no se modifico la configuracion global para ocultarlas.
- Falta validacion visual ejecutada: las comprobaciones de fuentes no prueban colores, transiciones, clipping de objetos o alineacion tactil.

## 10. Proximos pasos
Prioridad inmediata: flashear y registrar capturas de las siete vistas, validar touch/PWM/NVS y medir memoria real. Luego incorporar una fuente de hora, definir sensores concretos de salud/bateria e implementar sus drivers. BLE requiere perfil GATT, protocolo, seguridad y pruebas con telefono antes de habilitar sus controles. La medicion de sueno necesita adquisicion y validacion propias; no basta con una IMU declarada.

Conviene ampliar pruebas unitarias de AppState, HealthService y StorageManager con dependencias simuladas y pruebas de reinicio en placa. Mantener el historial de demo separado del real y agregar versionado explicito de formato antes de una migracion incompatible.

### Conceptos para leer el codigo
- Task: tarea FreeRTOS con su propio stack; varias pueden avanzar sin que la UI lea sensores directamente.
- Queue: cola que copia mensajes entre tareas. En ajustes se conserva el ultimo valor solicitado.
- Mutex: protege la copia y modificacion del estado compartido.
- Timer LVGL: callback ejecutado por el loop grafico, no otra tarea que pueda tocar widgets libremente.
- SPI: bus del LCD; DMA mueve los pixeles y un semaforo indica cuando deja de usar el buffer.
- I2C: bus direccionado usado por el touch.
- GPIO: pin del microcontrolador; LEDC genera el PWM del backlight.
- Backlight: iluminacion del LCD; bajarla no equivale a dormir el procesador.
- NVS: almacenamiento persistente por claves en Flash; los commits se mantienen fuera de callbacks graficos.
- Mock: fuente simulada para probar presentacion y flujo, nunca una medicion del usuario.
- Servicio: adapta datos del estado para las vistas sin acoplarlas a drivers.
