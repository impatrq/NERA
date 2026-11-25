import 'dart:async';
import 'dart:convert';
import 'dart:math';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';
import 'user_preferences.dart';
import 'watch_time_protocol.dart';
import 'watch_history.dart';

// CAMBIAR: nombre con el que se anuncia tu reloj por Bluetooth.
const String neraWatchName = 'NERA Watch';

// CAMBIAR: UUID del servicio BLE que debe usar el firmware del reloj.
const String neraServiceUuid = '7e400001-b5a3-f393-e0a9-e50e24dcca9e';

// CAMBIAR: UUID de la caracteristica BLE que manda datos hacia la app.
const String neraDataCharacteristicUuid =
    '7e400003-b5a3-f393-e0a9-e50e24dcca9e';
const String neraConfigCharacteristicUuid =
    '7e400004-b5a3-f393-e0a9-e50e24dcca9e';

// FUNCIONAL: nombre para firmware anterior, UUID para identificar sin nombre.
// El servicio y la caracteristica se verifican otra vez despues de conectar.
bool isNeraAdvertisement({
  required String advertisedName,
  required String platformName,
  required Iterable<String> serviceUuids,
}) => [advertisedName, platformName].any((name) =>
        name.trim().toLowerCase() == neraWatchName.toLowerCase()) ||
    serviceUuids.any((uuid) => uuid.toLowerCase() == neraServiceUuid);

// Cada notificación contiene una métrica: {"temp":36.6} o {"hr":76}.
// Un campo ausente no cambia la otra métrica; null la invalida explícitamente.
class WatchReading {
  const WatchReading({
    required this.heartRate,
    required this.spo2,
    required this.temperature,
    required this.accelX,
    required this.accelY,
    required this.accelZ,
    required this.hasHeartRate,
    required this.hasSpo2,
    required this.hasTemperature,
    required this.hasMotion,
    required this.receivedAt,
  });

  final int? heartRate;
  final int? spo2;
  final double? temperature;
  final double? accelX;
  final double? accelY;
  final double? accelZ;
  final bool hasHeartRate;
  final bool hasSpo2;
  final bool hasTemperature;
  final bool hasMotion;
  final DateTime receivedAt;

  double? get movement {
    final x = accelX, y = accelY, z = accelZ;
    if (x == null || y == null || z == null) return null;
    return sqrt(x * x + y * y + z * z);
  }

  factory WatchReading.fromJsonText(String text, {DateTime? receivedAt}) {
    final decoded = jsonDecode(text);
    if (decoded is! Map<String, dynamic>) {
      throw const FormatException('La lectura BLE debe ser un objeto JSON');
    }
    final data = decoded;
    const allowed = {'hr', 'spo2', 'temp', 'ax', 'ay', 'az'};
    if (data.keys.any((key) => !allowed.contains(key)) || data.isEmpty) {
      throw const FormatException('Campos BLE desconocidos o vacíos');
    }
    final hasMotion = data.containsKey('ax') || data.containsKey('ay') ||
        data.containsKey('az');
    if (hasMotion &&
        !(data.containsKey('ax') && data.containsKey('ay') &&
          data.containsKey('az'))) {
      throw const FormatException('Los tres ejes deben viajar juntos');
    }
    for (final entry in data.entries) {
      if (entry.value == null) continue;
      final value = entry.value;
      if (value is! num || !value.isFinite) {
        throw const FormatException('Métrica BLE no numérica');
      }
    }
    final reading = WatchReading(
      heartRate: _asInt(data['hr'], 30, 240),
      spo2: _asInt(data['spo2'], 1, 100),
      temperature: _asDouble(data['temp'], -70, 380),
      accelX: _asDouble(data['ax'], -16, 16),
      accelY: _asDouble(data['ay'], -16, 16),
      accelZ: _asDouble(data['az'], -16, 16),
      hasHeartRate: data.containsKey('hr'),
      hasSpo2: data.containsKey('spo2'),
      hasTemperature: data.containsKey('temp'),
      hasMotion: hasMotion,
      receivedAt: receivedAt ?? DateTime.now(),
    );
    if ((reading.hasHeartRate && data['hr'] != null && reading.heartRate == null) ||
        (reading.hasSpo2 && data['spo2'] != null && reading.spo2 == null) ||
        (reading.hasTemperature && data['temp'] != null && reading.temperature == null) ||
        (reading.hasMotion && data['ax'] != null && reading.movement == null)) {
      throw const FormatException('Métrica BLE fuera de rango');
    }
    return reading;
  }

  static int? _asInt(Object? value, int min, int max) {
    if (value is! int || value < min || value > max) {
      return null;
    }
    return value;
  }

  static double? _asDouble(Object? value, double min, double max) {
    if (value is! num || !value.isFinite || value < min || value > max) {
      return null;
    }
    return value.toDouble();
  }
}

enum WatchConnectionState {
  disconnected,
  scanning,
  connecting,
  connected,
  failed,
}

class WatchBleService {
  final StreamController<WatchReading> _readingsController =
      StreamController<WatchReading>.broadcast();
  final StreamController<WatchConnectionState> _stateController =
      StreamController<WatchConnectionState>.broadcast();
  final StreamController<String> _messageController =
      StreamController<String>.broadcast();
  final StreamController<WatchHistoryEntry> _historyController =
      StreamController<WatchHistoryEntry>.broadcast();

  BluetoothDevice? _device;
  BluetoothCharacteristic? _configCharacteristic;
  BluetoothCharacteristic? _timeCharacteristic;
  BluetoothCharacteristic? _historyCharacteristic;
  BluetoothCharacteristic? _historyControlCharacteristic;
  int _historyGeneration = 0;
  int? _androidSdk;
  StreamSubscription<List<ScanResult>>? _scanSubscription;
  StreamSubscription<BluetoothConnectionState>? _connectionSubscription;
  StreamSubscription<List<int>>? _dataSubscription;
  StreamSubscription<List<int>>? _historySubscription;

  Stream<WatchReading> get readings => _readingsController.stream;
  Stream<WatchConnectionState> get state => _stateController.stream;
  Stream<String> get messages => _messageController.stream;
  Stream<WatchHistoryEntry> get history => _historyController.stream;

  Future<void> connect() async {
    try {
      await _connectInternal();
    } catch (error) {
      // Un error de permisos o del escáner también debe salir de "Buscando".
      await _scanSubscription?.cancel();
      _scanSubscription = null;
      try {
        await FlutterBluePlus.stopScan().timeout(const Duration(seconds: 5));
      } catch (_) {
        // Conservamos el error original para mostrarlo en la app.
      }
      _stateController.add(WatchConnectionState.failed);
      _messageController.add(error is TimeoutException
          ? 'La búsqueda Bluetooth tardó demasiado. Reiniciá Bluetooth e intentá otra vez.'
          : 'Error BLE al buscar: $error');
    }
  }

  Future<void> _connectInternal() async {
    _stateController.add(WatchConnectionState.scanning);
    _messageController.add('Buscando $neraWatchName...');

    final allowed = await _requestPermissions()
        .timeout(const Duration(seconds: 30));
    if (!allowed) {
      _stateController.add(WatchConnectionState.failed);
      _messageController.add('Faltan permisos de Bluetooth.');
      return;
    }

    await disconnect().timeout(const Duration(seconds: 12));
    final adapter = await FlutterBluePlus.adapterState
        .firstWhere((state) => state != BluetoothAdapterState.unknown)
        .timeout(const Duration(seconds: 5));
    if (adapter != BluetoothAdapterState.on) {
      _stateController.add(WatchConnectionState.failed);
      _messageController.add('Activá Bluetooth en el teléfono y volvé a conectar.');
      return;
    }
    _stateController.add(WatchConnectionState.scanning);

    final completer = Completer<BluetoothDevice?>();
    final seenDevices = <String>{};
    Object? scanError;
    _scanSubscription = FlutterBluePlus.onScanResults.listen((results) {
      for (final result in results) {
        seenDevices.add(result.device.remoteId.toString());
        // FUNCIONAL: revisar ambos nombres. Android puede conservar un nombre
        // anterior; el servicio GATT se verifica siempre después de conectar.
        final matches = isNeraAdvertisement(
          advertisedName: result.advertisementData.advName,
          platformName: result.device.platformName,
          serviceUuids: result.advertisementData.serviceUuids.map((uuid) => uuid.toString()),
        );
        if (matches && !completer.isCompleted) {
          completer.complete(result.device);
          break;
        }
      }
    }, onError: (Object error) {
      scanError = error;
      if (!completer.isCompleted) completer.complete(null);
    });

    // FUNCIONAL: Android 12+ permite BLE sin ubicación (neverForLocation en
    // AndroidManifest.xml). En Android 11 o anterior sigue siendo necesaria.
    final legacyAndroid = defaultTargetPlatform == TargetPlatform.android &&
        (_androidSdk ?? 0) < 31;
    await FlutterBluePlus.startScan(
      timeout: const Duration(seconds: 12),
      // Seguir recibiendo anuncios: el nombre o UUID pueden llegar despues.
      continuousUpdates: true,
      androidUsesFineLocation: legacyAndroid,
      androidCheckLocationServices: legacyAndroid,
    )
        .timeout(const Duration(seconds: 15));
    final device = await completer.future.timeout(
      const Duration(seconds: 13),
      onTimeout: () => null,
    );
    if (scanError != null) throw scanError!;
    await FlutterBluePlus.stopScan();
    await _scanSubscription?.cancel();
    _scanSubscription = null;

    if (device == null) {
      _stateController.add(WatchConnectionState.failed);
      _messageController.add(
        seenDevices.isEmpty
            ? 'No se detectó ningún dispositivo BLE. Revisá el permiso de '
              'Dispositivos cercanos; en Android 11 o anterior, activá Ubicación.'
            : 'Se detectaron ${seenDevices.length} dispositivos BLE, pero no '
              '$neraWatchName. Reiniciá el reloj y cerrá otras apps que lo usen.',
      );
      return;
    }

    try {
      await _connectToDevice(device);
    } catch (error) {
      await disconnect();
      _stateController.add(WatchConnectionState.failed);
      _messageController.add('No pude leer datos del reloj: $error');
    }
  }

  Future<void> _connectToDevice(BluetoothDevice device) async {
    _device = device;
    _stateController.add(WatchConnectionState.connecting);
    _messageController.add('Conectando con el reloj...');

    _connectionSubscription = device.connectionState.listen((state) {
      if (state == BluetoothConnectionState.disconnected) {
        _stateController.add(WatchConnectionState.disconnected);
        _messageController.add('Reloj desconectado.');
      }
    });

    await device.connect(
      license: License.nonprofit,
      timeout: const Duration(seconds: 12),
    );
    // El reloj exige cifrado para habilitar notificaciones.
    if (defaultTargetPlatform == TargetPlatform.android) {
      await device.createBond(timeout: 45);
    }
    final services = await device.discoverServices();
    final targetService = services.where((service) {
      return service.uuid.toString().toLowerCase() == neraServiceUuid;
    }).firstOrNull;

    if (targetService == null) {
      throw Exception('El reloj no tiene el servicio BLE esperado.');
    }

    final dataCharacteristic = targetService.characteristics.where((item) {
      return item.uuid.toString().toLowerCase() == neraDataCharacteristicUuid;
    }).firstOrNull;
    _configCharacteristic = targetService.characteristics.where((item) =>
        item.uuid.toString().toLowerCase() == neraConfigCharacteristicUuid).firstOrNull;
    _timeCharacteristic = targetService.characteristics.where((item) =>
        item.uuid.toString().toLowerCase() == neraTimeCharacteristicUuid).firstOrNull;
    _historyCharacteristic = targetService.characteristics.where((item) =>
        item.uuid.toString().toLowerCase() == neraHistoryCharacteristicUuid).firstOrNull;
    _historyControlCharacteristic = targetService.characteristics.where((item) =>
        item.uuid.toString().toLowerCase() == neraHistoryControlUuid).firstOrNull;

    if (dataCharacteristic == null) {
      throw Exception('El reloj no tiene la caracteristica de datos esperada.');
    }

    _dataSubscription = dataCharacteristic.onValueReceived.listen((value) {
      try {
        final text = utf8.decode(value);
        _readingsController.add(WatchReading.fromJsonText(text));
      } catch (_) {
        _messageController.add('Dato BLE inválido; se ignoró la lectura.');
      }
    });
    await dataCharacteristic.setNotifyValue(true);
    var timeStatus = 'Hora sin sincronizar.';
    if (_timeCharacteristic != null) {
      try {
        await syncTime();
        timeStatus = 'Hora sincronizada con el teléfono.';
      } catch (_) {
        timeStatus = 'No se pudo sincronizar la hora del reloj.';
      }
    }
    _stateController.add(WatchConnectionState.connected);
    _messageController.add('Reloj conectado. $timeStatus Esperando lecturas reales.');
  }

  Future<void> syncTime() async {
    final characteristic = _timeCharacteristic;
    if (characteristic == null) throw StateError('Reloj sin característica de hora.');
    final sent = WatchTimePacket.fromPhone(DateTime.now());
    await characteristic.write(sent.encode(), withoutResponse: false)
        .timeout(const Duration(seconds: 5));
    final response = WatchTimePacket.decode(
        await characteristic.read().timeout(const Duration(seconds: 5)));
    final now = WatchTimePacket.fromPhone(DateTime.now());
    if (response.utcOffsetMinutes != sent.utcOffsetMinutes ||
        (response.epochSeconds - now.epochSeconds).abs() > 3) {
      throw StateError('El reloj no confirmó la hora.');
    }
  }

  Future<int> requestHistory({required int knownGeneration,
      required int lastSequence}) async {
    final history = _historyCharacteristic;
    final control = _historyControlCharacteristic;
    if (history == null || control == null) {
      throw StateError('El reloj no ofrece historial sincronizable.');
    }
    final status = await control.read().timeout(const Duration(seconds: 5));
    if (status.length != 8) throw const FormatException('Estado de historial inválido');
    final bytes = ByteData.sublistView(Uint8List.fromList(status));
    final latest = bytes.getUint32(0, Endian.little);
    final generation = bytes.getUint32(4, Endian.little);
    if (generation == 0) throw const FormatException('Generación de historial inválida');
    _historyGeneration = generation;
    await _historySubscription?.cancel();
    _historySubscription = history.onValueReceived.listen((value) {
      try {
        _historyController.add(
            WatchHistoryEntry.fromBytes(value, _historyGeneration));
      } catch (_) {
        _messageController.add('Registro de historial inválido; se ignoró.');
      }
    });
    await history.setNotifyValue(true);
    final cursor = knownGeneration == generation && lastSequence <= latest
        ? lastSequence : 0;
    final request = ByteData(4)..setUint32(0, cursor, Endian.little);
    await control.write(request.buffer.asUint8List(), withoutResponse: false)
        .timeout(const Duration(seconds: 5));
    return generation;
  }

  Future<void> disconnect() async {
    _configCharacteristic = null;
    _timeCharacteristic = null;
    _historyCharacteristic = null;
    _historyControlCharacteristic = null;
    _historyGeneration = 0;
    await _scanSubscription?.cancel();
    await _connectionSubscription?.cancel();
    await _dataSubscription?.cancel();
    await _historySubscription?.cancel();
    _scanSubscription = null;
    _connectionSubscription = null;
    _dataSubscription = null;
    _historySubscription = null;
    await FlutterBluePlus.stopScan();
    await _device?.disconnect();
    _device = null;
    _stateController.add(WatchConnectionState.disconnected);
  }

  // FUNCIONAL: escritura cifrada y lectura de confirmacion persistida.
  Future<void> applyPreferences(UserPreferences preferences) async {
    final characteristic = _configCharacteristic;
    if (characteristic == null) {
      throw StateError('El reloj necesita el firmware con configuracion personal.');
    }
    final bytes = preferences.wire;
    await characteristic.write(bytes, withoutResponse: false).timeout(const Duration(seconds: 5));
    for (var attempt = 0; attempt < 20; attempt++) {
      await Future<void>.delayed(const Duration(milliseconds: 250));
      if (_configCharacteristic != characteristic) throw StateError('Reloj desconectado.');
      final confirmed = await characteristic.read().timeout(const Duration(seconds: 3));
      if (confirmed.length == 5 && confirmed[4] == 0 &&
          List.generate(4, (i) => confirmed[i] == bytes[i]).every((same) => same)) {
        return;
      }
    }
    throw StateError('El reloj no confirmo el guardado. Reintentá la sincronizacion.');
  }

  Future<bool> _requestPermissions() async {
    if (defaultTargetPlatform != TargetPlatform.android) return true;
    final sdk = await const MethodChannel('nera/platform').invokeMethod<int>('androidSdk');
    if (sdk == null) return false;
    _androidSdk = sdk;
    // Android 12+ usa permisos BLE. En Android 11 o menor, el escaneo
    // necesita ubicación; el manifiesto limita ese permiso a API 30.
    final required = sdk >= 31
        ? [Permission.bluetoothScan, Permission.bluetoothConnect]
        : [Permission.locationWhenInUse];
    final statuses = await required.request();

    return statuses.values.every((status) {
      return status.isGranted || status.isLimited;
    });
  }

  Future<void> dispose() async {
    await disconnect();
    await _readingsController.close();
    await _stateController.close();
    await _messageController.close();
    await _historyController.close();
  }
}
