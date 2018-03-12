#include <stdio.h>
#include <string.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/service.h"

#ifdef SIMULATION
extern Trans_p enab_list;
extern double cur_time;

extern Event_p locate_in_list();
extern int get_static_subclass();
extern int get_obj_off();



extern Event_p event_list_h;
extern Event_p event_list_t;
#ifdef DEBUG_simulation
extern Event_p *event_free;
extern int *em;
#endif
#endif
extern char bname[MAX_TAG_SIZE];
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_object_name(int  cl,  int  obj_num,  char  name[MAX_TAG_SIZE]) {
    /* Init get_object_name */
    int j, sb, lim, index;
    struct OBJ *opt = NULL;

    sb = get_static_subclass(cl, obj_num);
    lim = get_obj_off(sb, cl);
    index = obj_num - lim;
    if ((opt = tabc[cl].sbclist[sb].obj_list) == NULL)
        sprintf(name, "%s%d", tabc[cl].sbclist[sb].obj_name, tabc[cl].sbclist[sb].low + index);
    else {
        for (j = 0; j < index; opt = opt->next, j++);
        sprintf(name, "%s", opt->nome);
    }
}/* End get_object_name */
#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void get_dynamic_subclass_name(int  cl,  int  dyn_num,  char  name[MAX_TAG_SIZE]) {
    /* Init get_dynamic_subclass_name */
    int sb;

    sb = get_static_subclass(cl, dyn_num);
    sprintf(name, "%s%d", GET_STATIC_NAME(cl, sb), dyn_num);
}/* End get_dynamic_subclass_name */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_dynamic_subclass_data_structure(FILE  *fp) {
    /* Init out_dynamic_subclass_data_structure */
    char name[MAX_TAG_SIZE];
    int i, j, h;

    for (i = 0 ; i < ncl; i++) {
        /* Per ogni classe */
        if (IS_UNORDERED(i)) {
            /* Classe non ordinata */
            fprintf(fp, "Color class %s : number of dynamic subclass %d\n", GET_CLASS_NAME(i), GET_NUM_CL(i));
            for (j = 0; j < GET_STATIC_SUBCLASS(i) ; j++) {
                /* Per ogni sottoclasse statica */
                fprintf(fp, "Static subclass %s : number of dynamic subclass %d\n", GET_STATIC_NAME(i, j), GET_NUM_SS(i, j));
                fprintf(fp, "---- Cardinalities :\n");
                for (h = 0; h < GET_NUM_SS(i, j); h++) {
                    get_dynamic_subclass_name(i, GET_STATIC_OFFSET(i, j) + h, name);
                    fprintf(fp, "%s ----> CARD %d\n", name, GET_CARD_BY_TOTAL(i, j, h));
                }
            }/* Per ogni sottoclasse statica */
            fprintf(fp, "-------------------------------------------\n");
        }/* Classe non ordinata */
    }/* Per ogni classe */
}/* End out_dynamic_subclass_data_structure */
#endif
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_single_instance(Event_p  ev_p,  FILE  *fp) {
    /* Init out_single_instance */
    int tr = GET_TRANSITION_INDEX(ev_p);
    int ii;
    int cl;
    char name[MAX_TAG_SIZE];
    name[0] = '\0';
#ifdef SWN
#ifdef SYMBOLIC
    fprintf(fp, "ORDINARY INSTANCES : %d - ", ev_p->ordinary_instances);
#endif
    if (GET_TRANSITION_COMPONENTS(tr) > 0) {
        /* Transizione colorata */
        cl = tabt[tr].dominio[0];
        if (ev_p->npla[0] != UNKNOWN) {
#ifdef COLOURED
            get_object_name(cl, ev_p->npla[0], name);
#endif
#ifdef SYMBOLIC
#ifdef REACHABILITY
            if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                get_dynamic_subclass_name(cl, ev_p->npla[0], name);
            else
                get_object_name(cl, ev_p->npla[0], name);
#endif
#ifdef SIMULATION
            if (IS_UNORDERED(cl))
                get_dynamic_subclass_name(cl, ev_p->npla[0], name);
            else
                get_object_name(cl, ev_p->npla[0], name);
#endif
#endif
        }
        else
            strcpy(name, "Unk");
        if (strcmp(tabt[tr].names[0], "?"))
            fprintf(fp, "BINDING  : %s <-- %s", tabt[tr].names[0], name);
        else
            fprintf(fp, "BINDING  : (implicit projection) <-- %s", name);
//DOT
        if (name[0] != '\0')
            sprintf(bname, "<%s", name);
        else
            bname[0] = '\0';
//DOT
        for (ii = 1; ii < tabt[tr].comp_num; ii++) {
            cl = tabt[tr].dominio[ii];
            if (ev_p->npla[ii] != UNKNOWN) {
#ifdef COLOURED
                get_object_name(cl, ev_p->npla[ii], name);
#endif
#ifdef SYMBOLIC
#ifdef REACHABILITY
                if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                    get_dynamic_subclass_name(cl, ev_p->npla[ii], name);
                else
                    get_object_name(cl, ev_p->npla[ii], name);
#endif
#ifdef SIMULATION
                if (IS_UNORDERED(cl))
                    get_dynamic_subclass_name(cl, ev_p->npla[ii], name);
                else
                    get_object_name(cl, ev_p->npla[ii], name);
#endif
#endif
            }
            else
                strcpy(name, "Unk");
            if (strcmp(tabt[tr].names[ii], "?"))
                fprintf(fp, ", %s <-- %s ", tabt[tr].names[ii], name);
            else
                fprintf(fp, ", (implicit projection) <-- %s ", name);
//DOT
            if (name[0] != '\0')
                sprintf(bname, "%s,%s", bname, name);
            else
                bname[0] = '\0';
//DOT
        }
        if (name[0] != '\0')
            sprintf(bname, "%s>", bname);
    }/* Transizione colorata */
    else {
        fprintf(fp, "NO BINDING (non-colored)");
        bname[0] = '\0';
    }
//DOT


#endif
    fprintf(fp, "\n");
}/* End out_single_instance */
#ifdef SIMULATION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_single_instance_detailed(Event_p  ev_p,  FILE  *fp) {
    /* Init out_single_instance_detailed */

    fprintf(fp, "----------------- FIRED -------------------------\n");

#ifdef SIMULATION
#ifndef SYMBOLIC
#ifdef DEBUG_simulation
    fprintf(fp, "Transition %s Degree %d Id=%d Creation time %1.6lg last_sched_desched %1.6lg residual_sched_time %1.6lg\n", tabt[ev_p->trans].trans_name,
            ev_p->enabling_degree,
            ev_p->tag,
            ev_p->creation_time, ev_p->last_sched_desched_time,
            ev_p->residual_sched_time);
#endif
#ifndef DEBUG_simulation
    fprintf(fp, "Transition %s Degree %d Creation time %1.6lg last_sched_desched %1.6lg residual_sched_time %1.6lg\n", tabt[ev_p->trans].trans_name,
            ev_p->enabling_degree,
            ev_p->creation_time, ev_p->last_sched_desched_time,
            ev_p->residual_sched_time);
#endif
#endif
#endif
#ifdef SWN
#ifdef SYMBOLIC
    fprintf(fp, "Transition %s Degree %d Ordinary %d Creation time %1.6lg last_sched_desched %1.6lg residual_sched_time %1.6lg\n", tabt[ev_p->trans].trans_name,
            ev_p->enabling_degree,
            ev_p->ordinary_instances,
            ev_p->creation_time, ev_p->last_sched_desched_time,
            ev_p->residual_sched_time);
#endif
    out_single_instance(ev_p, fp);
#endif
    fprintf(fp, "----------------- FIRED -------------------------\n");
}/* End out_single_instance_detailed */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_instances(FILE  *fp) {
    /* Init out_instances */
    Trans_p t_p;
    Event_p ev_p = NULL;
    int nt = 0;
#ifdef DEBUG_simulation
    Event_p tev_p = NULL;
    int scnt, ecnt;
    int mcnt;
#endif

    for (t_p = tabt ; nt < ntr; nt++, t_p++) {
        /* Per ogni transizione */
        if (t_p->instances_h != NULL) {
            /* La transizione e' abilitata */
            fprintf(fp, "TRANSIZIONE %s \n", t_p->trans_name);
#ifdef SIMULATION
#ifndef SYMBOLIC
#ifdef DEBUG_simulation
            if (!IS_IMMEDIATE(nt) && !IS_EXPONENTIAL(nt)) {
                /* Transizione con distribuzione generale */
                mcnt = 0;
                fprintf(fp, "ID in FREE LIST: ");
                for (ev_p = event_free[nt] ; ev_p != NULL ; ev_p = ev_p->next) {
                    /* Per ogni istanza in free */
                    fprintf(fp, "%d ", ev_p->tag);
                    mcnt++;
                }/* Per ogni istanza in free */
                fprintf(fp, "\n");
                fprintf(fp, "ID in EL LIST: ");
                for (ev_p = t_p->instances_h ; ev_p != NULL ; ev_p = ev_p->t_next) {
                    /* Per ogni istanza in EL */
                    fprintf(fp, "[%d] ", ev_p->tag);
                    mcnt++;
                    for (tev_p = (ev_p->e_next) ; tev_p != NULL; mcnt++, tev_p = tev_p->t_next)
                        fprintf(fp, "%d ", tev_p->tag);
                }/* Per ogni istanza in EL */
                fprintf(fp, "\n");
                fprintf(fp, "ID in STORED LIST: ");
                for (ev_p = t_p->d_instances_h ; ev_p != NULL ; ev_p = ev_p->t_next) {
                    /* Per ogni istanza in stored */
                    fprintf(fp, "[%d] ", ev_p->tag);
                    mcnt++;
                    for (tev_p = ev_p->e_next; tev_p != NULL; mcnt++, tev_p = tev_p->t_next)
                        fprintf(fp, "%d ", tev_p->tag);
                }/* Per ogni istanza in stored */
                fprintf(fp, "\n");
                if (em[nt] != mcnt)
                    fprintf(fp, "Lost Data Structure: Actually malloc'd %d, counted %d\n", em[nt], mcnt);
                else
                    fprintf(fp, "Actually malloc'd %d, counted %d\n", em[nt], mcnt);
            }/* Transizione con distribuzione generale */
#endif
#endif
#endif
            for (ev_p = t_p->instances_h ; ev_p != NULL ; ev_p = ev_p->t_next) {
                /* Per ogni istanza */
#ifdef SWN
                out_single_instance(ev_p, fp);
#endif
#ifdef SIMULATION
#ifndef SYMBOLIC
#ifdef DEBUG_simulation
                if (!IS_IMMEDIATE(nt) && !IS_EXPONENTIAL(nt)) {
                    /* Transizione con distribuzione generale */
                    ecnt = scnt = 0;
                    if (ev_p->e_next != NULL)
                        for (tev_p = (ev_p->e_next) ; tev_p != NULL; ecnt++, tev_p = tev_p->t_next);
                    tev_p = locate_in_list(ev_p, REUSE);
                    if (tev_p != NULL)
                        for (tev_p = tev_p->e_next; tev_p != NULL; scnt++, tev_p = tev_p->t_next);
                    fprintf(fp, "In EL=%d, stored times %d\n", ecnt, scnt);
                    if (ecnt != ev_p->enabling_degree)
                        fprintf(fp, "EL mismatch: counted %d enabling degree %d\n", ecnt, ev_p->enabling_degree);
                    tev_p = locate_in_list(ev_p, REUSE);
                    if (tev_p != NULL) {
                        if (scnt != tev_p->enabling_degree)
                            fprintf(fp, "STORED mismatch: counted %d enabling degree %d\n", scnt, tev_p->enabling_degree);
                        if (scnt == 0)
                            fprintf(fp, "DANGEROUS STORED mismatch: counted %d enabling degree %d\n", scnt, tev_p->enabling_degree);
                    }
                    else {
                        if (scnt != 0)
                            fprintf(fp, "STORED ZERO mismatch: counted %d enabling degree 0\n", scnt);
                    }
                }/* Transizione con distribuzione generale */
#endif
#endif
#endif
            }/* Per ogni istanza */
        }/* La transizione e' abilitata */
    }/* Per ogni transizione */
}/* End out_instances */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_enab_list(FILE  *fp) {
    /* Init out_enab_list */
    Trans_p t_p = NULL;

    fprintf(fp, "LISTA TRANSIZIONI ABILITATE .......\n");
    for (t_p = enab_list; t_p != NULL; t_p = t_p->enabl_next)
        fprintf(fp, "EN : %s\n", t_p->trans_name);
    fprintf(fp, "FINE LISTA \n");
}/* End out_enab_list */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_event_list(FILE  *fp) {
    /* Init out_event_list */
    Event_p ev_p = NULL;

    fprintf(fp, "\n**************** Event list ****************\n");
    fprintf(fp, "Tempo corrente : %1.6lg\n", cur_time);
    ev_p = event_list_h;
    while (ev_p != NULL) {
        /* Per ogni evento della event list */
        fprintf(fp, "--------------------------------------------------\n");
#ifdef SIMULATION
#ifndef SYMBOLIC
#ifdef DEBUG_simulation
        fprintf(fp, "Transition %s Degree %d Id=%d Creation time %1.6lg last_sched_desched %1.6lg residual_sched_time %1.6lg\n", tabt[ev_p->trans].trans_name,
                ev_p->enabling_degree,
                ev_p->tag,
                ev_p->creation_time, ev_p->last_sched_desched_time,
                ev_p->residual_sched_time);
#endif
#ifndef DEBUG_simulation
        fprintf(fp, "Transition %s Degree %d Creation time %1.6lg last_sched_desched %1.6lg residual_sched_time %1.6lg\n", tabt[ev_p->trans].trans_name,
                ev_p->enabling_degree,
                ev_p->creation_time, ev_p->last_sched_desched_time,
                ev_p->residual_sched_time);
#endif
#endif
#endif
#ifdef SYMBOLIC
        fprintf(fp, "Transition %s Degree %d Ordinary %d Creation time %1.6lg last_sched_desched %1.6lg residual_sched_time %1.6lg\n", tabt[ev_p->trans].trans_name,
                ev_p->enabling_degree,
                ev_p->ordinary_instances,
                ev_p->creation_time, ev_p->last_sched_desched_time,
                ev_p->residual_sched_time);
#endif
#ifdef SWN
        out_single_instance(ev_p, fp);
#endif
        ev_p = ev_p->e_next;
    }/* Per ogni evento della event list */
    fprintf(fp, "*************** Fine ***************\n");
}/* End out_event_list */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_cur_marking(FILE   *fp) {
    /* Init out_cur_marking */
    int k, i, ii, cl;
    char name[MAX_TAG_SIZE];
#ifdef SWN
    Token_p token_ptr = NULL;
#endif

    fprintf(fp, "****************** Marking attuale **************\n");
#ifdef SWN
#ifdef SYMBOLIC
    out_dynamic_subclass_data_structure(fp);
#endif
#endif
    for (i = 0; i < npl; i++) {
        /* Per ogni posto */
        if (IS_FULL(i)) {
            /* Contiene almeno un token */
#ifdef GSPN
            fprintf(fp, "M(%s) = %d \n", tabp[i].place_name, net_mark[i].total);
#endif
#ifdef SWN
#ifdef SYMBOLIC
            fprintf(fp, "Place %s -----> TOTAL %d DIFFERENT %d ORDINARY %d\n",
                    tabp[i].place_name, net_mark[i].total, net_mark[i].different, net_mark[i].ordinary);
#endif
#ifdef COLOURED
            fprintf(fp, "Place %s -----> TOTAL %d DIFFERENT %d\n",
                    tabp[i].place_name, net_mark[i].total, net_mark[i].different);
#endif
            if (!IS_NEUTRAL(i)) {
                /* Posto con dominio di colore */
                fprintf(fp, "%s ", tabc[tabp[i].dominio[0]].col_name);
                for (ii = 1; ii < tabp[i].comp_num; ii++)
                    fprintf(fp, "%s ", tabc[tabp[i].dominio[ii]].col_name);
                fprintf(fp, "\n");

                for (token_ptr = net_mark[i].marking ; token_ptr != NULL; token_ptr = token_ptr->next) {
                    /* Per ogni token della marcatura del posto */
                    cl = tabp[i].dominio[0];
#ifdef COLOURED
                    get_object_name(cl, token_ptr->id[0], name);
#endif
#ifdef SYMBOLIC
                    get_dynamic_subclass_name(cl, token_ptr->id[0], name);
#endif
                    fprintf(fp, "M(<%s", name);
                    for (k = 1; k < tabp[i].comp_num; k++) {
                        cl = tabp[i].dominio[k];
#ifdef COLOURED
                        get_object_name(cl, token_ptr->id[k], name);
#endif
#ifdef SYMBOLIC
                        get_dynamic_subclass_name(cl, token_ptr->id[k], name);
#endif
                        fprintf(fp, ",%s", name);
                    }
                    fprintf(fp, ">)=%d\n", token_ptr->molt);
                }/* Per ogni token della marcatura del posto */
            }/* Posto con dominio di colore */
#endif
        }/* Contiene almeno un token */
    }/* Per ogni posto */
    fprintf(fp, "****************** Fine marking ******************\n");
}/* End out_cur_marking */
#ifdef SIMULATION
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void out_cur_simulation_state(FILE   *fp) {
    fprintf(fp, "Tempo corrente : %1.6lg\n", cur_time);
    fprintf(fp, "\n");
    out_cur_marking(fp);
    fprintf(fp, "\n");
    out_enab_list(fp);
    fprintf(fp, "\n");
    out_instances(fp);
    fprintf(fp, "\n");
    out_event_list(fp);
    fprintf(fp, "\n");
}
#endif
