

#include "../../inc/Webserv.hpp"

static void clear_connexions(std::vector<Connexion *> data)
{
    for (unsigned long i = 0; i < data.size(); i++)
    {
        if (data[i])
        {
            close(data[i]->fd);
            delete(data[i]);
            data[i] = NULL;
        }
    }
}

static unsigned long get_number(std::string str)
{
    int i = 0;
    while (str[i] &&str[i] != '\n')
        i++;
    str.erase(i);
    std::stringstream ss(str);
    int value = 0;
    ss >> value;
    return value;
}

void clean_exit(int epoll_fd, std::vector<int> serveur_fd, std::vector<Connexion *> data)
{
    clear_connexions(data);
    for (unsigned long i = 0; i < serveur_fd.size(); i++)
        close(serveur_fd[i]);
    close(epoll_fd);
    perror("Error");
}
void clean_exit_two(int epoll_fd, std::vector<int> &serveur_fd, std::vector<Connexion *> *data)
{
    clear_connexions(*data); 

    for (unsigned long i = 0; i < serveur_fd.size(); i++)
    {
        close(serveur_fd[i]);
    }

    close(epoll_fd);

    std::cout << "Serveur arrêté proprement.\n";
}






static int post_body(Connexion *temp, HttpRequest *stru, int fd)
{
    // printf("Appel de post_body\n");
    std::string str;
    //std::cout << " voici la requete :\n" << temp->req << std::endl;
    size_t body_start = temp->req.find("\r\n\r\n");
    if (body_start == std::string::npos)
        return 0;
    body_start += 4;
    if (temp->req.find("\nContent-Length:") != std::string::npos)
    {
        str = temp->req.substr(temp->req.find("Content-Length:") + 15);  
        stru->content_length = get_number(str);
        //std::cout <<"1: " <<  temp->req.length() << " 2: " << body_start << " 3: " << stru->content_length << std::endl;
        if (temp->req.length() >= body_start + stru->content_length)
        {
            // std::cout << "la vie\n";

            str = temp->req.substr(body_start, stru->content_length);
            if (!fill_req(stru, temp->req, fd))
                return 2;
            stru->body = str;
            // write(1,temp->req.c_str(), temp->req.length());
            return (1);
        }
        return (0);
    }    
    else if (temp->req.find("\nTransfer-Encoding:") != std::string::npos)
    {
        // printf("arthur 1\n");
        // --- CAS CHUNKED ---
        std::string chunks = temp->req.substr(body_start);
        std::string decoded;
        size_t pos = 0;

        while (true)
        {
            // trouver la ligne de la taille
            size_t line_end = chunks.find("\r\n", pos);
            if (line_end == std::string::npos) 
                return 0;

            // lire taille en hexa
            std::string hex_size = chunks.substr(pos, line_end - pos);
            unsigned long chunk_size = strtoul(hex_size.c_str(), NULL, 16);

            pos = line_end + 2; // passer "\r\n"

            if (chunk_size == 0)
            {
                // fin du body
                stru->body = decoded;
                stru->content_length = stru->body.length();
                if (!fill_req(stru, temp->req, fd))
                    return 2;
                return 1;
            }

            // vérifier si chunk complet reçu
            if (chunks.length() < pos + chunk_size + 2) // +2 pour le \r\n après data
                return (printf("mehdi 2\n"), 0);

            decoded.append(chunks.substr(pos, chunk_size));
            pos += chunk_size + 2; // data + "\r\n"
        }
    }
    else
    {
        send_msg_error(400, fd);
        return (2);
    }
    return (0);
}


// static void print_httpreq(HttpRequest stru)
// {
//     std::cout << "Voici la methode :" << stru.method << std::endl;
//     std::cout << "Voici l'url :" << stru.uri << std::endl;
//     std::cout << "Voici la version http :" << stru.http_version << std::endl;
//     std::cout << "Voici le body :" << stru.body << std::endl;
//     std::map<std::string, std::string>::const_iterator it;
//     for (it = stru.headers.begin(); it != stru.headers.end(); ++it)
//         std::cout << "Voici le nom du Headers :" << it->first << " Sa Valeur->" << it->second << std::endl;
// }

static int  mybuildResponse(HttpRequest stru, int  fd, std::vector<Config_Data> config_data, std::string listen, Connexion *temp)
{
    //std::cout << "La valeur du body :\n" << stru.body << "\n Fin de la valeur du body\n"<< std::endl;
    // std::cout << "la valeur de host : " << stru.headers["Host"] << std::endl;
    if (stru.headers["Host"].empty())
        return (send_msg_error(400, fd), 0);
    for (unsigned long i = 0; i < config_data.size(); i++)
    {
        if (stru.headers["Host"] == config_data[i].server_name)
        {
            for (int j = 0; i < config_data[i].listen.size(); j++)
            {
                if (listen == config_data[i].listen[j])
                    return(buildResponse(stru, fd, config_data[i], temp), 1);
            }
        }  
    }
    return (send_msg_error(404, fd), 0);
}

static int end_of_fd(Connexion *temp, int count, int fd, std::vector<Config_Data> config_data)
{
    HttpRequest *stru = new HttpRequest;
    stru->content_length = 0;
    // if (temp->req.length() > MAX_REQ)
    //     //RETURN the correct error value
    if (std::string::npos == temp->req.find("\r\n\r\n") || std::string::npos != temp->req.find("POST "))
    {
        if (std::string::npos != temp->req.find("POST ") && std::string::npos != temp->req.find("\r\n\r\n"))
        {
            int flag = post_body(temp, stru, fd);
            
            if (flag == 0)
               return (delete(stru), 0);
            else if (flag == 1)
                mybuildResponse(*stru, fd, config_data, temp->listen, temp);
            return (delete(stru), 1);
        }
        if (count == 0)
            return (send_msg_error(400, fd), delete(stru), 1);//request will not be completed
        return (delete(stru), 0);//REQUEST CAN BE COMPLETED
    }
    //parsing
    if (!fill_req(stru, temp->req, fd))
        return (delete(stru), 1);

    //creation de la reponse

    //write(1,temp->req.c_str(), temp->req.length());
    // print_httpreq(*stru);
    mybuildResponse(*stru, fd, config_data, temp->listen, temp);
    return (delete(stru), 1);
}



int request_controle(Connexion *temp, std::vector<Config_Data> config_data)
{
    char buffer[BUF_SIZE] = {0};
    int fd = temp->fd;
    int count;

    while (true)
    {
        // std::cout << "appel de read from " << fd << std::endl;
        count = read(fd, buffer, BUF_SIZE);
        //std::cout << count << std::endl;
        if (count > 0)
        {
            temp->req.append(buffer, count);
        }
        // else if (errno == EINTR)
        //     continue;
        // else if (errno == EAGAIN)
        //     return 0;
        if(temp->req.length() >= MAX_REQ)
        {
            int ret;
            while (true)
            {
                ret = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if (ret == 0)
                    break ;
                else if (ret < 0)
                    break ;
            }
            std::string response = buildErrorResponse(413);
            send(fd, response.c_str(), response.size(), 0);
            return 1;
        }
        else if (count >= 0)
            return end_of_fd(temp, count, fd, config_data);
        else
            return -1;
    }
    return -1;

    // while ((( count = read(fd, buffer, BUF_SIZE))> 0 || errno == EINTR) && temp->req.length() < MAX_REQ)
    //     temp->req.append(buffer, count);
    // std::cout << "valeur de count :" << count << std::endl;
    // std::cout << "valeur de errno :" << errno << std::endl;
    // if (count >= 0 || errno == EWOULDBLOCK)
    //     return end_of_fd(temp, count, fd, config_data);
    // return -1;
}


// while (true) 
//     {
//         count = read(fd, buffer, BUF_SIZE);
//         if (count > 0)
//             temp->req.append(buffer, count);
//         else if (count == 0) 
//             return end_of_fd(temp, count, fd, config_data);
//         else 
//         {
//             if (errno == EINTR) 
//                 continue;
//             if (errno == EAGAIN || errno == EWOULDBLOCK) 
//                 break; 
//             return -1;
//         }
//     }
//     return -1;
//     }