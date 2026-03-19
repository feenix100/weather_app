#pragma once

#include <string>

namespace DateUtils {

// Returns local date in YYYY-MM-DD for startup defaults and Today button behavior.
std::string TodayLocal();

// Validates strict YYYY-MM-DD format and checks that the date actually exists.
bool IsValidIsoDate(const std::string& isoDate);

// Compares two valid ISO dates lexicographically in date order.
// Returns < 0 if a < b, 0 if equal, > 0 if a > b.
int CompareIsoDate(const std::string& a, const std::string& b);

// Returns an ISO timestamp for history entries.
std::string CurrentTimestampLocal();

} // namespace DateUtils
