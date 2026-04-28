
#include "Server.hpp"

void check_timeout(std::vector<Connexion *> *data, int epoll_fd)
{
    int cmd_found = 0;
    char buffer[BUF_SIZE] = {0};
    Connexion *temp;
    // if (data->size() > 0)
    //     std::cout << "Voici la taille de data :" << data->size() <<std::endl;
    for (unsigned long i = 0; i < data->size(); i++)
    {
        if ((*data)[i])
        {
            cmd_found++;
            struct timeval tv;
            gettimeofday(&tv, NULL);
            //std::cout<< (std::clock() - data[i]->time) << " > " << CLOCKS_PER_SEC / 40 <<std::endl;
            if (tv.tv_sec - (*data)[i]->time > 3)
            {
                // std::cout << "destruction timeout!!!\n";
                // std::cout << "voici le file descriptor " << (*data)[i]->fd << std::endl;
                std::string response = buildErrorResponse(408);

                int ret;
                while (true)
                {
                    ret = recv((*data)[i]->fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                    if (ret == 0)
                        break ;
                    else if (ret < 0)
                        break ;
                }
                send((*data)[i]->fd, response.c_str(), response.size(), 0);
                temp = (*data)[i];
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, temp->fd, NULL);
                if (temp->pid != 0)
                    kill(temp->pid, SIGKILL);
                close(temp->fd);
                delete(temp);
                data->erase(std::remove(data->begin(), data->end(), temp), data->end());
            }
        }
    }
}
int new_connexion(int serveur_fd,int epoll_fd, std::string socket_listen, std::vector<Connexion *>& data)
{
    struct epoll_event ev2;
    int client_fd;
    Connexion *connexion;

    // std::cout << "valeur de serveur_fd :" << serveur_fd << std::endl;
    //accept cree une socket pour ecouter ce que la nouvelle connexion veut nous dire
    client_fd = accept(serveur_fd, NULL, NULL);
    if (client_fd < 0)
        return 0;

    //on change le fd pour le rendre non bloquant
    int flags = fcntl(client_fd, F_GETFL, 0);
    if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        return 0;

    //On ajoute une nouvelle regle qui permet de voir tout ce que cette nouvelle connexion va nous dire
    ev2.events = EPOLLIN;
    connexion = new Connexion;
    connexion->epfd = epoll_fd;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    connexion->time = tv.tv_sec;
    connexion->fd = client_fd;
    connexion->is_dead = 0;
    connexion->pid = 0;
    connexion->listen = socket_listen;
    ev2.data.ptr = connexion;
    data.push_back(connexion);

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev2) < 0)
        return 0;

    // printf("nouvelle connexion enregistre!!\n");
    // std::cout << Connexion->time << std::endl;
    return 1;
}

static int check_incoming_request(std::vector<int> serveur_fd, struct epoll_event* events, int i, int epoll_fd,  std::vector<Connexion *>& data,  std::map<int, std::string> fd_to_listen)
{
    unsigned long j = 0;
    while (j < serveur_fd.size())
    {
        if (events[i].data.fd == serveur_fd[j])
        {
            if (!new_connexion(serveur_fd[j], epoll_fd, fd_to_listen[serveur_fd[j]], data))
                return (clean_exit(epoll_fd, serveur_fd, data), 0);
            return 1;
        }
        j++;
    }
    return -1;

}


static void add_data_connexion(std::vector<Connexion *> *data, Connexion *temp)
{
    for (unsigned long i = 0; i < data->size(); i++)
    {
        if ((*data)[i]->fd == temp->fd)
            return ;
    }
    data->push_back(temp);
}

int  loop(int epoll_fd, std::vector<int> serveur_fd, std::vector<Config_Data>config_data, std::map<int, std::string> fd_to_listen, std::vector<Connexion *> *data)
{
    struct epoll_event events[MAX_EVENTS];


    int flag;
    Connexion *temp;

    check_timeout(data, epoll_fd);
    int n = epoll_wait(epoll_fd, events, 50, 10);

    for (int i = 0; i < n; i++)
    {
        //Si la regle epoll activer n'est pas une connexion preexistante alors c'est une requete
        flag = check_incoming_request(serveur_fd, events, i, epoll_fd, *data, fd_to_listen);
        if (flag < 0)
        {
            temp = static_cast<Connexion *>(events[i].data.ptr);
            add_data_connexion(data, temp);
            flag = request_controle(temp, config_data);
            if (flag == -1)
                return (clean_exit(epoll_fd, serveur_fd, *data), 0);
            if (flag == 1)
            {
                data->erase(std::remove(data->begin(), data->end(), temp), data->end());
                close(temp->fd);
                delete(temp);
                temp = NULL;
            }
        }
        else if (flag == 0)
            return 0;
    }
    return 1;
}

// liste des choses a regler 
//un socket peut appartenir a plusieurs serveur et donc les socket doivent retenir les differents serveur pour lesquels elles ecoutent
//
