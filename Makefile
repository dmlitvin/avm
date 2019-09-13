# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dmlitvin <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/02/06 22:47:34 by dmlitvin          #+#    #+#              #
#    Updated: 2019/05/25 15:05:02 by dmlitvin         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=avm

FLAGS=-Wall -Wextra -Werror -std=c++11

CC=clang++ $(FLAGS)

SRC=main.cpp Lexer.cpp AVM.cpp

SRO=$(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(SRO)
	@$(CC) $(SRO) -o $(NAME) && printf "\x1b[32mBinary file compiled \
	succesfully!\nLaunch: ./$(NAME) < \"source_file\"\n\x1b[0m"

$(SRO): $(SRC) AVM.hpp Operand.hpp
	@$(CC) -c $(SRC) && printf "\x1b[32mObject files compiled succesfully!\n\x1b[0m"

clean:
	@rm -f $(SRO) && printf "\x1b[31mObject files have been deleted!\n\x1b[0m"

fclean: clean
	@rm -f $(NAME) && printf "\x1b[31mBinary file has been deleted!\n\x1b[0m"

re: fclean all

.PHONY: re clean fclean all
