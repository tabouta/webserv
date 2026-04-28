#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest
{
	public:
	    std::string method;
	    std::string uri;
	    std::string http_version;
	    std::map<std::string, std::string> headers;
	    std::string body;
	    size_t content_length;
};

#endif
