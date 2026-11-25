# Codigo de NERA

- [Codigo Reloj](Codigo%20Reloj/): proyecto ESP-IDF para ESP32-S3.
- [Codigo Vincha](Codigo%20Vincha/): proyecto ESP-IDF independiente para ESP32-C3.
- [Codigo Aplicacion](Codigo%20Aplicacion/): proyecto Flutter completo. Su entrada es lib/main.dart.
- [Protocolo Reloj Vincha](Protocolo%20Reloj%20Vincha/): formato de mensajes utilizado por ambos dispositivos.

El reloj y la vincha se compilan abriendo cada uno su propia carpeta.
El protocolo se mantiene en la carpeta hermana indicada arriba; conservar esta estructura al descargarlos.

## Archivos anteriores conservados

La carpeta Aplicacion ya existia en el repositorio y se conserva.
Los archivos main.dart y app_theme.dart en la raiz de Codigo Aplicacion se conservan como referencias anteriores.
La entrada del proyecto Flutter preparado es Codigo Aplicacion/lib/main.dart.
Reloj.c y los sensores anteriores del repositorio tambien se conservan; la lista activa de compilacion del reloj esta en main/CMakeLists.txt.

## Alcance de esta preparacion

Se copian las fuentes actuales y se ajustan las rutas necesarias para separar los proyectos.
No se modifican el funcionamiento de los sensores, BLE, ESP-NOW, GPIO ni la configuracion de LVGL.
Los archivos generados, configuraciones locales y herramientas instaladas en cada computadora no forman parte de esta entrega.
