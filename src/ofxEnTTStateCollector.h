#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ofxEnTTStateCollector
//
// Captures and applies named ECS field-level presets, with optional morphing
// and a lightweight preset-event timeline.
//
// Architecture overview:
//   • StateLibrary     — owns a list of StatePresets. Thread-safe if access is
//                        serialized to the main thread.
//   • StatePreset      — a named snapshot of FieldSnapshot items.
//   • FieldSnapshot    — entity UID + component name + field name + PropertyValue.
//   • PropertyValue    — from ofxEnTTInspector: an owned deep-copy of a field.
//
//   • StateCollectorPanel — ImGui UI for the library (capture / apply / morph).
//   • TimelinePanel       — ImGui UI for a sequence of preset events.
//
// Quick start:
//   #include "ofxEnTTStateCollector.h"
//
//   state::StateLibrary library;
//
//   // Describe what to capture (must match registerProperties specializations):
//   state::StateCollectorPanel::ReflectFnList reflectFns = {
//       {"transform", [&](inspector::ComponentInspector& ci) {
//           auto& xf = registry.get<transform_component>(entity);
//           ci.addProperty("x", &xf.x, -1000.f, 1000.f);
//           ci.addProperty("y", &xf.y, -1000.f, 1000.f);
//       }}
//   };
//
//   state::StateCollectorPanel panel;
//   panel.setLibrary(&library);
//   panel.setReflectFns(reflectFns);
//   panel.setEntityUid("my_entity_001");
//
//   // In draw loop:
//   panel.draw();
// ─────────────────────────────────────────────────────────────────────────────

#include "StatePreset.h"
#include "StateCollectorPanel.h"
#include "TimelinePanel.h"
