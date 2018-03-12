#directories
CURDIR=.
BINDIR=./obj/
BINTOOLDIR=./bintools/
OBJDIR=./obj/
SRCDIR=./src/

CC  =/usr/bin/gcc 
LIB =../../../../$(shell ../../../contrib/version.sh)/2bin/lib/
#GLIB = $(LPATH)glib-2.0/lib
GLIBINC = -I ./src/ 
#FLEXLIB =$(LPATH)flex/lib/
INCGREAT =../../INCLUDE/

#commands
COMP   = $(CC) -O3
CFLAG  = -o 
LFLAGS = -lgspnMCDSRG -lm -lfl -lglib-2.0  
LIBFLAG =-L$(LIB) 
#-L$(GLIB) 
# -L$(FLEXLIB)
OBJ     = $(OBJDIR)parser.o
INCFLAG = -I$(INCGREAT) $(GLIBINC) 

#general rules
all  : dsrg raf #tool
dsrg : $(BINDIR)dsrg 
raf  : $(BINDIR)raf_dsrg
#tool : $(BINTOOLDIR)tools

$(BINDIR)dsrg :  ${LIB}libgspnMCDSRG.a $(SRCDIR)dsrg.c 
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ;
	${COMP} ${LIBFLAG} ${GLIBINC} ${CFLAG} $(BINDIR)dsrg $(SRCDIR)dsrg.c $(LFLAGS) 
	cp $(BINDIR)dsrg ../../../../$(shell ../../../contrib/version.sh)/WNDSRG
	rm $(BINDIR)dsrg

$(BINDIR)raf_dsrg : ${LIB}libgspnMCDSRG.a ${OBJ} $(SRCDIR)refDsrg.c
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ;
	${COMP} ${LIBFLAG} ${INCFLAG} ${OBJ} ${CFLAG} $(BINDIR)rdsrg $(SRCDIR)refDsrg.c $(LFLAGS)
	cp $(BINDIR)rdsrg ../../../../$(shell ../../../contrib/version.sh)/WNRDSRG
	rm $(BINDIR)rdsrg
$(OBJDIR)parser.o : $(SRCDIR)parse.c
	pushd $(INCGREAT); /bin/rm -f const.h; ln const.libmcdsrg const.h ; popd ;
	${COMP} -c ${LIBFLAG} ${INCFLAG} ${CFLAG} $(OBJDIR)parser.o $(SRCDIR)parse.c

#$(BINTOOLDIR)tools : 
#	ln -f $(BINTOOLDIR)compute_dsrg.sh $(LPATH)greatspn/bin/dsrgtools.sh
#	ln -f $(BINTOOLDIR)compute_esrg.sh $(LPATH)greatspn/bin/esrgtools.sh
clean : 
	-cd $(BINDIR) ; rm * ; cd ../
	-cd $(OBJDIR) ; rm * ; cd ../
