#include "WeatherService.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <nlohmann/json.hpp>

#include "DateUtils.h"
#include "WeatherCodeMap.h"

namespace {

std::string DoubleToString(double value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

const char* kDailyFields =
    "weather_code,temperature_2m_max,temperature_2m_min,"
    "apparent_temperature_max,apparent_temperature_min,"
    "precipitation_sum,wind_speed_10m_max";

} // namespace

WeatherService::WeatherService(const HttpClient& httpClient) : http(httpClient) {}

std::optional<WeatherQueryResult> WeatherService::FetchWeatherRange(
    const std::string& locationLabel,
    const std::string& city,
    const std::string& state,
    double latitude,
    double longitude,
    const std::string& startDate,
    const std::string& endDate,
    std::string& errorOut) const {
    WeatherQueryResult result;
    result.queryTimestamp = DateUtils::CurrentTimestampLocal();
    result.locationLabel = locationLabel;
    result.city = city;
    result.state = state;
    result.latitude = latitude;
    result.longitude = longitude;
    result.startDate = startDate;
    result.endDate = endDate;

    const std::string today = DateUtils::TodayLocal();

    // Split strategy:
    // 1) Entirely in the past => archive endpoint only.
    // 2) Entirely today/future => forecast endpoint only.
    // 3) Crossing today => archive for past slice + forecast for today/future slice.
    const bool allPast = DateUtils::CompareIsoDate(endDate, today) < 0;
    const bool allFutureOrToday = DateUtils::CompareIsoDate(startDate, today) >= 0;

    if (allPast) {
        if (!AppendFromEndpoint(
                "https://archive-api.open-meteo.com/v1/era5",
                latitude,
                longitude,
                startDate,
                endDate,
                "archive",
                result,
                errorOut,
                false)) {
            return std::nullopt;
        }
    } else if (allFutureOrToday) {
        if (!AppendFromEndpoint(
                "https://api.open-meteo.com/v1/forecast",
                latitude,
                longitude,
                startDate,
                endDate,
                "forecast",
                result,
                errorOut,
                true)) {
            return std::nullopt;
        }
    } else {
        // Archive up to yesterday.
        std::tm splitTm{};
        std::istringstream stream(today);
        stream >> std::get_time(&splitTm, "%Y-%m-%d");
        splitTm.tm_mday -= 1;
        splitTm.tm_isdst = -1;
        std::mktime(&splitTm);
        std::ostringstream yesterday;
        yesterday << std::put_time(&splitTm, "%Y-%m-%d");

        if (!AppendFromEndpoint(
                "https://archive-api.open-meteo.com/v1/era5",
                latitude,
                longitude,
                startDate,
                yesterday.str(),
                "archive",
                result,
                errorOut,
                false)) {
            return std::nullopt;
        }

        if (!AppendFromEndpoint(
                "https://api.open-meteo.com/v1/forecast",
                latitude,
                longitude,
                today,
                endDate,
                "forecast",
                result,
                errorOut,
                true)) {
            return std::nullopt;
        }
    }

    if (result.days.empty()) {
        errorOut = "No weather data returned for the selected range";
        return std::nullopt;
    }

    std::sort(result.days.begin(), result.days.end(), [](const WeatherDayRecord& a, const WeatherDayRecord& b) {
        return a.date < b.date;
    });

    return result;
}

bool WeatherService::AppendFromEndpoint(
    const std::string& endpoint,
    double latitude,
    double longitude,
    const std::string& startDate,
    const std::string& endDate,
    const std::string& source,
    WeatherQueryResult& result,
    std::string& errorOut,
    bool includeCurrent) const {
    if (DateUtils::CompareIsoDate(startDate, endDate) > 0) {
        return true;
    }

    std::string url = endpoint +
        "?latitude=" + DoubleToString(latitude) +
        "&longitude=" + DoubleToString(longitude) +
        "&start_date=" + startDate +
        "&end_date=" + endDate +
        "&daily=" + kDailyFields +
        "&timezone=auto";

    if (includeCurrent) {
        url += "&current=temperature_2m,apparent_temperature";
    }

    const HttpResponse response = http.Get(url);
    if (!response.ok) {
        errorOut = "Weather request failed: " + response.error;
        return false;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(response.body);
    } catch (const std::exception&) {
        errorOut = "Weather JSON parse failed";
        return false;
    }

    result.timezone = json.value("timezone", result.timezone);

    if (includeCurrent && json.contains("current") && json["current"].is_object()) {
        const auto& current = json["current"];
        if (current.contains("temperature_2m") && !current["temperature_2m"].is_null()) {
            result.currentTemperatureC = current["temperature_2m"].get<double>();
        }
        if (current.contains("apparent_temperature") && !current["apparent_temperature"].is_null()) {
            result.currentApparentTemperatureC = current["apparent_temperature"].get<double>();
        }
    }

    if (!json.contains("daily") || !json["daily"].is_object()) {
        errorOut = "Weather JSON missing daily data";
        return false;
    }

    const auto& daily = json["daily"];
    const auto& dates = daily.value("time", std::vector<std::string>{});
    const auto& tempMax = daily.value("temperature_2m_max", std::vector<double>{});
    const auto& tempMin = daily.value("temperature_2m_min", std::vector<double>{});
    const auto& precip = daily.value("precipitation_sum", std::vector<double>{});
    const auto& weatherCode = daily.value("weather_code", std::vector<int>{});

    std::vector<double> wind;
    if (daily.contains("wind_speed_10m_max") && daily["wind_speed_10m_max"].is_array()) {
        wind = daily["wind_speed_10m_max"].get<std::vector<double>>();
    }

    std::vector<double> apparentMax;
    if (daily.contains("apparent_temperature_max") && daily["apparent_temperature_max"].is_array()) {
        apparentMax = daily["apparent_temperature_max"].get<std::vector<double>>();
    }

    std::vector<double> apparentMin;
    if (daily.contains("apparent_temperature_min") && daily["apparent_temperature_min"].is_array()) {
        apparentMin = daily["apparent_temperature_min"].get<std::vector<double>>();
    }

    const size_t count = dates.size();
    if (count == 0 || tempMax.size() < count || tempMin.size() < count || precip.size() < count || weatherCode.size() < count) {
        errorOut = "Weather JSON daily arrays are incomplete";
        return false;
    }

    for (size_t i = 0; i < count; ++i) {
        WeatherDayRecord day;
        day.date = dates[i];
        day.temperatureMaxC = tempMax[i];
        day.temperatureMinC = tempMin[i];
        day.precipitationMm = precip[i];
        day.weatherCode = weatherCode[i];
        day.source = source;

        if (i < wind.size()) {
            day.windSpeedMaxKmh = wind[i];
        }
        if (i < apparentMax.size()) {
            day.apparentTemperatureMaxC = apparentMax[i];
        }
        if (i < apparentMin.size()) {
            day.apparentTemperatureMinC = apparentMin[i];
        }

        const WeatherCodeInfo info = GetWeatherCodeInfo(day.weatherCode);
        day.description = info.description;

        result.days.push_back(std::move(day));
    }

    return true;
}


