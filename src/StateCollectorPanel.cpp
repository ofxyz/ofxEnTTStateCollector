#include "StateCollectorPanel.h"
#include "ofJson.h"
#include <fstream>

namespace state {

void StateCollectorPanel::draw()
{
    if (!m_library || !m_reflectFns) {
        ImGui::TextDisabled("StateCollectorPanel: library or reflect functions not set.");
        return;
    }

    // ── Capture ──────────────────────────────────────────────────────────────
    ImGui::SeparatorText("Capture");
    ImGui::SetNextItemWidth(200.f);
    ImGui::InputText("Name##capture", m_captureNameBuf, sizeof(m_captureNameBuf));
    ImGui::SameLine();
    if (ImGui::Button("Capture")) {
        m_library->capture(m_captureNameBuf, m_entityUid, *m_reflectFns);
        // Suggest next name
        snprintf(m_captureNameBuf, sizeof(m_captureNameBuf),
                 "preset_%d", (int)m_library->presets().size() + 1);
    }

    // ── Preset list ──────────────────────────────────────────────────────────
    ImGui::SeparatorText("Presets");
    const auto& presets = m_library->presets();

    if (presets.empty()) {
        ImGui::TextDisabled("(no presets)");
    } else {
        for (int i = 0; i < (int)presets.size(); ++i) {
            const auto& p = presets[i];
            const bool selected = (m_selectedIdx == i);

            if (ImGui::Selectable(p.name.c_str(), selected)) m_selectedIdx = i;

            ImGui::SameLine();
            ImGui::PushID(i);

            if (ImGui::SmallButton("Apply")) {
                m_library->apply(p, 1.f, *m_reflectFns);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Morph")) {
                m_library->apply(p, m_morphT, *m_reflectFns);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("X")) {
                m_library->remove(p.name);
                if (m_selectedIdx >= (int)m_library->presets().size())
                    m_selectedIdx = (int)m_library->presets().size() - 1;
                ImGui::PopID();
                break;  // list invalidated
            }
            ImGui::PopID();
        }
    }

    // ── Morph amount ─────────────────────────────────────────────────────────
    ImGui::SeparatorText("Morph amount");
    ImGui::SetNextItemWidth(200.f);
    ImGui::SliderFloat("##morphT", &m_morphT, 0.f, 1.f, "t = %.2f");

    // ── Preset field details ─────────────────────────────────────────────────
    if (m_selectedIdx >= 0 && m_selectedIdx < (int)presets.size()) {
        const auto& p = presets[m_selectedIdx];
        ImGui::SeparatorText(("Fields: " + p.name).c_str());
        if (ImGui::BeginTable("##fields", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY, ImVec2(0, 150))) {
            ImGui::TableSetupColumn("Component");
            ImGui::TableSetupColumn("Field");
            ImGui::TableSetupColumn("Type");
            ImGui::TableHeadersRow();
            for (const auto& f : p.fields) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(f.componentName.c_str());
                ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(f.fieldName.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(inspector::getPinTypeName(f.value.type));
            }
            ImGui::EndTable();
        }
    }

    // ── Save / Load ──────────────────────────────────────────────────────────
    ImGui::SeparatorText("Persistence");
    if (ImGui::Button("Save JSON")) {
        std::ofstream file("state_library.json");
        if (file.is_open()) file << m_library->toJson().dump(2);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load JSON")) {
        std::ifstream file("state_library.json");
        if (file.is_open()) {
            ofJson j;
            file >> j;
            m_library->fromJson(j);
            m_selectedIdx = -1;
        }
    }
}

} // namespace state
