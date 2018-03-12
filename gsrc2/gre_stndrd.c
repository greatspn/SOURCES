
/*
#define DEBUG
*/

unsigned p_num;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}
extern int yyparse();

static int sumop[100];
static int multop[100];
static int level_op = 0;
static int cur_intval;
static char cur_inttype;

FILE *nfp, * dfp, * ofp, *extra_def;

#include "const.h"


static unsigned no_pro = 0;
static unsigned no_res = 0;

char   **p_list;

char   **m_list;
unsigned *MP;

char   **r_list;
double *RP;

struct Cond_def {
    char op;
    char type;
    char top1;
    char top2;
    int op1;
    int op2;
    struct Cond_def *next;
};

union Val_def {
    int intval;
    double realval;
    unsigned place;
};

struct Prob_def {
    char op;
    char type;
    int num;
    int nocond;
    union Val_def val;
    struct Cond_def *cond;
    struct Prob_def *next;
};

struct Res_def {
    int nprobs;
    char *name;
    struct Prob_def *probs;
    struct Res_def *next;
};

static struct Cond_def *cur_cond = NULL;
static struct Prob_def *cur_prob = NULL;
static struct Res_def *cur_res = NULL;
static struct Res_def *first_res = NULL;

static struct Res_def *new_res;
static struct Prob_def *new_prob;
static struct Cond_def *new_cond;

static double cur_real;
static double cur_rat;
static char smant[20];
static char srat[40];
static char sreal[20];

int	nomp, place_num, trans_num, norp;

/* predeclaration */
void getname(char 	*name_pr);

char *ecalloc(unsigned nitm, unsigned sz) {
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}


void add_op(char op) {
    new_prob = (struct Prob_def *)ecalloc(1, sizeof(struct Prob_def));
    cur_res->probs = new_prob;
    ++(cur_res->nprobs);
    new_prob->next = cur_prob;
    cur_prob = new_prob;
    cur_prob->op = op;
    cur_prob->type = ' ';
    cur_prob->num = 0;
    cur_prob->nocond = 0;
    cur_cond = NULL;
#ifdef DEBUG
    fprintf(stderr, "  add_op %c\n", op);
#endif
}


void add_cond(char op) {
    new_cond = (struct Cond_def *)ecalloc(1, sizeof(struct Cond_def));
    cur_prob->cond = new_cond;
    ++(cur_prob->nocond);
    new_cond->next = cur_cond;
    cur_cond = new_cond;
    cur_cond->op = op;
    cur_cond->type = ' ';
#ifdef DEBUG
    fprintf(stderr, "  add_cond %c\n", op);
#endif
}


int checkplace(char *cp) {
    register int i = 0;
    char **p_n = p_list;

    while (i++ < place_num) {
        if (! strcmp(cp, *p_n))
            return (i);
        ++p_n;
    }
    return (0);
}

int checkmpar(char *cp) {
    register int i = 0;
    char **p_n = m_list;

    while (i++ < nomp) {
        if (! strcmp(cp, *p_n))
            return (i);
        ++p_n;
    }
    return (0);
}

int checkrpar(char *cp) {
    register int i = 0;
    char **p_n = r_list;

    while (i++ < norp) {
        if (! strcmp(cp, *p_n))
            return (i);
        ++p_n;
    }
    return (0);
}

void
load_net() {

    char		linebuf[LINEMAX];
    float		ftemp;
    int                 group_num, i, j, mark;
#define VBAR '|'

#ifdef DEBUG
    printf("  Start of load_net\n");
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
    printf("    %d marking parameters\n", nomp);
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
            printf("    %d %s=%d\n", i, linebuf, mark);
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
        fscanf(nfp, "%d ", &mark); while (getc(nfp) != '\n');
#ifdef DEBUG
        printf("    place %s\n", linebuf);
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
    printf("  End of load_net\n");
#endif
}


void getname(char 	*name_pr) {
#define	BLANK ' '
#define	EOLN  '\0'
    unsigned		i;

    for ((*name_pr) = fgetc(nfp) , i = 1 ;
            (*name_pr) != BLANK &&
            (*name_pr) != '\n' &&
            (*name_pr) != '\0' &&
            i++ <= TAG_SIZE ;
            (*(++name_pr)) = fgetc(nfp));
    (*name_pr) = EOLN;
}



int main(int argc, char **argv) {
    static char   *can_t_open = "Can't open file %s for %c\n";
    char  filename[LINEMAX];
    int ii, np, jj;
    struct Cond_def *cp;
    struct Prob_def *pp;
    struct Res_def *rp;

#ifdef DEBUG
    fprintf(stderr, "Start\n");
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
    load_net();
    (void) fclose(nfp);

    // Read other command line arguments (after the load_net() call)
    for (ii = 2; ii < argc; ii++) {
        // Change marking parameters on-the-fly
        if (0 == strcmp(argv[ii], "-mpar") && ii+2 < argc) {
            for (jj=0; jj<nomp; jj++)
                if (0 == strcmp(argv[ii + 1], m_list[jj]))
                    break;
            if (jj == nomp) {
                fprintf(stderr, "Error: no marking parameter named %s.\n", argv[ii + 1]);
                exit(1);
            }
            MP[jj] = atoi(argv[ii + 2]);
            fprintf(stderr, "Changing marking parameter %s value to %d.\n", m_list[jj], MP[jj]);
            ii += 2;
        }
        // Change rate parameters on-the-fly
        else if (0 == strcmp(argv[ii], "-rpar") && ii+2 < argc) {
            for (jj=0; jj<norp; jj++)
                if (0 == strcmp(argv[ii + 1], r_list[jj]))
                    break;
            if (jj == norp) {
                fprintf(stderr, "Error: no rate parameter named %s.\n", argv[ii + 1]);
                exit(1);
            }
            RP[jj] = atof(argv[ii + 2]);
            fprintf(stderr, "Changing rate parameter %s value to %lf.\n", r_list[jj], RP[jj]);
            ii += 2;
        }
        // Extra measure:  -measure <name> <expression>
        else if (0 == strcmp(argv[ii], "-measure") && ii+2 < argc) {
            if (extra_def == NULL) // Extra definition file still not opened
                extra_def = tmpfile(); // Temporary file that will be deleted at exit
            fprintf(extra_def, "|%s 0.0 0.0 : %s;\n", argv[ii+1], argv[ii+2]);
            ii += 2;
        }
        else {
            fprintf(stderr, "Could not understand command line argument %s.\n", argv[ii]);
            exit(1);
        }
    }
    for (jj=0; jj<nomp; jj++) {
        if (MP[jj] == -7134) {
            fprintf(stderr, "Specify all the marking parameter values with the -mpar <name> <value> command line arg.\n");
        }
    }
    for (jj=0; jj<norp; jj++) {
        if (RP[jj] == -7134.0) {
            fprintf(stderr, "Specify all the rate parameter values with the -mpar <name> <value> command line arg.\n");
        }
    }

    // Complete extra definitions file with the terminating bar, and restart for reading.
    if (extra_def != NULL) {
        fprintf(extra_def, "|\n");
        rewind(extra_def);
    }

    sprintf(filename, "%s.def", argv[1]);
    if ((dfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    while (getc(dfp) != '%');
    sprintf(filename, "%s.gst", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "  Start of yyparse\n");
#endif
    yyin = dfp;
    yyparse();
#ifdef DEBUG
    fprintf(stderr, "  End of yyparse\n");
#endif
    (void) fclose(dfp);
    fprintf(ofp, "%d %d\n", no_pro, no_res);
    for (rp = first_res ; rp != NULL ; rp = rp->next)
        for (pp = rp->probs ; pp != NULL ; pp = pp->next) {
            switch (pp->type) {
            case 'c' :
            case 'C' :
            case 'p' :
            case 'P' :
                fprintf(ofp, "%d %d %c", pp->num, pp->nocond,
                        pp->type);
                if (pp->type == 'c' || pp->type == 'C')
                    fprintf(ofp, " %d\n", pp->val.place);
                else
                    fprintf(ofp, "\n");
                for (cp = pp->cond ; cp != NULL ;
                        cp = cp->next) {
                    if (cp->op == 'c')
                        fprintf(ofp, " c %c %c %d %c %d\n",
                                cp->type, cp->top1, cp->op1,
                                cp->top2, cp->op2);
                    else
                        fprintf(ofp, " %c\n", cp->op);
                }
                break;
            case 'e' :
            case 'E' :
                fprintf(ofp, "%d %d %c %d\n", pp->num,
                        pp->nocond, pp->type, pp->val.place);
                break;
            default :
                break;
            }
        }
    for (ii = 0, rp = first_res ; rp != NULL ; rp = rp->next) {
        ++ii;
        np = rp->nprobs;
        fprintf(ofp, "%d %d %s\n", ii, np, rp->name);
        for (pp = rp->probs ; pp != NULL ; pp = pp->next) {
            fprintf(ofp, "  %c %c", pp->op, pp->type);
            switch (pp->type) {
            case 'p' :
            case 'P' :
            case 'c' :
            case 'C' :
            case 'e' :
            case 'E' :
                fprintf(ofp, " %d\n", pp->num);
                break;
            case 'r' :
                fprintf(ofp, " %f\n", pp->val.realval);
                break;
            case 'i' :
                fprintf(ofp, " %d\n", pp->val.intval);
                break;
            default :
                fprintf(ofp, "\n");
                break;
            }
        }
    }
    (void) fclose(ofp);
    return 0;
}
