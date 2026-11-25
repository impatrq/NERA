// lib/models.dart
import 'package:flutter/material.dart';

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