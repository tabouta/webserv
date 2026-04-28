


#include "../inc/Webserv.hpp"
#include "httpResponse/HttpResponse.hpp"
#include "server/Server.hpp"


volatile sig_atomic_t g_stop = 0;


void handle_sigint(int sig)
{
    if (sig == SIGINT)
    {
        std::cout << "\nSIGINT reçu (CTRL+C), arrêt du serveur demandé.\n";
        g_stop = 1;
    }
}

void setup_signal_handling()
{
    signal(SIGINT, handle_sigint);
}


int main(int ac, char** av, char** env) 
{
    if (ac != 2) {
        std::cerr << "No config file" << std::endl;
        return 1;
    } 

    setup_signal_handling();
    ////////////////////////////
    std::vector<Config_Data> config_data;
    Config config(av[1]);
    if (config.parseFile(config_data, env)) {
        std::cerr << "Couldn't read config file" << std::endl;
        return 0;
    }
    // std::cout << "Numbers of servers: " << config_data.size() << std::endl;
    std::vector< int > serveur_fd;
    std::map<int, std::string> fd_to_listen;
    int epoll_fd;
    struct epoll_event ev;

    epoll_fd = epoll_create1(0);

    for(unsigned long j = 0; j < config_data.size(); j++)
    {
        for (unsigned long i = 0; i < config_data[j].listen.size();i++)
        {
            int temp = create_serveur(config_data[j].listen[i], serveur_fd, config_data);
            if (!temp)
                return 0;
            serveur_fd.push_back(temp);
            fd_to_listen[temp] = config_data[j].listen[i];
            ev.events = EPOLLIN;//ce mot cle detecte les tentatives de connexion au serveur
            ev.data.fd = temp;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev);
        }
    }
    if (config_data.size() == 0)
        return (printf("No serveur\n"), 0);
    std::vector<Connexion *> data;
    // while (1)
    // {
    //     if (!loop(epoll_fd, serveur_fd, config_data, fd_to_listen, &data))
    //         return 0;
    // }
    while (!g_stop)
    {
        if (!loop(epoll_fd, serveur_fd, config_data, fd_to_listen, &data))
            return 0;
    }

    std::cout << "Arrêt du serveur...\n";
    clean_exit_two(epoll_fd, serveur_fd, &data);
        
    return 0;
}