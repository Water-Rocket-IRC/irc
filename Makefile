# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jinwoole <indibooks@naver.com>             +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/01/27 10:57:54 by sesim             #+#    #+#              #
#    Updated: 2023/01/29 15:49:46 by jinwoole         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX			=	c++
CXXFLAGS	=	-g3 #-fsanitize=address
RM			=	rm -rf

OBJS_DIR	:=	objs/
DEPS_DIR	:=	deps/

# SRCS		:=	$(wildcard *.cpp)
SRCS		:=	main.cpp
OBJS		:=	$(addprefix $(OBJS_DIR), $(SRCS:.cpp=.o))
DEPS		:=	$(addprefix $(DEPS_DIR), $(SRCS:.cpp=.d))
NAME		:=	ircserv

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
# $(CXX) $^ -o $@

$(OBJS_DIR)%.o : %.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(DEPS_DIR)
	$(CXX) $(CXXFLAGS) $< -c -o $@ -MMD
# $(CXX) $< -c -o $@ -MMD
	mv $(@:.o=.d) $(DEPS_DIR)

clean:
	$(RM) $(OBJS_DIR)
	$(RM) $(DEPS_DIR)

fclean: clean
	$(RM) $(NAME)

re: | fclean all

-include $(DEPS)

.PHONY: all clean fclean re