#directories
CURDIR   =.
BINDIR   =./obj/
OBJDIR   =./obj/
SRCDIR   =./src/
CC       =/usr/bin/gcc

LIBGREAT =../../../../$(shell ../../../contrib/version.sh)/2bin/lib/
INCGREAT =../../INCLUDE/
FLEXLIB  =
GLIB     =
GLIBINC  =

#commands
COMP     = $(CC) -O3
YACC     = byacc
LEX      = flex
CFLAG    = -o 
LDFLAGS  = -lgspnMCDSRG -lm -lfl -lglib-2.0
LEFLAGS  = -lgspnMCESRG -lm -lfl -lglib-2.0
LIBFLAG  = -L$(LIBGREAT)
INCFLAG  = -I$(INCGREAT) $(GLIBINC) -I$(SRCDIR)/common/
OBJDSRG  = $(OBJDIR)Gauss_Seidel.o $(OBJDIR)IndexDsrg.tab.o $(OBJDIR)IndexDsrg.o 
OBJESRG  = $(OBJDIR)Gauss_Seidel.o $(OBJDIR)IndexEsrg.tab.o $(OBJDIR)IndexEsrg.o

#general rules
all :  $(BINDIR)/SolverDsrg  $(BINDIR)/SolverEsrg

DSRG : $(BINDIR)/SolverDsrg  

ESRG : $(BINDIR)/SolverEsrg

$(BINDIR)/SolverDsrg : ${LIBGREAT}libgspnMCDSRG.a  ${OBJDSRG} $(SRCDIR)srcdsrg/SolverDsrg.c 
	 pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ;
	${COMP} ${INCFLAG} ${LIBFLAG} ${GLIBINC} ${CFLAG} $(BINDIR)/solverDsrg  $(SRCDIR)srcdsrg/SolverDsrg.c $(LDFLAGS)  ${OBJDSRG}
	ln -f $(BINDIR)solverDsrg ../../../../$(shell ../../../contrib/version.sh)/WNDSRGSOLVER

$(BINDIR)/SolverEsrg : ${LIBGREAT}libgspnMCESRG.a  ${OBJESRG} $(SRCDIR)srcesrg/SolverEsrg.c 
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcesrg const.h ; popd ;
	${COMP} ${INCFLAG} ${LIBFLAG} ${GLIBINC} ${CFLAG} $(BINDIR)/solverEsrg  $(SRCDIR)srcesrg/SolverEsrg.c $(LEFLAGS)  ${OBJESRG}
	ln -f $(BINDIR)solverEsrg ../../../../$(shell ../../../contrib/version.sh)/WNESRGSOLVER

$(OBJDIR)Gauss_Seidel.o : $(SRCDIR)common/gauss-seidel_algo.c   
	${COMP} -c  ${CFLAG} $(OBJDIR)Gauss_Seidel.o $(SRCDIR)/common/gauss-seidel_algo.c 

$(OBJDIR)IndexDsrg.tab.o : $(SRCDIR)/srcdsrg/IndexDsrg.tab.c 
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ; 
	${COMP} -I${INCGREAT} -c  ${CFLAG}  $(OBJDIR)IndexDsrg.tab.o $(SRCDIR)/srcdsrg/IndexDsrg.tab.c

$(SRCDIR)/srcdsrg/IndexDsrg.tab.c : $(SRCDIR)srcdsrg/IndexDsrg.y
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ; 
	$(YACC) -d -p Index -b IndexDsrg $(SRCDIR)srcdsrg/IndexDsrg.y ; mv IndexDsrg* $(SRCDIR)srcdsrg/

$(OBJDIR)IndexDsrg.o : $(SRCDIR)srcdsrg/IndexDsrg.c 
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ;
	${COMP} -c ${CFLAG} $(OBJDIR)IndexDsrg.o $(SRCDIR)srcdsrg/IndexDsrg.c

$(SRCDIR)srcdsrg/IndexDsrg.c : $(SRCDIR)srcdsrg/IndexDsrg.l
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ; 
	$(LEX)  -PIndex -o${SRCDIR}srcdsrg/IndexDsrg.c $(SRCDIR)srcdsrg/IndexDsrg.l

$(OBJDIR)IndexEsrg.tab.o : $(SRCDIR)/srcesrg/IndexEsrg.tab.c 
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcesrg const.h ; popd ; 
	${COMP} -I${INCGREAT} -c  ${CFLAG}  $(OBJDIR)IndexEsrg.tab.o $(SRCDIR)/srcesrg/IndexEsrg.tab.c

$(SRCDIR)/srcesrg/IndexEsrg.tab.c : $(SRCDIR)srcesrg/IndexEsrg.y
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcesrg const.h ; popd ; 
	$(YACC) -d -p Index -b IndexEsrg $(SRCDIR)srcesrg/IndexEsrg.y ; mv IndexEsrg* $(SRCDIR)srcesrg/

$(OBJDIR)IndexEsrg.o : $(SRCDIR)srcesrg/IndexEsrg.c 
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcesrg const.h ; popd ;
	${COMP} -c ${CFLAG} $(OBJDIR)IndexEsrg.o $(SRCDIR)srcesrg/IndexEsrg.c

$(SRCDIR)srcesrg/IndexEsrg.c : $(SRCDIR)srcesrg/IndexEsrg.l
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcesrg const.h ; popd ; 
	$(LEX)  -PIndex -o${SRCDIR}srcesrg/IndexEsrg.c ${SRCDIR}srcesrg/IndexEsrg.l

clean : 
	-cd $(SRCDIR)srcdsrg/ ; rm IndexDsrg.tab.* IndexDsrg.c ; cd ../../
	-cd $(SRCDIR)srcesrg/ ; rm IndexEsrg.tab.* IndexEsrg.c ; cd ../../	
	-cd $(OBJDIR) ; rm * ; cd ../ 
	-cd $(BINDIR) ; rm * ; cd ../
