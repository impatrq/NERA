# MAX30102, MPU6050 y MLX90614 en la pantalla activa

## Cableado y arranque

- Los tres sensores comparten I2C1 a 100 kHz: SDA GPIO21 y SCL GPIO2.
- Direcciones: MAX30102 0x57, MPU6050 0x68, MLX90614 0x5A.
- El tactil CST816D conserva I2C0 en GPIO48/47.
- `sensor_service_start()` crea una tarea que llama a `init_i2c()` del componente
  `mpumlx` antes de leer los dispositivos. Si falla la inicializacion del bus,
  reintenta sin detener la pantalla.
- El MPU6050 se comprueba mediante `WHO_AM_I` y se saca del modo sleep.
- El MAX30102 se comprueba mediante `PART_ID` y se configura para 100 muestras/s
  con LED rojo e IR. Se consulta el FIFO cada 20 ms; el pulso
  requiere tres intervalos coherentes despues del asentamiento optico.

## Que se muestra

| Zona | Fuente | Valor sin sensor |
| --- | --- | --- |
| Pulso | Estimacion experimental desde IR del MAX30102 | Sin lectura |
| SpO2 est. | Estimacion experimental desde rojo e IR del MAX30102 | -- |
| Temperatura de objeto IR | Registro 0x07 del MLX90614 | Sin lectura |
| Movimiento | Modulo de aceleracion y velocidad angular del MPU6050 | Sin lectura |
| Temperatura ambiente | Registro 0x06 del MLX90614 | Sin lectura |

La aceleracion incluye gravedad. Los valores del MLX son temperaturas IR;
la temperatura de objeto no equivale a temperatura corporal central.
El MLX90614 se debe apuntar sin apoyarlo en la piel; el contacto calienta el
encapsulado y puede alterar la lectura. Cuando falta un nuevo intervalo de
pulso, la pantalla conserva la cifra anterior rotulada "Ultimo pulso"; BLE e
historial siguen exigiendo una lectura vigente.
La pantalla conserva igualmente la ultima SpO2 estimada, rotulada "Ult. SpO2"
y en color atenuado cuando la senal no pasa la validacion. La notificacion BLE
se invalida y no publica esa cifra como una lectura actual.
El componente valida el PEC y el bit de error de cada lectura MLX antes de
publicar las dos temperaturas juntas. La pantalla muestra valores validos
solo mientras llegan muestras recientes (menos de tres segundos).
La SpO2 no está calibrada para este montaje y no debe usarse para decisiones
médicas. El BPM deja de ser valido sin contacto, con saturacion,
desbordamiento del FIFO o falla I2C. No es una medicion clinica.

## Verificacion en la placa

Compilar con `build.bat`, cargar el nuevo binario y observar los logs
`MPUMLX` y `NERA/SENSOR`. Apuntar el MLX a superficies con distinta temperatura
y mover/girar el MPU para comprobar que cambian las cifras. Desconectar cada
sensor con el equipo apagado y confirmar que su zona muestra "Sin lectura"
sin afectar el tactil ni los demas sensores.
Para el MAX, comprobar primero alimentacion y cableado: el modulo se calento
en pruebas anteriores. Si vuelve a calentarse, desconectarlo y revisar el
hardware antes de continuar. Con el modulo estable, apoyar un dedo y esperar
varios latidos; retirar el dedo y confirmar "Sin lectura".
