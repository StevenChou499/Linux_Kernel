NAME = test
CFILE = $(NAME).c

check:
	@gcc $(CFILE) -o $(NAME) -lpthread
	@./$(NAME) > output
	@python3 ./verify.py output
	@gcc $(NAME)_dyn.c -o $(NAME)_dyn -lpthread
	@./$(NAME) > valid.txt
	@python3 ./verify.py valid.txt

clean:
	@rm $(NAME) output valid.txt