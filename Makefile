NAME = ft_irc
BOT_NAME = bot

CXX = c++
CXXFLAGS = -std=c++98

SERVER_DIR = server
UTILS_DIR = ft_utils

SRC = main.cpp \
	$(SERVER_DIR)/Server.cpp \
	$(SERVER_DIR)/authentication.cpp \
	$(SERVER_DIR)/Client.cpp \
	$(SERVER_DIR)/parsing.cpp \
	$(SERVER_DIR)/Commands.cpp\
	$(SERVER_DIR)/GettersSetters.cpp \
	$(SERVER_DIR)/Channel.cpp

B_SRC = bonus/bot.cpp server/parsing.cpp


HEAD = server/Server.hpp server/Client.hpp server/Channel.hpp

OBJ = $(SRC:.cpp=.o)
B_OBJ = $(B_SRC:.cpp=.o)


INC = -I./ -I$(SERVER_DIR) -I$(UTILS_DIR)

all: $(NAME)

bonus : $(BOT_NAME)

$(BOT_NAME): $(B_OBJ)
	$(CXX) $(CXXFLAGS) $(B_OBJ) -o $(BOT_NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEAD)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
