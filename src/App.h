#pragma once

#include <future>
#include <optional>
#include <string>
#include <vector>

#include "AnimationRenderer.h"
#include "GeocodingService.h"
#include "HistoryService.h"
#include "HttpClient.h"
#include "WeatherModels.h"
#include "WeatherService.h"

class App {
public:
    App();

    // Called once per frame from the render loop.
    void RenderFrame();

private:
    enum class LocationMode {
        CityState = 0,
        Coordinates = 1,
    };

    struct FetchOutcome {
        bool success = false;
        WeatherQueryResult result;
        std::string error;
    };
    enum class TextSize {
        Small = 0,
        Medium = 1,
        Large = 2,
        ExtraLarge = 3,
    };

    void RenderHeader();
    void RenderQuickSelects();
    void RenderInputCard();
    void RenderStatus();
    void RenderLatestResultCard();
    void RenderHistoryCard();
    void RenderAnimationCard();
    void RenderTextSizeControl();
    void ApplyTextScale();

    void ApplyQuickCity(const std::string& city, const std::string& state, double lat, double lon);
    void StartFetch();
    void PollBackgroundTask();
    bool ValidateInputs(std::string& errorOut, std::optional<double>& latOut, std::optional<double>& lonOut);

    HttpClient http;
    GeocodingService geocoding;
    WeatherService weather;
    HistoryService historyService;
    AnimationRenderer animationRenderer;

    LocationMode locationMode = LocationMode::CityState;
    std::string cityInput;
    std::string stateInput;
    std::string latitudeInput;
    std::string longitudeInput;
    std::string startDateInput;
    std::string endDateInput;

    AnimationMode animationOverride = AnimationMode::Auto;
    TextSize textSize = TextSize::Large;
    float textScale = 1.15f;
    std::string statusText;
    bool isLoading = false;

    std::optional<WeatherQueryResult> latestResult;
    std::vector<HistoryEntry> historyEntries;

    std::future<FetchOutcome> fetchFuture;
};
