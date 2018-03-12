#directories
CSOU = SOURCE/SHARED
SSOU = SOURCE/SIMULAT
RSOU = SOURCE/READNET
EDT = EDIT
SFK = SUPPORT
OBJ = ../../$(GSPN2BINS)/2bin
INC = INCLUDE
BCK = BACKUP
TRS = TRANSL
BINDIR=../../$(GSPN2BINS)

#commands
COMP = $(CC) $(CFLAGS) -o
LINK = $(CC) $(OFLAGS)
LFLAGS = -lm -l$(LEX_LIB) $(SOCKET_LIB)

#files groups
EXECUTABLES  =  WNSIM
T_INCLUDES = ${INC}/const.h \
	     ${INC}/struct.h \
	     ${INC}/var_ext.h \
	     ${INC}/decl.h \
	     ${INC}/fun_ext.h \
	     ${INC}/ealloc.h \
	     ${INC}/macros.h
T_OBJECTS = ${OBJ}/ealloc.o \
	    ${OBJ}/CSCOLORED_grammar.o \
	    ${OBJ}/CSCOLORED_service.o \
	    ${OBJ}/CSCOLORED_token.o \
	    ${OBJ}/CSCOLORED_eng_wn.o \
	    ${OBJ}/CSCOLORED_lists.o \
	    ${OBJ}/CSCOLORED_dimensio.o \
	    ${OBJ}/CSCOLORED_errors.o \
	    ${OBJ}/CSCOLORED_common.o \
	    ${OBJ}/CSCOLORED_random.o \
	    ${OBJ}/CSCOLORED_stat.o \
            ${OBJ}/CSCOLORED_report.o \
            ${OBJ}/CSCOLORED_read_arc.o \
            ${OBJ}/CSCOLORED_read_t_c.o \
            ${OBJ}/CSCOLORED_read_DEF.o \
            ${OBJ}/CSCOLORED_read_NET.o \
            ${OBJ}/CSCOLORED_read_t_s.o \
            ${OBJ}/CSCOLORED_wn_yac.o \
	    ${OBJ}/CSCOLORED_engine.o \
	    ${OBJ}/CSCOLORED_distribu.o \
            ${OBJ}/CSCOLORED_enabling.o \
	    ${OBJ}/CSCOLORED_fire.o\
	    ${OBJ}/CSCOLORED_increm.o\
	    ${OBJ}/CSCOLORED_presel.o\
	    ${OBJ}/CSCOLORED_shared1.o\
	    ${OBJ}/CSCOLORED_shared2.o\
	    ${OBJ}/CSCOLORED_outdom.o\
	    ${OBJ}/CSCOLORED_precheck.o \
	    ${OBJ}/CSCOLORED_degree.o
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
	    ${CSOU}/degree.c \
	    ${SSOU}/eng_wn.c \
	    ${SSOU}/lists.c \
	    ${SSOU}/engine.c \
	    ${SSOU}/distribu.c \
	    ${SSOU}/increm.c\
	    ${SSOU}/presel.c\
	    ${SSOU}/random.c\
	    ${SSOU}/stat.c\
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

$(BINDIR)/WNSIM : ${T_INCLUDES} ${T_OBJECTS} ${T_LEX} ${T_YACC} ${CSOU}/main.c
	${LINK} $(BINDIR)/WNSIM ${CSOU}/main.c ${T_OBJECTS} $(LFLAGS)

${OBJ}/CSCOLORED_outdom.o : ${CSOU}/outdom.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_outdom.o ${CSOU}/outdom.c

${OBJ}/CSCOLORED_shared1.o : ${CSOU}/shared1.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_shared1.o ${CSOU}/shared1.c

${OBJ}/CSCOLORED_shared2.o : ${CSOU}/shared2.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_shared2.o ${CSOU}/shared2.c

${OBJ}/CSCOLORED_precheck.o : ${CSOU}/precheck.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_precheck.o ${CSOU}/precheck.c

${OBJ}/CSCOLORED_fire.o : ${CSOU}/fire.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_fire.o ${CSOU}/fire.c

${OBJ}/CSCOLORED_degree.o : ${CSOU}/degree.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_degree.o ${CSOU}/degree.c

${OBJ}/CSCOLORED_report.o : ${CSOU}/report.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_report.o ${CSOU}/report.c
       
${OBJ}/CSCOLORED_common.o : ${CSOU}/common.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_common.o ${CSOU}/common.c
       
${OBJ}/CSCOLORED_errors.o : ${CSOU}/errors.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_errors.o ${CSOU}/errors.c
       
${OBJ}/CSCOLORED_dimensio.o : ${CSOU}/dimensio.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_dimensio.o ${CSOU}/dimensio.c
       
${OBJ}/CSCOLORED_enabling.o : ${CSOU}/enabling.c ${T_INCLUDES} ${INC}/shared.h
	${COMP}  ${OBJ}/CSCOLORED_enabling.o ${CSOU}/enabling.c
       
${OBJ}/CSCOLORED_token.o : ${CSOU}/token.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_token.o ${CSOU}/token.c
       
${OBJ}/CSCOLORED_service.o : ${CSOU}/service.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_service.o ${CSOU}/service.c
       
${OBJ}/CSCOLORED_engine.o : ${SSOU}/engine.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_engine.o ${SSOU}/engine.c
       
${OBJ}/CSCOLORED_lists.o : ${SSOU}/lists.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_lists.o ${SSOU}/lists.c
       
${OBJ}/CSCOLORED_eng_wn.o : ${SSOU}/eng_wn.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_eng_wn.o ${SSOU}/eng_wn.c
       
${OBJ}/CSCOLORED_distribu.o : ${SSOU}/distribu.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_distribu.o ${SSOU}/distribu.c
       
${OBJ}/CSCOLORED_stat.o : ${SSOU}/stat.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_stat.o ${SSOU}/stat.c
       
${OBJ}/CSCOLORED_random.o : ${SSOU}/random.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_random.o ${SSOU}/random.c
       
${OBJ}/CSCOLORED_increm.o : ${SSOU}/increm.c ${T_INCLUDES} ${INC}/shared.h
	${COMP}  ${OBJ}/CSCOLORED_increm.o ${SSOU}/increm.c

${OBJ}/CSCOLORED_presel.o : ${SSOU}/presel.c ${T_INCLUDES} ${INC}/shared.h
	${COMP}  ${OBJ}/CSCOLORED_presel.o ${SSOU}/presel.c

${OBJ}/CSCOLORED_wn_yac.o : ${RSOU}/wn_yac.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_wn_yac.o ${RSOU}/wn_yac.c
       
${OBJ}/CSCOLORED_read_arc.o : ${RSOU}/read_arc.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_read_arc.o ${RSOU}/read_arc.c
       
${OBJ}/CSCOLORED_read_t_s.o : ${RSOU}/read_t_s.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_read_t_s.o ${RSOU}/read_t_s.c
       
${OBJ}/CSCOLORED_read_t_c.o : ${RSOU}/read_t_c.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_read_t_c.o ${RSOU}/read_t_c.c

${OBJ}/CSCOLORED_read_DEF.o : ${RSOU}/read_DEF.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_read_DEF.o ${RSOU}/read_DEF.c
       
${OBJ}/CSCOLORED_read_NET.o : ${RSOU}/read_NET.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_read_NET.o ${RSOU}/read_NET.c
       
${OBJ}/CSCOLORED_grammar.o : ${SFK}/grammar.c ${T_INCLUDES}
	${COMP}  ${OBJ}/CSCOLORED_grammar.o ${SFK}/grammar.c

${OBJ}/ealloc.o : ${CSOU}/ealloc.c ${T_INCLUDES}
	${COMP}  ${OBJ}/ealloc.o ${CSOU}/ealloc.c

${SFK}/lex.c : ${TRS}/wn.lex
	$(LEX) ${TRS}/wn.lex
	mv lex.jj.c ${SFK}/lex.c
	#ed ${SFK}/lex.c < ${EDT}/ed_lex

${SFK}/grammar.c : ${TRS}/wn.yac ${SFK}/lex.c ${T_INCLUDES}
	$(YACC) -v -t ${TRS}/wn.yac
	mv y.tab.c ${SFK}/grammar.c
	ed ${SFK}/grammar.c < ${EDT}/ed_gram
