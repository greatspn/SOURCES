/*
 * object.h
 *
 * Definitions of objects used in Petri net editor.
 */

typedef  unsigned long LayerListElement;
typedef LayerListElement *LayerPun;
typedef  unsigned Layer;

union f_fp {
    float           ff;
    float          *fp;
};


struct coordinate {
    float           x, y;
    struct coordinate *next;
};

struct position {
    float           x, y;
};

struct mpar_object {
    char           *tag;
    LayerPun        layer;
    short           value;
    struct position center;
    struct mpar_object *next;
};


struct lisp_object {
    char           *tag;
    struct position center;
    char            type;
    /*   struct com_object *text; */
    char *text;
    struct lisp_object *next;
};

struct place_object {
    char           *tag;
    struct position tagpos;
    char           *color;
    struct lisp_object *lisp;
    struct position colpos;
    LayerPun           layer;
    short           tokens;
    short           m0;
    struct mpar_object *mpar;
    struct lisp_object *cmark;
    struct position center;
    struct place_object *next;
};

struct rpar_object {
    char           *tag;
    LayerPun           layer;
    float           value;
    struct position center;
    struct rpar_object *next;
};

struct com_object {
    char           *line;
    struct com_object *next;
};

struct trans_object {
    char           *tag;
    struct position tagpos;
    struct position pripos;
    char           *color;
    struct lisp_object *lisp;
    struct position colpos;
    LayerPun           layer;
    short           kind;
    short           enabl;

    /* used in firing */ short enabled;
    union f_fp      fire_rate;
    struct rpar_object *rpar;
    char  *mark_dep;

    struct arc_object *arcs;

    struct position center;
    struct position ratepos;
    short           orient;
    struct trans_object *next;
};

struct group_object {
    char           *tag;
    short           pri;
    struct trans_object *trans;
    struct position center;
    struct group_object *movelink;
    struct group_object *next;
};

struct arc_object {
    char            type;
    LayerPun           layer;
    short           mult;
    struct place_object *place;
    struct trans_object *trans;
    struct coordinate *point;
    char           *color;
    struct lisp_object *lisp;
    struct position colpos;
    struct arc_object *next;
};

struct tok_object {
    struct tarc    *larc;
    struct tarc    *pas;
    struct tarc    *jum;
    struct tarc    *mpl;
    struct coordinate *pt;
    struct coordinate *tt;
    struct tarc    *lspez;
    struct coordinate *token;
    struct coordinate *coor;
    struct tarc    *rep1;
    struct tok_object *next;
};


struct tarc {
    float           arr;
    struct tarc    *next;
};


struct text_object {
    int             font, size, style;
    int             height, length;
    struct position swcorner;
    char            cstring[80];
    struct text_object *next;
};

struct res_object {
    char           *tag;
    struct position center;
    float           value1;
    float           value2;
    /*   struct com_object *text;  */
    char *text;
    struct res_object *next;
};

struct net_object {
    char *comment;
    struct position nwcorner;
    struct position secorner;
    int             boxht;
    int             boxwid;
    int             circlerad;
    struct mpar_object *mpars;
    struct place_object *places;
    struct rpar_object *rpars;
    struct trans_object *trans;
    struct group_object *groups;
    struct res_object *results;
    struct lisp_object *lisps;
    struct text_object *texts;
    struct net_object *next;
};
#define                 TOKOBJ_SIZE     (unsigned)sizeof(struct tok_object)
#define         COORD_SIZE  (unsigned)sizeof(struct coordinate)
#define         PROB_SIZE   (unsigned)sizeof(struct probability)
#define                 TARC_SIZE       (unsigned)sizeof(struct tarc)
#define                 AWAY_SIZE       (unsigned)sizeof(struct away)
#define                 SPEZ_SIZE       (unsigned)sizeof(struct spez)
#define         MPAOBJ_SIZE (unsigned)sizeof(struct mpar_object)
#define         PLAOBJ_SIZE (unsigned)sizeof(struct place_object)
#define         RPAOBJ_SIZE (unsigned)sizeof(struct rpar_object)
#define         TRNOBJ_SIZE (unsigned)sizeof(struct trans_object)
#define         GRPOBJ_SIZE (unsigned)sizeof(struct group_object)
#define         ARCOBJ_SIZE (unsigned)sizeof(struct arc_object)
#define         RESOBJ_SIZE (unsigned)sizeof(struct res_object)
#define         LISPOBJ_SIZE    (unsigned)sizeof(struct lisp_object)
#define         TEXOBJ_SIZE (unsigned)sizeof(struct text_object)
#define         CMMOBJ_SIZE (unsigned)sizeof(struct com_object)
#define         NETOBJ_SIZE (unsigned)sizeof(struct net_object)

#define         TAG_SIZE    (unsigned)32

/**********************  Generic object code  **********************/

#define             PLACE       1
#define             IMTRANS     2
#define             EXTRANS     3
#define             DETRANS     4
#define             IO_ARC      5
#define             INH_ARC     6
#define             MPAR        7
#define             MARK        7
#define             RPAR        8
#define             RATE        8
#define             RESULT      9
#define             COLOR       10
#define             NAME        11

/*
 * #define              TEXT        14
 */

#define             LISP        15

/***********************  Object attributes  *************************/

#define HORIZONTAL  0   /* horizontal transition */
#define VERTICAL    1   /* vertical transition */
#define FORTYFIVE   2   /* slandish transition */
#define MINUSFORTYFIVE  3   /* slandish transition */

#define TO_TRANS    'i' /* arc type flag: place -> trans */
#define TO_PLACE    'o' /* arc type flag: trans -> place */
#define INHIBITOR   'h' /* arc type flag: inhibitor arc  */

#define EXPONENTIAL 0   /* exponential timed transition */
#define IMMEDIATE   1   /* immediate transition */
#define DETERMINISTIC   127 /* deterministic timed transition */

/***********************************************************************/
