#include "HttpUpload.hpp"

int checkMultipartHeader(const HttpRequest& req)
{
    std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");

    // std::cerr << "Content-Type: " << req.headers["Content-Type"] << std::endl;
    if (it == req.headers.end())
    {
        printf("hola one \n");
        return 400;
    }

    const std::string& contentType = it->second;

    if (contentType.find("multipart/form-data") != 0)
        return 415; 

    return 0;
}

int extractBoundary(const HttpRequest& req, std::string& boundary_out)
{
    std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");

    if (it == req.headers.end())
    {
        printf("hola two \n");
        return 400;
    }

    const std::string& contentType = it->second;
    std::string boundaryPrefix = "boundary=";

    std::size_t pos = contentType.find(boundaryPrefix);

    if (pos == std::string::npos)
    {
        printf("hola three\n");
        return 400;
    } 

    std::size_t boundaryStart = pos + boundaryPrefix.size();
    if (boundaryStart >= contentType.size())
    {
        printf("hola four\n");
        return 400;
    }

    boundary_out = contentType.substr(boundaryStart);

    return 0;
}

int ParseUpload(const HttpRequest& req, std::string& boundary)
{
    int ret;

    ret = checkMultipartHeader(req);
    if (ret != 0)
        return ret;

    ret = extractBoundary(req, boundary);
    if (ret != 0)
        return ret;

    return 0; 
}


bool saveMultipartFile(const MultipartBlock& part, const Locations& loc)
{
    std::map<std::string, std::string>::const_iterator it = part.headers.find("Content-Disposition");
    if (it == part.headers.end()) {
        return false;
    }

    const std::string& header = it->second;

    size_t filename_pos = header.find("filename=\"");
    if (filename_pos == std::string::npos) {
        return false;
    }

    filename_pos = filename_pos + 10;
    size_t end_pos = header.find("\"", filename_pos);
    if (end_pos == std::string::npos) {
        return false;
    }

    std::string filename = header.substr(filename_pos, end_pos - filename_pos);

    std::string path = loc.upload_store + "/" + filename;

    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(part.content.data(), part.content.size());
    file.close();

    return true;
}


int ActivateUpload(const HttpRequest& req, const Locations& loc, const std::string& boundary)
{
    std::vector<MultipartBlock> parts = splitBodyIntoParts(req.body, boundary);
    // std::cout << "Voici le body dans son ensemble : \n" << req.body << std::endl;
    if (parts.empty())
    {
        printf("hola six\n");
        return 400;
    }

    bool file_found = false;

    for (std::size_t i = 0; i < parts.size(); i = i + 1)
    {
        if (!isFilePart(parts[i]))
            continue;

        file_found = true;

        if (!saveMultipartFile(parts[i], loc))
        {
            return 500;
        }
    }

    if (!file_found)
        return 422;

    return 0;
}

std::string getContentTypeFromExtension(const std::string& path) {
    size_t dot_pos = path.rfind('.');

    if (dot_pos == std::string::npos) {
        return "application/octet-stream"; 
    }

    std::string ext = path.substr(dot_pos); 

    if (ext == ".html" || ext == ".htm") {
        return "text/html";
    }

    if (ext == ".css") {
        return "text/css";
    }

    if (ext == ".js") {
        return "application/javascript";
    }

    if (ext == ".json") {
        return "application/json";
    }

    if (ext == ".png") {
        return "image/png";
    }

    if (ext == ".jpg" || ext == ".jpeg") {
        return "image/jpeg";
    }

    if (ext == ".gif") {
        return "image/gif";
    }

    if (ext == ".txt") {
        return "text/plain";
    }

    return "application/octet-stream"; // Type par défaut si inconnu
}

bool isFilePart(const MultipartBlock& part)
{
    std::map<std::string, std::string>::const_iterator it = part.headers.find("Content-Disposition");
    if (it == part.headers.end())
        return false;

    const std::string& value = it->second;
    return value.find("filename=") != std::string::npos;
}

std::vector<MultipartBlock> splitBodyIntoParts(const std::string& body, const std::string& boundary)
{
    std::vector<MultipartBlock> parts;
    std::string delimiter = "--" + boundary;
    std::string close_delimiter = delimiter + "--";

    size_t pos = 0;
    size_t end = 0;

    while ((pos = body.find(delimiter, pos)) != std::string::npos) {
        pos = pos + delimiter.size();

        if (body.substr(pos, 2) == "--") {
            break;
        }

        if (body.substr(pos, 2) == "\r\n") {
            pos = pos + 2;
        }

        end = body.find(delimiter, pos);
        if (end == std::string::npos) {
            break;
        }

        std::string part = body.substr(pos, end - pos);

        size_t header_end = part.find("\r\n\r\n");
        if (header_end == std::string::npos) {
            pos = end;
            continue;
        }

        std::string raw_headers = part.substr(0, header_end);
        std::string content = part.substr(header_end + 4);

        MultipartBlock block;
        std::istringstream stream(raw_headers);
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r') {
                line = line.substr(0, line.size() - 1);
            }
            size_t sep = line.find(": ");
            if (sep != std::string::npos) {
                std::string key = line.substr(0, sep);
                std::string value = line.substr(sep + 2);
                block.headers[key] = value;
            }
        }

        block.content = content;
        parts.push_back(block);

        pos = end;
    }

    return parts;
}





