#include "Request_parsing.hpp"

bool handle_400(const HttpRequest &request) {
    if (request.method.empty())
    {
        return true;
    }

    if (request.uri.empty())
    {
        return true;
    }
    if (request.http_version != "HTTP/1.1")
    {
        return true;
    }

    if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
    {
        return true;
    }

    return false;
}
