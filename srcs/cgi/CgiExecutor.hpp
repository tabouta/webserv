#ifndef CGIEXECUTOR_HPP
# define CGIEXECUTOR_HPP

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <unistd.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <cstring>
# include "../../inc/Webserv.hpp"

class CgiExecutor {
	private:

	public:
		// char **createEnv(const HttpRequest &http);
		std::string envExecute(const std::string& cgi_path, const std::string& script_path, const HttpRequest& req, Connexion *temp);
		char **createEnv(const HttpRequest &http, const std::string& script_path);

		void freeEnv(char **env);
};

std::string parseCgiOutput(std::string cgi_output);

#endif


