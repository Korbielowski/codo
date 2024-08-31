main: main.c array.c db.c
	cc -o notes db.c models.h array.c main.c -lncursesw -lsqlite3
	./notes

test: tests.c array.h
	cc -o tests array.c tests.c
	./tests
