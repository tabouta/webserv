
#include "HttpResponse.hpp"
#include "../httpRequest/Request_parsing.hpp"
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "../cgi/CgiExecutor.hpp"
#include <fstream>
#include <sstream>
# include "../../inc/Webserv.hpp"

const Locations* findBestLocation(const std::string& uri, const std::map<std::string, Locations>& locations) {
    const Locations* bestLocation = NULL;
    std::size_t bestMatchLength = 0;

    for (std::map<std::string, Locations>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const std::string& path = it->first;

        if (uri.compare(0, path.size(), path) == 0) {
            if (path.size() > bestMatchLength) {
                bestMatchLength = path.size();
                bestLocation = &it->second;
            }
        }
    }

    return bestLocation;
}


HttpResponse::HttpResponse() // constructeur par defautl
{
    http_version = "HTTP/1.1";
    status_code = 200;
    reason_phrase = "OK";
    headers["Content-Type"] = "text/plain";
    body = "Hello, world!";
    std::ostringstream oss;
    oss << body.length();
    headers["Content-Length"] = oss.str();
}


HttpResponse generate400Response()
{
    std::cout << ">>> generate400Response CALLED" << std::endl; // debug ici
    HttpResponse response;

    response.http_version = "HTTP/1.1";
    response.status_code = 400;
    response.reason_phrase = "Bad Request";

    response.body = "400 Bad Request";
    response.headers["Content-Type"] = "text/plain";

    std::ostringstream oss;
    oss << response.body.size();
    response.headers["Content-Length"] = oss.str();

    return response;
}

std::string HttpResponse::to_string() const
{
    std::ostringstream response;

    response << http_version << " " << status_code << " " << reason_phrase << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n";
    response << body;

    return response.str();
}

std::string create_path(const HttpRequest& request, const Locations* location, const Config_Data& config)
{
    std::string path;
    const std::string& uri = request.uri;
    std::string uri_relative;

    if (location != NULL && uri.size() >= location->path.size())
    {
        uri_relative = uri.substr(location->path.size());
    }

    if (location != NULL && !location->alias.empty())
    {
        path = location->alias + "/" + uri_relative;
    }
    else if (location != NULL && !location->root.empty())
    {
        path = location->root + "/" + uri_relative;
    }
    else {
        path = config.root + "/" + uri;
    }

    std::string clean_path;
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '/' && i > 0 && path[i - 1] == '/') {
            continue;
        }
        clean_path = clean_path + path[i];
    }
    return clean_path;
}

bool fileExists(const std::string& path)
{
    return access(path.c_str(), F_OK) == 0;
}


HttpResponse build404Response(){
    HttpResponse response;

    response.http_version = "HTTP/1.1";
    response.status_code = 404;
    response.reason_phrase = "Not Found";
    response.headers["Content-Type"] = "text/html";

    std::ifstream file("var/www/html/errors/404.html");

    if (file.is_open()) {
        std::ostringstream buffer;
        buffer << file.rdbuf();
        response.body = buffer.str();
        file.close();
    } else {
        response.body = "<html><body><h1>404 Not Found</h1></body></html>";
    }

    std::ostringstream oss;
    oss << response.body.size();
    response.headers["Content-Length"] = oss.str();

    return response;
}

bool is_method_allowed(const std::string& method, const Locations* location)
{
    if (location == NULL) {
        return false;
    }

    for (std::size_t i = 0; i < location->methods.size(); ++i) {
        if (location->methods[i] == method) {
            return true;
        }
    }

    return false;
}

HttpResponse build405Response()
{
    HttpResponse response;

    response.http_version = "HTTP/1.1";
    response.status_code = 405;
    response.reason_phrase = "Method Not Allowed";
    response.headers["Content-Type"] = "text/html";

    response.body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";

    std::ostringstream oss;
    oss << response.body.size();
    response.headers["Content-Length"] = oss.str();

    return response;
}

HttpResponse handle_404_405(const HttpRequest& req, const Config_Data& config)
{
    const Locations* location = findBestLocation(req.uri, config.locations);

    if (!is_method_allowed(req.method, location)) {
        return build405Response();
    }

    std::string path = create_path(req, location, config);

    if (!fileExists(path)) {
        return build404Response();
    }

    HttpResponse response;
    response.status_code = 200;
    response.reason_phrase = "OK";
    response.http_version = "HTTP/1.1";
    response.body = "File exists (pas encore servi)";
    response.headers["Content-Type"] = "text/plain";

    std::ostringstream oss;
    oss << response.body.size();
    response.headers["Content-Length"] = oss.str();

    return response;
}


bool deal_413(const HttpRequest& request, const Config_Data& config, const Locations* location)
{
    int max_allowed = config.max_body_size;

    if (location != NULL && location->body_max_size >= 0)
    {
        max_allowed = location->body_max_size;
    }

    if (max_allowed < 0)
        return false;

    // std::cout << "valeur de content-lenght :" << request.content_length << std::endl;
    if (request.content_length > static_cast<size_t>(max_allowed))
        return true;

    return false;
}


bool handle_501(const HttpRequest &req)
{
    std::string method = req.method;

    return method == "HEAD" ||
           method == "OPTIONS" ||
           method == "PUT" ||
           method == "PATCH" ||
           method == "CONNECT" ||
           method == "TRACE";
}



HttpResponse generate501Response()
{
    HttpResponse response;

    response.http_version = "HTTP/1.1";
    response.status_code = 501;
    response.reason_phrase = "Not Implemented";

    response.body = "501 Not Implemented";
    response.headers["Content-Type"] = "text/plain";

    std::ostringstream oss;
    oss << response.body.size();
    response.headers["Content-Length"] = oss.str();

    return response;
}

std::string getReasonPhrase(int code)
{
    if (code == 200) return "OK";
    if (code == 201) return "Created";
    if (code == 204) return "No Content";
    if (code == 301) return "Moved Permanently";
    if (code == 302) return "Found";
    if (code == 400) return "Bad Request";
    if (code == 403) return "Forbidden";
    if (code == 404) return "Not Found";
    if (code == 405) return "Method Not Allowed";
    if (code == 408) return "Request Timeout"; // a gerer avec Arthur
    if (code == 413) return "Payload Too Large";
    if (code == 415) return "Unsupported Media Type";
    if (code == 500) return "Internal Server Error";
    if (code == 501) return "Not Implemented";
    if (code == 502) return "Bad Gateway";
    if (code == 504) return "Gateway Timeout";

    return "Unknown";
}

std::string toString(int nombre)
{
    std::ostringstream flux;
    flux << nombre;
    return flux.str();
}

std::string display404Page()
{
    std::string path = "./var/www/html/errors/404.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>404 Not Found</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 404 Not Found\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string display400Page()
{
    std::string path = "./var/www/html/errors/400.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>404 Bad Request</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 400 Bad Request\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string display403Page()
{
    std::string path = "./var/www/html/errors/403.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>403 Forbidden</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 403 Forbidden\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}


std::string display405Page()
{
    std::string path = "./var/www/html/errors/405.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 405 Method Not Allowed\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string display408Page()
{
    std::string path = "./var/www/html/errors/408.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>408 Request Timeout</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 408 Request Timeout\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}


std::string display413Page()
{
    std::string path = "./var/www/html/errors/413.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>413 Payload Too Large</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 413 Payload Too Large\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string display415Page()
{
    std::string path = "./var/www/html/errors/415.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>415 Unsupported Media Type</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 415 Unsupported Media Type\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}


std::string display500Page()
{
    std::string path = "./var/www/html/errors/500.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 500 Internal Server Error\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string display501Page()
{
    std::string path = "./var/www/html/errors/501.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>501 Not Implemented</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 501 Not Implemented\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string display502Page()
{
    std::string path = "./var/www/html/errors/502.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>502 Bad Gateway</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 502 Bad Gateway\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string display504Page()
{
    std::string path = "./var/www/html/errors/504.html";
    std::ifstream file(path.c_str());
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
        file.close();
    } else {
        body = "<html><body><h1>504 Gateway Timeout</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 504 Gateway Timeout\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string buildErrorResponse(int status_code)
{
    if (status_code == 404)
    {
        return (display404Page());
    }
    else if (status_code == 400)
    {
        return (display400Page());
    }
    else if (status_code == 403)
    {
        return (display403Page());
    }
    else if (status_code == 413)
    {
        return (display413Page());
    }
    else if (status_code == 408)
    {
        return (display408Page());
    }
    else if (status_code == 415)
    {
        return (display415Page());
    }
    else if (status_code == 500)
    {
        return (display500Page());
    }
    else if (status_code == 501)
    {
        return (display501Page());
    }
    else if (status_code == 502)
    {
        return (display502Page());
    }
    else if (status_code == 504)
    {
        return (display504Page());
    }
    else if (status_code == 405)
    {
        return (display405Page());
    }

    std::string status_code_str = toString(status_code);
    std::string reason_phrase = getReasonPhrase(status_code);
    std::string status_line = "HTTP/1.1 " + status_code_str + " " + reason_phrase + "\r\n";

    // ici je devrais faire une fonction qui recherche si 
    // y'a un 
    std::string body_start = "<html><head><title>";
    std::string body_middle = "</title></head><body><h1>";
    std::string body_end = "</h1></body></html>";
    std::string body = body_start + status_code_str + " " + reason_phrase + body_middle
                     + status_code_str + " " + reason_phrase + body_end;

    std::string header_1 = "Content-Type: text/html\r\n";

    std::string content_length_str = toString(body.length());
    std::string header_2 = "Content-Length: " + content_length_str + "\r\n";

    std::string header_3 = "Connection: close\r\n";

    std::string empty_line = "\r\n";

    std::string full_response = status_line
                               + header_1
                               + header_2
                               + header_3
                               + empty_line
                               + body;

    return full_response;
}



std::string readFileContent(const std::string& path) {
    std::ifstream file;
    file.open(path.c_str(), std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier " << path << std::endl;
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    file.close();
    return buffer.str();
}


std::string buildAutoIndex(const std::string& path, const std::string& uri)
{
    std::string html = "<html>\n";
    html = html + "<head><title>Index of " + uri + "</title></head>\n";
    html = html + "<body>\n";
    html = html + "<h1>Index of " + uri + "</h1>\n";
    html = html + "<ul>\n";

    if (uri != "/")
    {
        html = html + "<li><a href=\"../\">../</a></li>\n";
    }

    DIR* dir = opendir(path.c_str());
    if (!dir)
    {
        return "<html><body><h1>500 Internal Server Error</h1></body></html>";
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        if (name == ".")
            continue;

        std::string link = uri;
        if (!uri.empty() && uri[uri.size() - 1] != '/')
        {
            link = link + "/";
        }
        link = link + name;

        html = html + "<li><a href=\"" + link + "\">" + name + "</a></li>\n";
    }

    closedir(dir);

    html = html + "</ul>\n";
    html = html + "</body>\n</html>\n";

    return html;
}

std::string buildSimpleResponse(int code, const std::string& message)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " Created\r\n"
        << "Content-Type: text/plain\r\n"
        << "Content-Length: " << message.size() << "\r\n"
        << "\r\n"
        << message;
    return oss.str();
}

std::string build_Response_200(const std::string& filepath) {
    std::string body = readFileContent(filepath);

    std::string content_type = getContentTypeFromExtension(filepath);

    std::string status_line = "HTTP/1.1 200 OK\r\n";

    std::string header_1 = "Content-Type: " + content_type + "\r\n";

    std::string content_length_str = toString(body.length());
    std::string header_2 = "Content-Length: " + content_length_str + "\r\n";

    std::string header_3 = "Connection: close\r\n";

    std::string empty_line = "\r\n";

    std::string response = status_line
                         + header_1
                         + header_2
                         + header_3
                         + empty_line
                         + body;

    return response;
}


bool is_path_folder(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    return S_ISDIR(st.st_mode);
}

std::string buildRedirectResponse(int status_code, const std::string& target) {
    std::string code_str = toString(status_code);
    std::string reason_phrase = getReasonPhrase(status_code);

    std::string status_line = "HTTP/1.1 " + code_str + " " + reason_phrase + "\r\n";

    std::string header_1 = "Location: " + target + "\r\n";
    std::string header_2 = "Content-Type: text/html\r\n";

    std::string body = "<html><head><title>" + code_str + " " + reason_phrase + "</title></head>";
    body = body + "<body><h1>" + code_str + " " + reason_phrase + "</h1>";
    body = body + "<p><a href=\"" + target + "\">" + target + "</a></p></body></html>";

    std::string content_length_str = toString(body.length());
    std::string header_3 = "Content-Length: " + content_length_str + "\r\n";

    std::string header_4 = "Connection: close\r\n";

    std::string empty_line = "\r\n";

    std::string response = status_line
                         + header_1
                         + header_2
                         + header_3
                         + header_4
                         + empty_line
                         + body;

    return response;
}


bool isCgiRequest(const HttpRequest &req, const Locations *loc)
{
	if (loc == NULL) {
		return false;
	}

	if (loc->cgi_extension.empty()) {
		return false;
	}

	if (loc->cgi_path.empty()) {
		return false;
	}

	const std::string &uri = req.uri;
	const std::string &ext = loc->cgi_extension;

	if (uri.length() < ext.length()) {
		return false;
	}

	if (uri.compare(uri.length() - ext.length(), ext.length(), ext) != 0) {
		return false;
	}

	return true;
}


std::string build405Response(const Locations* loc)
{
    std::string status_code_str = "405";
    std::string reason_phrase = "Method Not Allowed";
    std::string status_line = "HTTP/1.1 " + status_code_str + " " + reason_phrase + "\r\n";

    std::string body_start = "<html><head><title>";
    std::string body_middle = "</title></head><body><h1>";
    std::string body_end = "</h1></body></html>";
    std::string body = body_start + status_code_str + " " + reason_phrase + body_middle
                     + status_code_str + " " + reason_phrase + body_end;

    std::string header_1 = "Content-Type: text/html\r\n";
    std::string header_3 = "Connection: close\r\n";

    std::string header_allow = "Allow: ";
    for (std::size_t i = 0; i < loc->methods.size(); ++i)
    {
        header_allow = header_allow + loc->methods[i];
        if (i < loc->methods.size() - 1)
            header_allow = header_allow + ", ";
    }
    header_allow = header_allow + "\r\n";

    std::string content_length_str = toString(body.length());
    std::string header_2 = "Content-Length: " + content_length_str + "\r\n";

    std::string empty_line = "\r\n";

    std::string full_response = status_line
                               + header_1
                               + header_2
                               + header_3
                               + header_allow
                               + empty_line
                               + body;

    return full_response;
}

std::string url_encodage(const std::string& string)
{
    std::string new_string = "";
    std::size_t i = 0;

    while (i < string.length())
    {
        if (i + 2 < string.length() &&
            string[i] == '%' &&
            string[i + 1] == '2' &&
            string[i + 2] == '0')
            {

                new_string = new_string + ' ';
                i = i + 3; // on saute les 3 caractères "%20"
            } 
            else
            {
                new_string = new_string + string[i];
                i = i + 1;
            }
    }

    return new_string;
}
int ProtoclDelete(const HttpRequest& req, const Locations& loc, const Config_Data& config)
{
    std::string path = create_path(req, &loc, config);
    path = url_encodage(path);
    // std::cout << "DELETE path: " << path << std::endl;

    struct stat s;
    if (stat(path.c_str(), &s) != 0)
    {
        return 404; 
    }

    if (S_ISDIR(s.st_mode)) {
        return 403; 
    }

    if (std::remove(path.c_str()) != 0)
    {
        std::perror("remove"); 
        return 500; 
    }

    return 200;
}


int hexCharToInt(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        return 10 + (c - 'A');
    }
    else if (c >= 'a' && c <= 'f')
    {
        return 10 + (c - 'a');
    }
    else
    {
        return -1; // caractère hex invalide
    }
}

char decodeHexPair(char highChar, char lowChar)
{
    int high = hexCharToInt(highChar);
    int low = hexCharToInt(lowChar);

    if (high == -1 || low == -1)
    {
        return '?';
    }

    int value = (high * 16) + low;
    char decoded = static_cast<char>(value);
    return decoded;
}

std::string decodePercentEncoding(const std::string &input)
{
    std::string result;
    std::size_t i = 0;

    while (i < input.size())
    {
        if (input[i] == '%' && (i + 2) < input.size())
        {
            char highChar = input[i + 1];
            char lowChar = input[i + 2];

            char decoded = decodeHexPair(highChar, lowChar);
            result.push_back(decoded);

            i = i + 3; 
        }
        else if (input[i] == '+')
        {
            result.push_back(' ');
            i = i + 1;
        }
        else
        {
            result.push_back(input[i]);
            i = i + 1;
        }
    }

    return result;
}


std::string generateCustomUploadListing(const std::string& folder_path, const std::string& uri_prefix)
{
    std::string html =
        "<!DOCTYPE html>\n"
        "<html lang=\"fr\">\n"
        "<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <title>UploadStore</title>\n"
        "  <link rel=\"stylesheet\" href=\"/delete.css\">\n"
        "</head>\n"
        "<body>\n"
        "  <div class=\"section1\">\n"
        "    <h2 class=\"title\">Delete</h2>\n"
        "    <div class=\"background\">\n"
        "      <p class=\"text\">Voici la liste des fichiers stockés sur le serveur. Vous pouvez les consulter ou les supprimer.</p>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div id=\"file-list\">\n";

    DIR *dir = opendir(folder_path.c_str());
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;

            if (name == "." || name == "..")
                continue;

            std::string url = uri_prefix + "/" + name;

            html += "    <div class=\"box\">\n";
            html += "      <a href=\"" + url + "\" class=\"text\">" + name + "</a>\n";
            html += "      <button onclick=\"deleteFile('" + url + "')\">Supprimer</button>\n";
            html += "    </div>\n";
        }
        closedir(dir);
    } else {
        html += "<p class=\"text\">Erreur lors de l'ouverture du dossier.</p>\n";
    }

    html +=
        "  </div>\n"
        "  <script>\n"
        "  async function deleteFile(url) {\n"
        "    const res = await fetch(url, { method: 'DELETE' });\n"
        "    if (res.ok) location.reload();\n"
        "    else alert('Erreur: ' + res.status);\n"
        "  }\n"
        "  </script>\n"
        "</body>\n"
        "</html>\n";

    return html;
}


int buildResponse(const HttpRequest& req, int fd, const Config_Data& config, Connexion *temp) {

    if (handle_501(req)) {
        std::string response = buildErrorResponse(501);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }

    if (handle_400(req)) {
        std::string response = buildErrorResponse(400);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }

    const Locations* loc = findBestLocation(req.uri, config.locations);

    // std::cout << "Location path matched: " << loc->path << std::endl;

    if (loc == NULL)
    {


        std::string path = config.root;
        if (!path.empty() && path[path.size() - 1] != '/')
            path = path + "/";
        path = path + config.index;


        if (!fileExists(path)) 
        {
            std::string response = buildErrorResponse(404);
            send(fd, response.c_str(), response.size(), 0);
            return 0;
        }

        std::string response = build_Response_200(path);
        send(fd, response.c_str(), response.size(), 0);
        return (0);
    }
    if (loc->redirect_code != 0)
    {
        std::string response = buildRedirectResponse(loc->redirect_code, loc->redirect_target);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }
    if (deal_413(req, config, loc))
    {
        std::string response = buildErrorResponse(413);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }

    if (!is_method_allowed(req.method, loc))
    {
        std::string response = buildErrorResponse(405);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }

    if (req.method == "DELETE")
    {
        int code = ProtoclDelete(req, *loc, config);

        if (code == 200)
        {
                std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
                send(fd, response.c_str(), response.size(), 0);
                return (0);
        }
        else
        {
            std::string response = buildErrorResponse(code);
            send(fd, response.c_str(), response.size(), 0);
            return (0);
        }
        return (0);
    }


    if (loc && loc->path == "/uploads")
    {
        if (!is_method_allowed("POST", loc))
        {
            std::cout << "HELLO NOT 405" << std::endl;
            std::string response = buildErrorResponse(405);
            send(fd, response.c_str(), response.size(), 0);
            return 0;
        }
        if (req.method == "POST")
        {
            if (loc->upload_enable == false)
            {
                std::cout << "hola 403" << std::endl;
                std::string response = buildErrorResponse(403);
                send(fd, response.c_str(), response.size(), 0);
                return 0;
            }

            std::string boundary;
            int code_return = ParseUpload(req, boundary);
            if (code_return != 0)
            {
                std::string response = buildErrorResponse(code_return);
                send(fd, response.c_str(), response.size(), 0);
                return 0;
            }

            code_return = ActivateUpload(req, *loc, boundary);
            if (code_return != 0)
            {
                std::string response = buildErrorResponse(code_return);
                send(fd, response.c_str(), response.size(), 0);
                return 0;
            }

            // std::cout << "hoal mehdo" << std::endl;
            std::string response = buildSimpleResponse(201, "Upload successful");
            send(fd, response.c_str(), response.size(), 0);
            return 0;
        }
    }


    std::string path = create_path(req, loc, config);
    path = decodePercentEncoding(path);
    // std::cout << "the path is : " << path << std::endl;

    if (is_path_folder(path))
    {
        if (!path.empty() && path[path.size() - 1] != '/')
        {
            path = path + "/";
        }
        if (!loc->index.empty())
        {
            std::string index_path = path + loc->index;
            // std::cout << "the index_path is : " << index_path << std::endl;
            if (fileExists(index_path))
            {
                size_t dot = index_path.rfind('.');
                if (dot != std::string::npos)
                {
                    std::string ext = index_path.substr(dot);
                    if (ext == loc->cgi_extension)
                    {
                        CgiExecutor executor;
                        std::string cgi_output = executor.envExecute(loc->cgi_path, index_path, req, temp);
                        // std::cout << "----------- CGI OUTPUT RAW -----------\n";
                        // std::cout << cgi_output << "\n";
                        // std::cout << "----------- END CGI OUTPUT -----------\n";

                        if (cgi_output == "500 Internal Server Error")
                        {
                            std::string response = buildErrorResponse(500);
                            send(fd, response.c_str(), response.size(), 0);
                            return 0;
                        }
                        std::string response = parseCgiOutput(cgi_output);
                        send(fd, response.c_str(), response.size(), 0);
                        return 0;
                    }
                }
                std::string response = build_Response_200(index_path);
                send(fd, response.c_str(), response.size(), 0);
                return 0;
            }
        }
        if (loc->autoindex == true)
        {
            std::string html = buildAutoIndex(path, req.uri);
            std::string response = "HTTP/1.1 200 OK\r\n";
            response = response + "Content-Type: text/html\r\n";
            response = response + "Content-Length: " + toString(html.size()) + "\r\n";
            response = response + "\r\n";
            response = response + html;
            send(fd, response.c_str(), response.size(), 0);
            return (0);
        }
        else
        {
            std::string html = generateCustomUploadListing(path, req.uri); 
            std::string response = "HTTP/1.1 200 OK\r\n";
            response = response + "Content-Type: text/html\r\n";
            response = response + "Content-Length: " + toString(html.size()) + "\r\n";
            response = response + "\r\n";
            response = response + html;
            send(fd, response.c_str(), response.size(), 0);
            return 0;
        }
    }

// ici on gere le cgii
    if (isCgiRequest(req, loc) == true && !loc->cgi_path.empty())
    {
        if (!fileExists(path)) {
            std::string response = buildErrorResponse(404);
            send(fd, response.c_str(), response.size(), 0);
            return 0;
        }

        CgiExecutor executor;
        std::string cgi_output = executor.envExecute(loc->cgi_path, path, req, temp);
        if (cgi_output == "500 Internal Server Error")
        {
            std::string response = buildErrorResponse(500);
            send(fd, response.c_str(), response.size(), 0);
            return 0;
        }
        std::string response = parseCgiOutput(cgi_output);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }

    if (!fileExists(path)) {
        std::string response = buildErrorResponse(404);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }

    size_t dot = path.rfind('.');
    if (dot != std::string::npos && loc->cgi_extension == path.substr(dot) && !loc->cgi_path.empty())
    {
        CgiExecutor executor;
        std::string cgi_output = executor.envExecute(loc->cgi_path, path, req, temp);
        if (cgi_output == "500 Internal Server Error")
        {
            std::string response = buildErrorResponse(500);
            send(fd, response.c_str(), response.size(), 0);
            return 0;
        }
        std::string response = parseCgiOutput(cgi_output);
        send(fd, response.c_str(), response.size(), 0);
        return 0;
    }


    std::string response = build_Response_200(path);
    send(fd, response.c_str(), response.size(), 0);
    return 0;

}

