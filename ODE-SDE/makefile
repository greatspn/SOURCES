
OPTION = -g -std=c++11 -lfl  -lm
OPTIONW = -Wall

CC = gcc
C++ = g++
YACC = yacc
LEX  = flex++

all:	automa normal

clean:
	rm  $(name_file).solver *.o y.tab.h

automa:	y.tab.o lex.yy.o $(name_file).cpp
	$(C++)  -c lex.yy.cc  $(OPTION)
	$(C++)  -c y.tab.c  $(OPTION)
	$(C++) -D AUTOMATON -I./ -o $(name_file).solver lex.yy.o y.tab.o $(name_file).cpp ./class.cpp  ./lsode.cpp ./automa.cpp $(OPTION) $(OPTIONW)

lex.yy.o:	readingAutomaton.ll
	$(LEX)  -+ readingAutomaton.ll

y.tab.o:	 readingAutomaton.yy
	$(YACC) -v -d readingAutomaton.yy

normal:	$(name_file).cpp ./class.cpp
	 $(C++) -I./  -o  $(name_file).solver  $(name_file).cpp  ./class.cpp  ./lsode.cpp $(OPTION) $(OPTIONW)
doc:
	doxygen  DoxyFile
