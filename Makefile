# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sesim <sesim@student.42seoul.kr>           +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/01/27 10:57:54 by sesim             #+#    #+#              #
#    Updated: 2023/02/10 23:00:22 by sesim            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CXX			=	c++
CXXFLAGS	=	-g3 -Wall -Wextra -Werror -std=c++98

RM			=	rm -rf

OBJS_DIR	:=	objs/
DEPS_DIR	:=	deps/

SRCS		:=	$(wildcard *.cpp)
OBJS		:=	$(addprefix $(OBJS_DIR), $(SRCS:.cpp=.o))
DEPS		:=	$(addprefix $(DEPS_DIR), $(SRCS:.cpp=.d))
NAME		:=	ircserv


all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJS_DIR)%.o : %.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(DEPS_DIR)
	$(CXX) $(CXXFLAGS) $< -c -o $@ -MMD
	mv $(@:.o=.d) $(DEPS_DIR)

clean:
	$(RM) $(OBJS_DIR)
	$(RM) $(DEPS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean 
	@make all

-include $(DEPS)

.PHONY: all clean fclean re