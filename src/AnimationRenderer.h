#pragma once

#include "WeatherModels.h"

class AnimationRenderer {
public:
    // Draws animated weather visuals directly in an ImGui child panel.
    void Render(AnimationMode mode, float timeSeconds, float width, float height) const;
};
