#pragma once

#include <string>

#include "WeatherModels.h"

// Maps Open-Meteo weather codes into UI-friendly text and animation categories.
struct WeatherCodeInfo {
    std::string description;
    AnimationMode animationMode = AnimationMode::Cloud;
};

WeatherCodeInfo GetWeatherCodeInfo(int weatherCode);

// Human-readable text for the animation dropdown.
const char* AnimationModeToString(AnimationMode mode);
