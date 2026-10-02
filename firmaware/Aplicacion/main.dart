import 'dart:math';

import 'package:flutter/material.dart';

import 'watch_ble_service.dart';

// GUIA RAPIDA:
// Busca "CAMBIAR" con Ctrl + F para encontrar colores, textos y valores faciles
// de modificar. La app esta organizada en pantallas: Inicio, Salud, Apoyo y Perfil.

void main() {
  runApp(const NeraApp());
}

class AppColors {
  // CAMBIAR: color principal de marca. Afecta botones, iconos activos y detalles.
  static const primary = Color(0xFF0E9384);

  // CAMBIAR: color secundario para detalles suaves.
  static const secondary = Color(0xFF2563EB);

  // CAMBIAR: color de fondo general de todas las pantallas.
  static const background = Color(0xFFF5F8FA);

  // CAMBIAR: color de tarjetas.
  static const card = Colors.white;

  // CAMBIAR: color de texto principal.
  static const text = Color(0xFF102033);

  // CAMBIAR: color de textos secundarios.
  static const mutedText = Color(0xFF667085);

  // CAMBIAR: color de alerta/emergencia.
  static const danger = Color(0xFFE5484D);

  // CAMBIAR: color para estados correctos o tranquilos.
  static const success = Color(0xFF12B76A);
}

class EmergencyContact {
  const EmergencyContact({
    required this.name,
    required this.relation,
    required this.phone,
    required this.active,
  });

  final String name;
  final String relation;
  final String phone;
  final bool active;

  EmergencyContact copyWith({
    String? name,
    String? relation,
    String? phone,
    bool? active,
  }) {
    return EmergencyContact(
      name: name ?? this.name,
      relation: relation ?? this.relation,
      phone: phone ?? this.phone,
      active: active ?? this.active,
    );
  }
}

class AlertEvent {
  const AlertEvent({
    required this.title,
    required this.description,
    required this.time,
    required this.icon,
    required this.color,
  });

  final String title;
  final String description;
  final DateTime time;
  final IconData icon;
  final Color color;
}

class NeraApp extends StatelessWidget {
  const NeraApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      // CAMBIAR: nombre interno de la app.
      title: 'NERA',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        useMaterial3: true,
        brightness: Brightness.light,
        scaffoldBackgroundColor: AppColors.background,
        colorScheme: ColorScheme.fromSeed(
          seedColor: AppColors.primary,
          brightness: Brightness.light,
          primary: AppColors.primary,
          secondary: AppColors.secondary,
          surface: AppColors.card,
          error: AppColors.danger,
        ),
        fontFamily: 'Roboto',
        textTheme: const TextTheme(
          headlineMedium: TextStyle(
            color: AppColors.text,
            fontSize: 28,
            fontWeight: FontWeight.w800,
            letterSpacing: 0,
          ),
          titleLarge: TextStyle(
            color: AppColors.text,
            fontSize: 20,
            fontWeight: FontWeight.w800,
            letterSpacing: 0,
          ),
          titleMedium: TextStyle(
            color: AppColors.text,
            fontSize: 16,
            fontWeight: FontWeight.w700,
            letterSpacing: 0,
          ),
          bodyMedium: TextStyle(color: AppColors.mutedText, fontSize: 14),
        ),
        cardTheme: CardThemeData(
          color: AppColors.card,
          elevation: 0,
          margin: EdgeInsets.zero,
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(24),
            side: const BorderSide(color: Color(0xFFE4E7EC)),
          ),
        ),
        filledButtonTheme: FilledButtonThemeData(
          style: FilledButton.styleFrom(
            minimumSize: const Size(0, 52),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(16),
            ),
            textStyle: const TextStyle(
              fontSize: 15,
              fontWeight: FontWeight.w800,
            ),
          ),
        ),
        outlinedButtonTheme: OutlinedButtonThemeData(
          style: OutlinedButton.styleFrom(
            minimumSize: const Size(0, 52),
            side: const BorderSide(color: Color(0xFFD0D5DD)),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(16),
            ),
            textStyle: const TextStyle(
              fontSize: 15,
              fontWeight: FontWeight.w800,
            ),
          ),
        ),
        navigationBarTheme: NavigationBarThemeData(
          height: 72,
          elevation: 0,
          backgroundColor: Colors.white,
          indicatorColor: AppColors.primary.withValues(alpha: 0.12),
          labelTextStyle: WidgetStateProperty.all(
            const TextStyle(fontSize: 12, fontWeight: FontWeight.w800),
          ),
        ),
        inputDecorationTheme: InputDecorationTheme(
          filled: true,
          fillColor: const Color(0xFFF9FAFB),
          border: OutlineInputBorder(
            borderRadius: BorderRadius.circular(16),
            borderSide: const BorderSide(color: Color(0xFFD0D5DD)),
          ),
          enabledBorder: OutlineInputBorder(
            borderRadius: BorderRadius.circular(16),
            borderSide: const BorderSide(color: Color(0xFFD0D5DD)),
          ),
        ),
      ),
      home: const NeraShell(),
    );
  }
}

class NeraShell extends StatefulWidget {
  const NeraShell({super.key});

  @override
  State<NeraShell> createState() => _NeraShellState();
}

class _NeraShellState extends State<NeraShell> {
  int _selectedIndex = 0;
  final WatchBleService _watchService = WatchBleService();

  // CAMBIAR: datos iniciales del perfil.
  String _name = 'Usuario NERA';
  String _email = 'usuario@nera.app';

  // CAMBIAR: valores iniciales de salud.
  int _heartRate = 76;
  int _stress = 24;
  int _oxygen = 98;
  double _temperature = 36.6;
  double _movement = 1.0;
  double _sleepHours = 7.4;
  WatchConnectionState _watchState = WatchConnectionState.disconnected;
  String _watchMessage = 'Reloj no conectado';

  // CAMBIAR: switches iniciales de ajustes.
  bool _automaticAlerts = true;
  bool _shareLocation = true;
  bool _healthNotifications = true;
  bool _calmMode = false;

  // CAMBIAR: contactos iniciales de Red de apoyo.
  final List<EmergencyContact> _contacts = <EmergencyContact>[
    const EmergencyContact(
      name: 'Mama',
      relation: 'Familia',
      phone: '+54 9 11 5555-0101',
      active: true,
    ),
    const EmergencyContact(
      name: 'Sofia',
      relation: 'Amiga',
      phone: '+54 9 11 5555-0198',
      active: true,
    ),
  ];

  final List<AlertEvent> _events = <AlertEvent>[
    AlertEvent(
      title: 'Chequeo matutino',
      description: 'Estado estable y descanso dentro del rango esperado.',
      time: DateTime.now().subtract(const Duration(hours: 2)),
      icon: Icons.check_circle,
      color: AppColors.success,
    ),
    AlertEvent(
      title: 'Respiracion guiada',
      description: 'Rutina de calma completada.',
      time: DateTime.now().subtract(const Duration(hours: 5)),
      icon: Icons.self_improvement,
      color: AppColors.primary,
    ),
  ];

  @override
  void initState() {
    super.initState();
    _watchService.state.listen((state) {
      if (!mounted) return;
      setState(() => _watchState = state);
    });
    _watchService.messages.listen((message) {
      if (!mounted) return;
      setState(() => _watchMessage = message);
    });
    _watchService.readings.listen(_applyWatchReading);
  }

  @override
  void dispose() {
    _watchService.dispose();
    super.dispose();
  }

  void _applyWatchReading(WatchReading reading) {
    if (!mounted) return;
    setState(() {
      _heartRate = reading.heartRate;
      _oxygen = reading.spo2;
      _temperature = reading.temperature;
      _movement = reading.movement;
      _stress = reading.estimatedStress;
      _events.insert(
        0,
        AlertEvent(
          title: 'Lectura del reloj',
          description:
              'Pulso ${reading.heartRate} bpm, SpO2 ${reading.spo2}%, temp ${reading.temperature.toStringAsFixed(1)} C.',
          time: DateTime.now(),
          icon: Icons.watch,
          color: AppColors.primary,
        ),
      );
    });
  }

  void _runMeasurement() {
    if (_watchState != WatchConnectionState.connected) {
      _connectWatch();
      return;
    }
    final random = Random();
    setState(() {
      _heartRate = 64 + random.nextInt(24);
      _stress = 12 + random.nextInt(42);
      _oxygen = 96 + random.nextInt(4);
      _sleepHours = 6.2 + random.nextDouble() * 2.5;
      _events.insert(
        0,
        AlertEvent(
          title: 'Medicion actualizada',
          description: 'Se recalcularon pulso, oxigeno, estres y sueno.',
          time: DateTime.now(),
          icon: Icons.sensors,
          color: AppColors.secondary,
        ),
      );
    });
    _showSnack('Medicion actualizada.');
  }

  Future<void> _connectWatch() async {
    if (_watchState == WatchConnectionState.connected) {
      await _watchService.disconnect();
      return;
    }
    await _watchService.connect();
  }

  void _sendEmergencyAlert() {
    final activeContacts = _contacts
        .where((contact) => contact.active)
        .toList();
    final message =
        'Alerta enviada a ${activeContacts.length} contacto${activeContacts.length == 1 ? '' : 's'}.';

    setState(() {
      _events.insert(
        0,
        AlertEvent(
          title: 'Alerta de emergencia',
          description: activeContacts.isEmpty
              ? 'No habia contactos activos para avisar.'
              : message,
          time: DateTime.now(),
          icon: Icons.notification_important,
          color: AppColors.danger,
        ),
      );
    });

    showDialog<void>(
      context: context,
      builder: (context) => AlertDialog(
        icon: const Icon(
          Icons.notification_important,
          color: AppColors.danger,
          size: 34,
        ),
        title: const Text('Protocolo de ayuda activado'),
        content: Text(
          activeContacts.isEmpty
              ? 'Agrega al menos un contacto activo para poder enviar alertas.'
              : '$message\n\nSe compartio tu estado actual y ubicacion aproximada.',
        ),
        actions: [
          FilledButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Entendido'),
          ),
        ],
      ),
    );
  }

  void _startBreathingSession() {
    setState(() {
      _stress = max(5, _stress - 8);
      _events.insert(
        0,
        AlertEvent(
          title: 'Modo calma iniciado',
          description: 'Respiracion guiada para reducir senales de estres.',
          time: DateTime.now(),
          icon: Icons.self_improvement,
          color: AppColors.primary,
        ),
      );
    });
    showModalBottomSheet<void>(
      context: context,
      showDragHandle: true,
      backgroundColor: Colors.white,
      builder: (context) => const BreathingSheet(),
    );
  }

  void _addContact(EmergencyContact contact) {
    setState(() => _contacts.add(contact));
    _showSnack('Contacto agregado.');
  }

  void _updateContact(int index, EmergencyContact contact) {
    setState(() => _contacts[index] = contact);
  }

  void _removeContact(int index) {
    setState(() => _contacts.removeAt(index));
    _showSnack('Contacto eliminado.');
  }

  void _updateProfile(String name, String email) {
    setState(() {
      _name = name;
      _email = email;
    });
    _showSnack('Perfil actualizado.');
  }

  void _showSnack(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        behavior: SnackBarBehavior.floating,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(14)),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final pages = [
      HomeScreen(
        name: _name,
        heartRate: _heartRate,
        stress: _stress,
        oxygen: _oxygen,
        temperature: _temperature,
        movement: _movement,
        sleepHours: _sleepHours,
        contactsCount: _contacts.where((contact) => contact.active).length,
        watchState: _watchState,
        watchMessage: _watchMessage,
        onMeasure: _runMeasurement,
        onWatchConnect: _connectWatch,
        onEmergency: _sendEmergencyAlert,
        onBreathing: _startBreathingSession,
      ),
      HealthScreen(
        heartRate: _heartRate,
        stress: _stress,
        oxygen: _oxygen,
        temperature: _temperature,
        movement: _movement,
        sleepHours: _sleepHours,
        events: _events,
        watchState: _watchState,
        watchMessage: _watchMessage,
        onWatchConnect: _connectWatch,
        onBreathing: _startBreathingSession,
      ),
      SupportScreen(
        contacts: _contacts,
        onAddContact: _addContact,
        onUpdateContact: _updateContact,
        onRemoveContact: _removeContact,
        onSendAlert: _sendEmergencyAlert,
      ),
      ProfileScreen(
        name: _name,
        email: _email,
        automaticAlerts: _automaticAlerts,
        shareLocation: _shareLocation,
        healthNotifications: _healthNotifications,
        calmMode: _calmMode,
        onProfileChanged: _updateProfile,
        onAutomaticAlertsChanged: (value) =>
            setState(() => _automaticAlerts = value),
        onShareLocationChanged: (value) =>
            setState(() => _shareLocation = value),
        onHealthNotificationsChanged: (value) =>
            setState(() => _healthNotifications = value),
        onCalmModeChanged: (value) => setState(() => _calmMode = value),
      ),
    ];

    return Scaffold(
      body: SafeArea(child: pages[_selectedIndex]),
      bottomNavigationBar: NavigationBar(
        selectedIndex: _selectedIndex,
        onDestinationSelected: (index) =>
            setState(() => _selectedIndex = index),
        destinations: const [
          NavigationDestination(
            icon: Icon(Icons.dashboard_outlined),
            selectedIcon: Icon(Icons.dashboard),
            label: 'Inicio',
          ),
          NavigationDestination(
            icon: Icon(Icons.monitor_heart_outlined),
            selectedIcon: Icon(Icons.monitor_heart),
            label: 'Salud',
          ),
          NavigationDestination(
            icon: Icon(Icons.groups_outlined),
            selectedIcon: Icon(Icons.groups),
            label: 'Apoyo',
          ),
          NavigationDestination(
            icon: Icon(Icons.person_outline),
            selectedIcon: Icon(Icons.person),
            label: 'Perfil',
          ),
        ],
      ),
    );
  }
}

class HomeScreen extends StatelessWidget {
  const HomeScreen({
    super.key,
    required this.name,
    required this.heartRate,
    required this.stress,
    required this.oxygen,
    required this.temperature,
    required this.movement,
    required this.sleepHours,
    required this.contactsCount,
    required this.watchState,
    required this.watchMessage,
    required this.onMeasure,
    required this.onWatchConnect,
    required this.onEmergency,
    required this.onBreathing,
  });

  final String name;
  final int heartRate;
  final int stress;
  final int oxygen;
  final double temperature;
  final double movement;
  final double sleepHours;
  final int contactsCount;
  final WatchConnectionState watchState;
  final String watchMessage;
  final VoidCallback onMeasure;
  final VoidCallback onWatchConnect;
  final VoidCallback onEmergency;
  final VoidCallback onBreathing;

  @override
  Widget build(BuildContext context) {
    final score =
        ((100 - stress) * 0.42 + oxygen * 0.36 + sleepHours * 9 * 0.22)
            .round()
            .clamp(0, 100);

    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 16, 20, 24),
      children: [
        AppHeader(
          title: 'Hola, ${firstName(name)}',
          subtitle: 'Resumen claro de tu bienestar de hoy.',
          action: IconButton.filled(
            style: IconButton.styleFrom(backgroundColor: AppColors.danger),
            onPressed: onEmergency,
            tooltip: 'Enviar alerta SOS',
            icon: const Icon(Icons.sos),
          ),
        ),
        const SizedBox(height: 18),
        WellnessCard(
          score: score,
          stress: stress,
          contactsCount: contactsCount,
          onMeasure: onMeasure,
          onBreathing: onBreathing,
        ),
        const SizedBox(height: 18),
        WatchStatusCard(
          state: watchState,
          message: watchMessage,
          onConnect: onWatchConnect,
        ),
        const SizedBox(height: 18),
        Row(
          children: [
            Expanded(
              child: QuickActionCard(
                icon: Icons.sensors,
                title: watchState == WatchConnectionState.connected
                    ? 'Leer reloj'
                    : 'Conectar reloj',
                subtitle: watchState == WatchConnectionState.connected
                    ? 'Esperar sensores'
                    : 'Bluetooth BLE',
                color: AppColors.secondary,
                onTap: onMeasure,
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: QuickActionCard(
                icon: Icons.self_improvement,
                title: 'Modo calma',
                subtitle: 'Respirar 4-4-6',
                color: AppColors.primary,
                onTap: onBreathing,
              ),
            ),
          ],
        ),
        const SizedBox(height: 22),
        const SectionTitle(title: 'Senales principales'),
        const SizedBox(height: 12),
        MetricGrid(
          metrics: [
            MetricData(
              title: 'Pulso',
              value: '$heartRate',
              unit: 'bpm',
              icon: Icons.favorite,
              color: const Color(0xFFEF4444),
              detail: 'Frecuencia cardiaca estimada en la ultima medicion.',
            ),
            MetricData(
              title: 'Oxigeno',
              value: '$oxygen',
              unit: '%',
              icon: Icons.air,
              color: AppColors.success,
              detail: 'Saturacion estimada registrada por la app.',
            ),
            MetricData(
              title: 'Estres',
              value: '$stress',
              unit: '%',
              icon: Icons.psychology,
              color: const Color(0xFFF59E0B),
              detail: 'Nivel aproximado segun las senales actuales.',
            ),
            MetricData(
              title: 'Sueno',
              value: sleepHours.toStringAsFixed(1),
              unit: 'h',
              icon: Icons.bedtime,
              color: const Color(0xFF6366F1),
              detail: 'Horas del ultimo descanso registrado.',
            ),
            MetricData(
              title: 'Temp.',
              value: temperature.toStringAsFixed(1),
              unit: 'C',
              icon: Icons.thermostat,
              color: const Color(0xFFEC4899),
              detail: 'Temperatura corporal recibida desde el MLX90614.',
            ),
            MetricData(
              title: 'Mov.',
              value: movement.toStringAsFixed(2),
              unit: 'g',
              icon: Icons.directions_run,
              color: const Color(0xFF8B5CF6),
              detail: 'Movimiento estimado desde el acelerometro MPU6050.',
            ),
          ],
        ),
        const SizedBox(height: 18),
        EmergencyBanner(onEmergency: onEmergency),
      ],
    );
  }
}

class HealthScreen extends StatelessWidget {
  const HealthScreen({
    super.key,
    required this.heartRate,
    required this.stress,
    required this.oxygen,
    required this.temperature,
    required this.movement,
    required this.sleepHours,
    required this.events,
    required this.watchState,
    required this.watchMessage,
    required this.onWatchConnect,
    required this.onBreathing,
  });

  final int heartRate;
  final int stress;
  final int oxygen;
  final double temperature;
  final double movement;
  final double sleepHours;
  final List<AlertEvent> events;
  final WatchConnectionState watchState;
  final String watchMessage;
  final VoidCallback onWatchConnect;
  final VoidCallback onBreathing;

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 16, 20, 24),
      children: [
        const AppHeader(
          title: 'Salud',
          subtitle: 'Tendencias y acciones para anticiparte.',
        ),
        const SizedBox(height: 18),
        WatchStatusCard(
          state: watchState,
          message: watchMessage,
          onConnect: onWatchConnect,
        ),
        const SizedBox(height: 18),
        Card(
          child: Padding(
            padding: const EdgeInsets.all(20),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    const Expanded(
                      child: Text(
                        'Balance del dia',
                        style: TextStyle(
                          fontSize: 20,
                          fontWeight: FontWeight.w800,
                          color: AppColors.text,
                        ),
                      ),
                    ),
                    StatusChip(
                      text: stress < 35 ? 'Estable' : 'Revisar',
                      color: stress < 35 ? AppColors.success : AppColors.danger,
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                HealthProgress(
                  label: 'Pulso estable',
                  value: heartRate <= 85 ? 0.88 : 0.62,
                  color: const Color(0xFFEF4444),
                ),
                HealthProgress(
                  label: 'Estres bajo',
                  value: 1 - stress / 100,
                  color: const Color(0xFFF59E0B),
                ),
                HealthProgress(
                  label: 'Oxigeno',
                  value: oxygen / 100,
                  color: AppColors.success,
                ),
                HealthProgress(
                  label: 'Sueno',
                  value: min(sleepHours / 8, 1),
                  color: const Color(0xFF6366F1),
                ),
                HealthProgress(
                  label: 'Temperatura estable',
                  value: (1 - ((temperature - 36.7).abs() / 2)).clamp(0.0, 1.0),
                  color: const Color(0xFFEC4899),
                ),
                HealthProgress(
                  label: 'Movimiento controlado',
                  value: (1 - ((movement - 1).abs() / 2)).clamp(0.0, 1.0),
                  color: const Color(0xFF8B5CF6),
                ),
                const SizedBox(height: 12),
                SizedBox(
                  width: double.infinity,
                  child: FilledButton.icon(
                    onPressed: onBreathing,
                    icon: const Icon(Icons.self_improvement),
                    label: const Text('Iniciar respiracion guiada'),
                  ),
                ),
              ],
            ),
          ),
        ),
        const SizedBox(height: 22),
        const SectionTitle(title: 'Actividad reciente'),
        const SizedBox(height: 12),
        for (final event in events) TimelineTile(event: event),
      ],
    );
  }
}

class SupportScreen extends StatelessWidget {
  const SupportScreen({
    super.key,
    required this.contacts,
    required this.onAddContact,
    required this.onUpdateContact,
    required this.onRemoveContact,
    required this.onSendAlert,
  });

  final List<EmergencyContact> contacts;
  final ValueChanged<EmergencyContact> onAddContact;
  final void Function(int index, EmergencyContact contact) onUpdateContact;
  final ValueChanged<int> onRemoveContact;
  final VoidCallback onSendAlert;

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 16, 20, 24),
      children: [
        AppHeader(
          title: 'Red de apoyo',
          subtitle: 'Contactos preparados para ayudarte rapido.',
          action: IconButton.filledTonal(
            onPressed: () =>
                showContactDialog(context: context, onSaved: onAddContact),
            tooltip: 'Agregar contacto',
            icon: const Icon(Icons.person_add),
          ),
        ),
        const SizedBox(height: 18),
        AlertCard(onSendAlert: onSendAlert),
        const SizedBox(height: 22),
        SectionTitle(
          title: 'Contactos activos',
          actionText: 'Agregar',
          onAction: () =>
              showContactDialog(context: context, onSaved: onAddContact),
        ),
        const SizedBox(height: 12),
        for (var i = 0; i < contacts.length; i++)
          Padding(
            padding: const EdgeInsets.only(bottom: 12),
            child: ContactCard(
              contact: contacts[i],
              onToggle: (value) =>
                  onUpdateContact(i, contacts[i].copyWith(active: value)),
              onEdit: () => showContactDialog(
                context: context,
                contact: contacts[i],
                onSaved: (contact) => onUpdateContact(i, contact),
              ),
              onDelete: () => onRemoveContact(i),
            ),
          ),
      ],
    );
  }
}

class ProfileScreen extends StatelessWidget {
  const ProfileScreen({
    super.key,
    required this.name,
    required this.email,
    required this.automaticAlerts,
    required this.shareLocation,
    required this.healthNotifications,
    required this.calmMode,
    required this.onProfileChanged,
    required this.onAutomaticAlertsChanged,
    required this.onShareLocationChanged,
    required this.onHealthNotificationsChanged,
    required this.onCalmModeChanged,
  });

  final String name;
  final String email;
  final bool automaticAlerts;
  final bool shareLocation;
  final bool healthNotifications;
  final bool calmMode;
  final void Function(String name, String email) onProfileChanged;
  final ValueChanged<bool> onAutomaticAlertsChanged;
  final ValueChanged<bool> onShareLocationChanged;
  final ValueChanged<bool> onHealthNotificationsChanged;
  final ValueChanged<bool> onCalmModeChanged;

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 16, 20, 24),
      children: [
        const AppHeader(
          title: 'Perfil',
          subtitle: 'Datos personales, privacidad y alertas.',
        ),
        const SizedBox(height: 18),
        ProfileCard(
          name: name,
          email: email,
          onEdit: () => showProfileDialog(
            context: context,
            name: name,
            email: email,
            onSaved: onProfileChanged,
          ),
        ),
        const SizedBox(height: 22),
        const SectionTitle(title: 'Ajustes de seguridad'),
        const SizedBox(height: 12),
        SettingsTile(
          icon: Icons.auto_awesome,
          title: 'Alertas automaticas',
          subtitle: 'Activar ayuda cuando NERA detecte senales criticas.',
          value: automaticAlerts,
          onChanged: onAutomaticAlertsChanged,
        ),
        SettingsTile(
          icon: Icons.location_on_outlined,
          title: 'Compartir ubicacion',
          subtitle: 'Incluir ubicacion aproximada en emergencias.',
          value: shareLocation,
          onChanged: onShareLocationChanged,
        ),
        SettingsTile(
          icon: Icons.notifications_active_outlined,
          title: 'Notificaciones',
          subtitle: 'Recordatorios de medicion, descanso y calma.',
          value: healthNotifications,
          onChanged: onHealthNotificationsChanged,
        ),
        SettingsTile(
          icon: Icons.do_not_disturb_on_outlined,
          title: 'Modo calma',
          subtitle: 'Reducir estimulos visuales durante una crisis.',
          value: calmMode,
          onChanged: onCalmModeChanged,
        ),
        const SizedBox(height: 12),
        Card(
          child: ListTile(
            contentPadding: const EdgeInsets.symmetric(
              horizontal: 18,
              vertical: 10,
            ),
            leading: const Icon(Icons.verified_user_outlined),
            title: const Text(
              'Privacidad y datos',
              style: TextStyle(fontWeight: FontWeight.w800),
            ),
            subtitle: const Text(
              'Permisos, consentimiento y datos compartidos.',
            ),
            trailing: const Icon(Icons.chevron_right),
            onTap: () => showInfoDialog(
              context,
              'Privacidad y datos',
              'En una version final, esta seccion administraria permisos de ubicacion, notificaciones y almacenamiento seguro.',
            ),
          ),
        ),
      ],
    );
  }
}

class AppHeader extends StatelessWidget {
  const AppHeader({
    super.key,
    required this.title,
    required this.subtitle,
    this.action,
  });

  final String title;
  final String subtitle;
  final Widget? action;

  @override
  Widget build(BuildContext context) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(title, style: Theme.of(context).textTheme.headlineMedium),
              const SizedBox(height: 6),
              Text(subtitle, style: Theme.of(context).textTheme.bodyMedium),
            ],
          ),
        ),
        ?action,
      ],
    );
  }
}

class WellnessCard extends StatelessWidget {
  const WellnessCard({
    super.key,
    required this.score,
    required this.stress,
    required this.contactsCount,
    required this.onMeasure,
    required this.onBreathing,
  });

  final int score;
  final int stress;
  final int contactsCount;
  final VoidCallback onMeasure;
  final VoidCallback onBreathing;

  @override
  Widget build(BuildContext context) {
    final status = stress < 35 ? 'Buen estado' : 'Necesita atencion';
    final statusColor = stress < 35 ? AppColors.success : AppColors.danger;

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(22),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      StatusChip(text: status, color: statusColor),
                      const SizedBox(height: 14),
                      const Text(
                        'Indice de bienestar',
                        style: TextStyle(
                          color: AppColors.mutedText,
                          fontWeight: FontWeight.w700,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        '$score',
                        style: const TextStyle(
                          fontSize: 58,
                          height: 0.95,
                          color: AppColors.text,
                          fontWeight: FontWeight.w900,
                        ),
                      ),
                    ],
                  ),
                ),
                SizedBox(
                  height: 112,
                  width: 112,
                  child: Stack(
                    fit: StackFit.expand,
                    children: [
                      CircularProgressIndicator(
                        value: score / 100,
                        strokeWidth: 12,
                        strokeCap: StrokeCap.round,
                        backgroundColor: const Color(0xFFE4E7EC),
                        color: AppColors.primary,
                      ),
                      Center(
                        child: Text(
                          '$contactsCount\napoyo',
                          textAlign: TextAlign.center,
                          style: const TextStyle(
                            color: AppColors.mutedText,
                            fontSize: 12,
                            fontWeight: FontWeight.w800,
                          ),
                        ),
                      ),
                    ],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 22),
            Row(
              children: [
                Expanded(
                  child: FilledButton.icon(
                    onPressed: onMeasure,
                    icon: const Icon(Icons.sensors),
                    label: const Text('Medir'),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: OutlinedButton.icon(
                    onPressed: onBreathing,
                    icon: const Icon(Icons.self_improvement),
                    label: const Text('Calmar'),
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class QuickActionCard extends StatelessWidget {
  const QuickActionCard({
    super.key,
    required this.icon,
    required this.title,
    required this.subtitle,
    required this.color,
    required this.onTap,
  });

  final IconData icon;
  final String title;
  final String subtitle;
  final Color color;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: InkWell(
        borderRadius: BorderRadius.circular(24),
        onTap: onTap,
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              CircleAvatar(
                backgroundColor: color.withValues(alpha: 0.12),
                foregroundColor: color,
                child: Icon(icon),
              ),
              const SizedBox(height: 14),
              Text(
                title,
                style: const TextStyle(
                  color: AppColors.text,
                  fontWeight: FontWeight.w900,
                ),
              ),
              const SizedBox(height: 4),
              Text(subtitle, style: Theme.of(context).textTheme.bodyMedium),
            ],
          ),
        ),
      ),
    );
  }
}

class WatchStatusCard extends StatelessWidget {
  const WatchStatusCard({
    super.key,
    required this.state,
    required this.message,
    required this.onConnect,
  });

  final WatchConnectionState state;
  final String message;
  final VoidCallback onConnect;

  @override
  Widget build(BuildContext context) {
    final connected = state == WatchConnectionState.connected;
    final loading =
        state == WatchConnectionState.scanning ||
        state == WatchConnectionState.connecting;
    final color = connected
        ? AppColors.success
        : state == WatchConnectionState.failed
        ? AppColors.danger
        : AppColors.secondary;
    final label = connected
        ? 'Desconectar'
        : loading
        ? 'Buscando...'
        : 'Conectar';

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(18),
        child: Row(
          children: [
            CircleAvatar(
              backgroundColor: color.withValues(alpha: 0.12),
              foregroundColor: color,
              child: Icon(connected ? Icons.watch : Icons.bluetooth_searching),
            ),
            const SizedBox(width: 14),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text(
                    'Reloj NERA',
                    style: TextStyle(
                      color: AppColors.text,
                      fontWeight: FontWeight.w900,
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    message,
                    maxLines: 2,
                    overflow: TextOverflow.ellipsis,
                    style: const TextStyle(color: AppColors.mutedText),
                  ),
                ],
              ),
            ),
            const SizedBox(width: 10),
            FilledButton(
              onPressed: loading ? null : onConnect,
              style: FilledButton.styleFrom(
                backgroundColor: color,
                minimumSize: const Size(104, 44),
              ),
              child: Text(label),
            ),
          ],
        ),
      ),
    );
  }
}

class MetricGrid extends StatelessWidget {
  const MetricGrid({super.key, required this.metrics});

  final List<MetricData> metrics;

  @override
  Widget build(BuildContext context) {
    return GridView.builder(
      shrinkWrap: true,
      physics: const NeverScrollableScrollPhysics(),
      itemCount: metrics.length,
      gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
        crossAxisCount: 2,
        crossAxisSpacing: 12,
        mainAxisSpacing: 12,
        childAspectRatio: 1.02,
      ),
      itemBuilder: (context, index) => MetricCard(data: metrics[index]),
    );
  }
}

class MetricData {
  const MetricData({
    required this.title,
    required this.value,
    required this.unit,
    required this.icon,
    required this.color,
    required this.detail,
  });

  final String title;
  final String value;
  final String unit;
  final IconData icon;
  final Color color;
  final String detail;
}

class MetricCard extends StatelessWidget {
  const MetricCard({super.key, required this.data});

  final MetricData data;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: InkWell(
        borderRadius: BorderRadius.circular(24),
        onTap: () => showInfoDialog(context, data.title, data.detail),
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Row(
                children: [
                  CircleAvatar(
                    backgroundColor: data.color.withValues(alpha: 0.12),
                    foregroundColor: data.color,
                    child: Icon(data.icon, size: 20),
                  ),
                  const Spacer(),
                  const Icon(Icons.chevron_right, color: AppColors.mutedText),
                ],
              ),
              Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    data.title,
                    style: Theme.of(context).textTheme.bodyMedium,
                  ),
                  const SizedBox(height: 6),
                  FittedBox(
                    fit: BoxFit.scaleDown,
                    alignment: Alignment.centerLeft,
                    child: Text.rich(
                      TextSpan(
                        text: data.value,
                        style: const TextStyle(
                          color: AppColors.text,
                          fontSize: 32,
                          fontWeight: FontWeight.w900,
                        ),
                        children: [
                          TextSpan(
                            text: ' ${data.unit}',
                            style: const TextStyle(
                              color: AppColors.mutedText,
                              fontSize: 14,
                              fontWeight: FontWeight.w700,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class EmergencyBanner extends StatelessWidget {
  const EmergencyBanner({super.key, required this.onEmergency});

  final VoidCallback onEmergency;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(18),
      decoration: BoxDecoration(
        color: const Color(0xFFFFFBFA),
        borderRadius: BorderRadius.circular(24),
        border: Border.all(color: const Color(0xFFFECACA)),
      ),
      child: Row(
        children: [
          const CircleAvatar(
            backgroundColor: Color(0xFFFEE2E2),
            foregroundColor: AppColors.danger,
            child: Icon(Icons.sos),
          ),
          const SizedBox(width: 14),
          const Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  'Ayuda inmediata',
                  style: TextStyle(
                    color: AppColors.text,
                    fontWeight: FontWeight.w900,
                  ),
                ),
                SizedBox(height: 4),
                Text(
                  'Avisa a tu red de apoyo en una emergencia.',
                  style: TextStyle(color: AppColors.mutedText),
                ),
              ],
            ),
          ),
          IconButton.filled(
            style: IconButton.styleFrom(backgroundColor: AppColors.danger),
            onPressed: onEmergency,
            icon: const Icon(Icons.arrow_forward),
          ),
        ],
      ),
    );
  }
}

class AlertCard extends StatelessWidget {
  const AlertCard({super.key, required this.onSendAlert});

  final VoidCallback onSendAlert;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(22),
      decoration: BoxDecoration(
        color: AppColors.danger,
        borderRadius: BorderRadius.circular(28),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Icon(Icons.health_and_safety, color: Colors.white, size: 34),
          const SizedBox(height: 18),
          const Text(
            'Protocolo de emergencia',
            style: TextStyle(
              color: Colors.white,
              fontSize: 22,
              fontWeight: FontWeight.w900,
            ),
          ),
          const SizedBox(height: 8),
          Text(
            'Envia una alerta con tu estado actual a tus contactos activos.',
            style: TextStyle(color: Colors.white.withValues(alpha: 0.86)),
          ),
          const SizedBox(height: 18),
          FilledButton.icon(
            style: FilledButton.styleFrom(
              backgroundColor: Colors.white,
              foregroundColor: AppColors.danger,
            ),
            onPressed: onSendAlert,
            icon: const Icon(Icons.notification_important),
            label: const Text('Enviar alerta ahora'),
          ),
        ],
      ),
    );
  }
}

class HealthProgress extends StatelessWidget {
  const HealthProgress({
    super.key,
    required this.label,
    required this.value,
    required this.color,
  });

  final String label;
  final double value;
  final Color color;

  @override
  Widget build(BuildContext context) {
    final normalized = value.clamp(0.0, 1.0);

    return Padding(
      padding: const EdgeInsets.only(bottom: 16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              Expanded(
                child: Text(
                  label,
                  style: const TextStyle(
                    color: AppColors.text,
                    fontWeight: FontWeight.w800,
                  ),
                ),
              ),
              Text(
                '${(normalized * 100).round()}%',
                style: const TextStyle(
                  color: AppColors.mutedText,
                  fontWeight: FontWeight.w800,
                ),
              ),
            ],
          ),
          const SizedBox(height: 9),
          ClipRRect(
            borderRadius: BorderRadius.circular(999),
            child: LinearProgressIndicator(
              minHeight: 10,
              value: normalized,
              color: color,
              backgroundColor: const Color(0xFFE4E7EC),
            ),
          ),
        ],
      ),
    );
  }
}

class TimelineTile extends StatelessWidget {
  const TimelineTile({super.key, required this.event});

  final AlertEvent event;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 12),
      child: Card(
        child: ListTile(
          contentPadding: const EdgeInsets.symmetric(
            horizontal: 16,
            vertical: 8,
          ),
          leading: CircleAvatar(
            backgroundColor: event.color.withValues(alpha: 0.12),
            foregroundColor: event.color,
            child: Icon(event.icon, size: 20),
          ),
          title: Text(
            event.title,
            style: const TextStyle(fontWeight: FontWeight.w900),
          ),
          subtitle: Text(event.description),
          trailing: Text(
            formatTime(event.time),
            style: const TextStyle(
              color: AppColors.mutedText,
              fontWeight: FontWeight.w800,
            ),
          ),
        ),
      ),
    );
  }
}

class ContactCard extends StatelessWidget {
  const ContactCard({
    super.key,
    required this.contact,
    required this.onToggle,
    required this.onEdit,
    required this.onDelete,
  });

  final EmergencyContact contact;
  final ValueChanged<bool> onToggle;
  final VoidCallback onEdit;
  final VoidCallback onDelete;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.fromLTRB(16, 12, 8, 12),
        child: Row(
          children: [
            CircleAvatar(
              radius: 24,
              backgroundColor: contact.active
                  ? AppColors.primary.withValues(alpha: 0.12)
                  : const Color(0xFFF2F4F7),
              foregroundColor: contact.active
                  ? AppColors.primary
                  : AppColors.mutedText,
              child: Text(
                initialsFor(contact.name),
                style: const TextStyle(fontWeight: FontWeight.w900),
              ),
            ),
            const SizedBox(width: 12),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    contact.name,
                    style: const TextStyle(
                      color: AppColors.text,
                      fontSize: 16,
                      fontWeight: FontWeight.w900,
                    ),
                  ),
                  const SizedBox(height: 3),
                  Text(
                    '${contact.relation} - ${contact.phone}',
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                  ),
                ],
              ),
            ),
            Switch(value: contact.active, onChanged: onToggle),
            PopupMenuButton<String>(
              onSelected: (value) {
                if (value == 'edit') onEdit();
                if (value == 'delete') onDelete();
              },
              itemBuilder: (context) => const [
                PopupMenuItem(value: 'edit', child: Text('Editar')),
                PopupMenuItem(value: 'delete', child: Text('Eliminar')),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class ProfileCard extends StatelessWidget {
  const ProfileCard({
    super.key,
    required this.name,
    required this.email,
    required this.onEdit,
  });

  final String name;
  final String email;
  final VoidCallback onEdit;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(20),
        child: Row(
          children: [
            CircleAvatar(
              radius: 34,
              backgroundColor: AppColors.primary.withValues(alpha: 0.12),
              foregroundColor: AppColors.primary,
              child: Text(
                initialsFor(name),
                style: const TextStyle(
                  fontSize: 24,
                  fontWeight: FontWeight.w900,
                ),
              ),
            ),
            const SizedBox(width: 16),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    name,
                    style: const TextStyle(
                      color: AppColors.text,
                      fontSize: 20,
                      fontWeight: FontWeight.w900,
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    email,
                    overflow: TextOverflow.ellipsis,
                    style: const TextStyle(color: AppColors.mutedText),
                  ),
                ],
              ),
            ),
            IconButton.filledTonal(
              onPressed: onEdit,
              tooltip: 'Editar perfil',
              icon: const Icon(Icons.edit),
            ),
          ],
        ),
      ),
    );
  }
}

class SettingsTile extends StatelessWidget {
  const SettingsTile({
    super.key,
    required this.icon,
    required this.title,
    required this.subtitle,
    required this.value,
    required this.onChanged,
  });

  final IconData icon;
  final String title;
  final String subtitle;
  final bool value;
  final ValueChanged<bool> onChanged;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 12),
      child: Card(
        child: SwitchListTile(
          contentPadding: const EdgeInsets.symmetric(
            horizontal: 18,
            vertical: 8,
          ),
          secondary: CircleAvatar(
            backgroundColor: AppColors.primary.withValues(alpha: 0.10),
            foregroundColor: AppColors.primary,
            child: Icon(icon, size: 20),
          ),
          title: Text(
            title,
            style: const TextStyle(
              color: AppColors.text,
              fontWeight: FontWeight.w900,
            ),
          ),
          subtitle: Text(subtitle),
          value: value,
          onChanged: onChanged,
        ),
      ),
    );
  }
}

class SectionTitle extends StatelessWidget {
  const SectionTitle({
    super.key,
    required this.title,
    this.actionText,
    this.onAction,
  });

  final String title;
  final String? actionText;
  final VoidCallback? onAction;

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Expanded(
          child: Text(title, style: Theme.of(context).textTheme.titleLarge),
        ),
        if (actionText != null && onAction != null)
          TextButton(onPressed: onAction, child: Text(actionText!)),
      ],
    );
  }
}

class StatusChip extends StatelessWidget {
  const StatusChip({super.key, required this.text, required this.color});

  final String text;
  final Color color;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
      decoration: BoxDecoration(
        color: color.withValues(alpha: 0.10),
        borderRadius: BorderRadius.circular(999),
      ),
      child: Text(
        text,
        style: TextStyle(
          color: color,
          fontSize: 12,
          fontWeight: FontWeight.w900,
        ),
      ),
    );
  }
}

class BreathingSheet extends StatelessWidget {
  const BreathingSheet({super.key});

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(24, 8, 24, 28),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            height: 118,
            width: 118,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: AppColors.primary.withValues(alpha: 0.12),
              border: Border.all(
                color: AppColors.primary.withValues(alpha: 0.24),
                width: 2,
              ),
            ),
            child: const Icon(
              Icons.self_improvement,
              size: 48,
              color: AppColors.primary,
            ),
          ),
          const SizedBox(height: 18),
          const Text(
            'Respiracion 4-4-6',
            style: TextStyle(
              color: AppColors.text,
              fontSize: 24,
              fontWeight: FontWeight.w900,
            ),
          ),
          const SizedBox(height: 10),
          const Text(
            'Inhala 4 segundos, sostene 4 y exhala 6. Repeti el ciclo hasta sentir mas calma.',
            textAlign: TextAlign.center,
            style: TextStyle(color: AppColors.mutedText),
          ),
          const SizedBox(height: 20),
          const Row(
            children: [
              Expanded(child: StepPill(text: 'Inhalar 4s')),
              SizedBox(width: 8),
              Expanded(child: StepPill(text: 'Sostener 4s')),
              SizedBox(width: 8),
              Expanded(child: StepPill(text: 'Exhalar 6s')),
            ],
          ),
          const SizedBox(height: 22),
          SizedBox(
            width: double.infinity,
            child: FilledButton(
              onPressed: () => Navigator.pop(context),
              child: const Text('Finalizar'),
            ),
          ),
        ],
      ),
    );
  }
}

class StepPill extends StatelessWidget {
  const StepPill({super.key, required this.text});

  final String text;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(vertical: 11),
      decoration: BoxDecoration(
        color: const Color(0xFFF2F4F7),
        borderRadius: BorderRadius.circular(999),
      ),
      child: Text(
        text,
        textAlign: TextAlign.center,
        style: const TextStyle(
          color: AppColors.text,
          fontSize: 12,
          fontWeight: FontWeight.w900,
        ),
      ),
    );
  }
}

Future<void> showContactDialog({
  required BuildContext context,
  required ValueChanged<EmergencyContact> onSaved,
  EmergencyContact? contact,
}) async {
  final nameController = TextEditingController(text: contact?.name ?? '');
  final relationController = TextEditingController(
    text: contact?.relation ?? '',
  );
  final phoneController = TextEditingController(text: contact?.phone ?? '');
  var active = contact?.active ?? true;

  await showDialog<void>(
    context: context,
    builder: (context) {
      return StatefulBuilder(
        builder: (context, setDialogState) {
          return AlertDialog(
            title: Text(
              contact == null ? 'Agregar contacto' : 'Editar contacto',
            ),
            content: SingleChildScrollView(
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  TextField(
                    controller: nameController,
                    decoration: const InputDecoration(labelText: 'Nombre'),
                  ),
                  const SizedBox(height: 12),
                  TextField(
                    controller: relationController,
                    decoration: const InputDecoration(labelText: 'Relacion'),
                  ),
                  const SizedBox(height: 12),
                  TextField(
                    controller: phoneController,
                    decoration: const InputDecoration(labelText: 'Telefono'),
                    keyboardType: TextInputType.phone,
                  ),
                  SwitchListTile(
                    contentPadding: EdgeInsets.zero,
                    title: const Text('Activo para alertas'),
                    value: active,
                    onChanged: (value) => setDialogState(() => active = value),
                  ),
                ],
              ),
            ),
            actions: [
              TextButton(
                onPressed: () => Navigator.pop(context),
                child: const Text('Cancelar'),
              ),
              FilledButton(
                onPressed: () {
                  final name = nameController.text.trim();
                  final relation = relationController.text.trim();
                  final phone = phoneController.text.trim();
                  if (name.isEmpty || phone.isEmpty) return;
                  onSaved(
                    EmergencyContact(
                      name: name,
                      relation: relation.isEmpty ? 'Contacto' : relation,
                      phone: phone,
                      active: active,
                    ),
                  );
                  Navigator.pop(context);
                },
                child: const Text('Guardar'),
              ),
            ],
          );
        },
      );
    },
  );
}

Future<void> showProfileDialog({
  required BuildContext context,
  required String name,
  required String email,
  required void Function(String name, String email) onSaved,
}) async {
  final nameController = TextEditingController(text: name);
  final emailController = TextEditingController(text: email);

  await showDialog<void>(
    context: context,
    builder: (context) {
      return AlertDialog(
        title: const Text('Editar perfil'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(
              controller: nameController,
              decoration: const InputDecoration(labelText: 'Nombre'),
            ),
            const SizedBox(height: 12),
            TextField(
              controller: emailController,
              decoration: const InputDecoration(labelText: 'Email'),
              keyboardType: TextInputType.emailAddress,
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Cancelar'),
          ),
          FilledButton(
            onPressed: () {
              final newName = nameController.text.trim();
              final newEmail = emailController.text.trim();
              if (newName.isEmpty || newEmail.isEmpty) return;
              onSaved(newName, newEmail);
              Navigator.pop(context);
            },
            child: const Text('Guardar'),
          ),
        ],
      );
    },
  );
}

void showInfoDialog(BuildContext context, String title, String description) {
  showDialog<void>(
    context: context,
    builder: (context) {
      return AlertDialog(
        title: Text(title),
        content: Text(description),
        actions: [
          FilledButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Cerrar'),
          ),
        ],
      );
    },
  );
}

String initialsFor(String text) {
  final parts = text
      .trim()
      .split(RegExp(r'\s+'))
      .where((part) => part.isNotEmpty)
      .toList();
  if (parts.isEmpty) return 'N';
  if (parts.length == 1) return parts.first.substring(0, 1).toUpperCase();
  return '${parts.first.substring(0, 1)}${parts.last.substring(0, 1)}'
      .toUpperCase();
}

String firstName(String text) {
  final parts = text.trim().split(RegExp(r'\s+'));
  if (parts.isEmpty || parts.first.isEmpty) return 'Usuario';
  return parts.first;
}

String formatTime(DateTime time) {
  final hour = time.hour.toString().padLeft(2, '0');
  final minute = time.minute.toString().padLeft(2, '0');
  return '$hour:$minute';
}
