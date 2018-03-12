#directories
OBJ = ../../../$(GSPN2BINS)/remove_obj
BIN = ../../../$(GSPN2BINS)

#commands
COMP = $(CC) -O3 -w
LINK = $(CC) -O3 -w

LFLAG = -lm -l$(LEX_LIB)
# -ly

#files groups
EXECUTABLE = remove
INCLUDES = global.h \
	   alloc.h \
	   layer.h \
	   save.h
OBJECTS = ${OBJ}/y.tab.o \
	  ${OBJ}/lex.yy.o \
	  ${OBJ}/global.o \
	  ${OBJ}/alloc.o \
	  ${OBJ}/layer.o \
	  ${OBJ}/save.o \
	  ${OBJ}/load.o \
	  ${OBJ}/remove.o
SOURCES = y.tab.c \
	  lex.yy.c \
	  global.c \
	  alloc.c \
	  layer.c \
	  save.c \
	  load.c \
	  remove.c
LEX_FILE = lex.lex
YACC_FILE= yacc.yacc

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




