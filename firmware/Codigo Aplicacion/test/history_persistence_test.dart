import 'dart:async';

import 'package:aplicacion_nera/nera_controller.dart';
import 'package:aplicacion_nera/user_preferences.dart';
import 'package:aplicacion_nera/watch_ble_service.dart';
import 'package:aplicacion_nera/watch_history.dart';
import 'package:aplicacion_nera/watch_history_store.dart';
import 'package:flutter_test/flutter_test.dart';

class MemoryPreferences extends UserPreferencesStore {
  @override
  Future<UserPreferences> load() async => const UserPreferences();
  @override
  Future<void> save(UserPreferences preferences) async {}
}

class MemoryHistory extends WatchHistoryStore {
  WatchHistoryArchive archive = const WatchHistoryArchive([]);
  @override
  Future<WatchHistoryArchive> load() async => archive;
  @override
  Future<void> save(WatchHistoryArchive value) async { archive = value; }
}

class FakeWatch extends WatchBleService {
  final states = StreamController<WatchConnectionState>.broadcast(sync: true);
  final samples = StreamController<WatchHistoryEntry>.broadcast(sync: true);
  final notices = StreamController<String>.broadcast(sync: true);
  @override
  Stream<WatchConnectionState> get state => states.stream;
  @override
  Stream<WatchHistoryEntry> get history => samples.stream;
  @override
  Stream<String> get messages => notices.stream;
  @override
  Future<void> applyPreferences(UserPreferences preferences) async {}
  @override
  Future<int> requestHistory({required int knownGeneration,
      required int lastSequence}) async => 77;
  @override
  Future<void> dispose() async {
    await states.close();
    await samples.close();
    await notices.close();
  }
}

void main() {
  testWidgets('conserva el historial recibido al cerrar y abrir la app', (tester) async {
    final storage = MemoryHistory();
    final watch = FakeWatch();
    final controller = NeraController(preferencesStore: MemoryPreferences(),
        watchService: watch, historyStore: storage);
    await tester.pump();
    watch.states.add(WatchConnectionState.connected);
    await tester.pump();
    const point = WatchHistoryEntry(generation: 77, sequence: 1,
        epochSeconds: 1794140000, bpm: 76, objectDeciC: 295);
    watch.samples.add(point);
    watch.samples.add(point); // Reenvío tras reconexión: no duplicar.
    await tester.pump();
    expect(controller.history.length, 1);
    await tester.pump(const Duration(seconds: 3));
    await tester.pump();
    expect(storage.archive.entries.length, 1);

    controller.dispose();
    await tester.pump();
    final reopened = NeraController(preferencesStore: MemoryPreferences(),
        watchService: FakeWatch(), historyStore: storage);
    await tester.pump();
    expect(reopened.latestHistory.single.bpm, 76);
    reopened.dispose();
    await tester.pump();
  });
}
