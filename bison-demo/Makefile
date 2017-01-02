a.out: parser.tab.h parser.tab.c main.c
	gcc -Wextra -Wall -std=c99 -pedantic parser.tab.c main.c


parser.tab.c: parser.y
	~/bin/bison -d parser.y

parser.tab.h: parser.y
	~/bin/bison -d parser.y


.PHONY: clean

clean:
	$(RM) parser.tab.c parser.tab.h
