# Sensores del reloj

Este componente reúne los drivers que antes estaban repartidos entre
`components/mpumlx/` y `main/sensors/`. El traslado conserva las funciones y sus
valores. Después se añadió la estimación experimental de SpO2 desde rojo e IR;
no se añaden lecturas simuladas.

| Sección | Responsabilidad |
| --- | --- |
| `include/sensor_config.h` | Puerto I2C1 y GPIO externos: SDA21, SCL2 |
| `include/sensor_service.h` | Estructura pública `SensorSnapshot` y API del servicio |
| `sensor_service.cpp` | Tarea de muestreo, reintentos, validez y publicación protegida |
| `drivers/i2c_bus.c` | Inicialización del bus a 100 kHz, como antes |
| `drivers/mpu6050.c` | Identificación, configuración y lectura de acelerómetro/giroscopio |
| `drivers/mlx90614.c` | Lectura de objeto/ambiente y comprobación PEC |
| `drivers/max30102.cpp` | Identificación, LED rojo/IR, FIFO y muestras ópticas |
| `drivers/max30102_fifo.h` | Interpretación de las muestras rojo/IR de seis bytes |
| `processing/heart_rate_estimator.h` | Contacto, señal válida y caducidad del BPM |
| `processing/spo2_estimator.h` | Ventanas rojo/IR, calidad de señal y SpO2 experimental |
| `processing/pba_detector.h` | Filtro/detector de latidos; conservar su licencia |
| `processing/pulse_intervals.h` | Validación de intervalos entre latidos |

## FUNCIONAL: cómo usarlo desde otra parte del reloj

Incluir `sensor_service.h`, iniciar con `sensor_service_start()` una sola vez y
obtener lecturas mediante `sensor_service_get_snapshot()`. Los consumidores deben
consultar los indicadores de validez y las fechas de actualización. Un número
guardado no equivale por sí solo a una lectura válida actual.

## MODIFICABLE: dónde están los parámetros

Los intervalos de muestreo/reintento están al principio de `sensor_service.cpp`.
Los límites ópticos y la caducidad están al principio de
`processing/heart_rate_estimator.h`. Las corrientes de LED están en
`drivers/max30102.cpp`. Se indican para encontrar el código: **esta etapa no los
cambió**, y ajustarlos exige otra prueba física.

## IMPORTANTE

- Los registros y direcciones están en los headers de cada dispositivo; el MAX
  mantiene su dirección en su driver. Se conservan todos los valores originales.
- Los nombres `MLX_MPU6050_*` se mantienen por compatibilidad; identifican el bus
  compartido por los tres sensores externos, incluido el MAX.
- `MPU6050_INT` y las funciones Wake-On-Motion se conservaron, pero su declaración
  no habilita esa interrupción ni modifica el circuito.
- La SpO2 no está calibrada para este conjunto de sensor y montaje; no se debe
  usar para decisiones médicas.
- No crear otra tarea que lea el mismo bus por fuera del servicio.
- El bus del táctil, LVGL y los drivers del display permanecen en `main/`.
