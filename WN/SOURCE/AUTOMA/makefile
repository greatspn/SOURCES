YACC = yacc
LEX = lex

OPTION = -Imeddley -O3 -g3 -Wall -c -fmessage-length=0


CC = gcc
C++ = g++

all: AutoParser.cpp AutoLexer.c AutoParser

clean: 
	rm AutoLexer.o AutoLexer.c  AutoParser.h AutoLexer.h AutoParser AutoParser.cpp

AutoParser: AutoParser.cpp AutoLexer.o
	$(C++) -o AutoParser AutoParser.cpp AutoLexer.o

AutoLexer.o: AutoLexer.c
	$(CC) -c  AutoLexer.c

AutoParser.cpp:  AutoParser.y parser.h
	$(YACC)  -d -o AutoParser.cpp AutoParser.y

AutoLexer.c : AutoLexer.l  
	$(LEX) -o AutoLexer.c  --header-file=AutoLexer.h AutoLexer.l

