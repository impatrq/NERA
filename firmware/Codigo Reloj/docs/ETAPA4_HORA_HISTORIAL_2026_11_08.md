# Etapa 4: hora e historial de siete días

## Implementado

- La app envía hora UTC y desplazamiento horario local al conectar por BLE.
- El reloj muestra `HH:MM` tras sincronizar; antes muestra `--:--`.
- El reloj registra una muestra válida por minuto de pulso y temperatura de
  objeto IR en SPIFFS, con una capacidad circular de 10.080 muestras.
- La app solicita el historial por secuencia al reconectar, lo muestra y lo
  guarda en almacenamiento interno del teléfono.
- Las muestras sin hora de este mismo arranque se fechan al sincronizar. Si el
  reloj se apagó antes de sincronizar, esas muestras quedan sin hora confiable.

## Comprobaciones realizadas

- Firmware ESP-IDF compilado con éxito tras agregar hora e historial.
- Todas las pruebas Flutter pasaron, incluido el protocolo y la persistencia.
- APK Android de depuración compilado con éxito, incluida la capa nativa Kotlin.
- La primera carga de firmware terminó correctamente, pero la partición SPIFFS
  original de 12 MB demoró la inicialización. Se redujo a 1 MB y se volvió a
  cargar el firmware y la tabla de particiones por COM15; la escritura y el
  hash se verificaron.
- Tras soltar BOOT y pulsar RESET, el reloj volvió a encender y reapareció en
  COM16. El registro serie confirmó `Historial listo; ultimo=0,
  capacidad=10080 minutos`, `Servidor GATT listo` y `Anunciando NERA Watch`.
- El teléfono no autorizó ADB. El APK se compiló para instalación manual.
- Posteriormente, la persona confirmó que la etapa 4 funcionó en la prueba
  física, incluida la sincronización y recuperación del historial.

## Estado

Etapa 4 confirmada por la persona. El APK compilado está en
`aplicacion_nera/build/app/outputs/flutter-apk/app-debug.apk`.

El diseño y formato BLE están en `CONTRATO_DATOS_RELOJ_APP_V1.md`.
