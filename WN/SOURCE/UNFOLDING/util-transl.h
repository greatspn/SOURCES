/*   util-transl.h   */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef UT
#define UT
#define DEBUG_UNFOLD 0
#define EOS '\0'


enum ErrorTypes { UNKN_OBJ_ERR, REDECL_OBJ_ERR, NO_DECL_OBJ_ERR, UNKN_PAR_ERR,
                  UNKN_PRI_ERR, UNKN_PLACE_ERR, UNKN_TRANS_ERR, UNKN_ARC_ERR,
                  UNKN_LISP_ERR, UNKN_VAR_ERR, BND_ERR, SUBCLASS_DEF_ERR, CLASS_DEF_ERR,
                  UNKN_CLASS_TYPE_ERR, UNDEF_DOM_ERR, INVALID_DOM_ERR, UNKN_MARK_ERR,
                  UNDEF_DOM_MARK_ERR, UNKN_SUBCL_ERR, UNKN_DYNSUBCL_ERR, UNKN_CLASS_ERR,
                  UNKN_TOKEN_ERR, MISS_OBJ_ERR, SUCCESSOR_FUNCTION_ERR, PREDECESSOR_FUNCTION_ERR, ERROR_REPETITIONS_AND_NO_FUNCTION, NUM_COMP_ERR, MISS_SUBCL_EL_ERR
                };

void Error(enum ErrorTypes, char *nome_funz, char *s);
int Hashpjw(char *);
void *Emalloc(size_t);
void *Ecalloc(size_t nelem, size_t elsize);
void *Erealloc(void *, size_t);
char *Estrdup(char *);
FILE *Efopen(char *filename, char *open_mode);
char *EmptyString(void);
char *NewStringCat(char *old_name, char *ext);
char *EraseFinalCR(char *string);

#endif

