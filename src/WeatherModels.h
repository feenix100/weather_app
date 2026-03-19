#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

// Animation choices for the weather visualization panel.
enum class AnimationMode {
    Auto,
    Sunny,
    Rain,
    Snow,
    Cloud
};

// One daily weather data row returned from Open-Meteo.
struct WeatherDayRecord {
    std::string date;
    double temperatureMaxC = 0.0;
    double temperatureMinC = 0.0;
    std::optional<double> apparentTemperatureMaxC;
    std::optional<double> apparentTemperatureMinC;
    double precipitationMm = 0.0;
    std::optional<double> windSpeedMaxKmh;
    int weatherCode = 0;
    std::string description;
    std::string source;
};

// A full query result for a user-selected date range.
struct WeatherQueryResult {
    std::string queryTimestamp;
    std::string locationLabel;
    std::string city;
    std::string state;
    double latitude = 0.0;
    double longitude = 0.0;
    std::string timezone;
    std::string startDate;
    std::string endDate;
    std::optional<double> currentTemperatureC;
    std::optional<double> currentApparentTemperatureC;
    std::vector<WeatherDayRecord> days;
};

// Persistent history model. We keep a full query result snapshot per entry.
struct HistoryEntry {
    WeatherQueryResult result;
};

inline void to_json(nlohmann::json& j, const WeatherDayRecord& d) {
    j = nlohmann::json{
        {"date", d.date},
        {"temperature_max_c", d.temperatureMaxC},
        {"temperature_min_c", d.temperatureMinC},
        {"apparent_temperature_max_c", d.apparentTemperatureMaxC},
        {"apparent_temperature_min_c", d.apparentTemperatureMinC},
        {"precipitation_mm", d.precipitationMm},
        {"wind_speed_max_kmh", d.windSpeedMaxKmh},
        {"weather_code", d.weatherCode},
        {"description", d.description},
        {"source", d.source},
    };
}

inline void from_json(const nlohmann::json& j, WeatherDayRecord& d) {
    d.date = j.value("date", "");
    d.temperatureMaxC = j.value("temperature_max_c", 0.0);
    d.temperatureMinC = j.value("temperature_min_c", 0.0);
    if (j.contains("apparent_temperature_max_c") && !j["apparent_temperature_max_c"].is_null()) {
        d.apparentTemperatureMaxC = j["apparent_temperature_max_c"].get<double>();
    }
    if (j.contains("apparent_temperature_min_c") && !j["apparent_temperature_min_c"].is_null()) {
        d.apparentTemperatureMinC = j["apparent_temperature_min_c"].get<double>();
    }
    d.precipitationMm = j.value("precipitation_mm", 0.0);
    if (j.contains("wind_speed_max_kmh") && !j["wind_speed_max_kmh"].is_null()) {
        d.windSpeedMaxKmh = j["wind_speed_max_kmh"].get<double>();
    }
    d.weatherCode = j.value("weather_code", 0);
    d.description = j.value("description", "");
    d.source = j.value("source", "");
}

inline void to_json(nlohmann::json& j, const WeatherQueryResult& r) {
    j = nlohmann::json{
        {"query_timestamp", r.queryTimestamp},
        {"location_label", r.locationLabel},
        {"city", r.city},
        {"state", r.state},
        {"latitude", r.latitude},
        {"longitude", r.longitude},
        {"timezone", r.timezone},
        {"start_date", r.startDate},
        {"end_date", r.endDate},
        {"current_temperature_c", r.currentTemperatureC},
        {"current_apparent_temperature_c", r.currentApparentTemperatureC},
        {"days", r.days},
    };
}

inline void from_json(const nlohmann::json& j, WeatherQueryResult& r) {
    r.queryTimestamp = j.value("query_timestamp", "");
    r.locationLabel = j.value("location_label", "");
    r.city = j.value("city", "");
    r.state = j.value("state", "");
    r.latitude = j.value("latitude", 0.0);
    r.longitude = j.value("longitude", 0.0);
    r.timezone = j.value("timezone", "");
    r.startDate = j.value("start_date", "");
    r.endDate = j.value("end_date", "");
    if (j.contains("current_temperature_c") && !j["current_temperature_c"].is_null()) {
        r.currentTemperatureC = j["current_temperature_c"].get<double>();
    }
    if (j.contains("current_apparent_temperature_c") && !j["current_apparent_temperature_c"].is_null()) {
        r.currentApparentTemperatureC = j["current_apparent_temperature_c"].get<double>();
    }
    if (j.contains("days")) {
        r.days = j["days"].get<std::vector<WeatherDayRecord>>();
    }
}

inline void to_json(nlohmann::json& j, const HistoryEntry& h) {
    j = nlohmann::json{{"result", h.result}};
}

inline void from_json(const nlohmann::json& j, HistoryEntry& h) {
    h.result = j.at("result").get<WeatherQueryResult>();
}
