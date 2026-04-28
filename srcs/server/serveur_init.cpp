
#include "../../inc/Webserv.hpp"

static unsigned long my_inet_addr(const std::string &ip)
{
    unsigned int a,b,c,d;
    char dot;
    std::istringstream iss(ip);
    if (!(iss >> a >> dot >> b >> dot >> c >> dot >> d))
    {
        std::cerr << "Erreur parsing IP: " << ip << std::endl;
        return INADDR_NONE; // 0xFFFFFFFF
    }
    if (a>255 || b>255 || c>255 || d>255)
    {
        std::cerr << "Valeur d'octet hors plage dans l'IP: " << ip << std::endl;
        return INADDR_NONE;
    }
    unsigned long ip_host = (a << 24) | (b << 16) | (c << 8) | d;
    return htonl(ip_host);
}

static int  sock_create(int port,  std::vector<int> vecto_fd, int *serveur_fd)
{
    int opt = 1;
    if (port > 65535)
    {
        for (unsigned long i = 0; i < vecto_fd.size(); i++)
            close(vecto_fd[i]);
        perror("port out of range");
       return 0;
    }
    // Création du socket local, celui du serveur
    if ((*serveur_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        for (unsigned long i = 0; i < vecto_fd.size(); i++)
            close(vecto_fd[i]);
        perror("socket failed\n");
       return 0;
    }
    // Definition de l'adresse de la socket comme re utilisable instantanement !!
    if (setsockopt(*serveur_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)) < 0) {
        close(*serveur_fd);
        for (unsigned long i = 0; i < vecto_fd.size(); i++)
            close(vecto_fd[i]);
        perror("setsockopt\n");
       return 0;
    }
    return 1;
}

static int fill_adrress(struct sockaddr_in *address, std::string ip_str, int serveur_fd,  std::vector<int> vecto_fd, int port)
{
    address->sin_family = AF_INET;
    //si l'adresse ip est vide on accepte sur toutes les adresses
    if (ip_str == "0.0.0.0" || ip_str.empty())
        address->sin_addr.s_addr = htonl(INADDR_ANY);
    else
    {
        //sinon on attribue l'adresse ip a notre adresse
        unsigned long addr_bin = my_inet_addr(ip_str);
        if (addr_bin == INADDR_NONE)
        {
            close(serveur_fd);
            for (unsigned long i = 0; i < vecto_fd.size(); i++)
                close(vecto_fd[i]);
            perror("address is not good\n");
           return 0;
        }
        address->sin_addr.s_addr = addr_bin;
    }
    address->sin_port = htons(port);
    return 1;
}

static int finish_serveur(struct sockaddr_in address, int serveur_fd, std::vector<int> vecto_fd)
{
    //bind the socket to the serveur adress
    if (bind(serveur_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        close(serveur_fd);
        for (unsigned long i = 0; i < vecto_fd.size(); i++)
            close(vecto_fd[i]);
        perror("bind failed\n");
       return 0;
    }
    //now we have the fd of the socket of our server we adapt him so that read and write are non blocking !!
    int flags = fcntl(serveur_fd, F_GETFL, 0);
    if (fcntl(serveur_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        close(serveur_fd);
        for (unsigned long i = 0; i < vecto_fd.size(); i++)
            close(vecto_fd[i]);
        perror("Erreur fcntl\n");
       return 0;
    }
    //We start to listen with the serveur
    if (listen(serveur_fd, 1000) < 0)
    {
        close(serveur_fd);
        for (unsigned long i = 0; i < vecto_fd.size(); i++)
            close(vecto_fd[i]);
        perror("Error listen\n");
       return 0;
    }
    return 1;
    // std::cout << "le serveur " << serveur_fd << "ecoute maintenant grace a listen \n";
}

int create_serveur(std::string list, std::vector<int> vector_fd, std::vector<Config_Data> config_file)
{
    // printf("appel de create serveur\n");
    struct sockaddr_in address;
    int serveur_fd;
    std::string ip_str;
    std::string port_str;
    int port;
    (void )config_file ;
    ip_str = list.substr(0, list.find(":"));
    port_str = list.substr(list.find(":") + 1);
    port = std::atoi(port_str.c_str());



    if (!sock_create(port, vector_fd, &serveur_fd))
        return 0;
    if (!fill_adrress(&address, ip_str, serveur_fd, vector_fd, port))
        return 0;
    if (!finish_serveur(address, serveur_fd, vector_fd))
        return 0;
    return serveur_fd;
}



