#directories
BINDIR=.
CURDIR=.
CC  =/usr/bin/gcc
INC =../bdd/include
LIB =../bdd/lib
LEX_LIB =bdd
LIBSPOT= /root/SPOT/greatspn/i686_R2.4.21-0.13mdk/2bin/lib
#commands

COMP   = $(CC) -g
CFLAG  = -o
LFLAGS = -lgspnSRG  -lm -lfl 
YACC   = bison -v -p Prop
YACFLAG= -d
LEX    = flex -i -PProp
LEXFLAG= -o
LIBFLAG =-L$(LIBSPOT)

T_LEX  = ${CURDIR}/PROPL.c  

T_YACC = ${CURDIR}/PROPG.tab.c 

T_VECT = ${CURDIR}/Vectors.c

T_SYM  =  ${CURDIR}/FSymFmTokens2.c

T_MUL =   ${CURDIR}/MultiSetOp.c
#general rules

$(BINDIR)/PROPOS :  ${T_YACC} ${T_LEX}
	${COMP} ${LIBFLAG} ${CFLAG} $(BINDIR)/PROPS -I${INC}  ${T_YACC} ${T_LEX} ${T_VECT} ${T_SYM} ${T_MUL}  $(LFLAGS) 

${CURDIR}/PROPL.c : ${CURDIR}/PROPL.l
	${LEX} ${LEXFLAG}${CURDIR}/PROPL.c ${CURDIR}/PROPL.l

${CURDIR}/PROPG.tab.c :  ${CURDIR}/PROPG.y
	${YACC} ${YACFLAG} ${CURDIR}/PROPG.y


