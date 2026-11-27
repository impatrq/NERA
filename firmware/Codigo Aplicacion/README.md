# NERA — aplicación móvil

Aplicación Flutter del proyecto NERA. Se conecta por Bluetooth Low Energy al
reloj, muestra las lecturas que este publica, sincroniza hora e historial y
presenta los estados de las alertas de laboratorio dirigidas a la vincha.
La entrada actual del proyecto es lib/main.dart.

## Requisitos

- Flutter con una versión de Dart compatible con ^3.11.5, según pubspec.yaml.
- Para comprobar BLE: teléfono Android compatible, Bluetooth habilitado y
  permisos solicitados por la aplicación.
- Reloj NERA con el firmware compatible. El flujo de alertas requiere la versión
  v3 en reloj, vincha y aplicación.

## Ejecutar

Abrir esta carpeta como proyecto Flutter y ejecutar:

    flutter pub get
    flutter run

Para repetir la resolución de las dependencias se conserva pubspec.lock.
Las demás carpetas de plataforma pertenecen al proyecto Flutter; su presencia no
significa que se hayan validado todas las plataformas con el hardware.

## Conectar el reloj

1. Encender el reloj con su firmware correspondiente.
2. Habilitar Bluetooth en el teléfono y aceptar los permisos de dispositivos
   cercanos o ubicación que solicite Android según su versión.
3. Buscar el dispositivo anunciado como NERA Watch desde la aplicación y conectarlo.
4. Confirmar el emparejamiento cuando el sistema lo solicite.
5. Comprobar el estado de conexión y la llegada de lecturas reales.

Cuando una lectura no es válida o deja de actualizarse, la aplicación debe
mostrar su ausencia y no presentarla como una medición vigente.

## Alertas y confirmaciones

El recorrido es: reloj → app por BLE → reloj → vincha por ESP-NOW → confirmación.
El teléfono no usa ESP-NOW directamente.

| Estado visible | Significado |
| --- | --- |
| Alerta recibida | La app recibió el evento publicado por el reloj. |
| Esperando confirmación | Se está esperando la confirmación de la vincha. |
| Vincha confirmó GPIO5 | El firmware de la vincha confirmó que aplicó la orden al periférico. |
| Sin confirmación | La entrega falló o dejó de estar vigente. |

La confirmación de GPIO5 no mide la corriente ni el funcionamiento de las bobinas.
Los umbrales y estimaciones pertenecen a un prototipo de laboratorio.

## App en segundo plano y desconexión

El flujo requiere mantener la app abierta, en primer plano, conectada y suscrita.
Al pasar a segundo plano se suspenden las renovaciones y se limpian los estados
visibles. El reloj y la vincha aplican sus tiempos de vencimiento.
Esta versión no incorpora un servicio de alertas en segundo plano.
Los avisos de la interfaz no equivalen a mensajes enviados a familiares.

## Pruebas

    flutter analyze --no-fatal-infos lib test
    flutter test

Las pruebas de software cubren el formato de mensajes, los estados de entrega,
la caducidad, los duplicados y la interfaz. La recepción física y las condiciones
de la etapa de potencia se comprueban por separado con el prototipo.
El análisis anterior se limita al código activo y muestra los avisos informativos
por APIs obsoletas sin tratarlos como errores. El análisis de toda la carpeta
incluye también las referencias históricas main.dart y app_theme.dart de la raíz,
que actualmente presentan errores de compatibilidad e imports. Esos archivos se
conservan y no son la entrada de la aplicación actual.

## Mapa del código

| Ruta | Contenido |
| --- | --- |
| lib/main.dart | Arranque, navegación y cambios de primer/segundo plano. |
| lib/watch_ble_service.dart | Conexión BLE, lecturas y transporte de alertas. |
| lib/watch_alert.dart | Mensajes de alerta y estados de entrega. |
| lib/nera_controller.dart | Estado de la aplicación y vencimiento de avisos. |
| lib/screens/ | Pantallas de la aplicación. |
| lib/watch_history.dart y lib/watch_history_store.dart | Historial y persistencia. |
| test/ | Pruebas automatizadas. |

Consultar también [GUIA_CODIGO.md](GUIA_CODIGO.md). Los archivos main.dart y
app_theme.dart que permanecen en la raíz son referencias anteriores conservadas;
el proyecto actual usa sus archivos dentro de lib/.
