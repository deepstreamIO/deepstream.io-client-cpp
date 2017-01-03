CC := gcc
CFLAGS := -Wextra -Wall -std=c99 -pedantic -g
LEX_CFLAGS := -Wno-unused-function -Wno-unused-parameter


all: a.out

a.out: lex.yy.h lex.yy.c
	$(CC) $(CFLAGS) $(LEX_CFLAGS) lex.yy.c


lex.yy.h lex.yy.c: lexer.l
	flex --header-file=lex.yy.h lexer.l


.PHONY: clean all

clean:
	$(RM) lex.yy.h lex.yy.c
