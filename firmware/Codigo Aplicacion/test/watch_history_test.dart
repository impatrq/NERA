import 'dart:typed_data';

import 'package:aplicacion_nera/watch_history.dart';
import 'package:aplicacion_nera/watch_time_protocol.dart';
import 'package:flutter_test/flutter_test.dart';

List<int> pointBytes({int sequence = 7, int epoch = 1794140000,
    int bpm = 76, int temp10 = 295}) {
  final data = ByteData(20);
  data.setUint32(0, sequence, Endian.little);
  data.setUint32(4, epoch, Endian.little);
  data.setUint32(8, 3600, Endian.little);
  data.setInt16(12, bpm, Endian.little);
  data.setInt16(14, temp10, Endian.little);
  final bytes = data.buffer.asUint8List();
  var hash = 2166136261;
  for (var i = 0; i < 16; i++) {
    hash = ((hash ^ bytes[i]) * 16777619) & 0xffffffff;
  }
  data.setUint32(16, hash, Endian.little);
  return bytes;
}

void main() {
  test('hora BLE v1 usa segundos UTC y desplazamiento con signo', () {
    final packet = WatchTimePacket(1794140000, -180);
    expect(WatchTimePacket.decode(packet.encode()).epochSeconds, 1794140000);
    expect(WatchTimePacket.decode(packet.encode()).utcOffsetMinutes, -180);
    expect(() => WatchTimePacket.decode([1, 2]), throwsFormatException);
    expect(() => const WatchTimePacket(1, 0).encode(), throwsFormatException);
  });

  test('historial BLE valida el checksum, conserva hora desconocida y se guarda', () {
    final entry = WatchHistoryEntry.fromBytes(pointBytes(), 123);
    expect(entry.sequence, 7);
    expect(entry.bpm, 76);
    expect(entry.objectCelsius, 29.5);
    expect(entry.localTime, isNotNull);

    final unknown = WatchHistoryEntry.fromBytes(
        pointBytes(sequence: 8, epoch: 0, bpm: 0), 123);
    expect(unknown.localTime, isNull);
    expect(unknown.bpm, isNull);
    final archive = WatchHistoryArchive([entry, unknown]);
    final restored = WatchHistoryArchive.decode(archive.encode());
    expect(restored.entries.length, 2);
    expect(restored.entries.last.localTime, isNull);

    final damaged = pointBytes()..[12] = 99;
    expect(() => WatchHistoryEntry.fromBytes(damaged, 123), throwsFormatException);
  });
}
