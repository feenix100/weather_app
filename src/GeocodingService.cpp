#include "GeocodingService.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string_view>

#include <nlohmann/json.hpp>

namespace {

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string Trim(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

std::string TitleCaseWords(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    bool newWord = true;
    for (char ch : input) {
        const unsigned char uc = static_cast<unsigned char>(ch);
        if (std::isspace(uc)) {
            out.push_back(ch);
            newWord = true;
        } else {
            out.push_back(newWord ? static_cast<char>(std::toupper(uc)) : static_cast<char>(std::tolower(uc)));
            newWord = false;
        }
    }
    return out;
}

std::string ResolveStateName(const std::string& raw) {
    static const std::array<std::pair<std::string_view, std::string_view>, 51> kStateMap = {{
        {"AL", "Alabama"}, {"AK", "Alaska"}, {"AZ", "Arizona"}, {"AR", "Arkansas"},
        {"CA", "California"}, {"CO", "Colorado"}, {"CT", "Connecticut"}, {"DE", "Delaware"},
        {"FL", "Florida"}, {"GA", "Georgia"}, {"HI", "Hawaii"}, {"ID", "Idaho"},
        {"IL", "Illinois"}, {"IN", "Indiana"}, {"IA", "Iowa"}, {"KS", "Kansas"},
        {"KY", "Kentucky"}, {"LA", "Louisiana"}, {"ME", "Maine"}, {"MD", "Maryland"},
        {"MA", "Massachusetts"}, {"MI", "Michigan"}, {"MN", "Minnesota"}, {"MS", "Mississippi"},
        {"MO", "Missouri"}, {"MT", "Montana"}, {"NE", "Nebraska"}, {"NV", "Nevada"},
        {"NH", "New Hampshire"}, {"NJ", "New Jersey"}, {"NM", "New Mexico"}, {"NY", "New York"},
        {"NC", "North Carolina"}, {"ND", "North Dakota"}, {"OH", "Ohio"}, {"OK", "Oklahoma"},
        {"OR", "Oregon"}, {"PA", "Pennsylvania"}, {"RI", "Rhode Island"}, {"SC", "South Carolina"},
        {"SD", "South Dakota"}, {"TN", "Tennessee"}, {"TX", "Texas"}, {"UT", "Utah"},
        {"VT", "Vermont"}, {"VA", "Virginia"}, {"WA", "Washington"}, {"WV", "West Virginia"},
        {"WI", "Wisconsin"}, {"WY", "Wyoming"}, {"DC", "District of Columbia"},
    }};

    const std::string trimmed = Trim(raw);
    if (trimmed.size() == 2) {
        const std::string upper = {static_cast<char>(std::toupper(static_cast<unsigned char>(trimmed[0]))),
                                   static_cast<char>(std::toupper(static_cast<unsigned char>(trimmed[1])))};
        for (const auto& entry : kStateMap) {
            if (entry.first == upper) {
                return std::string(entry.second);
            }
        }
    }

    return TitleCaseWords(trimmed);
}

bool StateMatches(const std::string& admin1, const std::string& desiredState) {
    if (desiredState.empty()) {
        return true;
    }
    const std::string adminLower = ToLower(admin1);
    const std::string stateLower = ToLower(desiredState);
    return adminLower == stateLower || adminLower.find(stateLower) != std::string::npos;
}

} // namespace

GeocodingService::GeocodingService(const HttpClient& httpClient) : http(httpClient) {}

std::optional<GeocodeLocation> GeocodingService::ResolveCityState(
    const std::string& city,
    const std::string& state,
    std::string& errorOut) const {
    const std::string cityQuery = Trim(city);
    const std::string stateName = ResolveStateName(state);
    const std::string url =
        "https://geocoding-api.open-meteo.com/v1/search?name=" + HttpClient::UrlEncode(cityQuery) +
        "&count=10&language=en&format=json&country=US";

    const HttpResponse response = http.Get(url);
    if (!response.ok) {
        errorOut = "Geocoding request failed: " + response.error;
        return std::nullopt;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(response.body);
    } catch (const std::exception&) {
        errorOut = "Geocoding JSON parse failed";
        return std::nullopt;
    }

    if (!json.contains("results") || !json["results"].is_array() || json["results"].empty()) {
        errorOut = "City not found";
        return std::nullopt;
    }

    for (const auto& item : json["results"]) {
        if (item.value("country_code", "") != "US") {
            continue;
        }

        const std::string admin1 = item.value("admin1", "");
        if (!StateMatches(admin1, stateName)) {
            continue;
        }

        GeocodeLocation match;
        match.name = item.value("name", "");
        match.state = admin1;
        match.countryCode = "US";
        match.latitude = item.value("latitude", 0.0);
        match.longitude = item.value("longitude", 0.0);
        return match;
    }

    for (const auto& item : json["results"]) {
        if (item.value("country_code", "") != "US") {
            continue;
        }

        GeocodeLocation fallback;
        fallback.name = item.value("name", "");
        fallback.state = item.value("admin1", "");
        fallback.countryCode = "US";
        fallback.latitude = item.value("latitude", 0.0);
        fallback.longitude = item.value("longitude", 0.0);
        return fallback;
    }

    errorOut = "No matching US city/state result";
    return std::nullopt;
}
