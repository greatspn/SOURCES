#directories
SSOU = SOLVE
CSOU = SOURCE/SHARED
GSOU = SOURCE/REACHAB
RSOU = SOURCE/READNET
EDT = EDIT
SFK = SUPPORT
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
EXECUTABLES  =  WNRG
T_INCLUDES = ${INC}/const.h \
	     ${INC}/struct.h \
	     ${INC}/var_ext.h \
	     ${INC}/decl.h \
	     ${INC}/fun_ext.h \
	     ${INC}/ealloc.h \
	     ${INC}/macros.h
T_OBJECTS =  ${OBJ}/getline.o \
	    ${OBJ}/ealloc.o \
	    ${OBJ}/RGCOLORED_grammar.o \
	    ${OBJ}/RGCOLORED_service.o \
	    ${OBJ}/RGCOLORED_token.o \
	    ${OBJ}/RGCOLORED_dimensio.o \
	    ${OBJ}/RGCOLORED_errors.o \
	    ${OBJ}/RGCOLORED_compact.o \
	    ${OBJ}/RGCOLORED_common.o \
	    ${OBJ}/RGCOLORED_report.o \
	    ${OBJ}/RGCOLORED_enabling.o \
	    ${OBJ}/RGCOLORED_fire.o\
	    ${OBJ}/RGCOLORED_shared1.o\
	    ${OBJ}/RGCOLORED_shared2.o\
	    ${OBJ}/RGCOLORED_outdom.o\
	    ${OBJ}/RGCOLORED_degree.o \
	    ${OBJ}/RGCOLORED_precheck.o \
	    ${OBJ}/RGCOLORED_flush.o \
	    ${OBJ}/RGCOLORED_graph_se.o \
	    ${OBJ}/RGCOLORED_graph.o \
	    ${OBJ}/RGCOLORED_stack.o \
	    ${OBJ}/RGCOLORED_convert.o \
	    ${OBJ}/RGCOLORED_rg_files.o \
	    ${OBJ}/RGCOLORED_rgengwn.o \
	    ${OBJ}/RGCOLORED_read_arc.o \
            ${OBJ}/RGCOLORED_read_t_c.o \
            ${OBJ}/RGCOLORED_read_DEF.o \
            ${OBJ}/RGCOLORED_read_NET.o \
            ${OBJ}/RGCOLORED_read_t_s.o \
	    ${OBJ}/RGCOLORED_wn_yac.o
T_SOURCES = ${CSOU}/service.c \
	    ${CSOU}/ealloc.c \
	    ${CSOU}/token.c \
	    ${CSOU}/dimensio.c \
	    ${CSOU}/errors.c \
	    ${SSOU}/compact.c \
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
	    ${GSOU}/graph_se.c\
	    ${GSOU}/graph.c\
	    ${GSOU}/stack.c\
	    ${GSOU}/convert.c\
	    ${GSOU}/rg_files.c\
	    ${GSOU}/rgengwn.c\
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

$(BINDIR)/WNRG : ${T_INCLUDES} ${T_OBJECTS} ${T_LEX} ${T_YACC} ${CSOU}/main.c
	${LINK} $(BINDIR)/WNRG ${CSOU}/main.c ${T_OBJECTS} $(LFLAGS)

${OBJ}/RGCOLORED_outdom.o : ${CSOU}/outdom.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_outdom.o ${CSOU}/outdom.c

${OBJ}/RGCOLORED_shared1.o : ${CSOU}/shared1.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_shared1.o ${CSOU}/shared1.c

${OBJ}/RGCOLORED_shared2.o : ${CSOU}/shared2.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_shared2.o ${CSOU}/shared2.c

${OBJ}/RGCOLORED_precheck.o : ${CSOU}/precheck.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_precheck.o ${CSOU}/precheck.c

${OBJ}/RGCOLORED_fire.o : ${CSOU}/fire.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_fire.o ${CSOU}/fire.c

${OBJ}/RGCOLORED_degree.o : ${CSOU}/degree.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_degree.o ${CSOU}/degree.c

${OBJ}/RGCOLORED_flush.o : ${CSOU}/flush.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_flush.o ${CSOU}/flush.c
       
${OBJ}/RGCOLORED_compact.o : ${SSOU}/compact.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_compact.o ${SSOU}/compact.c
       
${OBJ}/RGCOLORED_common.o : ${CSOU}/common.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_common.o ${CSOU}/common.c
       
${OBJ}/RGCOLORED_errors.o : ${CSOU}/errors.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_errors.o ${CSOU}/errors.c
       
${OBJ}/RGCOLORED_dimensio.o : ${CSOU}/dimensio.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_dimensio.o ${CSOU}/dimensio.c
       
${OBJ}/RGCOLORED_token.o : ${CSOU}/token.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_token.o ${CSOU}/token.c
       
${OBJ}/RGCOLORED_service.o : ${CSOU}/service.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_service.o ${CSOU}/service.c
       
${OBJ}/RGCOLORED_enabling.o : ${CSOU}/enabling.c ${T_INCLUDES} ${INC}/shared.h
	${COMP} ${OBJ}/RGCOLORED_enabling.o ${CSOU}/enabling.c

${OBJ}/RGCOLORED_report.o : ${CSOU}/report.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_report.o ${CSOU}/report.c

${OBJ}/RGCOLORED_graph.o : ${GSOU}/graph.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_graph.o ${GSOU}/graph.c
       
${OBJ}/RGCOLORED_graph_se.o : ${GSOU}/graph_se.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_graph_se.o ${GSOU}/graph_se.c

${OBJ}/RGCOLORED_rgengwn.o : ${GSOU}/rgengwn.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_rgengwn.o ${GSOU}/rgengwn.c

${OBJ}/RGCOLORED_rg_files.o : ${GSOU}/rg_files.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_rg_files.o ${GSOU}/rg_files.c
       
${OBJ}/RGCOLORED_stack.o : ${GSOU}/stack.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_stack.o ${GSOU}/stack.c
       
${OBJ}/RGCOLORED_convert.o : ${GSOU}/convert.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_convert.o ${GSOU}/convert.c

${OBJ}/RGCOLORED_wn_yac.o : ${RSOU}/wn_yac.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_wn_yac.o ${RSOU}/wn_yac.c
       
${OBJ}/RGCOLORED_read_arc.o : ${RSOU}/read_arc.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_read_arc.o ${RSOU}/read_arc.c
       
${OBJ}/RGCOLORED_read_t_s.o : ${RSOU}/read_t_s.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_read_t_s.o ${RSOU}/read_t_s.c
       
${OBJ}/RGCOLORED_read_t_c.o : ${RSOU}/read_t_c.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_read_t_c.o ${RSOU}/read_t_c.c
       
${OBJ}/RGCOLORED_read_DEF.o : ${RSOU}/read_DEF.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_read_DEF.o ${RSOU}/read_DEF.c
       
${OBJ}/RGCOLORED_read_NET.o : ${RSOU}/read_NET.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_read_NET.o ${RSOU}/read_NET.c
       
${OBJ}/RGCOLORED_grammar.o : ${SFK}/grammar.c ${T_INCLUDES}
	${COMP} ${OBJ}/RGCOLORED_grammar.o ${SFK}/grammar.c

${OBJ}/ealloc.o : ${CSOU}/ealloc.c ${T_INCLUDES}
	${COMP}  ${OBJ}/ealloc.o ${SFK}/ealloc.c

${OBJ}/getline.o : ${CSOU}/getline.c ${T_INCLUDES}
	${COMP}  ${OBJ}/getline.o ${CSOU}/getline.c


${SFK}/lex.c : ${TRS}/wn.lex
	$(LEX) ${TRS}/wn.lex
	mv lex.jj.c ${SFK}/lex.c
	#ed ${SFK}/lex.c < ${EDT}/ed_lex

${SFK}/grammar.c : ${TRS}/wn.yac ${SFK}/lex.c ${T_INCLUDES}
	$(YACC) -v -t ${TRS}/wn.yac
	mv y.tab.c ${SFK}/grammar.c
	ed ${SFK}/grammar.c < ${EDT}/ed_gram
