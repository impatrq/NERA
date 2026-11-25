# NERA app: mapa del código

En la etapa 1 se dividió `lib/main.dart` conservando los mismos cuerpos de código.
Las pantallas siguen perteneciendo a la misma biblioteca Dart mediante `part` y
`part of`. Comparten los imports de `main.dart`; no hace falta duplicarlos en cada
pantalla. Esta separación permite encontrar cada parte sin rediseñar la app.

| Quiero encontrar... | Archivo |
| --- | --- |
| Arranque, tema y navegación principal | `lib/main.dart` |
| Inicio | `lib/screens/home_view.dart` |
| Salud | `lib/screens/health_view.dart` |
| Apoyo | `lib/screens/support_view.dart` |
| Perfil y configuración | `lib/screens/profile_view.dart` |
| Tarjetas, métricas, encabezados y controles compartidos | `lib/widgets/nera_widgets.dart` |
| Panel de respiración | `lib/widgets/breathing_sheet.dart` |
| Diálogos de contacto y perfil | `lib/dialogs/profile_contact_dialogs.dart` |
| Formato de hora, nombre e iniciales | `lib/utils/display_format.dart` |
| Estado y acciones de la app | `lib/nera_controller.dart` |
| Búsqueda, conexión y comunicación BLE | `lib/watch_ble_service.dart` |
| Preferencias del usuario | `lib/user_preferences.dart` |
| Colores y estilo compartido | `lib/app_theme.dart` |
| Modelos de datos | `lib/models.dart` |

## Qué se puede modificar

- **MODIFICABLE:** distribución y textos en las pantallas y widgets; los colores
  compartidos están en `app_theme.dart`. El diseño se trabajará en otra etapa.
- **FUNCIONAL:** callbacks y lectura del controlador. Conservarlos cuando se
  cambie la apariencia para no desconectar los botones de sus acciones.
- **IMPORTANTE:** `watch_ble_service.dart` contiene el protocolo BLE. No cambiar
  UUID, formato o permisos solamente para modificar la presentación.

Los archivos previos `healt_screen.dart`, `home_screens.dart`,
`profile_screen.dart` y `support_screen.dart` se conservaron. No son las pantallas
conectadas a `main.dart` en esta versión; las activas terminan en `_view.dart`.

Esta etapa no modifica la conexión BLE, las reglas de alerta ni implementa envío
de mensajes a familiares. El respaldo previo de la app está en:

`C:\Users\boca_\OneDrive\Documentos\NERA_Respaldos\2026-11-07_etapa0_bf5bf1ee\app_antes_de_ordenar.zip`

Las comprobaciones de software no sustituyen la prueba del teléfono con el reloj.
La app y el firmware del reloj se compilan e instalan por separado.
