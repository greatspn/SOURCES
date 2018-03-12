#ifdef ___PARSE___


Boolean  ParseRate(char *, float *);
Boolean ParseColor(char *, char **, Widget , Widget);
int  ParseMarkPar(char *, struct mpar_object **, struct lisp_object **);
int  ParseRatePar(char *, struct rpar_object **);
Boolean ParseTag(char *, char **, Widget , Widget);
Boolean ParseRes(char *, char **, Widget , Widget);
Boolean ParseTransTag(char *, char **, Widget , Widget , struct trans_object *);
Boolean ParsePlaceTag(char *, char **, Widget , Widget , struct place_object *);
Boolean ParseMarkParTag(char *, char **, Widget , Widget , struct mpar_object *);
Boolean ParseColorTag(char *, char **, Widget , Widget , struct lisp_object *);
Boolean ParseNatural(char *, int *);
Boolean ParseRateParTag(char *, char **, Widget , Widget , struct rpar_object *);
Boolean ParseResTag(char *, char **, Widget , Widget , struct res_object *);
char *LTrim(char *);

enum {
    GOOD_RATE,
    BAD_RATE,
    NO_SUCH_RATE
};

#else
#	ifndef 	__PARSE__
#	define	__PARSE__

extern Boolean  ParseRate(char *, float *);
extern Boolean ParseColor(char *, char **, Widget , Widget);
extern int  ParseMarkPar(char *, struct mpar_object **, struct lisp_object **);
extern int  ParseRatePar(char *, struct rpar_object **);
extern Boolean ParseTag(char *, char **, Widget , Widget);
extern Boolean ParseRes(char *, char **, Widget , Widget);
extern Boolean ParseTransTag(char *, char **, Widget , Widget , struct trans_object *);
extern Boolean ParsePlaceTag(char *, char **, Widget , Widget , struct place_object *);
extern Boolean ParseMarkParTag(char *, char **, Widget , Widget , struct mpar_object *);
extern Boolean ParseColorTag(char *, char **, Widget , Widget , struct lisp_object *);
extern Boolean ParseNatural(char *, int *);
extern Boolean ParseRateParTag(char *, char **, Widget , Widget , struct rpar_object *);
extern Boolean ParseResTag(char *, char **, Widget , Widget , struct res_object *);
extern char *LTrim(char *);
enum {
    GOOD_RATE,
    BAD_RATE,
    NO_SUCH_RATE
};

#	endif
#endif
