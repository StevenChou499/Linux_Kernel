NAME = test
CFILE = $(NAME).c

check:
	@gcc $(CFILE) -o $(NAME) -lpthread
	@./$(NAME)

clean:
	@rm $(NAME)