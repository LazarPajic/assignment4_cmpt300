FLAGS = -g -Wall -o
LEAKFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes
TEST1 = ./myls -l -iR -lRi test.txt a.txt b.txt
all: myls

cshell: myls.c
	gcc $(FLAGS) myls myls.c

leak:
	valgrind $(LEAKFLAGS) $(TEST1)
	
test:
	$(TEST1)

clean: 
	rm -rf *.o myls
