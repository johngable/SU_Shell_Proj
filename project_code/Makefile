all: sush

test: test.c environ.c list.c
	gcc -o test test.c environ.c list.c -ggdb

sush: *.c *.h
	gcc -o sush *.c -lm -ggdb

clean:
	rm sush test *.txt