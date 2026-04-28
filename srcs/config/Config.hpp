#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "../../inc/Webserv.hpp"

class	Config {
	private:
		std::string	filename;
		std::ifstream	config;
	public:
		Config();
		Config(std::string filename);
		~Config();
		void	getLocation(Config_Data *data, std::string line, std::ifstream& file, std::string path);
		void	getData(Config_Data *data, std::string line, std::ifstream& file, char **env);
		bool readServerConfig(Config_Data *data, std::ifstream& file, char **env);
		int parseFile(std::vector<Config_Data>& servers, char **env);
};

#endif
