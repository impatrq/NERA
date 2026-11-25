import 'dart:async';
import 'package:flutter/material.dart';
import 'models.dart';
import 'watch_ble_service.dart';
import 'app_theme.dart';
import 'user_preferences.dart';
import 'watch_history.dart';
import 'watch_history_store.dart';

class NeraController extends ChangeNotifier {
  final WatchBleService _watchService;
  final UserPreferencesStore _preferencesStore;
  final WatchHistoryStore _historyStore;
  final DateTime Function() _now;
  late final Future<void> _preferencesLoaded;
  late final Future<void> _historyLoaded;
  Future<void> _saveQueue = Future<void>.value();
  int _preferencesRevision = 0;
  bool _disposed = false;
  bool preferencesReady = false;
  String preferencesStatus = 'Cargando preferencias...';
  String watchPreferencesStatus = 'Pendiente de conectar con el reloj.';
  AlertSensitivity sensitivity = AlertSensitivity.high;
  ActivityMode activityMode = ActivityMode.everyday;
  final List<StreamSubscription<Object?>> _subscriptions = [];
  Timer? _staleTimer;
  Timer? _historySaveTimer;
  Future<void> _historySaveQueue = Future<void>.value();
  final Set<String> _historyKeys = {};
  final List<WatchHistoryEntry> history = [];
  String historyStatus = 'Cargando historial...';

  String name = 'Usuario NERA';
  String email = 'usuario@nera.app';

  // FUNCIONAL: null significa que aún no hay una medición real vigente.
  int? heartRate;
  int? oxygen;
  double? temperature;
  double? movement;
  DateTime? lastReadingAt;
  DateTime? _heartAt;
  DateTime? _oxygenAt;
  DateTime? _temperatureAt;
  DateTime? _movementAt;

  WatchConnectionState watchState = WatchConnectionState.disconnected;
  String watchMessage = 'Reloj no conectado';

  bool automaticAlerts = true;
  bool shareLocation = false;
  bool healthNotifications = false;
  bool calmMode = false;

  final List<EmergencyContact> contacts = [];

  final List<AlertEvent> events = [];

  final List<double> _movementHistory = [];
  static const int _smoothingWindow = 5;

  NeraController({UserPreferencesStore? preferencesStore, WatchBleService? watchService,
      WatchHistoryStore? historyStore, DateTime Function()? now})
      : _preferencesStore = preferencesStore ?? UserPreferencesStore(),
        _historyStore = historyStore ?? WatchHistoryStore(),
        _watchService = watchService ?? WatchBleService(),
        _now = now ?? DateTime.now {
    _preferencesLoaded = _loadPreferences();
    _historyLoaded = _loadHistory();
    _init();
  }

  List<WatchHistoryEntry> get latestHistory => history.reversed.take(20).toList();

  Future<void> _loadHistory() async {
    try {
      final saved = await _historyStore.load();
      if (_disposed) return;
      for (final entry in saved.entries) { _mergeHistory(entry, save: false); }
      historyStatus = history.isEmpty ? 'Aún no hay muestras guardadas.' :
          '${history.length} muestras en el historial.';
    } catch (_) {
      if (_disposed) return;
      historyStatus = 'No se pudo abrir el historial local.';
    }
    notifyListeners();
  }

  void _mergeHistory(WatchHistoryEntry entry, {bool save = true}) {
    final key = '${entry.generation}:${entry.sequence}';
    if (!_historyKeys.add(key)) return;
    history.add(entry);
    if (history.length > 10080) {
      final removed = history.removeAt(0);
      _historyKeys.remove('${removed.generation}:${removed.sequence}');
    }
    if (save) {
      historyStatus = '${history.length} muestras en el historial.';
      _historySaveTimer ??= Timer(const Duration(seconds: 3), () {
        _historySaveTimer = null;
        unawaited(_saveHistory());
      });
    }
    if (save) notifyListeners();
  }

  Future<void> _saveHistory() {
    _historySaveTimer?.cancel();
    _historySaveTimer = null;
    final snapshot = WatchHistoryArchive(List.of(history));
    _historySaveQueue = _historySaveQueue.then((_) async {
      try {
        await _historyStore.save(snapshot);
      } catch (_) {
        if (_disposed) return;
        historyStatus = 'No se pudo guardar el historial en el teléfono.';
        notifyListeners();
      }
    });
    return _historySaveQueue;
  }

  Future<void> _syncHistory() async {
    await _historyLoaded;
    if (_disposed || watchState != WatchConnectionState.connected) return;
    final last = history.isEmpty ? null : history.last;
    try {
      await _watchService.requestHistory(knownGeneration: last?.generation ?? 0,
          lastSequence: last?.sequence ?? 0);
    } catch (_) {
      if (_disposed) return;
      historyStatus = 'No se pudo sincronizar el historial del reloj.';
      notifyListeners();
    }
  }

  UserPreferences get userPreferences => UserPreferences(alerts: automaticAlerts,
      sensitivity: sensitivity, activity: activityMode, name: name, email: email,
      contacts: List.of(contacts));

  Future<void> _loadPreferences() async {
    try {
      final saved = await _preferencesStore.load();
      if (_disposed) return;
      automaticAlerts = saved.alerts;
      sensitivity = saved.sensitivity;
      activityMode = saved.activity;
      name = saved.name;
      email = saved.email;
      contacts.addAll(saved.contacts);
      preferencesReady = true;
      preferencesStatus = 'Preferencias cargadas. Guardado local en este teléfono.';
    } catch (_) {
      if (_disposed) return;
      preferencesStatus = 'No se pudieron cargar las preferencias. Cerrá y abrí la app para reintentar.';
    }
    notifyListeners();
  }

  void _persistPreferences() {
    final version = ++_preferencesRevision;
    final saved = userPreferences;
    preferencesStatus = 'Guardando en el teléfono...';
    watchPreferencesStatus = 'Cambios pendientes de confirmar en el reloj.';
    _saveQueue = _saveQueue.then((_) async {
      try {
        await _preferencesStore.save(saved);
        if (_disposed || version != _preferencesRevision) return;
        preferencesStatus = 'Guardado en el teléfono.';
        if (watchState == WatchConnectionState.connected) {
          await _syncPreferences(saved, version);
        }
      } catch (_) {
        if (_disposed || version != _preferencesRevision) return;
        preferencesStatus = 'No se pudo guardar. Usá Reintentar guardado y sincronización.';
      }
      if (!_disposed) notifyListeners();
    });
  }

  Future<void> _syncPreferences(UserPreferences saved, int version) async {
    watchPreferencesStatus = 'Esperando confirmación del reloj...';
    if (!_disposed) notifyListeners();
    try {
      await _watchService.applyPreferences(saved);
      if (_disposed || version != _preferencesRevision) return;
      watchPreferencesStatus = watchState == WatchConnectionState.connected
          ? 'Confirmado por el reloj y guardado para sus próximos arranques.'
          : 'Sin conexión; no se pudo verificar el estado actual del reloj.';
    } catch (error) {
      if (_disposed || version != _preferencesRevision) return;
      watchPreferencesStatus = 'Sin confirmar en el reloj: $error';
    }
  }

  Future<void> syncPreferences() async {
    await _preferencesLoaded;
    if (_disposed || !preferencesReady) return;
    _persistPreferences();
    notifyListeners();
    await _saveQueue;
  }

  void _init() {
    _subscriptions.add(_watchService.state.listen((state) {
      watchState = state;
      if (state != WatchConnectionState.connected) {
        _clearCurrentReading();
        if (history.isNotEmpty) unawaited(_saveHistory());
        watchPreferencesStatus = 'Sin conexión. El reloj conserva los últimos ajustes confirmados.';
      } else {
        unawaited(syncPreferences());
        unawaited(_syncHistory());
      }
      notifyListeners();
    }));

    _subscriptions.add(_watchService.messages.listen((message) {
      watchMessage = message;
      notifyListeners();
    }));

    _subscriptions.add(_watchService.readings.listen(_applyWatchReading));
    _subscriptions.add(_watchService.history.listen((entry) {
      _mergeHistory(entry);
    }));
    _staleTimer = Timer.periodic(const Duration(seconds: 1), (_) {
      final now = _now();
      bool expired(DateTime? at) => at != null &&
          now.difference(at) > const Duration(seconds: 5);
      var changed = false;
      if (expired(_heartAt)) { heartRate = null; _heartAt = null; changed = true; }
      if (expired(_oxygenAt)) { oxygen = null; _oxygenAt = null; changed = true; }
      if (expired(_temperatureAt)) { temperature = null; _temperatureAt = null; changed = true; }
      if (expired(_movementAt)) {
        movement = null;
        _movementAt = null;
        _movementHistory.clear();
        changed = true;
      }
      if (changed) {
        _refreshLastReadingAt();
        if (lastReadingAt == null) {
          watchMessage = 'Lecturas vencidas; esperando datos nuevos del reloj.';
        }
        notifyListeners();
      }
    });
  }

  void _refreshLastReadingAt() {
    final times = [_heartAt, _oxygenAt, _temperatureAt, _movementAt]
        .whereType<DateTime>().toList();
    times.sort();
    lastReadingAt = times.isEmpty ? null : times.last;
  }

  void _clearCurrentReading() {
    heartRate = null;
    oxygen = null;
    temperature = null;
    movement = null;
    lastReadingAt = null;
    _heartAt = _oxygenAt = _temperatureAt = _movementAt = null;
    _movementHistory.clear();
  }

  void _applyWatchReading(WatchReading reading) {
    if (reading.hasHeartRate) {
      heartRate = reading.heartRate;
      _heartAt = heartRate == null ? null : reading.receivedAt;
    }
    if (reading.hasSpo2) {
      oxygen = reading.spo2;
      _oxygenAt = oxygen == null ? null : reading.receivedAt;
    }
    if (reading.hasTemperature) {
      temperature = reading.temperature;
      _temperatureAt = temperature == null ? null : reading.receivedAt;
    }
    if (reading.hasMotion) {
      final sampleMovement = reading.movement;
      if (sampleMovement == null) {
        _movementHistory.clear();
        movement = null;
        _movementAt = null;
      } else {
        _movementHistory.add(sampleMovement);
        if (_movementHistory.length > _smoothingWindow) {
          _movementHistory.removeAt(0);
        }
        movement = _movementHistory.reduce((a, b) => a + b) / _movementHistory.length;
        _movementAt = reading.receivedAt;
      }
    }
    _refreshLastReadingAt();

    final values = <String>[
      if (reading.hasHeartRate && reading.heartRate != null) 'pulso $heartRate bpm',
      if (reading.hasSpo2 && reading.spo2 != null) 'SpO2 $oxygen%',
      if (reading.hasTemperature && reading.temperature != null)
        'objeto IR ${temperature!.toStringAsFixed(1)} °C',
      if (reading.hasMotion && reading.movement != null) 'movimiento recibido',
    ];
    if (values.isNotEmpty) {
      events.insert(
        0,
        AlertEvent(
          title: 'Lectura del reloj',
          description: values.join(', '),
          time: reading.receivedAt,
          icon: Icons.watch,
          color: AppTheme.primary,
        ),
      );
    }
    if (events.length > 60) {
      events.removeLast();
    }
    notifyListeners();
  }

  void runMeasurement() {
    if (watchState != WatchConnectionState.connected) {
      connectWatch();
      return;
    }
    watchMessage = 'Esperando la próxima lectura real del reloj.';
    notifyListeners();
  }

  Future<void> connectWatch() async {
    if (watchState == WatchConnectionState.connected) {
      await _watchService.disconnect();
      return;
    }
    await _watchService.connect();
  }

  void startBreathingSession() {
    events.insert(
      0,
      AlertEvent(
        title: 'Modo calma iniciado',
        description: 'Sesión de respiración guiada iniciada.',
        time: DateTime.now(),
        icon: Icons.self_improvement,
        color: AppTheme.primary,
      ),
    );
    notifyListeners();
  }

  void addContact(EmergencyContact contact) {
    if (!preferencesReady) return;
    contacts.add(contact);
    _persistPreferences();
    notifyListeners();
  }

  void updateContact(int index, EmergencyContact contact) {
    if (!preferencesReady) return;
    contacts[index] = contact;
    _persistPreferences();
    notifyListeners();
  }

  void removeContact(int index) {
    if (!preferencesReady) return;
    contacts.removeAt(index);
    _persistPreferences();
    notifyListeners();
  }

  void updateProfile(String newName, String newEmail) {
    if (!preferencesReady) return;
    name = newName;
    email = newEmail;
    _persistPreferences();
    notifyListeners();
  }

  void setAutomaticAlerts(bool value) {
    if (!preferencesReady) return;
    automaticAlerts = value;
    _persistPreferences();
    notifyListeners();
  }

  void setSensitivity(AlertSensitivity value) {
    if (!preferencesReady) return;
    sensitivity = value;
    _persistPreferences();
    notifyListeners();
  }

  void setActivityMode(ActivityMode value) {
    if (!preferencesReady) return;
    activityMode = value;
    _persistPreferences();
    notifyListeners();
  }

  void setShareLocation(bool value) {
    shareLocation = value;
    notifyListeners();
  }

  void setHealthNotifications(bool value) {
    healthNotifications = value;
    notifyListeners();
  }

  void setCalmMode(bool value) {
    calmMode = value;
    notifyListeners();
  }

  @override
  void dispose() {
    _disposed = true;
    _staleTimer?.cancel();
    _historySaveTimer?.cancel();
    if (history.isNotEmpty) unawaited(_saveHistory());
    for (final subscription in _subscriptions) {
      unawaited(subscription.cancel());
    }
    unawaited(_watchService.dispose());
    super.dispose();
  }
}
