#include "TimelinePanel.h"
#include <algorithm>

namespace state {

void TimelinePanel::update(int frame)
{
    if (m_currentFrame == frame) return;
    evaluateEvents(frame);
    m_lastFrame    = m_currentFrame;
    m_currentFrame = frame;
}

void TimelinePanel::evaluateEvents(int frame)
{
    if (!m_library || !m_reflectFns) return;
    const int prev = m_lastFrame;
    const int curr = frame;
    if (curr <= prev) return;

    for (const auto& ev : m_events) {
        if (ev.frame > prev && ev.frame <= curr) {
            if (ev.morphFrames <= 0.f) {
                m_library->apply(ev.presetName, 1.f, *m_reflectFns);
            } else {
                // Partial morph on the exact trigger frame;
                // caller can continue calling update for a smooth transition.
                float t = 1.f / ev.morphFrames;
                m_library->apply(ev.presetName, t, *m_reflectFns);
            }
        }
    }
}

void TimelinePanel::addEvent(int frame, const std::string& presetName, float morphFrames)
{
    TimelineEvent ev;
    ev.frame       = std::clamp(frame, 0, m_totalFrames);
    ev.presetName  = presetName;
    ev.morphFrames = morphFrames;
    m_events.push_back(ev);
    std::sort(m_events.begin(), m_events.end(),
              [](const TimelineEvent& a, const TimelineEvent& b) {
                  return a.frame < b.frame;
              });
}

void TimelinePanel::removeEvent(int index)
{
    if (index < 0 || index >= (int)m_events.size()) return;
    m_events.erase(m_events.begin() + index);
    if (m_selectedEvent >= (int)m_events.size()) m_selectedEvent = -1;
}

void TimelinePanel::draw()
{
    if (!m_library) {
        ImGui::TextDisabled("TimelinePanel: library not set.");
        return;
    }

    // ── Transport ─────────────────────────────────────────────────────────────
    ImGui::SeparatorText("Timeline");
    int frame = m_currentFrame;
    ImGui::SetNextItemWidth(300.f);
    if (ImGui::SliderInt("Frame##tlframe", &frame, 0, m_totalFrames))
    {
        evaluateEvents(frame);
        m_lastFrame    = m_currentFrame;
        m_currentFrame = frame;
    }
    ImGui::SameLine();
    ImGui::Text("/ %d", m_totalFrames);

    // ── Event list ────────────────────────────────────────────────────────────
    ImGui::SeparatorText("Events");
    if (m_events.empty()) {
        ImGui::TextDisabled("(no events — right-click to add)");
    } else {
        if (ImGui::BeginTable("##events", 4,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Frame",   ImGuiTableColumnFlags_WidthFixed, 60.f);
            ImGui::TableSetupColumn("Preset",  ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Morph f", ImGuiTableColumnFlags_WidthFixed, 60.f);
            ImGui::TableSetupColumn("##del",   ImGuiTableColumnFlags_WidthFixed, 24.f);
            ImGui::TableHeadersRow();

            for (int i = 0; i < (int)m_events.size(); ++i) {
                auto& ev = m_events[i];
                ImGui::TableNextRow();
                ImGui::PushID(i);

                ImGui::TableSetColumnIndex(0);
                ImGui::SetNextItemWidth(-1.f);
                ImGui::InputInt("##f", &ev.frame, 0, 0);
                ev.frame = std::clamp(ev.frame, 0, m_totalFrames);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(ev.presetName.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(-1.f);
                ImGui::InputFloat("##m", &ev.morphFrames, 0.f, 0.f, "%.0f");

                ImGui::TableSetColumnIndex(3);
                if (ImGui::SmallButton("X")) {
                    removeEvent(i);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    // ── Add event popup ───────────────────────────────────────────────────────
    if (ImGui::BeginPopupContextItem("##tlctx")) {
        ImGui::TextDisabled("Add event at frame %d", m_currentFrame);
        ImGui::Separator();
        if (m_library) {
            for (const auto& p : m_library->presets()) {
                if (ImGui::MenuItem(p.name.c_str())) {
                    addEvent(m_currentFrame, p.name);
                }
            }
        }
        ImGui::EndPopup();
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        ImGui::OpenPopup("##tlctx");
}

} // namespace state
