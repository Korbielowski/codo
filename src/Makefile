main: main.c list.c db.c
	cc -o notes db.c models.h list.c main.c -lncursesw -lsqlite3
	./notes

test: tests.c list.h
	cc -o tests list.c tests.c
	./tests
