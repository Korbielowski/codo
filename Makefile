main: src/main.c src/array.c src/db.c
	cc -g -o codo src/db.c include/models.h src/array.c src/main.c -lncursesw -lsqlite3
	#./codo

test: tests/tests.c include/array.h
	cc -o test src/array.c tests/tests.c
	./test
	rm test
