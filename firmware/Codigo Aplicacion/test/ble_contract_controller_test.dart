import 'dart:async';

import 'package:aplicacion_nera/nera_controller.dart';
import 'package:aplicacion_nera/user_preferences.dart';
import 'package:aplicacion_nera/watch_ble_service.dart';
import 'package:flutter_test/flutter_test.dart';

class MemoryPreferences extends UserPreferencesStore {
  @override
  Future<UserPreferences> load() async => const UserPreferences();

  @override
  Future<void> save(UserPreferences preferences) async {}
}

class FakeWatch extends WatchBleService {
  final states = StreamController<WatchConnectionState>.broadcast(sync: true);
  final packets = StreamController<WatchReading>.broadcast(sync: true);
  final notices = StreamController<String>.broadcast(sync: true);

  @override
  Stream<WatchConnectionState> get state => states.stream;

  @override
  Stream<WatchReading> get readings => packets.stream;

  @override
  Stream<String> get messages => notices.stream;

  @override
  Future<void> applyPreferences(UserPreferences preferences) async {}

  @override
  Future<void> dispose() async {
    await states.close();
    await packets.close();
    await notices.close();
  }
}

void main() {
  testWidgets('una métrica ausente conserva la otra hasta que vence', (tester) async {
    final watch = FakeWatch();
    var now = DateTime(2026, 11, 8, 12);
    final controller = NeraController(
      preferencesStore: MemoryPreferences(), watchService: watch,
      now: () => now);

    watch.states.add(WatchConnectionState.connected);
    await tester.pump();
    watch.packets.add(WatchReading.fromJsonText('{"temp":29.5}', receivedAt: now));
    await tester.pump();
    expect(controller.temperature, 29.5);
    expect(controller.heartRate, isNull);

    now = now.add(const Duration(seconds: 3));
    await tester.pump(const Duration(seconds: 3));
    watch.packets.add(WatchReading.fromJsonText('{"hr":76}', receivedAt: now));
    await tester.pump();
    expect(controller.temperature, 29.5);
    expect(controller.heartRate, 76);

    now = now.add(const Duration(seconds: 3));
    await tester.pump(const Duration(seconds: 3));
    expect(controller.temperature, isNull);
    expect(controller.heartRate, 76);

    watch.packets.add(WatchReading.fromJsonText('{"hr":null}', receivedAt: now));
    await tester.pump();
    expect(controller.heartRate, isNull);
    expect(controller.lastReadingAt, isNull);
    controller.dispose();
    await tester.pump();
  });
}
