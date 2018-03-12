EXTRA_DIST = README

# yacc flags
AM_YFLAGS = -d 
# C compiler flags
AM_CFLAGS =  -O3 -g
# to specify file built by yacc 
BUILT_SOURCES = lisp-obj_yacc.h 

# main makefile target is a program
noinst_PROGRAMS = swn-translator


# mention ALL source files here. No intermediate build files ! only those done by hand.
swn_translator_SOURCES = alloc.c        convers.h      global.h       lista-adt.c     mainTransl.c    save.c         var-lex.h \
	alloc.h        defines.h      layer.c        lista-adt.h     multiset-adt.c  save.h       unfold.c         var-yacc.h \
	assignment.c   evaluate.c     layer.h        load.c          multiset-adt.h     unfolding-obj.c  \
	assignment.h   evaluate.h       load.h                set-adt.c    unfolding-obj.h  \
	attrib-yacc.c  funct.h             object.h        set-adt.h    util-transl.c \
	attrib-yacc.h  generic-def.h  lisp-obj_yacc.y lisp-obj_lex.l     SWN-types.c  util-transl.h \
	const.h        global.c       option.h        SWN-types.h  var1.h 
