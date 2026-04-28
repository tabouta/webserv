#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include "../httpRequest/HttpRequest.hpp"
# include "../../inc/Webserv.hpp"


struct Locations;
struct Config_Data;   

class HttpResponse
{
public:
    std::string http_version;
    int status_code;
    std::string reason_phrase;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse();
    std::string to_string() const;
};

HttpResponse generate400Response();
const Locations* findBestLocation(const std::string& uri, const std::map<std::string, Locations>& locations);
std::string create_path(const HttpRequest& request, const Locations* location, const Config_Data& config);
bool fileExists(const std::string& path);
HttpResponse build404Response();
std::string buildSimpleResponse(int code, const std::string& message);
HttpResponse handle_404_405(const HttpRequest& req, const Config_Data& config);
bool is_method_allowed(const std::string& method, const Locations* location);
HttpResponse build405Response();
std::string getReasonPhrase(int code);
std::string build405Response(const Locations* loc);
bool deal_413(const HttpRequest& request, const Config_Data& config, const Locations* location);
HttpResponse generate501Response();
bool handle_501(const HttpRequest &req);
std::string toString(int nombre);
std::string buildErrorResponse(int status_code);
std::string buildRedirectResponse(int status_code, const std::string& target);
int buildResponse(const HttpRequest& req, int fd, const Config_Data& config, Connexion *temp);
std::string build_Response_200(const std::string& filepath);
std::string readFileContent(const std::string& path);
bool is_path_folder(const std::string& path);
bool isCgiRequest(const HttpRequest &req, const Locations *loc);
#endif
