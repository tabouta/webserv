#ifndef SERVER_HPP
# define SERVER_HPP

# include "../../inc/Webserv.hpp"
# include <iostream>
# include <algorithm>



int new_connexion(int serveur_fd,int epoll_fd, int fdlisten);
int request_controle(Connexion *temp, std::vector<Config_Data> config_file);
void clean_exit(int epoll_fd, std::vector<int> serveur_fd, std::vector<Connexion *> data);
int create_serveur(std::string list, std::vector<int> vector_fd, std::vector<Config_Data> config_file);
int  loop(int epoll_fd, std::vector<int> serveur_fd, std::vector<Config_Data>config_data, std::map<int, std::string> fd_to_listen, std::vector<Connexion *> *data);
int fill_req(HttpRequest *stru, std::string req, int fd);
void send_msg_error(int nerr, int fd);
void clean_exit_two(int epoll_fd, std::vector<int> &serveur_fd, std::vector<Connexion *> *data);
int	check_double_listen(Config_Data config_data, int i, std::vector<std::string> socket_mem);
#endif
