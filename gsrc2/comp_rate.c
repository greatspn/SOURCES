/*
#define DEBUG
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


yyerror(s) char *s; {
    fprintf(stderr, "%s\n", s);
}

extern FILE *dfp, * rafp, * ofp, * rsfp, * ssfp;

#include "const.h"

#include "comp.h"

extern FILE *yyin;
extern int	place_num;
char   **p_names;

int nomp;
ROWP MP;
char   **m_names;

int norp;
float 	*RP;
char   **r_names;


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


extern int no_byte;

extern int	trans_num;

static some_timed_MD = FALSE;
static some_imm_MD = FALSE;

static FILE *eof;

start_generate_rate() {
    extern FILE *fopen();

#ifdef DEBUG
    fprintf(stderr, "  Start of start_generate_rate\n");
#endif
    fprintf(rafp, "\n#include <stdio.h>\n\n");
    fprintf(rafp, "/* DECODED MARKING */ extern unsigned char DP[];\n\n");
    fprintf(rafp, "\n/* TRANSITION RATES */ double rate_from_RG ( rgfp )\n");
    fprintf(rafp, "FILE * rgfp;\n{ unsigned long nt;\n");
    fprintf(rafp, "  load_compact(&nt,rgfp);\n  switch ( nt ) {\n");

    fprintf(ssfp, "\ndouble ERROR2[%d];\n", place_num + 1);
    fprintf(ssfp, "double precq[%d];\n", place_num + 1);
    fprintf(ssfp, "double gammo[%d];\n", place_num + 1);
    fprintf(ssfp, "double gammoq[%d];\n", place_num + 1);
    fprintf(ssfp, "double gamips[%d];\n", place_num + 1);
    fprintf(ssfp, "double mintok[%d];\n", place_num + 1);
    fprintf(ssfp, "double maxtok[%d];\n", place_num + 1);

    fprintf(rsfp, "\n#include <stdio.h>\n\n");
    fprintf(rsfp, "/* DECODED MARKING */ extern unsigned char DP[];\n\n");
    fprintf(rsfp, "\n/* TRANSITION RATES */ double rate_sim ( nt, Mp )\n");
    fprintf(rsfp, "unsigned long nt;\nchar * Mp;\n{\n");
    fprintf(rsfp, "  switch ( nt ) {\n");
    if ((eof = fopen("empty", "w")) == NULL) {
        fprintf(stderr, "Couldn't create empty file!\n");
        exit(22);
    }
    fclose(eof);
    if ((eof = fopen("empty", "r")) == NULL) {
        fprintf(stderr, "Couldn't open empty file!\n");
        exit(23);
    }
#ifdef DEBUG
    fprintf(stderr, "  End of start_generate_rate\n");
#endif
}

void
generate_t_rate(ftemp, load_d, nt, pri)
float ftemp;
int load_d, nt, pri;
{
    unsigned nn;

#ifdef DEBUG
    fprintf(stderr, "    Start of generate_t_rate %f %d %d\n",
            ftemp, load_d, nt);
#endif
    fprintf(rafp, "  case %d :\n", nt);
    fprintf(rsfp, "  case %d :\n", nt);
    if (ftemp <= 0.0) {
        int i = (int)((-ftemp) + 0.5);
        if (--i >= norp) {    /* Marking-Dependent rate  */
            if (pri)
                some_imm_MD = TRUE;
            else
                some_timed_MD = TRUE;
            while (getc(dfp) != '|');
            fscanf(dfp, "%d", &nn);
            if (nn != nt) {
                fprintf(stderr, "ERROR: file '.def' is inconsistent!\n");
                exit(55);
            }
            yyin = dfp;
            yyparse();
#ifdef DEBUG
            fprintf(stderr, "    End of generate_t_rate M-D\n");
#endif
            return;
        }
        ftemp = RP[i];
    }
    fprintf(rsfp, "    return((double)%f);\n", ftemp);
    if (load_d == 1) {
        fprintf(rafp, "    return((double)%f);\n", ftemp);
    }
    else {
        fprintf(rafp, "    load_compact(&nt,rgfp);\n");
        fprintf(rafp, "    return((double)nt*%f);\n", ftemp);
    }
#ifdef DEBUG
    fprintf(stderr, "    End of generate_t_rate\n");
#endif
}


terminate_t_rate_gen() {

#ifdef DEBUG
    fprintf(stderr, "    Start of terminate_t_rate_gen\n");
#endif
    fprintf(rafp, "  default : fprintf(stderr,\"ERROR:");
    fprintf(rafp, " try to compute rate for transition #%%d\\n\",nt);\n");
    fprintf(rafp, "     exit(60);\n  }\n}\n\n");

    fprintf(rsfp, "  default : fprintf(stderr,\"ERROR:");
    fprintf(rsfp, " try to compute rate for transition #%%d\\n\",nt);\n");
    fprintf(rsfp, "     exit(60);\n  }\n}\n\n");
    fprintf(ofp, "unsigned load_vrs = %d;\n\n", some_imm_MD);
    fprintf(rafp, "unsigned load_vrs = %d;\n\n", some_imm_MD);
    fprintf(rafp, "unsigned load_trs = %d;\n\n", some_timed_MD);
    fclose(eof);
#ifdef DEBUG
    fprintf(stderr, "    End of terminate_t_rate_gen\n");
#endif
}
