/**************************************************************/
/* Funziona anche con archi spezzati                          */
/* Funziona anche con commenti multilinea  		      */
/* Considera solo transizioni esponenziali   		      */
/**************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"

#define STRINGSIZE 256
#define FORMSIZE 1000

#define LINESIZE 1000

#define MAXRANGE 32
#define VBAR '|'


#define MAXLAYER 16
#define MAXSTRING 256
#define MAXLIV 10


/* definizione del tipo bool, a valori su 0 e 1    */
typedef short bool;



/* definizione del tipo che descrive un singolo posto */

typedef struct {
    char tag[MAXSTRING];    /* tag del posto */
    int num_token; /* old MO */
    float xp, yp, xg, yg;    /* coordinate xp,yp del posto e xg,yg del tag */
    int livello;    /* livello a cui il posto appartiene */
    int index; /* indice del posto nel layer */
    int write;  /* indica se è gia stato scritto il termine per questo posto */
    int lbound; /* limite inferiore */
    int ubound; /* limite superiore */
} POSTI;


/* definizione della struttura atta a contenere le coordinate degli archi
   che presentano spezzamenti */

typedef struct { float x; float y; } COORD_ARCHI;


/* definizione del singolo elemento della lista dei posti di input (output) */
typedef struct    {
    int moltepl;
    /* primo elemento della riga, molteplicita' */
    int update;
    int write;
    int p_index;
    /* indice del posto di input (output) all'interno
       dell'insieme del livello */
    int spezzato;
    /* numero di punti intermedi dell'arco */
    int livello;
    COORD_ARCHI *list;
    /* puntatore alle coordinate spezzamenti degki archi */
} POSTI_I_O;


/* definizione del tipo che descrive una singola transizione, con le
   informazioni aggiuntive su quali siano i posti di input e output,
   con i relativi archi  */

typedef struct {
    char tag[MAXSTRING];
    /* tag della transizione */
    float rate;
    /* tasso della transizione */
    int serv;
    /* terzo elemento della riga */
    int type;
    /* tipo della transizione (timmed, immediate) */
    int n_in;
    /* numero di posti di input */
    int inclinazione;
    /* gradi di inclinazione della transizione: 90, 45, etc */
    float xt, yt, xg, yg, xr, yr;
    /* coordinate xp,yp della transizione,  xg,yg del tag
       e xr, yr del rate */
    int num_l;
    /* numero di livelli a cui la transizione appartiene */
    int livello[MAXLIV];
    /* livelli a cui la transizione appartiene */
    POSTI_I_O *in_list;
    /* puntatore alla lista dei posti di input e relativi
       archi */
    int n_out;
    /* numero di posti di output */
    POSTI_I_O *out_list;
    /* puntatore alla lista dei posti di output e relativi    */
    int n_hinib;
    /* numero di posti inibitori */
    POSTI_I_O *hinib_list;
} TRANSIZIONI;




/* VARIABILI */

int num_livelli;

FILE *nfp;  /* sara' il .net, file di descrizione della SSA con livelli */
FILE *nfpp; /* sara` il .def */
FILE *bndfp; /* file dei bounds */

int markp_num, pl_num, ratep_num, tr_num, gr_num, cset_num, lay_num;
/* contengono la "prima" riga del file .net */

POSTI *pun_posti;

TRANSIZIONI *pun_transizioni;

struct rpar_object *rpar, *rpar_index;
struct mpar_object *mpar;


char linebuf[MAXSTRING];
char layer[MAXLAYER][MAXSTRING];
char c;

main(argc, argv)
int argc;
char *argv[];
{

    FILE  *fp;
    char nome[STRINGSIZE];
    int i;



    if (argc < 2) {
        fprintf(stderr, "ERROR: no net name !\n");
        exit(1);
    }

    sprintf(nome, "%s.net", argv[1]);
    nfp = fopen(nome, "r");
    if (!nfp) { fprintf(stderr, "Unable to open file nets/%s for r\n", nome);  exit(1); }


    loadnet();

    fclose(nfp);

    sprintf(nome, "%s.bnd", argv[1]);
    bndfp = fopen(nome, "r");
    if (!bndfp) { fprintf(stderr, "Unable to open file bounds/%s for r\n", nome);  exit(1); }

    loadbounds();

    fclose(bndfp);

    sprintf(nome, "%s.sm", argv[1]);



    if ((fp = fopen(nome, "w+")) == NULL) {
        printf("NON HO APERTO IL FILE %s\n", nome);
        exit(-1);
    }

    print_prism(fp, argv[1]);

    fclose(fp);
}




/* ********************************************************************* */

getname(name_pr)
char           *name_pr;
{
#define BLANK ' '
#define EOLN  '\0'
    short           i;

    for ((*name_pr) = fgetc(nfp), i = 1;
            (*name_pr) != BLANK && (*name_pr) != '\n' &&
            (*name_pr) != (char)0 && i++ <= TAG_SIZE;
            (*(++name_pr)) = fgetc(nfp));
    if (*name_pr != BLANK) {
        char            c;
        for (c = fgetc(nfp);
                c != BLANK && c != '\n' && c != (char)0;
                c = fgetc(nfp));
    }
    (*name_pr) = EOLN;
}




reverse(s)
char s[];
{
    int c, i, j;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}


itoa(n, s)
int n;
char s[];
{
    int i, segno;
    if ((segno = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    }
    while ((n /= 10) > 0);
    if (segno < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

bool isempty(s)
char *s;
{
    if (s == NULL || *s == '\0')
        return (TRUE);
    else
        return (FALSE);
}



loadnet() {
    int i, j, com;
    int num, goon, ln;
    int temp_liv, npunti, old_index;
    COORD_ARCHI *p_arc;
    POSTI_I_O *pun_p_i, *pun_p_o, *pun_p_h;
    int *num_l;




    /* skip first line containing '|0|' */
    fgets(linebuf, LINESIZE, nfp);

    /* skip comment */
    for (; ;) {
        fgets(linebuf, LINESIZE, nfp);
        if (linebuf[0] == VBAR) break;
    }

    fscanf(nfp, "f   %d %d %d %d %d %d %d\n", &markp_num, &pl_num,
           &ratep_num, &tr_num, &gr_num, &cset_num, &lay_num);




    mpar = (struct mpar_object *) calloc(markp_num, sizeof(struct mpar_object));

    i = 0;
    while (i < markp_num) {
        float x, y;
        short value;
        int goon;

        mpar[i].tag = (char *) malloc(TAG_SIZE);
        getname(mpar[i].tag);
//     printf("STR %s eeeeeee\n", rpar[i].tag);
        fscanf(nfp, "%d %f %f ", &value, &x, &y);

        if (goon = fscanf(nfp, "%d", &ln)) {
            for (; goon && ln > 0; goon = fscanf(nfp, "%d", &ln));
            while (getc(nfp) != '\n');
        }

        mpar[i].value = value;

        i++;
    }


    pun_posti = (POSTI *) calloc(pl_num, sizeof(POSTI));



    /* legge le informazioni del posto */

    for (j = 0; j < pl_num; j++) {
        fscanf(nfp, "%s   %d %f %f %f %f %d ",
               pun_posti[j].tag, &pun_posti[j].num_token,
               &pun_posti[j].xp, &pun_posti[j].yp, &pun_posti[j].xg,
               &pun_posti[j].yg, &pun_posti[j].livello) ;


        printf("RGESG4 %s \n", pun_posti[j].tag);
        {
            int ii;
            ii = pun_posti[j].livello;
            while (ii != 0) { fscanf(nfp, "%d", &ii);  }
            fscanf(nfp, "\n");
        }
    }



    /* *** read groups *** */

    /* Cosa sono ???, non dovrebbero mai essere presenti */

    for (i = 0 ; i < gr_num ; i++) {
        getname(linebuf);
        while ((c = getc(nfp)) != '\n');
    }



#ifdef DEBUG
    fprintf(stderr, "reading Rate Parameters\n");
    fflush(stderr);
#endif
    /* read rate parameters */



    rpar = (struct rpar_object *) calloc(ratep_num, sizeof(struct rpar_object));

    i = 0;
    while (i < ratep_num) {
        float ftemp, x, y;
        int goon;



        rpar[i].tag = (char *) malloc(TAG_SIZE);
        getname(rpar[i].tag);
//     printf("STR %s eeeeeee\n", rpar[i].tag);
        fscanf(nfp, "%f %f %f ", &ftemp, &x, &y);

        if (goon = fscanf(nfp, "%d", &ln)) {
            for (; goon && ln > 0; goon = fscanf(nfp, "%d", &ln));
            while (getc(nfp) != '\n');
        }

        rpar[i].value = ftemp;

        i++;
    }


    pun_transizioni = (TRANSIZIONI *) calloc(tr_num, sizeof(TRANSIZIONI));


    /* leggi le informazioni sulle transizioni */

    for (j = 0; j < tr_num; j++) {
        fscanf(nfp, "%s  %f %d   %d   %d %d %f %f %f %f %f %f %d ",
               pun_transizioni[j].tag, &pun_transizioni[j].rate,
               &pun_transizioni[j].serv, &pun_transizioni[j].type,
               &pun_transizioni[j].n_in, &pun_transizioni[j].inclinazione,
               &pun_transizioni[j].xt, &pun_transizioni[j].yt,
               &pun_transizioni[j].xg, &pun_transizioni[j].yg,
               &pun_transizioni[j].xr, &pun_transizioni[j].yr, &com);

        {
            int ii;
            ii = com;
            while (ii != 0) { fscanf(nfp, "%d", &ii);}
            fscanf(nfp, "\n");
        }


        pun_p_i = (POSTI_I_O *) calloc(pun_transizioni[j].n_in, sizeof(POSTI_I_O));

        for (i = 0; i < pun_transizioni[j].n_in ; i++) {
            fscanf(nfp, "  %d   %d   %d  %d  ",
                   &pun_p_i[i].moltepl, &pun_p_i[i].p_index,
                   &pun_p_i[i].spezzato, &com);
            {
                int ii;
                ii = com;
                while (ii != 0) { fscanf(nfp, "%d", &ii);}
                fscanf(nfp, "\n");
            }
            pun_p_i[i].moltepl = 0;
            pun_p_i[i].write = 0;



            p_arc = (COORD_ARCHI *) calloc(pun_p_i[i].spezzato, sizeof(COORD_ARCHI));

            for (npunti = 0; npunti < pun_p_i[i].spezzato; npunti++)
                fscanf(nfp, "%f %f\n", &p_arc[npunti].x, &p_arc[npunti].y);

            pun_p_i[i].list = p_arc;
        }

        fscanf(nfp, "%d\n", &pun_transizioni[j].n_out);

        pun_p_o = (POSTI_I_O *) calloc(pun_transizioni[j].n_out, sizeof(POSTI_I_O));

        for (i = 0; i < pun_transizioni[j].n_out ; i++) {
            fscanf(nfp, "  %d   %d   %d  %d  ",
                   &(pun_p_o[i].moltepl), &pun_p_o[i].p_index,
                   &pun_p_o[i].spezzato, &com);

            {
                int ii;
                ii = com;
                while (ii != 0) { fscanf(nfp, "%d", &ii);}
                fscanf(nfp, "\n");
            }

            pun_p_o[i].moltepl = 0;
            pun_p_o[i].write = 0;

            p_arc = (COORD_ARCHI *) calloc(pun_p_o[i].spezzato, sizeof(COORD_ARCHI));

            for (npunti = 0; npunti < pun_p_o[i].spezzato; npunti++)
                fscanf(nfp, "%f %f\n", &p_arc[npunti].x, &p_arc[npunti].y);

            pun_p_o[i].list = p_arc;
        }



        /* lettura degli inibitori */

        fscanf(nfp, "%d\n", &pun_transizioni[j].n_hinib);

        pun_p_h = (POSTI_I_O *) calloc(pun_transizioni[j].n_hinib, sizeof(POSTI_I_O));

        for (i = 0; i < pun_transizioni[j].n_hinib ; i++) {
            fscanf(nfp, "  %d   %d   %d  %d  ",
                   &(pun_p_h[i].moltepl), &pun_p_h[i].p_index,
                   &pun_p_h[i].spezzato, &com);

            {
                int ii;
                ii = com;
                while (ii != 0) { fscanf(nfp, "%d", &ii);}
                fscanf(nfp, "\n");
            }
            pun_p_h[i].moltepl = 0;
            pun_p_h[i].moltepl = 0;

            p_arc = (COORD_ARCHI *) calloc(pun_p_h[i].spezzato, sizeof(COORD_ARCHI));

            for (npunti = 0; npunti < pun_p_h[i].spezzato; npunti++)
                fscanf(nfp, "%f %f\n", &p_arc[npunti].x, &p_arc[npunti].y);

            pun_p_h[i].list = p_arc;
        }




        pun_transizioni[j].in_list = pun_p_i;
        pun_transizioni[j].out_list = pun_p_o;
        pun_transizioni[j].hinib_list = pun_p_h;
    }

}



loadbounds() {
    int j;

    for (j = 0; j < pl_num; j++) { /* Leggo limite inferiore e superiore di ciascun posto */
        fscanf(bndfp, "%d %d\n",
               &pun_posti[j].lbound, &pun_posti[j].ubound);
    }
}


print_prism(fp, nome)
FILE *fp;
char nome[STRINGSIZE];
{
    char line[STRINGSIZE];
    char guard_str[FORMSIZE];  /* buffer per l'espressione guard */
    char rate_str[FORMSIZE];   /* buffer per il rate */
    char update_str[FORMSIZE]; /* buffer per l'update */

    int k = 0; /*indice per creare nomi assoluti per gli archi */
    int i = 0, j = 0;
    POSTI_I_O *pun_p_i, * pun_p_o, * pun_p_h;
    struct rpar_object *rpar_index;


    /* Intestazione */
    sprintf(line, "stochastic");
    fprintf(fp, "%s\n\n", line);
    fflush(fp);

    for (i = 0; i < ratep_num; i++) { /* per ogni rate parameter */
        sprintf(line, "const double %s = %f;", rpar[i].tag, rpar[i].value);
        fprintf(fp, "%s\n", line);
    }
    fprintf(fp, "\n");

    for (i = 0; i < markp_num; i++) { /* per ogni marking parameter*/
        sprintf(line, "const int %s = %d;", mpar[i].tag, mpar[i].value);
        fprintf(fp, "%s\n", line);
    }
    fprintf(fp, "\n");





    sprintf(line, "module M");
    fprintf(fp, "%s\n", line);
    fflush(fp);



    for (i = 0; i < pl_num; i++) { /* per ogni posto */
        sprintf(line, "%s : [%d..%d]", pun_posti[i].tag, pun_posti[i].lbound, pun_posti[i].ubound);
        fprintf(fp, "%s", line); /* definisco il range tra limite inferiore e superiore */
        if (pun_posti[i].num_token != 0) {  /* se specificato il marker iniziale */

            if (pun_posti[i].num_token >= 0)
                sprintf(line, " init %d;", pun_posti[i].num_token);
            else
                sprintf(line, " init %s;", mpar[-(int)(pun_posti[i].num_token) - 1].tag);

        }
        else
            sprintf(line, ";");
        fprintf(fp, "%s \n", line);
    }

    fprintf(fp, "\n");

    for (i = 0; i < tr_num; i++) { /* per ogni transizione */



        if (pun_transizioni[i].type == 0) { /* considero solo transizioni esponenziali */

            guard_str[0] = rate_str[0] = update_str[0] = '\0';   /* azzero le stringhe */


            if (pun_transizioni[i].rate >= 0)
                sprintf(rate_str, "%f", pun_transizioni[i].rate); /* scrivo il rate */
            else
                sprintf(rate_str, "%f", rpar[-(int)(pun_transizioni[i].rate) - 1].value); /* scrivo il rate */



            /* archi di ingresso */
            if (pun_transizioni[i].n_in != 0) {
                pun_p_i = pun_transizioni[i].in_list;
                for (j = 0; j < pun_transizioni[i].n_in; j++) {

                    if (isempty(guard_str) == TRUE) {
                        sprintf(guard_str, "(%s > %d)", pun_posti[pun_p_i[j].p_index - 1].tag, pun_p_i[j].moltepl - 1); /* scrivo termine */
                    }
                    else {
                        sprintf(guard_str, "%s & (%s > %d) ", guard_str, pun_posti[pun_p_i[j].p_index - 1].tag, pun_p_i[j].moltepl - 1); /* scrivo termine */
                    }

                    if (isempty(update_str) == TRUE)
                        sprintf(update_str, "(%s' = %s %+d)", pun_posti[pun_p_i[j].p_index - 1].tag, pun_posti[pun_p_i[j].p_index - 1].tag, - pun_p_i[j].moltepl); /* scrivo termine */
                    else
                        sprintf(update_str, "%s & (%s' = %s %+d)", update_str, pun_posti[pun_p_i[j].p_index - 1].tag, pun_posti[pun_p_i[j].p_index - 1].tag, - pun_p_i[j].moltepl); /* scrivo termine */
                }
            }


            if (pun_transizioni[i].n_hinib != 0) {
                pun_p_h = pun_transizioni[i].hinib_list;
                for (j = 0; j < pun_transizioni[i].n_hinib; j++)
                    if (isempty(guard_str) == TRUE)
                        sprintf(guard_str, "(%s < %d) ", pun_posti[pun_p_h[j].p_index - 1].tag, pun_p_h[j].moltepl); /* scrivo termine */
                    else
                        sprintf(guard_str, "%s & (%s < %d) ", guard_str, pun_posti[pun_p_h[j].p_index - 1].tag, pun_p_h[j].moltepl); /* scrivo termine */


            }

            /* archi di uscita */
            if (pun_transizioni[i].n_out != 0) {
                pun_p_o = pun_transizioni[i].out_list;
                for (j = 0; j < pun_transizioni[i].n_out; j++) {
                    /*	      pun_posti[pun_p_o[j].p_index-1].update += pun_p_o[j].moltepl; /* aggiorno con i token da aggiungere */
// 	      if(pun_posti[pun_p_o[j].p_index-1].update !=0) /* se è necessario aggiornare */
                    if (isempty(update_str) == TRUE)
                        sprintf(update_str, "(%s' = %s %+d)", pun_posti[pun_p_o[j].p_index - 1].tag, pun_posti[pun_p_o[j].p_index - 1].tag, pun_p_o[j].moltepl); /* scrivo termine */
                    else
                        sprintf(update_str, "%s & (%s' = %s %+d)", update_str, pun_posti[pun_p_o[j].p_index - 1].tag, pun_posti[pun_p_o[j].p_index - 1].tag, pun_p_o[j].moltepl); /* scrivo termine */

                }
            }


            if (isempty(guard_str) == TRUE)
                fprintf(fp, "	[%s] true -> %s : %s; \n", pun_transizioni[i].tag, rate_str, update_str);
            else
                fprintf(fp, "	[%s] %s -> %s : %s; \n", pun_transizioni[i].tag, guard_str, rate_str, update_str);
        } /* fine transizioni*/
        else
            fprintf(fp, "Errore transizione non esponenziale \n");




    }
    fprintf(fp, "endmodule \n");
}





