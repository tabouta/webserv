#include "../../inc/Webserv.hpp"
#include "ConfigValidatorUtils.hpp"

bool ConfigValidator::checkIP(std::string ip) {
	ConfigValidatorUtils utils;
	size_t space = ip.find(' ');
	if (space != std::string::npos)
		return false;
	size_t pos = ip.find(':');
	if (pos == std::string::npos)
		return false;
	std::string ipAdress = ip.substr(0, pos);
	std::string portAdress = ip.substr(pos + 1);
	if (!utils.isDigit(portAdress))
		return false;
	int port = atoi(portAdress.c_str());
	if (port < 0 || port > 65535)
		return false;
	std::vector<std::string> parts;
	std::string nbr;
	std::istringstream iss(ipAdress);
	while (std::getline(iss, nbr, '.'))
		parts.push_back(nbr);
	if (parts.size() != 4)
		return false;
	for (size_t i = 0; i < 4; i++) {
		if (!utils.isDigit(parts[i]))
			return false;
		int num = atoi(parts[i].c_str());
		if (num < 0 || num > 255)
			return false;
	}
	return true;
}

bool ConfigValidator::checkDirectories(const Config_Data& data) {
	ConfigValidatorUtils utils;
	if (!utils.isValidDirectory(data.root))
		return false;
	std::string root = data.root;
	std::map<std::string, Locations>::const_iterator it;
	for (it = data.locations.begin(); it != data.locations.end(); ++it) {
		if (!it->second.alias.empty()) {
			if (!utils.isValidDirectory(it->second.alias)) {
				std::cerr << "Error in alias " << " " << it->second.alias << std::endl;
				return false;
			}
		}
		std::string root_location = it->second.root;
		if (!utils.isValidDirectory(root_location)) {
			std::cerr << "Error in root" << std::endl;
			return false;
		}
		if (!utils.isValidDirectory(root_location + it->second.alias)) {
			std::cerr << "Error in root_location" << std::endl;
			return false;
		}
		// if (!utils.isValidDirectory(root_location + it->second.cgi_extension))
		// 	return false;
		// if (!utils.isValidDirectory(root_location + it->second.cgi_path))
		// 	return false;
		// if (!utils.isValidDirectory(root_location + it->second.upload_store))
		// 	return false;
	}
	return true;
}

bool ConfigValidator::checkFiles(const Config_Data& data) {
	ConfigValidatorUtils utils;
	if (!data.index.empty()) {
		std::string index = data.root + "/" + data.index;
		if (!utils.isValidFile(index)) {
			std::cerr << "Error in general index address" << std::endl;
			return false;
		}
	}
	std::map<int, std::string>::const_iterator it;
	for (it = data.error_pages.begin(); it != data.error_pages.end(); ++it) {
		if (!utils.isValidFile(it->second)) {
			std::cerr << "Error in error page " << it->second << " address" << std::endl;
			return false;
		}
	}
	std::map<std::string, Locations>::const_iterator itl;
	for (itl = data.locations.begin(); itl != data.locations.end(); ++itl) {
		std::string root_location;
		if (itl->second.root.empty())
			root_location = data.root;
		else
			root_location = itl->second.root;
		if (!itl->second.index.empty() && !utils.isValidFile(root_location + "/" + itl->second.index)) {
			std::cout << root_location + "/" + itl->second.index << std::endl;
			std::cerr << "Error in location " << itl->first << " index address" << std::endl;
			return false;
		}
	}
	return true;
}

bool ConfigValidator::checkAdresses(const Config_Data& data) {
	for (size_t i = 0; i < data.listen.size(); i++) {
		if (!checkIP(data.listen[i])) {
			std::cerr << "Error in ip adresses" << std::endl;
			return false;
		}
	}
	if (!checkDirectories(data)) {
		std::cerr << "Error in directories adresses" << std::endl;
		return false;
	}
	if (!checkFiles(data)) {
		std::cerr << "Error in file adresses" << std::endl;
		return false;
	}
	return true;
}
