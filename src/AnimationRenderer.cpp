#include "AnimationRenderer.h"

#include <cmath>

#include "imgui.h"

namespace {

ImU32 Color(float r, float g, float b, float a = 1.0f) {
    return IM_COL32(
        static_cast<int>(r * 255.0f),
        static_cast<int>(g * 255.0f),
        static_cast<int>(b * 255.0f),
        static_cast<int>(a * 255.0f));
}

void DrawCloud(ImDrawList* draw, ImVec2 center, float scale, ImU32 color) {
    draw->AddCircleFilled({center.x - 20.0f * scale, center.y}, 18.0f * scale, color, 24);
    draw->AddCircleFilled({center.x + 2.0f * scale, center.y - 8.0f * scale}, 24.0f * scale, color, 24);
    draw->AddCircleFilled({center.x + 25.0f * scale, center.y}, 16.0f * scale, color, 24);
    draw->AddRectFilled(
        {center.x - 30.0f * scale, center.y},
        {center.x + 35.0f * scale, center.y + 18.0f * scale},
        color,
        10.0f * scale);
}

} // namespace

void AnimationRenderer::Render(AnimationMode mode, float timeSeconds, float width, float height) const {
    ImGui::BeginChild("AnimationPanel", ImVec2(width, height), true);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 min = ImGui::GetWindowPos();
    const ImVec2 max = {min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y};

    ImU32 bgTop = Color(0.55f, 0.78f, 0.96f);
    ImU32 bgBottom = Color(0.83f, 0.92f, 0.98f);

    if (mode == AnimationMode::Rain) {
        bgTop = Color(0.27f, 0.34f, 0.45f);
        bgBottom = Color(0.45f, 0.52f, 0.63f);
    } else if (mode == AnimationMode::Snow) {
        bgTop = Color(0.68f, 0.77f, 0.88f);
        bgBottom = Color(0.90f, 0.94f, 0.98f);
    } else if (mode == AnimationMode::Cloud) {
        bgTop = Color(0.58f, 0.68f, 0.78f);
        bgBottom = Color(0.79f, 0.85f, 0.91f);
    }

    draw->AddRectFilledMultiColor(min, max, bgTop, bgTop, bgBottom, bgBottom);

    const ImVec2 center = {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f};

    if (mode == AnimationMode::Sunny) {
        const float pulse = 1.0f + 0.07f * std::sin(timeSeconds * 2.5f);
        const ImVec2 sun = {center.x, center.y - 10.0f};
        draw->AddCircleFilled(sun, 42.0f * pulse, Color(1.0f, 0.84f, 0.36f), 48);

        for (int i = 0; i < 12; ++i) {
            const float angle = (timeSeconds * 0.8f) + (i * 3.1415926f / 6.0f);
            const ImVec2 a = {sun.x + std::cos(angle) * 52.0f, sun.y + std::sin(angle) * 52.0f};
            const ImVec2 b = {sun.x + std::cos(angle) * 70.0f, sun.y + std::sin(angle) * 70.0f};
            draw->AddLine(a, b, Color(1.0f, 0.90f, 0.52f), 3.0f);
        }
    }

    if (mode == AnimationMode::Cloud || mode == AnimationMode::Rain || mode == AnimationMode::Snow) {
        const float drift = std::sin(timeSeconds * 0.4f) * 18.0f;
        DrawCloud(draw, {center.x - 55.0f + drift, center.y - 20.0f}, 1.0f, Color(0.90f, 0.93f, 0.97f, 0.95f));
        DrawCloud(draw, {center.x + 30.0f - drift * 0.6f, center.y - 2.0f}, 0.9f, Color(0.86f, 0.90f, 0.95f, 0.95f));
    }

    if (mode == AnimationMode::Rain) {
        for (int i = 0; i < 90; ++i) {
            const float seed = static_cast<float>(i);
            const float x = min.x + std::fmod(seed * 13.0f + timeSeconds * 170.0f, (max.x - min.x));
            const float y = min.y + std::fmod(seed * 29.0f + timeSeconds * 250.0f, (max.y - min.y));
            draw->AddLine({x, y}, {x - 4.0f, y + 12.0f}, Color(0.70f, 0.82f, 0.98f, 0.9f), 2.0f);
        }
    }

    if (mode == AnimationMode::Snow) {
        for (int i = 0; i < 65; ++i) {
            const float seed = static_cast<float>(i);
            const float x = min.x + std::fmod(seed * 19.0f + std::sin(timeSeconds + seed) * 14.0f + timeSeconds * 20.0f, (max.x - min.x));
            const float y = min.y + std::fmod(seed * 23.0f + timeSeconds * 40.0f, (max.y - min.y));
            draw->AddCircleFilled({x, y}, 2.3f, Color(1.0f, 1.0f, 1.0f, 0.9f), 8);
        }
    }

    ImGui::EndChild();
}
