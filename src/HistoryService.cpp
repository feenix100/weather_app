#include "HistoryService.h"

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

namespace {

std::string CsvEscape(const std::string& value) {
    if (value.find_first_of(",\"\n") == std::string::npos) {
        return value;
    }

    std::string escaped = "\"";
    for (char c : value) {
        if (c == '\"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

std::string OptionalDoubleToCsv(const std::optional<double>& value) {
    if (!value.has_value()) {
        return "";
    }
    std::ostringstream out;
    out << value.value();
    return out.str();
}

} // namespace

HistoryService::HistoryService(std::string path) : filePath(std::move(path)) {}

bool HistoryService::Load(std::vector<HistoryEntry>& entries, std::string& errorOut) const {
    entries.clear();

    std::ifstream input(filePath);
    if (!input.is_open()) {
        return true;
    }

    nlohmann::json json;
    try {
        input >> json;
    } catch (const std::exception&) {
        errorOut = "Failed to parse history file";
        return false;
    }

    if (!json.is_array()) {
        errorOut = "History file has invalid format";
        return false;
    }

    try {
        entries = json.get<std::vector<HistoryEntry>>();
    } catch (const std::exception&) {
        errorOut = "History file contains invalid entries";
        return false;
    }

    return true;
}

bool HistoryService::Save(const std::vector<HistoryEntry>& entries, std::string& errorOut) const {
    std::ofstream output(filePath, std::ios::trunc);
    if (!output.is_open()) {
        errorOut = "Could not write history file";
        return false;
    }

    try {
        nlohmann::json json = entries;
        output << json.dump(2);
    } catch (const std::exception&) {
        errorOut = "Failed to serialize history data";
        return false;
    }

    return true;
}

bool HistoryService::Append(const HistoryEntry& entry, std::vector<HistoryEntry>& entriesInMemory, std::string& errorOut) const {
    entriesInMemory.push_back(entry);
    if (!Save(entriesInMemory, errorOut)) {
        entriesInMemory.pop_back();
        return false;
    }
    return true;
}

bool HistoryService::Clear(std::vector<HistoryEntry>& entriesInMemory, std::string& errorOut) const {
    entriesInMemory.clear();
    return Save(entriesInMemory, errorOut);
}

bool HistoryService::ExportCsv(const std::vector<HistoryEntry>& entries, const std::string& csvPath, std::string& errorOut) const {
    std::ofstream output(csvPath, std::ios::trunc);
    if (!output.is_open()) {
        errorOut = "Could not open CSV file for writing";
        return false;
    }

    output << "timestamp,city,state,start_date,end_date,day_date,latitude,longitude,timezone,source,current_temperature_c,current_apparent_temperature_c,temperature_max_c,temperature_min_c,apparent_temperature_max_c,apparent_temperature_min_c,precipitation_mm,wind_speed_max_kmh,weather_code,description\n";

    for (const HistoryEntry& entry : entries) {
        const WeatherQueryResult& r = entry.result;
        for (const WeatherDayRecord& d : r.days) {
            output
                << CsvEscape(r.queryTimestamp) << ","
                << CsvEscape(r.city) << ","
                << CsvEscape(r.state) << ","
                << CsvEscape(r.startDate) << ","
                << CsvEscape(r.endDate) << ","
                << CsvEscape(d.date) << ","
                << r.latitude << ","
                << r.longitude << ","
                << CsvEscape(r.timezone) << ","
                << CsvEscape(d.source) << ","
                << OptionalDoubleToCsv(r.currentTemperatureC) << ","
                << OptionalDoubleToCsv(r.currentApparentTemperatureC) << ","
                << d.temperatureMaxC << ","
                << d.temperatureMinC << ","
                << OptionalDoubleToCsv(d.apparentTemperatureMaxC) << ","
                << OptionalDoubleToCsv(d.apparentTemperatureMinC) << ","
                << d.precipitationMm << ","
                << OptionalDoubleToCsv(d.windSpeedMaxKmh) << ","
                << d.weatherCode << ","
                << CsvEscape(d.description)
                << "\n";
        }
    }

    return true;
}
