#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// StateCollectorPanel
//
// An ImGui panel for managing a StateLibrary. Provides:
//   • Capture — snapshot current field values into a new named preset
//   • Apply   — apply a preset immediately (t = 1.0)
//   • Morph   — tween live values toward a preset at a configurable blend t
//   • Delete  — remove a preset
//   • Save / Load — serialize the library to/from a JSON file
//
// Usage:
//   state::StateCollectorPanel panel;
//   panel.setLibrary(&myLibrary);
//   panel.setReflectFns(myReflectFns);
//
//   // In draw loop:
//   panel.draw();
// ─────────────────────────────────────────────────────────────────────────────

#include "StatePreset.h"
#include "imgui.h"
#include "ofFileUtils.h"
#include <string>

namespace state {

class StateCollectorPanel {
public:
    using ReflectFnList =
        std::vector<std::pair<std::string,
                              std::function<void(inspector::ComponentInspector&)>>>;

    void setLibrary(StateLibrary* lib)        { m_library = lib; }
    void setReflectFns(ReflectFnList& fns)    { m_reflectFns = &fns; }
    void setEntityUid(const std::string& uid) { m_entityUid = uid; }

    void draw();

private:
    StateLibrary*   m_library    = nullptr;
    ReflectFnList*  m_reflectFns = nullptr;
    std::string     m_entityUid;
    char            m_captureNameBuf[128] = "preset_1";
    float           m_morphT = 1.0f;
    int             m_selectedIdx = -1;
};

} // namespace state
