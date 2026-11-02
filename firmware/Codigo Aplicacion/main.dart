import 'dart:math';
import 'package:flutter/material.dart';
import 'app_theme.dart';
import 'models.dart';
import 'nera_controller.dart';
import 'watch_ble_service.dart';

void main() {
  runApp(const NeraApp());
}

class NeraApp extends StatelessWidget {
  const NeraApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'NERA',
      debugShowCheckedModeBanner: false,
      theme: AppTheme.lightTheme,
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
  late final NeraController _controller;

  @override
  void initState() {
    super.initState();
    _controller = NeraController();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  void _showSnack(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        behavior: SnackBarBehavior.floating,
        backgroundColor: AppTheme.textDark,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(14)),
      ),
    );
  }

  void _sendEmergencyAlert() {
    final activeContacts = _controller.contacts.where((c) => c.active).toList();
    final message = 'Alerta enviada a ${activeContacts.length} contacto${activeContacts.length == 1 ? '' : 's'}.';

    _controller.events.insert(
      0,
      AlertEvent(
        title: 'Alerta de emergencia',
        description: activeContacts.isEmpty ? 'No había contactos activos.' : message,
        time: DateTime.now(),
        icon: Icons.notification_important,
        color: AppTheme.danger,
      ),
    );

    showDialog<void>(
      context: context,
      builder: (context) => AlertDialog(
        icon: const Icon(Icons.notification_important, color: AppTheme.danger, size: 34),
        title: const Text('Protocolo de ayuda activado'),
        content: Text(
          activeContacts.isEmpty
              ? 'Agregá al menos un contacto activo para poder enviar alertas.'
              : '$message\n\nSe compartió tu estado actual y ubicación aproximada.',
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
    _controller.startBreathingSession();
    showModalBottomSheet<void>(
      context: context,
      showDragHandle: true,
      backgroundColor: Colors.white,
      builder: (context) => const BreathingSheet(),
    );
  }

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: _controller,
      builder: (context, _) {
        final pages = [
          HomeScreen(
            controller: _controller,
            onEmergency: _sendEmergencyAlert,
            onBreathing: _startBreathingSession,
          ),
          HealthScreen(
            controller: _controller,
            onBreathing: _startBreathingSession,
          ),
          SupportScreen(
            controller: _controller,
            onSendAlert: _sendEmergencyAlert,
            onShowSnack: _showSnack,
          ),
          ProfileScreen(
            controller: _controller,
            onShowSnack: _showSnack,
          ),
        ];

        return Scaffold(
          body: SafeArea(child: pages[_selectedIndex]),
          bottomNavigationBar: Container(
            decoration: BoxDecoration(
              boxShadow: [
                BoxShadow(
                  color: AppTheme.textDark.withOpacity(0.04),
                  blurRadius: 16,
                  offset: const Offset(0, -4),
                )
              ],
            ),
            child: NavigationBar(
              selectedIndex: _selectedIndex,
              onDestinationSelected: (index) => setState(() => _selectedIndex = index),
              backgroundColor: Colors.white,
              elevation: 0,
              height: 72,
              indicatorColor: AppTheme.primary.withOpacity(0.08),
              destinations: const [
                NavigationDestination(
                  icon: Icon(Icons.dashboard_outlined),
                  selectedIcon: Icon(Icons.dashboard, color: AppTheme.primary),
                  label: 'Inicio',
                ),
                NavigationDestination(
                  icon: Icon(Icons.monitor_heart_outlined),
                  selectedIcon: Icon(Icons.monitor_heart, color: AppTheme.primary),
                  label: 'Salud',
                ),
                NavigationDestination(
                  icon: Icon(Icons.groups_outlined),
                  selectedIcon: Icon(Icons.groups, color: AppTheme.primary),
                  label: 'Apoyo',
                ),
                NavigationDestination(
                  icon: Icon(Icons.person_outline),
                  selectedIcon: Icon(Icons.person, color: AppTheme.primary),
                  label: 'Perfil',
                ),
              ],
            ),
          ),
        );
      },
    );
  }
}

class HomeScreen extends StatelessWidget {
  const HomeScreen({
    super.key,
    required this.controller,
    required this.onEmergency,
    required this.onBreathing,
  });

  final NeraController controller;
  final VoidCallback onEmergency;
  final VoidCallback onBreathing;

  @override
  Widget build(BuildContext context) {
    final score = ((100 - controller.stress) * 0.42 + controller.oxygen * 0.36 + controller.sleepHours * 9 * 0.22)
        .round()
        .clamp(0, 100);

    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 16, 20, 24),
      children: [
        AppHeader(
          title: 'Hola, ${firstName(controller.name)}',
          subtitle: 'Resumen claro de tu bienestar de hoy.',
          action: IconButton.filled(
            style: IconButton.styleFrom(backgroundColor: AppTheme.danger),
            onPressed: onEmergency,
            icon: const Icon(Icons.sos),
          ),
        ),
        const SizedBox(height: 20),
        WellnessCard(
          score: score,
          stress: controller.stress,
          contactsCount: controller.contacts.where((c) => c.active).length,
          onMeasure: controller.runMeasurement,
          onBreathing: onBreathing,
        ),
        const SizedBox(height: 16),
        WatchStatusCard(
          state: controller.watchState,
          message: controller.watchMessage,
          onConnect: controller.connectWatch,
        ),
        const SizedBox(height: 16),
        const SectionTitle(title: 'Señales principales'),
        const SizedBox(height: 12),
        MetricGrid(
          metrics: [
            MetricData(
              title: 'Pulso',
              value: '${controller.heartRate}',
              unit: 'bpm',
              icon: Icons.favorite,
              color: AppTheme.danger,
              detail: 'Frecuencia cardiaca estimada en la última medición.',
            ),
            MetricData(
              title: 'Oxígeno',
              value: '${controller.oxygen}',
              unit: '%',
              icon: Icons.air,
              color: AppTheme.success,
              detail: 'Saturación estimada registrada por el sensor del reloj.',
            ),
            MetricData(
              title: 'Estrés',
              value: '${controller.stress}',
              unit: '%',
              icon: Icons.psychology,
              color: const Color(0xFFF59E0B),
              detail: 'Nivel aproximado de tensión según tus variables biométricas.',
            ),
            MetricData(
              title: 'Temp.',
              value: controller.temperature.toStringAsFixed(1),
              unit: '°C',
              icon: Icons.thermostat,
              color: const Color(0xFFEC4899),
              detail: 'Temperatura corporal recibida desde el sensor MLX90614.',
            ),
          ],
        ),
      ],
    );
  }
}

class HealthScreen extends StatelessWidget {
  const HealthScreen({
    super.key,
    required this.controller,
    required this.onBreathing,
  });

  final NeraController controller;
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
        const SizedBox(height: 20),
        Container(
          decoration: BoxDecoration(
            color: Colors.white,
            borderRadius: BorderRadius.circular(24),
            boxShadow: AppTheme.softShadow,
          ),
          child: Padding(
            padding: const EdgeInsets.all(22),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    const Expanded(
                      child: Text(
                        'Balance del día',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.w800,
                          color: AppTheme.textDark,
                        ),
                      ),
                    ),
                    StatusChip(
                      text: controller.stress < 35 ? 'Estable' : 'Revisar',
                      color: controller.stress < 35 ? AppTheme.success : AppTheme.danger,
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                HealthProgress(
                  label: 'Pulso estable',
                  value: controller.heartRate <= 85 ? 0.90 : 0.60,
                  color: AppTheme.danger,
                ),
                HealthProgress(
                  label: 'Estrés bajo',
                  value: 1 - controller.stress / 100,
                  color: const Color(0xFFF59E0B),
                ),
                HealthProgress(
                  label: 'Oxígeno',
                  value: controller.oxygen / 100,
                  color: AppTheme.success,
                ),
                HealthProgress(
                  label: 'Temperatura estable',
                  value: (1 - ((controller.temperature - 36.7).abs() / 2)).clamp(0.0, 1.0),
                  color: const Color(0xFFEC4899),
                ),
                const SizedBox(height: 16),
                SizedBox(
                  width: double.infinity,
                  child: FilledButton.icon(
                    onPressed: onBreathing,
                    icon: const Icon(Icons.self_improvement),
                    label: const Text('Iniciar respiración guiada'),
                  ),
                ),
              ],
            ),
          ),
        ),
        const SizedBox(height: 24),
        const SectionTitle(title: 'Actividad reciente'),
        const SizedBox(height: 12),
        for (final event in controller.events) TimelineTile(event: event),
      ],
    );
  }
}

class SupportScreen extends StatelessWidget {
  const SupportScreen({
    super.key,
    required this.controller,
    required this.onSendAlert,
    required this.onShowSnack,
  });

  final NeraController controller;
  final VoidCallback onSendAlert;
  final ValueChanged<String> onShowSnack;

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 16, 20, 24),
      children: [
        AppHeader(
          title: 'Red de apoyo',
          subtitle: 'Contactos preparados para ayudarte rápido.',
          action: IconButton.filledTonal(
            onPressed: () => showContactDialog(
              context: context,
              onSaved: (contact) {
                controller.addContact(contact);
                onShowSnack('Contacto agregado.');
              },
            ),
            icon: const Icon(Icons.person_add),
          ),
        ),
        const SizedBox(height: 20),
        AlertCard(onSendAlert: onSendAlert),
        const SizedBox(height: 24),
        SectionTitle(
          title: 'Contactos activos',
          actionText: 'Agregar',
          onAction: () => showContactDialog(
            context: context,
            onSaved: (contact) {
              controller.addContact(contact);
              onShowSnack('Contacto agregado.');
            },
          ),
        ),
        const SizedBox(height: 12),
        for (var i = 0; i < controller.contacts.length; i++)
          Padding(
            padding: const EdgeInsets.only(bottom: 12),
            child: ContactCard(
              contact: controller.contacts[i],
              onToggle: (value) => controller.updateContact(i, controller.contacts[i].copyWith(active: value)),
              onEdit: () => showContactDialog(
                context: context,
                contact: controller.contacts[i],
                onSaved: (contact) {
                  controller.updateContact(i, contact);
                  onShowSnack('Contacto actualizado.');
                },
              ),
              onDelete: () {
                controller.removeContact(i);
                onShowSnack('Contacto eliminado.');
              },
            ),
          ),
      ],
    );
  }
}

class ProfileScreen extends StatelessWidget {
  const ProfileScreen({
    super.key,
    required this.controller,
    required this.onShowSnack,
  });

  final NeraController controller;
  final ValueChanged<String> onShowSnack;

  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 16, 20, 24),
      children: [
        const AppHeader(
          title: 'Perfil',
          subtitle: 'Datos personales, privacidad y alertas.',
        ),
        const SizedBox(height: 20),
        ProfileCard(
          name: controller.name,
          email: controller.email,
          onEdit: () => showProfileDialog(
            context: context,
            name: controller.name,
            email: controller.email,
            onSaved: (name, email) {
              controller.updateProfile(name, email);
              onShowSnack('Perfil actualizado.');
            },
          ),
        ),
        const SizedBox(height: 24),
        const SectionTitle(title: 'Ajustes de seguridad'),
        const SizedBox(height: 12),
        SettingsTile(
          icon: Icons.auto_awesome,
          title: 'Alertas automáticas',
          subtitle: 'Activar ayuda cuando NERA detecte señales críticas.',
          value: controller.automaticAlerts,
          onChanged: controller.setAutomaticAlerts,
        ),
        SettingsTile(
          icon: Icons.location_on_outlined,
          title: 'Compartir ubicación',
          subtitle: 'Incluir ubicación aproximada en emergencias.',
          value: controller.shareLocation,
          onChanged: controller.setShareLocation,
        ),
        SettingsTile(
          icon: Icons.notifications_active_outlined,
          title: 'Notificaciones',
          subtitle: 'Recordatorios de medición, descanso y calma.',
          value: controller.healthNotifications,
          onChanged: controller.setHealthNotifications,
        ),
        SettingsTile(
          icon: Icons.do_not_disturb_on_outlined,
          title: 'Modo calma',
          subtitle: 'Reducir estímulos visuales durante una crisis.',
          value: controller.calmMode,
          onChanged: controller.setCalmMode,
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
              const SizedBox(height: 4),
              Text(subtitle, style: Theme.of(context).textTheme.bodyMedium),
            ],
          ),
        ),
        if (action != null) action!,
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
    final status = stress < 35 ? 'Buen estado' : 'Necesita atención';
    final statusColor = stress < 35 ? AppTheme.success : AppTheme.danger;

    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(28),
        boxShadow: AppTheme.softShadow,
      ),
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
                      const SizedBox(height: 16),
                      const Text(
                        'Índice de bienestar',
                        style: TextStyle(
                          color: AppTheme.textMuted,
                          fontSize: 14,
                          fontWeight: FontWeight.w700,
                        ),
                      ),
                      const SizedBox(height: 4),
                      Text(
                        '$score',
                        style: const TextStyle(
                          fontSize: 62,
                          height: 1.0,
                          color: AppTheme.textDark,
                          fontWeight: FontWeight.w900,
                          letterSpacing: -1.5,
                        ),
                      ),
                    ],
                  ),
                ),
                SizedBox(
                  height: 110,
                  width: 110,
                  child: Stack(
                    fit: StackFit.expand,
                    children: [
                      CircularProgressIndicator(
                        value: score / 100,
                        strokeWidth: 10,
                        strokeCap: StrokeCap.round,
                        backgroundColor: AppTheme.textMuted.withOpacity(0.08),
                        color: AppTheme.primary,
                      ),
                      Center(
                        child: Text(
                          '$contactsCount\napoyos',
                          textAlign: TextAlign.center,
                          style: const TextStyle(
                            color: AppTheme.textMuted,
                            fontSize: 11,
                            fontWeight: FontWeight.w800,
                            height: 1.2,
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
                    icon: const Icon(Icons.sensors, size: 18),
                    label: const Text('Medir'),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: OutlinedButton.icon(
                    onPressed: onBreathing,
                    icon: const Icon(Icons.self_improvement, size: 18),
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
    final loading = state == WatchConnectionState.scanning || state == WatchConnectionState.connecting;
    final color = connected
        ? AppTheme.success
        : state == WatchConnectionState.failed
            ? AppTheme.danger
            : AppTheme.secondary;
    final label = connected
        ? 'Desconectar'
        : loading
            ? 'Buscando...'
            : 'Conectar';

    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(24),
        boxShadow: AppTheme.softShadow,
      ),
      child: Padding(
        padding: const EdgeInsets.all(18),
        child: Row(
          children: [
            CircleAvatar(
              backgroundColor: color.withOpacity(0.08),
              foregroundColor: color,
              radius: 22,
              child: Icon(connected ? Icons.watch : Icons.bluetooth_searching, size: 20),
            ),
            const SizedBox(width: 14),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text(
                    'Reloj NERA',
                    style: TextStyle(
                      color: AppTheme.textDark,
                      fontWeight: FontWeight.w800,
                      fontSize: 15,
                    ),
                  ),
                  const SizedBox(height: 3),
                  Text(
                    message,
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                    style: const TextStyle(color: AppTheme.textMuted, fontSize: 12),
                  ),
                ],
              ),
            ),
            const SizedBox(width: 10),
            FilledButton(
              onPressed: loading ? null : onConnect,
              style: FilledButton.styleFrom(
                backgroundColor: color,
                minimumSize: const Size(100, 42),
                shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
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
        childAspectRatio: 1.15,
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
    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(24),
        boxShadow: AppTheme.softShadow,
      ),
      child: Material(
        color: Colors.transparent,
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
                      backgroundColor: data.color.withOpacity(0.08),
                      foregroundColor: data.color,
                      radius: 18,
                      child: Icon(data.icon, size: 18),
                    ),
                    const Spacer(),
                    Icon(Icons.chevron_right, color: AppTheme.textMuted.withOpacity(0.5), size: 18),
                  ],
                ),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      data.title,
                      style: const TextStyle(color: AppTheme.textMuted, fontSize: 13, fontWeight: FontWeight.w600),
                    ),
                    const SizedBox(height: 2),
                    Text.rich(
                      TextSpan(
                        text: data.value,
                        style: const TextStyle(
                          color: AppTheme.textDark,
                          fontSize: 26,
                          fontWeight: FontWeight.w900,
                          letterSpacing: -0.5,
                        ),
                        children: [
                          TextSpan(
                            text: ' ${data.unit}',
                            style: const TextStyle(
                              color: AppTheme.textMuted,
                              fontSize: 12,
                              fontWeight: FontWeight.w700,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
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
        color: AppTheme.danger,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: AppTheme.danger.withOpacity(0.15),
            blurRadius: 16,
            offset: const Offset(0, 6),
          )
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Icon(Icons.health_and_safety, color: Colors.white, size: 30),
          const SizedBox(height: 14),
          const Text(
            'Protocolo de emergencia',
            style: TextStyle(
              color: Colors.white,
              fontSize: 20,
              fontWeight: FontWeight.w900,
            ),
          ),
          const SizedBox(height: 6),
          Text(
            'Envía una alerta inmediata con tu estado actual a tus contactos activos.',
            style: TextStyle(color: Colors.white.withOpacity(0.9), fontSize: 13),
          ),
          const SizedBox(height: 16),
          FilledButton.icon(
            style: FilledButton.styleFrom(
              backgroundColor: Colors.white,
              foregroundColor: AppTheme.danger,
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
            ),
            onPressed: onSendAlert,
            icon: const Icon(Icons.notification_important, size: 18),
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
      padding: const EdgeInsets.only(bottom: 14),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              Expanded(
                child: Text(
                  label,
                  style: const TextStyle(
                    color: AppTheme.textDark,
                    fontWeight: FontWeight.w700,
                    fontSize: 13,
                  ),
                ),
              ),
              Text(
                '${(normalized * 100).round()}%',
                style: const TextStyle(
                  color: AppTheme.textMuted,
                  fontWeight: FontWeight.w700,
                  fontSize: 13,
                ),
              ),
            ],
          ),
          const SizedBox(height: 6),
          ClipRRect(
            borderRadius: BorderRadius.circular(99),
            child: LinearProgressIndicator(
              minHeight: 8,
              value: normalized,
              color: color,
              backgroundColor: AppTheme.textMuted.withOpacity(0.08),
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
      padding: const EdgeInsets.only(bottom: 10),
      child: Container(
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(20),
          boxShadow: AppTheme.softShadow,
        ),
        child: ListTile(
          contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
          leading: CircleAvatar(
            backgroundColor: event.color.withOpacity(0.08),
            foregroundColor: event.color,
            radius: 18,
            child: Icon(event.icon, size: 18),
          ),
          title: Text(
            event.title,
            style: const TextStyle(fontWeight: FontWeight.w800, fontSize: 14, color: AppTheme.textDark),
          ),
          subtitle: Text(event.description, style: const TextStyle(fontSize: 12)),
          trailing: Text(
            formatTime(event.time),
            style: const TextStyle(
              color: AppTheme.textMuted,
              fontWeight: FontWeight.w700,
              fontSize: 12,
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
    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(20),
        boxShadow: AppTheme.softShadow,
      ),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(16, 12, 8, 12),
        child: Row(
          children: [
            CircleAvatar(
              radius: 20,
              backgroundColor: contact.active ? AppTheme.primary.withOpacity(0.08) : AppTheme.textMuted.withOpacity(0.08),
              foregroundColor: contact.active ? AppTheme.primary : AppTheme.textMuted,
              child: Text(
                initialsFor(contact.name),
                style: const TextStyle(fontWeight: FontWeight.w800, fontSize: 13),
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
                      color: AppTheme.textDark,
                      fontSize: 15,
                      fontWeight: FontWeight.w800,
                    ),
                  ),
                  const SizedBox(height: 2),
                  Text(
                    '${contact.relation} • ${contact.phone}',
                    style: const TextStyle(color: AppTheme.textMuted, fontSize: 12),
                  ),
                ],
              ),
            ),
            Switch(
              value: contact.active,
              onChanged: onToggle,
              activeColor: AppTheme.primary,
            ),
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
    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(24),
        boxShadow: AppTheme.softShadow,
      ),
      child: Padding(
        padding: const EdgeInsets.all(20),
        child: Row(
          children: [
            CircleAvatar(
              radius: 28,
              backgroundColor: AppTheme.primary.withOpacity(0.08),
              foregroundColor: AppTheme.primary,
              child: Text(
                initialsFor(name),
                style: const TextStyle(fontSize: 20, fontWeight: FontWeight.w800),
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
                      color: AppTheme.textDark,
                      fontSize: 18,
                      fontWeight: FontWeight.w800,
                    ),
                  ),
                  const SizedBox(height: 2),
                  Text(
                    email,
                    style: const TextStyle(color: AppTheme.textMuted, fontSize: 13),
                  ),
                ],
              ),
            ),
            IconButton.filledTonal(
              onPressed: onEdit,
              icon: const Icon(Icons.edit, size: 18),
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
      padding: const EdgeInsets.only(bottom: 10),
      child: Container(
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(20),
          boxShadow: AppTheme.softShadow,
        ),
        child: SwitchListTile(
          contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 6),
          secondary: CircleAvatar(
            backgroundColor: AppTheme.primary.withOpacity(0.06),
            foregroundColor: AppTheme.primary,
            radius: 18,
            child: Icon(icon, size: 18),
          ),
          title: Text(
            title,
            style: const TextStyle(
              color: AppTheme.textDark,
              fontWeight: FontWeight.w800,
              fontSize: 14,
            ),
          ),
          subtitle: Text(subtitle, style: const TextStyle(fontSize: 12)),
          value: value,
          onChanged: onChanged,
          activeColor: AppTheme.primary,
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
          TextButton(
            onPressed: onAction,
            child: Text(actionText!, style: const TextStyle(color: AppTheme.primary, fontWeight: FontWeight.bold)),
          ),
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
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
      decoration: BoxDecoration(
        color: color.withOpacity(0.08),
        borderRadius: BorderRadius.circular(99),
      ),
      child: Text(
        text,
        style: TextStyle(
          color: color,
          fontSize: 11,
          fontWeight: FontWeight.w800,
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
            height: 100,
            width: 100,
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: AppTheme.primary.withOpacity(0.08),
              border: Border.all(
                color: AppTheme.primary.withOpacity(0.15),
                width: 2,
              ),
            ),
            child: const Icon(
              Icons.self_improvement,
              size: 44,
              color: AppTheme.primary,
            ),
          ),
          const SizedBox(height: 16),
          const Text(
            'Respiración 4-4-6',
            style: TextStyle(
              color: AppTheme.textDark,
              fontSize: 22,
              fontWeight: FontWeight.w900,
            ),
          ),
          const SizedBox(height: 8),
          const Text(
            'Inhalá 4 segundos, sostené 4 y exhalá 6. Repetí el ciclo hasta sentir más calma.',
            textAlign: TextAlign.center,
            style: TextStyle(color: AppTheme.textMuted, fontSize: 13),
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
          const SizedBox(height: 24),
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
      padding: const EdgeInsets.symmetric(vertical: 10),
      decoration: BoxDecoration(
        color: AppTheme.textMuted.withOpacity(0.06),
        borderRadius: BorderRadius.circular(99),
      ),
      child: Text(
        text,
        textAlign: TextAlign.center,
        style: const TextStyle(
          color: AppTheme.textDark,
          fontSize: 11,
          fontWeight: FontWeight.w800,
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
  final relationController = TextEditingController(text: contact?.relation ?? '');
  final phoneController = TextEditingController(text: contact?.phone ?? '');
  var active = contact?.active ?? true;

  await showDialog<void>(
    context: context,
    builder: (context) {
      return StatefulBuilder(
        builder: (context, setDialogState) {
          return AlertDialog(
            title: Text(contact == null ? 'Agregar contacto' : 'Editar contacto'),
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
                    decoration: const InputDecoration(labelText: 'Relación'),
                  ),
                  const SizedBox(height: 12),
                  TextField(
                    controller: phoneController,
                    decoration: const InputDecoration(labelText: 'Teléfono'),
                    keyboardType: TextInputType.phone,
                  ),
                  SwitchListTile(
                    contentPadding: EdgeInsets.zero,
                    title: const Text('Activo para alertas', style: TextStyle(fontSize: 14)),
                    value: active,
                    onChanged: (value) => setDialogState(() => active = value),
                    activeColor: AppTheme.primary,
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
  final parts = text.trim().split(RegExp(r'\s+')).where((part) => part.isNotEmpty).toList();
  if (parts.isEmpty) return 'N';
  if (parts.length == 1) return parts.first.substring(0, 1).toUpperCase();
  return '${parts.first.substring(0, 1)}${parts.last.substring(0, 1)}'.toUpperCase();
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
