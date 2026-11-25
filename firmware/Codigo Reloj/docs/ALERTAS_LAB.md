# Alertas de laboratorio S3 -> C3, protocolo v2

Solo prueba funcional, sin actuador. No son limites medicos.

| Metrica | Activacion | Restablecimiento |
|---|---|---|
| Pulso | >100 BPM durante 5 s | <90 BPM |
| Temperatura de objeto IR | >33 C durante 5 s | <31 C |

Reglas independientes (OR). Igualar el limite de activacion no dispara.
Una lectura invalida, vencida (>3 s) o simulada rompe la continuidad.
Si ya habia alerta, se transmite su inactivacion. No se evalua SpO2 ni sueno.
La duracion se calcula con fechas de muestras, no repitiendo un dato almacenado.

El reloj envia eventos unicast a 30:ED:A0:65:F9:68. El C3 acepta el reloj
28:84:85:47:BC:98. Si se reemplaza una placa hay que actualizar esas constantes.
Canal 1. Cada evento tiene sesion aleatoria por arranque, id, metrica, estado
y valor en centesimas. Hasta 3 envios, separados por 1 s, esperando ACK.
Sin ACK se informa el fallo y no se afirma que la alerta llego.
Un estado nuevo reemplaza el evento pendiente anterior de esa metrica.
No hay recuperacion de eventos historicos ni persistencia tras reiniciar.

El C3 imprime eventos nuevos, confirma tambien duplicados y nunca configura
GPIO de salida. ACK significa recepcion por la aplicacion C3, no actuacion.
Los mensajes no estan cifrados ni autenticados criptograficamente. La lista
de MAC es un filtro de laboratorio, no una medida de autenticacion segura.

## Prueba

1. Compilar/flashear ambos proyectos por separado: Codigo Reloj (S3), ../Codigo Vincha (C3).
2. Mantener bobina desconectada y C3 alimentado solo por USB.
3. Comprobar balizas. Acercar un objeto templado al MLX para mantener >33 C
   durante al menos 5 s de lecturas validas. Retirarlo y esperar <31 C.
4. C3: ALERTA LAB ACTIVA, ACK LAB. S3: ACK C3 CONFIRMADO.
5. Retirar/desconectar solo con alimentacion apagada si se cambia cableado.
   Perder lectura valida debe romper la duracion y/o inactivar el evento.
6. Con C3 apagado, provocar una transicion: S3 debe registrar SIN CONFIRMACION
   tras tres intentos. Al volver a encender C3 no se reenvia una alerta antigua;
   restablecer la condicion y generar otra transicion para repetir la prueba.

No acelerar el pulso de una persona para probar: se puede verificar primero
con temperatura. El test C++ usa entradas sinteticas solo fuera del firmware.

Parametros: main/connectivity/lab_alert_rules.h.
Protocolo compartido: ../Protocolo Reloj Vincha/nera_lab_protocol.h.
Validacion estatica: tests/lab_alert_test.cpp (C++17, static_assert).
