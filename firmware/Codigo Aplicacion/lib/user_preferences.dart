import 'dart:convert';
import 'package:flutter/services.dart';
import 'models.dart';

// MODIFICABLE: textos en Perfil. La sensibilidad cambia duracion, no BPM/grados.
enum AlertSensitivity { high, medium, low }
enum ActivityMode { everyday, exercise }

class UserPreferences {
  const UserPreferences({this.alerts = true, this.sensitivity = AlertSensitivity.high,
    this.activity = ActivityMode.everyday, this.name = 'Usuario NERA',
    this.email = 'usuario@nera.app', this.contacts = const []});
  final bool alerts;
  final AlertSensitivity sensitivity;
  final ActivityMode activity;
  final String name, email;
  final List<EmergencyContact> contacts;
  int get confirmationSeconds => const [5, 10, 20][sensitivity.index];
  List<int> get wire => [1, alerts ? 1 : 0, sensitivity.index, activity.index];

  String encode() => jsonEncode({'version': 1, 'alerts': alerts,
    'sensitivity': sensitivity.index, 'activity': activity.index,
    'name': name, 'email': email, 'contacts': [for (final c in contacts)
      {'name': c.name, 'relation': c.relation, 'phone': c.phone, 'active': c.active}]});

  // FUNCIONAL: rechazar formatos incompatibles, no convertir datos corruptos
  // en una confirmacion de ajustes. El controlador informa el error.
  factory UserPreferences.decode(String text) {
    final data = jsonDecode(text);
    if (data is! Map<String, dynamic> || data['version'] != 1 ||
        data['alerts'] is! bool || data['name'] is! String || data['email'] is! String ||
        data['sensitivity'] is! int || data['sensitivity'] < 0 || data['sensitivity'] > 2 ||
        data['activity'] is! int || data['activity'] < 0 || data['activity'] > 1 ||
        data['contacts'] is! List) {
      throw const FormatException('Preferencias incompatibles');
    }
    final contacts = <EmergencyContact>[];
    for (final c in data['contacts']) {
      if (c is! Map || c['name'] is! String || c['relation'] is! String ||
          c['phone'] is! String || c['active'] is! bool) {
        throw const FormatException('Contacto incompatible');
      }
      contacts.add(EmergencyContact(name: c['name'], relation: c['relation'],
        phone: c['phone'], active: c['active']));
    }
    return UserPreferences(alerts: data['alerts'],
      sensitivity: AlertSensitivity.values[data['sensitivity']],
      activity: ActivityMode.values[data['activity']], name: data['name'],
      email: data['email'], contacts: contacts);
  }
}

// FUNCIONAL: almacenamiento privado Android, sin agregar dependencias.
// Los contactos quedan en el telefono; no se envian por BLE ni a terceros.
class UserPreferencesStore {
  static const _platform = MethodChannel('nera/platform');
  Future<UserPreferences> load() async {
    final text = await _platform.invokeMethod<String>('loadUserPreferences');
    return text == null ? const UserPreferences() : UserPreferences.decode(text);
  }
  Future<void> save(UserPreferences preferences) async {
    final saved = await _platform.invokeMethod<bool>('saveUserPreferences', preferences.encode());
    if (saved != true) throw PlatformException(code: 'save_failed');
  }
}
