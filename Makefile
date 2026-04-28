CXX				= c++
CXXFLAGS		= -Wall -Werror -Wextra -std=c++98
NAME			= webserv

SRC_DIR			= srcs
SRC				= main.cpp \
				  config/Config.cpp \
				  config/ConfigParser.cpp \
				  config/ConfigUtils.cpp \
				  config/ConfigValidator.cpp \
				  config/ConfigValidatorUtils.cpp \
				  cgi/CgiExecutor.cpp \
				  httpResponse/HttpResponse.cpp \
				  httpRequest/Request_parsing.cpp \
				  httpUpload/HttpUpload.cpp \
				  server/fill_request.cpp \
				  server/serveur_init.cpp \
				  server/handle_epoll_events.cpp \
				  server/loop_server.cpp \

OBJ_DIR			= objs
OBJ				= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all:			$(NAME)

$(NAME):		$(OBJ)
				$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
				mkdir -p $(dir $@)
				$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
				rm -rf $(OBJ_DIR)

fclean: clean
				rm -f $(NAME)

re:				fclean all

.PHONY:			all clean fclean re
