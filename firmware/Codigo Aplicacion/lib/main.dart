import 'package:flutter/material.dart';
import 'app_theme.dart';
import 'models.dart';
import 'nera_controller.dart';
import 'watch_ble_service.dart';
import 'user_preferences.dart';
import 'watch_alert.dart';

// FUNCIONAL: partes de la misma biblioteca; comparten los imports de este archivo.
part 'screens/home_view.dart';
part 'screens/health_view.dart';
part 'screens/support_view.dart';
part 'screens/profile_view.dart';
part 'widgets/nera_widgets.dart';
part 'widgets/breathing_sheet.dart';
part 'dialogs/profile_contact_dialogs.dart';
part 'utils/display_format.dart';

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

class _NeraShellState extends State<NeraShell> with WidgetsBindingObserver {
  int _selectedIndex = 0;
  late final NeraController _controller;
  final Set<String> _shownAlerts = {};

  @override
  void initState() {
    super.initState();
    _controller = NeraController();
    _controller.addListener(_announceDeviceAlerts);
    WidgetsBinding.instance.addObserver(this);
  }

  void _announceDeviceAlerts() {
    for (final alert in _controller.currentAlerts.values) {
      if (!alert.active ||
          (alert.phase != AlertDeliveryPhase.waitingApp &&
              alert.phase != AlertDeliveryPhase.failed)) {
        continue;
      }
      final key = '${alert.key}:${alert.phase.name}';
      if (!_shownAlerts.add(key)) continue;
      if (_shownAlerts.length > 200) _shownAlerts.remove(_shownAlerts.first);
      final message = alert.phase == AlertDeliveryPhase.failed
          ? '${alert.label}: la vincha no confirmó la orden.'
          : 'Alerta: ${alert.label.toLowerCase()} ${alert.value}. Enviando a la vincha...';
      WidgetsBinding.instance.addPostFrameCallback((_) {
        if (mounted) _showSnack(message);
      });
    }
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    _controller.setAppForeground(state == AppLifecycleState.resumed);
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    _controller.removeListener(_announceDeviceAlerts);
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
    // FUNCIONAL: el envío a contactos todavía no existe. No informar éxito falso.
    showDialog<void>(
      context: context,
      builder: (context) => AlertDialog(
        icon: const Icon(
          Icons.notification_important,
          color: AppTheme.danger,
          size: 34,
        ),
        title: const Text('Alertas aún no disponibles'),
        content: const Text(
          'La app todavía no envía mensajes ni comparte tu ubicación. '
          'Si necesitás ayuda, llamá o escribí directamente a un contacto.',
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
          ProfileScreen(controller: _controller, onShowSnack: _showSnack),
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
                ),
              ],
            ),
            child: NavigationBar(
              selectedIndex: _selectedIndex,
              onDestinationSelected: (index) =>
                  setState(() => _selectedIndex = index),
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
                  selectedIcon: Icon(
                    Icons.monitor_heart,
                    color: AppTheme.primary,
                  ),
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
