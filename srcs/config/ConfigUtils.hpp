#ifndef CONFIGUTILS_HPP
# define CONFIGUTILS_HPP

# include "../../inc/Webserv.hpp"

class ConfigUtils {
	public:
		bool filenameCheck(std::string filename);
		std::string trim(std::string& str);
		bool isExactWord(std::string line, std::string word);
		void printLocations(const Config_Data& data);
		void printConfigData(const Config_Data& data);
};

#endif
