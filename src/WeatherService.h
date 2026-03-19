#pragma once

#include <optional>
#include <string>

#include "HttpClient.h"
#include "WeatherModels.h"

class WeatherService {
public:
    explicit WeatherService(const HttpClient& httpClient);

    // Fetches weather for [startDate, endDate] and fills a complete query result model.
    std::optional<WeatherQueryResult> FetchWeatherRange(
        const std::string& locationLabel,
        const std::string& city,
        const std::string& state,
        double latitude,
        double longitude,
        const std::string& startDate,
        const std::string& endDate,
        std::string& errorOut) const;

private:
    const HttpClient& http;

    bool AppendFromEndpoint(
        const std::string& endpoint,
        double latitude,
        double longitude,
        const std::string& startDate,
        const std::string& endDate,
        const std::string& source,
        WeatherQueryResult& result,
        std::string& errorOut,
        bool includeCurrent) const;
};
