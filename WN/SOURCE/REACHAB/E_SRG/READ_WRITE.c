
#include <stdio.h>
#include <stdlib.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"
#include "../../../INCLUDE/service.h"


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

#ifndef LIBSPOT


void SR_EVENT_POS_STORE(FILE *fp, Tree_p ESM, Tree_Esrg_p event) {
    LP_ESRG = CACHE_STRING_ESRG;
    fseek(fp, 0, SEEK_END);

    if (!event) {
        MY_CHAR_STORE((unsigned long)ESM->marking->marking_as_string);
        MY_CHAR_STORE((unsigned long)ESM->marking->length);
        MY_CHAR_STORE((unsigned long)ESM->marking->d_ptr);
    }
    else {
        MY_CHAR_STORE((unsigned long)ESM->marking->cont_tang);
        MY_CHAR_STORE((unsigned long)event->marking->marking_as_string);
        MY_CHAR_STORE((unsigned long)event->marking->length);
    }

    fwrite(CACHE_STRING_ESRG, 1, LP_ESRG - CACHE_STRING_ESRG, fp);
}



/* int Number_of_Instances(Result_p enb_h ,Result_p enb_h_store, int pri, int type ) */
/* { */
/*   Result_p enb=enb_h; */
/*   Event_p ptr=NULL; */
/*   int nb=0; */
/*   int tr; */
/*   while (enb) */
/*     { */
/*       tr= GET_TRANSITION_INDEX(enb->list); */
/*       switch (type) */
/* 	{ */
/* 	case 1 :  */
/* 	  if(tabt[tr].pri == pri) */
/* 	    { */
/* 	      ptr= enb->list; */
/* 	      while(ptr) */
/* 		{ */
/* 		  nb+=1; */
/* 		  ptr=ptr->next; */
/* 		} */
/* 	    } */
/* 	break; */

/* 	case 2: */

/* 	  if( ((tabt[tr].pri==pri) && (IS_ASYMETRIC(tr)))|| */
/* 	      ((tabt[tr].pri==pri) && (IS_SYMETRIC(tr)) && !enb_h_store ) */
/* 	     ) */
/* 	    { */
/* 	      ptr= enb->list; */
/* 	      while(ptr) */
/* 		{ */
/* 		  nb+=1; */
/* 		  ptr=ptr->next; */
/* 		} */
/* 	    } */
/* 	  break; */
/* 	}  */
/*       enb=enb->next; */
/*     } */
/*   return nb; */
/* } */

int Number_of_Instances(Result_p enb_h , Result_p enb_h_store, int pri, int greatest, int type) {
    Result_p enb = enb_h;
    Event_p ptr = NULL;
    int nb = 0;
    int tr;
    while (enb) {
        tr = GET_TRANSITION_INDEX(enb->list);
        switch (type) {
        case 1 :
            if (tabt[tr].pri == pri) {
                ptr = enb->list;
                while (ptr) {
                    nb += 1;
                    ptr = ptr->next;
                }
            }
            break;

        case 2:

            if (((tabt[tr].pri == pri) && (IS_ASYMETRIC(tr))) ||
                    ((tabt[tr].pri == pri) && (IS_SYMETRIC(tr)) &&
                     (!enb_h_store || (greatest != pri)))
               ) {
                ptr = enb->list;
                while (ptr) {
                    nb += 1;
                    ptr = ptr->next;
                }
            }
            break;
        }
        enb = enb->next;
    }
    return nb;
}



int Number_of_Event(Tree_Esrg_p event, Result_p enb_h_store, int great) {
    Tree_Esrg_p ptr = event;
    int nb = 0;

    while (ptr) {
        if (Number_of_Instances(ptr->enabled_head , enb_h_store, ptr->marking->pri, great, 2))
            nb += 1;
        ptr = ptr->Head_Next_Event;
    }

    return nb;

}
void WRITE_INSTANCE(FILE *fp, Event_p ev_p)

{
    int tr = GET_TRANSITION_INDEX(ev_p);
    int ii;
    int cl;


    fprintf(fp, "%d ", tr);

    if (GET_TRANSITION_COMPONENTS(tr) > 0) {
        cl = tabt[tr].dominio[0];

        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
            fprintf(fp, "%d %d ", ev_p->npla[0], ev_p->split[0]);

        for (ii = 1; ii < tabt[tr].comp_num; ii++) {

            cl = tabt[tr].dominio[ii];

            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                fprintf(fp, "%d %d ",  ev_p->npla[ii], ev_p->split[ii]);
        }

        //  fprintf(fp,"%d ",ev_p->ordinary_instances);
    }

    fprintf(fp, "%d ", ev_p->ordinary_instances);
}

void WRITE_REACHED_ESM(FILE *fp, Tree_p ESM, Tree_Esrg_p event) {

    fprintf(fp, "%lu ", ESM->marking->cont_tang) ;
    fprintf(fp, "%lu ", ESM->marking->marking_as_string);
    fprintf(fp, "%lu ", ESM->marking->length);
    fprintf(fp, "%lu ", ESM->marking-> d_ptr);

    if (event) {
        fprintf(fp, "%lu ", event->marking->cont_tang);
        fprintf(fp, "%lu ", event->marking->marking_as_string);
        fprintf(fp, "%lu ", event->marking->length);
    }
    else {
        fprintf(fp, "%d ", -1);
        fprintf(fp, "%d ", 0);
        fprintf(fp, "%d ", 0);
    }
    fprintf(fp, "\n");

}

void WRITE_HEAD_EVENT(FILE *fp, Tree_p esm, Tree_Esrg_p event, int NB_INST) {


    fprintf(fp, "%lu ", event->marking->cont_tang);
    fprintf(fp, "%lu ", event->marking->marking_as_string);
    fprintf(fp, "%lu ", event->marking->length);
    fprintf(fp, "%d ", NB_INST);
    fprintf(fp, "\n");
}

void WRITE_HEAD_SR(FILE *fp, Tree_p ESM, Result_p enb_h_store, int great, int NB_INST) {
    int i, k, l, s;


    fprintf(fp, "%lu ", ESM->marking->cont_tang);
    fprintf(fp, "%lu ", ESM->marking->marking_as_string);
    fprintf(fp, "%lu ", ESM->marking->length);
    fprintf(fp, "%lu ", ESM->marking-> d_ptr);

    if (ESM->Marking_Type == SATURED_SYM)
        fprintf(fp, "%d ", SATURED_SYM);
    else
        fprintf(fp, "%d ", NO_SATURED_SYM);

    fprintf(fp, "%d ", NB_INST);

    fprintf(fp, "%d ", Number_of_Event(ESM->NOT_Treated, enb_h_store, great));

    fprintf(fp, "\n");
}

void END_EV(FILE *fp) {
    fprintf(fp, "#\n");
}

#endif

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

                            sprintf(obj_name, "%s(%d)", tabc[cl].col_name, tk_p->id[k]);

                        else
                            get_object_name(cl, tk_p->id[k], obj_name);

                        fprintf(srg, "%s,", obj_name);
                    }
                    cl = tabp[pl].dominio[k];

                    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                        sprintf(obj_name, "%s(%d)", tabc[cl].col_name, tk_p->id[k]);
                    else
                        get_object_name(cl, tk_p->id[k], obj_name);
                    fprintf(srg, "%s>", obj_name);
                }
                fprintf(srg, ")");
            }

        }
    }
    //  fprintf(srg,"\n");

    for (cl = 0 ; cl < ncl; cl++) {
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            for (sb = 0, base = 0; sb < tabc[cl].sbc_num; sb++) {
                for (k = 0; k < GET_NUM_SS(cl, sb); k++) {

                    offset = sfl_h->min[cl][k + base];
                    sprintf(obj_name, "%s(%d)", tabc[cl].col_name, offset);
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

int print_esrg_state(char **st) {

    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");
    WRITE_ON_ESRG(fd);
    pos = ftell(fd);
    *st = malloc((pos + 1) * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);


    return 0;
}

int print_esrg_ev_state(char **st) {

    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");
    write_on_srg(fd, 1);
    pos = ftell(fd);
    *st = malloc((pos + 1) * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);


    return 0;
}

void
Print_Gen_Arc(FILE *fp, int tr, int source, int reach) {


    fprintf(fp, "ESM%d -> ESM%d  [style=bold, label=\"%s\"];\n",
            source, reach, tabt[tr].trans_name);

}

void
PrintESM(FILE *fp, int nu) {
    char *st;
    print_esrg_state(&st);
    fprintf(fp, "ESM%d [shape=box,color=lightgrey,style=filled,fontsize=7, label=\"ESM%d: %s \",group=%d];\n",
            nu, nu, st, nu);
    free(st);
}

void
PrintESMEVENT(FILE *fp, int nuev, int nuesm) {

    fprintf(fp, "ESM%d -> EV%d  [style=dashed,dir=none];\n",
            nuesm, nuev);
}
void
PrintEvent(FILE *fp, int nuev, int nuesm) {
    char *st;
    print_esrg_ev_state(&st);
    fprintf(fp, "EV%d [fontsize=7, label=\"EV%d(%d): %s \",group=%d];\n",
            nuev, nuev, nuesm, st, nuesm);
    free(st);

}
void
Print_Ins_Arc(FILE *fp, int tr, int source, int reach, int flg) {

    if (flg)
        fprintf(fp, "EV%d -> EV%d  [ label=\"%s\"];\n",
                source, reach, tabt[tr].trans_name);
    else
        fprintf(fp, "EV%d -> ESM%d  [ label=\"%s\"];\n",
                source, reach, tabt[tr].trans_name);

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
        if (tabc[i].type == ORDERED) {
            printf("\n Sorry, ordred classes are not allowed yet  : Class (%s) \n", tabc[i].col_name);
            exit(0);
        }
    return 0;
}
#endif
