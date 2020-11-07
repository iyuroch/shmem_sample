main: main.c
	cc main.c -o main

debug: main.c
	cc main.c -o main -DDEBUG

clean:
	rm -f main
