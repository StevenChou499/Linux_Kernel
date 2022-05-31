NAME = test
CFILE = $(NAME).c

check:
	@gcc $(CFILE) -o $(NAME) -lpthread
	@./$(NAME) > output
	@python3 ./verify.py

clean:
	@rm $(NAME) output