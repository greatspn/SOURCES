
OPTION = -O3 -std=c++11 -lm
OPTIONW = -Wall
OPTIONGLPK=  -lglpk

CC = gcc
C++ = g++
YACC = yacc
LEX  = flex++

all:	automa normal

clean:
	rm  $(name_file).solver *.o y.tab.h

automa:	y.tab.o lex.yy.o $(name_file).cpp ./lsode.cpp ./automa.cpp ./readSupport.cpp
	$(C++)  -c lex.yy.cc  $(OPTION)
	$(C++)  -c y.tab.c  $(OPTION)
	$(C++) -D AUTOMATON -I./ -o $(name_file).solver lex.yy.o y.tab.o $(name_file).cpp ./class.cpp  ./lsode.cpp ./automa.cpp ./readSupport.cpp   $(OPTION) $(OPTIONW)

lex.yy.o:	readingAutomaton.ll
	$(LEX)  -+ -P mm readingAutomaton.ll

y.tab.o:	 readingAutomaton.yy
	$(YACC) -v -d readingAutomaton.yy

normal:	$(name_file).cpp ./class.cpp  ./lsode.cpp
	 $(C++) -I./  -o  $(name_file).solver  $(name_file).cpp  ./class.cpp  ./lsode.cpp ./readSupport.cpp  $(OPTION) $(OPTIONW)
	 
fluxb:	$(name_file).cpp ./class.cpp  ./lsode.cpp ./GLPKsolve.cpp ./general.cpp ./readSupport.cpp
	 $(C++) -D CGLPK -I./  -o  $(name_file).solver  $(name_file).cpp  ./class.cpp  ./lsode.cpp ./GLPKsolve.cpp ./general.cpp ./readSupport.cpp $(OPTION) $(OPTIONW) $(OPTIONGLPK)
	 	 
doc:
	doxygen  DoxyFile
