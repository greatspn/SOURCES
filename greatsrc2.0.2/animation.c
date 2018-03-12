/*
 * animation.c  - timed interactive simulation of a GSPN.
 */



#define DEBUG
/*
#define DEBUG_GC
*/


/*#define SunOS4x
#define SunOS5x*/

/*
#define SunOS3x
*/

#include "../simsrc2/simul_decl.h"
#include "global.h"
#include "optionsdialog.h"
#include "filer.h"
#include "draw.h"
#include "res.h"
#include "liveness.h"
#include "rate.h"
#include "showgdi.h"
#include "Canvas.h"
#include "afire.h"
#include <unistd.h>
#include <stdlib.h>


#ifdef SunOS5x
#	include <sys/types.h>
#	include <sys/ddi.h>
#endif

#define TOKENS 0
#define PARAM 1



int             esist_buff = FALSE;
int             outvector_flag = FALSE;
int             sn, nn, mem;

double          sim_time = 0.0;
int             cur_pri = 0;
extern XtAppContext	appContext;

static struct coordinate *en_list = NULL;

/* predeclaration */
void create_synchro();


//static void anim_highlight_place(struct place_object *place)
//{
//    struct coordinate *c_p;
//
//    c_p = (struct coordinate *) emalloc(COORD_SIZE);
//    c_p->next = en_list;
//    en_list = c_p;
//    c_p->x = place->center.x;
//    c_p->y = place->center.y;
//}
//

static void anim_highlight_trans(struct trans_object *trans) {
    struct coordinate *c_p;

    c_p = (struct coordinate *) emalloc(COORD_SIZE);
    c_p->next = en_list;
    en_list = c_p;
    c_p->x = trans->center.x;
    c_p->y = trans->center.y;
}


static void anim_reset_enlist() {
    struct coordinate *n_p;

    dehighlight_trans();
    dehighlight_list(en_list, (int) trans_length);
    if (en_list == NULL)
        return;
    for (; en_list != NULL; en_list = n_p) {
        n_p = en_list->next;
        free((char *) en_list);
    }
}

#define SLEEP_MICROSEC (unsigned)125000

void create_synchro() {
    char            fname[256];
    FILE *fp;

    sprintf(fname, "%s.SIMCC", GetCurrentFilename());
    fp = fopen(fname, "w");
    fprintf(fp, "waiting\n");
    (void)fclose(fp);
#ifdef DEBUG
    fprintf(stderr, "GreatSPN: created SIMCC\n");
#endif
}


void wait_synchro() {
    char            fname[256];
    FILE *fp;
    int count = 16;

#ifdef DEBUG
    fprintf(stderr, "GreatSPN: Start wait_synchro\n");
#endif
    sprintf(fname, "%s.SIMCC", GetCurrentFilename());
    while ((count-- > 0) && (fp = fopen(fname, "r")) != NULL) {
        (void)fclose(fp);
#ifdef DEBUG
        fprintf(stderr, "GreatSPN: waiting for SIMCC to be deleted\n");
#endif
        usleep(SLEEP_MICROSEC);
    }
#ifdef DEBUG
    if (count >= 0)
        fprintf(stderr, "GreatSPN: synchronized on SIMCC\n");
    else
        fprintf(stderr, "GreatSPN: NOT synchronized on SIMCC (Timeout)\n");
#endif
}


void open_s_a() {
}


void read_us() {
    if (cur_pri > 0)
        return;
    wait_synchro();
    CollectRes(-1);
    histogram_list_new();
    ind_prest_list_new();
}


void delete_s_a() {
}


void synchro_s_r() {
}


void open_s_r() {
}


int             my_pid;
int             out_sock_id;
char            hostname[128];

static struct sockaddr_in gi_sock_addr;

#ifdef SunOS3x
static struct sockaddr_in kk_sock_addr;
#else
static struct sockaddr kk_sock_addr;
#endif

static char     bbb[GI_SIM_BUF_SIZE];
static int      kk;


void init_animation() {
    unsigned int              jj;
    struct hostent *h_ent;


#ifdef DEBUG
    fprintf(stderr, "      Start init_animation\n");
#endif
    if ((out_sock_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Simulation cntrl ERROR: couldn't create socket !\n");
        exit(1);
    }
    if (gethostname(hostname, 128)) {
        fprintf(stderr, "Simulation cntrl ERROR: couldn't get hostname !\n");
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "      ... init_animation: hostname=%s\n", hostname);
#endif
    if ((h_ent = gethostbyname(hostname)) == NULL) {
        fprintf(stderr,
                "ERROR init_animation: host information not found for hostname=%s\n",
                hostname);
        return;
    }
#ifdef DEBUG
    fprintf(stderr, "      ... init_animation: got host by name\n");
#endif
    /*
      bzero((char *) &gi_sock_addr, sizeof(gi_sock_addr));
      bcopy(h_ent->h_addr, (char *) &gi_sock_addr.sin_addr, h_ent->h_length);
    */
    (void)memcpy(&(gi_sock_addr.sin_addr), *(h_ent->h_addr_list),
                 sizeof(gi_sock_addr.sin_addr));

#ifdef DEBUG
    fprintf(stderr, "     ... init_animation: inet address=%s\n",
            inet_ntoa(gi_sock_addr.sin_addr));
#endif
    gi_sock_addr.sin_family = AF_INET;
    gi_sock_addr.sin_port = htons((u_short) 0);
#ifdef DEBUG
    fprintf(stderr, "      ... init_animation: binding socket\n");
#endif
#ifdef SunOS3x
    if (bind(out_sock_id, (caddr_t) & gi_sock_addr, sizeof(gi_sock_addr)) == -1) {
#else
    if (bind(out_sock_id, (struct sockaddr *) & gi_sock_addr, sizeof(gi_sock_addr))	== -1) {
#endif
        fprintf(stderr, "Simulation cntrl ERROR: couldn't bind socket !\n");
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "      ... init_animation: listen socket\n");
#endif
    if (listen(out_sock_id, 2) == -1) {
        fprintf(stderr, "Simulation cntrl ERROR: couldn't listen socket !\n");
        exit(1);
    }
    jj = sizeof(gi_sock_addr);
#ifdef DEBUG
    fprintf(stderr, "      ... init_animation: getting socket name\n");
#endif
#ifdef SunOS3x
    if (getsockname(out_sock_id, &gi_sock_addr, &jj) < 0) {
#else
    if (getsockname(out_sock_id, (struct sockaddr *) & gi_sock_addr, &jj) < 0) {
#endif
        fprintf(stderr, "Simulation cntrl ERROR: couldn't getsockname !\n");
        exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "      End init_animation:\n");
    sprintf(bbb, inet_ntoa(gi_sock_addr.sin_addr));
    fprintf(stderr, "%s, h_name=%s, h_addrtype=%d, h_length=%d\n",
            bbb, h_ent->h_name, h_ent->h_addrtype, h_ent->h_length);
#endif
}

#ifdef DEBUG
#define read_bbb_from_kk \
  while ( read(kk,bbb,GI_SIM_BUF_SIZE) != GI_SIM_BUF_SIZE ) { \
    if ( errno != EINTR ) { \
	fprintf(stderr,"GreatSPN ERROR: couldn't read socket %d\n", errno ); \
	fire_close_proc_alt(); \
	return; \
      } \
    } \
  fprintf(stderr,"... GreatSPN receiving:%s\n", bbb );
#else
#define read_bbb_from_kk \
  while ( read(kk,bbb,GI_SIM_BUF_SIZE) != GI_SIM_BUF_SIZE ) { \
    if ( errno != EINTR ) { \
	fprintf(stderr,"GreatSPN ERROR: couldn't read socket %d\n", errno ); \
	fire_close_proc_alt(); \
	return; \
      } \
    }
#endif

#ifdef DEBUG
#define write_bbb_in_kk \
  while ( write(kk,bbb,GI_SIM_BUF_SIZE) != GI_SIM_BUF_SIZE ) { \
    if ( errno != EINTR ) { \
	fprintf(stderr,"GreatSPN ERROR: couldn't write socket %d\n", errno ); \
	fire_close_proc_alt(); \
	return; \
      } \
    } \
  fprintf(stderr,"... GreatSPN sending:%s\n", bbb );
#else
#define write_bbb_in_kk \
  while ( write(kk,bbb,GI_SIM_BUF_SIZE) != GI_SIM_BUF_SIZE ) { \
    if ( errno != EINTR ) { \
	fprintf(stderr,"GreatSPN ERROR: couldn't write socket %d\n", errno ); \
	fire_close_proc_alt(); \
	return; \
      } \
    }
#endif

void init_int_sim() {
    unsigned int             jj;
    char            command[1024];
    char *h_n;

#ifdef DEBUG
    fprintf(stderr, "start of init_int_sim\n");
#endif
    sprintf(bbb, inet_ntoa(gi_sock_addr.sin_addr));
    /* fprintf(stderr,"... bbb written\n"); */
    Gspn = getenv("GREATSPN_SCRIPTDIR");
    /* fprintf(stderr,"... Gspn read\n"); */
    nets_dir_path = getenv("PWD");
    /* fprintf(stderr,"... nets_dir_path read\n"); */
    /*    sprintf(edit_file, (char *)


    		xv_get(net_name_item, PANEL_VALUE));
    	*/



    /* fprintf(stderr,"... edit_file read\n"); */
    /*    sprintf(host_name, (char *)

    		xv_get(host_name_item, PANEL_VALUE));
    */
    h_n = optGetHostname();
    /* fprintf(stderr,"... host_name read\n");
    gethostname( buf, 512);*/
//    if ( strcmp(host_name,h_n) == 0 )
    /*        sprintf(command,"csh -x %s/engine %s -G %d %s -T &",
                    Gspn,GetCurrentFilename(),
                    ntohs(gi_sock_addr.sin_port), bbb );*/
//        sprintf(command,"csh -x %s/engine %s -G %d %s -T &",
//                        Gspn,GetCurrentFilename(),
//                        ntohs(gi_sock_addr.sin_port), bbb );
    sprintf(command, "/bin/sh -x %s/engine %s -G %d %s -T &",
            Gspn, GetCurrentFilename(),
            ntohs(gi_sock_addr.sin_port), bbb);
//    else
//        sprintf(command,"rsh %s csh -x %s/engine %s -G %d %s -T &",
//                        host_name, Gspn, GetCurrentFilename(),
//                        ntohs(gi_sock_addr.sin_port), bbb );
#ifdef DEBUG
    fprintf(stderr, "GreatSPN: %s\n", command);
#endif
    system(command);
    jj = sizeof(kk_sock_addr);
    while ((kk = accept(out_sock_id, &kk_sock_addr, &jj)) == -1
            && errno == EINTR);
    if (kk == -1) {
        fprintf(stderr,
                "Simulation cntrl ERROR: couldn't accept socket ERRNO=%d !\n",
                errno);
        exit(1);
    }
    read_bbb_from_kk
    sscanf(bbb, "%d", &my_pid);
    set_int_sim_visibility();
#ifdef DEBUG
    fprintf(stderr,
            "  ... GreatSPN init_int_sim (cur_command=%d) starting redisplay_canvas\n",
            cur_command);
#endif
    redisplay_canvas();
    show_result_flag = TRUE;

#ifdef DEBUG
    fprintf(stderr, "END init_int_sim\n");
#endif
}

void set_int_sim_visibility() {
    int             count;
    struct group_object *cur_group;

    if (IsLayerVisible(WHOLE_NET_LAYER)) {
        if (cur_object == DETRANS || cur_object == EXTRANS ||
                cur_object == IMTRANS || cur_object == RESULT) {
            if (cur_object == RESULT)
                res_visible = TRUE;
            else
                res_visible = FALSE;
            if (cur_object == DETRANS || cur_object == EXTRANS
                    || cur_object == RESULT)
                sprintf(bbb, "Vt -1");
            else
                sprintf(bbb, "Vt 0");
            write_bbb_in_kk
        }
    }
    else {
        if (cur_object == DETRANS || cur_object == EXTRANS ||
                cur_object == IMTRANS || cur_object == RESULT) {
            if (cur_object == RESULT)
                res_visible = TRUE;
            else
                res_visible = FALSE;
            if (cur_object == DETRANS || cur_object == EXTRANS
                    || cur_object == RESULT)
                sprintf(bbb, "vt -1");
            else
                sprintf(bbb, "vt 0");
            write_bbb_in_kk
        }
        for (count = 1, cur_trans = netobj->trans; cur_trans != NULL;
                cur_trans = cur_trans->next, ++count) {
            if (IsLayerListVisible(cur_trans->layer)) {
                sprintf(bbb, "Vt %d", count);
                write_bbb_in_kk
            }
        }
        if (cur_object != DETRANS && cur_object != EXTRANS &&
                cur_object != RESULT) {
            for (cur_group = netobj->groups; cur_group != NULL;
                    cur_group = cur_group->next) {
                for (cur_trans = cur_group->trans; cur_trans != NULL;
                        cur_trans = cur_trans->next, ++count) {
                    if (IsLayerListVisible(cur_trans->layer)) {
                        sprintf(bbb, "Vt %d", count);
                        write_bbb_in_kk
                    }
                }
            }
        }
    }
    if (IsLayerVisible(WHOLE_NET_LAYER)) {
        sprintf(bbb, "Vp 0");
        write_bbb_in_kk
    }
    else {
        sprintf(bbb, "vp 0");
        write_bbb_in_kk
        for (count = 1, cur_place = netobj->places; cur_place != NULL;
                cur_place = cur_place->next, ++count) {
            if (IsLayerListVisible(cur_place->layer)) {
                sprintf(bbb, "Vp %d", count);
                write_bbb_in_kk
            }
        }
    }
    sprintf(bbb, "T");
    write_bbb_in_kk
}


void modify_evlist(int             nt) {
    create_synchro();
    sprintf(bbb, "l %d", nt);
    write_bbb_in_kk
    sprintf(bbb, "T");
    write_bbb_in_kk
    sprintf(bbb, "S");
    write_bbb_in_kk
    sprintf(bbb, "L");
    write_bbb_in_kk
    read_bbb_from_kk
    receive_newstate(bbb);
    sprintf(bbb, "p %f", sim_time);
    write_bbb_in_kk
}


void receive_newstate(char           *bbbp) {
    int             np, pm, cp;
    struct place_object *pp;
    struct trans_object *tp;
    char            tim[TAG_SIZE];
    float           xloc, yloc;
    int             nt;
    double          ff;
    int             ii;

#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : start of receive_newstate\n");
#endif
    for (ii = 2; ii;) {
        switch (*bbbp) {
        case 's':
#ifdef DEBUG
            fprintf(stderr, " ... GreatSPN received '%c'\n", *bbb);
#endif
            --ii;
            sscanf(bbbp + 1, "%lf %d %d", &sim_time, &cur_pri, &np);
            display_curtime(sim_time);
            if (np > 0)
                sscanf(bbbp + 1, "%*f %*d %*d %d", &pm);
            for (pp = netobj->places, cp = 1; pp != NULL;
                    ++cp, pp = pp->next) {
                DrawMarking(pp, CLEAR);
#ifdef DEBUG
                fprintf(stderr,
                        " ... GreatSPN: old-marking (%s) cp=%d, m=%d\n", pp->tag, cp, pp->tokens);
#endif
                if (cp == np) {
#ifdef DEBUG
                    fprintf(stderr, " ... GreatSPN: new-marking np=%d, pm=%d\n", np, pm);
#endif
                    pp->tokens = pm;
                    read_bbb_from_kk
                    sscanf(bbb, "%d", &np);
                    if (np > 0)
                        sscanf(bbb, "%*d %d", &pm);
                }
                else {
#ifdef DEBUG
                    fprintf(stderr, " ... GreatSPN: new-marking cp=%d, 0\n", cp);
#endif
                    pp->tokens = 0;
                }
                DrawMarking(pp, SET);
            }
            break;
        case 'l':
#ifdef DEBUG
            fprintf(stderr, " ... GreatSPN received '%c'\n", *bbb);
#endif
            --ii;
            anim_reset_enlist();
            ClearThroughputs();
            EraseLive();
            if (rate_visible) {
                ShowRate(FALSE);
                rate_visible = FALSE;
                update_mode();
            }
            for (tp = netobj->trans; tp != NULL; tp = tp->next) {
                if (tp->f_time > 0.0) {
                    if (!res_visible) {
                        xloc = (tp->center.x + tp->ratepos.x) * zoom_level;
                        yloc = (tp->center.y + tp->ratepos.y + 10) * zoom_level;
                        sprintf(tim, "%1.5g", tp->f_time);
                        printf("******** %s\n", tim);
                        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc), SET,
                                    canvas_font, tim);

                        /*****************************************************

                        xv_text(canvas_pw, (int) (xloc), (int) (yloc),
                        GXinvert, time_font14, tim);
                        */
                    }
                    tp->f_time = 0.0;
                }
            }
            bbbp = bbbp + 1;
            sscanf(bbbp, "%d", &nt);
            while (nt != 0) {
                sscanf(bbbp, "%*d %lf", &ff);
#ifdef DEBUG
                fprintf(stderr, " ... GreatSPN: e-list nt=%d, ff=%f\n", nt, ff);
#endif
                tp = n_to_trans(nt);
                if (ff <= 0.0) {
#ifdef DEBUG
                    fprintf(stderr, "  ... GreatSPN: imm trans nt=%d\n", nt);
#endif
                    anim_highlight_trans(tp);
                }
                else if (!cur_pri) {
#ifdef DEBUG
                    fprintf(stderr, "  ... GreatSPN: timed trans nt=%d\n", nt);
#endif
                    tp->f_time = ff;
                    if (!res_visible) {
                        sprintf(tim, "%1.5g", ff);
                        printf("******** %s\n", tim);
                        xloc = (tp->center.x + tp->ratepos.x) * zoom_level;
                        yloc = (tp->center.y + tp->ratepos.y + 10) * zoom_level;
                        gdiDrawText(&mainDS, (int)(xloc), (int)(yloc), SET,
                                    canvas_font, tim);

                        /******************************************************
                        xv_text(canvas_pw, (int) (xloc), (int) (yloc),
                            GXinvert, time_font14, tim);*/

                        anim_highlight_trans(tp);
                    }
                }
                read_bbb_from_kk
                bbbp = bbb;
                sscanf(bbbp, "%d", &nt);
            }
            if (!res_visible)
                highlight_list(en_list, (int) trans_length, FALSE, NULL);
            break;
        case 'E':
            fprintf(stderr, " ... GreatSPN ERROR: Got a simulation error\n");
            close_int_sim();
            return;
            break;
        default:
            fprintf(stderr, " ... GreatSPN ERROR: unexpected answer '%c'\n", *bbb);
            exit(1);
            break;
        }
        if (ii) {
            read_bbb_from_kk
        }
    }
    read_us(); /* collect performance results */
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : end of receive_newstate\n");
#endif
}


#define FORW 1
#define BACK 2


static int      flag_end_transition = FALSE;

void receive_transition() {
    struct trans_object *tp;
    int             nt;

#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : start of receive_transition\n");
#endif
    flag_end_transition = FALSE;
    read_bbb_from_kk
    switch (*bbb) {
    case 't':
#ifdef DEBUG
        fprintf(stderr, " ... GreatSPN received '%c'\n", *bbb);
#endif
        sscanf(bbb + 1, "%d", &nt);
        tp = n_to_trans(nt);
        anim_reset_enlist();
#ifdef DEBUG
        fprintf(stderr, " ... GreatSPN firing trans %d (%s)\n", nt, tp->tag);
#endif
        do_fire(tp);
        break;
    case '&':
        flag_end_transition = TRUE;
        if (direction == FORW) {
            put_msg(2, "Cannot continue simulation !\n");
        }
        else {
            put_msg(2, "Cannot backtrack any more !\n");
        }
        anim_reset_enlist();
        reset_mode_proc();
#ifdef DEBUG
        fprintf(stderr, " ... GreatSPN : end of receive_transition\n");
#endif
        return;
    case 'E':
        fprintf(stderr, "GreatSPN ERROR: Got a simulation error\n");
        close_int_sim();
        return;
    case 's':
    case 'l':
#ifdef DEBUG
        fprintf(stderr, " ... GreatSPN : end of receive_transition\n");
#endif
        receive_newstate(bbb);
        return;
    default:
        fprintf(stderr, "GreatSPN ERROR: unexpected answer '%c'\n", *bbb);
        exit(1);
        break;
    }
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : end of receive_transition\n");
#endif
    read_bbb_from_kk
    receive_newstate(bbb);
}


void close_int_sim() {
    struct trans_object *tp;

    sprintf(bbb, "I");
    send(kk, bbb, GI_SIM_BUF_SIZE, MSG_OOB);
    sprintf(bbb, "Q");
    write_bbb_in_kk
    setup_mark(PARAM);
    anim_reset_enlist();
    for (tp = netobj->trans; tp != NULL; tp = tp->next) {
        tp->f_time = 0.0;
    }
    cur_command = NULL_CHOICE;
    redisplay_canvas();
    close(kk);
}


void set_simulation_clock(double          ddd) {

#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : start of set_simulation_clock\n");
#endif
    create_synchro();
    sprintf(bbb, "t\n"); // , ddd ??
    write_bbb_in_kk
    sprintf(bbb, "c %f\n", ddd);
    write_bbb_in_kk
    sprintf(bbb, "T\n"); // , ddd); ??
    write_bbb_in_kk
    sprintf(bbb, "S");
    write_bbb_in_kk
    sprintf(bbb, "L");
    write_bbb_in_kk
    read_bbb_from_kk
    receive_newstate(bbb);
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : end of set_simulation_clock\n");
#endif
}


void one_step_simulation(char  fw_bw) {

#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : start of one_step_simulation\n");
#endif
    sprintf(bbb, "%c %d\n", fw_bw, int_val_step);
    create_synchro();
    write_bbb_in_kk
    receive_transition();
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN : end of one_step_simulation\n");
#endif
}


void get_number_step(const char   *numstep) {
#ifdef DEBUG
    fprintf(stderr, "GET_NUM\n");
#endif
    sn = atoi(numstep);
#ifdef DEBUG
    fprintf(stderr, "%d\n", sn);
#endif
    open_s_r();
    fprintf(sim_req, "b\n");
    fprintf(sim_req, "%d\n ", sn);
    fprintf(sim_req, "q\n");
    (void) fclose(sim_req);
    delete_s_a();
    /* setup_mark(TOKENS); */
    dehighlight_trans();
    redisplay_canvas();
}




void auto_simulation(int  vel) {
    open_s_r();
    fprintf(sim_req, "t\n");
    fprintf(sim_req, "%d\n ", vel);
    (void) fclose(sim_req);
    delete_s_a();
    outvector_flag = TRUE;
}


void force_simulation(int  nt) {
    open_s_r();
    fprintf(sim_req, "f\n");
    fprintf(sim_req, "%d\n ", nt);
    (void) fclose(sim_req);
    delete_s_a();
}



void reset_simulator(float           ftime) {
    struct place_object *pp;

    open_s_r();
    fprintf(sim_req, "M\n%f\n", ftime);
    for (pp = netobj->places; pp != NULL; pp = pp->next)
        fprintf(sim_req, " %d", (int)(pp->tokens));
    fprintf(sim_req, "\n");
    fprintf(sim_req, "0\n");
    (void) fclose(sim_req);
    delete_s_a();
    synchro_s_r();
}


void one_step_anim() {
    char            fw_bw;

    if (direction == FORW)
        fw_bw = 'F';
    else
        fw_bw = 'B';
    one_step_simulation(fw_bw);
}




#define NULL_TIM ((struct itimerval *)0)

static void my_animation(XtPointer client_data, XtIntervalId *id) {
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN: Start of my_animation\n");
#endif
    one_step_anim();
    if (autofire_flag) {
        int             delay = 1000 * (int)GetSimSliderTime();



        if (!delay)
            delay = 1;
        XtAppAddTimeOut(appContext, delay, my_animation, NULL);
    }
#ifdef DEBUG
    fprintf(stderr, " ... GreatSPN: End of my_animation\n");
#endif

}


void start_simula() {
#ifdef DEBUG
    fprintf(stderr, "GreatSPN: start_simula called\n");
#endif
    (void) my_animation((XtPointer)NULL, (XtIntervalId *)NULL);
    set_t_fire_name("");
}

void reset_simulation_screen() {
    struct trans_object *tp;

    for (tp = netobj->trans; tp != NULL; tp = tp->next) {
        tp->f_time = 0.0;
    }
    set_int_sim_visibility();
    create_synchro();
    sprintf(bbb, "S");
    write_bbb_in_kk
    sprintf(bbb, "L");
    write_bbb_in_kk
    setup_mark(TOKENS);
    read_bbb_from_kk
    receive_newstate(bbb);
}

#ifdef SunOS5x

static int ww, jj;

void usleep(unsigned howmuch) {
    while (howmuch--) {
        int ii = 2;
        while ((ii = decr(ii)) > 0) {
            ii--;
            /*
            jj = 1;
            jj = decr(jj);
            ii -= jj;
                 */
        }
    }
}

int decr(int vv) {
    ww = vv;
    vv *= 3;
    ww += vv;
    ww /= 4;
    ww--;
    return (ww);
}

#endif
