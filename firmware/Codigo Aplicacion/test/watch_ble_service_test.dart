import 'package:aplicacion_nera/watch_ble_service.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  test('identifica NERA por nombre o servicio, sin aceptar dispositivos ajenos', () {
    bool matches(String name, String cached, List<String> uuids) =>
        isNeraAdvertisement(advertisedName: name, platformName: cached, serviceUuids: uuids);
    expect(matches(' NERA Watch ', '', []), isTrue);
    expect(matches('', 'nera watch', []), isTrue);
    expect(matches('', '', [neraServiceUuid.toUpperCase()]), isTrue);
    expect(matches('Otro reloj', '', [neraServiceUuid]), isTrue);
    expect(matches('NERA Watch falso', '', []), isFalse);
    expect(matches('', '', ['180f']), isFalse);
  });

  test('acepta solo métricas presentes y deja el resto sin lectura', () {
    final reading = WatchReading.fromJsonText('{"temp":36.6,"ax":0.0,"ay":0.0,"az":1.0}');
    expect(reading.temperature, 36.6);
    expect(reading.heartRate, isNull);
    expect(reading.spo2, isNull);
    expect(reading.movement, 1.0);
  });

  test('separa métricas y permite invalidación explícita', () {
    final pulse = WatchReading.fromJsonText('{"hr":76}');
    expect(pulse.hasHeartRate, isTrue);
    expect(pulse.hasTemperature, isFalse);
    final lost = WatchReading.fromJsonText('{"temp":null}');
    expect(lost.hasTemperature, isTrue);
    expect(lost.temperature, isNull);
    final oxygen = WatchReading.fromJsonText('{"spo2":97}');
    expect(oxygen.hasSpo2, isTrue);
    expect(oxygen.spo2, 97);
    final oxygenLost = WatchReading.fromJsonText('{"spo2":null}');
    expect(oxygenLost.hasSpo2, isTrue);
    expect(oxygenLost.spo2, isNull);
  });

  test('rechaza números inválidos y mensajes ajenos al protocolo', () {
    expect(() => WatchReading.fromJsonText('{"hr":0}'), throwsFormatException);
    expect(() => WatchReading.fromJsonText('{"hr":76.5}'), throwsFormatException);
    expect(() => WatchReading.fromJsonText('{"hr":241}'), throwsFormatException);
    expect(() => WatchReading.fromJsonText('{"temp":"36.6"}'), throwsFormatException);
    expect(() => WatchReading.fromJsonText('{"alarm":1}'), throwsFormatException);
    expect(() => WatchReading.fromJsonText('[]'), throwsFormatException);
  });

  test('acepta los límites y la pérdida explícita de cada métrica v1', () {
    final pulse = WatchReading.fromJsonText('{"hr":30}');
    expect(pulse.heartRate, 30);
    expect(pulse.hasTemperature, isFalse);

    final temperature = WatchReading.fromJsonText('{"temp":-70}');
    expect(temperature.temperature, -70);
    expect(temperature.hasHeartRate, isFalse);

    final pulseLost = WatchReading.fromJsonText('{"hr":null}');
    expect(pulseLost.hasHeartRate, isTrue);
    expect(pulseLost.heartRate, isNull);
    expect(() => WatchReading.fromJsonText('{"temp":381}'), throwsFormatException);
  });
}
