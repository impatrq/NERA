import 'dart:typed_data';

const String neraTimeCharacteristicUuid =
    '7e400005-b5a3-f393-e0a9-e50e24dcca9e';

class WatchTimePacket {
  const WatchTimePacket(this.epochSeconds, this.utcOffsetMinutes);

  final int epochSeconds;
  final int utcOffsetMinutes;

  static WatchTimePacket fromPhone(DateTime now) => WatchTimePacket(
      now.millisecondsSinceEpoch ~/ 1000, now.timeZoneOffset.inMinutes);

  List<int> encode() {
    if (epochSeconds < 1704067200 || epochSeconds >= 4102444800 ||
        utcOffsetMinutes < -720 || utcOffsetMinutes > 840) {
      throw const FormatException('Hora fuera de rango');
    }
    final data = ByteData(7);
    data.setUint8(0, 1);
    data.setUint32(1, epochSeconds, Endian.little);
    data.setInt16(5, utcOffsetMinutes, Endian.little);
    return data.buffer.asUint8List();
  }

  factory WatchTimePacket.decode(List<int> bytes) {
    if (bytes.length != 7 || bytes[0] != 1) {
      throw const FormatException('Respuesta de hora incompatible');
    }
    final data = ByteData.sublistView(Uint8List.fromList(bytes));
    return WatchTimePacket(data.getUint32(1, Endian.little),
        data.getInt16(5, Endian.little));
  }
}
