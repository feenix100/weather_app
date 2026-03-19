#pragma once

#include <string>

struct HttpResponse {
    bool ok = false;
    long statusCode = 0;
    std::string body;
    std::string error;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpResponse Get(const std::string& url) const;
    static std::string UrlEncode(const std::string& value);
};
