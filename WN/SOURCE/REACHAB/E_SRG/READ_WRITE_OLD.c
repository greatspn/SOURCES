#include <stdio.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"


extern Canonic_p sfl_h;

TO_MERGEP *TO_MERGE(int ncl, struct COLORS *tabc) {
    int i, j;
    int *sc = NULL;
    TO_MERGEP *merg = NULL;
    TO_MERGEP var = NULL;

    merg = (TO_MERGEP *)calloc(ncl, sizeof(TO_MERGEP));

    for (i = 0; i < ncl; i++) {

        var = (TO_MERGEP)malloc(sizeof(struct TO_MERGE));
        var->NB = tabc[i].sbc_num;
        var->sbc_num = 0;
        var->SSCs = (int *)calloc(tabc[i].sbc_num, sizeof(int));
        for (j = 0; j < tabc[i].sbc_num; j++)  var->SSCs[j] = j;

        var->next = NULL;
        var->prev = NULL;
        merg[i] = var;
    }
    return merg;
}



void FIRE_INSTANCE(Event_p ev_p, FILE *fp, int sym)

{
    int tr = GET_TRANSITION_INDEX(ev_p);
    int ii;
    int cl;
    char name[MAX_TAG_SIZE];

    fprintf(fp, "%s :", tabt[tr].trans_name);

    if (GET_TRANSITION_COMPONENTS(tr) > 0) {
        cl = tabt[tr].dominio[0];
        if (ev_p->npla[0] != UNKNOWN) {

            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                if (sym == 0) {
                    int sb;

                    sb = get_static_subclass(cl, ev_p->npla[0]);
                    sprintf(name, "%s_%d", GET_STATIC_NAME(cl, sb),
                            /*my_get_dynamic_from_total(cl,ev_p->npla[0])*/ev_p->npla[0] - GET_STATIC_OFFSET(cl, sb));

                }
                else
                    sprintf(name, "%s_%d", tabc[cl].col_name, ev_p->npla[0]);

            else
                get_object_name(cl, ev_p->npla[0], name);
        }
        else
            strcpy(name, "Unk");

        if (strcmp(tabt[tr].names[0], "?"))
            fprintf(fp, "BINDING  : %s <-- %s", tabt[tr].names[0], name);
        else
            fprintf(fp, "BINDING  : (implicit projection) <-- %s", name);

        for (ii = 1; ii < tabt[tr].comp_num; ii++) {
            cl = tabt[tr].dominio[ii];
            if (ev_p->npla[ii] != UNKNOWN) {

                if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))

                    if (sym == 0) {
                        int sb;

                        sb = get_static_subclass(cl, ev_p->npla[ii]);
                        sprintf(name, "%s_%d", GET_STATIC_NAME(cl, sb),
                                /*my_get_dynamic_from_total(cl,ev_p->npla[0])*/ev_p->npla[ii] - GET_STATIC_OFFSET(cl, sb));

                    }
                    else
                        sprintf(name, "%s_%d", tabc[cl].col_name, ev_p->npla[ii]);

                else
                    get_object_name(cl, ev_p->npla[ii], name);

            }
            else
                strcpy(name, "Unk");
            if (strcmp(tabt[tr].names[ii], "?"))
                fprintf(fp, ", %s <-- %s ", tabt[tr].names[ii], name);
            else
                fprintf(fp, ", (implicit projection) <-- %s ", name);
        }
    }
    else
        fprintf(fp, "NO BINDING (non-colored)");

    fprintf(fp, "\n");
}


void ESM_SOURCE(FILE *fp, Tree_p ESM, int EVENT) {
    int i, k, l, s;
    int ARRAY[MAX_CARD];

    fprintf(fp, "%s%d", "ESM", ESM->marking->cont_tang);

    if (EVENT) {
        fprintf(fp, "%s", ":");
        for (i = 0; i < ncl; i++) {
            if (ASYM_STATIC_STORE[i].sbc_num == 1) {
                for (l = 0; l < SYM_TOT[i]; l++) {
                    fprintf(fp, "%s_%d%s", tabc[i].col_name, sfl_h->min[i][l], "[");
                    fprintf(fp, "%s%s_%d%s%d%s", "(", ASYM_STATIC_STORE[i].ptr[0].name, l, ":",
                            RESULT[i][0][sfl_h->min[i][l]][0] , ")");
                }
            }
            else {
                for (s = 0; s < MAX_CARD; s++) ARRAY[s] = 0;
                for (l = 0; l < SYM_TOT[i]; l++) {
                    fprintf(fp, "%s_%d%s", tabc[i].col_name, l, "[");
                    for (k = 0; k < ASYM_STATIC_STORE[i].sbc_num; k++)
                        if (RESULT[i][0][l][k] > 0)

                        {
                            fprintf(fp, "%s%s_%d%s%d%s", "(", ASYM_STATIC_STORE[i].ptr[k].name, ARRAY[k], ":", RESULT[i][0][l][k], ")");
                            ARRAY[k] += 1;
                        }

                    fprintf(fp, "%s", "]");
                }
            }
        }
    }
    fprintf(fp, "\n");
}
void WRITE_ON_ESRG(FILE *srg)

{
    int pl, k, cl;
    int offset, sb, base;
    char obj_name[MAX_TAG_SIZE];

    for (pl = 0 ; pl < npl; pl++) {
        if (IS_FULL(pl)) {
            fprintf(srg, "%s(", PLACE_NAME(pl));
            if (IS_NEUTRAL(pl)) {
                fprintf(srg, "%d)", net_mark[pl].total);
            }
            else {
                Token_p tk_p;

                for (tk_p = net_mark[pl].marking; tk_p != NULL; tk_p = NEXT_TOKEN(tk_p)) {
                    fprintf(srg, "%d<", tk_p->molt);
                    for (k = 0 ; k < tabp[pl].comp_num - 1; k++) {
                        cl = tabp[pl].dominio[k];

                        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))

                            sprintf(obj_name, "%s_%d", tabc[cl].col_name, tk_p->id[k]);

                        else
                            get_object_name(cl, tk_p->id[k], obj_name);

                        fprintf(srg, "%s,", obj_name);
                    }
                    cl = tabp[pl].dominio[k];

                    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))

                        sprintf(obj_name, "%s_%d", tabc[cl].col_name, tk_p->id[k]);
                    else
                        get_object_name(cl, tk_p->id[k], obj_name);
                    fprintf(srg, "%s>", obj_name);
                }
                fprintf(srg, ")");
            }

        }
    }
    fprintf(srg, "\n");

    for (cl = 0 ; cl < ncl; cl++) {
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            for (sb = 0, base = 0; sb < tabc[cl].sbc_num; sb++) {
                for (k = 0; k < GET_NUM_SS(cl, sb); k++) {

                    offset = sfl_h->min[cl][k + base];
                    sprintf(obj_name, "%s_%d", tabc[cl].col_name, offset);
                    fprintf(srg, "|%s|=%d "
                            , obj_name
                            , GET_CARD(cl, sb, offset));

                }
                base += GET_NUM_SS(cl, sb);
            }
        }
    }
    fprintf(srg, "\n");
}

void ESM_REACHED(FILE *fp, Tree_p ESM, int EVENT) {
    int i, k, l, s;

    int ARRAY[MAX_CARD];
    for (i = 0; i < MAX_CARD; i++) ARRAY[i] = 0;

    fprintf(fp, "%s%d", "ESM", ESM->marking->cont_tang);

    if (EVENT) {
        fprintf(fp, "%s", ":");
        for (i = 0; i < ncl; i++) {
            if (ASYM_STATIC_STORE[i].sbc_num == 1) {
                for (l = 0; l < SYM_TOT[i]; l++) {
                    fprintf(fp, "%s_%d%s", tabc[i].col_name, l, "[");
                    fprintf(fp, "%s%s_%d%s%d%s", "(", ASYM_STATIC_STORE[i].ptr[0].name, sfl_h->min[i][l], ":",
                            RESULT[i][0][l][0] , ")");
                }
            }
            else {
                for (s = 0; s < MAX_CARD; s++) ARRAY[s] = 0;
                for (l = 0; l < SYM_TOT[i]; l++) {
                    fprintf(fp, "%s_%d%s", tabc[i].col_name, l, "[");
                    for (k = 0; k < ASYM_STATIC_STORE[i].sbc_num; k++)
                        if (RESULT[i][0][l][k] > 0)

                        {
                            fprintf(fp, "%s%s_%d%s%d%s", "(", ASYM_STATIC_STORE[i].ptr[k].name, ARRAY[k], ":", RESULT[i][0][l][k], ")");
                            ARRAY[k] += 1;
                        }

                    fprintf(fp, "%s", "]");
                }
            }
        }
    }

    fprintf(fp, "\n");
    WRITE_ON_ESRG(fp);
    fprintf(fp, "\n");
}

#ifndef LIBSPOT

int  EXCEPTION()

{
    int i ;

    for (i = 0; i < ncl; i++)
        if ((tabc[i].type == ORDERED) && (tabc[i].sbc_num != 1)) {
            printf("\n Sorry, ordred classes with more than one static sub-class are not allowed : Class (%s) \n", tabc[i].col_name);
            exit(0);
        }
    return 0;
}
#else
int  EXCEPTION()

{
    int i ;

    for (i = 0; i < ncl; i++)
        if ((tabc[i].type == ORDERED)) {
            printf("\n Sorry, ordred classes are not allowed yet  : Class (%s) \n", tabc[i].col_name);
            exit(0);
        }
    return 0;
}
#endif
