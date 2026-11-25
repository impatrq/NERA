# ESP-NOW restaurado sobre el firmware actual

El S3 conserva sensor_service, el estimador PBA y la pantalla actuales.
La radio usa snapshots; no consulta I2C ni modifica objetos LVGL.
Se inicia desde lvgl_task despues de crear interfaz, reservar memoria y
procesar el primer dibujo. Conserva pool LVGL PSRAM y buffers Wi-Fi acotados.
CONFIG_NERA_ESPNOW_DIAGNOSTIC=y debe estar activo (lo esta en sdkconfig actual).

## Compatibilidad y prueba

- Canal 1, sin router. S3 emisor 28:84:85:47:BC:98.
- Destino C3 30:ED:A0:65:F9:68. Actualizar constantes si se reemplazan placas.
- Baliza v1 cada 5 s, compatible con ../Codigo Vincha existente.
- Alertas v2 con id/sesion, confirmacion de aplicacion, hasta 3 envios separados
  por 1 s. Un nuevo estado de la metrica sustituye un pendiente anterior.
- No hay cifrado/autenticacion criptografica de estos mensajes de laboratorio.
  La lista MAC no equivale a autenticacion. No se habilitan GPIO ni bobina.
- El C3 no necesita cambios si ya tiene el receptor de balizas y ALERTAS LAB + ACK.

Flashear S3 desde Codigo Reloj. Encender el C3 con su firmware propio y abrir
ambos monitores. S3 muestra ESP-NOW SENSORES V3 y Baliza encolada; C3 debe
mostrar Baliza recibida. El encolado no confirma recepcion.
Para un evento, C3 imprime ALERTA LAB y ACK LAB; S3 confirma ACK C3 CONFIRMADO.
Si no llega, S3 registra SIN CONFIRMACION C3 despues de los intentos acotados.

## Reglas recuperadas, solo laboratorio

Pulso >100 BPM o temperatura de objeto IR >33 C. Restablecer <90 BPM / <31 C.
Por defecto duracion 5 s; recuperar ajustes guardados puede cambiarla a 10/20 s,
pausar avisos o pausar solo pulso en Ejercicio. Ver Perfil inicial en el log.
No son limites medicos, no identifican ansiedad ni determinan temperatura central.
No se restauro BLE ni se modifico la app en esta etapa.

Solo se consideran lecturas validas, finitas, recientes (hasta 3 s).
Un BPM retenido conserva su fecha: repetirlo no completa el tiempo requerido.
No hay datos demo en este servicio. Datos invalidos/vencidos inactivan la regla.
No hay recuperacion de eventos historicos tras apagar/reiniciar las placas.

## Archivos

- main/CMakeLists.txt: fuentes condicionales y dependencias de radio explicitas.
- main/lvgl/lvgl_task.cpp: inicializacion despues de dibujar interfaz.
- main/connectivity/espnow_diagnostic.cpp: adaptacion al SensorSnapshot actual,
  carga de preferencias, beacons, eventos y ACK.
- main/connectivity/user_alert_preferences.cpp: reutilizado sin modificar.
- ../Protocolo Reloj Vincha/nera_lab_protocol.h y ../Codigo Vincha/main/main.c: protocolo/receptor sin cambios.
- tests/lab_alert_test.cpp: duracion, histeresis, datos repetidos, formato y perfil.
