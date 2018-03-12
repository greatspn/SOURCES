#directories
OBJ = ../../../$(GSPN2BINS)/algebra_obj
BIN = ../../../$(GSPN2BINS)

#commands
COMP = $(CC) -O2 -w
LINK = $(CC) -O2 -w

LFLAG = -lm -l$(LEX_LIB) 
#-l$(YACC_LIB)

#files groups
EXECUTABLE = algebra
INCLUDES = global.h \
	   alloc.h \
	   layer.h \
	   rescale.h \
	   save.h
OBJECTS = ${OBJ}/y.tab.o \
	  ${OBJ}/lex.yy.o \
	  ${OBJ}/global.o \
	  ${OBJ}/alloc.o \
	  ${OBJ}/layer.o \
	  ${OBJ}/save.o \
	  ${OBJ}/load.o \
	  ${OBJ}/rescale.o \
	  ${OBJ}/algebra.o
SOURCES = y.tab.c \
	  lex.yy.c \
	  global.c \
	  alloc.c \
	  layer.c \
	  save.c \
	  load.c \
	  rescale.c \
	  algebra.c
LEX_FILE = lex.lex
YACC_FILE = yacc.yacc

#general rules
$(BIN)/$(EXECUTABLE) : $(OBJECTS) $(SOURCES) $(INCLUDES)
	$(LINK) $(OBJECTS) -o $@ $(LFLAG)

$(OBJ)/%.o : %.c
	$(COMP) -c $<  -o $@

y.tab.c: yacc.yacc
	$(YACC) -d yacc.yacc

lex.yy.c: lex.lex
	$(LEX) lex.lex

clean:
	rm y.tab.c
	rm lex.yy.c
	rm $(OBJ)/*.o
	rm $(BIN)/$(EXECUTABLE)









