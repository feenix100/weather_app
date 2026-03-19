#include "HttpClient.h"

#include <curl/curl.h>

#include <array>
#include <mutex>

namespace {

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    const size_t bytes = size * nmemb;
    auto* output = static_cast<std::string*>(userdata);
    output->append(ptr, bytes);
    return bytes;
}

std::once_flag g_curlInitFlag;

bool PerformRequest(CURL* curl, std::string& outError) {
    std::array<char, CURL_ERROR_SIZE> errorBuffer{};
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer.data());

    const CURLcode result = curl_easy_perform(curl);
    if (result == CURLE_OK) {
        return true;
    }

    if (errorBuffer[0] != '\0') {
        outError = errorBuffer.data();
    } else {
        outError = curl_easy_strerror(result);
    }
    return false;
}

} // namespace

HttpClient::HttpClient() {
    std::call_once(g_curlInitFlag, []() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    });
}

HttpClient::~HttpClient() = default;

HttpResponse HttpClient::Get(const std::string& url) const {
    HttpResponse response;

    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cppweatherapp/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

    if (!PerformRequest(curl, response.error)) {
        // Some Windows networks have broken IPv6 routes for these hosts.
        // Retry once forcing IPv4 before failing.
        if (response.error.find("Could not connect to server") != std::string::npos) {
            body.clear();
            response.error.clear();
            curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
            if (!PerformRequest(curl, response.error)) {
                curl_easy_cleanup(curl);
                return response;
            }
        } else {
            curl_easy_cleanup(curl);
            return response;
        }
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    response.statusCode = status;
    response.body = std::move(body);
    response.ok = status >= 200 && status < 300;
    if (!response.ok && response.error.empty()) {
        response.error = "HTTP status " + std::to_string(status);
    }

    return response;
}

std::string HttpClient::UrlEncode(const std::string& value) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return value;
    }

    char* encoded = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.size()));
    std::string out = encoded ? encoded : value;

    if (encoded) {
        curl_free(encoded);
    }
    curl_easy_cleanup(curl);
    return out;
}
