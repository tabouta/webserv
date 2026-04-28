#include "../../inc/Webserv.hpp"
#include "CgiExecutor.hpp"

char **CgiExecutor::createEnv(const HttpRequest &http, const std::string& script_path)
{
    char **http_env = new char*[13];
    int i = 0;

    std::string query_string;
    std::string content_type;

    size_t pos = http.uri.find('?');
    if (pos != std::string::npos)
        query_string = http.uri.substr(pos + 1);
    else
        query_string = "";

    std::map<std::string, std::string>::const_iterator it = http.headers.find("Content-Type");
    if (it != http.headers.end())
        content_type = it->second;
    else
        content_type = "";

    http_env[i++] = strdup(("REQUEST_METHOD=" + http.method).c_str());
    http_env[i++] = strdup(("SCRIPT_NAME=" + http.uri).c_str());
    http_env[i++] = strdup(("SCRIPT_FILENAME=" + script_path).c_str()); // 🧨 la ligne manquante
    http_env[i++] = strdup(("QUERY_STRING=" + query_string).c_str());
    http_env[i++] = strdup(("CONTENT_TYPE=" + content_type).c_str());
    http_env[i++] = strdup(("CONTENT_LENGTH=" + toString(http.content_length)).c_str());
    http_env[i++] = strdup(("SERVER_PROTOCOL=" + http.http_version).c_str());
    http_env[i++] = strdup("GATEWAY_INTERFACE=CGI/1.1");
    http_env[i++] = strdup("REDIRECT_STATUS=200");
    http_env[i++] = NULL;

    return http_env;
}


void CgiExecutor::freeEnv(char **env)
{
	if (!env)
		return;

	for (int i = 0; env[i] != NULL; ++i) {
		free(env[i]);
	}
	delete[] env;
}

std::string CgiExecutor::envExecute(const std::string& cgi_path, const std::string& script_path, const HttpRequest& req, Connexion *temp)
{
	pid_t pid;
	char **env;
	int inputPipe[2];
	int outputPipe[2];

	// std::cout << "cgi_path: " << cgi_path << std::endl;

	env = createEnv(req, script_path);

	pipe(inputPipe);
	pipe(outputPipe);

	pid = fork();
    temp->pid = pid;
	if (pid < 0) {
		std::cerr << "Error: fork" << std::endl;
		freeEnv(env);
		return ("500 Internal Server Error");
	}
	else if (pid == 0) {
		dup2(inputPipe[0], STDIN_FILENO);
		dup2(outputPipe[1], STDOUT_FILENO);
		dup2(outputPipe[1], STDERR_FILENO);

		close(inputPipe[1]);
		close(outputPipe[0]);

		char *av[] = {
			strdup(cgi_path.c_str()),       
			strdup(script_path.c_str()),    
			NULL
		};

		execve(cgi_path.c_str(), av, env);
		perror("execve failed");
		exit(1); 
	}
	else {
        
        close(inputPipe[0]);
		close(inputPipe[1]);
		write(inputPipe[1], req.body.c_str(), req.body.size());
        close(outputPipe[1]);
        
		std::string cgi_output;
		char buffer[1024];


        int epfd = epoll_create1(0);
        if (epfd == -1) 
        {
            perror("epoll_create1");
            freeEnv(env);
		    return cgi_output;
        }
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = outputPipe[0];

        if (epoll_ctl(epfd, EPOLL_CTL_ADD, outputPipe[0], &ev) == -1)
        {
            perror("epoll_ctl");
            close(outputPipe[0]);
            close(epfd);
            freeEnv(env);
            return cgi_output;
        }
        time_t start = time(NULL);


        while (true)
        {
            epoll_event events[1];
            int nfds = epoll_wait(epfd, events, 1, 100); // 100ms timeout pour boucle
            if (nfds == -1) 
            {
                perror("epoll_wait");
                break;
            } 
            else if (nfds > 0) 
            {
                if (events[0].data.fd == outputPipe[0] && (events[0].events & EPOLLIN)) 
                {
                    ssize_t bytes = read(outputPipe[0], buffer, sizeof(buffer));
                    if (bytes > 0) 
                        cgi_output.append(buffer, bytes);
                     else if (bytes == 0) 
                        break;
                }
            }
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid)
                break;

            // Timeout
            if (difftime(time(NULL), start) >= 3) {
                std::cerr << "CGI timeout: killing pid " << pid << std::endl;
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0); // éviter zombie
                break;
            }
        }

        // std::cout << "On continue de tourner 3\n";

		if (cgi_output.empty()) {
    		std::cerr << "CGI OUTPUT EMPTY " << std::endl;
		} else {
    		std::cerr << "CGI OUTPUT DEBUG:\n" << cgi_output << std::endl;
		}

		close(outputPipe[0]);
        close(epfd);
		freeEnv(env);
		return cgi_output;
	}
}

std::string toLower(const std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = std::tolower(result[i]);
    return result;
}


std::string parseCgiOutput(std::string cgi_output)
{
    size_t pos = cgi_output.find("\r\n\r\n");
	if (pos == std::string::npos) {
        pos = cgi_output.find("\n\n");
    }
    if (pos == std::string::npos || pos + 2 >= cgi_output.size()) {
        return buildErrorResponse(502);
    }

    std::string headers_part = cgi_output.substr(0, pos);
    std::string body_part = cgi_output.substr(pos + 4);

    std::string content_type;
    std::string status = "200 OK"; 
    size_t content_length = body_part.size();

    std::istringstream header_stream(headers_part);
    std::string line;

    while (std::getline(header_stream, line)) {
		while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n' || line[line.size() - 1] == ' '))
    		line.erase(line.size() - 1);

        size_t sep = line.find(':');
        if (sep == std::string::npos)
            continue;

        std::string key = line.substr(0, sep);
        std::string value = line.substr(sep + 1);

        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);

        while (!value.empty() && (value[value.size() - 1] == '\r' || value[value.size() - 1] == '\n' || value[value.size() - 1] == ' '))
            value.erase(value.size() - 1);

        std::string key_lower = toLower(key);

        if (key_lower == "content-type") {
            content_type = value;
        }
        else if (key_lower == "content-length") {
            std::istringstream(value) >> content_length;
        }
        else if (key_lower == "status") {
            status = value;
        }
    }

    // std::cout << "CGI PARSED content_type=[" << content_type << "]" << std::endl;

    if (content_type.empty()) {
        return buildErrorResponse(502);
    }

    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + toString(content_length) + "\r\n";
    response += "\r\n";
    response += body_part;

    return response;
}


