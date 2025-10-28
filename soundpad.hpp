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
            auto baked = letterFont->GetFontBaked(size.y);
            auto hGlyph = baked->FindGlyph('H');
            auto charHeight = hGlyph->Y1 - hGlyph->Y0;
            //SDL_Log("Font size: %f, Y0 %f, Y1 %f, diff %f", size.y, hGlyph->Y0, hGlyph->Y1, charHeight);
            for (auto &row : pads) {
                for (auto &pad : row) {
                    if (pad.render(size, interactive, letterFont, (7.0/8.0) * size.y * (size.y / charHeight))) {
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
