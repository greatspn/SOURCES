#directories
OBJ =   ../../../$(GSPN2BINS)/2bin
BINDIR =  ../../../$(GSPN2BINS)

#commands
COMP = $(CC) -g $(CFLAGS)
LINK = $(CC) $(OFLAGS)

#files groups

#general rules
sol : compact.o ${BINDIR}/swn_ntrs ${BINDIR}/swn_ggsc ${BINDIR}/swn_stndrd ${BINDIR}/swn_gst_stndrd $(BINDIR)/swn_gst_prep 

compact.o : compact.c 
	${COMP} compact.c

${BINDIR}/swn_ntrs : swn_ntrs.c compact.o compact.h
	${LINK} ${BINDIR}/swn_ntrs swn_ntrs.c compact.o

${BINDIR}/swn_ggsc : swn_ggsc.c compact.o compact.h
	${LINK} ${BINDIR}/swn_ggsc swn_ggsc.c compact.o

${BINDIR}/swn_stndrd : swn_stndrd.c compact.o compact.h
	${LINK} ${BINDIR}/swn_stndrd swn_stndrd.c compact.o

${BINDIR}/swn_gst_stndrd : swn_gst_stndrd.c compact.o compact.h
	${LINK} ${BINDIR}/swn_gst_stndrd swn_gst_stndrd.c compact.o

$(BINDIR)/swn_gst_prep : gre_stndrd.c res_stndrd.lex res_stndrd.yacc
	$(LEX) res_stndrd.lex
	$(YACC) res_stndrd.yacc
	${COMP} y.tab.c
	$(LINK) $(BINDIR)/swn_gst_prep y.tab.c -l$(LEX_LIB)
	/bin/rm -f lex.yy.c y.tab.c *.o
