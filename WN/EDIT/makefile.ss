#directories
CSOU = SOURCE/SHARED
SSOU = SOURCE/SIMULAT
RSOU = SOURCE/READNET
EDT = EDIT
SFK = SUPPORT
OBJ = OBJ
INC = INCLUDE
BCK = BACKUP
TRS = TRANSL
OBJ = ../../$(GSPN2BINS)/2bin
BINDIR=../../$(GSPN2BINS)


#commands
COMP = $(CC) $(CFLAGS) -o
LINK = $(CC) $(OFLAGS)
LFLAGS = -lm -l$(LEX_LIB) $(SOCKET_LIB)




#files groups
EXECUTABLES  =	WNSYMB
T_INCLUDES = ${INC}/const.h \
	     ${INC}/struct.h \
	     ${INC}/var_ext.h \
	     ${INC}/decl.h \
	     ${INC}/fun_ext.h \
	     ${INC}/ealloc.h \
	     ${INC}/macros.h
T_OBJECTS = ${OBJ}/ealloc.o \
	    ${OBJ}/SSSYMBOLIC_grammar.o \
	    ${OBJ}/SSSYMBOLIC_service.o \
	    ${OBJ}/SSSYMBOLIC_token.o \
	    ${OBJ}/SSSYMBOLIC_dimensio.o \
	    ${OBJ}/SSSYMBOLIC_errors.o \
	    ${OBJ}/SSSYMBOLIC_common.o \
	    ${OBJ}/SSSYMBOLIC_report.o \
	    ${OBJ}/SSSYMBOLIC_enabling.o \
	    ${OBJ}/SSSYMBOLIC_fire.o\
	    ${OBJ}/SSSYMBOLIC_shared1.o\
	    ${OBJ}/SSSYMBOLIC_shared2.o\
	    ${OBJ}/SSSYMBOLIC_outdom.o\
	    ${OBJ}/SSSYMBOLIC_precheck.o \
	    ${OBJ}/SSSYMBOLIC_flush.o \
	    ${OBJ}/SSSYMBOLIC_degree.o \
	    ${OBJ}/SSSYMBOLIC_split.o \
	    ${OBJ}/SSSYMBOLIC_group.o \
	    ${OBJ}/SSSYMBOLIC_increm.o\
	    ${OBJ}/SSSYMBOLIC_engine.o \
	    ${OBJ}/SSSYMBOLIC_random.o \
	    ${OBJ}/SSSYMBOLIC_stat.o \
	    ${OBJ}/SSSYMBOLIC_distribu.o \
	    ${OBJ}/SSSYMBOLIC_eng_wn.o \
	    ${OBJ}/SSSYMBOLIC_lists.o \
	    ${OBJ}/SSSYMBOLIC_presel.o\
	    ${OBJ}/SSSYMBOLIC_after.o\
	    ${OBJ}/SSSYMBOLIC_read_arc.o \
	    ${OBJ}/SSSYMBOLIC_read_t_c.o \
	    ${OBJ}/SSSYMBOLIC_read_DEF.o \
	    ${OBJ}/SSSYMBOLIC_read_NET.o \
	    ${OBJ}/SSSYMBOLIC_read_t_s.o \
	    ${OBJ}/SSSYMBOLIC_wn_yac.o
T_SOURCES = ${CSOU}/service.c \
	    ${CSOU}/ealloc.c \
	    ${CSOU}/token.c \
	    ${CSOU}/dimensio.c \
	    ${CSOU}/errors.c \
	    ${CSOU}/common.c \
	    ${CSOU}/enabling.c	\
	    ${CSOU}/fire.c\
	    ${CSOU}/shared1.c\
	    ${CSOU}/shared2.c\
	    ${CSOU}/outdom.c\
	    ${CSOU}/report.c\
	    ${CSOU}/precheck.c \
	    ${CSOU}/flush.c \
	    ${CSOU}/degree.c \
	    ${CSOU}/split.c \
	    ${CSOU}/group.c \
	    ${SSOU}/eng_wn.c \
	    ${SSOU}/lists.c \
	    ${SSOU}/engine.c \
	    ${SSOU}/distribu.c \
	    ${SSOU}/increm.c\
	    ${SSOU}/presel.c\
	    ${SSOU}/random.c\
	    ${SSOU}/stat.c\
	    ${SSOU}/after.c\
	    ${RSOU}/read_arc.c \
	    ${RSOU}/read_t_c.c \
	    ${RSOU}/read_DEF.c \
	    ${RSOU}/read_NET.c \
	    ${RSOU}/read_t_s.c \
	    ${RSOU}/wn_yac.c
T_LEX = ${TRS}/wn.lex
T_YACC = ${TRS}/wn.yac

#general rules
#${EXECUTABLES} : ${T_INCLUDES} ${T_LEX} ${T_YACC} {T_SOURCES}

$(BINDIR)/WNSYMB : ${T_INCLUDES} ${T_OBJECTS} ${T_LEX} ${T_YACC} ${CSOU}/main.c
	${LINK} $(BINDIR)/WNSYMB ${CSOU}/main.c ${T_OBJECTS} $(LFLAGS)

${OBJ}/SSSYMBOLIC_outdom.o : ${CSOU}/outdom.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_outdom.o ${CSOU}/outdom.c

${OBJ}/SSSYMBOLIC_shared1.o : ${CSOU}/shared1.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_shared1.o ${CSOU}/shared1.c

${OBJ}/SSSYMBOLIC_shared2.o : ${CSOU}/shared2.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_shared2.o ${CSOU}/shared2.c

${OBJ}/SSSYMBOLIC_precheck.o : ${CSOU}/precheck.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_precheck.o ${CSOU}/precheck.c

${OBJ}/SSSYMBOLIC_fire.o : ${CSOU}/fire.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_fire.o ${CSOU}/fire.c

${OBJ}/SSSYMBOLIC_degree.o : ${CSOU}/degree.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_degree.o ${CSOU}/degree.c

${OBJ}/SSSYMBOLIC_report.o : ${CSOU}/report.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_report.o ${CSOU}/report.c
       
${OBJ}/SSSYMBOLIC_flush.o : ${CSOU}/flush.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_flush.o ${CSOU}/flush.c
       
${OBJ}/SSSYMBOLIC_common.o : ${CSOU}/common.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_common.o ${CSOU}/common.c
       
${OBJ}/SSSYMBOLIC_errors.o : ${CSOU}/errors.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_errors.o ${CSOU}/errors.c
       
${OBJ}/SSSYMBOLIC_dimensio.o : ${CSOU}/dimensio.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_dimensio.o ${CSOU}/dimensio.c
       
${OBJ}/SSSYMBOLIC_enabling.o : ${CSOU}/enabling.c ${T_INCLUDES} ${INC}/shared.h
	${COMP} ${OBJ}/SSSYMBOLIC_enabling.o ${CSOU}/enabling.c
       
${OBJ}/SSSYMBOLIC_token.o : ${CSOU}/token.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_token.o ${CSOU}/token.c
       
${OBJ}/SSSYMBOLIC_service.o : ${CSOU}/service.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_service.o ${CSOU}/service.c
       
${OBJ}/SSSYMBOLIC_split.o : ${CSOU}/split.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_split.o ${CSOU}/split.c
       
${OBJ}/SSSYMBOLIC_group.o : ${CSOU}/group.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_group.o ${CSOU}/group.c
       
${OBJ}/SSSYMBOLIC_engine.o : ${SSOU}/engine.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_engine.o ${SSOU}/engine.c
       
${OBJ}/SSSYMBOLIC_lists.o : ${SSOU}/lists.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_lists.o ${SSOU}/lists.c
       
${OBJ}/SSSYMBOLIC_eng_wn.o : ${SSOU}/eng_wn.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_eng_wn.o ${SSOU}/eng_wn.c
       
${OBJ}/SSSYMBOLIC_distribu.o : ${SSOU}/distribu.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_distribu.o ${SSOU}/distribu.c
       
${OBJ}/SSSYMBOLIC_stat.o : ${SSOU}/stat.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_stat.o ${SSOU}/stat.c
       
${OBJ}/SSSYMBOLIC_random.o : ${SSOU}/random.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_random.o ${SSOU}/random.c
       
${OBJ}/SSSYMBOLIC_increm.o : ${SSOU}/increm.c ${T_INCLUDES} ${INC}/shared.h
	${COMP} ${OBJ}/SSSYMBOLIC_increm.o ${SSOU}/increm.c

${OBJ}/SSSYMBOLIC_presel.o : ${SSOU}/presel.c ${T_INCLUDES} ${INC}/shared.h
	${COMP} ${OBJ}/SSSYMBOLIC_presel.o ${SSOU}/presel.c

${OBJ}/SSSYMBOLIC_after.o : ${SSOU}/after.c ${T_INCLUDES} ${INC}/shared.h
	${COMP} ${OBJ}/SSSYMBOLIC_after.o ${SSOU}/after.c

${OBJ}/SSSYMBOLIC_wn_yac.o : ${RSOU}/wn_yac.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_wn_yac.o ${RSOU}/wn_yac.c
       
${OBJ}/SSSYMBOLIC_read_arc.o : ${RSOU}/read_arc.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_read_arc.o ${RSOU}/read_arc.c
       
${OBJ}/SSSYMBOLIC_read_t_s.o : ${RSOU}/read_t_s.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_read_t_s.o ${RSOU}/read_t_s.c
       
${OBJ}/SSSYMBOLIC_read_t_c.o : ${RSOU}/read_t_c.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_read_t_c.o ${RSOU}/read_t_c.c
       
${OBJ}/SSSYMBOLIC_read_DEF.o : ${RSOU}/read_DEF.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_read_DEF.o ${RSOU}/read_DEF.c
       
${OBJ}/SSSYMBOLIC_read_NET.o : ${RSOU}/read_NET.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_read_NET.o ${RSOU}/read_NET.c
       
${OBJ}/SSSYMBOLIC_grammar.o : ${SFK}/grammar.c ${T_INCLUDES}
	${COMP} ${OBJ}/SSSYMBOLIC_grammar.o ${SFK}/grammar.c

${SFK}/lex.c : ${TRS}/wn.lex
	$(LEX) ${TRS}/wn.lex
	mv lex.jj.c ${SFK}/lex.c
	#ed ${SFK}/lex.c < ${EDT}/ed_lex

${SFK}/grammar.c : ${TRS}/wn.yac ${SFK}/lex.c ${T_INCLUDES}
	$(YACC) -v -t ${TRS}/wn.yac
	mv y.tab.c ${SFK}/grammar.c
	ed ${SFK}/grammar.c < ${EDT}/ed_gram

${OBJ}/ealloc.o : ${CSOU}/ealloc.c ${T_INCLUDES}
	${COMP}  ${OBJ}/ealloc.o ${CSOU}/ealloc.c

