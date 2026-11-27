import 'package:flutter_test/flutter_test.dart';
import 'package:aplicacion_nera/watch_alert.dart';

WatchAlert event(AlertDeliveryPhase phase, {int metric = 1, int state = 1}) =>
    WatchAlert(
      metric: metric,
      state: state,
      session: 0x12345678,
      id: 9,
      centiValue: 11000,
      phase: phase,
    );

void main() {
  test(
    'contrato binario coincide con firmware y no acepta tramas antiguas',
    () {
      final bytes = event(AlertDeliveryPhase.waitingApp).receipt();
      expect(bytes.length, 20);
      expect(bytes.take(8), [78, 69, 82, 65, 3, 5, 1, 1]);
      expect(bytes.sublist(8, 16), [0x78, 0x56, 0x34, 0x12, 9, 0, 0, 0]);
      bytes[5] = 4;
      final received = WatchAlert.decode(bytes);
      expect(received.phase, AlertDeliveryPhase.waitingApp);
      expect(received.centiValue, 11000);
      expect(received.receipt()[5], 5);
      for (final type in [6, 7, 8]) {
        bytes[5] = type;
        expect(
          WatchAlert.decode(bytes).phase,
          AlertDeliveryPhase.values[type - 5],
        );
      }
      bytes[4] = 2;
      expect(() => WatchAlert.decode(bytes), throwsFormatException);
      bytes[4] = 3;
      bytes[6] = 3;
      expect(() => WatchAlert.decode(bytes), throwsFormatException);
      expect(() => WatchAlert.decode(bytes.sublist(1)), throwsFormatException);
      expect(WatchAlert.phoneReady(), [
        78,
        69,
        82,
        65,
        3,
        9,
        1,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
      ]);
    },
  );
}
