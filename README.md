# ofxEnTTStateCollector

ECS field-level state presets with capture, apply, morph, and a lightweight preset-event timeline for openFrameworks.

Built on [ofxEnTTInspector](../ofxEnTTInspector)'s `PropertyValue` and `ReflectedProperty` system.

---

## What it does

- **Capture** named snapshots of any subset of ECS component fields across any set of entities.
- **Apply** a preset instantly or **morph** toward it smoothly at a configurable blend amount `t ∈ [0, 1]`.
- **TimelinePanel** places preset-events on a frame ruler; as the playhead passes an event the preset is snapped or morphed in.
- **Serialize / deserialize** via ofJSON (no external library needed).

This is a *preset* system, not a per-field curve editor. For per-field Bezier-curve animation use [ofxTanim]([..](https://github.com/ofxyz/ofxTanim). Both addons can be used together — Tanim handles smooth curves, StateCollector handles high-level scene-state switching.

---

## Dependencies

| Addon | Why |
|-------|-----|
| `ofxEnTTKit` | ECS types |
| `ofxImGui` | ImGui panels |
| `ofxEnTTInspector` | `PropertyValue`, `ReflectedProperty`, `PinDataType`, `ComponentInspector` |

---

## Quick start

### 1 — Describe what to capture (ReflectFnList)

```cpp
#include "ofxEnTTStateCollector.h"

// A list of (component_name → reflect function) pairs.
// The reflect function fills a ComponentInspector with the live component's fields.
state::StateCollectorPanel::ReflectFnList reflectFns = {
    {"transform", [&](inspector::ComponentInspector& ci) {
        auto& xf = registry.get<transform_component>(entity);
        ci.addProperty("x",        &xf.x,         -1000.f, 1000.f);
        ci.addProperty("y",        &xf.y,         -1000.f, 1000.f);
        ci.addProperty("rotation", &xf.rotation,  -360.f,  360.f);
    }},
    {"appearance", [&](inspector::ComponentInspector& ci) {
        auto& ap = registry.get<appearance_component>(entity);
        ci.addProperty("color",   &ap.color);
        ci.addProperty("opacity", &ap.opacity, 0.f, 1.f);
    }}
};
```

Alternatively, if you have `registerProperties<T>` specializations in `ofxEnTTInspector`, you can call them here directly:

```cpp
{"transform", [&](inspector::ComponentInspector& ci) {
    inspector::registerProperties(registry.get<transform_component>(entity), ci);
}}
```

### 2 — Set up the panel

```cpp
state::StateLibrary      library;
state::StateCollectorPanel panel;

panel.setLibrary(&library);
panel.setReflectFns(reflectFns);
panel.setEntityUid("player_001");  // stable UID used in serialization
```

### 3 — Draw the panel (inside ImGui frame)

```cpp
if (ImGui::Begin("State Collector")) {
    panel.draw();
}
ImGui::End();
```

### 4 — Apply presets from code

```cpp
// Instant snap to preset "fire":
library.apply("fire", 1.f, reflectFns);

// Morph 50% toward preset "ice":
library.apply("ice", 0.5f, reflectFns);
```

---

## TimelinePanel

```cpp
state::TimelinePanel timeline;
timeline.setLibrary(&library);
timeline.setReflectFns(reflectFns);
timeline.setLength(300);  // total frames

// Per frame (in your update loop):
timeline.update(currentFrame);

// Add preset events:
timeline.addEvent(0,   "intro",  0.f);   // snap at frame 0
timeline.addEvent(60,  "fire",   10.f);  // morph over 10 frames starting at frame 60
timeline.addEvent(150, "ice",    0.f);

// Draw (inside ImGui frame):
if (ImGui::Begin("Timeline")) {
    timeline.draw();
}
ImGui::End();
```

---

## Serialization

```cpp
// Save the library to a JSON file:
ofJson j = library.toJson();
std::ofstream("state_library.json") << j.dump(2);

// Load it back:
std::ifstream f("state_library.json");
ofJson j; f >> j;
library.fromJson(j);
```

Individual presets and field snapshots also serialize via `StatePreset::toJson()` / `fromJson()`.

---

## API reference

### StateLibrary

```cpp
void capture(const std::string& presetName, const std::string& entityUid, ReflectFnList& fns);
void apply(const StatePreset& preset, float t, ReflectFnList& fns);
void apply(const std::string& presetName, float t, ReflectFnList& fns);
StatePreset* find(const std::string& name);
void remove(const std::string& name);
void clear();
const std::vector<StatePreset>& presets() const;
ofJson toJson() const;
void fromJson(const ofJson& j);
```

### StateCollectorPanel

```cpp
void setLibrary(StateLibrary*);
void setReflectFns(ReflectFnList&);
void setEntityUid(const std::string&);
void draw();
```

### TimelinePanel

```cpp
void setLibrary(StateLibrary*);
void setReflectFns(ReflectFnList&);
void setLength(int frames);
void update(int frame);
void draw();
void addEvent(int frame, const std::string& presetName, float morphFrames = 0.f);
void removeEvent(int index);
int  getCurrentFrame() const;
```
