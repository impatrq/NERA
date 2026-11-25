// MODIFICABLE: presentacion; conservar callbacks y datos al ajustar el diseno.
part of '../main.dart';

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
          subtitle: 'Contactos guardados en este teléfono; sin envío automático.',
          action: IconButton.filledTonal(
            onPressed: () => showContactDialog(
              context: context,
              onSaved: (contact) {
                if (!controller.preferencesReady) {
                  onShowSnack(controller.preferencesStatus);
                  return;
                }
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
              if (!controller.preferencesReady) {
                onShowSnack(controller.preferencesStatus);
                return;
              }
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

