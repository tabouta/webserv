#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

# include "../../inc/Webserv.hpp"

class ConfigParser {
	public:
		std::string	parseString(std::string line, std::string word);
		std::string	getAddress(Config_Data *data, std::string line, std::string word);
		void parseRedirect(Config_Data *data, std::string line, std::string parse);
		void parseMap(Config_Data *data, std::map<int, std::string>& map, std::string str);
		std::string parseLocation(std::string line);
		void parseMethods(Config_Data *data, std::string path, std::string str);
};

#endif
