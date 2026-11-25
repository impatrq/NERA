// MODIFICABLE: presentacion; conservar callbacks y datos al ajustar el diseno.
part of '../main.dart';

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

