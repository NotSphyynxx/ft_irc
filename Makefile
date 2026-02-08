NAME = ft_irc

CXX = c++
CXXFLAGS = -std=c++98

SERVER_DIR = server
UTILS_DIR = ft_utils

SRC = main.cpp \
	$(SERVER_DIR)/Server.cpp \
	$(SERVER_DIR)/authentication.cpp \
	$(SERVER_DIR)/Client.cpp \
	$(SERVER_DIR)/parsing.cpp

HEAD = server/Server.hpp server/Client.hpp

OBJ = $(SRC:.cpp=.o)

INC = -I./ -I$(SERVER_DIR) -I$(UTILS_DIR)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEAD)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all