


#include "../../inc/Webserv.hpp"


void send_msg_error(int nerr, int fd)
{
    std::string response = buildErrorResponse(nerr);
    send(fd, response.c_str(), response.size(), 0);
}

int parsing_req(HttpRequest *stru)
{
    if (stru->method != "GET" && stru->method != "DELETE" && stru->method != "POST" )
        return 0;
    if (stru->uri.length() < 1 || stru->http_version.length() < 1)
        return 0;
    if (stru->headers["Host"].empty())
        return 0;
    
    return 1;
}

static void erase_spaces(std::string &line)
{
    int i = 0;
    // char c = line[i];
    while (line[i] && (line[i] == 32 || (line[i] > 6 && line[i] < 14)))
        i++;
    line.erase(0, i);
    if (line.empty())
        return ;
    i = line.length() - 1;
    // c = line[i];
    while (line[i] == 32 || (line[i] > 6 && line[i] < 14))
        i--;
    line.erase(i + 1);
}
static int size_w(std::string word)
{
    int i = 0;

    while (word[i] && !(word[i] == 32 || (word[i] > 6 && word[i] < 14)))
        i++;
    return i;
}
static std::string get_nth_word(std::string line, int n)
{
    int current = 1;
    int i = 0;

    while (line[i] && (line[i] == 58 || line[i] == 32 || (line[i] > 6 && line[i] < 14)))
        i++;
    if (n == 1 && line[i])
    {

        line.erase(0, i);

        line.erase(size_w(line));
        return(line);
    }
    while (line[i])
    {
        if ( (line[i] == 58 || line[i] == 32 || (line[i] > 6 && line[i] < 14)))
        {
            current++;
            while (line[i] && (line[i] == 58 || line[i] == 32 || (line[i] > 6 && line[i] < 14)))
                i++;
            if (!line[i])
                return "";
            if (current == n)
            {
                line.erase(0, i);;
                line.erase(size_w(line));
                return(line);
            }
        }
        i++;
    }
    return "";
}
static std::string get_nth_line(std::string& req, int n)
{
    std::istringstream iss(req);
    std::string line;
    int i = 0;

    if (n < 1 || n > MAX_REQ)
        return "";
    while (std::getline(iss, line))
    {
        if (++i == n)
            return line;
    }
    return "";
}

static int check_spaces(std::string line)
{
    int i = 0;

    while (line[i] && !(line[i] == 32 || (line[i] > 6 && line[i] < 14)))
        i++;
    if (line[i])
        i++;
    if (!line[i] || line[i] == 32 || (line[i] > 6 && line[i] < 14))
        return 0;

    while (line[i] && !(line[i] == 32 || (line[i] > 6 && line[i] < 14)))
        i++;
    if (line[i])
        i++;
    if (!line[i] || line[i] == 32 || (line[i] > 6 && line[i] < 14))
        return 0;

    while (line[i] && !(line[i] == 32 || (line[i] > 6 && line[i] < 14)))
        i++;
    // std::cout << "valeur de line[i - 1] : {" << line[i] <<"}" << std::endl;
    if (line[i] != '\r')
        return 0;

    return 1;
}

int static check_httpversion(std::string str)
{
    int i = 0;
    std::string http("HTTP");
    while(str[i] && i < 4)
    {
        if (str[i] != http[i])
            return 0;
        i++;
    }
    if (!str[i] || str[i] != '/')
        return 0;
    i++;
    if (!str[i] || !(str[i] <= '9' && str[i] >= '0'))
        return 0;
    i++;
    if (!str[i] || str[i] != '.')
        return 0;
    i++;
    if (!str[i] || !(str[i] <= '9' && str[i] >= '0'))
        return 0;
    i++; 
    return (!str[i]);

}


int fill_req(HttpRequest *stru, std::string req, int fd)
{
    std::string line;
    std::string word;
    int i = 2;

    if (req.empty())
        return (0);
    line = get_nth_line(req, 1);
    if (!check_spaces(line))
        return (send_msg_error(400, fd), 0);

    stru->method = get_nth_word(line, 1);
    stru->uri = get_nth_word(line, 2);
    stru->http_version = get_nth_word(line, 3);

    if (!check_httpversion(stru->http_version))
        return (send_msg_error(505, fd), 0);

    line = get_nth_line(req, 2);
    while ((word = get_nth_word(line, 1)) != "")
    {
        word.erase(word.length() - 1);//on retire le ':'
        line.erase(0, word.length() + 1);
        erase_spaces(line);
        stru->headers[word] = line;//Donc toute la ligne sans le mot
        line = get_nth_line(req, ++i);
    }
    i++;
    if (stru->method == "POST")
    {
        while ((line = get_nth_line(req, i)) != "")
        {
            stru->body.append(line);
            stru->body.append("\n");
            i++;
        }
    }
    if (!parsing_req(stru))
        return (send_msg_error(400, fd), 0);
    return 1;
}

// int main()
// {
//     HttpRequest stru;
//     std::string req;

//     //erase_spaces(req);
//     //std::cout << req << std::endl;
//     req.append("POST /api/utilisateur HTTP/1.1\r\nHost: www.exemple.com\r\nUser-Agent: CustomClient/1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 79\r\nConnection: keep-alive\r\n\r\nnom=JeanDupont&age=42&ville=Marseille&email=jean.dupont%40mail.com&profession=ingenieur\r\nje rajoute des trucs au hasard\r\nmais bon c'est juste pour verifier que ca marche bien quoi\r\n\r\n");
//     fill_req(&stru, req, 1);
//     std::cout << req;
//     std::cout << "valeur method : " << stru.method << std::endl;
//     std::cout << "valeur url : " << stru.uri << std::endl;
//     std::cout << "valeur http_version : " << stru.http_version << std::endl;
//     std::map<std::string, std::string>::const_iterator it;
//     for (it = stru.headers.begin(); it != stru.headers.end(); ++it)
//         std::cout << it->first << ": " << it->second << std::endl;
//     std::cout << "\r\n" << "Body : " << stru.body << std::endl;
    
// }

