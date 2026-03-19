#include "DateUtils.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace {

bool ParseIsoDate(const std::string& text, std::tm& out) {
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') {
        return false;
    }

    std::istringstream stream(text);
    stream >> std::get_time(&out, "%Y-%m-%d");
    if (stream.fail()) {
        return false;
    }

    // mktime normalizes invalid dates (for example 2026-02-31),
    // so we re-format and compare to enforce strict validity.
    std::tm copy = out;
    copy.tm_isdst = -1;
    if (std::mktime(&copy) == -1) {
        return false;
    }

    std::ostringstream verify;
    verify << std::put_time(&copy, "%Y-%m-%d");
    return verify.str() == text;
}

} // namespace

namespace DateUtils {

std::string TodayLocal() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    local = *std::localtime(&now);
#endif

    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d");
    return out.str();
}

bool IsValidIsoDate(const std::string& isoDate) {
    std::tm parsed{};
    return ParseIsoDate(isoDate, parsed);
}

int CompareIsoDate(const std::string& a, const std::string& b) {
    std::tm tmA{};
    std::tm tmB{};
    if (!ParseIsoDate(a, tmA) || !ParseIsoDate(b, tmB)) {
        return a.compare(b);
    }

    std::tm copyA = tmA;
    std::tm copyB = tmB;
    copyA.tm_isdst = -1;
    copyB.tm_isdst = -1;
    const std::time_t tA = std::mktime(&copyA);
    const std::time_t tB = std::mktime(&copyB);

    if (tA < tB) {
        return -1;
    }
    if (tA > tB) {
        return 1;
    }
    return 0;
}

std::string CurrentTimestampLocal() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    local = *std::localtime(&now);
#endif

    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

} // namespace DateUtils
