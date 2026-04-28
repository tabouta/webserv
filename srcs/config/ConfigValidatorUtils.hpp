#ifndef CONFIGVALIDATORUTILS_HPP
# define CONFIGVALIDATORUTILS_HPP

# include "../../inc/Webserv.hpp"

class ConfigValidatorUtils {
	public:
		bool isValidDirectory(std::string path);
		bool isValidFile(std::string path);
		bool isDigit(std::string str);
};

#endif
