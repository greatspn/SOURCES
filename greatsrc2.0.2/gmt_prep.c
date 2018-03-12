/*
 *  Paris, February 8, 1990
 *  program: gmt_prep.c
 *  purpose: precompute transition firing rates and weights
 *           for the mapping of the TRG of a GSPN onto an MC
 *           without net-dependent files compilation.
 *  programmer: Giovanni Chiola
 *  notes:
 *   1) No marking dependency is allowed for immediate transition weigths.
 */


/*
#define DEBUG
*/

#include <stdlib.h>
#include <stdio.h>

FILE *nfp, * ofp, *gfp, *dfp;

#include "const.h"

#ifndef Linux
extern double atof();
#endif

unsigned place_num, group_num, trans_num, nomp, norp;

double *RP;
char **r_names;

unsigned *MP;
char **m_names;

char **p_names;

static int sumop[100];
static int multop[100];
static int level_op = 0;
static int cur_intval;
static char cur_inttype;

static FILE *eof;

struct Cond_def {
    char op;
    char type;
    char top2;
    int p1;
    int op2;
    struct Cond_def *next;
};


union Val_type {
    double real;
    unsigned place;
};


struct Val_def {
    char op;
    char type;
    union Val_type val;
    struct Val_def *next;
};


struct Clause_def {
    char type;
    int nocond;
    int noval;
    struct Val_def *vals;
    struct Cond_def *conds;
    struct Clause_def *next;
};


yyerror(s) char *s; {
    fprintf(stderr, "%s\n", s);
}


char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *       calloc();
    char               *callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}


static struct Cond_def *cur_cond = NULL;
static struct Cond_def *new_cond;
static struct Val_def *cur_val = NULL;
static struct Val_def *new_val;
static struct Clause_def *cur_clause = NULL;
static struct Clause_def *new_clause;
static struct Clause_def *first_clause = NULL;

static double cur_real;
static double cur_rat;
static char smant[20];
static char srat[40];
static char sreal[20];


add_op(op)
char op;
{
    new_val = (struct Val_def *)ecalloc(1, sizeof(struct Val_def));
    cur_clause->vals = new_val;
    ++(cur_clause->noval);
    new_val->next = cur_val;
    cur_val = new_val;
    cur_val->op = op;
    cur_val->type = ' ';
#ifdef DEBUG
    fprintf(stderr, "  add_op %c\n", op);
#endif
}

static int num_clause;

add_clause(type)
char type;
{
    new_clause = (struct Clause_def *)ecalloc(1, sizeof(struct Clause_def));
    new_clause->next = NULL;
    if (cur_clause == NULL) {
        first_clause = new_clause;
        num_clause = 1;
    }
    else {
        ++num_clause;
        cur_clause->next = new_clause;
    }
    cur_clause = new_clause;
    cur_clause->type = type;
    cur_clause->noval = 0;
    cur_clause->nocond = 0;
    cur_val = NULL;
    cur_cond = NULL;
#ifdef DEBUG
    fprintf(stderr, "  add_clause %c\n", type);
#endif
}


add_cond(op)
char op;
{
    new_cond = (struct Cond_def *)ecalloc(1, sizeof(struct Cond_def));
    new_cond->next = cur_cond;
    cur_clause->conds = new_cond;
    cur_cond = new_cond;
    ++(cur_clause->nocond);
    cur_cond->op = op;
    cur_cond->type = ' ';
#ifdef DEBUG
    fprintf(stderr, "  add_cond %c\n", op);
#endif
}

read_distrib(policy, distrib)
int policy, distrib;
{
    int nost = distrib & TIMING_NOSTAGES;
    int d_type = distrib & TIMING_DISTRIBUTION;
    double dd;

#ifdef DEBUG
    fprintf(stderr, "  Start read_distrib policy=%d, distrib=%d\n",
            policy, distrib);
#endif
    num_clause = -1;
    if (d_type == TIMING_ERLANG)
        nost = 1;
    else
        nost++;
    fprintf(ofp, " %d %d %d %d\n", num_clause, nost, policy, distrib);
    while (nost--) {
        switch (distrib & TIMING_DISTRIBUTION) {
        case TIMING_LINEAR :
        case TIMING_DISCRETE :
        case TIMING_COX_normalized :
        case TIMING_IPEREXP :
            fscanf(dfp, "%lg", &dd); fprintf(ofp, "   %lg", dd);
            if ((distrib & TIMING_DISTRIBUTION) ==  TIMING_LINEAR) {
                fscanf(dfp, "%lg", &dd); fprintf(ofp, " %lg", dd);
            }
            fscanf(dfp, "%lg", &dd); fprintf(ofp, " %lg\n", dd);
            break;
        case TIMING_ERLANG :
            fscanf(dfp, "%lg", &dd); fprintf(ofp, "   %lg\n", dd);
            break;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "  End read_distrib\n");
#endif
}

print_md() {
    if (num_clause < 0)   /* general distribution for simulation */
        return;
    fprintf(ofp, " %d\n", num_clause);
    for (cur_clause = first_clause ; first_clause != NULL ;
            cur_clause = first_clause) {
        first_clause = cur_clause->next;
        fprintf(ofp, "  %c %d %d\n", cur_clause->type,
                cur_clause->nocond, cur_clause->noval);
        for (cur_cond = cur_clause->conds ; cur_cond != NULL ;
                cur_cond = new_cond) {
            new_cond = cur_cond->next;
            if (cur_cond->op == 'c')
                fprintf(ofp, "      c %c %d %c %d\n", cur_cond->type,
                        cur_cond->p1, cur_cond->top2, cur_cond->op2);
            else
                fprintf(ofp, "      %c\n", cur_cond->op);
            free(cur_cond);
        }
        for (cur_val = cur_clause->vals ; cur_val != NULL ;
                cur_val = new_val) {
            new_val = cur_val->next;
            if (cur_val->op == 'o') {
                fprintf(ofp, "      o %c ", cur_val->type);
                if (cur_val->type == 'p')
                    fprintf(ofp, "%d\n", cur_val->val.place);
                else
                    fprintf(ofp, "%f\n", cur_val->val.real);
            }
            else
                fprintf(ofp, "      %c\n", cur_val->op);
            free(cur_val);
        }
        free(cur_clause);
    }
}


int checkplace(cp)
char *cp;
{
    register int i = 0;
    char **p_n = p_names;

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
    char **p_n = m_names;

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
    char **p_n = r_names;

    while (i++ < norp) {
        if (! strcmp(cp, *p_n))
            return (i);
        ++p_n;
    }
}


read_noobjects() {
    char linebuf[LINEMAX];
    int  i, j;
    char cc;
    char **charpp;
    unsigned *upp;
    int sub_num, ecs_num, jj2, ii, nn, kk, n2, np, jj, ll;
#define VBAR '|'

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

    MP = (unsigned *)ecalloc(nomp, sizeof(unsigned));
    RP = (double *)ecalloc(norp, sizeof(double));
    p_names = (char **)ecalloc(place_num, sizeof(char *));
    m_names = (char **)ecalloc(nomp, sizeof(char *));
    r_names = (char **)ecalloc(norp, sizeof(char *));

    /* skip marking parameters */
    {
        register int i = 0;
        unsigned int mark;
        for (charpp = m_names, upp = MP; i++ < nomp ; ++charpp, ++upp) {
            getname(linebuf);
            *charpp = ecalloc(strlen(linebuf) + 1, 1);
            sprintf(*charpp, linebuf);
            fscanf(nfp, "%d", &mark);
            *upp = mark;
#ifdef DEBUG
            fprintf(stderr, "  ...reading mark par. %s %d\n", linebuf, mark);
#endif
            while (getc(nfp) != '\n');
        }
    }

    /* skip places */
    i = 0;
    for (charpp = p_names;  i++ < place_num ; ++charpp) {
        getname(linebuf);
        *charpp = ecalloc(strlen(linebuf) + 1, 1);
        sprintf(*charpp, linebuf);
#ifdef DEBUG
        fprintf(stderr, "  ...place %s\n", linebuf);
#endif
        while (getc(nfp) != '\n');
    }

    /* read number of objects in the grg file */
    fscanf(gfp, "%d %d %d %d\n", &sub_num, &place_num, &ecs_num, &trans_num);

    /* skip codings */
    for (nn = 0 ; nn < sub_num ; nn++) {
        fscanf(gfp, "%*d %d\n", &jj2);
        for (ii = 0 ; ++ii <= jj2 ;) {
            fscanf(gfp, "%*d %*d %*d %d", &np);
            while (getc(gfp) != '\n');
            for (kk = 0 ; kk++ < np ;) {
                while (getc(gfp) != '\n');
            }
        }
    }

    /* skip places */
    for (ii = 0 ; ii++ < place_num ;) {
        while (getc(gfp) != '\n');
    }

    /* skip groups */
    for (ii = 0 ; ii++ < ecs_num;) {
        fscanf(gfp, "%*d %d\n", &kk);
        for (jj = 0 ; jj++ < kk ;) {
            fscanf(gfp, " %*d %d", &n2);
            while (getc(gfp) != '\n');
            for (ll = 0 ; ll++ < n2 ;) {
                while (getc(gfp) != '\n');
            }
        }
    }

}


getname(name_pr)
char   *name_pr;
{
#define BLANK ' '
#define EOLN  '\0'
    unsigned            i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != NULL &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}



read_trans() {
    unsigned *t_ecs;
    unsigned *t_perm;
    int ii, jj, nt, nb, nc, nge, nlt;
    char		linebuf[LINEMAX];
    double		ftemp;
    double              ddd;
    int                 knd, noar, load_d;
    char **charpp;


#ifdef DEBUG
    printf("  Start of read_trans\n");
#endif

    yyin = dfp;

    t_ecs = (unsigned *)ecalloc(trans_num + 1, sizeof(unsigned));
    t_perm = (unsigned *)ecalloc(trans_num + 1, sizeof(unsigned));
    /* read transitions */
    for (nt = 0 ; nt++ < trans_num ;) {
        fscanf(gfp, "%d %d %*d %*d %*d %d %d", &ii, &knd, &nb, &nc);
        while (getc(gfp) != '\n');
        t_perm[ii] = nt;
        t_ecs[ii] = knd;
        for (ii = 0 ; ++ii <= nb ;) {
            fscanf(gfp, "  %*d %d %d\n", &nge, &nlt);
            nge += nlt;
            for (jj = 0 ; ++jj <= nge ;) {
                while (getc(gfp) != '\n');
            }
        }
        for (ii = 0 ; ++ii <= nc ;) {
            while (getc(gfp) != '\n');
        }
        while (getc(gfp) != '\n');
        while (getc(gfp) != '\n');
    }

    /* read rate parameters */
    ii = 0;
    charpp = r_names;
    while (ii < norp) {
        getname(linebuf);
        *charpp = ecalloc(strlen(linebuf) + 1, 1);
        sprintf(*charpp, linebuf);
        fscanf(nfp, "%lf", &ftemp); while (getc(nfp) != '\n');
        RP[ii++] = ftemp;
        ++charpp;
#ifdef DEBUG
        fprintf(stderr, "  reading rate par. %s = %f\n", linebuf, ftemp);
#endif
    }

    /* skip groups */
    ii = 1;
    while (ii++ <= group_num) {
        getname(linebuf);
#ifdef DEBUG
        fprintf(stderr, "  ...skipping group %s\n", linebuf);
#endif
        while (getc(nfp) != '\n');
    }

    fprintf(ofp, "%d\n", trans_num);
    /* read transitions */
    for (nt = 0 ; ++nt <= trans_num ;) {
        getname(linebuf);
#ifdef DEBUG
        fprintf(stderr, "  reading trans. %s\n", linebuf);
#endif
        fscanf(nfp, "%lf %d %d %d", &ftemp, &load_d, &knd, &noar);
        while (getc(nfp) != '\n');
        if (ftemp < 0.0) {
            int nrp;
            if ((nrp = (int)(-ftemp)) > norp) {
                int nn;
                if (knd /* || TRUE */) {
                    fprintf(stderr,
                            "\n***  Sorry: no marking dependency is allowed!\n");
                    exit(1);
                }
                /*yylex(); printf("%c YYLEX\n",*yytext);
                while ( *yytext != '|' ){
                   yylex(); printf("%c YYLEX\n", *yytext);}
                yylex(); printf("%s YYLEX\n", yytext);*/
                for (yylex(); *yytext != '|' ; yylex());
                yylex();
                nn = atoi(yytext);
                /*fscanf(dfp,"%d", &nn);*/
                if (nn != nt) {
                    fprintf(stderr, "ERROR: file '.def' is inconsistent!\n");
                    exit(55);
                }
#ifdef DEBUG
                fprintf(stderr, "    Start of generate_t_rate M-D\n");
#endif
                fprintf(ofp, " %d %d %d -512.0", t_perm[nt], t_ecs[nt], knd);
                cur_clause = NULL;
                yyparse();
                print_md();
#ifdef DEBUG
                fprintf(stderr, "    End of generate_t_rate M-D\n");
#endif
            }
            else {
                ftemp = RP[nrp - 1];
                fprintf(ofp, "%d %d %d %lf 0\n", t_perm[nt], t_ecs[nt], knd,
                        ftemp);
            }
        }
        else {
            fprintf(ofp, "%d %d %d %lf 0\n", t_perm[nt], t_ecs[nt], knd, ftemp);
        }
        if (load_d < 0) {
            ii = -load_d - 1;
            ddd = ftemp;
        }
        else
            ii = 0;
        fprintf(ofp, " %d", ii);
        while (ii--) {
            fscanf(nfp, "%lf\n", &ftemp);
            fprintf(ofp, " %lf", ftemp * ddd);
        }
        fprintf(ofp, "\n");
        skiparcs(noar);
        fscanf(nfp, "%d\n", &noar);
        skiparcs(noar);
        fscanf(nfp, "%d\n", &noar);
        skiparcs(noar);
    }
    free(t_ecs);
    free(t_perm);


#ifdef DEBUG
    printf("  End of read_trans\n");
#endif
}

skiparcs(noar)
int	noar;
{
    int			i, j, pl, mlt, ip;
    float		x, y;

    for (i = noar ; i-- ;) {
        fscanf(nfp, "%d %d %d", &mlt, &pl, &ip);
        while (getc(nfp) != '\n');
        for (j = 0; j++ < ip; fscanf(nfp, "%f %f\n", &x, &y));
    }
}


main(argc, argv)
int argc;
char **argv;
{
    extern FILE *fopen();

    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[100];

#ifdef DEBUG
    printf("Start\n");
#endif
    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(filename, "%s.net", argv[1]);
    if ((nfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    sprintf(filename, "%s.grg", argv[1]);
    if ((gfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    read_noobjects();
    sprintf(filename, "%s.gmt", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s.def", argv[1]);
    if ((dfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    if ((eof = fopen("empty", "w")) == NULL) {
        fprintf(stderr, "Couldn't create empty file!\n");
        exit(22);
    }
    fclose(eof);
    if ((eof = fopen("empty", "r")) == NULL) {
        fprintf(stderr, "Couldn't open empty file!\n");
        exit(23);
    }
    read_trans();
    fclose(dfp);
    fclose(nfp);
    fclose(ofp);
#ifdef DEBUG
    printf("End\n");
#endif
    return 0;
}


