all:
	gcc main.c -o main
	clear

run:
	./main

clean:
	rm -rf *.o main

make start:
	make clean
	make
	clear
	make run
