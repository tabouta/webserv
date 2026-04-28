
#ifndef WEBSERV_HPP
# define WEBSERV_HPP


#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>





# include <string>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <cstdio>  
# include <unistd.h>
# include <errno.h>
# include <fcntl.h>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/stat.h>
#include <sys/time.h>
# include <sys/epoll.h>
# include <strings.h>
# include <iostream>
# include <cctype>
# include <map>
# include <vector>
#include <dirent.h>
# include <sstream>
# include <fstream>
# include <ctime>
# include <string>


# define PORT 8080
# define CLOCK_UNINITIALIZE (clock_t)(-1)

# define MAXLINE 4080
# define MAX_REQ 1048576
# define MAX_EVENTS 100
# define BUF_SIZE 1024

class Connexion
{
    public:
        std::string     req;
        int             fd;
        clock_t         time;
        std::string     listen;
        int             is_dead;
        pid_t           pid;
        int             epfd;

};

/*                  Config                   */
# include "../srcs/config/ConfigData.hpp"
# include "../srcs/config/ConfigUtils.hpp"
# include "../srcs/config/ConfigValidatorUtils.hpp"
# include "../srcs/config/ConfigValidator.hpp"
# include "../srcs/config/ConfigParser.hpp"
# include "../srcs/config/Config.hpp"

/*                  HTTP Upload                   */
# include "../srcs/httpUpload/HttpUpload.hpp"


/*                  HTTP Request/Response                   */
# include "../srcs/httpRequest/HttpRequest.hpp"
# include "../srcs/httpRequest/Request_parsing.hpp"
# include "../srcs/httpResponse/HttpResponse.hpp"

/*                  Server                   */
# include "../srcs/server/Server.hpp"

#endif

