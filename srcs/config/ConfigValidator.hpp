#ifndef CONFIGVALIDATOR_HPP
# define CONFIGVALIDATOR_HPP

# include "../../inc/Webserv.hpp"

class ConfigValidator {
	public:
		bool checkIP(std::string ip);
		bool checkDirectories(const Config_Data& data);
		bool checkFiles(const Config_Data& data);
		bool checkAdresses(const Config_Data& data);
};

#endif
