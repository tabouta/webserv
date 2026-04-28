#include "../../inc/Webserv.hpp"
#include "ConfigUtils.hpp"

std::string	ConfigParser::parseString(std::string line, std::string word) {
	ConfigUtils utils;
	size_t pos = line.find(word);
	if (pos == std::string::npos)
		return "";
	std::string newWord = line.substr(pos + word.length());
	std::string trimmed = utils.trim(newWord);
	if (trimmed.at(trimmed.length() - 1) != ';') {
		std::cerr << "Missing ';' in .conf" << std::endl;
		return "";
	}
	trimmed = trimmed.substr(0, trimmed.length() - 1);
	return trimmed;
}

std::string	ConfigParser::getAddress(Config_Data *data, std::string line, std::string word) {
	std::string address = parseString(line, word);
	std::string path = data->path + address;
	// std::cout << path << std::endl;
	return path;
}

void ConfigParser::parseRedirect(Config_Data *data, std::string line, std::string path) {
	std::string str = parseString(line, "return");
	size_t last = str.find_last_of(' ');
	if (last == std::string::npos)
		return ;
	std::string redirect = str.substr(last + 1);
	str = str.substr(0, last);
	size_t start = 0;
	while ((start = str.find_first_not_of(' ', start)) != std::string::npos) {
		size_t end = str.find(' ', start);
		std::string num_str = (end == std::string::npos)
			? str.substr(start)
			: str.substr(start, end - start);
		int value = atoi(num_str.c_str());
		if (value != 0) {
			data->locations[path].redirect_target = redirect;
			data->locations[path]. redirect_code = value;
		}
		if (end == std::string::npos)
			break ;
		start = end + 1;
	}
}

void ConfigParser::parseMap(Config_Data *data, std::map<int, std::string>& map, std::string str) {
	size_t last = str.find_last_of(' ');
	if (last == std::string::npos)
		return ;
	std::string path = str.substr(last + 1);
	str = str.substr(0, last);
	size_t start = 0;
	while ((start = str.find_first_not_of(' ', start)) != std::string::npos) {
		size_t end = str.find(' ', start);
		std::string num_str = (end == std::string::npos)
			? str.substr(start)
			: str.substr(start, end - start);
		int value = atoi(num_str.c_str());
		if (value != 0)
			map[value] = data->path + path;
		if (end == std::string::npos)
			break ;
		start = end + 1;
	}
}

std::string ConfigParser::parseLocation(std::string line) {
	ConfigUtils utils;
	std::string word = "location";
	size_t pos = line.find(word);
	if (pos == std::string::npos)
		return "";
	std::string newWord = line.substr(pos + word.length());
	pos = newWord.find('{');
	if (pos == std::string::npos)
		return "";
	newWord = newWord.substr(0, pos - 1);
	std::string trimmed = utils.trim(newWord);
	return trimmed;
}

void ConfigParser::parseMethods(Config_Data *data, std::string path, std::string str) {

	size_t start = 0;
	while ((start = str.find_first_not_of(' ', start)) != std::string::npos) {
		size_t end = str.find(' ', start);
		std::string method;
		if (end == std::string::npos) {
			method = str.substr(start);
			start = std::string::npos;
		} else {
			method = str.substr(start, end - start);
			start = end + 1;
		}
		if (!method.empty() && method[0] == ',')
			method = method.substr(1);
		if (!method.empty() && method[method.size() - 1] == ',')
			method = method.substr(0, method.size() - 1);
		if (method == "GET" || method == "POST" || method == "DELETE")
			data->locations[path].methods.push_back(method);
	}
}
