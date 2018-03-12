bin_PROGRAMS = MDP RG2RRG MDWN2WN GDTMC PARSER
MDP_SOURCES = mdp_main.cc general.cpp 


# set the include path found by configure
INCLUDES= $(all_includes) -I/usr/include/libxml++-2.6 -I/usr/include/libxml++-2.6/libxml++/ -I/usr/lib/libxml++-2.6/include  -I/usr/lib64/libxml++-2.6/include -I/usr/include/libxml2 -I/usr/include/glibmm-2.4 -I/usr/lib64/glibmm-2.4/include -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/lib64/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -I/usr/lib64/glib-2.0/include -I/usr/include/glibmm-2.4 -I/usr/lib/glibmm-2.4/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/glpk/ -I /usr/local/include/graphMDP
# the library search path.
MDP_LDFLAGS = $(all_libraries) 

MDP_includedir = $(pkgincludedir)


MDP_LDADD =   -lgraphmdp -lglut -lglpk -lxml++-2.6 -lglibmm-2.4

AM_CXXFLAGS =\
	 -Wall
noinst_HEADERS = class.h mdp2dtmc.h mdwnclass.h

RG2RRG_SOURCES = rg2rrg.cpp function.cpp general.cpp
# set the include path found by configure
RG2RRG_LDFLAGS = $(all_libraries) -L../../../../../$(PATHL)/2bin/lib/

RG2RRG_LDADD = -lgspnMCESRG -lm -lfl -lglpk
MDWN2WN_SOURCES = mdwn2wn.cpp functionmdwn.cpp general.cpp

GDTMC_SOURCES = GenDTMC.cpp functionGenDTMC.cpp general.cpp
GDTMC_LDFLAGS = $(all_libraries) -L../../../../../$(PATHL)/2bin/lib/ 

GDTMC_LDADD = -lgspnMCESRG -lm -lfl -lglpk

PARSER_SOURCES= parser.cpp general.cpp
PARSER_LDFLAGS= $(all_libraries) 
