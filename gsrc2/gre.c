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

/* predeclaration */
void getname(char 	*name_pr);

FILE *nfp, * dfp, * ofp, * rfp, *sfp;

#include "const.h"



static unsigned no_pro = 0;
static unsigned no_res = 0;

char   **p_list;

char   **m_list;
unsigned *MP;

char   **r_list;
double *RP;


int	nomp, place_num, trans_num, norp;


char *ecalloc(unsigned nitm, unsigned sz) {
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
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


void load_net() {
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
    sprintf(filename, "%s.def", argv[1]);
    if ((dfp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'r');
        exit(1);
    }
    while (getc(dfp) != '%');
    sprintf(filename, "%s_cres.c", argv[1]);
    if ((rfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s_cress.c", argv[1]);
    if ((sfp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    sprintf(filename, "%s_cpro.c", argv[1]);
    if ((ofp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, can_t_open, filename, 'w');
        exit(1);
    }
    fprintf(rfp, "\n#include <stdio.h>\n");
    fprintf(rfp, "\nextern double STIMA[];\n");
    fprintf(rfp, "\nextern double ERROR[];\n");
    fprintf(rfp, "\nprint_res(ofp)\nFILE * ofp;\n{\n");
    fprintf(sfp, "\nextern double PRO[];\n");
    fprintf(sfp, "\nextern double PROD[];\n");
    fprintf(sfp, "\nextern double tr_mean_tok();\n");
    fprintf(sfp, "\ncompute_sim()\n{\n");
    fprintf(ofp, "\nextern unsigned char DP[];\n");
    fprintf(ofp, "\nextern double PRO[];\n");
    fprintf(ofp, "\ncompute_pro( val, ival )\n");
    fprintf(ofp, "double val, ival;\n{ double * pp = PRO;\n");
    yyin = dfp;
    yyparse();
    (void) fclose(dfp);
    fprintf(rfp, "}\n\n");
    (void) fclose(rfp);
    fprintf(sfp, "}\n\n");
    fprintf(sfp, "double PROD[%d];\n", no_res + 1);
    fprintf(sfp, "double STIMA[%d];\n", no_res + 1);
    fprintf(sfp, "double ERROR[%d];\n", no_res + 1);
    (void) fclose(sfp);
    if (no_pro > 0)
        fprintf(ofp, "}\n\ndouble PRO[%d];\n", no_pro);
    else
        fprintf(ofp, "}\n\ndouble PRO[1];\n");
    fprintf(ofp, "\nunsigned maxprob = %d;\n", no_pro);
    fprintf(ofp, "\nunsigned maxres = %d;\n\n", no_res);
    (void) fclose(ofp);
    return 0;
}
