hy345sh.o: hy345sh.c
	gcc -c hy345sh.c

hy345sh: hy345sh.o
	gcc -o hy345sh hy345sh.o

all: hy345sh

clean:
	rm hy345sh
	rm hy345sh.o