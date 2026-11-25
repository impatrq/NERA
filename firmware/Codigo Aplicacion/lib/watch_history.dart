import 'dart:convert';
import 'dart:typed_data';

const String neraHistoryCharacteristicUuid =
    '7e400006-b5a3-f393-e0a9-e50e24dcca9e';
const String neraHistoryControlUuid =
    '7e400007-b5a3-f393-e0a9-e50e24dcca9e';

class WatchHistoryEntry {
  const WatchHistoryEntry({required this.generation, required this.sequence,
    required this.epochSeconds, required this.bpm, required this.objectDeciC});

  final int generation;
  final int sequence;
  final int epochSeconds;
  final int? bpm;
  final int? objectDeciC;

  DateTime? get localTime => epochSeconds == 0 ? null :
      DateTime.fromMillisecondsSinceEpoch(epochSeconds * 1000).toLocal();
  double? get objectCelsius => objectDeciC == null ? null : objectDeciC! / 10;

  factory WatchHistoryEntry.fromBytes(List<int> bytes, int generation) {
    if (bytes.length != 20 || generation <= 0) {
      throw const FormatException('Registro de historial incompatible');
    }
    final data = ByteData.sublistView(Uint8List.fromList(bytes));
    var hash = 2166136261;
    for (var i = 0; i < 16; i++) {
      hash = ((hash ^ bytes[i]) * 16777619) & 0xffffffff;
    }
    if (data.getUint32(16, Endian.little) != hash) {
      throw const FormatException('Registro de historial dañado');
    }
    final sequence = data.getUint32(0, Endian.little);
    final epoch = data.getUint32(4, Endian.little);
    final rawBpm = data.getInt16(12, Endian.little);
    final rawTemp = data.getInt16(14, Endian.little);
    if (sequence == 0 || (epoch != 0 &&
        (epoch < 1704067200 || epoch >= 4102444800)) ||
        (rawBpm != 0 && (rawBpm < 30 || rawBpm > 240)) ||
        (rawTemp != -32768 && (rawTemp < -700 || rawTemp > 3800))) {
      throw const FormatException('Registro de historial fuera de rango');
    }
    return WatchHistoryEntry(generation: generation, sequence: sequence,
      epochSeconds: epoch, bpm: rawBpm == 0 ? null : rawBpm,
      objectDeciC: rawTemp == -32768 ? null : rawTemp);
  }

  Map<String, Object?> toJson() => {'g': generation, 's': sequence,
    't': epochSeconds, 'hr': bpm, 'temp10': objectDeciC};

  factory WatchHistoryEntry.fromJson(Map<String, dynamic> data) {
    final generation = data['g'], sequence = data['s'], epoch = data['t'];
    final bpm = data['hr'], temp = data['temp10'];
    if (generation is! int || generation <= 0 || sequence is! int || sequence <= 0 ||
        epoch is! int || epoch < 0 || epoch >= 4102444800 ||
        (bpm != null && (bpm is! int || bpm < 30 || bpm > 240)) ||
        (temp != null && (temp is! int || temp < -700 || temp > 3800))) {
      throw const FormatException('Historial guardado incompatible');
    }
    return WatchHistoryEntry(generation: generation, sequence: sequence,
      epochSeconds: epoch, bpm: bpm, objectDeciC: temp);
  }
}

class WatchHistoryArchive {
  const WatchHistoryArchive(this.entries);
  final List<WatchHistoryEntry> entries;

  String encode() => jsonEncode({'version': 1,
    'entries': [for (final entry in entries) entry.toJson()]});

  factory WatchHistoryArchive.decode(String text) {
    final data = jsonDecode(text);
    if (data is! Map<String, dynamic> || data['version'] != 1 ||
        data['entries'] is! List) {
      throw const FormatException('Historial guardado incompatible');
    }
    final entries = <WatchHistoryEntry>[];
    for (final item in data['entries'] as List) {
      if (item is! Map<String, dynamic>) {
        throw const FormatException('Historial guardado incompatible');
      }
      entries.add(WatchHistoryEntry.fromJson(item));
    }
    return WatchHistoryArchive(entries);
  }
}
