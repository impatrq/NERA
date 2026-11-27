import 'dart:typed_data';

const neraAlertCharacteristicUuid = '7e400008-b5a3-f393-e0a9-e50e24dcca9e';

enum AlertDeliveryPhase { waitingApp, sending, confirmed, failed }

class WatchAlert {
  const WatchAlert({
    required this.metric,
    required this.state,
    required this.session,
    required this.id,
    required this.centiValue,
    required this.phase,
  });
  final int metric, state, session, id, centiValue;
  final AlertDeliveryPhase phase;
  bool get active => state == 1;
  String get key => '$session:$metric:$id';
  String get label => metric == 1 ? 'Pulso' : 'Temperatura IR';
  String get value => metric == 1
      ? '${(centiValue / 100).toStringAsFixed(0)} BPM'
      : '${(centiValue / 100).toStringAsFixed(2)} °C';

  factory WatchAlert.decode(List<int> bytes) {
    if (bytes.length != 20 ||
        bytes[0] != 78 ||
        bytes[1] != 69 ||
        bytes[2] != 82 ||
        bytes[3] != 65 ||
        bytes[4] != 3 ||
        ![4, 6, 7, 8].contains(bytes[5]) ||
        bytes[6] < 1 ||
        bytes[6] > 2 ||
        bytes[7] > 2) {
      throw const FormatException('Evento de alerta inválido');
    }
    final data = ByteData.sublistView(Uint8List.fromList(bytes));
    final session = data.getUint32(8, Endian.little);
    final id = data.getUint32(12, Endian.little);
    if (session == 0 || id == 0) {
      throw const FormatException('Evento sin identidad');
    }
    return WatchAlert(
      metric: bytes[6],
      state: bytes[7],
      session: session,
      id: id,
      centiValue: data.getInt32(16, Endian.little),
      phase: switch (bytes[5]) {
        4 => AlertDeliveryPhase.waitingApp,
        6 => AlertDeliveryPhase.sending,
        7 => AlertDeliveryPhase.confirmed,
        _ => AlertDeliveryPhase.failed,
      },
    );
  }

  List<int> receipt() => _packet(5, metric, state, session, id, centiValue);
  static List<int> phoneReady() => _packet(9, 1, 0, 0, 0, 0);

  static List<int> _packet(
    int type,
    int metric,
    int state,
    int session,
    int id,
    int value,
  ) {
    final data = ByteData(20);
    final bytes = data.buffer.asUint8List();
    bytes.setAll(0, [78, 69, 82, 65, 3, type, metric, state]);
    data.setUint32(8, session, Endian.little);
    data.setUint32(12, id, Endian.little);
    data.setInt32(16, value, Endian.little);
    return bytes;
  }
}
