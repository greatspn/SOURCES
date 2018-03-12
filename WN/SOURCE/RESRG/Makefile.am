bin_PROGRAMS =ESRG_CTMC
ESRG_CTMC_SOURCES = esrg1.cpp interface.cpp

# set the include path found by configure
INCLUDES= $(all_includes) -I../../../INCLUDE '-D"const.h"="const.h"' \
		  '-D"../INCLUDE/const.h"="../../INCLUDE/const.h"' \
		  '-D"../../INCLUDE/const.h"="../../INCLUDE/const.h"' \
		  '-D"../../../INCLUDE/const.h"="../../../INCLUDE/const.h"' 

# the library search path.
ESRG_CTMC_LDFLAGS = $(all_libraries)  -L../../../../../$(PATHL)/2bin/lib/
ESRG_CTMC_LDADD = -lgspnMCESRG -lm -lfl
ESRG_CTMC_CFLAGS= -Wall 
noinst_HEADERS = class.h macro.h

