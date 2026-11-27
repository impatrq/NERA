import 'package:flutter_test/flutter_test.dart';

import 'package:aplicacion_nera/main.dart';

void main() {
  testWidgets('NERA shows the main dashboard', (WidgetTester tester) async {
    await tester.pumpWidget(const NeraApp());

    expect(find.text('Sin lecturas reales'), findsOneWidget);
    expect(find.text('Reloj NERA'), findsOneWidget);
    expect(find.text('Alertas y vincha'), findsOneWidget);
    await tester.scrollUntilVisible(find.text('Señales principales'), 200);
    expect(find.text('Señales principales'), findsOneWidget);
    expect(find.text('76'), findsNothing);
    expect(find.text('Inicio'), findsOneWidget);
  });
}
