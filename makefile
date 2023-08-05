FLAGS = -g -Wall -o

all: myls

myls: myls.c
	gcc $(FLAGS) myls myls.c

clean: 
	rm -rf *.o myls
