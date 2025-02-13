main: src/main.c src/array.c src/db.c
	cc -O3 -o codo src/db.c src/models.h src/array.c src/main.c -lncursesw -lsqlite3

d: src/main.c src/array.c src/db.c src/logger.c
	cc -g -Wall -Wextra -Werror -o codo src/db.c src/logger.c src/models.h src/array.c src/main.c -lncursesw -lsqlite3

install:
	cp codo /usr/local/bin/

test: tests/tests.c src/array.h
	cc -o test src/array.c tests/tests.c
	# ./test
	# rm test
