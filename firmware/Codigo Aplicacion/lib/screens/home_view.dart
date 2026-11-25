// MODIFICABLE: presentacion; conservar callbacks y datos al ajustar el diseno.
part of '../main.dart';

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
          hasReading: controller.lastReadingAt != null,
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
              value: controller.heartRate?.toString() ?? '--',
              unit: 'bpm',
              icon: Icons.favorite,
              color: AppTheme.danger,
              detail: 'Última lectura válida de pulso recibida del reloj.',
            ),
            MetricData(
              title: 'SpO₂ estimada',
              value: controller.oxygen?.toString() ?? '--',
              unit: '%',
              icon: Icons.air,
              color: AppTheme.success,
              detail: 'Estimación experimental del MAX30102; requiere dedo quieto. Sin calibración clínica.',
            ),
            MetricData(
              title: 'Estrés',
              value: '--',
              unit: '',
              icon: Icons.psychology,
              color: const Color(0xFFF59E0B),
              detail: 'Aún no existe una medición de estrés validada.',
            ),
            MetricData(
              title: 'Temp.',
              value: controller.temperature?.toStringAsFixed(1) ?? '--',
              unit: '°C',
              icon: Icons.thermostat,
              color: const Color(0xFFEC4899),
              detail: 'Objeto IR del MLX90614: apuntar sin apoyar el sensor. No es temperatura corporal central.',
            ),
          ],
        ),
      ],
    );
  }
}

