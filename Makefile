############################################################################################
###                   New Makefile for the GreatSPN project.                             ###
############################################################################################

### Global Makefile variables ###
CC := ./contrib/colorgcc -c
CPP := ./contrib/colorg++ -c 
LD := ./contrib/colorgcc
LDPP := ./contrib/colorg++
RM := rm -f
RMDIR := rm -rf
MKDIR := mkdir -p
CPDIR := cp -r
LEX := flex
LEXPP := flex++
YACC := byacc -d
YACCPP := byacc -d
LEMON := ./bin/lemon
AR := ar rcs

### The following variables can be overridden 
### by defining them as environment variables.
# CFLAGS ?= -g -DGLIBCXX_DEBUG
CFLAGS ?= -O2
CPPFLAGS ?= $(CFLAGS)
LDFLAGS ?= -O2

ifdef STATIC_LINK
	LDFLAGS += -static
endif

ifdef SELF_CONTAINED
	LDFLAGS += -Wl,-rpath=./sclib/ -Wl,-dynamic-linker,./sclib/ld-linux-x86-64.so.2
endif

# LDFLAGS ?= -g
INCLUDES ?= 
LEXFLAGS ?=
YACCFLAGS ?=
LEXPPFLAGS ?=
YACCPPFLAGS ?=
ARFLAGS :=
UIL ?= /usr/bin/uil
ENABLE_Cxx17 ?= -std=c++17
ENABLE_Cxx14 ?= -std=c++14

# External libraries
FLEX-INCLUDE :=
FLEX-LIB := 
#-lfl

GLIB-INCLUDE := $(shell pkg-config --static --silence-errors --cflags glib-2.0)
GLIB-LIB := $(shell pkg-config --static --silence-errors --libs glib-2.0)

X11-INCLUDE := -I/usr/include/X11 -I/usr/include/openmotif -I/usr/include/Xm
X11-LIB := -L/usr/lib/X11 -L/usr/lib/openmotif -L/usr/lib/Xm -L/usr/local/lib

MOTIF-INCLUDE := 
MOTIF-LIB := 

OPENGL-LIB := -lGL -lGLU -lglut

# Destination directories
OBJDIR := objects
BINDIR := bin
SCRIPTDIR := scripts
LIBDIR := lib
INSTALLEDSOURCEDIR :=inst_src
INSTALLDIR ?= /usr/local/GreatSPN
PKGDIR = /usr/local/GreatSPN

### - Platform-specific variations - ###
UNAME_S := $(shell uname -s)
UNAME_R := $(shell uname -r)
ifeq ($(UNAME_S),Darwin)
   # MacOSX specific variables
   X11-INCLUDE := -I/opt/X11/include
   X11-LIB := -L/opt/X11/lib
   MOTIF-INCLUDE := -I/usr/OpenMotif/include
   MOTIF-LIB := -L/usr/OpenMotif/lib
   UIL := /usr/OpenMotif/bin/uil
#    FLEX-LIB := -lfl -L/usr/local/opt/flex/lib
   OPENGL-LIB := -lgl -lglu -lglut
   CC := gcc -g -c -std=c99 -Wno-deprecated-register
   CPP := g++ -g -c -std=c++17 -Wno-unused-local-typedef -Wno-deprecated-register
   LD := gcc -g 
   LDPP := g++ -g 
   CFLAGS += -I/usr/include/malloc -I/usr/local/include/
   LDFLAGS += -L/usr/local/lib/
   ENABLE_Cxx17 := -std=c++17 -stdlib=libc++ -U__STRICT_ANSI__
   LAUNCH4J := java -jar JavaGUI/launch4j-macosx/launch4j.jar

   
	# CFLAGS += -Wno-implicit-function-declaration \
	# 		  -Wno-unused-value -Wno-int-conversion -Wno-int-to-pointer-cast \
	# 		  -Wno-return-type -Wno-format -Wno-parentheses-equality \
	# 		  -Wno-logical-op-parentheses -Wno-comment -Wno-parentheses \
	# 		  -Wno-incompatible-pointer-types -Wno-dangling-else \
	# 		  -Wno-implicit-int -Wno-empty-body

	CFLAGS += -Wno-unused-value -Wno-int-conversion -Wno-int-to-pointer-cast \
			  -Wno-return-type -Wno-format -Wno-parentheses-equality \
			  -Wno-logical-op-parentheses -Wno-comment -Wno-parentheses \
			  -Wno-incompatible-pointer-types -Wno-dangling-else \
			  -Wno-implicit-int -Wno-empty-body

endif

### - Platform-specific variations - ###
ifneq (,$(findstring Microsoft,$(UNAME_R)))
  ifeq ($(UNAME_S),Linux)
    IS_WSL := 1
    #$(info "Running on WSL.")
  endif
endif
###

# # REMOVE THESE LINES
# ifeq "Linux" "$(shell uname)"
# 	OBJDIR := linux-objects
# endif

# have_command = $(eval $(1):=$(if $(shell which $(2)),1,))

##############################################################################
# Dependency resolution
##############################################################################
# Macros used to simplify the work

# search for a file $(2). If it exists, define variable HAS_$(1) to 1
define search_file
	$(if $(HAS_$(1)), , $(if $(wildcard $(2)), $(eval HAS_$(1):=1) ))
endef
# search for a library in path $(2). If it exists, define two variables:
#  HAS_$(1) to 1,  LINK_$(1) to the -L/path and the additional rules $(3)
define search_lib
	$(if $(HAS_$(1)), , \
	  $(if $(wildcard $(2)), \
	    $(eval HAS_$(1):=1) ; $(eval LINK_$(1):=-L$(dir $(2)) $(3))  )\
	 )
endef
define warn_missing
	$(if $(HAS_$(1)), , $(warning Missing $(2). Some packages will not be compiled.) )
endef
##############################################################################

$(call search_file,OPENMOTIF,$(UIL))
$(call warn_missing,OPENMOTIF,OpenMotif)

# ifneq ($(shell which $(UIL)),$(UIL))
#   $(warning "OpenMotif is not installed. Some packages will not compile properly.")
# else
#   HAS_OPENMOTIF_LIB := 1
# endif

$(call search_file,PKGCONFIG,$(shell which pkg-config))
$(call warn_missing,PKGCONFIG,pkg-config tool)

# ifeq ($(shell which pkg-config),)
#   $(warning "The pkg-config tool is not installed. Some packages will not compile properly.")
# endif

ifeq ($(GLIB-INCLUDE),)
  $(warning "The glib-2.0 library is not installed. Some packages will not compile properly.")
else
	HAS_GLIB_LIB := 1
endif

$(call search_file,GRAPHMDP_LIB,/usr/local/lib/libgraphmdp.*,-lgraphmdp)
$(call warn_missing,GRAPHMDP_LIB,GraphMDP library)
# GRAPHMDP_LIB := /usr/local/lib/libgraphmdp.a
# ifeq ($(wildcard $(GRAPHMDP_LIB)),)
#   $(warning "The GraphMDP library is not installed. Some packages will not be compiled.")
# else
#   HAS_GRAPHMDP_LIB := 1
# endif


$(call search_lib,LIBXMLPP2-6_LIB,/usr/local/lib/libxml++-2.6)
$(call search_lib,LIBXMLPP2-6_LIB,/usr/lib/libxml++-2.6)
$(call search_lib,LIBXMLPP2-6_LIB,/usr/lib64/libxml++-2.6)
$(call warn_missing,LIBXMLPP2-6_LIB,libXML++-2.6 library)
# LIBXMLPP2-6_LIB := /usr/local/lib/libxml++-2.6
# LIBXMLPP2-6_LIB_2 := /usr/lib/libxml++-2.6
# ifneq ($(wildcard $(LIBXMLPP2-6_LIB)),)
#   HAS_LIBXMLPP2-6_LIB := 1
# else ifneq ($(wildcard $(LIBXMLPP2-6_LIB_2)),)
#   HAS_LIBXMLPP2-6_LIB := 1
# else
#   $(warning "The libXML++-2.6 library is not installed. Some packages will not be compiled.")
# endif

$(call search_lib,GLIBMM2-4_LIB,/usr/local/lib/libglibmm-2.4.*)
$(call search_lib,GLIBMM2-4_LIB,/usr/lib/libglibmm-2.4.*)
$(call search_lib,GLIBMM2-4_LIB,/usr/lib64/libglibmm-2.4.*)
$(call warn_missing,GLIBMM2-4_LIB,glibmm-2.4 library)
# GLIBMM2-4_LIB := /usr/local/lib/libglibmm-2.4.*
# GLIBMM2-4_LIB_2 := /usr/lib/libglibmm-2.4.*
# ifneq ($(wildcard $(GLIBMM2-4_LIB)),)
#   HAS_GLIBMM2-4_LIB := 1
# else ifneq ($(wildcard $(GLIBMM2-4_LIB_2)),)
#   HAS_GLIBMM2-4_LIB := 1
# else
#   $(warning "The glibmm-2.4 library is not installed. Some packages will not be compiled.")
# endif

$(call search_lib,GLPK_LIB,/usr/local/lib/libglpk.*,-lglpk)
$(call search_lib,GLPK_LIB,/usr/lib/libglpk.*,-lglpk)
$(call search_lib,GLPK_LIB,/usr/lib64/libglpk.*,-lglpk)
$(call warn_missing,GLPK_LIB,GLPK library)
# $(info GLPK_LIB  $(HAS_GLPK_LIB)  $(LINK_GLPK_LIB))
# GLPK_LIB := /usr/local/lib/libglpk.a
# GLPK_LIB_2 := /usr/lib/libglpk.a
# ifneq ($(wildcard $(GLPK_LIB)),)
#   HAS_GLPK_LIB := 1
# else ifneq ($(wildcard $(GLPK_LIB)),)
#   HAS_GLPK_LIB := 1
# else
#   $(warning "The GLPJ library is not installed. Some packages will not be compiled.")
# endif


# LP_SOLVE_LIB := /usr/local/lib/liblpsolve55.a
# ifeq ($(wildcard $(LP_SOLVE_LIB)),)
#   $(warning "The lp_solve dynamic library is not installed. Some packages will not be compiled.")
# else
#   HAS_LP_SOLVE_LIB := 1
# endif

$(call search_lib,LP_SOLVE_LIB,/usr/local/lib/liblpsolve55.*,-llpsolve55 -ldl -lcolamd)
$(call search_lib,LP_SOLVE_LIB,/usr/lib/liblpsolve55.*,-llpsolve55 -ldl -lcolamd)
$(call search_lib,LP_SOLVE_LIB,/usr/lib64/liblpsolve55.*,-llpsolve55 -ldl -lcolamd)
$(call warn_missing,LP_SOLVE_LIB,lp_solve55 library)
ifdef HAS_LP_SOLVE_LIB
	INCLUDE_LP_SOLVE_LIB := -DHAS_LP_SOLVE_LIB=1 -I/usr/local/include/lpsolve/ -I/usr/include/lpsolve/
endif
# $(info LP_SOLVE_LIB  $(HAS_LP_SOLVE_LIB)  $(LINK_LP_SOLVE_LIB)  $(INCLUDE_LP_SOLVE_LIB))

# LP_SOLVE_LIB_1 := /usr/include/lpsolve/lp_lib.h
# LP_SOLVE_LIB_2 := /usr/local/include/lp_lib.h
# LP_SOLVE_LIB_3 := /usr/local/include/lpsolve/lp_lib.h
# ifeq ($(wildcard $(LP_SOLVE_LIB_1)),)
#   ifeq ($(wildcard $(LP_SOLVE_LIB_2)),)
#     ifeq ($(wildcard $(LP_SOLVE_LIB_3)),)
#       $(warning "The lp-solve package is not installed. Some packages will not be compiled.")
#     else
#       HAS_LP_SOLVE_LIB := 1
#       LINK_LP_SOLVE_LIB := -L/usr/local/lib -llpsolve55
#       INCLUDE_LP_SOLVE_LIB := -DHAS_LP_SOLVE_LIB=1 -I/usr/local/include/lpsolve/
#     endif
#   else
#     HAS_LP_SOLVE_LIB := 1
#     LINK_LP_SOLVE_LIB := -L/usr/local/lib -llpsolve55 -lcolamd
#     INCLUDE_LP_SOLVE_LIB := -DHAS_LP_SOLVE_LIB=1
#   endif
# else
#   HAS_LP_SOLVE_LIB := 1
#   LINK_LP_SOLVE_LIB := -L/usr/lib64 -L/usr/lib/lp_solve/ -llpsolve55 -ldl -lcolamd
#   INCLUDE_LP_SOLVE_LIB := -DHAS_LP_SOLVE_LIB=1 -I/usr/include/lpsolve/
# endif

$(call search_lib,GMP_LIB,/usr/local/lib/libgmpxx.*,-lgmpxx -lgmp)
$(call search_lib,GMP_LIB,/usr/lib/libgmpxx.*,-lgmpxx -lgmp)
$(call search_lib,GMP_LIB,/usr/lib64/libgmpxx.*,-lgmpxx -lgmp)
$(call warn_missing,GMP_LIB,GMP library)
ifdef HAS_GMP_LIB
	INCLUDE_GMP_LIB := -DHAS_GMP_LIB=1
endif
# $(info GMP_LIB  $(HAS_GMP_LIB)  $(LINK_GMP_LIB)  $(INCLUDE_GMP_LIB))


# GMP_LIB_1 := /usr/include/gmpxx.h
# GMP_LIB_2 := /usr/local/include/gmpxx.h
# ifeq ($(wildcard $(GMP_LIB_1)),)
#   ifeq ($(wildcard $(GMP_LIB_2)),)
#     $(warning "The GMP library is not installed. Some packages will not be compiled.")
#   else
#     HAS_GMP_LIB := 1
#     LINK_GMP_LIB := -L/usr/local/lib -lgmpxx -lgmp
#     INCLUDE_GMP_LIB := -DHAS_GMP_LIB=1
#   endif
# else
#   HAS_GMP_LIB := 1
#   LINK_GMP_LIB := -L/usr/lib64 -lgmpxx -lgmp
#   INCLUDE_GMP_LIB := -DHAS_GMP_LIB=1
# endif

$(call search_file,JAVA_DEVELOPMENT_KIT,$(shell which javac))
$(call warn_missing,JAVA_DEVELOPMENT_KIT,Java JDK)

$(call search_file,APACHE_ANT,$(shell which ant))
$(call warn_missing,APACHE_ANT,Apache ANT)

# ifeq ($(shell which javac),)
#   $(warning "Java JDK is not installed. Some packages will not compile properly.")
# else
#   HAS_JAVA_DEVELOPMENT_KIT := 1
#   # Test for Apache ANT
#   ifeq ($(shell which ant),)
#     $(warning "Apache ANT build system is not installed. Some packages will not compile properly.")
#   else
#     HAS_APACHE_ANT := 1
#   endif
#   # Test for ANTLR version 4
#   # ifeq ($(shell which antlr4),)
#   #   $(warning "ANTLRv4 is not installed. Some packages will not compile properly.")
#   # else
#   #   HAS_ANTLRv4 := 1
#   # endif
# endif

$(call search_lib,BOOST_CXX_LIB,/usr/local/lib/libboost_context.*)
$(call search_lib,BOOST_CXX_LIB,/usr/lib/libboost_context.*)
$(call search_lib,BOOST_CXX_LIB,/usr/lib64/libboost_context.*)
$(call warn_missing,BOOST_CXX_LIB,Boost C++ library)
# $(info BOOST_CXX_LIB  $(HAS_BOOST_CXX_LIB)  $(LINK_BOOST_CXX_LIB)  $(INCLUDE_BOOST_CXX_LIB))

# ifeq ($(wildcard $(BOOST_Cxx)/boost/config.hpp), )
#   $(warning "Boost C++ is not installed. Some packages will not be compiled.")
# else
#   HAS_BOOST_CXX_LIB := 1
# endif

# ifeq ($(wildcard JavaGUI/launch4j-macosx/launch4j.jar), )
#   #$(warning ".")
# else
#   ifdef LAUNCH4J
#     HAVE_LAUNCH4J := 1
#   endif
# endif


ifneq ("$(wildcard /home/user/Desktop/HowToODE-SDE)","")
  IS_VBOX_VERSION71 := 1
  $(info "GreatSPN virtual machine version 7.1")
endif

ifneq ("$(wildcard /home/user/.greatspn-on-vbox)","")
  IS_VBOX_VERSION71 := 1
  HAS_VBOX_MARK := 1
  $(info "This is the GreatSPN distribution on the virtual machine")
endif

ifeq ($(IS_VBOX_VERSION71),1)
  ifneq ($(HAS_VBOX_MARK),1)
    X1:=$(shell touch /home/user/.greatspn-on-vbox)
    X2:=$(shell /home/user/GreatSPN/SOURCES/NSRC/VirtualMachineSupport/vbox_install_script.sh skip_make)
    $(error You must re-run the Upgrade procedure by double-clicking the 'Upgrade GreatSPN' icon on the desktop.)
  endif
endif

ifeq ($(IS_VBOX_VERSION71),1)
  INCLUDE_ELVIO_CPP_SOLVER := 1
endif
ifneq ("$(wildcard ~/.extra-greatspn-solvers)","")
  INCLUDE_ELVIO_CPP_SOLVER := 1
endif

ifeq ($(INCLUDE_ELVIO_CPP_SOLVER),1)
  ifneq ($(wildcard JavaGUI/launch4j-macosx/launch4j.jar), )
    ifdef LAUNCH4J
      $(info Have Launch4j)
      HAVE_LAUNCH4J := 1
    endif
  endif
endif

ifneq ("$(wildcard ../PRIVATE)","")
  ifeq ("$(wildcard PRIV)","")
    $(warning have ../PRIVATE but not PRIV. Making a symbolic link.)
    $(shell ln -s ../PRIVATE PRIV)
  endif
endif

ifneq ("$(wildcard PRIV)","")
  HAVE_PRIVATE_SECTION := 1
endif

# ifneq ("$(wildcard ~/.use-rgmedd2)","")
#   USE_RGMEDD2 := 1
#   $(warning "RGMEDD2 will be compiled.")
# endif

# ifneq "$(shell which pkg-config)" ""
# 	HAVE_PKG_CONFIG := 1
# endif

# prova:
# 	@echo $(HAVE_PKG_CONFIG)
# 	@echo $(call have_command,HAVE_PKG_CONFIG2,pkg-config) $(HAVE_PKG_CONFIG2)

### Note on variable definitions: ###
# The makefile evaluates compilation variables from the most specific
# to the most general. For instance, consider the compilation of
# a file src/code.c for target MYAPP requesting the variable CFLAGS.
# The makefile first searches:  MYAPP_CFLAGS_src/code.c
# and uses this variable if it exists. If such variable is not defined,
# the makefile searches for:  MYAPP_CFLAGS  . If even that variable
# is not defined, then it uses the global CFLAGS. 
# Note that file-specific and target-specific variables have precedence
# and if present only their values is used in expansion.

### The root rule. ###

all: libraries binaries scripts

### Common definitions ###

# generate_WN_FLAGS = '-DCONST_H="$(1)"' '-DCONST1_H="../INCLUDE/$(1)"' \
# 					'-DCONST2_H="../../INCLUDE/$(1)"' '-DCONST3_H="../../../INCLUDE/$(1)"' \
# 					'-DWN_GRAMMAR_H="../../objects/$(2)/WN/TRANSL/wn_grammar.y.h"' $(CFLAGS)
# generate_GSPN_FLAGS = '-DCONST_H="$(1)"' '-DCONST1_H="../INCLUDE/$(1)"' \
# 					  '-DCONST2_H="../../INCLUDE/$(1)"' '-DCONST3_H="../../../INCLUDE/$(1)"' \
# 					  '-DWN_GRAMMAR_H="../../objects/$(2)/WN/TRANSL/gspn_grammar.y.h"' $(CFLAGS)
generate_WN_FLAGS = '-D$(1)=1' '-Iobjects/$(2)/WN/TRANSL/' $(CFLAGS)

######################################
### gsrc2/ package
######################################

TARGETS += pinvar tinvar unbound deadlock implp \
		   transpose ggsc gtrc ntrs ntrex2 gre comp \
		   struct grg_prep gst_prep grg_stndrd gmt_prep \
		   show_stndrd strong_con liveness gmt_stndrd \
		   gst_stndrd throughput dortmund_sortmark \
		   showprob showmtx showtpd mark_lp flow_lp disab_lp 

pinvar_SOURCES := gsrc2/pinvar.c
tinvar_SOURCES := gsrc2/tinvar.c
unbound_SOURCES := gsrc2/unbound.c
deadlock_SOURCES := gsrc2/deadlock.c
implp_SOURCES := gsrc2/implp.c
transpose_SOURCES := gsrc2/transpose.c gsrc2/compact.c
ggsc_SOURCES := gsrc2/ggsc.c gsrc2/compact.c
gtrc_SOURCES := gsrc2/gtrc.c gsrc2/compact.c
ntrs_SOURCES := gsrc2/ntrs.c gsrc2/compact.c
ntrex2_SOURCES := gsrc2/ntrex2.c gsrc2/compact.c
gre_SOURCES := gsrc2/res_gram.l gsrc2/res_gram.y
comp_SOURCES := gsrc2/comp_main.c gsrc2/comp_menc.c gsrc2/comp_mark.c \
				gsrc2/comp_tran.c gsrc2/mdr_gram.l gsrc2/mdr_gram.y
struct_SOURCES := gsrc2/struct.c
grg_prep_SOURCES := gsrc2/grg_prep.c
gmt_prep_SOURCES := gsrc2/mdr_stndrd.l gsrc2/mdr_stndrd.y
gst_prep_SOURCES := gsrc2/res_stndrd.l gsrc2/res_stndrd.y
grg_stndrd_SOURCES := gsrc2/grg_stndrd.c gsrc2/compact.c
show_stndrd_SOURCES := gsrc2/show_stndrd.c gsrc2/compact.c
strong_con_SOURCES := gsrc2/strong_con.c gsrc2/compact.c
liveness_SOURCES := gsrc2/liveness.c gsrc2/compact.c
gmt_stndrd_SOURCES := gsrc2/gmt_stndrd.c gsrc2/compact.c
gst_stndrd_SOURCES := gsrc2/gst_stndrd.c gsrc2/compact.c
throughput_SOURCES := gsrc2/throughput.c gsrc2/compact.c
dortmund_sortmark_SOURCES := gsrc2/dortmund_sortmark.c gsrc2/compact.c
showprob_SOURCES := gsrc2/show_prob.c gsrc2/compact.c
showmtx_SOURCES := gsrc2/seemtx.c gsrc2/compact.c
showtpd_SOURCES := gsrc2/readtpd.c gsrc2/compact.c
mark_lp_SOURCES := gsrc2/mark_lp.c
flow_lp_SOURCES := gsrc2/flow_lp.c
disab_lp_SOURCES := gsrc2/disab_lp.c

### Precedence rules: Yacc sources must be compiled before Lex sources. ###
gsrc2/res_gram.l: $(OBJDIR)/gre/gsrc2/res_gram.y.o
gsrc2/mdr_gram.l: $(OBJDIR)/comp/gsrc2/mdr_gram.y.o
gsrc2/res_stndrd.l: $(OBJDIR)/gst_prep/gsrc2/res_stndrd.y.o
gsrc2/mdr_stndrd.l: $(OBJDIR)/gmt_prep/gsrc2/mdr_stndrd.y.o


SCRIPTS += RMNET newRG showRG checkRG newMT newSO randomTR rande2TR  \
		   showprob showmtx showtpd showCTMCdot showCTMC \
		   showCTMC2SMART struct pinv deadl traps implp tinv sbound lpbound lpmark\
		   glp_solve 
# DortmundMT gst 2RG 2GS 2SO 2MT 2TR newTR

RMNET_SOURCEFILE := gsrc2/RMNET
newRG_SOURCEFILE := gsrc2/newRG
showRG_SOURCEFILE := gsrc2/showRG
checkRG_SOURCEFILE := gsrc2/checkRG
newMT_SOURCEFILE := gsrc2/newMT
newSO_SOURCEFILE := gsrc2/newSO
# newTR_SOURCEFILE := gsrc2/newTR
randomTR_SOURCEFILE := gsrc2/randomTR
rande2TR_SOURCEFILE := gsrc2/rande2TR
# 2RG_SOURCEFILE := gsrc2/2RG
# 2GS_SOURCEFILE := gsrc2/2GS
# 2SO_SOURCEFILE := gsrc2/2SO
# 2MT_SOURCEFILE := gsrc2/2MT
# 2TR_SOURCEFILE := gsrc2/2TR
# DortmundMT_SOURCEFILE := gsrc2/DortmundMT
showprob_SOURCEFILE := gsrc2/showprob
showmtx_SOURCEFILE := gsrc2/showmtx
showtpd_SOURCEFILE := gsrc2/showtpd.sh
showCTMCdot_SOURCEFILE := gsrc2/showCTMCdot
showCTMC_SOURCEFILE := gsrc2/showCTMC
showCTMC2SMART_SOURCEFILE := gsrc2/showCTMC2SMART
struct_SOURCEFILE := gsrc2/struct.sh
pinv_SOURCEFILE := gsrc2/pinv.sh
deadl_SOURCEFILE := gsrc2/deadl.sh
traps_SOURCEFILE := gsrc2/traps.sh
implp_SOURCEFILE := gsrc2/implp.sh
tinv_SOURCEFILE := gsrc2/tinv.sh
sbound_SOURCEFILE := gsrc2/sbound.sh
lpbound_SOURCEFILE := gsrc2/lpbound.sh
lpmark_SOURCEFILE := gsrc2/lpmark.sh
glp_solve_SOURCEFILE := gsrc2/glp_solve.sh




# gst_SOURCEFILE := gsrc2/gst.sh





######################################
### algebra package
######################################

TARGETS += remove algebra

remove_SOURCES := algebra/Remove/global.c \
				  algebra/Remove/alloc.c \
				  algebra/Remove/layer.c \
				  algebra/Remove/save.c \
				  algebra/Remove/load.c \
				  algebra/Remove/remove.c \
				  algebra/Remove/lexer.l \
				  algebra/Remove/parser.y

algebra_SOURCES := algebra/Composition/global.c \
				   algebra/Composition/alloc.c \
				   algebra/Composition/layer.c \
				   algebra/Composition/save.c \
				   algebra/Composition/load.c \
				   algebra/Composition/rescale.c \
				   algebra/Composition/algebra.c \
				   algebra/Composition/lexer.l \
				   algebra/Composition/parser.y
				   
algebra_LDFLAGS:= $(LDFLAGS) -lm
SCRIPTS += unfolding algebra remove

algebra/Remove/lexer.l: $(OBJDIR)/remove/algebra/Remove/parser.y.o
algebra/Composition/lexer.l: $(OBJDIR)/algebra/algebra/Composition/parser.y.o

unfolding_SOURCEFILE := WN/SOURCE/UNFOLDING/unfolding.sh
algebra_SOURCEFILE := contrib/algebra/algebra.sh
remove_SOURCEFILE := contrib/algebra/remove.sh





######################################
### SOLVE package
######################################

TARGETS += swn_ntrs swn_ggsc swn_stndrd \
		   swn_gst_stndrd swn_gst_prep


swn_ntrs_SOURCES := WN/SOLVE/swn_ntrs.c WN/SOLVE/compact.c
swn_ntrs_CFLAGS := $(call generate_WN_FLAGS,TOOL_SWN_NTRS,swn_ntrs)

swn_ggsc_SOURCES := WN/SOLVE/swn_ggsc.c WN/SOLVE/compact.c
swn_ggsc_CFLAGS := $(call generate_WN_FLAGS,TOOL_SWN_GGSC,swn_ggsc)

swn_stndrd_SOURCES := WN/SOLVE/swn_stndrd.c WN/SOLVE/compact.c
swn_stndrd_CFLAGS := $(call generate_WN_FLAGS,TOOL_SWN_STNDRD,swn_stndrd)

swn_gst_stndrd_SOURCES := WN/SOLVE/swn_gst_stndrd.c WN/SOLVE/compact.c
swn_gst_stndrd_CFLAGS := $(call generate_WN_FLAGS,TOOL_SWN_GST_STNDRD,swn_gst_stndrd)

swn_gst_prep_SOURCES := WN/SOLVE/res_stndrd.l WN/SOLVE/res_stndrd.y
swn_gst_prep_CFLAGS := $(call generate_WN_FLAGS,TOOL_SWN_GST_PREP,swn_gst_prep)


WN/SOLVE/res_stndrd.l: $(OBJDIR)/swn_gst_prep/WN/SOLVE/res_stndrd.y.o


SCRIPTS += swn_ord_rg multipleRun swn_ord_sim swn_sym_rg swn_sym_sim \
		   swn_sym_tr swn_ord_tr gspn_sim gspn_rg gspn_tr

swn_ord_rg_SOURCEFILE := WN/EDIT/swn_ord_rg.sh
multipleRun_SOURCEFILE := WN/EDIT/multipleRun.sh
swn_ord_sim_SOURCEFILE := WN/EDIT/swn_ord_sim.sh
swn_sym_rg_SOURCEFILE := WN/EDIT/swn_sym_rg.sh
swn_sym_sim_SOURCEFILE := WN/EDIT/swn_sym_sim.sh
swn_sym_tr_SOURCEFILE := WN/EDIT/swn_sym_tr.sh
swn_ord_tr_SOURCEFILE := WN/EDIT/swn_ord_tr.sh
gspn_sim_SOURCEFILE := WN/EDIT/gspn_sim.sh
gspn_rg_SOURCEFILE := WN/EDIT/gspn_rg.sh
gspn_tr_SOURCEFILE := WN/EDIT/gspn_tr.sh









######################################
### WN package
######################################

TARGETS += WNSIM WNSYMB WNRG WNSRG MDWNRG MDWNSRG WNESRG \
		   GSPNRG GSPNSIM swn-translator PN2ODE

WNSIM_CFLAGS := $(call generate_WN_FLAGS,TOOL_WNSIM,WNSIM)
WNSIM_LDFLAGS:= $(LDFLAGS) -lm
WNSIM_SOURCES := WN/SOURCE/SHARED/service.c \
				 WN/SOURCE/SHARED/ealloc.c \
				 WN/SOURCE/SHARED/token.c \
				 WN/SOURCE/SHARED/dimensio.c \
				 WN/SOURCE/SHARED/errors.c \
				 WN/SOURCE/SHARED/common.c \
				 WN/SOURCE/SHARED/enabling.c \
				 WN/SOURCE/SHARED/fire.c \
				 WN/SOURCE/SHARED/shared1.c \
				 WN/SOURCE/SHARED/shared2.c \
				 WN/SOURCE/SHARED/outdom.c \
				 WN/SOURCE/SHARED/report.c \
				 WN/SOURCE/SHARED/precheck.c \
				 WN/SOURCE/SHARED/degree.c \
				 WN/SOURCE/SHARED/main.c \
				 WN/SOURCE/SIMULAT/eng_wn.c \
				 WN/SOURCE/SIMULAT/lists.c \
				 WN/SOURCE/SIMULAT/engine.c \
				 WN/SOURCE/SIMULAT/distribu.c \
				 WN/SOURCE/SIMULAT/increm.c \
				 WN/SOURCE/SIMULAT/presel.c \
				 WN/SOURCE/SIMULAT/random.c \
				 WN/SOURCE/SIMULAT/stat.c \
				 WN/SOURCE/READNET/read_arc.c \
				 WN/SOURCE/READNET/read_t_c.c \
				 WN/SOURCE/READNET/read_DEF.c \
				 WN/SOURCE/READNET/read_NET.c \
				 WN/SOURCE/READNET/read_t_s.c \
				 WN/SOURCE/READNET/wn_yac.c \
				 WN/TRANSL/wn_grammar.y \
				 WN/TRANSL/wn.l

WNSYMB_CFLAGS := $(call generate_WN_FLAGS,TOOL_WNSYMB,WNSYMB)
WNSYMB_LDFLAGS:= $(LDFLAGS) -lm
WNSYMB_SOURCES := $(WNSIM_SOURCES) \
				  WN/SOURCE/SHARED/split.c \
				  WN/SOURCE/SHARED/group.c \
				  WN/SOURCE/SIMULAT/after.c

# SSOU = WN/SOLVE
# CSOU = WN/SOURCE/SHARED
# GSOU = WN/SOURCE/REACHAB
# RSOU = WN/SOURCE/READNET
# ESOU = WN/SOURCE/REACHAB/E_SRG
# EDT = WN/EDIT
# INC = WN/INCLUDE
# BCK = WN/BACKUP
# TRS = WN/TRANSL
# SPOTSOU = WN/SOURCE/SPOT

WNRG_CFLAGS := $(call generate_WN_FLAGS,TOOL_WNRG,WNRG)
WNRG_LDFLAGS:= $(LDFLAGS) -lm
WNRG_SOURCES := WN/SOURCE/SHARED/service.c \
				WN/SOURCE/SHARED/ealloc.c \
				WN/SOURCE/SHARED/token.c \
				WN/SOURCE/SHARED/dimensio.c \
				WN/SOURCE/SHARED/errors.c \
				WN/SOLVE/compact.c \
				WN/SOURCE/SHARED/common.c \
				WN/SOURCE/SHARED/enabling.c	\
				WN/SOURCE/SHARED/fire.c \
				WN/SOURCE/SHARED/shared1.c \
				WN/SOURCE/SHARED/shared2.c \
				WN/SOURCE/SHARED/outdom.c \
				WN/SOURCE/SHARED/report.c \
				WN/SOURCE/SHARED/precheck.c \
				WN/SOURCE/SHARED/flush.c \
				WN/SOURCE/SHARED/degree.c \
				WN/SOURCE/SHARED/main.c \
				WN/SOURCE/REACHAB/graph_se.c \
				WN/SOURCE/REACHAB/graph.c \
				WN/SOURCE/REACHAB/stack.c \
				WN/SOURCE/REACHAB/convert.c \
				WN/SOURCE/REACHAB/rg_files.c \
				WN/SOURCE/REACHAB/rgengwn.c \
				WN/SOURCE/READNET/read_arc.c \
				WN/SOURCE/READNET/read_t_c.c \
				WN/SOURCE/READNET/read_DEF.c \
				WN/SOURCE/READNET/read_NET.c \
				WN/SOURCE/READNET/read_t_s.c \
				WN/SOURCE/READNET/wn_yac.c \
				WN/TRANSL/wn_grammar.y \
				WN/TRANSL/wn.l

WNSRG_CFLAGS := $(call generate_WN_FLAGS,TOOL_WNSRG,WNSRG)
WNSRG_LDFLAGS:= $(LDFLAGS) -lm
WNSRG_SOURCES := WN/SOURCE/SHARED/service.c \
				 WN/SOURCE/SHARED/ealloc.c \
				 WN/SOURCE/SHARED/token.c \
				 WN/SOURCE/SHARED/dimensio.c \
				 WN/SOURCE/SHARED/errors.c \
				 WN/SOLVE/compact.c \
				 WN/SOURCE/SHARED/common.c \
				 WN/SOURCE/SHARED/enabling.c	\
				 WN/SOURCE/SHARED/fire.c \
				 WN/SOURCE/SHARED/split.c \
				 WN/SOURCE/SHARED/group.c \
				 WN/SOURCE/SHARED/shared1.c \
				 WN/SOURCE/SHARED/shared2.c \
				 WN/SOURCE/SHARED/outdom.c \
				 WN/SOURCE/SHARED/report.c \
				 WN/SOURCE/SHARED/precheck.c \
				 WN/SOURCE/SHARED/flush.c \
				 WN/SOURCE/SHARED/degree.c \
				 WN/SOURCE/SHARED/main.c \
				 WN/SOURCE/REACHAB/graph_se.c \
				 WN/SOURCE/REACHAB/canonic.c \
				 WN/SOURCE/REACHAB/schemes.c \
				 WN/SOURCE/REACHAB/graph.c \
				 WN/SOURCE/REACHAB/stack.c \
				 WN/SOURCE/REACHAB/convert.c \
				 WN/SOURCE/REACHAB/rg_files.c \
				 WN/SOURCE/REACHAB/rgengwn.c \
				 WN/SOURCE/READNET/read_arc.c \
				 WN/SOURCE/READNET/read_t_c.c \
				 WN/SOURCE/READNET/read_NET.c \
				 WN/SOURCE/READNET/read_DEF.c \
				 WN/SOURCE/READNET/read_t_s.c \
				 WN/SOURCE/READNET/wn_yac.c \
				 WN/TRANSL/wn_grammar.y \
				 WN/TRANSL/wn.l

MDWNRG_CFLAGS := $(call generate_WN_FLAGS,TOOL_MDWNRG,MDWNRG)
MDWNRG_LDFLAGS:= $(LDFLAGS) -lm
MDWNRG_SOURCES := WN/SOURCE/SHARED/service.c \
				  WN/SOURCE/SHARED/ealloc.c \
				  WN/SOURCE/SHARED/token.c \
				  WN/SOURCE/SHARED/dimensio.c \
				  WN/SOURCE/SHARED/errors.c \
				  WN/SOLVE/compact.c \
				  WN/SOURCE/SHARED/common.c \
				  WN/SOURCE/SHARED/enabling.c	\
				  WN/SOURCE/SHARED/fire.c \
				  WN/SOURCE/SHARED/shared1.c \
				  WN/SOURCE/SHARED/shared2.c \
				  WN/SOURCE/SHARED/outdom.c \
				  WN/SOURCE/SHARED/report.c \
				  WN/SOURCE/SHARED/precheck.c \
				  WN/SOURCE/SHARED/flush.c \
				  WN/SOURCE/SHARED/degree.c \
				  WN/SOURCE/SHARED/main.c \
				  WN/SOURCE/REACHAB/graph_se.c \
				  WN/SOURCE/REACHAB/graph.c \
				  WN/SOURCE/REACHAB/stack.c \
				  WN/SOURCE/REACHAB/convert.c \
				  WN/SOURCE/REACHAB/rg_files.c \
				  WN/SOURCE/REACHAB/rgengwn.c \
				  WN/SOURCE/READNET/read_arc.c \
				  WN/SOURCE/READNET/read_t_c.c \
				  WN/SOURCE/READNET/read_DEF.c \
				  WN/SOURCE/READNET/read_NET.c \
				  WN/SOURCE/READNET/read_t_s.c \
				  WN/SOURCE/READNET/wn_yac.c \
				  WN/TRANSL/wn_grammar.y \
				  WN/TRANSL/wn.l

MDWNSRG_CFLAGS := $(call generate_WN_FLAGS,TOOL_MDWNSRG,MDWNSRG)
MDWNSRG_LDFLAGS:= $(LDFLAGS) -lm
MDWNSRG_SOURCES := WN/SOURCE/SHARED/service.c \
				   WN/SOURCE/SHARED/ealloc.c \
				   WN/SOURCE/SHARED/token.c \
				   WN/SOURCE/SHARED/dimensio.c \
				   WN/SOURCE/SHARED/errors.c \
				   WN/SOLVE/compact.c \
				   WN/SOURCE/SHARED/common.c \
				   WN/SOURCE/SHARED/enabling.c	\
				   WN/SOURCE/SHARED/fire.c \
				   WN/SOURCE/SHARED/split.c \
				   WN/SOURCE/SHARED/group.c \
				   WN/SOURCE/SHARED/shared1.c \
				   WN/SOURCE/SHARED/shared2.c \
				   WN/SOURCE/SHARED/outdom.c \
				   WN/SOURCE/SHARED/report.c \
				   WN/SOURCE/SHARED/precheck.c \
				   WN/SOURCE/SHARED/flush.c \
				   WN/SOURCE/SHARED/degree.c \
				   WN/SOURCE/SHARED/main.c \
				   WN/SOURCE/REACHAB/graph_se.c \
				   WN/SOURCE/REACHAB/canonic.c \
				   WN/SOURCE/REACHAB/schemes.c \
				   WN/SOURCE/REACHAB/graph.c \
				   WN/SOURCE/REACHAB/stack.c \
				   WN/SOURCE/REACHAB/convert.c \
				   WN/SOURCE/REACHAB/rg_files.c \
				   WN/SOURCE/REACHAB/rgengwn.c \
				   WN/SOURCE/READNET/read_arc.c \
				   WN/SOURCE/READNET/read_t_c.c \
				   WN/SOURCE/READNET/read_NET.c \
				   WN/SOURCE/READNET/read_DEF.c \
				   WN/SOURCE/READNET/read_t_s.c \
				   WN/SOURCE/READNET/wn_yac.c \
				   WN/TRANSL/wn_grammar.y \
				   WN/TRANSL/wn.l

WNESRG_CFLAGS := $(call generate_WN_FLAGS,TOOL_WNESRG,WNESRG)
WNESRG_LDFLAGS:= $(LDFLAGS) -lm
WNESRG_SOURCES := WN/SOURCE/SHARED/service.c \
				  WN/SOURCE/SHARED/ealloc.c \
				  WN/SOURCE/SHARED/token.c \
				  WN/SOURCE/SHARED/dimensio.c \
				  WN/SOURCE/SHARED/errors.c \
				  WN/SOLVE/compact.c \
				  WN/SOURCE/SHARED/common.c \
				  WN/SOURCE/SHARED/enabling.c \
				  WN/SOURCE/SHARED/fire.c \
				  WN/SOURCE/SHARED/split.c \
				  WN/SOURCE/SHARED/group.c \
				  WN/SOURCE/SHARED/shared1.c \
				  WN/SOURCE/SHARED/shared2.c \
				  WN/SOURCE/SHARED/outdom.c \
				  WN/SOURCE/SHARED/report.c \
				  WN/SOURCE/SHARED/precheck.c \
				  WN/SOURCE/SHARED/flush.c \
				  WN/SOURCE/SHARED/degree.c \
				  WN/SOURCE/SHARED/main.c \
				  WN/SOURCE/REACHAB/graph_se.c \
				  WN/SOURCE/REACHAB/canonic.c \
				  WN/SOURCE/REACHAB/schemes.c \
				  WN/SOURCE/REACHAB/esrg_graph.c \
				  WN/SOURCE/REACHAB/stack.c \
				  WN/SOURCE/REACHAB/convert.c \
				  WN/SOURCE/REACHAB/rg_files.c \
				  WN/SOURCE/REACHAB/rgengwn.c \
				  WN/SOURCE/READNET/read_arc.c \
				  WN/SOURCE/READNET/read_t_c.c \
				  WN/SOURCE/READNET/read_NET.c \
				  WN/SOURCE/READNET/read_DEF.c \
				  WN/SOURCE/READNET/read_t_s.c \
				  WN/SOURCE/READNET/wn_yac.c \
				  WN/SOURCE/REACHAB/E_SRG/AFTER_FIRING_CASES.c \
				  WN/SOURCE/REACHAB/E_SRG/GET_SYM.c \
				  WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_GROUPING.c \
				  WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_DECOMPOSING.c \
				  WN/SOURCE/REACHAB/E_SRG/GROUP_STATICS.c \
				  WN/SOURCE/REACHAB/E_SRG/STATIC_LIST.c \
				  WN/SOURCE/REACHAB/E_SRG/MY_ALLOCATION.c \
				  WN/SOURCE/REACHAB/E_SRG/GET_FIRST_EVENT.c \
				  WN/SOURCE/REACHAB/E_SRG/INST_SYM_EVENT.c \
				  WN/SOURCE/REACHAB/E_SRG/EVENTUALITIES.c \
				  WN/SOURCE/REACHAB/E_SRG/READ_WRITE.c \
				  WN/SOURCE/REACHAB/E_SRG/SAVE.c \
				  WN/TRANSL/wn_grammar.y \
				  WN/TRANSL/wn.l

RGMEDD_CFLAGS := $(call generate_WN_FLAGS,TOOL_RGMEDD,RGMEDD) \
				 $(FLEX-INCLUDE) 
RGMEDD_CPPFLAGS := $(RGMEDD_CFLAGS) -I/usr/local/include
RGMEDD_LDFLAGS := $(LDFLAGS) -L/usr/local/lib -lmeddly $(FLEX-LIB)
RGMEDD_SOURCES := WN/SOURCE/SHARED/service.c \
				  WN/SOURCE/SHARED/ealloc.c \
				  WN/SOURCE/SHARED/token.c \
				  WN/SOURCE/SHARED/dimensio.c \
				  WN/SOURCE/SHARED/errors.c \
				  WN/SOLVE/compact.c \
				  WN/SOURCE/SHARED/common.c \
				  WN/SOURCE/SHARED/enabling.c \
				  WN/SOURCE/SHARED/fire.c \
				  WN/SOURCE/SHARED/shared1.c \
				  WN/SOURCE/SHARED/shared2.c \
				  WN/SOURCE/SHARED/outdom.c \
				  WN/SOURCE/SHARED/report.c \
				  WN/SOURCE/SHARED/precheck.c \
				  WN/SOURCE/SHARED/flush.c \
				  WN/SOURCE/SHARED/degree.c \
				  WN/SOURCE/SHARED/mainMEDD.cpp \
				  WN/SOURCE/SHARED/meddEv.cpp \
				  WN/SOURCE/SHARED/general.cpp \
				  WN/SOURCE/REACHAB/graph_se.c \
				  WN/SOURCE/REACHAB/graphMEDD.cpp \
				  WN/SOURCE/REACHAB/stack.c \
				  WN/SOURCE/REACHAB/convert.c \
				  WN/SOURCE/REACHAB/rg_files.c \
				  WN/SOURCE/REACHAB/rgengwn.c \
				  WN/SOURCE/READNET/read_arc.c \
				  WN/SOURCE/READNET/read_t_c.c \
				  WN/SOURCE/READNET/read_DEF.c \
				  WN/SOURCE/READNET/read_NET.c \
				  WN/SOURCE/READNET/read_PIN.c \
				  WN/SOURCE/READNET/read_t_s.c \
				  WN/SOURCE/READNET/wn_yac.c \
				  WN/TRANSL/wn_grammar.y \
				  WN/TRANSL/wn.l \
				  WN/SOURCE/CTL/CTL.cpp \
				  WN/SOURCE/CTL/CTLParser.yy \
				  WN/SOURCE/CTL/CTLLexer.ll 
				  # WN/SOURCE/AUTOMA/AutoParser.yy \
				  # WN/SOURCE/AUTOMA/AutoLexer.l

# Modify the lexer and the parser generators used by the
RGMEDD_LEX_WN/SOURCE/AUTOMA/AutoLexer.l = $(LEX) -P kk --header-file=$(@:.c=.h)
RGMEDD_YACCPP_WN/SOURCE/AUTOMA/AutoParser.yy := byacc -v -p kk -d
RGMEDD_YACCPP_WN/SOURCE/CTL/CTLParser.yy := byacc -p mm -v -d
RGMEDD_LEXPP_WN/SOURCE/CTL/CTLLexer.ll = $(LEXPP) -+ --header-file=$(@:.cpp=.h)
RGMEDD_LD := $(LDPP)

$(OBJDIR)/RGMEDD/WN/SOURCE/CTL/CTLParser.yy.o: $(OBJDIR)/RGMEDD/WN/SOURCE/CTL/CTLLexer.ll.cpp

$(OBJDIR)/RGMEDD/WN/SOURCE/CTL/CTLLexer.ll.o: $(OBJDIR)/RGMEDD/WN/SOURCE/CTL/CTLParser.yy.cpp

$(OBJDIR)/RGMEDD/WN/SOURCE/AUTOMA/AutoLexer.l.o: $(OBJDIR)/RGMEDD/WN/SOURCE/AUTOMA/AutoParser.yy.cpp

$(OBJDIR)/RGMEDD/WN/SOURCE/AUTOMA/AutoParser.yy.o: $(OBJDIR)/RGMEDD/WN/SOURCE/AUTOMA/AutoLexer.l.c

#### RGMEDD version 2 ########################################

RGMEDD2_CFLAGS := $(CFLAGS) $(call generate_WN_FLAGS,TOOL_RGMEDD2,RGMEDD2) \
				          $(FLEX-INCLUDE) 
RGMEDD2_CPPFLAGS := $(CPPFLAGS) $(ENABLE_Cxx17) -Wno-deprecated-register \
                    -I/usr/local/include $(INCLUDE_GMP_LIB) \
                    $(RGMEDD2_CFLAGS) -I/usr/local/include 
                    
                    # -D_GLIBCXX_DEBUG=1
RGMEDD2_LDFLAGS := -L/usr/local/lib $(LDFLAGS) $(FLEX-LIB) -lmeddly $(LINK_GMP_LIB)
RGMEDD2_SOURCES := WN/SOURCE/SHARED/service.c \
				   WN/SOURCE/SHARED/ealloc.c \
				   WN/SOURCE/SHARED/token.c \
				   WN/SOURCE/SHARED/dimensio.c \
				   WN/SOURCE/SHARED/errors.c \
				   WN/SOLVE/compact.c \
				   WN/SOURCE/SHARED/common.c \
				   WN/SOURCE/SHARED/enabling.c \
				   WN/SOURCE/SHARED/fire.c \
				   WN/SOURCE/SHARED/shared1.c \
				   WN/SOURCE/SHARED/shared2.c \
				   WN/SOURCE/SHARED/outdom.c \
				   WN/SOURCE/SHARED/report.c \
				   WN/SOURCE/SHARED/precheck.c \
				   WN/SOURCE/SHARED/flush.c \
				   WN/SOURCE/SHARED/degree.c \
				   WN/SOURCE/REACHAB/graph_se.c \
				   WN/SOURCE/REACHAB/stack.c \
				   WN/SOURCE/REACHAB/convert.c \
				   WN/SOURCE/REACHAB/rg_files.c \
				   WN/SOURCE/REACHAB/rgengwn.c \
				   WN/SOURCE/READNET/read_arc.c \
				   WN/SOURCE/READNET/read_t_c.c \
				   WN/SOURCE/READNET/read_DEF.c \
				   WN/SOURCE/READNET/read_NET.c \
				   WN/SOURCE/READNET/read_PIN.c \
				   WN/SOURCE/READNET/read_t_s.c \
				   WN/SOURCE/READNET/wn_yac.c \
				   WN/TRANSL/wn_grammar.y \
				   WN/TRANSL/wn.l \
				   WN/SOURCE/SHARED/mainMEDD2.cpp \
				   WN/SOURCE/RGMEDD2/nsf_subtree.cpp \
				   WN/SOURCE/RGMEDD2/varorders.cpp \
				   WN/SOURCE/RGMEDD2/varorders_bgl.cpp \
				   WN/SOURCE/RGMEDD2/varorders_meta.cpp \
				   WN/SOURCE/RGMEDD2/meddEv.cpp \
				   WN/SOURCE/RGMEDD2/general.cpp \
				   WN/SOURCE/RGMEDD2/graphMEDD.cpp \
				   WN/SOURCE/RGMEDD2/CTL.cpp \
				   WN/SOURCE/RGMEDD2/CTLParser.yy \
				   WN/SOURCE/RGMEDD2/CTLLexer.ll 

# Modify the lexer and the parser generators used by the
# RGMEDD2_LEX_WN/SOURCE/AUTOMA/AutoLexer.l = $(LEX) -P kk --header-file=$(@:.c=.h)
# RGMEDD2_YACCPP_WN/SOURCE/AUTOMA/AutoParser.yy := byacc -v -p kk -d
RGMEDD2_YACCPP_WN/SOURCE/RGMEDD2/CTLParser.yy := byacc -p mm -v -d
RGMEDD2_LEXPP_WN/SOURCE/RGMEDD2/CTLLexer.ll = $(LEXPP) -+ -P mm --header-file=$(@:.cpp=.h)
RGMEDD2_LD := $(LDPP) -shared-libgcc

$(OBJDIR)/RGMEDD2/WN/SOURCE/RGMEDD2/CTLParser.yy.o: $(OBJDIR)/RGMEDD2/WN/SOURCE/RGMEDD2/CTLLexer.ll.cpp

$(OBJDIR)/RGMEDD2/WN/SOURCE/RGMEDD2/CTLLexer.ll.o: $(OBJDIR)/RGMEDD2/WN/SOURCE/RGMEDD2/CTLParser.yy.cpp

# $(OBJDIR)/RGMEDD2/WN/SOURCE/CTL/CTLLexer.ll.o: $(OBJDIR)/RGMEDD2/WN/SOURCE/CTL/CTLParser.yy.cpp

# $(OBJDIR)/RGMEDD2/WN/SOURCE/AUTOMA/AutoLexer.l.o: $(OBJDIR)/RGMEDD/WN/SOURCE/AUTOMA/AutoParser.yy.cpp

# $(OBJDIR)/RGMEDD2/WN/SOURCE/AUTOMA/AutoParser.yy.o: $(OBJDIR)/RGMEDD/WN/SOURCE/AUTOMA/AutoLexer.l.c

# ifdef HAS_LP_SOLVE_LIB
#   RGMEDD2_CPPFLAGS := $(RGMEDD2_CPPFLAGS) $(INCLUDE_LP_SOLVE_LIB)
#   RGMEDD2_LDFLAGS := $(RGMEDD2_LDFLAGS) $(LINK_LP_SOLVE_LIB) 
# endif

# ifdef USE_RGMEDD2
# TARGETS += RGMEDD2
 #  ifneq ("$(wildcard ~/.elvio-temporary-hack)","")
 #  	 $(warning "Using both RGMEDD & RGMEDD2.")
 #  	RGMEDD2_CPPFLAGS := -I/Users/elvio/Desktop/meddly2015/src/ $(RGMEDD2_CPPFLAGS) 
	# RGMEDD2_LDFLAGS := -L/Users/elvio/Desktop/meddly2015/src/.libs/ $(RGMEDD2_LDFLAGS) 

 #  	TARGETS += RGMEDD
 #  endif
# else
#   TARGETS += RGMEDD
# endif

#### RGMEDD version 3 ########################################

RGMEDD3_CFLAGS := $(CFLAGS) $(call generate_WN_FLAGS,TOOL_RGMEDD3,RGMEDD3) \
                  $(FLEX-INCLUDE) 
RGMEDD3_CPPFLAGS := $(CPPFLAGS) $(ENABLE_Cxx14) \
                    -I/usr/local/include $(INCLUDE_GMP_LIB) \
                    $(RGMEDD3_CFLAGS) -I/usr/local/include 
                    
                    # -D_GLIBCXX_DEBUG=1 /usr/local/lib/libmeddly.a
RGMEDD3_LDFLAGS := -L/usr/local/lib $(LDFLAGS) $(FLEX-LIB) -lmeddly $(LINK_GMP_LIB)
          #-lmeddly 
RGMEDD3_SOURCES := WN/SOURCE/SHARED/service.c \
           WN/SOURCE/SHARED/ealloc.c \
           WN/SOURCE/SHARED/token.c \
           WN/SOURCE/SHARED/dimensio.c \
           WN/SOURCE/SHARED/errors.c \
           WN/SOLVE/compact.c \
           WN/SOURCE/SHARED/common.c \
           WN/SOURCE/SHARED/enabling.c \
           WN/SOURCE/SHARED/fire.c \
           WN/SOURCE/SHARED/shared1.c \
           WN/SOURCE/SHARED/shared2.c \
           WN/SOURCE/SHARED/outdom.c \
           WN/SOURCE/SHARED/report.c \
           WN/SOURCE/SHARED/precheck.c \
           WN/SOURCE/SHARED/flush.c \
           WN/SOURCE/SHARED/degree.c \
           WN/SOURCE/REACHAB/graph_se.c \
           WN/SOURCE/REACHAB/stack.c \
           WN/SOURCE/REACHAB/convert.c \
           WN/SOURCE/REACHAB/rg_files.c \
           WN/SOURCE/REACHAB/rgengwn.c \
           WN/SOURCE/READNET/read_arc.c \
           WN/SOURCE/READNET/read_t_c.c \
           WN/SOURCE/READNET/read_DEF.c \
           WN/SOURCE/READNET/read_NET.c \
           WN/SOURCE/READNET/read_PIN.c \
           WN/SOURCE/READNET/read_t_s.c \
           WN/SOURCE/READNET/wn_yac.c \
           WN/TRANSL/wn_grammar.y \
           WN/TRANSL/wn.l \
           WN/SOURCE/RGMEDD3/mainMEDD3.cpp \
           WN/SOURCE/RGMEDD3/utils/mt19937-64.c \
           WN/SOURCE/RGMEDD3/varorders.cpp \
           WN/SOURCE/RGMEDD3/varorders_bgl.cpp \
           WN/SOURCE/RGMEDD3/varorders_meta.cpp \
           WN/SOURCE/RGMEDD3/varorders_soups.cpp \
           WN/SOURCE/RGMEDD3/varorders_pbasis.cpp \
           WN/SOURCE/RGMEDD3/meddEv.cpp \
           WN/SOURCE/RGMEDD3/general.cpp \
           WN/SOURCE/RGMEDD3/graphMEDD.cpp \
           WN/SOURCE/RGMEDD3/CTL.cpp \
           WN/SOURCE/RGMEDD3/CTLParser.yy \
           WN/SOURCE/RGMEDD3/CTLLexer.ll 

# Modify the lexer and the parser generators used by the
# RGMEDD3_LEX_WN/SOURCE/AUTOMA/AutoLexer.l = $(LEX) -P kk --header-file=$(@:.c=.h)
# RGMEDD3_YACCPP_WN/SOURCE/AUTOMA/AutoParser.yy := byacc -v -p kk -d
RGMEDD3_YACCPP_WN/SOURCE/RGMEDD3/CTLParser.yy := byacc -p mm -v -d
RGMEDD3_LEXPP_WN/SOURCE/RGMEDD3/CTLLexer.ll = $(LEXPP) -+ -P mm --header-file=$(@:.cpp=.h)
RGMEDD3_LD := $(LDPP) -shared-libgcc
RGMEDD3_CPPFLAGS := $(RGMEDD3_CPPFLAGS) -I.

$(OBJDIR)/RGMEDD3/WN/SOURCE/RGMEDD3/CTLParser.yy.o: $(OBJDIR)/RGMEDD3/WN/SOURCE/RGMEDD3/CTLLexer.ll.cpp

$(OBJDIR)/RGMEDD3/WN/SOURCE/RGMEDD3/CTLLexer.ll.o: $(OBJDIR)/RGMEDD3/WN/SOURCE/RGMEDD3/CTLParser.yy.cpp

# $(OBJDIR)/RGMEDD3/WN/SOURCE/RGMEDD3/CTLLexer.h: $(OBJDIR)/RGMEDD3/WN/SOURCE/RGMEDD3/CTLLexer.ll.cpp


ifdef HAS_LP_SOLVE_LIB
  RGMEDD3_CPPFLAGS := $(RGMEDD3_CPPFLAGS) $(INCLUDE_LP_SOLVE_LIB)
  RGMEDD3_LDFLAGS := $(RGMEDD3_LDFLAGS) $(LINK_LP_SOLVE_LIB) 
endif

# ifdef USE_RGMEDD3
TARGETS += RGMEDD3
# endif

###################################################################################

GSPNRG_CFLAGS := $(call generate_WN_FLAGS,TOOL_GSPNRG,GSPNRG)
GSPNRG_LDFLAGS:= $(LDFLAGS) -lm
GSPNRG_SOURCES := WN/SOURCE/SHARED/service.c \
				  WN/SOURCE/SHARED/ealloc.c \
				  WN/SOURCE/SHARED/token.c \
				  WN/SOURCE/SHARED/dimensio.c \
				  WN/SOURCE/SHARED/errors.c \
				  WN/SOLVE/compact.c \
				  WN/SOURCE/SHARED/common.c \
				  WN/SOURCE/SHARED/enabling.c	\
				  WN/SOURCE/SHARED/fire.c \
				  WN/SOURCE/SHARED/split.c \
				  WN/SOURCE/SHARED/group.c \
				  WN/SOURCE/SHARED/shared1.c \
				  WN/SOURCE/SHARED/shared2.c \
				  WN/SOURCE/SHARED/outdom.c \
				  WN/SOURCE/SHARED/report.c \
				  WN/SOURCE/SHARED/precheck.c \
				  WN/SOURCE/SHARED/flush.c \
				  WN/SOURCE/SHARED/degree.c \
				  WN/SOURCE/SHARED/main.c \
				  WN/SOURCE/REACHAB/graph_se.c \
				  WN/SOURCE/REACHAB/canonic.c \
				  WN/SOURCE/REACHAB/schemes.c \
				  WN/SOURCE/REACHAB/graph.c \
				  WN/SOURCE/REACHAB/stack.c \
				  WN/SOURCE/REACHAB/convert.c \
				  WN/SOURCE/REACHAB/rg_files.c \
				  WN/SOURCE/REACHAB/rgengwn.c \
				  WN/SOURCE/READNET/read_arc.c \
				  WN/SOURCE/READNET/read_t_c.c \
				  WN/SOURCE/READNET/read_NET.c \
				  WN/SOURCE/READNET/read_DEF.c \
				  WN/SOURCE/READNET/read_t_s.c \
				  WN/SOURCE/READNET/wn_yac.c \
				  WN/TRANSL/gspn_grammar.y \
				  WN/TRANSL/wn.l

GSPNSIM_CFLAGS := $(call generate_WN_FLAGS,TOOL_GSPNSIM,GSPNSIM)
GSPNSIM_LDFLAGS:= $(LDFLAGS) -lm
GSPNSIM_SOURCES := WN/SOURCE/SHARED/service.c \
				   WN/SOURCE/SHARED/ealloc.c \
				   WN/SOURCE/SHARED/token.c \
				   WN/SOURCE/SHARED/dimensio.c \
				   WN/SOURCE/SHARED/errors.c \
				   WN/SOURCE/SHARED/common.c \
				   WN/SOURCE/SHARED/enabling.c	\
				   WN/SOURCE/SHARED/fire.c \
				   WN/SOURCE/SHARED/shared1.c \
				   WN/SOURCE/SHARED/shared2.c \
				   WN/SOURCE/SHARED/outdom.c \
				   WN/SOURCE/SHARED/report.c \
				   WN/SOURCE/SHARED/precheck.c \
				   WN/SOURCE/SHARED/flush.c \
				   WN/SOURCE/SHARED/degree.c \
				   WN/SOURCE/SHARED/main.c \
				   WN/SOURCE/SHARED/split.c \
				   WN/SOURCE/SHARED/group.c \
				   WN/SOURCE/SIMULAT/eng_wn.c \
				   WN/SOURCE/SIMULAT/lists.c \
				   WN/SOURCE/SIMULAT/engine.c \
				   WN/SOURCE/SIMULAT/distribu.c \
				   WN/SOURCE/SIMULAT/increm.c \
				   WN/SOURCE/SIMULAT/presel.c \
				   WN/SOURCE/SIMULAT/random.c \
				   WN/SOURCE/SIMULAT/stat.c \
				   WN/SOURCE/SIMULAT/after.c \
				   WN/SOURCE/READNET/read_arc.c \
				   WN/SOURCE/READNET/read_t_c.c \
				   WN/SOURCE/READNET/read_DEF.c \
				   WN/SOURCE/READNET/read_NET.c \
				   WN/SOURCE/READNET/read_t_s.c \
				   WN/SOURCE/READNET/wn_yac.c \
				   WN/TRANSL/gspn_grammar.y \
				   WN/TRANSL/wn.l
  
swn-translator_CFLAGS := $(CFLAGS) -DVERSION=1
swn-translator_LDFLAGS := $(LDFLAGS) $(FLEX-LIB)
swn-translator_SOURCES := WN/SOURCE/UNFOLDING/alloc.c \
					 	  WN/SOURCE/UNFOLDING/lista-adt.c \
					 	  WN/SOURCE/UNFOLDING/mainTransl.c \
					 	  WN/SOURCE/UNFOLDING/save.c \
					 	  WN/SOURCE/UNFOLDING/layer.c \
					 	  WN/SOURCE/UNFOLDING/multiset-adt.c \
					 	  WN/SOURCE/UNFOLDING/unfold.c \
					 	  WN/SOURCE/UNFOLDING/assignment.c \
					 	  WN/SOURCE/UNFOLDING/evaluate.c \
					 	  WN/SOURCE/UNFOLDING/load.c \
					 	  WN/SOURCE/UNFOLDING/unfolding-obj.c \
					 	  WN/SOURCE/UNFOLDING/set-adt.c \
					 	  WN/SOURCE/UNFOLDING/attrib-yacc.c \
					 	  WN/SOURCE/UNFOLDING/util-transl.c \
					 	  WN/SOURCE/UNFOLDING/lisp-obj_yacc.y \
					 	  WN/SOURCE/UNFOLDING/lisp-obj_lex.l \
					 	  WN/SOURCE/UNFOLDING/SWN-types.c \
					 	  WN/SOURCE/UNFOLDING/global.c \

					 	  
PN2ODE_LDFLAGS := $(LDFLAGS) $(FLEX-LIB)
PN2ODE_CFLAGS := $(call generate_WN_FLAGS,TOOL_PN2ODE,PN2ODE)
PN2ODE_CPPFLAGS := $(PN2ODE_CFLAGS) $(ENABLE_Cxx17) -I.
PN2ODE_LD := $(LDPP)
PN2ODE_SOURCES := WN/SOURCE/SHARED/service.c \
				  WN/SOURCE/SHARED/ealloc.c \
				  WN/SOURCE/SHARED/token.c \
				  WN/SOURCE/SHARED/dimensio.c \
				  WN/SOURCE/SHARED/errors.c \
				  WN/SOLVE/compact.c \
				  WN/SOURCE/SHARED/common.c \
				  WN/SOURCE/SHARED/enabling.c	\
				  WN/SOURCE/SHARED/fire.c \
				  WN/SOURCE/SHARED/shared1.c \
				  WN/SOURCE/SHARED/shared2.c \
				  WN/SOURCE/SHARED/outdom.c \
				  WN/SOURCE/SHARED/report.c \
				  WN/SOURCE/SHARED/precheck.c \
				  WN/SOURCE/SHARED/flush.c \
				  WN/SOURCE/SHARED/degree.c \
				  WN/SOURCE/SHARED/mainODE.cpp \
				  WN/SOURCE/REACHAB/graph_se.c \
				  WN/SOURCE/REACHAB/buildODE.cpp\
				  WN/SOURCE/REACHAB/general.cpp\
				  WN/SOURCE/REACHAB/stack.c \
				  WN/SOURCE/REACHAB/convert.c \
				  WN/SOURCE/REACHAB/rg_files.c \
				  WN/SOURCE/REACHAB/rgengwn.c \
				  WN/SOURCE/READNET/read_arc.c \
				  WN/SOURCE/READNET/read_t_c.c \
				  WN/SOURCE/READNET/read_DEF.c \
				  WN/SOURCE/READNET/read_NET.c \
				  WN/SOURCE/READNET/read_PIN.c \
				  WN/SOURCE/READNET/read_t_s.c \
				  WN/SOURCE/READNET/wn_yac.c \
				  WN/TRANSL/wn_grammar.y \
				  WN/TRANSL/wn.l \
				  ODE-SDE/readingObjectiveFunction.ll\
				  ODE-SDE/readingObjectiveFunction.yy\
				  ODE-SDE/ObjectiveFunction.cpp
				  
# Fix for byacc: use "-b readingObjectiveFunction" argument
PN2ODE_YACCPP_ODE-SDE/readingObjectiveFunction.yy = byacc -p mm -v -d 
PN2ODE_LEXPP_ODE-SDE/readingObjectiveFunction.ll = $(LEXPP) -+ --header-file=$(@:.cpp=.hpp)				  
				  
$(OBJDIR)/PN2ODE/ODE-SDE/readingObjectiveFunction.yy.o: $(OBJDIR)/PN2ODE/ODE-SDE/readingObjectiveFunction.ll.cpp

$(OBJDIR)/PN2ODE/ODE-SDE/readingObjectiveFunction.ll.o: $(OBJDIR)/PN2ODE/ODE-SDE/readingObjectiveFunction.yy.cpp


SCRIPTS += PN2ODE.sh
PN2ODE.sh_SOURCEFILE := ODE-SDE/PN2ODE.sh

INSTALLEDSOURCES := 	ODE-SDE/class.hpp \
			ODE-SDE/class.cpp \
			ODE-SDE/lsode.hpp \
			ODE-SDE/lsode.cpp \
			ODE-SDE/makefile \
			ODE-SDE/automa.hpp \
			ODE-SDE/automa.cpp \
			ODE-SDE/readingAutomaton.yy \
			ODE-SDE/readingAutomaton.ll


### Additional custom rules ###
WN/TRANSL/wn_grammar.y: WN/TRANSL/wn.head WN/TRANSL/common.yac WN/TRANSL/wn.colour \
						WN/TRANSL/wn.domain WN/TRANSL/wn.dynamic_subclass \
						WN/TRANSL/wn.initial_marking WN/TRANSL/wn.function \
						WN/TRANSL/wn.predicate WN/TRANSL/yyerror.c
	@echo "  [GEN] " $@
	@$(MKDIR) $(dir $@)
	@cat $^ > $@

WN/TRANSL/gspn_grammar.y: WN/TRANSL/gspn.head WN/TRANSL/common.yac WN/TRANSL/gspn.tguard \
						  WN/TRANSL/gspn.initial_marking WN/TRANSL/gspn.function \
						  WN/TRANSL/gspn.predicate WN/TRANSL/yyerror.c
	@echo "  [GEN] " $@
	@$(MKDIR) $(dir $@)
	@cat $^ > $@



SCRIPTS += v_graph swn_sym_dsrg swn_sym_esrg ord_rgMEDD pnml2netdef

v_graph_SOURCEFILE := WN/UTIL/graph.sh
swn_sym_dsrg_SOURCEFILE := WN/UTIL/swn_sym_dsrg.sh
swn_sym_esrg_SOURCEFILE := WN/UTIL/swn_sym_esrg.sh
ord_rgMEDD_SOURCEFILE := WN/UTIL/ord_rgMEDD.sh
pnml2netdef_SOURCEFILE := WN/UTIL/pnml2netdef









# libgspnSSP.a_SOURCES := WN/SOURCE/SPOT/PROPS/TRANS.tab.c NO GET THE RIGHT yacc/lex files\
# 						WN/SOURCE/SPOT/PROPS/TRANS.c \
# 						WN/SOURCE/SPOT/PROPS/PROPG.tab.c \
# 						WN/SOURCE/SPOT/PROPS/PROPL.c \
# 						WN/SOURCE/SPOT/PROPS/FSymFmTokens2.c \
# 						WN/SOURCE/SPOT/PROPS/MultiSetOp.c \
# 						WN/SOURCE/SPOT/PROPS/STATIC_CONF.c \
# 						WN/SOURCE/SPOT/PROPS/GroupMark.c \
# 						WN/SOURCE/SPOT/PROPS/INCLUSION.c \
# 						WN/SOURCE/SPOT/PROPS/cache.c \
# 						WN/SOURCE/SHARED/main.c \
# 						WN/SOURCE/SHARED/service.c \
# 						WN/SOURCE/SHARED/token.c \
# 						WN/SOURCE/SHARED/dimensio.c \
# 						WN/SOURCE/SHARED/errors.c \
# 						WN/SOLVE/compact.c \
# 						WN/SOURCE/SHARED/common.c \
# 						WN/SOURCE/SHARED/enabling.c \
# 						WN/SOURCE/SHARED/fire.c \
# 						WN/SOURCE/SHARED/split.c \
# 						WN/SOURCE/SHARED/group.c \
# 						WN/SOURCE/SHARED/shared1.c \
# 						WN/SOURCE/SHARED/shared2.c \
# 						WN/SOURCE/SHARED/outdom.c \
# 						WN/SOURCE/SHARED/report.c \
# 						WN/SOURCE/SHARED/precheck.c \
# 						WN/SOURCE/SHARED/flush.c \
# 						WN/SOURCE/SHARED/degree.c \
# 						WN/SOURCE/REACHAB/graph_se.c \
# 						WN/SOURCE/REACHAB/canonic.c \
# 						WN/SOURCE/REACHAB/schemes.c \
# 						WN/SOURCE/REACHAB/dsrg_graph.c \
# 						WN/SOURCE/REACHAB/stack.c \
# 						WN/SOURCE/REACHAB/convert.c \
# 						WN/SOURCE/REACHAB/rg_files.c \
# 						WN/SOURCE/REACHAB/rgengwn.c \
# 						WN/SOURCE/READNET/read_arc.c \
# 						WN/SOURCE/READNET/read_t_c.c \
# 						WN/SOURCE/READNET/read_NET.c \
# 						WN/SOURCE/READNET/read_DEF.c \
# 						WN/SOURCE/READNET/read_t_s.c \
# 						WN/SOURCE/READNET/wn_yac.c \
# 						WN/SOURCE/REACHAB/E_SRG/AFTER_FIRING_CASES.c \
# 						WN/SOURCE/REACHAB/E_SRG/GET_SYM.c \
# 						WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_GROUPING.c \
# 						WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_DECOMPOSING.c \
# 						WN/SOURCE/REACHAB/E_SRG/GROUP_STATICS.c \
# 						WN/SOURCE/REACHAB/E_SRG/STATIC_LIST.c \
# 						WN/SOURCE/REACHAB/E_SRG/MY_ALLOCATION.c \
# 						WN/SOURCE/REACHAB/E_SRG/GET_FIRST_EVENT.c \
# 						WN/SOURCE/REACHAB/E_SRG/INST_SYM_EVENT.c \
# 						WN/SOURCE/REACHAB/E_SRG/EVENTUALITIES.c \
# 						WN/SOURCE/REACHAB/E_SRG/READ_WRITE.c \
# 						WN/SOURCE/REACHAB/E_SRG/SAVE.c \
# 						WN/TRANSL/wn_grammar.y \
# 						WN/TRANSL/wn.l

# libgspnSSP.a_LD := ar r
# #libgspnSSP.a_LDFLAGS := $(LDFLAGS) $(FLEX-LIB)
# libgspnSSP.a_CFLAGS := $(call generate_WN_FLAGS,TOOL_LIBGSPNSSP,libgspnSSP.a) \
# 					   $(FLEX-INCLUDE) $(GLIB-INCLUDE)

#LIBRARIES += libgspnSSP.a



libgspnRG.a_CFLAGS := $(call generate_WN_FLAGS,TOOL_LIBGSPNRG,libgspnRG.a) \
					  $(FLEX-INCLUDE) 
libgspnRG.a_SOURCES := WN/SOURCE/SPOT/properties.c \
					   WN/SOURCE/SHARED/main.c \
					   WN/SOURCE/SHARED/service.c \
					   WN/SOURCE/SHARED/token.c \
					   WN/SOURCE/SHARED/dimensio.c \
					   WN/SOURCE/SHARED/errors.c \
					   WN/SOLVE/compact.c \
					   WN/SOURCE/SHARED/common.c \
					   WN/SOURCE/SHARED/enabling.c	\
					   WN/SOURCE/SHARED/fire.c \
					   WN/SOURCE/SHARED/shared1.c \
					   WN/SOURCE/SHARED/shared2.c \
					   WN/SOURCE/SHARED/outdom.c \
					   WN/SOURCE/SHARED/report.c \
					   WN/SOURCE/SHARED/precheck.c \
					   WN/SOURCE/SHARED/flush.c \
					   WN/SOURCE/SHARED/degree.c \
					   WN/SOURCE/REACHAB/graph_se.c \
					   WN/SOURCE/REACHAB/graph.c \
					   WN/SOURCE/REACHAB/stack.c \
					   WN/SOURCE/REACHAB/convert.c \
					   WN/SOURCE/REACHAB/rg_files.c \
					   WN/SOURCE/REACHAB/rgengwn.c \
					   WN/SOURCE/READNET/read_arc.c \
					   WN/SOURCE/READNET/read_t_c.c \
					   WN/SOURCE/READNET/read_DEF.c \
					   WN/SOURCE/READNET/read_NET.c \
					   WN/SOURCE/READNET/read_t_s.c \
					   WN/SOURCE/READNET/wn_yac.c \
					   WN/TRANSL/wn_grammar.y \
					   WN/TRANSL/wn.l
LIBRARIES += libgspnRG.a


libgspnSRG.a_CFLAGS := $(call generate_WN_FLAGS,TOOL_LIBGSPNSRG,libgspnSRG.a) \
					   $(FLEX-INCLUDE) 
libgspnSRG.a_SOURCES := WN/SOURCE/SPOT/properties.c \
						WN/SOURCE/SHARED/main.c \
						WN/SOURCE/SHARED/service.c \
						WN/SOURCE/SHARED/token.c \
						WN/SOURCE/SHARED/dimensio.c \
						WN/SOURCE/SHARED/errors.c \
						WN/SOURCE/SHARED/ealloc.c \
						WN/SOLVE/compact.c \
						WN/SOURCE/SHARED/common.c \
						WN/SOURCE/SHARED/enabling.c	\
						WN/SOURCE/SHARED/fire.c \
						WN/SOURCE/SHARED/split.c \
						WN/SOURCE/SHARED/group.c \
						WN/SOURCE/SHARED/shared1.c \
						WN/SOURCE/SHARED/shared2.c \
						WN/SOURCE/SHARED/outdom.c \
						WN/SOURCE/SHARED/report.c \
						WN/SOURCE/SHARED/precheck.c \
						WN/SOURCE/SHARED/flush.c \
						WN/SOURCE/SHARED/degree.c \
						WN/SOURCE/REACHAB/graph_se.c \
						WN/SOURCE/REACHAB/canonic.c \
						WN/SOURCE/REACHAB/schemes.c \
						WN/SOURCE/REACHAB/graph.c \
						WN/SOURCE/REACHAB/stack.c \
						WN/SOURCE/REACHAB/convert.c \
						WN/SOURCE/REACHAB/rg_files.c \
						WN/SOURCE/REACHAB/rgengwn.c \
						WN/SOURCE/READNET/read_arc.c \
						WN/SOURCE/READNET/read_t_c.c \
						WN/SOURCE/READNET/read_NET.c \
						WN/SOURCE/READNET/read_DEF.c \
						WN/SOURCE/READNET/read_t_s.c \
						WN/SOURCE/READNET/wn_yac.c \
						WN/TRANSL/wn_grammar.y \
						WN/TRANSL/wn.l
LIBRARIES += libgspnSRG.a



libgspnMCESRG.a_CFLAGS := $(call generate_WN_FLAGS,TOOL_LIBGSPNMCESRG,libgspnMCESRG.a) \
					      $(FLEX-INCLUDE) 
libgspnMCESRG.a_SOURCES := WN/SOURCE/SHARED/main.c \
						   WN/SOURCE/SHARED/ealloc.c \
						   WN/SOURCE/SHARED/service.c \
						   WN/SOURCE/SHARED/token.c \
						   WN/SOURCE/SHARED/dimensio.c \
						   WN/SOURCE/SHARED/errors.c \
						   WN/SOLVE/compact.c \
						   WN/SOURCE/SHARED/common.c \
						   WN/SOURCE/SHARED/enabling.c	\
						   WN/SOURCE/SHARED/fire.c \
						   WN/SOURCE/SHARED/split.c \
						   WN/SOURCE/SHARED/group.c \
						   WN/SOURCE/SHARED/shared1.c \
						   WN/SOURCE/SHARED/shared2.c \
						   WN/SOURCE/SHARED/outdom.c \
						   WN/SOURCE/SHARED/report.c \
						   WN/SOURCE/SHARED/precheck.c \
						   WN/SOURCE/SHARED/flush.c \
						   WN/SOURCE/SHARED/degree.c \
						   WN/SOURCE/REACHAB/esrg_graph.c \
						   WN/SOURCE/REACHAB/canonic.c \
						   WN/SOURCE/REACHAB/convert.c \
						   WN/SOURCE/REACHAB/schemes.c \
						   WN/SOURCE/REACHAB/stack.c \
						   WN/SOURCE/REACHAB/rg_files.c \
						   WN/SOURCE/REACHAB/rgengwn.c \
						   WN/SOURCE/REACHAB/graph_se.c \
						   WN/SOURCE/READNET/read_arc.c \
						   WN/SOURCE/READNET/read_t_c.c \
						   WN/SOURCE/READNET/read_NET.c \
						   WN/SOURCE/READNET/read_DEF.c \
						   WN/SOURCE/READNET/read_t_s.c \
						   WN/SOURCE/READNET/wn_yac.c \
						   WN/SOURCE/REACHAB/E_SRG/AFTER_FIRING_CASES.c \
						   WN/SOURCE/REACHAB/E_SRG/GET_SYM.c \
						   WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_GROUPING.c \
						   WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_DECOMPOSING.c \
						   WN/SOURCE/REACHAB/E_SRG/GROUP_STATICS.c \
						   WN/SOURCE/REACHAB/E_SRG/STATIC_LIST.c \
						   WN/SOURCE/REACHAB/E_SRG/MY_ALLOCATION.c \
						   WN/SOURCE/REACHAB/E_SRG/GET_FIRST_EVENT.c \
						   WN/SOURCE/REACHAB/E_SRG/INST_SYM_EVENT.c \
						   WN/SOURCE/REACHAB/E_SRG/EVENTUALITIES.c \
						   WN/SOURCE/REACHAB/E_SRG/READ_WRITE.c \
						   WN/SOURCE/REACHAB/E_SRG/SAVE.c \
						   WN/TRANSL/wn_grammar.y \
						   WN/TRANSL/wn.l
LIBRARIES += libgspnMCESRG.a




libgspnMCDSRG.a_CFLAGS := $(call generate_WN_FLAGS,TOOL_LIBGSPNMCDSRG,libgspnMCDSRG.a) \
						  $(FLEX-INCLUDE) $(GLIB-INCLUDE)
libgspnMCDSRG.a_SOURCES := WN/SOURCE/SPOT/PROPS/TRANS.y \
						   WN/SOURCE/SPOT/PROPS/TRANS.l \
						   WN/SOURCE/SPOT/PROPS/PROPG.y \
						   WN/SOURCE/SPOT/PROPS/PROPL.l \
						   WN/SOURCE/SPOT/PROPS/FSymFmTokens2.c \
						   WN/SOURCE/SPOT/PROPS/MultiSetOp.c \
						   WN/SOURCE/SPOT/PROPS/STATIC_CONF.c \
						   WN/SOURCE/SPOT/PROPS/GroupMark.c \
						   WN/SOURCE/SPOT/PROPS/Vectors.c \
						   WN/SOURCE/SPOT/PROPS/INCLUSION.c \
						   WN/SOURCE/SPOT/PROPS/cache.c \
						   WN/SOURCE/SHARED/main.c \
						   WN/SOURCE/SHARED/ealloc.c \
						   WN/SOURCE/SHARED/service.c \
						   WN/SOURCE/SHARED/token.c \
						   WN/SOURCE/SHARED/dimensio.c \
						   WN/SOURCE/SHARED/errors.c \
						   WN/SOLVE/compact.c \
						   WN/SOURCE/SHARED/common.c \
						   WN/SOURCE/SHARED/enabling.c \
						   WN/SOURCE/SHARED/fire.c \
						   WN/SOURCE/SHARED/split.c \
						   WN/SOURCE/SHARED/group.c \
						   WN/SOURCE/SHARED/shared1.c \
						   WN/SOURCE/SHARED/shared2.c \
						   WN/SOURCE/SHARED/outdom.c \
						   WN/SOURCE/SHARED/report.c \
						   WN/SOURCE/SHARED/precheck.c \
						   WN/SOURCE/SHARED/flush.c \
						   WN/SOURCE/SHARED/degree.c \
						   WN/SOURCE/REACHAB/graph_se.c \
						   WN/SOURCE/REACHAB/canonic.c \
						   WN/SOURCE/REACHAB/schemes.c \
						   WN/SOURCE/REACHAB/dsrg_graph.c \
						   WN/SOURCE/REACHAB/stack.c \
						   WN/SOURCE/REACHAB/convert.c \
						   WN/SOURCE/REACHAB/rg_files.c \
						   WN/SOURCE/REACHAB/rgengwn.c \
						   WN/SOURCE/READNET/read_arc.c \
						   WN/SOURCE/READNET/read_t_c.c \
						   WN/SOURCE/READNET/read_NET.c \
						   WN/SOURCE/READNET/read_DEF.c \
						   WN/SOURCE/READNET/read_t_s.c \
						   WN/SOURCE/READNET/wn_yac.c \
						   WN/SOURCE/REACHAB/E_SRG/GET_SYM.c \
						   WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_GROUPING.c \
						   WN/SOURCE/REACHAB/E_SRG/MAPP_FOR_DECOMPOSING.c \
						   WN/SOURCE/REACHAB/E_SRG/GROUP_STATICS.c \
						   WN/SOURCE/REACHAB/E_SRG/STATIC_LIST.c \
						   WN/SOURCE/REACHAB/E_SRG/MY_ALLOCATION.c \
						   WN/SOURCE/REACHAB/E_SRG/GET_FIRST_EVENT.c \
						   WN/SOURCE/REACHAB/E_SRG/INST_SYM_EVENT.c \
						   WN/SOURCE/REACHAB/E_SRG/EVENTUALITIES.c \
						   WN/SOURCE/REACHAB/E_SRG/READ_WRITE.c \
						   WN/SOURCE/REACHAB/E_SRG/SAVE.c \
						   WN/TRANSL/wn_grammar.y \
						   WN/TRANSL/wn.l
						   # WN/SOURCE/REACHAB/E_SRG/AFTER_FIRING_CASES.c

# special flags for the lex/yacc files of this library
libgspnMCDSRG.a_LEXFLAGS_WN/SOURCE/SPOT/PROPS/TRANS.l := $(LEXFLAGS) -PTrans
libgspnMCDSRG.a_LEXFLAGS_WN/SOURCE/SPOT/PROPS/PROPL.l := $(LEXFLAGS) -PProp
libgspnMCDSRG.a_YACCFLAGS_WN/SOURCE/SPOT/PROPS/TRANS.y := $(YACCFLAGS) -p Trans
libgspnMCDSRG.a_YACCFLAGS_WN/SOURCE/SPOT/PROPS/PROPG.y := $(YACCFLAGS) -p Prop

ifdef HAS_GLIB_LIB
LIBRARIES += libgspnMCDSRG.a
endif





ESRG_CTMC_SOURCES := WN/SOURCE/RESRG/esrg1.cpp WN/SOURCE/RESRG/interface.cpp
ESRG_CTMC_DEPENDS := $(LIBDIR)/libgspnMCESRG.a
ESRG_CTMC_LD := $(LDPP)
ESRG_CTMC_CPPFLAGS := $(CPPFLAGS) -I../../../INCLUDE -Wall \
					  $(call generate_WN_FLAGS,TOOL_ESRG_CTMC,ESRG_CTMC)
ESRG_CTMC_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCESRG -lm $(FLEX-LIB)

TARGETS += ESRG_CTMC




MDP_includes := $(CPPFLAGS) `pkg-config --static --cflags glib-2.0 libxml++-2.6 glibmm-2.4` \
				$(call generate_WN_FLAGS,TOOL_MDP,MDP) \
				-I/usr/local/include/graphMDP -I/usr/include/glpk/ $(FLEX-INCLUDE) \
				$(X11-INCLUDE)

MDP_SOURCES := WN/SOURCE/MDWN/mdp_main.cc WN/SOURCE/MDWN/general.cpp 
MDP_CPPFLAGS := $(MDP_includes)
MDP_LD := $(LDPP)
MDP_LDFLAGS := $(LDFLAGS) $(X11-LIB) $(LINK_GRAPHMDP_LIB) $(OPENGL-LIB) $(LINK_GLPK_LIB) \
	`pkg-config --static --libs glib-2.0 libxml++-2.6 glibmm-2.4`

ifdef HAS_GRAPHMDP_LIB
  ifdef HAS_LIBXMLPP2-6_LIB
    ifdef HAS_GLIBMM2-4_LIB
      ifdef HAS_GLPK_LIB
        TARGETS += MDP
      endif
    endif
  endif
endif

RG2RRG_SOURCES := WN/SOURCE/MDWN/rg2rrg.cpp \
				  WN/SOURCE/MDWN/function.cpp \
				  WN/SOURCE/MDWN/general.cpp
RG2RRG_CPPFLAGS := $(MDP_includes) 
RG2RRG_LD := $(LDPP)
RG2RRG_DEPENDS := $(LIBDIR)/libgspnMCESRG.a
RG2RRG_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCESRG -lm $(LINK_GLPK_LIB) $(FLEX-LIB)


MDWN2WN_SOURCES := WN/SOURCE/MDWN/mdwn2wn.cpp \
				   WN/SOURCE/MDWN/functionmdwn.cpp \
				   WN/SOURCE/MDWN/general.cpp
MDWN2WN_CPPFLAGS := $(MDP_includes)
MDWN2WN_LD := $(LDPP)
MDWN2WN_DEPENDS := $(LIBDIR)/libgspnMCESRG.a
MDWN2WN_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCESRG -lm $(LINK_GLPK_LIB) $(FLEX-LIB)

GDTMC_SOURCES := WN/SOURCE/MDWN/GenDTMC.cpp \
				 WN/SOURCE/MDWN/functionGenDTMC.cpp \
				 WN/SOURCE/MDWN/general.cpp
GDTMC_CPPFLAGS := $(MDP_includes)
GDTMC_LD := $(LDPP)
GDTMC_DEPENDS := $(LIBDIR)/libgspnMCESRG.a
GDTMC_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCESRG -lm $(LINK_GLPK_LIB) $(FLEX-LIB)

PARSER_SOURCES := WN/SOURCE/MDWN/parser.cpp \
				  WN/SOURCE/MDWN/general.cpp
PARSER_CPPFLAGS := $(MDP_includes)
PARSER_LD := $(LDPP)
PARSER_DEPENDS := $(LIBDIR)/libgspnMCESRG.a
PARSER_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCESRG -lm $(LINK_GLPK_LIB) $(FLEX-LIB)

mdwn2mdp_SOURCEFILE := WN/SOURCE/MDWN/MDWN2MDP.sh
mdwnsolve_SOURCEFILE := WN/SOURCE/MDWN/MDWNSolve.sh
mdwn2Prism_SOURCEFILE := WN/SOURCE/MDWN/MDWN2PRISM.sh

ifdef HAS_GLPK_LIB
  TARGETS += RG2RRG MDWN2WN GDTMC PARSER
  SCRIPTS += mdwn2mdp mdwnsolve mdwn2Prism
endif








WNDSRG_SOURCES := WN/SOURCE/DSRG/src/dsrg.c
WNDSRG_CFLAGS := $(CFLAGS) -IWN/INCLUDE -Wall \
			     $(call generate_WN_FLAGS,TOOL_WNDSRG,WNDSRG)		     
WNDSRG_DEPENDS := $(LIBDIR)/libgspnMCDSRG.a
WNDSRG_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCDSRG $(GLIB-LIB) -lm

WNRDSRG_SOURCES := WN/SOURCE/DSRG/src/refDsrg.c WN/SOURCE/DSRG/src/parse.c
WNRDSRG_CFLAGS := $(CFLAGS) -IWN/INCLUDE -Wall \
			      $(call generate_WN_FLAGS,TOOL_WNRDSRG,WNRDSRG)
WNRDSRG_DEPENDS := $(LIBDIR)/libgspnMCDSRG.a
WNRDSRG_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCDSRG $(GLIB-LIB) -lm

ifdef HAS_GLIB_LIB
TARGETS += WNDSRG WNRDSRG
endif



WNDSRGSOLVER_CFLAGS := -IWN/INCLUDE -IWN/SOURCE/SOLVEXSRG/src/common \
					   $(call generate_WN_FLAGS,TOOL_WNDSRGSOLVER,WNDSRGSOLVER)
WNDSRGSOLVER_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCDSRG $(GLIB-LIB) -lm $(FLEX-LIB)
WNDSRGSOLVER_DEPENDS := $(LIBDIR)/libgspnMCDSRG.a
WNDSRGSOLVER_SOURCES := WN/SOURCE/SOLVEXSRG/src/srcdsrg/SolverDsrg.c \
					    WN/SOURCE/SOLVEXSRG/src/srcdsrg/IndexDsrg.l \
					    WN/SOURCE/SOLVEXSRG/src/srcdsrg/IndexDsrg.y \
					    WN/SOURCE/SOLVEXSRG/src/common/gauss-seidel_algo.c
WNDSRGSOLVER_LEXFLAGS := -PIndex
WNDSRGSOLVER_YACCFLAGS := -p Index -b IndexDsrg			  

WN/SOURCE/SOLVEXSRG/src/srcdsrg/IndexDsrg.l: $(OBJDIR)/WNDSRGSOLVER/WN/SOURCE/SOLVEXSRG/src/srcdsrg/IndexDsrg.y.o 

ifdef HAS_GLIB_LIB
TARGETS += WNDSRGSOLVER
endif





WNESRGSOLVER_CFLAGS := $(CFLAGS) -IWN/INCLUDE -IWN/SOURCE/SOLVEXSRG/src/common \
					   $(call generate_WN_FLAGS,TOOL_WNESRGSOLVER,WNESRGSOLVER)
WNESRGSOLVER_LDFLAGS := $(LDFLAGS) -L$(LIBDIR) -lgspnMCESRG $(GLIB-LIB) -lm $(FLEX-LIB)
WNESRGSOLVER_DEPENDS := $(LIBDIR)/libgspnMCESRG.a
WNESRGSOLVER_SOURCES := WN/SOURCE/SOLVEXSRG/src/srcesrg/SolverEsrg.c \
					    WN/SOURCE/SOLVEXSRG/src/srcesrg/IndexEsrg.l \
					    WN/SOURCE/SOLVEXSRG/src/srcesrg/IndexEsrg.y \
					    WN/SOURCE/SOLVEXSRG/src/common/gauss-seidel_algo.c
WNESRGSOLVER_LEXFLAGS := -PIndex
WNESRGSOLVER_YACCFLAGS := -p Index -b IndexEsrg			  

ifdef HAS_GLIB_LIB
TARGETS += WNESRGSOLVER
endif

WN/SOURCE/SOLVEXSRG/src/srcesrg/IndexEsrg.l: $(OBJDIR)/WNESRGSOLVER/WN/SOURCE/SOLVEXSRG/src/srcesrg/IndexEsrg.y.o 



######################################
### GreatSPN GUI package
######################################

GREATSRC := greatsrc2.0.2

GreatSPN_SOURCES := $(GREATSRC)/Display.c           $(GREATSRC)/MenuCallBacks.c    $(GREATSRC)/aboutdialog.c \
					$(GREATSRC)/afire.c             $(GREATSRC)/alloc.c            $(GREATSRC)/animation.c \
					$(GREATSRC)/arc.c               $(GREATSRC)/arcdialog.c        $(GREATSRC)/box.c \
					$(GREATSRC)/canvas.c            $(GREATSRC)/cc.c               $(GREATSRC)/chop.c \
					$(GREATSRC)/circle.c            $(GREATSRC)/color.c            $(GREATSRC)/colordialog.c \
					$(GREATSRC)/command.c           $(GREATSRC)/commentdialog.c    $(GREATSRC)/compact.c \
					$(GREATSRC)/consoledialog.c     $(GREATSRC)/deadl.c            $(GREATSRC)/dismsg.c \
					$(GREATSRC)/drag.c              $(GREATSRC)/draw.c             $(GREATSRC)/ecs.c \
					$(GREATSRC)/errordialog.c       $(GREATSRC)/filer.c            $(GREATSRC)/fire.c \
					$(GREATSRC)/flpinv.c            $(GREATSRC)/fltinv.c           $(GREATSRC)/getmsg.c \
					$(GREATSRC)/global.c            $(GREATSRC)/grid.c             $(GREATSRC)/help.c \
					$(GREATSRC)/help2.c             $(GREATSRC)/highlig.c          $(GREATSRC)/histo.c \
					$(GREATSRC)/implp.c             $(GREATSRC)/indprest.c         $(GREATSRC)/infodialog.c \
					$(GREATSRC)/inputdialog.c       $(GREATSRC)/layer.c            $(GREATSRC)/layereditdialog.c \
					$(GREATSRC)/layerviewdialog.c   $(GREATSRC)/line.c             $(GREATSRC)/lisp.c \
					$(GREATSRC)/liveness.c          $(GREATSRC)/load.c             $(GREATSRC)/lpabso.c \
					$(GREATSRC)/lpbounds.c          $(GREATSRC)/main.c             $(GREATSRC)/mark.c \
					$(GREATSRC)/markdialog.c        $(GREATSRC)/mdgrammardialog.c  $(GREATSRC)/me.c \
					$(GREATSRC)/menu.c              $(GREATSRC)/menuinit.c         $(GREATSRC)/mirror.c \
					$(GREATSRC)/modify.c            $(GREATSRC)/mouse.c            $(GREATSRC)/move.c \
					$(GREATSRC)/near.c              $(GREATSRC)/net.c              $(GREATSRC)/optionsdialog.c \
					$(GREATSRC)/overview.c          $(GREATSRC)/panel.c            $(GREATSRC)/parse.c \
					$(GREATSRC)/pinv.c              $(GREATSRC)/place.c            $(GREATSRC)/placedialog.c \
					$(GREATSRC)/popup.c             $(GREATSRC)/postscript.c       $(GREATSRC)/printarea.c \
					$(GREATSRC)/printdialog.c       $(GREATSRC)/rate.c             $(GREATSRC)/ratedialog.c \
					$(GREATSRC)/res.c               $(GREATSRC)/rescale.c          $(GREATSRC)/restore.c \
					$(GREATSRC)/resultdialog.c      $(GREATSRC)/save.c             $(GREATSRC)/sc.c \
					$(GREATSRC)/search.c            $(GREATSRC)/search2.c          $(GREATSRC)/select.c \
					$(GREATSRC)/showdialog.c        $(GREATSRC)/showgdi.c          $(GREATSRC)/simpan.c \
					$(GREATSRC)/solvedialog.c       $(GREATSRC)/spline.c           $(GREATSRC)/swn.c \
					$(GREATSRC)/tag.c               $(GREATSRC)/tinv.c             $(GREATSRC)/tokens.c \
					$(GREATSRC)/toolkit.c           $(GREATSRC)/tramd.c            $(GREATSRC)/trans.c \
					$(GREATSRC)/transdialog.c       $(GREATSRC)/trap.c             $(GREATSRC)/unbound.c \
					$(GREATSRC)/unfold.c            $(GREATSRC)/warningdialog.c    $(GREATSRC)/zoom.c

GreatSPN_CFLAGS	:= -DLinux $(X11-INCLUDE) $(MOTIF-INCLUDE)
GreatSPN_LDFLAGS := $(LDFLAGS) $(X11-LIB) $(MOTIF-LIB) -lMrm -lXm -lXp -lXext -lXt -lX11 -lm

UIL_FILES:= UIL/aboutdialog.uil       UIL/appconfig.uil           UIL/arcdialog.uil\
			UIL/bitmaps.uil           UIL/colorchangedialog.uil   UIL/colors.uil\
			UIL/commentdialog.uil     UIL/consoledialog.uil       UIL/errordialog.uil\
			UIL/filedialog.uil        UIL/fonts.uil               UIL/histodialog.uil\
			UIL/infodialog.uil        UIL/inputdialog.uil         UIL/layereditdialog.uil\
			UIL/layerviewdialog.uil   UIL/markchangedialog.uil    UIL/mdgrammardialog.uil\
			UIL/menu.uil              UIL/menuAttributes.uil      UIL/mousehelp.uil\
			UIL/optionsdialog.uil     UIL/overdialog.uil          UIL/placechangedialog.uil\
			UIL/printdialog.uil       UIL/ratechangedialog.uil    UIL/resultdialog.uil\
			UIL/showdialog.uil        UIL/simdialog.uil           UIL/simoptions.uil\
			UIL/solvedialog.uil       UIL/swnrgoptions.uil        UIL/transdialog.uil\
			UIL/warningdialog.uil             

# $(BINDIR)/GreatSPN: $(BINDIR)/GreatSPN.uid $(SCRIPTDIR)/great_package.sh

GreatSPN_DEPENDS := $(BINDIR)/GreatSPN.uid

$(BINDIR)/GreatSPN.uid: UIL/Great.uil $(UIL_FILES)
	@echo "  [UID] " $@
	@LANG=C $(UIL) -o $@ UIL/Great.uil

ifdef HAS_OPENMOTIF_LIB
EXTRA_INSTALLED_BINARIES += $(BINDIR)/GreatSPN.uid
EXTRA_INSTALLED_SCRIPTS += gsrc2/GreatConfig
endif

######################################
### Private section
######################################

ifdef HAVE_PRIVATE_SECTION
  FROM_MAIN_MAKEFILE:=1
  include PRIV/Makefile
endif

######################################
### New Java GUI
######################################

GUI_ZIP_DIR := .
GUI_NAMEVER := GreatSPN-Editor-v1.5
JAVA_BUILD_DIR += JavaGUI/Editor/build
# JAVA_BUILD_DIR += JavaGUI/MathProvider/build 

# # Builds the MathProvider.jar java application
# JavaGUI/MathProvider/dist/MathProvider.jar: $(wildcard JavaGUI/MathProvider/src/*/*.java \
# 	                                                   JavaGUI/MathProvider/src/*/*/*.java \
# 	                                                   JavaGUI/MathProvider/src/*/*/*/*.java \
# 	                                                   JavaGUI/MathProvider/src/*/*/*/*/*.java \
# 	                                                   JavaGUI/MathProvider/src/*/*/*/*/*/*.java )
# 	@echo "  [ANT] " $@                             
# 	@ant -quiet -buildfile  JavaGUI/MathProvider/build.xml  jar

JAVA_GUI_GRAMMAR := JavaGUI/Editor/src/editor/domain/grammar/ExprLang.g4
JAVA_GUI_DEPS := JavaGUI/Editor/src/editor/domain/grammar/ExprLang.tokens

# Generate the ANTLR parser of the Editor.jar application
$(JAVA_GUI_DEPS): $(JAVA_GUI_GRAMMAR)
	@echo "  [ANTLR]" $@
	@java -jar JavaGUI/antlr-4.2.1-complete.jar -visitor -no-listener \
	      -package editor.domain.grammar $<

JavaGUI-antlr: $(JAVA_GUI_DEPS)

# Builds the Editor.jar java application (the new GreatSPN editor)
JavaGUI/Editor/dist/GreatSPN\ Editor.app \
JavaGUI/Editor/dist/Editor.jar: $(wildcard JavaGUI/Editor/src/*/*.java \
	                                       JavaGUI/Editor/src/*/*/*.java \
	                                       JavaGUI/Editor/src/*/*/*/*.java \
	                                       JavaGUI/Editor/src/*/*/*/*/*.java \
	                                       JavaGUI/Editor/src/*/*/*/*/*/*.java ) \
                                 $(JAVA_GUI_DEPS)
	@echo "  [ANT] " $@                             
	@ant -quiet -buildfile  JavaGUI/Editor/build.xml  jar bundle-app


# java-jars: JavaGUI/MathProvider/dist/MathProvider.jar
java-jars: JavaGUI/Editor/dist/Editor.jar


$(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Mac.zip: JavaGUI/Editor/dist/GreatSPN\ Editor.app
	@echo "  [MAKE] Java GUI (MacOSX App Bundle)"
	@(cd JavaGUI/Editor/dist/ && zip -q -r - GreatSPN\ Editor.app ) > $@

# Application package objects
$(OBJDIR)/JavaGUI/bin/Editor.jar: JavaGUI/Editor/dist/Editor.jar
	@echo "  [JAR] " $@
	@$(MKDIR) $(dir $@)/lib
	@rm -f $@
	@# Regenerate the jar file by hand, to avoid buggy ant generation
	@$(MKDIR) JavaGUI/Editor/build
	@(cd JavaGUI/Editor/build && zip -q ../../../$@ built-jar.properties)
	@(cd JavaGUI/Editor/build/classes && zip -r -q ../../../../$@ *)
	@$(MKDIR) $(OBJDIR)/JavaGUI/bin/META-INF
	@echo 'Manifest-Version: 1.0' > $(OBJDIR)/JavaGUI/bin/META-INF/MANIFEST.MF
	@echo 'Main-Class: editor.Main' >> $(OBJDIR)/JavaGUI/bin/META-INF/MANIFEST.MF
	@echo "Class-Path: $(subst JavaGUI/Editor/,,$(wildcard JavaGUI/Editor/lib/*.jar))" >> $(OBJDIR)/JavaGUI/bin/META-INF/MANIFEST.MF
	@(cd $(OBJDIR)/JavaGUI/bin && zip -q Editor.jar META-INF/MANIFEST.MF)
	@$(RM) $(OBJDIR)/JavaGUI/bin/META-INF/MANIFEST.MF
	@$(RMDIR) $(OBJDIR)/JavaGUI/bin/META-INF
	@# Copy extra files
	@cp JavaGUI/Editor/dist/lib/*.jar  $(OBJDIR)/JavaGUI/bin/lib/
	@cp JavaGUI/Additional/splash.png  $(OBJDIR)/JavaGUI/bin/lib/
	@cp JavaGUI/Additional/greatspn48.png  $(OBJDIR)/JavaGUI/bin/pnpro-editor.png
	@cp JavaGUI/Additional/pnpro-doc48.png  $(OBJDIR)/JavaGUI/bin/application-x-pnpro-editor.png

# $(OBJDIR)/JavaGUI/bin/MathProvider.jar: JavaGUI/MathProvider/dist/MathProvider.jar
# 	@echo "  [CP]  " $<
# 	@$(MKDIR) $(dir $@)
# 	@cp $<  $@

$(OBJDIR)/JavaGUI/bin/lib/splash.png: JavaGUI/Additional/splash.png
	@echo "  [CP]  " $<
	@$(MKDIR) $(dir $@)
	@cp $<  $@

JAVA_GUI_OBJECTS := $(OBJDIR)/JavaGUI/bin/Editor.jar \
					# $(OBJDIR)/JavaGUI/bin/MathProvider.jar \
					$(OBJDIR)/JavaGUI/bin/lib/splash.png

# Windows exe
$(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Win.zip: $(JAVA_GUI_OBJECTS) \
	                                   JavaGUI/DISTRIB/GreatSPN\ Editor.exe
	@echo "  [MAKE] Java GUI (Windows)"
	@(cd $(OBJDIR)/JavaGUI && zip -q -r - *.exe bin/* ) > $@

JavaGUI/DISTRIB/GreatSPN\ Editor.exe: JavaGUI/DISTRIB/launch4j-config.cfg.xml \
                                      $(OBJDIR)/JavaGUI/bin/Editor.jar
	@echo "  [LAUNCH4J] Java GUI (Windows EXE)"
	@$(LAUNCH4J) JavaGUI/DISTRIB/launch4j-config.cfg.xml
	@cp JavaGUI/DISTRIB/GreatSPN\ Editor.exe $(OBJDIR)/JavaGUI/

# Linux installer
$(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Linux.tgz: $(JAVA_GUI_OBJECTS) \
	                                     JavaGUI/DISTRIB/install.sh
	@echo "  [MAKE] Java GUI (Linux Tar)"
	@cp JavaGUI/DISTRIB/install.sh $(OBJDIR)/JavaGUI/
	@(cd $(OBJDIR)/JavaGUI && tar cz *.sh bin/* ) > $@

# Generic Jar installer
$(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Jar.zip: $(JAVA_GUI_OBJECTS) 
	@echo "  [MAKE] Java GUI (Generic Jars)"
	@(cd $(OBJDIR)/JavaGUI && zip -q -r - bin/* ) > $@

JavaGUI-win: $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Win.zip

JavaGUI-macosx: $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Mac.zip

JavaGUI-linux: $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Linux.tgz

JavaGUI-jar: $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Jar.zip

JAVA_GUI_ARCHIVES := $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Mac.zip \
				     $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Linux.tgz \
				     $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Jar.zip

# Windows exe generation requires the launch4j tool to be installed.
ifdef HAVE_LAUNCH4J
  JAVA_GUI_ARCHIVES += $(GUI_ZIP_DIR)/$(GUI_NAMEVER)-Win.zip
endif

JavaGUI: java-jars $(JAVA_GUI_ARCHIVES)

SCRIPTS += unfolding2 greatspn_editor

unfolding2_SOURCEFILE := JavaGUI/unfolding2.sh
greatspn_editor_SOURCEFILE := JavaGUI/greatspn_editor.sh


upload_JavaGUI: JavaGUI
	scp $(JAVA_GUI_ARCHIVES) amparore@pianeta.di.unito.it:/docsrv/amparore/public_html/mc4cslta/EditorBinaries/

clean_JavaGUI_x:
	@echo "  [CLEAN]  Editor.jar"
	@rm -rf  JavaGUI/Editor/build
	@rm -f 	 JavaGUI/Editor/dist/Editor.jar
	#@ant -quiet -silent -buildfile  JavaGUI/Editor/build.xml  clean
	# @echo "  [CLEAN]  MathProvider.jar"
	# @rm -rf  JavaGUI/MathProvider/build
	#@ant -quiet -silent -buildfile  JavaGUI/MathProvider/build.xml  clean
	@echo "  [CLEAN]  GUI Installers"                              
	@rm -rf $(OBJDIR)/JavaGUI/*
	@rm -f $(JAVA_GUI_ARCHIVES)

clean_JavaGUI: clean_JavaGUI_x


install_JavaGUI_jars: JavaGUI
	@echo "  [INSTALL] JavaGUI"
	@mkdir -p $(INSTALLDIR)/bin
	@cp -R $(OBJDIR)/JavaGUI/bin/* $(INSTALLDIR)/bin

# On Linux,also install the JavaGUI in the system menu using the XDG tools
linux-install-JavaGUI: install_JavaGUI_jars
	@echo "  [INSTALL] JavaGUI XDG resources"
	@(cd $(OBJDIR)/JavaGUI/ && export INSTALLDIR=$(INSTALLDIR) && bash install.sh -silent )

# Install GUI library of models
install_JavaGUI_models:
	@mkdir -p $(INSTALLDIR)/models
	@cp models/*.PNPRO $(INSTALLDIR)/models/


# On Linux, make install automatically re-installs the Java GUI
ifeq ($(UNAME_S),Linux)
 ifdef HAS_APACHE_ANT
install: linux-install-JavaGUI

 endif
endif

ifdef HAS_JAVA_DEVELOPMENT_KIT
 ifdef HAS_APACHE_ANT
all: JavaGUI

clean: clean_JavaGUI

install: install_JavaGUI_models install_JavaGUI_jars

 endif
endif

.PHONY += JavaGUI clean_JavaGUI JavaGUI-antlr java-jars clean_java-gui linux-install-JavaGUI
.PHONY += JavaGUI-win JavaGUI-macosx JavaGUI-linux JavaGUI-jar upload_JavaGUI install_JavaGUI_jars


######################################
### Upgrade procedures for the 
### preinstalled VirtualBox image.
######################################

# ifdef IS_VBOX_VERSION71

# install: run-vbox-install-script

# run-vbox-install-script:
# 	@echo "Running VirtualBox upgrade script..."
# 	@echo $(shell /home/user/GreatSPN/SOURCES/NSRC/VirtualMachineSupport/vbox_install_script.sh)

# .PHONY += run-vbox-install-script

# endif

######################################
### Lemon parser generator
######################################

lemon_SOURCES := NSRC/lemon/lemon.c

TARGETS += lemon

######################################
# CSLTA Solver & DSPN tool
######################################

CSLTA_SOURCES := NSRC/CSLTA-solver/asmc.cpp \
                 NSRC/CSLTA-solver/commands.cpp \
                 NSRC/CSLTA-solver/cslta.cpp \
                 NSRC/CSLTA-solver/dspn.cpp \
                 NSRC/CSLTA-solver/dta.cpp \
                 NSRC/CSLTA-solver/parser.y \
                 NSRC/CSLTA-solver/lexer.l

CSLTA_CPPFLAGS := $(CPPFLAGS) -Wall $(ENABLE_Cxx17)
CSLTA_LEX := flex
CSLTA_YACC := bison -d
CSLTA_LD := $(LDPP)
CSLTA_LDFLAGS :=  $(LDFLAGS) $(LINK_BOOST_CXX_LIB)/lib/libboost_timer.a \
                  $(LINK_BOOST_CXX_LIB)/lib/libboost_system.a \
                  $(LINK_BOOST_CXX_LIB)/lib/libboost_chrono.a

ifdef HAS_BOOST_CXX_LIB
  # TARGETS += CSLTA
endif

DSPN-Tool_SOURCES := NSRC/DSPN-Tool/newparser.lyy \
			         NSRC/DSPN-Tool/lexer.ll \
			         NSRC/DSPN-Tool/CSLTA.cpp \
			         NSRC/DSPN-Tool/DTA.cpp \
			         NSRC/DSPN-Tool/Experiment.cpp \
			         NSRC/DSPN-Tool/Language.cpp \
			         NSRC/DSPN-Tool/LinearProg.cpp \
			         NSRC/DSPN-Tool/MRP.cpp \
			         NSRC/DSPN-Tool/MRP_Explicit.cpp \
			         NSRC/DSPN-Tool/MRP_Implicit.cpp \
			         NSRC/DSPN-Tool/MRP_SccDecomp.cpp \
			         NSRC/DSPN-Tool/Measure.cpp \
			         NSRC/DSPN-Tool/PackedMemoryPool.cpp \
			         NSRC/DSPN-Tool/ParserDefs.cpp \
			         NSRC/DSPN-Tool/PetriExport.cpp \
			         NSRC/DSPN-Tool/PetriNet.cpp \
			         NSRC/DSPN-Tool/PetriNetTool.cpp \
			         NSRC/DSPN-Tool/ReachabilityGraph.cpp \
			         NSRC/DSPN-Tool/NewReachabilityGraph.cpp \
			         NSRC/DSPN-Tool/Semiflows.cpp \
			         NSRC/DSPN-Tool/SolveCTMC.cpp \
			         NSRC/DSPN-Tool/SynchProduct.cpp \
			         NSRC/DSPN-Tool/SynchProductSCC.cpp \
			         NSRC/DSPN-Tool/utils.cpp \
			         NSRC/numeric/numeric.cpp

$(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/newparser.lyy.o: NSRC/DSPN-Tool/newparser.lyy bin/lemon

$(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/PetriNet.o: $(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/newparser.lyy.o

$(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/DTA.o: $(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/newparser.lyy.o

$(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/CSLTA.o: $(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/newparser.lyy.o


NSRC/DSPN-Tool/lexer.ll: $(OBJDIR)/DSPN-Tool/NSRC/DSPN-Tool/newparser.lyy.o bin/lemon

DSPN-Tool_CPPFLAGS := -O2 -Wall $(ENABLE_Cxx17) \
                      -Iobjects/DSPN-Tool/NSRC/DSPN-Tool/ \
                      -Wno-unused-function \
                      -DNDEBUG=1 
DSPN-Tool_LD := $(LDPP)
DSPN-Tool_LEXPP := $(LEX)


$(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/newparser.lyy.o: NSRC/DSPN-Tool/newparser.lyy bin/lemon

$(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/PetriNet.o: $(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/newparser.lyy.o

$(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/DTA.o: $(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/newparser.lyy.o

$(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/CSLTA.o: $(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/newparser.lyy.o

NSRC/DSPN-Tool/lexer.ll: $(OBJDIR)/DSPN-Tool-Debug/NSRC/DSPN-Tool/newparser.lyy.o bin/lemon

DSPN-Tool-Debug_SOURCES := $(DSPN-Tool_SOURCES)
DSPN-Tool-Debug_CPPFLAGS := -Wall $(ENABLE_Cxx17) \
                      		-Iobjects/DSPN-Tool-Debug/NSRC/DSPN-Tool/ \
                      		-g -Wall -Wextra -Wno-unused-parameter \
                      		-Wno-unused-function \
                       		-DUSE_PRIVATE_TYPES=1 -D_GLIBCXX_DEBUG=1
DSPN-Tool-Debug_LD := $(DSPN-Tool_LD) -g
DSPN-Tool-Debug_LEXPP := $(LEX)

ifdef HAS_LP_SOLVE_LIB
  DSPN-Tool_CPPFLAGS := $(DSPN-Tool_CPPFLAGS) $(INCLUDE_LP_SOLVE_LIB)
  DSPN-Tool_LDFLAGS := $(LDFLAGS) $(DSPN-Tool_LDFLAGS) $(LINK_LP_SOLVE_LIB)
  DSPN-Tool-Debug_CPPFLAGS := $(DSPN-Tool-Debug_CPPFLAGS) $(INCLUDE_LP_SOLVE_LIB)
  DSPN-Tool-Debug_LDFLAGS := $(LDFLAGS) $(DSPN-Tool-Debug_LDFLAGS) $(LINK_LP_SOLVE_LIB)
endif

alphaFactory_SOURCES := NSRC/alphaFactory/alphaFactory.cpp

alphaFactory_CPPFLAGS := $(CPPFLAGS) -Wall $(ENABLE_Cxx17) \
                         -Wno-unused-function 
alphaFactory_LD := $(LDPP)


# ifeq ($(INCLUDE_ELVIO_CPP_SOLVER),1)
TARGETS += DSPN-Tool alphaFactory
ifndef HAS_VBOX_MARK
TARGETS += DSPN-Tool-Debug
endif
# endif


######################################
##### MAIN PACKAGE LOCATOR SCRIPT #####
######################################
install: install-great_package-script

GREAT_PKG_SCRIPT := $(INSTALLDIR)/$(SCRIPTDIR)/great_package.sh
install-great_package-script:
	@echo "  [GEN] " $(GREAT_PKG_SCRIPT)
	@$(MKDIR) $(dir $(GREAT_PKG_SCRIPT))
	@#echo GSPN2PACKAGE=$(INSTALLDIR) > $@
	@cat contrib/great_package.sh | sed 's/@@@@@@/$(subst /,\/,$(PKGDIR))/g' >> $(GREAT_PKG_SCRIPT)
	@chmod a+x $(GREAT_PKG_SCRIPT)

# $(SCRIPTDIR)/great_package.sh: contrib/great_package.sh
# 	@echo "  [GEN] " $@
# 	@$(MKDIR) $(dir $@)
# 	@echo GSPN2PACKAGE=$(INSTALLDIR) > $@
# 	@cat contrib/great_package.sh | sed 's/@@@@@@/$(subst /,\/,$(INSTALLDIR))/g' >> $@
# 	@chmod a+x $@

first_SOURCES := contrib/first.c
first_CFLAGS := $(GreatSPN_CFLAGS)
first_LDFLAGS := $(GreatSPN_LDFLAGS)

ifdef HAS_OPENMOTIF_LIB
  ifndef IS_WSL
TARGETS += GreatSPN first
  endif
endif

PrintCommand_SOURCEFILE := $(GREATSRC)/PrintCommand.csh
RemoveResCommand_SOURCEFILE := $(GREATSRC)/RemoveResCommand
RemoveAllCommand_SOURCEFILE := $(GREATSRC)/RemoveAllCommand
greatspn_SOURCEFILE := contrib/greatspn.sh
greatspn_version_SOURCEFILE := contrib/version.sh

ifdef HAS_OPENMOTIF_LIB
SCRIPTS += PrintCommand RemoveResCommand RemoveAllCommand greatspn greatspn_version
endif






######################################
### multisolve package
######################################

$(BINDIR)/multisolve/MultiSolve.class: multisolve/MultiSolve.java multisolve/NetFilter.java multisolve/Utils.java
	@echo "  [JAVA]" $@
	@$(MKDIR) $(dir $@)
	@javac -sourcepath multisolve/ -d $(BINDIR)/multisolve/ -cp $(BINDIR)/multisolve/ $^

$(SCRIPTDIR)/multisolve: $(BINDIR)/multisolve/MultiSolve.class

install-multisolve:
	@$(MKDIR) $(INSTALLDIR)/$(BINDIR)/multisolve/
	@cp $(BINDIR)/multisolve/MultiSolve.class  $(INSTALLDIR)/$(BINDIR)/multisolve/
	@cp $(BINDIR)/multisolve/NetFilter.class  $(INSTALLDIR)/$(BINDIR)/multisolve/
	@cp $(BINDIR)/multisolve/Utils.class  $(INSTALLDIR)/$(BINDIR)/multisolve/

ifdef HAS_JAVA_DEVELOPMENT_KIT
install: install-multisolve
endif

multisolve_SOURCEFILE := contrib/multisolve/multisolve.sh
gspn_st_ex_SOURCEFILE := multisolve/gspn_st_ex
gspn_st_ex.oc_SOURCEFILE := multisolve/gspn_st_ex.oc
gspn_tr_ex_SOURCEFILE := multisolve/gspn_tr_ex
gspn_tr_ex.oc_SOURCEFILE := multisolve/gspn_tr_ex.oc
swn_st_ex_ord_SOURCEFILE := multisolve/swn_st_ex_ord
swn_st_ex_ord.oc_SOURCEFILE := multisolve/swn_st_ex_ord.oc
swn_st_ex_sym_SOURCEFILE := multisolve/swn_st_ex_sym
swn_st_ex_sym.oc_SOURCEFILE := multisolve/swn_st_ex_sym.oc
swn_st_sim_ord_SOURCEFILE := multisolve/swn_st_sim_ord
swn_st_sim_sym_SOURCEFILE := multisolve/swn_st_sim_sym
swn_tr_ex_ord_SOURCEFILE := multisolve/swn_tr_ex_ord
swn_tr_ex_ord.oc_SOURCEFILE := multisolve/swn_tr_ex_ord.oc
swn_tr_ex_sym_SOURCEFILE := multisolve/swn_tr_ex_sym
swn_tr_ex_sym.oc_SOURCEFILE := multisolve/swn_tr_ex_sym.oc
GnuPlot_SOURCEFILE := multisolve/GnuPlot
commands.el_SOURCEFILE := multisolve/commands.el

ifdef HAS_JAVA_DEVELOPMENT_KIT
SCRIPTS += multisolve gspn_st_ex gspn_st_ex.oc gspn_tr_ex gspn_tr_ex.oc \
		   swn_st_ex_ord swn_st_ex_ord.oc swn_st_ex_sym swn_st_ex_sym.oc \
		   swn_st_sim_ord swn_st_sim_sym swn_tr_ex_ord swn_tr_ex_ord.oc \
		   swn_tr_ex_sym swn_tr_ex_sym.oc GnuPlot commands.el
endif





######################################
### simsrc2 package
######################################

engine_LDFLAGS := $(LDFLAGS) -lm
engine_SOURCES := simsrc2/engine_control.c simsrc2/engine_event.c \
				  simsrc2/engine_pn.c WN/SOLVE/compact.c
cntrl_SOURCES := simsrc2/cntrl.c
measure_LDFLAGS := $(LDFLAGS) -lm
measure_SOURCES := simsrc2/measure_checkpoint.c simsrc2/measure_pn.c \
				   WN/SOLVE/compact.c 

TARGETS += engine cntrl measure

engine_SOURCEFILE := simsrc2/engine.sh

SCRIPTS += engine







#####################################################################################################
###                                Internal Makefile procedures                                   ###
#####################################################################################################

### Helper make functions ###

# Generate object file names from each source file names
src2obj = $(foreach src, $(1), $(addprefix $(OBJDIR)/$(2)/, \
			$(patsubst %.c,   %.o,     $(filter %.c,   $(src))) \
			$(patsubst %.cc,  %.o,     $(filter %.cc,  $(src))) \
			$(patsubst %.cpp, %.o,     $(filter %.cpp, $(src))) \
			$(patsubst %.y,   %.y.o,   $(filter %.y,   $(src))) \
			$(patsubst %.yy,  %.yy.o,  $(filter %.yy,  $(src))) \
			$(patsubst %.l,   %.l.o,   $(filter %.l,   $(src))) \
			$(patsubst %.ll,  %.ll.o,  $(filter %.ll,  $(src))) \
			$(patsubst %.ly,  %.ly.o,  $(filter %.ly,  $(src))) \
			$(patsubst %.lyy, %.lyy.o, $(filter %.lyy, $(src))) \
		   ))

# Removes the target name from a path name: TARGET/path
null      :=
SPACE     := $(null) $(null)
rmprefix = $(call rmprefix2,$(subst /, ,$(1)))
rmprefix2 = $(subst $(SPACE),/,$(wordlist 2,$(words $(1)),$(1)))

# Extract the target from a pathname TARGET/path
get_target = $(word 1,$(subst /,$(SPACE),$(1)))

# $(getvar VAR,TARGET,FILE) 
# Returns the first variable that is defined among the following:
#    $(TARGET)_$(VAR)_$(FILE)   Target & file specific variable
#    $(TARGET)_$(VAR)           Target-specific variable
#    $(VAR)                     Global variable
# For instance, if the compiled file is src/code.c for target myApp and
# the requested var is CFLAGS, it first searches for a variable
# myApp_src/code.c_CFLAGS, then if not defined searches for
# myApp_CFLAGS, and if it is not defined uses the global var CFLAGS.
getvar = $(if $($(2)_$(1)_$(3)),$($(2)_$(1)_$(3)), \
		   $(if $($(2)_$(1)),$($(2)_$(1)),$($(1))))


### Lists of all sources, objects and compiled binaries ###

### Object files generated from .c, .cc and .cpp sources
COBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
			  $(filter %.c, $($(target)_SOURCES)),$(target)))
CCOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
			   $(filter %.cc, $($(target)_SOURCES)),$(target)))
CPPOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
				$(filter %.cpp, $($(target)_SOURCES)),$(target)))

### Source and object files generated from .l and .ll sources
LEXOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
				$(filter %.l, $($(target)_SOURCES)),$(target)))
LEXDERIVEDSOURCES := $(foreach obj, $(LEXOBJECTS), $(obj:.o=.c))
LEXDERIVEDHEADERS := $(foreach obj, $(LEXOBJECTS), $(obj:.o=.h))

LEXPPOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
				$(filter %.ll, $($(target)_SOURCES)),$(target)))
LEXPPDERIVEDSOURCES := $(foreach obj, $(LEXPPOBJECTS), $(obj:.o=.cpp))
LEXPPDERIVEDHEADERS := $(foreach obj, $(LEXPPOBJECTS), $(obj:.o=.h))

### Source and object files generated from .y and .yy sources
YACCOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
				$(filter %.y, $($(target)_SOURCES)),$(target)))
YACCDERIVEDSOURCES := $(foreach obj, $(YACCOBJECTS), $(obj:.o=.c))
YACCDERIVEDHEADERS := $(foreach obj, $(YACCOBJECTS), $(obj:.o=.h))

YACCPPOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
				$(filter %.yy, $($(target)_SOURCES)),$(target)))
YACCPPDERIVEDSOURCES := $(foreach obj, $(YACCPPOBJECTS), $(obj:.o=.cpp))
YACCPPDERIVEDHEADERS := $(foreach obj, $(YACCPPOBJECTS), $(obj:.o=.h))

### Source and object files generated from .ly/.lyy sources (lemon parser generator)
LEMONOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
				$(filter %.ly, $($(target)_SOURCES)),$(target)))
LEMONDERIVEDSOURCES := $(foreach obj, $(LEMONOBJECTS), $(obj:.o=.c))
LEMONDERIVEDHEADERS := $(foreach obj, $(LEMONOBJECTS), $(obj:.o=.h))

LEMONPPOBJECTS := $(foreach target, $(TARGETS) $(LIBRARIES), $(call src2obj, \
				$(filter %.lyy, $($(target)_SOURCES)),$(target)))
LEMONPPDERIVEDSOURCES := $(foreach obj, $(LEMONPPOBJECTS), $(obj:.o=.cpp))
LEMONPPDERIVEDHEADERS := $(foreach obj, $(LEMONPPOBJECTS), $(obj:.o=.h))

### All the object files generated by .c/.cpp sources produced by Yacc/Flex
DERIVED_COBJECTS := $(LEXOBJECTS) $(YACCOBJECTS) $(LEMONOBJECTS)
DERIVED_CPPOBJECTS := $(LEXPPOBJECTS) $(YACCPPOBJECTS) $(LEMONPPOBJECTS)

### All the .o files generated in the compile process
OBJECTS := $(COBJECTS) $(CCOBJECTS) $(CPPOBJECTS) \
		   $(LEXOBJECTS) $(LEXPPOBJECTS) \
		   $(YACCOBJECTS) $(YACCPPOBJECTS) \
		   $(LEMONOBJECTS) $(LEMONPPOBJECTS)

### The .h/.c/.cpp sources and headers generated by Yacc/Flex
DERIVEDSOURCES := $(LEXDERIVEDSOURCES) $(LEXPPDERIVEDSOURCES) \
				  $(YACCDERIVEDSOURCES) $(YACCPPDERIVEDSOURCES) \
				  $(LEMONDERIVEDSOURCES) $(LEMONPPDERIVEDSOURCES)
DERIVEDHEADERS := $(YACCDERIVEDHEADERS) $(YACCPPDERIVEDHEADERS) \
				  $(LEMONDERIVEDHEADERS) $(LEMONPPDERIVEDHEADERS)

### Target binaries, copyed scripts and generated static libraries ###
BINARIES := $(foreach target,$(TARGETS),$(addprefix $(BINDIR)/,$(target)))

SCRIPT_FILES := $(foreach script,$(SCRIPTS),$(addprefix $(SCRIPTDIR)/,$(script)))
SCRIPT_SOURCES := $(foreach script,$(SCRIPTS),$($(script)_SOURCEFILE))

AR_FILES := $(foreach lib,$(LIBRARIES),$(addprefix $(LIBDIR)/,$(lib)))



### Clean rules ###
CLEAN_TARGETS := $(TARGETS:%=clean_%)
CLEAN_LIBRARIES := $(LIBRARIES:%=clean_%)
CLEAN_INSTALLEDSOURCES := $(INSTALLEDSOURCES:%=clean_%)

$(CLEAN_TARGETS): clean_%:
	@echo "  [CLEAN] " $*
	@$(RM) $(BINDIR)/$*  $(call src2obj,$($*_SOURCES),$*)

$(CLEAN_LIBRARIES): clean_%:
	@echo "  [CLEAN] " $*
	@$(RM) $(LIBDIR)/$*  $(call src2obj,$($*_SOURCES),$*)
	
$(CLEAN_INSTALLEDSOURCES): clean_%:
	@echo "  [CLEAN] " $*
	@$(RM) $(INSTALLEDSOURCEDIR)/$*  $(call src2obj,$($*_SOURCES),$*)
	
all_clean: $(CLEAN_TARGETS) $(CLEAN_LIBRARIES) $(CLEAN_INSTALLEDSOURCES)

### Reindentation of all c/c++ source files ###
REINDENT_CMD := /usr/local/bin/astyle -n -A2 --pad-oper --unpad-paren --pad-header \
				 --align-pointer=name --align-reference=name --break-closing-brackets \
				 --keep-one-line-blocks --keep-one-line-statements \
				 --attach-extern-c --attach-inlines --attach-classes --attach-namespaces \

reindent_all:
	@$(eval REINDENT_FILES := $(foreach target,$(TARGETS),\
		$(filter %.c, $($(target)_SOURCES)) \
		$(filter %.cc, $($(target)_SOURCES)) \
		$(filter %.cpp, $($(target)_SOURCES)) \
		$(filter %.h, $($(target)_SOURCES))))
	@for X in $(REINDENT_FILES); do \
		echo "  [REINDENT]  " $$X ; \
		$(REINDENT_CMD) $$X ; \
	 done
	@echo "  [REINDENT]  HEADERS"
	@$(REINDENT_CMD) */*.h */*/*.h */*/*/*.h */*/*/*/*.h 

### Main rules of the makefile ###
binaries: $(BINARIES)
	@

scripts: $(SCRIPT_FILES)
	@

libraries: $(AR_FILES)
	@

clean: $(CLEAN_TARGETS) $(CLEAN_LIBRARIES) $(CLEAN_INSTALLEDSOURCE)
	@echo "  [CLEAN]"
	@$(RM) $(OBJECTS) $(DERIVEDSOURCES) $(DERIVEDHEADERS)
	@$(RM) $(BINARIES) $(AR_FILES) $(SCRIPT_FILES)

distclean:
	@echo "  [DISTCLEAN]"
	@$(RMDIR) $(BINDIR) $(SCRIPTDIR) $(LIBDIR) $(OBJDIR) $(JAVA_BUILD_DIR) $(CLEAN_INSTALLEDSOURCE)

install: all
	@echo "  [INSTALL]"
	@$(MKDIR) $(INSTALLDIR)/$(BINDIR)/
	@cp $(BINARIES) $(EXTRA_INSTALLED_BINARIES)  $(INSTALLDIR)/$(BINDIR)/
	@$(MKDIR) $(INSTALLDIR)/$(SCRIPTDIR)/
	@cp $(SCRIPT_FILES) $(EXTRA_INSTALLED_SCRIPTS)  $(INSTALLDIR)/$(SCRIPTDIR)/
	@$(MKDIR) $(INSTALLDIR)/$(LIBDIR)/
	@cp $(AR_FILES)  $(INSTALLDIR)/$(LIBDIR)/
	@$(MKDIR) $(INSTALLDIR)/$(INSTALLEDSOURCEDIR)/
	@cp $(INSTALLEDSOURCES)  $(INSTALLDIR)/$(INSTALLEDSOURCEDIR)/	
	@echo "Remember to add "$(INSTALLDIR)/$(SCRIPTDIR)" to the PATH environment variable."

uninstall:
	@echo "  [UNINSTALL]"
	@$(RMDIR) $(INSTALLDIR)/$(BINDIR)
	@$(RMDIR) $(INSTALLDIR)/$(SCRIPTDIR)
	@$(RMDIR) $(INSTALLDIR)/$(LIBDIR)
	@$(RMDIR) $(INSTALLDIR)/$(INSTALLEDSOURCEDIR)
	@$(RMDIR) $(INSTALLDIR)

print:
	@echo "COBJECTS = " $(COBJECTS)
	@echo "CCOBJECTS = " $(CCOBJECTS)
	@echo "CPPOBJECTS = " $(CPPOBJECTS)
	@echo "BINARIES = " $(BINARIES)
	@echo "LEXOBJECTS = " $(LEXOBJECTS)
	@echo "LEXDERIVEDSOURCES = " $(LEXDERIVEDSOURCES)
	@echo "LEXDERIVEDHEADERS = " $(LEXDERIVEDHEADERS)
	@echo "YACCOBJECTS = " $(YACCOBJECTS)
	@echo "YACCDERIVEDSOURCES = " $(YACCDERIVEDSOURCES)
	@echo "YACCDERIVEDHEADERS = " $(YACCDERIVEDHEADERS)
	@echo "LEX++OBJECTS = " $(LEXPPOBJECTS)
	@echo "LEX++DERIVEDSOURCES = " $(LEXPPDERIVEDSOURCES)
	@echo "LEX++DERIVEDHEADERS = " $(LEXPPDERIVEDHEADERS)
	@echo "YACC++OBJECTS = " $(YACCPPOBJECTS)
	@echo "YACC++DERIVEDSOURCES = " $(YACCPPDERIVEDSOURCES)
	@echo "YACC++DERIVEDHEADERS = " $(YACCPPDERIVEDHEADERS)
	@echo "SCRIPT_FILES = " $(SCRIPT_FILES)
	@echo "SCRIPT_SOURCES = " $(SCRIPT_SOURCES)

print_binaries:
	@echo "BINARIES = " $(BINARIES)

print2:
	@echo "DERIVEDSOURCES = " $(DERIVEDSOURCES)
	@echo "DERIVEDHEADERS = " $(DERIVEDHEADERS)

derived_objects: $(DERIVEDSOURCES) $(DERIVEDHEADERS)
	@echo "  [DERIVED OBJECTS] " $*

.PHONY: all binaries scripts libraries clean distclean install uninstall print reindent_all derived_objects

$(TARGETS): % : $(BINDIR)/%
	@

$(SCRIPT_SOURCES): % :
	@

$(LIBRARIES): % : $(LIBDIR)/%
	@

### Automatic dependency of source files ###
DEPENDS := $(patsubst %.o, %.d, $(OBJECTS))
-include $(DEPENDS)

### General .c compilation rule ###
.SECONDEXPANSION:
$(COBJECTS): $(OBJDIR)/%.o: $$(call rmprefix,%.c)
	@# Determine if the variables should be file-specific, project specific or global
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_CC := $(call getvar,CC,$($@_TARGET),$^))
	@$(eval $@_CFLAGS := $(call getvar,CFLAGS,$($@_TARGET),$^))
	@$(eval $@_INCLUDES := $(call getvar,INCLUDES,$($@_TARGET),$^))
	@# Invoke the compiler
	@echo "  [CC]  " $<
	@$(MKDIR) $(dir $@)
	@$($@_CC) $($@_CFLAGS) $($@_INCLUDES) -MMD -MF $(@:%.o=%.d) -o $@  $< 

### Compilation of .c files generated by Yacc/Lex/Lemon ###
$(DERIVED_COBJECTS): $(OBJDIR)/%.o: $(OBJDIR)/%.c
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_CC := $(call getvar,CC,$($@_TARGET),$^))
	@$(eval $@_CFLAGS := $(call getvar,CFLAGS,$($@_TARGET),$^))
	@$(eval $@_INCLUDES := $(call getvar,INCLUDES,$($@_TARGET),$^))
	@echo "  [CC]  " $<
	@$(MKDIR) $(dir $@)
	@$($@_CC) $($@_CFLAGS) $($@_INCLUDES) -I$(call rmprefix,$(dir $*)) \
		-MMD -MF $(@:%.o=%.d) -o $@ $< 

### General .cc compilation rule ###
.SECONDEXPANSION:
$(CCOBJECTS): $(OBJDIR)/%.o: $$(call rmprefix,%.cc)
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_CPP := $(call getvar,CPP,$($@_TARGET),$^))
	@$(eval $@_CPPFLAGS := $(call getvar,CPPFLAGS,$($@_TARGET),$^))
	@$(eval $@_INCLUDES := $(call getvar,INCLUDES,$($@_TARGET),$^))
	@echo "  [C++] " $<
	@$(MKDIR) $(dir $@)
	@$($@_CPP) $($@_CPPFLAGS) $($($@_INCLUDES)) -MMD -MF $(@:%.o=%.d) -o $@  $< 

### General .cpp compilation rule ###
.SECONDEXPANSION:
$(CPPOBJECTS): $(OBJDIR)/%.o: $$(call rmprefix,%.cpp)
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_CPP := $(call getvar,CPP,$($@_TARGET),$^))
	@$(eval $@_CPPFLAGS := $(call getvar,CPPFLAGS,$($@_TARGET),$^))
	@$(eval $@_INCLUDES := $(call getvar,INCLUDES,$($@_TARGET),$^))
	@echo "  [C++] " $<
	@$(MKDIR) $(dir $@)
	@$($@_CPP) $($@_CPPFLAGS) $($@_INCLUDES) -MMD -MF $(@:%.o=%.d) -o $@  $< 

### Compilation of C++ files generated by Yacc/Lex/Lemon in C++ mode ###
$(DERIVED_CPPOBJECTS): $(OBJDIR)/%.o: $(OBJDIR)/%.cpp
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_CPP := $(call getvar,CPP,$($@_TARGET),$^))
	@$(eval $@_CPPFLAGS := $(call getvar,CPPFLAGS,$($@_TARGET),$^))
	@$(eval $@_INCLUDES := $(call getvar,INCLUDES,$($@_TARGET),$^))
	@echo "  [C++] " $<
	@$(MKDIR) $(dir $@)
	@$($@_CPP) $($@_CPPFLAGS) $($@_INCLUDES) -I$(call rmprefix,$(dir $*)) \
		-MMD -MF $(@:%.o=%.d) -o $@  $< 

### Generation of a C file from a .l grammar ###
.SECONDEXPANSION:
$(LEXDERIVEDSOURCES): $(OBJDIR)/%.l.c: $$(call rmprefix,%.l)
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_LEX := $(call getvar,LEX,$($@_TARGET),$^))
	@$(eval $@_LEXFLAGS := $(call getvar,LEXFLAGS,$($@_TARGET),$^))
	@echo "  [LEX] " $<
	@$(MKDIR) $(dir $@)
	@$($@_LEX) $($@_LEXFLAGS) -o $@ $^

.SECONDEXPANSION:
$(LEXDERIVEDHEADERS): $(OBJDIR)/%.l.h: $$(call rmprefix,%.l)
	@

### Generation of the C/H files from a .y grammar ###
.SECONDEXPANSION:
$(YACCDERIVEDSOURCES): $(OBJDIR)/%.y.c: $$(call rmprefix,%.y)
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_YACC := $(call getvar,YACC,$($@_TARGET),$^))
	@$(eval $@_YACCFLAGS := $(call getvar,YACCFLAGS,$($@_TARGET),$^))
	@echo "  [YACC]" $<
	@$(MKDIR) $(dir $@)
	@$($@_YACC) $($@_YACCFLAGS) -o $@ $^

$(YACCDERIVEDHEADERS): %:
	@

### Generation of the C/H files from a .ly grammar ###
.SECONDEXPANSION:
$(LEMONDERIVEDSOURCES): $(OBJDIR)/%.ly.c: $$(call rmprefix,%.ly) bin/lemon
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_LEMONFLAGS := $(call getvar,LEMONFLAGS,$($@_TARGET),$^))
	@echo "  [LMN]  " $<
	@$(MKDIR) $(dir $@)
	@$(LEMON) $($@_LEMONFLAGS) $^
	@mv $(basename $<).h $(basename $@).h
	@mv $(basename $<).out $(basename $@).out
	@mv $(basename $<).c $(basename $@).c

$(LEMONDERIVEDHEADERS): %:
	@

### Generation of a C++ file from a Lex++ grammar ###
.SECONDEXPANSION:
$(LEXPPDERIVEDSOURCES): $(OBJDIR)/%.ll.cpp: $$(call rmprefix,%.ll)
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_LEXPP := $(call getvar,LEXPP,$($@_TARGET),$^))
	@$(eval $@_LEXPPFLAGS := $(call getvar,LEXPPFLAGS,$($@_TARGET),$^))
	@echo "  [LEX] " $<
	@$(MKDIR) $(dir $@)
	@$($@_LEXPP) $($@_LEXPPFLAGS) -o $@ $^

.SECONDEXPANSION:
$(LEXPPDERIVEDHEADERS): $(OBJDIR)/%.ll.h: $$(call rmprefix,%.ll)
	@

### Generation of the CPP/H files from a Yacc++ grammar ###
.SECONDEXPANSION:
$(YACCPPDERIVEDSOURCES): $(OBJDIR)/%.yy.cpp: $$(call rmprefix,%.yy)
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_YACCPP := $(call getvar,YACCPP,$($@_TARGET),$^))
	@$(eval $@_YACCPPFLAGS := $(call getvar,YACCPPFLAGS,$($@_TARGET),$^))
	@echo "  [YACC]" $<
	@$(MKDIR) $(dir $@)
	@$($@_YACCPP) $($@_YACCPPFLAGS) -i -b $(basename $@) $^
	@cat $(basename $@).tab.c | sed 's?$(basename $@).tab?$(basename $(notdir $@))?g' > $(basename $@).cpp
	@cat $(basename $@).tab.h > $(basename $@).h
	@cat $(basename $@).tab.i > $(basename $@).i
	@rm -f $(basename $@).tab.c $(basename $@).tab.h $(basename $@).tab.i

$(YACCPPDERIVEDHEADERS): %:
	@

### Generation of the CPP/H files from a Lemon++ grammar ###
.SECONDEXPANSION:
$(LEMONPPDERIVEDSOURCES): $(OBJDIR)/%.lyy.cpp: $$(call rmprefix,%.lyy)   $(BINDIR)/lemon
	@$(eval $@_TARGET := $(call get_target,$*))
	@$(eval $@_LEMONPPFLAGS := $(call getvar,LEMONPPFLAGS,$($@_TARGET),$^))
	@echo "  [LEMON]" $<
	@$(MKDIR) $(dir $@)
	@$(LEMON) $($@_LEMONPPFLAGS) $(subst $(BINDIR)/lemon,,$^)
	@mv $(basename $<).h $(basename $@).h
	@mv $(basename $<).out $(basename $@).out
	@cat $(basename $<).c | sed "s/$(subst /,\/,$(basename $<).c)/$(subst /,\/,$(basename $@).cpp)/g" > $(basename $@).cpp
	@rm $(basename $<).c

$(LEMONPPDERIVEDHEADERS): %:
	@

### General linking rule for binaries ###
.SECONDEXPANSION:
$(BINARIES): $(BINDIR)/% : $$(call src2obj, $$($$(@F)_SOURCES),$$*) $$($$*_DEPENDS)
	@$(eval $@_TARGET := $*)
	@$(eval $@_LD := $(call getvar,LD,$($@_TARGET),$^))
	@$(eval $@_LDFLAGS := $(call getvar,LDFLAGS,$($@_TARGET),$^))
	@echo "  [LD]  " $@
	@$(MKDIR) $(dir $@)
	@$($@_LD) -o $@ $(filter %.o,$^) $($@_LDFLAGS)

### General linking rules for static libraries (.a) ###
.SECONDEXPANSION:
$(AR_FILES): $(LIBDIR)/% : $$(call src2obj, $$($$(@F)_SOURCES),$$*) $$($$*_DEPENDS)
	@$(eval $@_TARGET := $*)
	@$(eval $@_AR := $(call getvar,AR,$($@_TARGET),$^))
	@$(eval $@_ARFLAGS := $(call getvar,ARFLAGS,$($@_TARGET),$^))
	@echo "  [AR]  " $@
	@$(MKDIR) $(dir $@)
	@$($@_AR) $($@_ARFLAGS) $@ $(filter %.o,$^)

### Derived Yacc/Lex sources are secondary chain targets ###
.SECONDARY: $(DERIVEDSOURCES)

### Scripts that are just copied in the script directory ###
.SECONDEXPANSION:
$(SCRIPT_FILES): $(SCRIPTDIR)/% : $$($$(@F)_SOURCEFILE)
	@echo "  [CP]  " $@
	@$(MKDIR) $(dir $@)
	@cp $($(@F)_SOURCEFILE) $@
	@chmod a+x $@






