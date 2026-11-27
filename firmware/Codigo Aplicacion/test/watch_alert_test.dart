import 'dart:async';
import 'package:flutter_test/flutter_test.dart';
import 'package:aplicacion_nera/watch_alert.dart';
import 'package:aplicacion_nera/watch_ble_service.dart';
import 'package:aplicacion_nera/nera_controller.dart';
import 'package:aplicacion_nera/user_preferences.dart';

class AlertWatch extends WatchBleService {
  final notifications = StreamController<WatchAlert>.broadcast(sync: true);
  final states = StreamController<WatchConnectionState>.broadcast(sync: true);
  @override
  Stream<WatchAlert> get alerts => notifications.stream;
  @override
  Stream<WatchConnectionState> get state => states.stream;
  @override
  Future<void> dispose() async {
    await notifications.close();
    await states.close();
  }
}

class AlertPreferences extends UserPreferencesStore {
  @override
  Future<UserPreferences> load() async => const UserPreferences();
}

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
  testWidgets(
    'recepcion, ACK, fallo y vencimiento se distinguen sin duplicar avisos',
    (tester) async {
      final watch = AlertWatch();
      var now = DateTime(2026, 11, 26, 12);
      final controller = NeraController(
        preferencesStore: AlertPreferences(),
        watchService: watch,
        now: () => now,
      );
      await tester.pump();
      watch.notifications.add(event(AlertDeliveryPhase.waitingApp));
      expect(controller.alertStatus, contains('Alerta recibida en la app'));
      expect(controller.alertStatus, isNot(contains('GPIO5 activado')));
      watch.notifications.add(event(AlertDeliveryPhase.sending));
      expect(controller.alertStatus, contains('Esperando confirmación'));
      watch.notifications.add(event(AlertDeliveryPhase.confirmed));
      expect(
        controller.alertStatus,
        contains('Vincha confirmó GPIO5 activado'),
      );
      final count = controller.events.length;
      watch.notifications.add(event(AlertDeliveryPhase.confirmed));
      expect(controller.events.length, count);
      watch.notifications.add(event(AlertDeliveryPhase.failed));
      expect(controller.alertStatus, contains('Sin confirmación'));
      watch.notifications.add(event(AlertDeliveryPhase.confirmed, metric: 2));
      expect(controller.alertStatuses.length, 2);
      expect(controller.alertStatuses.first, contains('Sin confirmación'));
      expect(controller.alertStatuses.last, contains('Temperatura IR'));
      now = now.add(const Duration(seconds: 4));
      await tester.pump(const Duration(seconds: 4));
      expect(controller.currentAlerts, isEmpty);
      expect(controller.alertStatus, contains('vencido'));
      watch.notifications.add(event(AlertDeliveryPhase.confirmed));
      watch.states.add(WatchConnectionState.disconnected);
      expect(controller.currentAlerts, isEmpty);
      expect(controller.alertStatus, contains('sin confirmar'));
      watch.notifications.add(event(AlertDeliveryPhase.confirmed));
      controller.setAppForeground(false);
      expect(controller.currentAlerts, isEmpty);
      expect(controller.alertStatus, contains('segundo plano'));
      controller.dispose();
      await tester.pump();
    },
  );
}
