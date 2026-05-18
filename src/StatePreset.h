#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// StatePreset
//
// A named snapshot of one or more component fields on one or more ECS entities.
// Each FieldSnapshot stores the field's identity (entity UID, component name,
// field name) and an owned copy of its value via PropertyValue.
//
// Use StateLibrary to manage a collection of presets and apply / morph between
// them at runtime.
// ─────────────────────────────────────────────────────────────────────────────

#include "PropertyValue.h"
#include "PropertyReflector.h"
#include "ComponentInspector.h"
#include "ofJson.h"
#include <entt/entt.hpp>
#include <string>
#include <vector>

namespace state {

// ─── FieldSnapshot ────────────────────────────────────────────────────────────

struct FieldSnapshot {
    std::string             entityUid;      ///< Stable UID string (e.g. from tag_component)
    std::string             componentName;  ///< Matches the name used in registerProperties
    std::string             fieldName;      ///< Matches ReflectedProperty::name
    inspector::PropertyValue value;         ///< Owned deep-copy of the field value at snapshot time

    ofJson toJson() const {
        return {
            {"entityUid",      entityUid},
            {"componentName",  componentName},
            {"fieldName",      fieldName},
            {"type",           static_cast<int>(value.type)},
            {"value",          value.toJson()}
        };
    }

    static FieldSnapshot fromJson(const ofJson& j) {
        FieldSnapshot fs;
        fs.entityUid     = j["entityUid"].get<std::string>();
        fs.componentName = j["componentName"].get<std::string>();
        fs.fieldName     = j["fieldName"].get<std::string>();
        auto type        = static_cast<inspector::PinDataType>(j["type"].get<int>());
        fs.value         = inspector::PropertyValue::fromJson(j["value"], type);
        return fs;
    }
};

// ─── StatePreset ──────────────────────────────────────────────────────────────

struct StatePreset {
    std::string               name;
    std::vector<FieldSnapshot> fields;

    bool empty() const { return fields.empty(); }

    ofJson toJson() const {
        ofJson j;
        j["name"] = name;
        ofJson arr = ofJson::array();
        for (const auto& f : fields) arr.push_back(f.toJson());
        j["fields"] = arr;
        return j;
    }

    static StatePreset fromJson(const ofJson& j) {
        StatePreset p;
        p.name = j["name"].get<std::string>();
        for (const auto& fj : j["fields"]) p.fields.push_back(FieldSnapshot::fromJson(fj));
        return p;
    }
};

// ─── StateLibrary ─────────────────────────────────────────────────────────────
// Holds a list of presets and provides capture / apply / morph operations.

class StateLibrary {
public:
    // Capture a preset by snapshotting the current values of all registered fields
    // for the given entity. A ComponentInspector for each component is obtained
    // via the supplied reflect function.
    //
    // reflectFns maps component name → function that fills a ComponentInspector
    // from a live entity. This mirrors how Tanim's Registry works.
    void capture(
        const std::string& presetName,
        const std::string& entityUid,
        const std::vector<std::pair<std::string, std::function<void(inspector::ComponentInspector&)>>>& reflectFns)
    {
        StatePreset preset;
        preset.name = presetName;
        for (const auto& [compName, reflectFn] : reflectFns) {
            inspector::ComponentInspector ci("_");
            reflectFn(ci);
            for (const auto& prop : ci.getReflectedProperties()) {
                FieldSnapshot fs;
                fs.entityUid     = entityUid;
                fs.componentName = compName;
                fs.fieldName     = prop.name;
                fs.value         = inspector::PropertyValue::from(prop);
                preset.fields.push_back(std::move(fs));
            }
        }
        m_presets.push_back(std::move(preset));
    }

    // Apply a preset immediately (t=1) or morph toward it from the current live
    // values (0 < t < 1) by lerping each field.
    void apply(
        const StatePreset& preset,
        float t,
        const std::vector<std::pair<std::string, std::function<void(inspector::ComponentInspector&)>>>& reflectFns)
    {
        for (const auto& fs : preset.fields) {
            for (const auto& [compName, reflectFn] : reflectFns) {
                if (compName != fs.componentName) continue;
                inspector::ComponentInspector ci("_");
                reflectFn(ci);
                const inspector::ReflectedProperty* prop = ci.findProperty(fs.fieldName);
                if (!prop) continue;
                if (t >= 1.f) {
                    fs.value.apply(*prop);
                } else {
                    // Lerp from live value toward the preset value
                    inspector::PropertyValue current = inspector::PropertyValue::from(*prop);
                    inspector::PropertyValue::lerp(current, fs.value, t).apply(*prop);
                }
            }
        }
    }

    void apply(const std::string& presetName, float t,
               const std::vector<std::pair<std::string, std::function<void(inspector::ComponentInspector&)>>>& fn)
    {
        const StatePreset* p = find(presetName);
        if (p) apply(*p, t, fn);
    }

    // ── Accessors ────────────────────────────────────────────────────────────
    const std::vector<StatePreset>& presets() const { return m_presets; }
    std::vector<StatePreset>&       presets()       { return m_presets; }

    StatePreset* find(const std::string& name) {
        for (auto& p : m_presets) if (p.name == name) return &p;
        return nullptr;
    }
    const StatePreset* find(const std::string& name) const {
        for (const auto& p : m_presets) if (p.name == name) return &p;
        return nullptr;
    }

    void remove(const std::string& name) {
        m_presets.erase(
            std::remove_if(m_presets.begin(), m_presets.end(),
                           [&](const StatePreset& p) { return p.name == name; }),
            m_presets.end());
    }

    void clear() { m_presets.clear(); }
    bool empty() const { return m_presets.empty(); }

    // ── Serialization ────────────────────────────────────────────────────────
    ofJson toJson() const {
        ofJson arr = ofJson::array();
        for (const auto& p : m_presets) arr.push_back(p.toJson());
        return arr;
    }

    void fromJson(const ofJson& j) {
        m_presets.clear();
        for (const auto& pj : j) m_presets.push_back(StatePreset::fromJson(pj));
    }

private:
    std::vector<StatePreset> m_presets;
};

} // namespace state
