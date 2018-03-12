#include <stdio.h>

#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"

extern void update_en_list();
extern void initialize_en_list();
extern void string_to_marking();
extern void dispose_old_event();
extern void add_pop();
extern Tree_p treenode_pop();
extern void push_result();
extern Result_p enabled_head;

extern int     COMPARE_EVENTUALITIES();
extern void    EVENTUALITIES();
extern int     ALL_NB_OF_EVENTUALITIES();
extern void    EVENTUALITIE_TO_STRING();
extern void    ADD_EVENTUALITIE_TO_FILE();
extern void    STORE_STATIC_CONF();
extern void    POP_STATIC_CONF();
extern void    INIT_4D_MATRIX();
extern Tree_Esrg_p TREE_EVENT_ALLOCATION();
extern void    TREE_EVENT_FREE();
extern void    Write_The_ESRG();
extern void    MARQUAGE();
//extern Tree_Esrg_p MY_TREE_POP           ();

extern int cur_priority;
extern int out_mc;
static Tree_p                    REACHED_MARKING = NULL;
static Tree_p                    SOURCE_MARKING = NULL;
static Tree_Esrg_p              *CURRENT_PTR = NULL;
int compt_event;

Tree_Esrg_p MY_TREE_POP(int buff, int length) {
    Tree_Esrg_p ptr = NULL;

    ptr = (Tree_Esrg_p)TREE_EVENT_ALLOCATION(buff, length);
    ptr->Head_Next_Event = NULL;
    ptr->enabled_head    = NULL;
    ptr->Marking_Type    = -1;
    ptr->input_tr = NULL;
    ptr->asym_reach = 0;
    return (ptr);
}

void FREE_TREE(Tree_Esrg_p *Head, Tree_Esrg_p To_free, Tree_Esrg_p Prec) {
    if ((*Head) == To_free)(*Head) = To_free->Head_Next_Event;
    else if (To_free->Head_Next_Event == NULL) Prec->Head_Next_Event = NULL;
    else Prec->Head_Next_Event = To_free->Head_Next_Event;

    TREE_EVENT_FREE(To_free);
}

void FREE_EVENTUALITIES(Tree_Esrg_p Head) {
    Tree_Esrg_p cur = Head, next;
    while (cur) {
        next = cur->Head_Next_Event;
        TREE_EVENT_FREE(cur);
        cur = next;
    }
}

Tree_Esrg_p TEST_EXISTENCE(Tree_Esrg_p List_Events, Tree_Esrg_p *prec) {
    (*prec) = List_Events;

    while (List_Events) {
        if (COMPARE_EVENTUALITIES(List_Events->marking->marking_as_string,
                                  List_Events->marking->length) == EQUAL)
            return List_Events;

        (*prec) = List_Events;
        List_Events = List_Events->Head_Next_Event;
    }

    return NULL;
}

void INIT_ARRAYS(TO_MERGEP *merg,
                 int **num,
                 int *** card,
                 STORE_STATICS_ARRAY STORED_CONF) {
    int i, j;
    TO_MERGEP ptr = NULL;

    for (i = 0; i < ncl; i++) {
        ptr = merg[i];
        while (ptr) {
            for (j = 0; j < ptr->NB; j++) STATICS[i][ptr->sbc_num][j] =
                    STORED_CONF[i].ptr[ptr->SSCs[j]].card;
            for (j = 0; j < num[i][ptr->sbc_num]; j++) DYNAMIC[i][ptr->sbc_num][j] =
                    card[i][ptr->sbc_num][j];
            NB_DSC[i][ptr->sbc_num] =
                num[i][ptr->sbc_num];
            NB_SSBC[i][ptr->sbc_num] =
                ptr->NB;
            ptr = ptr->next;
        }
    }
}

void TRAITEMENT_1() {
    Tree_Esrg_p event = NULL;
    enabled_head = NULL;
    int i, j;

    compt_event++;
    NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                     MERG, ASYM_STATIC_STORE, RESULT);
    my_initialize_en_list();

    if (enabled_head) {
        EVENTUALITIE_TO_STRING();
        ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
        event = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
        event->enabled_head = enabled_head;
        event->Marking_Type = SATURED_INS;
        event->marking->pri = cur_priority;
        event->marking->cont_tang = COMPT_STORED_EVENT;
        (*CURRENT_PTR) = event;
        CURRENT_PTR = &(event->Head_Next_Event);

        if (out_mc)
            SR_EVENT_POS_STORE(OFFSET_EVENT, REACHED_MARKING, event);
    }

    /**************** Re-load the symetric config for the next computation **************/
    POP_STATIC_CONF(SYM_STATIC_STORE, &tabc);
    string_to_marking(REACHED_MARKING->marking->marking_as_string,
                      REACHED_MARKING->marking->d_ptr,
                      REACHED_MARKING->marking->length);
    /***********************************************************************************/
    COMPT_EVENT++;
}

int TEST_SYMETRIC_INITIAL(TO_MERGEP *merg) {
    int nb;
    INIT_ARRAYS(merg, num, card, ASYM_STATIC_STORE);
    nb = ALL_NB_OF_EVENTUALITIES();
    return nb;
}


int SYM_SAT_2_NOT_EXIST(Tree_p Reached_Marking,
                        Tree_p Source_Marking,
                        Event_p ev_p, TO_MERGEP *merg) {
    Tree_Esrg_p event = NULL;
    Result_p enabling = Source_Marking->enabled_head;
    int i;
    compt_event = 0;
    /********* for the symetric Marking ***************/
    MARKING_TYPE = SATURED_SYM;
    enabled_head = NULL;
    my_initialize_en_list();
    Reached_Marking->enabled_head = enabled_head;
    Reached_Marking->Marking_Type = SATURED_SYM;
    Reached_Marking->marking->pri = cur_priority;
    /**************************************************/

    /********* for eventualities *****************/
    MARKING_TYPE = SATURED_INS;
    Reached_Marking->Head_Next_Event = NULL;
    CURRENT_PTR = &(Reached_Marking->Head_Next_Event);
    REACHED_MARKING = Reached_Marking;
    INIT_4D_MATRIX(RESULT);
    INIT_ARRAYS(merg, num, card, ASYM_STATIC_STORE);
    EVENTUALITIES(0, 0, 1, 1, TRAITEMENT_1);
    /*************************************************/
    Reached_Marking->Num_Event = compt_event;
    Reached_Marking->NOT_Treated = Reached_Marking->Head_Next_Event;
    COMPT_SAT++;

    return INSERT_SAT;

}

void TRAITEMENT_2() {
    Tree_Esrg_p event, exist, prec = NULL;
    Tree_Esrg_p last = REACHED_MARKING->Head_Next_Event;
    int i, j;

    EVENTUALITIE_TO_STRING();

    if ((event = TEST_EXISTENCE(REACHED_MARKING->Head_Next_Event, &prec)) == NULL) {
        enabled_head = NULL;
        NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                         MERG, ASYM_STATIC_STORE, RESULT);

        my_initialize_en_list();

        if (enabled_head) {
            EVENTUALITIE_TO_STRING();
            ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
            event = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
            event->enabled_head = enabled_head;
            event->Marking_Type = MARKING_TYPE;
            event->marking->pri = cur_priority;
            event->marking->cont_tang = COMPT_STORED_EVENT;
            prec->Head_Next_Event = event;

            if (out_mc)
                SR_EVENT_POS_STORE(OFFSET_EVENT, REACHED_MARKING, event);
        }

        /******* Re-load the symetric config for the next computation *****/
        POP_STATIC_CONF(SYM_STATIC_STORE, &tabc);
        string_to_marking(REACHED_MARKING->marking->marking_as_string,
                          REACHED_MARKING->marking->d_ptr,
                          REACHED_MARKING->marking->length);
        /*****************************************************************/
        COMPT_EVENT++;
    }
    else {
        event->Marking_Type = MARKING_TYPE;
    }

}

int SYM_SAT_2_EXIST(Tree_p Reached_Marking, Tree_p Source_Marking,
                    Event_p ev_p, TO_MERGEP *merg) {
    Tree_Esrg_p Head, event, prec, exist, The_Queue;
    int TO_STACK = NO_THINK;
    int event_num = 0;

    if (Reached_Marking->Marking_Type == NO_SATURED_SYM) {
        /****************** When the reached marking is not saturated**********/

        /******* Compute the enable instances of the symetric transitions *******/
        Reached_Marking->Marking_Type = MARKING_TYPE = SATURED_SYM;
        enabled_head = NULL; cur_priority = UNKNOWN;
        if (Reached_Marking->NOT_Treated)
            my_initialize_en_list();
        Reached_Marking->enabled_head = enabled_head;
        Reached_Marking->marking->pri = cur_priority;
        /***********************************************************************/

        Tree_Esrg_p last = Reached_Marking->Head_Next_Event;
        while (last->Head_Next_Event) last = last->Head_Next_Event;

        REACHED_MARKING = Reached_Marking;
        MARKING_TYPE = SATURED_INS;
        INIT_4D_MATRIX(RESULT);
        INIT_ARRAYS(merg, num, card, ASYM_STATIC_STORE);
        EVENTUALITIES(0, 0, 1, 1, TRAITEMENT_2);

        /****** If the ESM was already treated: it will be add in the stack *****/
        if (Reached_Marking->NOT_Treated == NULL) {
            Reached_Marking->NOT_Treated = last->Head_Next_Event;
            TO_STACK = INSERT_SAT;
        }
        else  TO_STACK = DEPLACE;
        /***********************************************************************/

        COMPT_SAT++;
    }
    return TO_STACK;
}


int ASYM_2_EXIST_SYM(Tree_p Reached_Marking,
                     Tree_p Source_Marking,
                     Tree_Esrg_p Eventual,
                     Event_p ev_p, TO_MERGEP *merg) {

    Tree_Esrg_p event, New_event = NULL, The_Queue, Output = NULL, prec;
    int i, j, TO_STACK = NO_THINK;

    if (Reached_Marking->Marking_Type == SATURED_SYM) {
    }
    else {
        INIT_ARRAYS(merg, num, card, ASYM_STATIC_STORE);
        EVENTUALITIE_TO_STRING();

        if ((Output = TEST_EXISTENCE(Reached_Marking->Head_Next_Event, &prec)) == NULL) {

            COMPT_EVENT++;
            MARKING_TYPE = NO_SATURED_INS;

            /** the symetric SM, becomes satured **/
            if (Reached_Marking->Num_Event == 1) {

                COMPT_SAT++;
                /************ The marking becomes satured ***********************/

                /************ Computation of the symetrical instances for the symetric marking *****/
                Reached_Marking->Marking_Type = MARKING_TYPE = SATURED_SYM;
                Reached_Marking->Num_Event = 0;
                enabled_head = NULL; cur_priority = UNKNOWN;
                if (Reached_Marking->NOT_Treated)
                    my_initialize_en_list();
                Reached_Marking->enabled_head = enabled_head;
                Reached_Marking->marking->pri = cur_priority;
                MARKING_TYPE = SATURED_INS;
                /**********************************************************************************/
                event = Reached_Marking->Head_Next_Event;

                while (event) {
                    event->Marking_Type = SATURED_INS;
                    event = event->Head_Next_Event;
                }

                if (Reached_Marking->NOT_Treated)  TO_STACK = DEPLACE;
                else TO_STACK = INSERT_SAT;
            }
            else {
                if (Reached_Marking->NOT_Treated == NULL) TO_STACK = INSERT_NO_SAT;
                Reached_Marking->Num_Event--;

            }

            /**************** Compute the new instances **************************************/

            enabled_head = NULL;
            NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot, merg, ASYM_STATIC_STORE, RESULT);

            my_initialize_en_list();

            if ((MARKING_TYPE != SATURED_INS) || ((MARKING_TYPE == SATURED_INS) && (enabled_head))) {

                ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
                New_event = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
                New_event->enabled_head = enabled_head;
                New_event->marking->cont_tang = COMPT_STORED_EVENT;
                Output = New_event;
                New_event->Marking_Type = MARKING_TYPE;
                New_event->marking->pri = cur_priority;
                New_event->asym_reach = 1;
                event = Reached_Marking->Head_Next_Event;
                while (event->Head_Next_Event)
                    event = event->Head_Next_Event;
                event->Head_Next_Event = New_event;

                /******************* If the marking is archeaved it must inqueued in the stack ***/
                if (Reached_Marking->NOT_Treated == NULL) Reached_Marking->NOT_Treated = New_event;
                /*********************************************************************************/

                if (out_mc)
                    SR_EVENT_POS_STORE(OFFSET_EVENT, Reached_Marking, New_event);
            }
        }
    }

    if (out_mc)
        if (Output == NULL)
            WRITE_REACHED_ESM(OUTPUT_FILE, Reached_Marking, NULL);
        else
            WRITE_REACHED_ESM(OUTPUT_FILE, Reached_Marking, Output);

    if (output_flag)
        if (Output == NULL)
            Print_Ins_Arc(OUTPUT_FILE, ev_p->trans, Eventual->marking->cont_tang,
                          Reached_Marking->marking->cont_tang, 0) ;
        else {

            Print_Ins_Arc(OUTPUT_FILE, ev_p->trans, Eventual->marking->cont_tang,
                          Output->marking->cont_tang, 1) ;
        }
    return TO_STACK;
}


int ASYM_2_NOT_EXIST_SYM(Tree_p Reached_Marking,
                         Tree_p Source_Marking,
                         Tree_Esrg_p Eventual,
                         Event_p ev_p, int Index,
                         TO_MERGEP *merg) {
    Tree_Esrg_p event, exist, The_Queue, Output;
    int i, j, TO_STACK = INSERT_NO_SAT;
    Result_p ptr;

    COMPT_EVENT++;
    INIT_ARRAYS(merg, num, card, ASYM_STATIC_STORE);
    EVENTUALITIE_TO_STRING();
    ADD_EVENTUALITIE_TO_FILE(LEGTH_OF_CACHE);
    event = MY_TREE_POP(FILE_POS_PTR, LEGTH_OF_CACHE);
    Reached_Marking->Head_Next_Event = event;
    Reached_Marking->Num_Event = ALL_NB_OF_EVENTUALITIES();

    GET_EVENTUALITIE_FROM_FILE(event->marking->marking_as_string,
                               event->marking->length);
    COPY_CACHE(event->marking->length);
    STRING_TO_EVENTUALITIE();

    if (Reached_Marking->Num_Event == 1) {
        Reached_Marking->Marking_Type = MARKING_TYPE = SATURED_SYM;
        Reached_Marking->Num_Event = 0;
        enabled_head = NULL;
        my_initialize_en_list();
        Reached_Marking->enabled_head = enabled_head;
        Reached_Marking->marking->pri = cur_priority;
        event->Marking_Type = MARKING_TYPE = SATURED_INS;
        TO_STACK = INSERT_SAT;
        COMPT_SAT++;
    }
    else {
        Reached_Marking->Marking_Type = NO_SATURED_SYM;
        event->Marking_Type = MARKING_TYPE = NO_SATURED_INS;
    }

    enabled_head = NULL;
    NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num,
                     &tot, merg, ASYM_STATIC_STORE, RESULT);

    my_initialize_en_list();
    event->marking->pri = cur_priority;
    event->enabled_head = enabled_head;
    event->marking->cont_tang = COMPT_STORED_EVENT;
    event->asym_reach = 1;
    Reached_Marking->NOT_Treated =
        Reached_Marking->Head_Next_Event = event;
    Reached_Marking->Num_Event--;

    if (out_mc) {
        SR_EVENT_POS_STORE(OFFSET_EVENT, Reached_Marking, event);
        WRITE_REACHED_ESM(OUTPUT_FILE, Reached_Marking, event);
    }

    if (output_flag) {
        Print_Ins_Arc(OUTPUT_FILE, ev_p->trans, Eventual->marking->cont_tang,
                      event->marking->cont_tang, 1) ;
    }
    return TO_STACK;
}




