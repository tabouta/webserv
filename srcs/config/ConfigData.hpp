#ifndef CONFIGDATA_HPP
# define CONFIGDATA_HPP

# include "../../inc/Webserv.hpp"

struct	Locations
{
	std::string	path;
	std::string	root;
	std::string	alias;
	std::string	index;
	int body_max_size;
	std::string redirect_target;
	std::string	cgi_extension;
	std::string cgi_path;
	std::string	upload_store;
	bool	autoindex;
	bool	upload_enable;
	int	redirect_code;
	std::vector<std::string>	methods;

	Locations() : body_max_size(-1), autoindex(false), upload_enable(false), redirect_code(0) {}

};

struct	Config_Data
{
	std::string	path;
	std::vector<std::string>	listen;
	std::string	server_name;
	std::string	root;
	std::string	index;
	std::map<int, std::string>	error_pages;
	int	max_body_size;
	std::map<std::string, Locations>	locations;
};

#endif
