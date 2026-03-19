#include "App.h"

#include <chrono>
#include <cmath>
#include <sstream>

#include "DateUtils.h"
#include "WeatherCodeMap.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace {

bool ButtonChip(const char* label, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + 0.08f, color.y + 0.08f, color.z + 0.08f, color.w));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x - 0.05f, color.y - 0.05f, color.z - 0.05f, color.w));
    const bool pressed = ImGui::Button(label);
    ImGui::PopStyleColor(3);
    return pressed;
}

double CelsiusToFahrenheit(double celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
}

} // namespace

App::App()
    : geocoding(http),
      weather(http),
      historyService("weather_history.json") {
    cityInput = "Phoenix";
    stateInput = "Arizona";
    latitudeInput = "33.4484";
    longitudeInput = "-112.0740";
    startDateInput = DateUtils::TodayLocal();
    endDateInput = startDateInput;
    statusText = "Ready";

    std::string historyError;
    if (!historyService.Load(historyEntries, historyError)) {
        statusText = "History load warning: " + historyError;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 7.0f;
    style.ChildRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(10.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.97f, 0.99f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.99f, 1.00f, 1.00f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.40f, 0.63f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.44f, 0.68f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.51f, 0.79f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.29f, 0.57f, 0.86f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.46f, 0.72f, 1.0f);

    ApplyTextScale();
}

void App::RenderFrame() {
    PollBackgroundTask();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Open-Meteo Desktop Weather (C++)", nullptr, flags);

    RenderHeader();
    RenderQuickSelects();
    RenderInputCard();
    RenderStatus();

    ImGui::Separator();
    ImGui::Spacing();

    // Two-column lower section: latest results + history on the left, animation on the right.
    ImGui::Columns(2, "main_columns", false);
    RenderLatestResultCard();
    RenderHistoryCard();

    ImGui::NextColumn();
    RenderAnimationCard();

    ImGui::Columns(1);
    ImGui::End();
}

void App::RenderHeader() {
    ImGui::TextColored(ImVec4(0.10f, 0.25f, 0.42f, 1.0f), "C++ Weather Studio");
    ImGui::TextWrapped("Fetch Open-Meteo weather by city/state or coordinates, including date ranges and saved history.");
    RenderTextSizeControl();
    ImGui::Spacing();
}

void App::RenderTextSizeControl() {
    int selected = static_cast<int>(textSize);
    const char* labels[] = {"Small", "Medium", "Large", "Extra Large"};
    if (ImGui::Combo("Text Size", &selected, labels, IM_ARRAYSIZE(labels))) {
        textSize = static_cast<TextSize>(selected);
        switch (textSize) {
            case TextSize::Small:
                textScale = 0.95f;
                break;
            case TextSize::Medium:
                textScale = 1.05f;
                break;
            case TextSize::Large:
                textScale = 1.15f;
                break;
            case TextSize::ExtraLarge:
                textScale = 1.30f;
                break;
        }
        ApplyTextScale();
    }

    if (ImGui::SliderFloat("Text Scale", &textScale, 0.85f, 1.80f, "%.2fx")) {
        ApplyTextScale();
    }
}

void App::ApplyTextScale() {
    if (textScale < 0.85f) {
        textScale = 0.85f;
    }
    if (textScale > 1.80f) {
        textScale = 1.80f;
    }
    ImGui::GetIO().FontGlobalScale = textScale;
}

void App::RenderQuickSelects() {
    ImGui::BeginChild("QuickCities", ImVec2(0.0f, 74.0f), true);
    ImGui::TextUnformatted("Quick Select Cities");

    if (ButtonChip("Phoenix, AZ", ImVec4(0.95f, 0.62f, 0.22f, 1.0f))) {
        ApplyQuickCity("Phoenix", "Arizona", 33.4484, -112.0740);
    }
    ImGui::SameLine();
    if (ButtonChip("New York, NY", ImVec4(0.23f, 0.55f, 0.87f, 1.0f))) {
        ApplyQuickCity("New York", "New York", 40.7128, -74.0060);
    }
    ImGui::SameLine();
    if (ButtonChip("Los Angeles, CA", ImVec4(0.38f, 0.67f, 0.52f, 1.0f))) {
        ApplyQuickCity("Los Angeles", "California", 34.0522, -118.2437);
    }
    ImGui::SameLine();
    if (ButtonChip("Chicago, IL", ImVec4(0.63f, 0.45f, 0.79f, 1.0f))) {
        ApplyQuickCity("Chicago", "Illinois", 41.8781, -87.6298);
    }
    ImGui::SameLine();
    if (ButtonChip("Dallas, TX", ImVec4(0.88f, 0.36f, 0.35f, 1.0f))) {
        ApplyQuickCity("Dallas", "Texas", 32.7767, -96.7970);
    }
    ImGui::SameLine();
    if (ButtonChip("Seattle, WA", ImVec4(0.20f, 0.69f, 0.72f, 1.0f))) {
        ApplyQuickCity("Seattle", "Washington", 47.6062, -122.3321);
    }

    ImGui::EndChild();
    ImGui::Spacing();
}

void App::RenderInputCard() {
    ImGui::BeginChild("Inputs", ImVec2(0.0f, 230.0f), true);

    const char* modes[] = {"City / State", "Latitude / Longitude"};
    int modeIndex = locationMode == LocationMode::CityState ? 0 : 1;
    if (ImGui::Combo("Input Mode", &modeIndex, modes, IM_ARRAYSIZE(modes))) {
        locationMode = modeIndex == 0 ? LocationMode::CityState : LocationMode::Coordinates;
    }

    if (locationMode == LocationMode::CityState) {
        ImGui::InputText("City", &cityInput);
        ImGui::InputText("State", &stateInput);
    } else {
        ImGui::InputText("Latitude", &latitudeInput);
        ImGui::InputText("Longitude", &longitudeInput);
    }

    ImGui::Separator();
    ImGui::InputText("Start Date (YYYY-MM-DD)", &startDateInput);
    ImGui::InputText("End Date (YYYY-MM-DD)", &endDateInput);

    if (ImGui::Button("Use Today for Both")) {
        const std::string today = DateUtils::TodayLocal();
        startDateInput = today;
        endDateInput = today;
    }

    ImGui::EndChild();
}

void App::RenderStatus() {
    ImGui::TextColored(ImVec4(0.08f, 0.24f, 0.39f, 1.0f), "Status: %s", statusText.c_str());
    ImGui::SameLine(0.0f, 24.0f);

    if (ImGui::Button("Today's Date")) {
        startDateInput = DateUtils::TodayLocal();
    }

    ImGui::SameLine();
    if (ImGui::Button("Get Weather", ImVec2(140.0f, 0.0f)) && !isLoading) {
        StartFetch();
    }

    if (isLoading) {
        ImGui::SameLine();
        const float t = static_cast<float>(ImGui::GetTime());
        const char spinnerChars[] = {'|', '/', '-', '\\'};
        const int idx = static_cast<int>(t * 8.0f) % 4;
        ImGui::Text("Loading %c", spinnerChars[idx]);
    }
}

void App::RenderLatestResultCard() {
    ImGui::BeginChild("LatestResult", ImVec2(0.0f, 280.0f), true);
    ImGui::TextUnformatted("Latest Result");
    ImGui::Separator();

    if (!latestResult.has_value()) {
        ImGui::TextUnformatted("No weather fetched yet.");
        ImGui::EndChild();
        return;
    }

    const WeatherQueryResult& r = latestResult.value();
    ImGui::Text("Location: %s", r.locationLabel.c_str());
    ImGui::Text("Range: %s -> %s", r.startDate.c_str(), r.endDate.c_str());
    ImGui::Text("Coordinates: %.4f, %.4f", r.latitude, r.longitude);
    ImGui::Text("Timezone: %s", r.timezone.c_str());

    if (r.currentTemperatureC.has_value()) {
        const double c = r.currentTemperatureC.value();
        ImGui::Text("Current Temp: %.1f F (%.1f C)", CelsiusToFahrenheit(c), c);
    }
    if (r.currentApparentTemperatureC.has_value()) {
        const double c = r.currentApparentTemperatureC.value();
        ImGui::Text("Current Feels Like: %.1f F (%.1f C)", CelsiusToFahrenheit(c), c);
    }

    ImGui::Spacing();
    if (ImGui::BeginTable("days_table", 7, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0.0f, 150.0f))) {
        ImGui::TableSetupColumn("Date");
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn("High (F / C)");
        ImGui::TableSetupColumn("Low (F / C)");
        ImGui::TableSetupColumn("Precip mm");
        ImGui::TableSetupColumn("Wind km/h");
        ImGui::TableSetupColumn("Description");
        ImGui::TableHeadersRow();

        for (const WeatherDayRecord& day : r.days) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(day.date.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(day.source.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.1f (%.1f)", CelsiusToFahrenheit(day.temperatureMaxC), day.temperatureMaxC);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.1f (%.1f)", CelsiusToFahrenheit(day.temperatureMinC), day.temperatureMinC);
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%.1f", day.precipitationMm);
            ImGui::TableSetColumnIndex(5);
            if (day.windSpeedMaxKmh.has_value()) {
                ImGui::Text("%.1f", day.windSpeedMaxKmh.value());
            } else {
                ImGui::TextUnformatted("-");
            }
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%s (%d)", day.description.c_str(), day.weatherCode);
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();
}

void App::RenderHistoryCard() {
    ImGui::Spacing();
    ImGui::BeginChild("History", ImVec2(0.0f, 250.0f), true);
    ImGui::Text("History (%d)", static_cast<int>(historyEntries.size()));

    if (ImGui::Button("Export CSV")) {
        std::string error;
        if (historyService.ExportCsv(historyEntries, "weather_history_export.csv", error)) {
            statusText = "Export complete: weather_history_export.csv";
        } else {
            statusText = "CSV export error: " + error;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear Log")) {
        std::string error;
        if (historyService.Clear(historyEntries, error)) {
            statusText = "Log cleared";
        } else {
            statusText = "Clear log error: " + error;
        }
    }

    ImGui::Separator();
    ImGui::BeginChild("HistoryScroll", ImVec2(0.0f, 180.0f), false);

    for (int i = static_cast<int>(historyEntries.size()) - 1; i >= 0; --i) {
        const WeatherQueryResult& r = historyEntries[static_cast<size_t>(i)].result;
        const std::string label = r.queryTimestamp + " - " + r.locationLabel + " (" + r.startDate + " to " + r.endDate + ")";
        if (ImGui::TreeNode(label.c_str())) {
            ImGui::Text("Coordinates: %.4f, %.4f", r.latitude, r.longitude);
            ImGui::Text("Days returned: %d", static_cast<int>(r.days.size()));
            if (!r.days.empty()) {
                const WeatherDayRecord& d = r.days.front();
                ImGui::Text(
                    "First day: %s  %.1f F / %.1f F (%.1f C / %.1f C)  %s",
                    d.date.c_str(),
                    CelsiusToFahrenheit(d.temperatureMaxC),
                    CelsiusToFahrenheit(d.temperatureMinC),
                    d.temperatureMaxC,
                    d.temperatureMinC,
                    d.description.c_str());
            }
            ImGui::TreePop();
        }
    }

    ImGui::EndChild();
    ImGui::EndChild();
}

void App::RenderAnimationCard() {
    ImGui::BeginChild("AnimationCard", ImVec2(0.0f, 0.0f), true);
    ImGui::TextUnformatted("Weather Animation");

    int selected = static_cast<int>(animationOverride);
    const char* labels[] = {"Auto", "Sunny", "Rain", "Snow", "Cloud"};
    if (ImGui::Combo("Mode", &selected, labels, IM_ARRAYSIZE(labels))) {
        animationOverride = static_cast<AnimationMode>(selected);
    }

    AnimationMode effectiveMode = animationOverride;
    if (effectiveMode == AnimationMode::Auto && latestResult.has_value() && !latestResult->days.empty()) {
        effectiveMode = GetWeatherCodeInfo(latestResult->days.front().weatherCode).animationMode;
    }
    if (effectiveMode == AnimationMode::Auto) {
        effectiveMode = AnimationMode::Cloud;
    }

    ImGui::Text("Current visual mode: %s", AnimationModeToString(effectiveMode));
    animationRenderer.Render(effectiveMode, static_cast<float>(ImGui::GetTime()), ImGui::GetContentRegionAvail().x, 260.0f);

    ImGui::EndChild();
}

void App::ApplyQuickCity(const std::string& city, const std::string& state, double lat, double lon) {
    cityInput = city;
    stateInput = state;
    latitudeInput = std::to_string(lat);
    longitudeInput = std::to_string(lon);
    locationMode = LocationMode::CityState;
}

bool App::ValidateInputs(std::string& errorOut, std::optional<double>& latOut, std::optional<double>& lonOut) {
    if (!DateUtils::IsValidIsoDate(startDateInput)) {
        errorOut = "Error: invalid start date format (use YYYY-MM-DD)";
        return false;
    }
    if (!DateUtils::IsValidIsoDate(endDateInput)) {
        errorOut = "Error: invalid end date format (use YYYY-MM-DD)";
        return false;
    }
    if (DateUtils::CompareIsoDate(startDateInput, endDateInput) > 0) {
        errorOut = "Error: start date cannot be after end date";
        return false;
    }

    if (locationMode == LocationMode::CityState) {
        if (cityInput.empty() || stateInput.empty()) {
            errorOut = "Error: city and state are required";
            return false;
        }
        return true;
    }

    if (latitudeInput.empty() || longitudeInput.empty()) {
        errorOut = "Error: latitude and longitude are required";
        return false;
    }

    try {
        latOut = std::stod(latitudeInput);
        lonOut = std::stod(longitudeInput);
    } catch (const std::exception&) {
        errorOut = "Error: coordinates must be numeric";
        return false;
    }

    if (latOut.value() < -90.0 || latOut.value() > 90.0 || lonOut.value() < -180.0 || lonOut.value() > 180.0) {
        errorOut = "Error: coordinates are out of valid range";
        return false;
    }

    return true;
}

void App::StartFetch() {
    std::string validationError;
    std::optional<double> lat;
    std::optional<double> lon;
    if (!ValidateInputs(validationError, lat, lon)) {
        statusText = validationError;
        return;
    }

    isLoading = true;
    statusText = "Fetching weather...";

    const LocationMode mode = locationMode;
    const std::string city = cityInput;
    const std::string state = stateInput;
    const std::string latText = latitudeInput;
    const std::string lonText = longitudeInput;
    const std::string startDate = startDateInput;
    const std::string endDate = endDateInput;

    fetchFuture = std::async(std::launch::async, [this, mode, city, state, latText, lonText, startDate, endDate]() {
        FetchOutcome outcome;

        double latitude = 0.0;
        double longitude = 0.0;
        std::string locationLabel;
        std::string resolvedCity = city;
        std::string resolvedState = state;

        if (mode == LocationMode::CityState) {
            std::string geocodeError;
            const auto location = geocoding.ResolveCityState(city, state, geocodeError);
            if (!location.has_value()) {
                outcome.error = "Error: " + geocodeError;
                return outcome;
            }
            latitude = location->latitude;
            longitude = location->longitude;
            resolvedCity = location->name;
            resolvedState = location->state;
            locationLabel = resolvedCity + ", " + resolvedState;
        } else {
            try {
                latitude = std::stod(latText);
                longitude = std::stod(lonText);
            } catch (const std::exception&) {
                outcome.error = "Error: invalid coordinates";
                return outcome;
            }
            locationLabel = "Lat " + latText + ", Lon " + lonText;
        }

        std::string weatherError;
        auto weatherResult = weather.FetchWeatherRange(
            locationLabel,
            resolvedCity,
            resolvedState,
            latitude,
            longitude,
            startDate,
            endDate,
            weatherError);

        if (!weatherResult.has_value()) {
            outcome.error = "Error: " + weatherError;
            return outcome;
        }

        outcome.success = true;
        outcome.result = std::move(weatherResult.value());
        return outcome;
    });
}

void App::PollBackgroundTask() {
    if (!isLoading || !fetchFuture.valid()) {
        return;
    }

    using namespace std::chrono_literals;
    if (fetchFuture.wait_for(0ms) != std::future_status::ready) {
        return;
    }

    const FetchOutcome outcome = fetchFuture.get();
    isLoading = false;

    if (!outcome.success) {
        statusText = outcome.error;
        return;
    }

    latestResult = outcome.result;

    HistoryEntry entry;
    entry.result = outcome.result;

    std::string historyError;
    if (!historyService.Append(entry, historyEntries, historyError)) {
        statusText = "Weather loaded, but history save failed: " + historyError;
        return;
    }

    statusText = "Weather loaded";
}
