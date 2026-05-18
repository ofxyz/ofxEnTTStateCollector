#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// TimelinePanel
//
// A simple preset-event timeline panel. Events are placed on a frame ruler;
// when the playhead passes an event the corresponding preset is applied (or
// morphed over a configurable number of frames).
//
// This is intentionally lightweight — it is not a replacement for ofxTanim
// (which provides per-field Bezier curves). Use this when you want to switch
// between whole presets over time rather than animate individual fields.
//
// Usage:
//   state::TimelinePanel tl;
//   tl.setLibrary(&myLibrary);
//   tl.setReflectFns(myReflectFns);
//   tl.setLength(300);  // frames
//
//   // Per-frame update:
//   tl.update(currentFrame);
//
//   // In draw loop:
//   tl.draw();
// ─────────────────────────────────────────────────────────────────────────────

#include "StatePreset.h"
#include "imgui.h"
#include <string>
#include <vector>

namespace state {

struct TimelineEvent {
    int         frame     = 0;
    std::string presetName;
    float       morphFrames = 0.f;  ///< 0 = snap, >0 = morph over N frames
};

class TimelinePanel {
public:
    using ReflectFnList =
        std::vector<std::pair<std::string,
                              std::function<void(inspector::ComponentInspector&)>>>;

    void setLibrary(StateLibrary* lib)        { m_library = lib; }
    void setReflectFns(ReflectFnList& fns)    { m_reflectFns = &fns; }
    void setLength(int frames)                { m_totalFrames = frames; }
    int  getCurrentFrame() const              { return m_currentFrame; }

    // Call every frame during playback to evaluate preset events.
    void update(int frame);

    // Draw the timeline UI (playhead + event lane).
    void draw();

    // Add / remove events
    void addEvent(int frame, const std::string& presetName, float morphFrames = 0.f);
    void removeEvent(int index);

    const std::vector<TimelineEvent>& events() const { return m_events; }

private:
    StateLibrary*    m_library      = nullptr;
    ReflectFnList*   m_reflectFns   = nullptr;
    int              m_totalFrames  = 300;
    int              m_currentFrame = 0;
    int              m_lastFrame    = -1;
    bool             m_playing      = false;
    std::vector<TimelineEvent> m_events;

    int              m_selectedEvent = -1;

    void evaluateEvents(int frame);
};

} // namespace state
