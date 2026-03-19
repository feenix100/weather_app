#pragma once

#include <string>
#include <vector>

#include "WeatherModels.h"

class HistoryService {
public:
    explicit HistoryService(std::string path);

    bool Load(std::vector<HistoryEntry>& entries, std::string& errorOut) const;
    bool Save(const std::vector<HistoryEntry>& entries, std::string& errorOut) const;
    bool Append(const HistoryEntry& entry, std::vector<HistoryEntry>& entriesInMemory, std::string& errorOut) const;
    bool Clear(std::vector<HistoryEntry>& entriesInMemory, std::string& errorOut) const;
    bool ExportCsv(const std::vector<HistoryEntry>& entries, const std::string& csvPath, std::string& errorOut) const;

private:
    std::string filePath;
};
