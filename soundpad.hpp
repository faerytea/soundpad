#include "preface.hpp"
#include <string>
#include <vector>
#include "Pad.hpp"

Pad *ShowSoundPad(SoundPad &pads, bool interactive, ImFont *letterFont) {
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushFont(NULL, 30);
    Pad *options = nullptr;
    if (ImGui::Begin("Actual pad", NULL, flags)) {
        size_t w = 0, h = pads.size();
        for (auto &row : pads) {
            w = std::max(w, row.size());
        }
        if (w != 0 && h != 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            unsigned padSize = std::min(viewport->WorkSize.y / h, viewport->WorkSize.x / w);
            auto size = ImVec2(padSize, padSize);
            for (auto &row : pads) {
                for (auto &pad : row) {
                    if (pad.render(size, interactive, letterFont)) {
                        options = &pad;
                    }
                    ImGui::SameLine();
                }
                ImGui::NewLine();
            }
            ImGui::PopStyleVar();
        }
    }
    ImGui::PopFont();
    ImGui::End();
    return options;
}
