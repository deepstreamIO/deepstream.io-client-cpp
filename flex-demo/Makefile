CC       := cc
CXX      := c++
CFLAGS   := -Wextra -Wall -std=c99 -pedantic -O2 -g -DNDEBUG
CXXFLAGS := -Wextra -Wall -std=c++11 -pedantic -O2 -g -DNDEBUG
LEX_CFLAGS := -Wno-unused-function -Wno-unused-parameter


all: a.out

a.out: lex.yy.o main.o parser.o
	$(CXX) $(CXXFLAGS) lex.yy.o main.o parser.o

lex.yy.o: lex.yy.h lex.yy.c
	$(CC) $(CFLAGS) $(LEX_CFLAGS) -c lex.yy.c

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $<

parser.o: parser.cpp
	$(CXX) $(CXXFLAGS) -c $<


lex.yy.h lex.yy.c: lexer.l
	flex --header-file=lex.yy.h lexer.l


.PHONY: clean all

clean:
	$(RM) lex.yy.h lex.yy.c
	$(RM) lex.yy.o main.o parser.o
