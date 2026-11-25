// MODIFICABLE: presentacion; conservar callbacks y datos al ajustar el diseno.
part of '../main.dart';

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
                    const StatusChip(
                      text: 'Lecturas del reloj',
                      color: AppTheme.primary,
                    ),
                  ],
                ),
                const SizedBox(height: 20),
                Text(
                  'Pulso: ${controller.heartRate?.toString() ?? '--'} bpm',
                  style: const TextStyle(color: AppTheme.textDark),
                ),
                const SizedBox(height: 12),
                Text(
                  'Objeto IR sin contacto: ${controller.temperature?.toStringAsFixed(1) ?? '--'} °C',
                  style: const TextStyle(color: AppTheme.textDark),
                ),
                const SizedBox(height: 12),
                Text(
                  'SpO₂ estimada: ${controller.oxygen?.toString() ?? '--'} % · Sueño y estrés: sin medición',
                  style: const TextStyle(color: AppTheme.textMuted),
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
        const SizedBox(height: 24),
        const SectionTitle(title: 'Historial del reloj'),
        const SizedBox(height: 8),
        Text(controller.historyStatus,
            style: const TextStyle(color: AppTheme.textMuted)),
        for (final entry in controller.latestHistory)
          ListTile(
            contentPadding: EdgeInsets.zero,
            leading: const Icon(Icons.watch_later_outlined, color: AppTheme.primary),
            title: Text('Pulso: ${entry.bpm?.toString() ?? '--'} bpm  ·  '
                'Objeto IR: ${entry.objectCelsius?.toStringAsFixed(1) ?? '--'} °C'),
            subtitle: Text(entry.localTime == null ? 'Hora no disponible' :
                '${entry.localTime!.day}/${entry.localTime!.month} '
                '${entry.localTime!.hour.toString().padLeft(2, '0')}:'
                '${entry.localTime!.minute.toString().padLeft(2, '0')}'),
          ),
      ],
    );
  }
}

