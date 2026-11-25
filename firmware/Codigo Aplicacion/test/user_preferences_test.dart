import 'dart:async';
import 'dart:convert';
import 'package:flutter_test/flutter_test.dart';
import 'package:flutter/material.dart';
import 'package:aplicacion_nera/main.dart';
import 'package:aplicacion_nera/models.dart';
import 'package:aplicacion_nera/nera_controller.dart';
import 'package:aplicacion_nera/user_preferences.dart';
import 'package:aplicacion_nera/watch_ble_service.dart';

class MemoryPreferences extends UserPreferencesStore {
  String? text;
  bool fail = false;
  @override
  Future<UserPreferences> load() async => text == null
      ? const UserPreferences() : UserPreferences.decode(text!);
  @override
  Future<void> save(UserPreferences preferences) async {
    if (fail) throw StateError('Disk full');
    text = preferences.encode();
  }
}

class TestWatch extends WatchBleService {
  final states = StreamController<WatchConnectionState>.broadcast(sync: true);
  final applied = <UserPreferences>[];
  bool fail = false;
  @override
  Stream<WatchConnectionState> get state => states.stream;
  @override
  Future<void> applyPreferences(UserPreferences preferences) async {
    if (fail) throw StateError('No confirmation');
    applied.add(preferences);
  }
  @override
  Future<void> dispose() => states.close();
}

Future<void> settle() => Future<void>.delayed(const Duration(milliseconds: 20));

void main() {
  testWidgets('Perfil permite elegir sensibilidad y ejercicio en pantalla estrecha', (tester) async {
    tester.view.physicalSize = const Size(320, 640);
    tester.view.devicePixelRatio = 1;
    addTearDown(tester.view.resetPhysicalSize);
    addTearDown(tester.view.resetDevicePixelRatio);
    final store = MemoryPreferences();
    final controller = NeraController(preferencesStore: store, watchService: TestWatch());
    await tester.pumpWidget(MaterialApp(home: Scaffold(body: AnimatedBuilder(
      animation: controller, builder: (_, child) => ProfileScreen(
        controller: controller, onShowSnack: (_) {})))));
    await tester.pump();
    final sensitivity = find.byType(DropdownButton<AlertSensitivity>);
    await tester.scrollUntilVisible(sensitivity, 200, scrollable: find.byType(Scrollable).first);
    await tester.tap(sensitivity);
    await tester.pumpAndSettle();
    await tester.tap(find.text('Baja · confirmar durante 20 s').last);
    await tester.pumpAndSettle();
    expect(controller.sensitivity, AlertSensitivity.low);
    final activity = find.byType(DropdownButton<ActivityMode>);
    await tester.ensureVisible(activity);
    await tester.tap(activity);
    await tester.pumpAndSettle();
    await tester.tap(find.text('Ejercicio').last);
    await tester.pumpAndSettle();
    expect(controller.activityMode, ActivityMode.exercise);
    expect((await store.load()).wire, [1,1,2,1]);
    expect(tester.takeException(), isNull);
    await tester.pumpWidget(const SizedBox.shrink());
    controller.dispose();
  });

  test('conserva ajustes y contactos sin convertirlos en mensajes a terceros', () {
    const original = UserPreferences(alerts: false, sensitivity: AlertSensitivity.low,
      activity: ActivityMode.exercise, name: 'Ana', email: 'ana@example.com',
      contacts: [EmergencyContact(name: 'Luis', relation: 'Amigo', phone: '123', active: true)]);
    final saved = UserPreferences.decode(original.encode());
    expect(saved.wire, [1,0,2,1]);
    expect(saved.confirmationSeconds, 20);
    expect(saved.name, 'Ana');
    expect(saved.contacts.single.phone, '123');
    expect(saved.contacts.single.active, isTrue);
    final bad = jsonDecode(original.encode()) as Map<String,dynamic>;
    bad['sensitivity'] = 99;
    expect(() => UserPreferences.decode(jsonEncode(bad)), throwsFormatException);
    bad['version'] = 2;
    expect(() => UserPreferences.decode(jsonEncode(bad)), throwsFormatException);
  });

  test('guarda offline, aplica al conectar y restaura despues de recrear controlador', () async {
    final store = MemoryPreferences();
    final watch = TestWatch();
    final controller = NeraController(preferencesStore: store, watchService: watch);
    await settle();
    controller.setAutomaticAlerts(false);
    controller.setSensitivity(AlertSensitivity.medium);
    controller.setActivityMode(ActivityMode.exercise);
    controller.addContact(const EmergencyContact(name:'Ana', relation:'Amiga', phone:'123', active:true));
    await controller.syncPreferences();
    expect(watch.applied, isEmpty);
    expect(store.text, isNotNull);
    watch.states.add(WatchConnectionState.connected);
    await settle();
    expect(watch.applied.last.wire, [1,0,1,1]);
    expect(controller.watchPreferencesStatus, startsWith('Confirmado'));
    controller.dispose();
    final next = NeraController(preferencesStore: store, watchService: TestWatch());
    await settle();
    expect(next.automaticAlerts, isFalse);
    expect(next.sensitivity, AlertSensitivity.medium);
    expect(next.activityMode, ActivityMode.exercise);
    expect(next.contacts.single.name, 'Ana');
    next.dispose();
  });

  test('un fallo de disco o de confirmacion nunca muestra exito', () async {
    final store = MemoryPreferences();
    final watch = TestWatch();
    final controller = NeraController(preferencesStore: store, watchService: watch);
    await settle();
    store.fail = true;
    controller.setAutomaticAlerts(false);
    await controller.syncPreferences();
    expect(controller.preferencesStatus, startsWith('No se pudo guardar'));
    store.fail = false;
    watch.fail = true;
    watch.states.add(WatchConnectionState.connected);
    await settle();
    expect(controller.watchPreferencesStatus, startsWith('Sin confirmar'));
    expect(watch.applied, isEmpty);
    controller.dispose();
  });
}
