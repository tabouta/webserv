#include "../../inc/Webserv.hpp"

bool ConfigUtils::filenameCheck(std::string filename) {
	std::string	end = ".conf";
	if (filename.length() < end.length())
		return false;
	return (filename.compare(filename.length() - end.length(), end.length(), end) == 0);
}

std::string ConfigUtils::trim(std::string& str) {
	size_t start = 0;
	while (start < str.size() && std::isspace(str[start]))
		start++;
	size_t end = str.size();
	while (end > start && std::isspace(str[end - 1]))
		end--;
	return str.substr(start, end - start);
}

bool ConfigUtils::isExactWord(std::string line, std::string word) {
	size_t pos = line.find(word);
	if (pos == std::string::npos)
		return false;
	if (pos > 0 && !std::isspace(line[pos - 1]))
		return false;
	if (pos + word.length() < line.length() && !std::isspace(line[pos + word.length()]))
		return false;
	return true;
}

void ConfigUtils::printLocations(const Config_Data& data) {
	std::cout << "---- Locations ----" << std::endl;
	for (std::map<std::string, Locations>::const_iterator it = data.locations.begin();
		 it != data.locations.end(); ++it) {
		const std::string& loc_key = it->first;
		const Locations& loc = it->second;
		std::cout << "Location: " << loc_key << std::endl;
		std::cout << "	path:			" << loc.path << std::endl;
		std::cout << "	root:			" << loc.root << std::endl;
		std::cout << "	alias:			" << loc.alias << std::endl;
		std::cout << "	index:			" << loc.index << std::endl;
		std::cout << "	redirect_code:			" << loc.redirect_code << std::endl;
		std::cout << "	redirect_target:			" << loc.redirect_target << std::endl;
		std::cout << "	cgi_extension:	" << loc.cgi_extension << std::endl;
		std::cout << "	cgi_path:		" << loc.cgi_path << std::endl;
		std::cout << "	upload_store:	" << loc.upload_store << std::endl;
		std::cout << "	autoindex:		" << (loc.autoindex ? "on" : "off") << std::endl;
		std::cout << "	upload_enable:	" << (loc.upload_enable ? "on" : "off") << std::endl;
		std::cout << "	methods:		";
		for (size_t i = 0; i < loc.methods.size(); ++i) {
			std::cout << loc.methods[i];
			if (i + 1 < loc.methods.size())
				std::cout << " ";
		}
		std::cout << std::endl << std::endl;
	}
	std::cout << "-------------------" << std::endl;
}

void ConfigUtils::printConfigData(const Config_Data& data) {
	std::cout << "---- Config Data ----" << std::endl;
	std::cout << "Path:" << data.path << std::endl;
	std::cout << "Listen:" << std::endl;
	for (size_t i = 0; i < data.listen.size(); ++i)
		std::cout << "  - " << data.listen[i] << std::endl;
	std::cout << "Server Name: " << data.server_name << std::endl;
	std::cout << "Root: " << data.root << std::endl;
	std::cout << "Index: " << data.index << std::endl;
	std::cout << "Client Max Body Size: " << data.max_body_size << std::endl;
	std::cout << "Error Pages:" << std::endl;
	std::map<int, std::string>::const_iterator it;
	for (it = data.error_pages.begin(); it != data.error_pages.end(); ++it) {
		std::cout << "  - " << it->first << " => " << it->second << std::endl;
	}
	std::cout << "---------------------" << std::endl;
}
