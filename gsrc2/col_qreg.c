
/*
#define DEBUG
*/

yyerror(s) char *s; {
    fprintf(stderr, "%s\n", s);
}

FILE *nfp, * dfp, * ofp;

#include "const.h"

static int check_flag = FALSE;

char def_name[256];

char   **p_list;

char   **m_list;
unsigned *MP;

char   **r_list;
double *RP;

struct Name_list {
    char *str;
    int from, to;
    struct Name_list *next;
};

static struct Name_list *cur_elist;
static struct Name_list *e_table = NULL;

union Name_def {
    char *str;
    char **arr;
    struct Class_def *c_ind;
    struct Set_def *s_ind;
    struct Dom_def *d_ind;
};

struct Class_def {
    char type;
    char used_in_set;
    int from;
    int num;
    char *name;
    union Name_def nn;
    struct Class_def *next;
};

struct Set_def {
    char type;
    char bypass;
    int card;
    char *name;
    union Name_def class;
    struct Set_def *next;
};

struct Dom_def {
    char type;
    char bypass;
    int dim;
    int card;
    char *name;
    union Name_def sets;
    struct Dom_def *next;
};


static struct Dom_def *cur_dom = NULL;
static struct Set_def *cur_set = NULL;
static struct Class_def *cur_class = NULL;

static struct Dom_def *new_dom = NULL;
static struct Set_def *new_set = NULL;
static struct Class_def *new_class = NULL;


int	nomp, place_num, trans_num, norp;

static unsigned no_pre = 0;
static unsigned no_col = 0;
static unsigned no_mar = 0;
static unsigned no_fun = 0;


char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *	calloc();
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}


int
search_name(nn, tt, from, to)
char *nn;
struct Name_list *tt;
int from, to;
{
#ifdef DEBUG
    fprintf(stderr, "      Start of search_name %s %d %d\n", nn, from, to);
#endif
    while (tt != NULL) {
        char *cp1, * cp2;
        for (cp1 = tt->str, cp2 = nn ;
                *cp1 != '\0' && *cp1 == *cp2 ; ++cp1, ++cp2);
        if (*cp1 == '\0' && *cp2 == '\0') {
            if (tt->from < 0 && from < 0) {
#ifdef DEBUG
                fprintf(stderr, "      End of search_name TRUE\n");
#endif
                return (TRUE);
            }
            if (tt->from >= 0 && from >= 0) {
                if (to >= tt->from && from <= tt->to) {
#ifdef DEBUG
                    fprintf(stderr, "      End of search_name TRUE\n");
#endif
                    return (TRUE);
                }
            }
        }
        else if (*cp1 == '\0' && *cp2 >= '0' && *cp2 <= '9'
                 && tt->from >= 0) {
            if (*cp2 > '0' || *(cp2 + 1) != '0') {
                for (cp1 = cp2 + 1 ;
                        *cp1 >= '0' && *cp1 <= '9' ; ++cp1);
                if (*cp1 == '\0') {
                    int ii;
                    if (from >= 0) {
                        char sss[256];
                        int jj;
                        sprintf(sss, "%s%d", cp2, from);
                        sscanf(sss, "%d", &ii);
                        sprintf(sss, "%s%d", cp2, to);
                        sscanf(sss, "%d", &jj);
                        if (jj >= tt->from && ii <= tt->to) {
#ifdef DEBUG
                            fprintf(stderr, "      End of search_name TRUE\n");
#endif
                            return (TRUE);
                        }
                    }
                    else {
                        sscanf(cp2, "%d", &ii);
                        if (ii >= tt->from && ii <= tt->to) {
#ifdef DEBUG
                            fprintf(stderr, "      End of search_name TRUE\n");
#endif
                            return (TRUE);
                        }
                    }
                }
            }
        }
        else if (*cp2 == '\0' && *cp1 >= '0' && *cp1 <= '9'
                 && from >= 0) {
            if (*cp1 > '0' || *(cp1 + 1) != '0') {
                for (cp2 = cp1 + 1 ;
                        *cp2 >= '0' && *cp2 <= '9' ; ++cp2);
                if (*cp2 == '\0') {
                    int ii;
                    if (tt->from >= 0) {
                        char sss[256];
                        int jj;
                        sprintf(sss, "%s%d", cp1, tt->from);
                        sscanf(sss, "%d", &ii);
                        sprintf(sss, "%s%d", cp1, tt->to);
                        sscanf(sss, "%d", &jj);
                        if (to >= ii && from <= jj) {
#ifdef DEBUG
                            fprintf(stderr, "      End of search_name TRUE\n");
#endif
                            return (TRUE);
                        }
                    }
                    else {
                        sscanf(cp1, "%d", &ii);
                        if (ii >= from && ii <= to) {
#ifdef DEBUG
                            fprintf(stderr, "      End of search_name TRUE\n");
#endif
                            return (TRUE);
                        }
                    }
                }
            }
        }
        tt = tt->next;
    }
#ifdef DEBUG
    fprintf(stderr, "      End of search_name FALSE\n");
#endif
    return (FALSE);
}


add_elist() {
    struct Name_list *new_n;
    char *cp;
    int ff, tt;

#ifdef DEBUG
    fprintf(stderr, "    Start of add_elist\n");
#endif
    if (cur_class->type == 'n') {
        ff = cur_class->from;
        tt = ff + cur_class->num - 1;
    }
    else {
        ++(cur_class->num);
        ff = tt = -1;
    }
    if (search_name(name, cur_elist, ff, tt) ||
            search_name(name, e_table, ff, tt)) {
        if (cur_class->type == 'n') {
            fprintf(stderr, "\nERROR: repeated definition of some colour");
            fprintf(stderr, " '%s[%d-%d]'\n", name, ff, tt);
        }
        else
            fprintf(stderr, "\nERROR: repeated definition of colour '%s'\n",
                    name);
        exit(1);
    }
    new_n = (struct Name_list *)ecalloc(1, sizeof(struct Name_list));
    new_n->from = ff;
    new_n->to = tt;
    new_n->str = cp = ecalloc(strlen(name) + 1, 1);
    sprintf(cp, name);
    new_n->next = cur_elist;
    cur_elist = new_n;
#ifdef DEBUG
    fprintf(stderr, "    End of add_elist\n");
#endif
}


convert_elist() {
    struct Name_list *new_n;
    char **cpp;
    int ii;

#ifdef DEBUG
    fprintf(stderr, "    Start of convert_elist (type=%c)\n", cur_class->type);
#endif
    switch (cur_class->type) {
    case 'n' :
        cur_elist->next = e_table;
        cur_class->nn.str = ecalloc(strlen(name) + 1, 1);
        sprintf(cur_class->nn.str, name);
        break;
    case 'l' :
        ii = cur_class->num;
        cur_class->nn.arr = cpp = (char **)ecalloc(ii, sizeof(char *));
        for (new_n = cur_elist ; ii-- ; ++cpp) {
            *cpp = new_n->str;
            if (new_n->next != NULL)
                new_n = new_n->next;
        }
        new_n->next = e_table;
        break;
    }
    e_table = cur_elist;
    cur_elist = NULL;
#ifdef DEBUG
    fprintf(stderr, "    End of convert_elist\n");
#endif
}


add_class(tt)
char tt;
{
#ifdef DEBUG
    fprintf(stderr, "    Start of add_class %c\n", tt);
#endif
    new_class = (struct Class_def *)ecalloc(1, sizeof(struct Class_def));
    new_class->type = tt;
    new_class->next = cur_class;
    cur_class = new_class;
    cur_class->name = NULL;
    cur_class->used_in_set = 0;
#ifdef DEBUG
    fprintf(stderr, "    End of add_class\n");
#endif
}


add_set(tt)
char tt;
{
#ifdef DEBUG
    fprintf(stderr, "    Start of add_set %c\n", tt);
#endif
    new_set = (struct Set_def *)ecalloc(1, sizeof(struct Set_def));
    new_set->next = cur_set;
    cur_set = new_set;
    cur_set->name = NULL;
    cur_set->bypass = FALSE;
    cur_set->class.c_ind = cur_class;
    switch ((new_set->type = tt)) {
    case 'u' :
        cur_set->card = cur_class->num;
        for (cur_class = cur_class->next ; cur_class != NULL ;
                cur_class = cur_class->next)
            cur_set->card += cur_class->num;
        break;
    case 's' :
        cur_set->card = cur_class->num;
        cur_class = NULL;
        break;
    case '?' :
        cur_set->card = 0;
        cur_class = NULL;
        break;
    }
#ifdef DEBUG
    fprintf(stderr, "    End of add_set\n");
#endif
}


add_dom(tt)
char tt;
{
#ifdef DEBUG
    fprintf(stderr, "    Start of add_dom %c\n", tt);
#endif
    new_dom = (struct Dom_def *)ecalloc(1, sizeof(struct Dom_def));
    new_dom->next = cur_dom;
    cur_dom = new_dom;
    cur_dom->name = NULL;
    cur_dom->bypass = FALSE;
    cur_dom->type = tt;
#ifdef DEBUG
    fprintf(stderr, "    End of add_dom\n");
#endif
}


end_dom() {
#ifdef DEBUG
    fprintf(stderr, "    Start of end_dom (bypass=%d)\n", bypass);
#endif
    cur_dom->sets.s_ind = cur_set;
    cur_set = NULL;
    cur_dom->dim = cur_dim;
    cur_dom->card = cur_card;
    cur_dom->bypass = bypass;
    if (cur_dom->type != '?') {
        ++no_col;
    }
    bypass = TRUE;
#ifdef DEBUG
    fprintf(stderr, "    End of end_dom\n");
#endif
}


check_set(new_dom)
struct Dom_def *new_dom;
{
    struct Dom_def *dom_p;
    int d_card = 1;

#ifdef DEBUG
    fprintf(stderr, "  Start of check_set (new_dom=%d)\n", (int)new_dom);
#endif
    for (; new_dom != NULL ; new_dom = new_dom->next) {
        for (new_set = new_dom->sets.s_ind ; new_set != NULL ;
                new_set = new_set->next) {
            switch (new_set->type) {
            case '?' :
            case '*' :
                for (dom_p = cur_dom ; dom_p != NULL ; dom_p = dom_p->next)
                    if ((dom_p->name != NULL) && (dom_p->dim == 1) &&
                            (strcmp(dom_p->name, new_set->class.str) == 0))
                        break;
                if (dom_p == NULL) {
                    fprintf(stderr, "\nERROR: colour set '%s' undefined !\n",
                            new_set->class.str);
                    exit(1);
                }
                switch (dom_p->sets.s_ind->type) {
                case '*' :
                    fprintf(stderr, "\nERROR: circular colourset definition:");
                    fprintf(stderr, " '%s' -> '%s' -> '%s' !\n",
                            new_set->class.str,
                            dom_p->sets.str, new_set->class.str);
                    exit(1);
                case 's' :
                    new_set->type = '@';
                    new_set->class.s_ind = dom_p->sets.s_ind;
                    break;
                case 'u' :
                    new_set->type = 'U';
                    new_set->class.s_ind = dom_p->sets.s_ind;
                    break;
                case '?' :
                    new_set->type = '*';
                    check_set(dom_p);
                case '@' :
                case 'U' :
                    new_set->type = dom_p->sets.s_ind->type;
                    new_set->class.s_ind = dom_p->sets.s_ind->class.s_ind;
                    break;
                }
            case '@' :
            case 'U' :
                cur_set = new_set->class.s_ind;
                break;
            case 'u' :
            case 's' :
                cur_set = new_set;
                break;
            }
            new_set->card = check_class(cur_set);
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End of check_set\n");
#endif
}

check_class(new_set)
struct Set_def *new_set;
{
    struct Dom_def *dom_p;
    struct Set_def *set_p;
    int s_card = 0;

#ifdef DEBUG
    fprintf(stderr, "    Start of check_class (new_set=%d)\n", (int)new_set);
#endif
    for (new_class = new_set->class.c_ind ; new_class != NULL ;
            new_class = new_class->next) {
        switch (new_class->type) {
        case '?' :
        case '*' :
            for (dom_p = cur_dom ; dom_p != NULL ; dom_p = dom_p->next)
                if ((dom_p->name != NULL) && (dom_p->bypass) &&
                        ((set_p = dom_p->sets.s_ind)->bypass) &&
                        (strcmp(dom_p->name, new_class->nn.str) == 0)) {
                    break;
                }
            if (dom_p == NULL) {
                fprintf(stderr, "\nERROR: colour set '%s' undefined !\n",
                        new_class->nn.str);
                exit(1);
            }
            switch (set_p->class.c_ind->type) {
            case '*' :
                fprintf(stderr, "\nERROR: circular class definition:");
                fprintf(stderr, " '%s' -> '%s' -> '%s' !\n",
                        new_class->nn.str,
                        set_p->class.str, new_class->nn.str);
                exit(1);
            case 'l' :
            case 'n' :
                new_class->type = '@';
                new_class->nn.c_ind = set_p->class.c_ind;
                break;
            case '?' :
                new_class->type = '*';
                check_class(set_p);
            case '@' :
                new_class->type = '@';
                new_class->nn.c_ind = set_p->class.c_ind;
                break;
            }
        case '@' :
            cur_class = new_class->nn.c_ind;
            break;
        case 'l' :
        case 'n' :
            cur_class = new_class;
            break;
        }
        if (new_set->type == 'u' || new_set->type == 'U')
            cur_class->used_in_set += 2;
        if (cur_class->used_in_set > 2) {
            fprintf(stderr, "\nERROR: class '%s' is used in more than",
                    cur_class->name);
            fprintf(stderr, " one colourset !\n");
            exit(1);
        }
        if (new_class->name == NULL)
            cur_class->used_in_set = 1;
        s_card += cur_class->num;
    }
#ifdef DEBUG
    fprintf(stderr, "    End of check_class %d\n", s_card);
#endif
    return (s_card);
}


int checkplace(cp)
char *cp;
{
    register int i = 0;
    char **p_n = p_list;

    while (i++ < place_num) {
        if (! strcmp(cp, *p_n))
            return (i);
        ++p_n;
    }
    return (0);
}


int checkmpar(cp)
char *cp;
{
    register int i = 0;
    char **p_n = m_list;

    while (i++ < nomp) {
        if (! strcmp(cp, *p_n))
            return (i);
        ++p_n;
    }
    return (0);
}


int checkrpar(cp)
char *cp;
{
    register int i = 0;
    char **p_n = r_list;

    while (i++ < norp) {
        if (! strcmp(cp, *p_n))
            return (i);
        ++p_n;
    }
    return (0);
}


load_net() {
    extern              getname();

    char		linebuf[LINEMAX], cc;
    float		ftemp, x, y;
    int                 group_num, i, j, knd, noar, ii, orien;
#define VBAR '|'

#ifdef DEBUG
    fprintf(stderr, "  Start of load_net\n");
#endif

    /* skip first line containing '|0|' */
    fgets(linebuf, LINEMAX, nfp);

    /* skip comment */
    for (; ;) {
        fgets(linebuf, LINEMAX, nfp);
        if (linebuf[0] == VBAR) break;
    }

    /* read number of objects in the net */

    fscanf(nfp, "f %d %d %d", &nomp, &place_num, &norp);
    fscanf(nfp, "%d %d %d %d\n", &trans_num, &group_num, &i, &j);
    p_list = (char **)ecalloc(place_num, sizeof(char *));
    m_list = (char **)ecalloc(nomp, sizeof(char *));
    MP = (unsigned *)ecalloc(nomp, sizeof(unsigned));
    r_list = (char **)ecalloc(norp, sizeof(char *));
    RP = (double *)ecalloc(norp, sizeof(double));

#ifdef DEBUG
    fprintf(stderr, "    %d marking parameters\n", nomp);
#endif

    /* read marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        while (i < nomp) {
            getname(linebuf);
            m_list[i] = (char *)ecalloc(strlen(linebuf) + 1, sizeof(char));
            strcpy(m_list[i], linebuf);
            fscanf(nfp, "%d", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
            fprintf(stderr, "    %d %s=%d\n", i, linebuf, mark);
#endif
            MP[i++] = mark;
        }
    }

    /* read places */
    i = 0;
    while (i < place_num) {
        getname(linebuf);
        p_list[i] = (char *)ecalloc(strlen(linebuf) + 1, sizeof(char));
        strcpy(p_list[i], linebuf);
        fscanf(nfp, "%*d %*f %*f %*f %*f %d", &ii);
        while (ii)
            fscanf(nfp, "%d", &ii);
        while ((cc = getc(nfp)) == ' ');
        if (cc == '\n') {
        }
        else {
            ungetc(cc, nfp);
            fscanf(nfp, "%*f %*f");
            while ((cc = getc(nfp)) == ' ');
            if (cc == '@') {
            }
            else {
                ungetc('@', nfp);
            }
            while (getc(nfp) != '\n');
        }
#ifdef DEBUG
        fprintf(stderr, "    place %s\n", linebuf);
#endif
        i++;
    }

    /* read rate parameters */
    i = 0;
    while (i < norp) {
        getname(linebuf);
        r_list[i] = (char *)ecalloc(strlen(linebuf) + 1, sizeof(char));
        strcpy(r_list[i], linebuf);
        fscanf(nfp, "%f", &ftemp); while (getc(nfp) != '\n');
        RP[i++] = ftemp;
    }

#ifdef DEBUG
    fprintf(stderr, "  End of load_net\n");
#endif
}


getname(name_pr)
char 	*name_pr;
{
#define	BLANK ' '
#define	EOLN  '\0'
    unsigned		i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != NULL &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}



main(argc, argv)
int argc;
char **argv;
{
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[LINEMAX], cc;
    int ii, np;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.def", argv[1]);
    if ((dfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "  ... skip MD-rates\n");
#endif
    while ((cc = getc(dfp)) != '%') {
#ifdef DEBUG
        putc(cc, stderr);
#endif
    }
    while (getc(dfp) != '\n');
    while (getc(dfp) == '|' && getc(dfp) != '\n')
        while (getc(dfp) != '\n');
    check_flag = FALSE;
    while (getc(dfp) == '(') {
        fscanf(dfp, "%s %c %*f %*f", def_name, &cc);
        if (cc == 'c') {
            while (getc(dfp) != '(');
#ifdef DEBUG
            fprintf(stderr, "  Start of yyparse (%s cc=%c)\n", def_name, cc);
#endif
            yyin = dfp;
            yyparse();
#ifdef DEBUG
            fprintf(stderr, "  End of yyparse no_col=%d\n", no_col);
#endif
            if (cur_dom->type == '#' || cur_dom->type == '?') {
                fprintf(stderr, "\nERROR: illegal definition ");
                fprintf(stderr, "for colour domain '%s' !\n", def_name);
                exit(1);
            }
            cur_dom->name = ecalloc(strlen(def_name) + 1, 1);
            sprintf(cur_dom->name, def_name);
            if (cur_dom->bypass) {
                cur_dom->sets.s_ind->name = cur_dom->name;
                if (cur_dom->sets.s_ind->bypass)
                    cur_dom->sets.s_ind->class.c_ind->name = cur_dom->name;
            }
            while (getc(dfp) != '\n');
        }
        else {
            char c1, c2;
            while (getc(dfp) != '\n');
            do {
                if ((c1 = getc(dfp)) != '\n') {
                    if ((c2 = getc(dfp)) != '\n')
                        while (getc(dfp) != '\n');
                }
                else
                    c2 = '\n';
            }
            while (c1 != ')' || c2 != ')');
        }
    }
    (void) fclose(dfp);
    if (no_col) {
        check_set(cur_dom);
    }
    if ((dfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "  ... skip MD-rates\n");
#endif
    while ((cc = getc(dfp)) != '%') {
#ifdef DEBUG
        putc(cc, stderr);
#endif
    }
    while (getc(dfp) != '\n');
    while (getc(dfp) == '|' && getc(dfp) != '\n')
        while (getc(dfp) != '\n');
    check_flag = FALSE;
    while (getc(dfp) == '(') {
        fscanf(dfp, "%s %c %*f %*f", def_name, &cc);
        if (cc != 'c') {
            while (getc(dfp) != '(');
#ifdef DEBUG
            fprintf(stderr, "  Start of yyparse (%s cc=%c)\n", def_name, cc);
#endif
            yyin = dfp;
            yyparse();
#ifdef DEBUG
            fprintf(stderr, "  End of yyparse no_pre=%d, no_mar=%d, no_fun=%d\n",
                    no_pre, no_mar, no_fun);
#endif
            switch (cc) {
            case 'p' :
                break;
            case 'f' :
                break;
            case 'm' :
                break;
            }
            while (getc(dfp) != '\n');
        }
        else {
            char c1, c2;
            while (getc(dfp) != '\n');
            do {
                if ((c1 = getc(dfp)) != '\n') {
                    if ((c2 = getc(dfp)) != '\n')
                        while (getc(dfp) != '\n');
                }
                else
                    c2 = '\n';
            }
            while (c1 != ')' || c2 != ')');
        }
    }
    (void) fclose(dfp);
    check_flag = TRUE;
    fprintf(stderr, "Temporary End of program\n");
    exit(0);
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    load_net();
    (void) fclose(nfp);
    sprintf(filename, "%s_U.net", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    (void) fclose(ofp);
}
