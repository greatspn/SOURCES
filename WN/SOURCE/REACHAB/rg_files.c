#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"

#ifdef REACHABILITY
extern int *code_place;
extern int *min_place;
extern int *max_place;
extern int *init_place;
extern int max_priority;
extern time_t new_time;
extern time_t old_time;
extern void get_object_name();
#ifdef SWN
#ifdef SYMBOLIC
extern void get_dynamic_subclass_name();
extern Canonic_p sfl_h;
extern int get_dynamic_from_total();
extern int get_static_subclass();
extern int get_obj_off();
#endif
#endif

extern int print_stat_for_gui();

void write_on_srg(FILE *srg, int dir);

//-----------------------------------------------------------------------------

static int is_MCC = -1;

int running_for_MCC() {
    if (is_MCC == -1) { // Not yet determined
        const char *env = getenv("MCC");
#ifdef MCC_BUILD
        is_MCC = (env == NULL || 0 != strcmp(env, "0"));
#else
        is_MCC = (env != NULL && 0 != strcmp(env, "0"));
#endif
    }
    return is_MCC != 0;
}

//-----------------------------------------------------------------------------

/*DOT
*File uses to store the SRG in dot format
*/
extern FILE *f_dot;
/*DOT
*File uses to store the SRG in dot format
*/

/*DOT
*Flag to set the store in the f_dot file
*/
extern int dot_flag;

/*DOT
*Flag to set the store in the f_dot file
*/
extern char bname[MAX_TAG_SIZE];
static int last_tang = 0;


#define INV_RATE   -30000.0

int exceeded_markings_bound();
/**************************************************************/
// This section is need to support the partial RG visualization 
// of v_graph. It remembers which markings have been written.
/**************************************************************/

// Maximum number of marking saved in the DOT file. -1 is unbounded.
static int max_dot_markings = -1;
// max ID saved - These two variables are used when there is an upper
// bound of marking saved in the DOT file.
static int max_tangible_id = 0, max_vanish_id = 0;

void dot_set_max_markings(int m) {
    max_dot_markings = m;
}

int dot_is_marking_saved(int id, char type) {
    if (max_dot_markings <= 0)
        return 1; // maximum is disabled, so every marking is saved.
    if (type == 'V')
        return id <= max_vanish_id;
    else
        return id <= max_tangible_id;
}

int dot_save_marking(int id, char type) {
    if (max_dot_markings <= 0)
        return 1; // maximum is disabled, always save

    if (max_tangible_id + max_vanish_id + 1 == max_dot_markings)
        return 0; // will not be saved

    if (type == 'V') {
        if (!(max_vanish_id + 1 == id)) {
            fprintf(stderr, "Error: unexpected id:  max_vanish_id=%d, id=%d.\n", max_vanish_id, id);
            exit(-1);
        }
        max_vanish_id = id;
    }
    else {
        if (!(max_tangible_id + 1 == id)) {
            fprintf(stderr, "Error: unexpected id:  max_tangible_id=%d, id=%d.\n", max_tangible_id, id);
            exit(-1);
        }
        max_tangible_id = id;
    }
    return 1;
}



/*DOT
*Function
*/

// Static count of the number of tangible/vanishing markings already printed
// in the dot file.
static unsigned long num_evan_dot = 0;
static unsigned long num_tang_dot = 0;
static unsigned long num_dead_dot = 0;


/**************************************************************/
/* NAME : write_dot*/
/* DESCRIPTION : this function writes the RG as a the .dot file*/
/* PARAMETERS : id marking, pri, mark_type  */
/* RETURN VALUE : */
/**************************************************************/
#define RATE_BUF_SIZE 128
void write_dot(int id, int pri, int mark_type, int tr, double rate) {
    // printf("write_dot(id=%d, pri=%d, mark_type=%d, tr=%d)\n", id, pri, mark_type, tr);
    // Last source marking visited
    static int s_id = 0;
    static int s_pri = 0;
    static int s_tr = 0;
    static char s_mark_type_ch = '\0';
    static int s_is_saved = 0;
    // Already printed the initial marking?
    static int init_mark_printed = 0;
    static int num_v = 0;
    static int num_t = 0;
    char rate_buf[RATE_BUF_SIZE];
    rate_buf[0] = '\0';

    //-------------------------------------------
    // Classify the marking
    //-------------------------------------------
    int *num_marks, dead_count=-1, is_dead_mark=0;
    char mark_type_ch;
    const char* marking_style = "";
    if (IS_VANISHING(pri)) {
        num_marks = &num_v;
        mark_type_ch = 'V';
        marking_style = "color=gray";
    }
    else if (IS_TANGIBLE(pri)) {
        num_marks = &num_t;
        mark_type_ch = 'T';
    }
    else { // Dead marking
        num_marks = &num_t;
        is_dead_mark = 1;
        mark_type_ch = 'D';
        marking_style = "color=red";
    }

    // Previous marking was vanishing?
    const char* edge_style = "";
    if (s_pri != 0 && IS_VANISHING(s_pri))
        edge_style = "style=dotted color=dimgray";

    //-------------------------------------------
    // Determine if this is a new marking
    //-------------------------------------------
    int is_new_marking = 0;
    if ((id == 1) && !init_mark_printed) { // Initial marking
        init_mark_printed = 1;
        is_new_marking = 1;
        *num_marks = 1;
    }
    else if (id > *num_marks) { // New marking
        is_new_marking = 1;
        *num_marks = id;
        if (is_dead_mark)
            dead_count++;
    }
    if (mark_type == DEAD) {
        if (!is_new_marking)
            exit (-200);
    }

    //-------------------------------------------
    // Print the marking
    //-------------------------------------------
    if (is_new_marking) {
        if (dot_save_marking(id, mark_type_ch)) {
            fprintf(f_dot, "  %c%d [%s label=\"", mark_type_ch, id, marking_style);
            write_on_srg(f_dot, 1);
            switch (mark_type_ch) {
                case 'D':
                    fprintf(f_dot, "DEAD MARKING\n");
                    num_tang_dot++;
                    num_dead_dot++;
                    break;
                case 'T':
                    num_tang_dot++;
                    break;
                case 'V':
                    num_evan_dot++;
                    break;
            }
            fprintf(f_dot, "\"];\n");
        }
    }

    //-------------------------------------------
    // Print the edge with the firing
    //-------------------------------------------
    switch (mark_type) {
        case NORMAL: 
        case DEAD:
            // New marking reached by a firing of transition s_tr from the source marking
            if (!s_is_saved)
                break; // source state has been omitted from the .dot file
            if (!dot_is_marking_saved(id, mark_type_ch)) {
                // Frontier marking, write the destination marking as '...'
                fprintf(f_dot, "  %c%d [shape=none label=\"...\"];\n", mark_type_ch, id);
            }
            if (rate != INV_RATE) {
                snprintf(rate_buf, RATE_BUF_SIZE, " <font color=\"#3070E0\">(%lg)</font>", rate);
            }
            // Convert bname in html format
            char bname_html[2*MAX_TAG_SIZE], *p = bname, *q = bname_html;
            while (*p) {
                if (*p == '<' || *p == '>') {
                    *q++ = '&'; 
                    *q++ = (*p == '<') ? 'l' : 'g'; 
                    *q++ = 't'; 
                    *q++ = ';'; 
                    p++;
                }
                else *q++ = *p++;
            }
            *q = '\0';

            fprintf(f_dot, "  %c%d -> %c%d [%s label=<%s%s%s>];\n",
                    s_mark_type_ch, s_id, mark_type_ch, id, edge_style, 
                    TRANS_NAME(s_tr), bname_html, rate_buf);
            break;

        case TANGIBLE_OR_VANISHING:
            // set this marking as the source marking for the next firings.
            s_id = id;
            s_pri = pri;
            s_tr = tr;
            s_mark_type_ch = mark_type_ch;
            s_is_saved = dot_is_marking_saved(s_id, s_mark_type_ch);
            break;

        default:
            break;
    }





    // char str_marking_node_descr[1024]; // Node of the marking, with its label.
    // char str_firing_edge_label[1024];  // Label of the edge (fired transition)
    // char str_marking_id[128];          // Id of the node of the marking.
    // str_marking_node_descr[0] = '\0';
    // str_marking_id[0] = '\0';
    // str_firing_edge_label[0] = '\0';
   // // Prepare marking and firing labels
    // if (IS_VANISHING(pri)) {
    //     if (id > num_v) { // new marking
    //         sprintf(str_marking_node_descr, "  V%d [label=\"V%d\" color=gray];\n", id, id);
    //         num_v = id;
    //     }
    //     sprintf(str_marking_id, "V%d", id);
    //     //sprintf(str_firing_edge_label,"[style=dotted color=dimgray label=\" %s%s\"]",TRANS_NAME(s_tr),bname);
    //     sprintf(str_firing_edge_label, "[label=\" %s%s\"]", TRANS_NAME(s_tr), bname);
    // }
    // else if (IS_TANGIBLE(pri)) {
    //     if (id > num_t) { // new marking
    //         sprintf(str_marking_node_descr, "  T%d [label=\"T%d\"];\n", id, id);
    //         num_t = id;
    //     }
    //     sprintf(str_marking_id, "T%d", id);
    //     sprintf(str_firing_edge_label, "[label=\" %s%s\"]", TRANS_NAME(s_tr), bname);
    // }
    // else {
    //     sprintf(str_marking_node_descr, "  D%d [label=\"D%d\" color=red];\n", id, id);
    //     sprintf(str_marking_id, "D%d", id);
    //     //sprintf(str_firing_edge_label,"[label=\" %s%s\"]",TRANS_NAME(s_tr),bname);
    // }

    // switch (mark_type) { /* Tipo di marcatura */
    
    // case NORMAL: // reached a new marking
    //     if (IS_VANISHING(s_pri)) {
    //         fprintf(f_dot, "%s  V%d -> %s [style=dotted color=dimgray label=\" %s%s\"];\n", 
    //                 str_marking_node_descr, s_id, str_marking_id, TRANS_NAME(s_tr), bname);
    //     }
    //     else if (IS_TANGIBLE(s_pri)) {
    //         fprintf(f_dot, "%s  T%d -> %s  %s;\n", 
    //                 str_marking_node_descr, s_id, str_marking_id, str_firing_edge_label);
    //     }
    //     break;

    // case TANGIBLE_OR_VANISHING: // set this marking as the source marking for the next firings.
    //     if ((id == 1) && !init_mark_printed) { // Initial marking
    //         init_mark_printed = 1;
    //         fprintf(f_dot, "%s", str_marking_node_descr);
    //     }
    //     s_id = id;
    //     s_pri = pri;
    //     s_tr = tr;
    //     break;

    // case DEAD: // reached a new dead marking.
    //     fprintf(f_dot, "  %s\n", str_marking_node_descr);
    //     if (IS_VANISHING(s_pri)) // reached from a vanishing marking
    //         fprintf(f_dot, "  V%d -> D%d [style=dotted color=dimgray label=\" %s%s\"];\n", 
    //                 s_id, id, TRANS_NAME(s_tr), bname);
    //     else // reached from a tangible marking
    //         fprintf(f_dot, "  T%d -> D%d [label=\" %s%s\"];\n", 
    //                 s_id, id, TRANS_NAME(s_tr), bname);
    //     break;

    // default:
    //     break;
    // }/* Tipo di marcatura */
}

/*DOT
*Function
*/

// Last call
void dot_finish() {
    fprintf(f_dot, "report [ style = \"filled, bold\" penwidth = 5 fillcolor = \"white\" shape=box label="
            "<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"white\">");
#ifdef SYMBOLIC
    fprintf(f_dot, "<tr><td bgcolor=\"black\" align=\"center\" colspan=\"2\"><font color=\"white\">Symbolic Reachability Graph</font></td></tr>");
#else
    fprintf(f_dot, "<tr><td bgcolor=\"black\" align=\"center\" colspan=\"2\"><font color=\"white\">Reachability Graph</font></td></tr>");
#endif


    if (exceeded_markings_bound()) {
        // Not all markings have been shown AND we don't known how many markings the RG has
        fprintf(f_dot, "<tr><td align=\"left\" colspan=\"2\"><font color=\"#E10000\" face=\"Sans Bold\"><b>INCOMPLETE GRAPH:</b></font> Showing a subset of markings.</td></tr>");
        fprintf(f_dot, "<tr><td align=\"left\">Total markings:</td><td>%ld out of <font color=\"#E10000\">?? </font></td></tr>", num_tang_dot + num_evan_dot);
        if (num_evan_dot > 0) {
            fprintf(f_dot, "<tr><td align=\"left\">Tangible markings:</td><td>%ld out of <font color=\"#E10000\">?? </font></td></tr>", num_tang_dot - num_dead_dot);
            fprintf(f_dot, "<tr><td align=\"left\">Vanishing markings:</td><td>%ld out of <font color=\"#E10000\">?? </font></td></tr>", num_evan_dot);
        }
        if (num_dead_dot > 0)
            fprintf(f_dot, "<tr><td align=\"left\">Dead markings:</td><td>%ld out of <font color=\"#E10000\">?? </font></td></tr>", num_dead_dot);

    }
    else if (max_dot_markings <= 0 || max_dot_markings > tang+evan) {
        // All markings are shown in dot
        fprintf(f_dot, "<tr><td align=\"left\" colspan=\"2\">Showing all markings.</td></tr>");
        fprintf(f_dot, "<tr><td align=\"left\">Total markings:</td><td>%ld</td></tr>", tang + evan);
        if (num_evan_dot > 0) {
            fprintf(f_dot, "<tr><td align=\"left\">Tangible markings:</td><td>%ld</td></tr>", tang - dead);
            fprintf(f_dot, "<tr><td align=\"left\">Vanishing markings:</td><td>%ld</td></tr>", evan);
        }
        if (num_dead_dot > 0)
            fprintf(f_dot, "<tr><td align=\"left\">Dead markings:</td><td>%ld</td></tr>", dead);
    }
    else {
        // Not all markings have been shown BUT we known how many markings the RG has
        fprintf(f_dot, "<tr><td align=\"left\" colspan=\"2\"><font color=\"#7602B9\" face=\"Sans Bold\"><b>OPEN GRAPH:</b></font> Showing a subset of markings.</td></tr>");
        fprintf(f_dot, "<tr><td align=\"left\">Total markings:</td><td>%ld out of %ld</td></tr>", num_tang_dot + num_evan_dot, tang + evan);
        if (num_evan_dot > 0) {
            fprintf(f_dot, "<tr><td align=\"left\">Tangible markings:</td><td>%ld out of %ld</td></tr>", num_tang_dot - num_dead_dot, tang - dead);
            fprintf(f_dot, "<tr><td align=\"left\">Vanishing markings:</td><td>%ld out of %ld</td></tr>", num_evan_dot, evan);
        }
        if (num_dead_dot > 0)
            fprintf(f_dot, "<tr><td align=\"left\">Dead markings:</td><td>%ld out of %ld</td></tr>", num_dead_dot, dead);
    }

    fprintf(f_dot, "</table>> ];\n");
}










/* ********************************************************************** */
/*                                                                        */
/*   Scrittura su file del Symbolic Reachability Graph                    */
/*                                                                        */
/* ********************************************************************** */
void write_on_srg(FILE  *srg, int dir) {
    /* Init write_on_srg */
    int pl, k, cl;
#ifdef SWN
#ifdef SYMBOLIC
    int offset, sb, base;
#endif
#endif
    char obj_name[MAX_TAG_SIZE];

    // omit the multiplicity of colored tokens when it is 1, i.e.   1<c1> becomes:  <c1>
    int omit_clr_mult_1 = 0;
    // separator of colored tokens, i.e.  2 <c1> separator 3<c2> separator 10<c5>
    const char* clrtok_sep = "";
    // separator between places
    const char* place_sep = "";

    if (srg == f_dot) { // Writing the DOT/Graphviz file, change the file style.
        omit_clr_mult_1 = 1;
        clrtok_sep = "+";
        place_sep = " ";
    }

    // For each place in the net
    int fullplc_count = 0;
    for (pl = 0 ; pl < npl; pl++) {
        if (IS_FULL(pl)) { // Place pl is marked
            if (fullplc_count++ > 0)
                fputs(place_sep, srg);
#ifdef GSPN
            fprintf(srg, "%s(%d)", PLACE_NAME(pl), net_mark[pl].total);
#endif
#ifdef SWN
            fprintf(srg, "%s(", PLACE_NAME(pl));
            if (IS_NEUTRAL(pl)) {
                /* Posto neutro */
                fprintf(srg, "%d)", net_mark[pl].total);
            }/* Posto neutro */
            else {
                /* Posto colorato */
                Token_p tk_p;
                int clrtok_count = 0;

                /* Per ogni token della marcatura */
                for (tk_p = net_mark[pl].marking; tk_p != NULL; tk_p = NEXT_TOKEN(tk_p)) {
                    if (clrtok_count++ > 0)
                        fputs(clrtok_sep, srg);
                    if (!omit_clr_mult_1 || tk_p->molt!=1)
                        fprintf(srg, "%d", tk_p->molt);
                    fprintf(srg, "<");
                    for (k = 0 ; k < tabp[pl].comp_num - 1; k++) {
                        cl = tabp[pl].dominio[k];
#ifdef COLOURED
                        get_object_name(cl, tk_p->id[k], obj_name);
#endif
#ifdef SYMBOLIC
                        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                            get_dynamic_subclass_name(cl, tk_p->id[k], obj_name);
                        else
                            get_object_name(cl, tk_p->id[k], obj_name);
#endif
                        fprintf(srg, "%s,", obj_name);
                    }
                    cl = tabp[pl].dominio[k];
#ifdef COLOURED
                    get_object_name(cl, tk_p->id[k], obj_name);
#endif
#ifdef SYMBOLIC
                    if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1))
                        get_dynamic_subclass_name(cl, tk_p->id[k], obj_name);
                    else
                        get_object_name(cl, tk_p->id[k], obj_name);
#endif
                    fprintf(srg, "%s>", obj_name);
                }/* Per ogni token della marcatura */
                fprintf(srg, ")");
            }/* Posto colorato */
#endif
        }/* Posto pieno */
    }/* Per ogni posto */
    fprintf(srg, "\n");
#ifdef SWN
#ifdef SYMBOLIC
    for (cl = 0 ; cl < ncl; cl++) {
        /* Per ogni classe */
        if (IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl) == 1)) {
            /* Classe non ordinata */
            for (sb = 0, base = 0; sb < tabc[cl].sbc_num; sb++) {
                /* Per ogni sottoclasse statica */
                for (k = 0; k < GET_NUM_SS(cl, sb); k++) {
                    /* Per ogni sottoclasse dinamica */
                    if (dir) {
                        offset = sfl_h->min[cl][k + base];
                        get_dynamic_subclass_name(cl, offset, obj_name);
                        fprintf(srg, "|%s|=%d "
                                , obj_name
                                , GET_CARD(cl, sb, offset));
                    }
                    else {
                        get_dynamic_subclass_name(cl, tabc[cl].sbclist[sb].offset + k, obj_name);
                        fprintf(srg, "|%s|=%d "
                                , obj_name
                                , GET_CARD_BY_TOTAL(cl, sb, k));
                    }
                }/* Per ogni sottoclasse dinamica */
                base += GET_NUM_SS(cl, sb);
            }/* Per ogni sottoclasse statica */
        }/* Classe non ordinata */
    }/* Per ogni classe */
    fprintf(srg, "\n");
#endif
#endif
}/* End write_on_srg */

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/*                                                            */
/*   Scrittura sul file .grg                                  */
/*                                                            */
/**************************************************************/
void write_ctrs(FILE  *ctrs) {
    /* Init write_ctrs */
    register int pl;
    register unsigned char cc;

    for (pl = 0; pl < npl; pl++) {
        /* Per ogni posto */
        if (code_place[pl] < 255) {
            /* Molteplicita' minore del massimo consentito */
            cc = (unsigned char)(0xFF & code_place[pl]);
            /* putc(cc,ctrs); */
            putc(code_place[pl], ctrs);
        }/* Molteplicita' minore del massimo consentito */
        else {
            /* Molteplicita' >= del massimo consentito */
            out_error(ERROR_MARKING_CODING, pl, 0, 0, 0, NULL, NULL);
        }/* Molteplicita' >= del massimo consentito */
    }/* Per ogni posto */
}/* End write_ctrs */



#ifdef SWN
/**************************************************************/
/* NAME : write the marking*/
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/*                                                            */
/*   Scrittura sul file .grg                                  */
/*                                                            */
/**************************************************************/

void write_cvrs(FILE *f_cvrs, FILE *f_cvrsoff, int evan) { /* Init write_ctrs */

    int pl, k, cl, sb, token = 0;
    int lim, numcard;
    fprintf(f_cvrsoff, "T%d %d\n", evan, (int)ftell(f_cvrs));
    for (pl = 0; pl < npl; pl++) {
        /* Per ogni posto */
        if (IS_FULL(pl)) {
            /* Posto pieno */
            fprintf(f_cvrs, "%d(", pl);
            if (IS_NEUTRAL(pl)) {
                /* Posto neutro */
                fprintf(f_cvrs, "%d)", net_mark[pl].total);
            }/* Posto neutro */
            else {
                Token_p tk_p;
                for (tk_p = net_mark[pl].marking; tk_p != NULL; tk_p = NEXT_TOKEN(tk_p)) {
                    /* Per ogni token della marcatura */
                    numcard = 1;
#ifdef SWN
#ifdef SYMBOLIC
                    for (k = 0 ; k < tabp[pl].comp_num ; k++) {
                        cl = tabp[pl].dominio[k];
                        sb = get_static_subclass(cl, tk_p->id[k]);
                        //lim = get_obj_off(sb,cl);
                        // numcard=numcard*GET_CARD(cl,sb,lim);
                        numcard = numcard * GET_CARD(cl, sb, tk_p->id[k]);
                    }
#endif
#endif
                    token = token + numcard * tk_p->molt;
                }/* Per ogni token della marcatura */
                fprintf(f_cvrs, "%d)", token);
                token = 0;
            }/* Posto colorato */
        }/* Posto pieno */
    }/* Per ogni posto */
    fprintf(f_cvrs, "\n");
}/* End write_ctrs */
#endif


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/*                                                                        */
/*   Scrittura sul file .grg                                              */
/*                                                                        */
/*   NOTA: il grg e' creato "ad hoc"                                      */
/*   1 byte per ogni posto, max numero codificabile 255, i valori         */
/*   non significativi sono messi a 0                                     */
/*                                                                        */
/* ********************************************************************** */
void write_grg(FILE  *grg) {
    /* Init write_grg */
    register int i;

    fprintf(grg, "%d %d %d %d\n", 1, npl, max_priority, ntr);
    fprintf(grg, "%d %d\n", 0, npl);
    for (i = 0; i < npl; i++) {
        /* Per ogni posto */
        fprintf(grg, " %d %d %d %d %d\n"
                , 0, 0, max_place[i], 1, 0);
        fprintf(grg, "  %d %d %d %d %d\n"
                , i + 1, 255, 0, 0, 0);
    }/* Per ogni posto */
    for (i = 0; i < npl; i++)
        fprintf(grg, "%d %d %d %d %d %d %d %s\n"
                , i + 1, 0, 0, 0, 0, 255, init_place[i], PLACE_NAME(i));
}/* End write_grg */
#ifdef DEBUG_THROUGHPUT
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void update_thr_files(int  mark_type,  Tree_p  fire,  int  tr,  int  pri) {
    /* Init update_thr_files */
    int pos;

    switch (mark_type) {
    /* Tipo di marcatura */
    case NORMAL :
        if (IS_VANISHING(pri))
            fprintf(thr, "V%d\n"
                    , current_marking->marking->cont_tang);
        else if (IS_TANGIBLE(pri))
            fprintf(thr, "T%d\n"
                    , current_marking->marking->cont_tang);
        else
            fprintf(thr, "D%d\n"
                    , current_marking->marking->cont_tang);
        break;
    case TANGIBLE_OR_VANISHING:
        if (IS_VANISHING(pri))
            fprintf(thr, "V%d ", fire->marking->cont_tang);
        else {
            if (evan == 0) {
                last_tang = fire->marking->cont_tang;
                pos = ftell(thr);
                fprintf(vnum, "%d ", pos);
            }
            else {
                if (last_tang != fire->marking->cont_tang) {
                    pos = ftell(thr);
                    fprintf(vnum, "%d\n", evan);
                    fprintf(vnum, "%d ", pos);
                    last_tang = fire->marking->cont_tang;
                }
            }
            fprintf(thr, "T%d ", fire->marking->cont_tang);
        }
        fprintf(thr, "%d ", tr);
        break;
    case DEAD:
        fprintf(thr, "D%d\n", current_marking->marking->cont_tang);
        break;
    default:
        break;
    }/* Tipo di marcatura */
}/* End update_thr_files */
#endif
/**************************************************************/
static double s_last_rate = INV_RATE;

void update_rg_rate(double rate) {
    s_last_rate = rate;
}

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void update_rg_files(int  mark_type,  Tree_p  fire,  int  tr,  int  pri) {
    /* Init update_rg_files */

    double rate = s_last_rate;
    s_last_rate = INV_RATE;

    switch (mark_type) {
    /* Tipo di marcatura */
    case NORMAL :
#ifdef GSPN
        if (IS_VANISHING(pri))
            fprintf(srg, "MARKING V%ld \n"
                    , current_marking->marking->cont_tang);
        else if (IS_TANGIBLE(pri))
            fprintf(srg, "MARKING T%ld \n"
                    , current_marking->marking->cont_tang);
        else
            fprintf(srg, "MARKING D%ld \n"
                    , current_marking->marking->cont_tang);
#endif
        /*DOT
         *write file dot
         */
        if (dot_flag)
            write_dot(current_marking->marking->cont_tang, pri, mark_type, tr, rate);
        /*DOT
        *write file dot
        */
#ifdef SWN
#ifdef SYMBOLIC
        if (IS_VANISHING(pri))
            fprintf(srg, "MARKING V%ld # ordinary markings : %lg\n"
                    , current_marking->marking->cont_tang
                    , current_marking->marking->ordinary);
        else if (IS_TANGIBLE(pri))
            fprintf(srg, "MARKING T%ld # ordinary markings : %lg\n"
                    , current_marking->marking->cont_tang
                    , current_marking->marking->ordinary);
        else
            fprintf(srg, "MARKING D%ld # ordinary markings : %lg\n"
                    , current_marking->marking->cont_tang
                    , current_marking->marking->ordinary);
#endif
#ifdef COLOURED
        if (IS_VANISHING(pri))
            fprintf(srg, "MARKING V%ld \n"
                    , current_marking->marking->cont_tang);
        else if (IS_TANGIBLE(pri))
            fprintf(srg, "MARKING T%ld \n"
                    , current_marking->marking->cont_tang);
        else
            fprintf(srg, "MARKING D%ld \n"
                    , current_marking->marking->cont_tang);
#endif
#endif
        write_on_srg(srg, 1);
        fprintf(srg, "\n");
        break;
    case TANGIBLE_OR_VANISHING:

        /*DOT
         *write file dot
         */
        if (dot_flag)
            write_dot(fire->marking->cont_tang, pri, mark_type, tr, rate);
        /*DOT
        *write file dot
        */
        if (IS_VANISHING(pri))
            fprintf(srg, "MARKING V%ld (vanishing)\n"
                    , fire->marking->cont_tang);
        else
            fprintf(srg, "MARKING T%ld (tangible)\n"
                    , fire->marking->cont_tang);
        /* write_on_srg(srg,0); */
        fprintf(srg, "[%s (<", TRANS_NAME(tr));
        break;
    case DEAD:
        /*DOT
        *write file dot
        */
        if (dot_flag)
            write_dot(current_marking->marking->cont_tang, pri, mark_type, tr, rate);
        /*DOT
        *write file dot
        */
#ifdef GSPN
        fprintf(srg, "MARKING D%ld (dead)\n"
                , current_marking->marking->cont_tang);
#endif
#ifdef SWN
#ifdef SYMBOLIC
        fprintf(srg, "MARKING D%ld # ordinary marking: %lg (dead)\n"
                , current_marking->marking->cont_tang
                , current_marking->marking->ordinary);
#endif
#ifdef COLOURED
        fprintf(srg, "MARKING D%ld (dead)\n"
                , current_marking->marking->cont_tang);
#endif
#endif
        write_on_srg(srg, 1);
        fprintf(srg, "\n");
        break;
    default:
        break;
    }/* Tipo di marcatura */
}/* End update_rg_files */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void code_marking() {
    /* Init code_marking */
    register int pl;

    for (pl = 0 ; pl < npl ; pl++) {
        /* Per ogni posto */
        code_place[pl] = net_mark[pl].total;
        if (code_place[pl] < min_place[pl])
            min_place[pl] = code_place[pl];
        else if (code_place[pl] > max_place[pl])
            max_place[pl] = code_place[pl];
    }/* Per ogni posto */
}/* End code_marking */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void write_final_results(FILE  *fp) {




    /* Init write_final_results */
    fprintf(fp, "\n");
#ifdef GSPN
    fprintf(fp, " ***** Reachability Graph ***** \n");
#endif
#ifdef SWN
#ifdef SYMBOLIC
    fprintf(fp, " ***** Symbolic Reachability Graph ***** \n");
#endif
#ifdef COLOURED
    fprintf(fp, " ***** Reachability Graph ***** \n");
#endif
#endif
    fprintf(fp, "\n");
    fprintf(fp, "TANGIBLE MARKINGS  : %ld\n", tang - dead);
    fprintf(fp, "VANISHING MARKINGS : %ld\n", evan);
    fprintf(fp, "DEAD MARKINGS      : %ld\n", dead);
    fprintf(fp, "\n");
    fprintf(fp, "TOTAL MARKINGS     : %ld\n", (tang + evan));
    fprintf(fp, "\n");
    fprintf(fp, " *************************************** \n");
    fprintf(fp, "\n");

    if (print_stat_for_gui()) { // Notify the GUI of these statistical values
        printf("#{GUI}# RESULT STAT num_tangible_markings %ld\n", tang - dead);
        printf("#{GUI}# RESULT STAT num_vanishing_markings %ld\n", evan);
        printf("#{GUI}# RESULT STAT num_dead_markings %ld\n", dead);
    }

#ifdef SWN
#ifdef SYMBOLIC
    fprintf(fp, "Ordinary tangible markings : %1.14lg\n", ord_tang);
    fprintf(fp, "Ordinary vanishing markings : %1.14lg\n", ord_evan);
    fprintf(fp, "Ordinary dead markings : %1.14lg\n", ord_dead);

    if (print_stat_for_gui()) { // Notify the GUI of these statistical values
        printf("#{GUI}# RESULT STAT num_ordinary_tangible_markings %1.14lg\n", ord_tang);
        printf("#{GUI}# RESULT STAT num_ordinary_vanishing_markings %1.14lg\n", ord_evan);
        printf("#{GUI}# RESULT STAT num_ordinary_dead_markings %1.14lg\n", ord_dead);
    }

#endif
#endif
    if (home)
        fprintf(fp, "The initial marking is a home state\n");
    fprintf(fp, "Time required ----------> %ld\n", new_time - old_time);
    fprintf(fp, "\n");
    fprintf(fp, " *************************************** \n");

    if (print_stat_for_gui()) { // Tell the Java GUI the total build time.
        printf("#{GUI}# RESULT STAT home_state %s\n", home?"True":"False");
        printf("#{GUI}# RESULT STAT build_time %ld seconds\n", new_time - old_time);
    }

#ifdef SWN
#ifdef SYMBOLIC
    if (running_for_MCC())
        fprintf(fp, "\nSTATE_SPACE STATES %g  TECHNIQUES SYMMETRIES SEQUENTIAL_PROCESSING\n\n", 
                ord_tang + ord_evan + ord_dead); // NOTE: probabilmente sbagliato: conta due volte le dead markings
#endif
#endif


}/* End write_final_results */
#endif
