// MODIFICABLE: presentacion; conservar callbacks y datos al ajustar el diseno.
part of '../main.dart';

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
              if (!controller.preferencesReady) {
                onShowSnack(controller.preferencesStatus);
                return;
              }
              controller.updateProfile(name, email);
              onShowSnack('Perfil actualizado.');
            },
          ),
        ),
        const SizedBox(height: 24),
        const SectionTitle(title: 'Tu configuración'),
        const SizedBox(height: 12),
        SettingsTile(
          icon: Icons.auto_awesome,
          title: 'Avisos de prueba a la vincha',
          subtitle: 'Permitir eventos de pulso y temperatura por ESP-NOW. La bobina sigue deshabilitada.',
          value: controller.automaticAlerts,
          onChanged: controller.preferencesReady ? controller.setAutomaticAlerts : null,
        ),
        Card(
          child: Padding(
            padding: const EdgeInsets.all(16),
            child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
              const Text('Sensibilidad de los avisos', style: TextStyle(fontWeight: FontWeight.bold)),
              const SizedBox(height: 8),
              DropdownButton<AlertSensitivity>(
                isExpanded: true,
                value: controller.sensitivity,
                items: const [
                  DropdownMenuItem(value: AlertSensitivity.high, child: Text('Alta · confirmar durante 5 s')),
                  DropdownMenuItem(value: AlertSensitivity.medium, child: Text('Media · confirmar durante 10 s')),
                  DropdownMenuItem(value: AlertSensitivity.low, child: Text('Baja · confirmar durante 20 s')),
                ],
                onChanged: !controller.preferencesReady ? null : (value) {
                  if (value != null) controller.setSensitivity(value);
                },
              ),
              const Text('Una confirmación más larga evita avisos por cambios breves. Opciones de laboratorio, sin validación clínica.',
                style: TextStyle(color: AppTheme.textMuted)),
              const SizedBox(height: 16),
              const Text('Actividad actual', style: TextStyle(fontWeight: FontWeight.bold)),
              DropdownButton<ActivityMode>(
                isExpanded: true,
                value: controller.activityMode,
                items: const [
                  DropdownMenuItem(value: ActivityMode.everyday, child: Text('Uso cotidiano')),
                  DropdownMenuItem(value: ActivityMode.exercise, child: Text('Ejercicio')),
                ],
                onChanged: !controller.preferencesReady ? null : (value) {
                  if (value != null) controller.setActivityMode(value);
                },
              ),
              const Text('Ejercicio pausa los avisos experimentales de pulso; mantiene temperatura. No detecta ansiedad. Cambiá a Cotidiano al terminar.',
                style: TextStyle(color: AppTheme.textMuted)),
              const SizedBox(height: 16),
              const Text('Límites de prueba: pulso >100 BPM y objeto IR >33 °C. No son límites médicos ni temperatura corporal central.',
                style: TextStyle(color: AppTheme.textMuted)),
              const SizedBox(height: 12),
              Text(controller.preferencesStatus),
              const SizedBox(height: 8),
              Text(controller.watchPreferencesStatus),
              TextButton.icon(
                onPressed: controller.preferencesReady ? () => controller.syncPreferences() : null,
                icon: const Icon(Icons.sync),
                label: const Text('Reintentar guardado y sincronización'),
              ),
              const Text('Los contactos se guardan en este teléfono. Todavía no se les envían avisos automáticos.',
                style: TextStyle(color: AppTheme.textMuted)),
            ]),
          ),
        ),
        const SizedBox(height: 16),
        const SectionTitle(title: 'Otras funciones pendientes'),
        SettingsTile(
          icon: Icons.location_on_outlined,
          title: 'Compartir ubicación',
          subtitle: 'Pendiente: no se comparte ubicación.',
          value: controller.shareLocation,
          onChanged: null,
        ),
        SettingsTile(
          icon: Icons.notifications_active_outlined,
          title: 'Notificaciones',
          subtitle: 'Pendiente: sin notificaciones automáticas.',
          value: controller.healthNotifications,
          onChanged: null,
        ),
        SettingsTile(
          icon: Icons.do_not_disturb_on_outlined,
          title: 'Modo calma',
          subtitle: 'Pendiente de integración.',
          value: controller.calmMode,
          onChanged: null,
        ),
      ],
    );
  }
}

