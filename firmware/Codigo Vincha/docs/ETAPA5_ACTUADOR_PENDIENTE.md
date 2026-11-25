# Preparación de bobinas de la vincha

Actualización 18/11/2026: `Codigo Vincha` vuelve a generar GPIO5 de 15 Hz,
activo bajo ~2,5 ms, exclusivamente SIN bobinas, tras la prueba temporal fija
y la reducción del tiempo activo a la mitad solicitada por el usuario.
Ver [prueba GPIO5](PRUEBA_GPIO5_15HZ.md). El esquema nuevo usa un 2N2222
y un IRFZ44N. Las notas de IRF3205/2S que siguen describen la preparación
anterior; la prueba actual es por USB y no habilita la conexión de bobinas.

La comunicación reloj ↔ C3 se probó el 8/11/2026. El firmware cargado en la C3
solo registra eventos y responde ACK: no configura GPIO de salida. Las bobinas
siguen desconectadas.

La frecuencia indicada para el futuro accionamiento es **15 Hz**, equivalente
a un período de **66,7 ms**. Ese dato no define el tiempo encendido por ciclo,
la corriente ni la disipación. Por eso todavía no se fijó un PWM ni un pulso.
La alimentación prevista para las bobinas es **dos celdas LiPo en serie (2S)**;
todavía no se midió la tensión real que llega a cada bobina. La persona no
puede medir ahora la resistencia, y esa medición no es necesaria para la
prueba de radio ya completada.

## Datos que faltan medir o confirmar

1. Tensión real en el bus de las bobinas con el conjunto 2S conectado.
2. Resistencia DC de cada bobina por separado, con las baterías desconectadas;
   inductancia y corriente nominal si existe ficha técnica.
3. Tiempo encendido previsto por ciclo y límite de temperatura/corriente del
   conjunto, definidos para el prototipo físico.
4. Confirmación de que la placa montada coincide con el esquema: pin del C3
   conectado a cada compuerta, resistencias y diodos instalados, y camino de
   alimentación de cada bobina.
5. Medición de tensión compuerta-fuente de cada MOSFET durante un pulso de
   banco. El esquema dibuja IRF3205; su ficha garantiza la baja resistencia de
   conducción a `VGS = 10 V`, no a una salida directa de 3,3 V del C3.

La corriente estacionaria aproximada se calculará como `I = V/R` después de
conocer la conexión y los valores reales. En una bobina, el crecimiento de
corriente durante un pulso también depende de su inductancia y duración; el
período de 66,7 ms por sí solo no permite calcularlo.

## Condiciones antes de habilitar una salida

- Ensayo en banco, fuera del cuerpo, con límite de corriente independiente.
- Salida apagada al arrancar, al perder enlace, ante datos vencidos y al
  agotarse un tiempo máximo de pulso implementado en la C3.
- Mensajes unicast autenticados y cifrados; el filtro MAC de laboratorio no
  autoriza una bobina. ESP-NOW permite cifrar unicast con una LMK compartida.
- Verificación de que un evento repetido no vuelve a disparar el pulso y de
  que un ACK no se confunde con confirmación de actuación física.

Referencias: [IRF3205, ficha de Infineon](https://www.infineon.com/assets/row/public/documents/24/49/infineon-irf3205-datasheet-en.pdf?fileId=5546d462533600a4015355def244190a), [seguridad ESP-NOW de Espressif](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/network/esp_now.html).
