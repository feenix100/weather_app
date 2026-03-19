#pragma once

#include <optional>
#include <string>

#include "HttpClient.h"

struct GeocodeLocation {
    std::string name;
    std::string state;
    std::string countryCode;
    double latitude = 0.0;
    double longitude = 0.0;
};

class GeocodingService {
public:
    explicit GeocodingService(const HttpClient& httpClient);

    // Resolves US city/state into coordinates.
    std::optional<GeocodeLocation> ResolveCityState(
        const std::string& city,
        const std::string& state,
        std::string& errorOut) const;

private:
    const HttpClient& http;
};
