#ifndef HTTPUPLOAD_HPP
#define HTTPUPLOAD_HPP

# include "../../inc/Webserv.hpp"
#include <string>
#include <map>
#include "../httpRequest/HttpRequest.hpp"


struct MultipartBlock
{
    std::map<std::string, std::string> headers;
    std::string content;
};

int ActivateUpload(const HttpRequest& req, const Locations& loc, const std::string& boundary);
int ParseUpload(const HttpRequest& req, std::string& boundary);
int extractBoundary(const HttpRequest& req, std::string& boundary_out);
int checkMultipartHeader(const HttpRequest& req);
std::string getContentTypeFromExtension(const std::string& path);
std::vector<MultipartBlock> splitBodyIntoParts(const std::string& body, const std::string& boundary);
bool isFilePart(const MultipartBlock& part);
bool saveMultipartFile(const MultipartBlock& part, const Locations& loc);
#endif
