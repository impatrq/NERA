# NERA - Revision UI/UX y navegacion tactil
Fecha: 2026-09-09. Continuacion sobre la interfaz existente, sin rehacer drivers, sensores, almacenamiento ni arquitectura general.

## 1. Problemas encontrados
Las metricas del inicio no eran accesos tactiles; faltaba un acceso a sueno. La navegacion solo recorria pantallas horizontalmente, sin panel rapido ni historial de retorno. El resumen de Salud mostraba valores pequenos y el encabezado competia con ellos. Los ajustes tenian texto pequeno y algunas zonas tactiles quedaban demasiado cercanas. Faltaba coherencia entre unidades, datos ausentes y los estados visuales.

## 2. Decisiones de diseno
Se mantuvo LVGL 8 y se concentro el cambio en ui/. Fondo casi negro, valores blancos, etiquetas gris claro y un acento NERA mas moderado. No se agregaron imagenes, sombras pesadas, nuevas fuentes ni efectos de escalado.

Se creo una fila de metrica compartida por Inicio y Salud. Tiene dimensiones estables, valor alineado a la derecha y feedback de fondo al presionar. Se preservaron los indicadores de pagina y las flechas como alternativa al swipe horizontal.

## 3. Tipografia
Se conserva Montserrat, una sans serif disponible en el proyecto. No se introdujo una fuente cuadrada, de consola o futurista.
- Hora: 48 px, centrada, blanca. Ya estaba en 48 px; se mejoro su entorno y distribucion, no se afirma que haya aumentado.
- Valores rapidos: 28 px con unidades visibles, por ejemplo 72 BPM y 36.5 grados Celsius.
- Detalle de pulso/temperatura: cifra de 48 px y unidad de 20 px, alineadas.
- Sueno: duracion de 48 px, antes 28; duraciones secundarias de 20 px, antes 14.
- Menus: 20 px, antes 14; etiquetas y metadatos: 14 px, gris claro.
- Se usa el glifo de grados de la fuente existente, codificado como UTF-8 explicito.

Los 27 casos seleccionados de ancho pasaron con las tablas reales de las fuentes. Por ejemplo: 180 BPM ocupa 123 de 128 px disponibles y 12h 59m a 48 px ocupa 201 de 208 px. Valores excepcionales largos se recortan con puntos en campos de una sola linea para no invadir otros objetos.

## 4. Navegacion implementada
| Accion | Resultado |
| --- | --- |
| Swipe izquierda/derecha | Recorre Inicio, Salud, Pulso, Temperatura, Sueno y Ajustes |
| Tocar Pulso en Inicio o Salud | Abre el detalle cardiaco |
| Tocar Temp. en Inicio o Salud | Abre el detalle de temperatura |
| Tocar Sueno en Inicio o Salud | Abre el detalle de sueno |
| Swipe abajo | Abre Panel rapido |
| Swipe arriba | Abre Salud; no se invento una bandeja de notificaciones |
| Swipe arriba en Panel rapido | Vuelve a la pantalla de origen |
| Zona inferior Volver | Recupera el origen del acceso directo; desde el carrusel vuelve a Inicio |
| Engranaje inferior en Inicio | Abre Panel rapido sin necesidad del gesto vertical |
| Ajustes en Panel rapido | Abre los ajustes completos |

El panel rapido contiene brillo, ahorro y acceso a Ajustes. Comparte PowerManager y StorageManager con los ajustes completos; al entrar se sincronizan los controles, sin mover el slider durante un arrastre.

El historial de retorno tiene capacidad fija de ocho entradas. No crece indefinidamente ni asigna memoria por cada toque. Al recorrer el carrusel horizontal se descarta el historial de accesos y Volver apunta a Inicio.

Durante la transicion se ignoran acciones nuevas; un swipe espera la liberacion del dedo para que no se convierta tambien en un toque sobre la metrica. Los accesos responden a SHORT_CLICKED. En Ajustes, el scroll vertical conserva su funcion; sobre el slider se cambia brillo, no se navega. Para abrir el panel rapido desde esa pantalla, usar el encabezado libre. En el panel rapido los swipes horizontales no recorren el carrusel.

## 5. Inicio
Se conserva hora y fecha, pero las metricas pasan a tres filas tactiles de 208x46 px: pulso, temperatura y sueno. Separacion vertical de 6 px. La informacion del reloj queda arriba y los accesos debajo.

La banda superior separa NERA, indicador Bluetooth, estado de datos y bateria. Bluetooth es gris cuando no esta conectado y usa el acento cuando lo esta. La bateria muestra nivel; el simbolo de carga solo se usa en estado de carga. Demo sigue visible para no presentar muestras simuladas como reales.

La hora sigue mostrando --:-- hasta recibir una fecha/hora sincronizada. Esta revision no implementa una fuente horaria.

## 6. Cambios por pantalla
- Salud: tres filas tactiles de 58 px; valores de 28 px y ultima actualizacion compacta. Cada fila abre su detalle.
- Pulso: numero principal blanco y grande; unidad BPM separada y secundaria; estado de lectura, grafico y estadisticas conservados.
- Temperatura: misma jerarquia, unidad con grados y tendencia con unidad correcta; mantiene aclaracion no clinica.
- Sueno: duracion mas dominante, fases secundarias legibles, barras con paleta moderada y guiones cuando no hay datos.
- Ajustes: texto de 20 px, filas de 56 px y separacion entre areas de slider/switch. Conserva scroll, guardado, opciones futuras deshabilitadas e informacion del equipo.
- Panel rapido: nueva vista compacta, sin duplicar la logica de brillo/ahorro.
- Splash: conserva estructura y fuente; hereda el acento moderado del tema.

## 7. Archivos y lineas para capturas
La comparacion de esta tabla es contra los archivos leidos al comenzar ESTA tarea, no contra todo el trabajo anterior sin commitear. Los rangos incluyen hasta dos lineas de contexto entre cambios cercanos. Los enlaces abren el comienzo; la numeracion corresponde al cierre de esta revision.

| Archivo | Lineas nuevas/modificadas en esta tarea |
| --- | --- |
| [main/ui/ui_manager.cpp](C:/esp/NERA_backup/main/ui/ui_manager.cpp) | [1-3](C:/esp/NERA_backup/main/ui/ui_manager.cpp:1), [15-19](C:/esp/NERA_backup/main/ui/ui_manager.cpp:15), [30-31](C:/esp/NERA_backup/main/ui/ui_manager.cpp:30), [37-38](C:/esp/NERA_backup/main/ui/ui_manager.cpp:37), [42-57](C:/esp/NERA_backup/main/ui/ui_manager.cpp:42), [61-80](C:/esp/NERA_backup/main/ui/ui_manager.cpp:61), [84-108](C:/esp/NERA_backup/main/ui/ui_manager.cpp:84), [112-127](C:/esp/NERA_backup/main/ui/ui_manager.cpp:112), [132](C:/esp/NERA_backup/main/ui/ui_manager.cpp:132), [143](C:/esp/NERA_backup/main/ui/ui_manager.cpp:143), [152](C:/esp/NERA_backup/main/ui/ui_manager.cpp:152), [161](C:/esp/NERA_backup/main/ui/ui_manager.cpp:161) |
| [main/ui/screens/watchface.cpp](C:/esp/NERA_backup/main/ui/screens/watchface.cpp) | [1-2](C:/esp/NERA_backup/main/ui/screens/watchface.cpp:1), [10-21](C:/esp/NERA_backup/main/ui/screens/watchface.cpp:10), [26-30](C:/esp/NERA_backup/main/ui/screens/watchface.cpp:26), [42-55](C:/esp/NERA_backup/main/ui/screens/watchface.cpp:42) |
| [main/ui/screens/health_screen.cpp](C:/esp/NERA_backup/main/ui/screens/health_screen.cpp) | [11-18](C:/esp/NERA_backup/main/ui/screens/health_screen.cpp:11), [23-26](C:/esp/NERA_backup/main/ui/screens/health_screen.cpp:23), [30](C:/esp/NERA_backup/main/ui/screens/health_screen.cpp:30), [34-35](C:/esp/NERA_backup/main/ui/screens/health_screen.cpp:34) |
| [main/ui/screens/metric_screen.cpp](C:/esp/NERA_backup/main/ui/screens/metric_screen.cpp) | [11-17](C:/esp/NERA_backup/main/ui/screens/metric_screen.cpp:11), [39-44](C:/esp/NERA_backup/main/ui/screens/metric_screen.cpp:39), [49-50](C:/esp/NERA_backup/main/ui/screens/metric_screen.cpp:49) |
| [main/ui/screens/sleep_screen.cpp](C:/esp/NERA_backup/main/ui/screens/sleep_screen.cpp) | [9-16](C:/esp/NERA_backup/main/ui/screens/sleep_screen.cpp:9), [25](C:/esp/NERA_backup/main/ui/screens/sleep_screen.cpp:25), [31-40](C:/esp/NERA_backup/main/ui/screens/sleep_screen.cpp:31) |
| [main/ui/screens/settings_screen.cpp](C:/esp/NERA_backup/main/ui/screens/settings_screen.cpp) | [29](C:/esp/NERA_backup/main/ui/screens/settings_screen.cpp:29), [40-45](C:/esp/NERA_backup/main/ui/screens/settings_screen.cpp:40), [56-60](C:/esp/NERA_backup/main/ui/screens/settings_screen.cpp:56), [70-85](C:/esp/NERA_backup/main/ui/screens/settings_screen.cpp:70) |
| [main/ui/screens/watch_screens.h](C:/esp/NERA_backup/main/ui/screens/watch_screens.h) | [7-9](C:/esp/NERA_backup/main/ui/screens/watch_screens.h:7), [28](C:/esp/NERA_backup/main/ui/screens/watch_screens.h:28) |
| [main/ui/theme/nera_theme.h](C:/esp/NERA_backup/main/ui/theme/nera_theme.h) | [9-12](C:/esp/NERA_backup/main/ui/theme/nera_theme.h:9), [42-57](C:/esp/NERA_backup/main/ui/theme/nera_theme.h:42) |
| [tests/check_ui_text.mjs](C:/esp/NERA_backup/tests/check_ui_text.mjs) | [48-65](C:/esp/NERA_backup/tests/check_ui_text.mjs:48) |

No se cambiaron drivers ni servicios en esta revision. Las modificaciones anteriores que siguen en el working tree se conservaron.

## 8. Como probar
Compilacion ejecutada correctamente con .\\build.bat y ESP-IDF 6.0.2. Binario final: 0xa1760 bytes, con 84% libre en la particion de aplicacion de 4 MiB.

Prueba reproducible de texto desde C:/esp/NERA_backup:
```powershell
node tests/check_ui_text.mjs
```
Resultado: 27 casos aprobados. git diff --check termino sin errores de whitespace, con advertencias locales de conversion LF/CRLF.

La primera compilacion detecto dos nombres de API no disponibles en LVGL 8; se reemplazaron por lv_slider_is_dragged y comparacion mediante lv_color_to32. La compilacion final fue exitosa. Persisten advertencias del SDK/LVGL ya existentes; no se ocultaron globalmente.

### Pruebas pendientes en la placa
1. Flashear con ESP-IDF usando el puerto real: idf.py -p COMx flash monitor. COMx es un marcador, no un puerto detectado.
2. En Inicio, tocar cada metrica y pulsar Volver. Debe retornar a Inicio.
3. Abrir Salud con swipe arriba, tocar Pulso y volver. Debe retornar a Salud, no saltar a Inicio.
4. Abrir Panel rapido con swipe abajo desde distintas paginas y cerrarlo con swipe arriba o Volver.
5. Cambiar brillo/ahorro en Panel rapido, abrir Ajustes y comprobar que ambos muestran el mismo estado.
6. Arrastrar el slider y desplazar Ajustes verticalmente. No debe dispararse otro acceso ni una pagina accidental.
7. Recorrer el carrusel, probar sus extremos y repetir swipes rapidos. Verificar que una sola accion no abra dos pantallas.
8. Verificar tipografia, margenes, feedback al presionar y ausencia de recortes en el LCD 240x320.
9. Esperar guardado y reiniciar para comprobar que las preferencias siguen persistiendo.

No se flasheo ni se ejecutaron gestos en hardware. No hay captura renderizada de LVGL, medicion de FPS ni de consumo. Los tests de ancho no prueban layout completo, deteccion tactil o transiciones. La prueba final en el reloj sigue siendo necesaria.

## 9. Capturas para Trello
### Avance UX-1: jerarquia y componentes
Descripcion: se reorganizaron valores, unidades y etiquetas con un componente de fila compartido.
Objetivo: lectura rapida en 240x320.
Trabajo: paleta moderada, filas tactiles, valores de 28/48 px, menus de 20 px y feedback al presionar.
Archivos: tema, watchface, health_screen, metric_screen y sleep_screen, enlazados arriba.
Resultado: sistema visual coherente integrado al firmware.
Pruebas: compilacion y casos de ancho; visual fisica pendiente.
Capturas recomendadas: codigo de metric_row, inicio completo y Salud; detalles de pulso/temperatura/sueno.
Explicacion sencilla: los numeros importantes se leen primero y las etiquetas dejan de competir.
Pendiente: ajuste optico sobre fotos reales del LCD.

### Avance UX-2: acceso directo y retorno
Descripcion: se agregaron destinos tactiles y retorno al origen, conservando el carrusel.
Objetivo: reducir los pasos para consultar una metrica y volver.
Trabajo: SHORT_CLICKED, historial fijo, swipe vertical y proteccion durante transiciones.
Archivos: ui_manager y watch_screens.
Resultado: rutas de navegacion compiladas.
Pruebas: revision de callbacks y compilacion; gestos fisicos pendientes.
Capturas recomendadas: open/back/gesture y un video Inicio -> Pulso -> Volver; Salud -> Sueno -> Volver.
Explicacion sencilla: entrar a un detalle no obliga a recorrer todas las pantallas para regresar.
Pendiente: validar swipes rapidos y umbrales tactiles con la placa.

### Avance UX-3: panel rapido y ajustes
Descripcion: se incorporo acceso rapido al brillo y ahorro usando la logica existente.
Objetivo: cambiar ajustes frecuentes con pocas acciones.
Trabajo: nueva pagina Quick, controles sincronizados y areas tactiles separadas.
Archivos: settings_screen y ui_manager.
Resultado: panel y ajustes completos conectados al mismo estado.
Pruebas: compilacion y textos; PWM/guardado en placa pendientes.
Capturas recomendadas: Panel rapido, Ajustes y sincronizacion del porcentaje de brillo entre ambos.
Explicacion sencilla: hay dos accesos al mismo ajuste, no dos configuraciones independientes.
Pendiente: medir interaccion real y conservar evidencia de reinicio.

## 10. Pendientes
Validar visual e interaccion sobre el dispositivo, ajustar gestos con datos reales de uso y medir memoria libre con la septima vista creada. No se agrego un simulador de LVGL ni input alternativo de hardware: las flechas y el engranaje siguen requiriendo touch operativo.

Hora real, BLE/GATT, notificaciones, vibracion y sensores medicos no se implementaron en esta revision de UI. Los datos demo siguen marcados. Una tipografia personalizada puede evaluarse luego con muestras visuales y presupuesto de memoria; no se incorporo una fuente nueva sin evidencia de que mejorara el resultado.
