#include "../../inc/Webserv.hpp"

bool ConfigValidatorUtils::isValidDirectory(std::string path) {
	if (path.empty())
		return true;
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return false;
	else if (info.st_mode & S_IFDIR)
		return true;
	return false;
}

bool ConfigValidatorUtils::isValidFile(std::string path) {
	if (path.empty())
		return true;
	struct stat info;
	
	if (stat(path.c_str(), &info) != 0)
		return false;
	else if (info.st_mode & S_IFDIR)
		return false;
	return true;
}

bool ConfigValidatorUtils::isDigit(std::string str) {
	if (str.empty())
		return false;
	for (size_t i = 0; i < str.size(); i++) {
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}
