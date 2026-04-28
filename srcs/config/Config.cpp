#include "../../inc/Webserv.hpp"

Config::Config() {
	if (!this->filename.empty()) {
		this->config.open(this->filename.c_str(), std::ios::in);
		if (!this->config.is_open()) {
			std::cerr << "Couldn't open config file" << std::endl;
			return ;
		}
	}
	else {
		std::cerr << "Couldn't open config file" << std::endl;
		return ;
	}
}

Config::Config(std::string filename) : filename(filename) {
	if (!filename.empty()) {
		this->config.open(filename.c_str(), std::ios::in);
		if (!this->config.is_open()) {
			std::cerr << "Couldn't open config file" << std::endl;
			return ;
		}
	}
	else {
		std::cerr << "Couldn't open config file" << std::endl;
		return ;
	}
}

Config::~Config() {
	this->config.close();
}

void	Config::getLocation(Config_Data *data, std::string line, std::ifstream& file, std::string path) {
	ConfigUtils utils;
	ConfigParser parse;

	data->locations[path].path = path;

	while (std::getline(file, line)) {
		std::string trimmed = utils.trim(line);
		if (trimmed.empty() || trimmed.at(0) == '#')
			continue;
		if (utils.isExactWord(line, "index")) {
			data->locations[path].index = parse.parseString(line, "index");
		}
		else if (utils.isExactWord(line, "root")) {
			data->locations[path].root = parse.getAddress(data, line, "root");
		}
		else if (utils.isExactWord(line, "alias")) {
			data->locations[path].alias = parse.getAddress(data, line, "alias");
		}
		else if (utils.isExactWord(line, "autoindex")) {
			std::string str_autoindex = parse.parseString(line, "autoindex");
			if (str_autoindex == "on")
				data->locations[path].autoindex = true;
			else
				data->locations[path].autoindex = false;
		}
		else if (utils.isExactWord(line, "return")) {
			parse.parseRedirect(data, line, path);
		}
		else if (utils.isExactWord(line, "upload_enable")) {
			std::string str_upload = parse.parseString(line, "upload_enable");
			if (str_upload == "on")
				data->locations[path].upload_enable = true;
			else
				data->locations[path].upload_enable = false;
		}
		else if (utils.isExactWord(line, "cgi_extension")) {
			data->locations[path].cgi_extension = parse.parseString(line, "cgi_extension");
		}
		else if (utils.isExactWord(line, "cgi_path")) {
			data->locations[path].cgi_path = parse.parseString(line, "cgi_path");
		}
		else if (utils.isExactWord(line, "upload_store")) {
			data->locations[path].upload_store = parse.getAddress(data, line, "upload_store");
		}
		else if (utils.isExactWord(line, "methods")) {
			std::string str_methods = parse.parseString(line, "methods");
			parse.parseMethods(data, path, str_methods);
		}
		else if (utils.isExactWord(line, "}")) {
			break ;
		}
	}
	return ;
}

std::string getPath(char **env) {
	for (int i = 0; env[i]; i++) {
		if (strncmp(env[i], "PWD=", 4) == 0) {
			std::string path = env[i];
			return path.substr(4);
		}
	}
	return NULL;
}

void	Config::getData(Config_Data *data, std::string line, std::ifstream& file, char **env) {
	ConfigUtils utils;
	ConfigParser parse;
	data->path = getPath(env);
	if (data->path.empty()) {
		std::cerr << "Error in PWD env" << std::endl;
		return ;
	}
	if (utils.isExactWord(line, "listen")) {
		data->listen.push_back(parse.parseString(line, "listen"));
	}
	else if (utils.isExactWord(line, "server_name")) {
		data->server_name = parse.parseString(line, "server_name");
	}
	else if (utils.isExactWord(line, "root")) {
		data->root = parse.getAddress(data, line, "root");
	}
	else if (utils.isExactWord(line, "index")) {
		data->index = parse.parseString(line, "index");
	}
	else if (utils.isExactWord(line, "client_max_body_size")) {
		std::string max_body = parse.parseString(line, "client_max_body_size");
		std::istringstream iss(max_body);
		int value;
		iss >> value;
		data->max_body_size = value;
	}
	else if (utils.isExactWord(line, "error_pages")) {
		std::string str = parse.parseString(line, "error_pages");
		parse.parseMap(data, data->error_pages, str);
	}
	else if (utils.isExactWord(line, "location")) {
		std::string path = parse.parseLocation(line);
		getLocation(data, line, file, path);
	}
}

bool Config::readServerConfig(Config_Data *data, std::ifstream& file, char **env) {
	ConfigUtils utils;
	std::string line;

	while (std::getline(file, line)) {
		std::string trimmed = utils.trim(line);
		if (trimmed.empty() || trimmed.at(0) == '#')
			continue;
		if (utils.isExactWord(line, "}")) {
			break ;
		}
		getData(data, line, file, env);
	}
	return true;
}

int Config::parseFile(std::vector<Config_Data>& servers, char **env)
{
	ConfigUtils utils;
	ConfigValidator valid;

	if (!utils.filenameCheck(this->filename)) {
		std::cerr << "File has not .conf" << std::endl;
		return 1;
	}

	std::string line;
	while (std::getline(this->config, line)) {
		line = utils.trim(line);
		if (line.empty())
			continue;

		if (utils.isExactWord(line, "server")) {
			if (line.find("{") != std::string::npos) {
				Config_Data data;
				if (!readServerConfig(&data, this->config, env)) {
					std::cerr << "Failed to read server config" << std::endl;
					return 1;
				}
				if (!valid.checkAdresses(data)) {
					std::cerr << "Error in config file" << std::endl;
					return 1;
				}
				servers.push_back(data);
				// utils.printConfigData(data);
				// utils.printLocations(data);
			} else {
				if (std::getline(this->config, line) && utils.isExactWord(line, "{")) {
					line = utils.trim(line);
					if (line != "{") {
						std::cerr << "Expected '{' after server aqui1" << std::endl;
						return 1;
					}
				Config_Data data;
				if (!readServerConfig(&data, this->config, env)) {
					std::cerr << "Failed to read server config" << std::endl;
					return 1;
				}
				if (!valid.checkAdresses(data)) {
					std::cerr << "Error in config file" << std::endl;
					return 1;
				}
				servers.push_back(data);
				// utils.printConfigData(data);
				// utils.printLocations(data);
				} else {
					std::cerr << "Expected '{' after server" << std::endl;
					return 1;
				}
			}
		}
	}
	return 0;
}

