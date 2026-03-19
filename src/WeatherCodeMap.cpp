#include "WeatherCodeMap.h"

WeatherCodeInfo GetWeatherCodeInfo(int weatherCode) {
    switch (weatherCode) {
    case 0:
        return {"Clear sky", AnimationMode::Sunny};
    case 1:
    case 2:
        return {"Mainly clear", AnimationMode::Sunny};
    case 3:
        return {"Overcast", AnimationMode::Cloud};
    case 45:
    case 48:
        return {"Fog", AnimationMode::Cloud};
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
    case 61:
    case 63:
    case 65:
    case 66:
    case 67:
    case 80:
    case 81:
    case 82:
        return {"Rain", AnimationMode::Rain};
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return {"Snow", AnimationMode::Snow};
    case 95:
    case 96:
    case 99:
        return {"Thunderstorm", AnimationMode::Rain};
    default:
        return {"Unknown", AnimationMode::Cloud};
    }
}

const char* AnimationModeToString(AnimationMode mode) {
    switch (mode) {
    case AnimationMode::Auto:
        return "Auto";
    case AnimationMode::Sunny:
        return "Sunny";
    case AnimationMode::Rain:
        return "Rain";
    case AnimationMode::Snow:
        return "Snow";
    case AnimationMode::Cloud:
        return "Cloud";
    default:
        return "Unknown";
    }
}
