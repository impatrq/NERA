# Configuracion personal del prototipo NERA

La app Android permite elegir avisos de prueba, sensibilidad y actividad desde
Perfil. No detecta ansiedad ni selecciona limites medicos personalizados.
Actualización 26/11/2026: los avisos ahora pasan por la app antes de ordenar
GPIO5 en la vincha. La app muestra recepción y confirmación del C3. Se requiere
la app abierta y conectada. Ver [flujo actual](ALERTAS_LAB.md).
No hay envio automatico a contactos.

## Comportamiento

- Avisos activados: se evaluan las reglas de laboratorio del S3.
- Avisos desactivados: se pausan ambas reglas; sensores y BLE siguen midiendo.
- Sensibilidad alta/media/baja: condicion sostenida durante 5/10/20 segundos.
  No modifica los limites experimentales >100 BPM y >33 C de objeto IR, ni
  los de restablecimiento <90 BPM y <31 C. No son opciones clinicamente validadas.
- Cotidiano: habilita pulso y temperatura si los avisos estan activados.
- Ejercicio: pausa pulso, mantiene temperatura. Es manual y persiste; cambiar
  a Cotidiano al terminar. No distingue automaticamente deporte y ansiedad.
- Lecturas invalidas, simuladas o vencidas no cuentan para la duracion.
- Cambiar perfil reinicia la calificacion parcial. Una alerta ya activa puede
  mantenerse si sigue siendo elegible. Pausar una regla activa envia su
  inactivacion con el estado INVALID existente, para conservar compatibilidad
  con el C3 (su log antiguo dice dato invalido/vencido tambien en este caso).
- Los eventos siguen siendo de laboratorio. Confirmar ajustes es distinto de
  recibir el ACK de GPIO5, que ahora se muestra en «Alertas y vincha».

## Persistencia y sincronizacion

Android guarda preferencias, nombre/email y contactos en SharedPreferences
privadas (JSON version 1). commit se ejecuta en un hilo dedicado y la app
informa si falla. No se agregan librerias ni servicios de red.
Los contactos no se envian al reloj. No se guarda historial de mediciones aqui.

El S3 publica una caracteristica READ/WRITE cifrada:
7e400004-b5a3-f393-e0a9-e50e24dcca9e, en el servicio NERA existente.
Escritura exacta de cuatro bytes: [1, avisos 0/1, sensibilidad 0..2, ejercicio 0/1].
Lectura de cinco bytes: los cuatro actuales + guardado pendiente 0/1.
Valores o versiones fuera de rango se rechazan.
Una tarea independiente guarda NVS nera_alert/policy y solo entonces publica
el perfil atomico. No hay escrituras Flash desde callback BLE ni desde LVGL.
La app espera coincidencia completa y pendiente=0 antes de informar confirmado.
Un fallo de NVS conserva el perfil anterior y la app no confirma el nuevo.
Las peticiones se serializan; no se pisan silenciosamente escrituras en curso.

Sin telefono, el reloj conserva el ultimo perfil confirmado. La app informa
pendiente si se cambia offline y lo envia al conectar. Si falla, permite
reintentar. Los ajustes locales de esa app son la fuente al conectar; otro
telefono puede sustituir el perfil tras una vinculacion permitida.
Valores iniciales: avisos activados, alta (5 s), cotidiano, para conservar la
prueba anterior. Desinstalar la app o borrar datos elimina el guardado local.

## Prueba fisica

1. Actualizar S3 y APK. C3 no requiere cambios; bobina desconectada.
2. Abrir Perfil, conectar, comprobar Guardado y Confirmado por el reloj.
3. Cambiar a Media: sostener objeto IR >33 C, comprobar que no dispare a los
   5 s y que lo haga tras 10 s de muestras validas continuas.
4. Restablecer (<31 C), desactivar avisos y repetir: no debe activarse.
5. Activar avisos, Ejercicio: no debe activarse pulso; temperatura sigue activa.
   No hacer subir el pulso de una persona para probar; usar test de reglas.
6. Cerrar/reabrir app y reiniciar S3 por separado; comprobar persistencia.
7. Cambiar offline: debe mostrar pendiente hasta conectar y confirmar.
8. No confundir la confirmacion de configuracion BLE con un ACK de evento C3.

## Mapa educativo

- lib/user_preferences.dart: valores, formato local y cable BLE, contactos.
- lib/nera_controller.dart: carga, guardado serial, estados y sincronizacion.
- lib/main.dart, ProfileScreen: etiquetas y controles.
- lib/watch_ble_service.dart, applyPreferences: escritura/confirmacion BLE.
- Android MainActivity.kt: almacenamiento privado y commit fuera del hilo UI.
- main/connectivity/user_alert_preferences.h: opciones y duraciones MODIFICABLES.
- user_alert_preferences.cpp: NVS y atomicos FUNCIONALES.
- ble_manager.cpp, config_access: contrato de bytes; no modificar solo por estetica.
- espnow_diagnostic.cpp: aplica elegibilidad/duracion sin cambiar pines ni drivers.
- tests/lab_alert_test.cpp y test/user_preferences_test.dart: validacion.

## Archivos de esta etapa y lineas para capturas

Reloj (base: C:/Users/boca_/OneDrive/Documentos/GitHub/NERA Github/Codigo Reloj):

| Archivo | Linea/seccion |
|---|---|
| main/CMakeLists.txt | 24: registrar el modulo |
| main/connectivity/user_alert_preferences.h (nuevo) | 8: formato; 12: duraciones; 15: ejercicio |
| main/connectivity/user_alert_preferences.cpp (nuevo) | 19: guardado; 41: solicitud; 49: recuperacion |
| main/connectivity/ble_manager.cpp | 58: config_access; 236: caracteristica cifrada |
| main/connectivity/lab_alert_rules.h | 12: duracion configurable; 24: reinicio de calificacion |
| main/connectivity/espnow_diagnostic.cpp | 62: perfil; 91: elegibilidad y duracion |
| tests/lab_alert_test.cpp | preferences_test: configuraciones rechazadas, pausa, duracion |
| docs/CONFIGURACION_PERSONAL.md (nuevo) | esta guia tecnica y de prueba |

App (base: C:/Users/boca_/OneDrive/Escritorio/ESCUELA/aplicacion_nera):

| Archivo | Linea/seccion |
|---|---|
| lib/user_preferences.dart (nuevo) | 9: opciones; 55: almacenamiento |
| lib/nera_controller.dart | 63: carga; 82: guardado; 103: confirmacion; 307: sensibilidad |
| lib/watch_ble_service.dart | 330: applyPreferences |
| lib/main.dart | 452: Perfil; 465: sensibilidad; 482: actividad; showContactDialog: seleccion local |
| android/app/src/main/kotlin/com/example/aplicacion_nera/MainActivity.kt | 17: carga; 19: guardado en Android |
| test/user_preferences_test.dart (nuevo) | pantalla de 320 px, persistencia, errores y reconexion |

La compilacion tambien regenera los artefactos build. No se editaron GPIO,
drivers, sdkconfig, dependencias, ni firmware C3 en esta etapa.

Referencias de API: [permisos de caracteristicas cifradas de Espressif](https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/ble_get_started/nimble/NimBLE_Security/README.md)
y [guardado SharedPreferences de Android](https://developer.android.com/training/data-storage/shared-preferences).
