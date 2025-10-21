#include "Utils.hpp"

ImGuiKey ImGuiKeyFromChar(char c) {
    if (c >= 'a' && c <= 'z') {
        return (ImGuiKey)(ImGuiKey_A + (c - 'a'));
    } else if (c >= 'A' && c <= 'Z') {
        return (ImGuiKey)(ImGuiKey_A + (c - 'A'));
    } else if (c >= '0' && c <= '9') {
        return (ImGuiKey)(ImGuiKey_0 + (c - '0'));
    }
    return ImGuiKey_None;
}