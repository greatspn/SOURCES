/********************************************************************
 *
 * Date: Torino, August 24, 1993
 * Program: engine_control.c
 * Description: simulation control and communication module for the
 *		simulation engine of GreatSPN 1.5 package
 * Programmer: Giovanni Chiola
 * notes:
 *
 ********************************************************************/


/*#define DEBUG*/


/*
#define DEBUGpipe
*/

#ifdef DEBUG

#define DEBUGsocket
/*
*/

#define DEBUGinterrupt
/*
*/

#endif

/*#define SunOS5x
#define SunOS4x
#define SunOS3x
*/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "engine_control.h"
#include "engine_pn.h"
#include "../WN/INCLUDE/compact.h"

#ifdef SunOS5x
#	include <sys/file.h>
#endif

#ifdef DEBUGpipe
#	define STORE_COMPACT(a,f) store_compact(a,f); \
				fprintf(stderr,"engine: store_compact(%ld)\n",a)
#	define STORE_DOUBLE(a,f) store_double(a,f); \
			fprintf(stderr,"engine: store_double(%lg)\n",*(a))
#else
#	define STORE_COMPACT(a,f) store_compact(a,f)
#	define STORE_DOUBLE(a,f) store_double(a,f)
#endif /* DEBUGpipe */

#include "engine_decl.h"
#include "engine_event.h"

int record_traces = FALSE;

int break_point_flag;
int backup_flag;
int terminate_flag;
int trace_flag;
double target_time;

int my_pid;
int out_sock_id;
char netname[256];
char hostname[128];

static struct itimerval itimer_value;
static struct itimerval itimer_ovalue;


void setup_alarm() {
    if (setitimer(ITIMER_REAL, &itimer_value, &itimer_ovalue)) {
        fprintf(stderr, "Simulation Engine ERROR: couldn't set ITIMER_REAL\n");
        exit(1);
    }
}

void clear_alarm() {
    itimer_value.it_value.tv_sec = 0;
    if (setitimer(ITIMER_REAL, &itimer_value, &itimer_ovalue)) {
        fprintf(stderr, "Simulation Engine ERROR: couldn't set ITIMER_REAL\n");
        exit(1);
    }
    itimer_value.it_value.tv_sec = CHECKPOINT_SECS;
}


void itimer_real_handler(int sig, int code,
                         struct sigcontext *scp,
                         char *addr) {
    backup_flag = TRUE;
    setup_alarm();
}


char bbb[GI_SIM_BUF_SIZE];

void interrupt_handler(int sig, int code,
                       struct sigcontext *scp,
                       char *addr) {
    int ii;

    switch (sig) {
    case SIGTERM :
#ifdef DEBUGinterrupt
        fprintf(stderr, " SIGTERM interrupt\n");
#endif
        clear_alarm();
        break_point_flag = TRUE;
        terminate_flag = TRUE;
        break;
    case SIGQUIT :
#ifdef DEBUGinterrupt
        fprintf(stderr, " SIGQUIT interrupt\n");
#endif
        backup_flag = TRUE;
        break_point_flag = TRUE;
        terminate_flag = TRUE;
        break;
    case SIGTSTP :
#ifdef DEBUGinterrupt
        fprintf(stderr, " SIGTSTP interrupt\n");
#endif
        break_point_flag = TRUE;
        clear_alarm();
        break;
#ifndef Linux
    case SIGIO :
#ifdef DEBUGinterrupt
        fprintf(stderr, " SIGIO interrupt\n");
#endif
        break;
#endif
    case SIGURG :
#ifdef DEBUGinterrupt
        fprintf(stderr, " SIGURG interrupt\n");
#endif
        break_point_flag = TRUE;
        clear_alarm();
        for (ioctl(out_sock_id, SIOCATMARK, &ii) ; ! ii ;) {
            read(out_sock_id, bbb, GI_SIM_BUF_SIZE);
            ioctl(out_sock_id, SIOCATMARK, &ii);
        }

        break;
    }
}

void init_control() {
#ifdef DEBUG
    fprintf(stderr, "... Engine: start init_control\n");
#endif
    if (signal(SIGALRM, (void (*)(int)) itimer_real_handler) == SIG_ERR) {
        perror("Simulation Engine ERROR: couldn't start SIGALRM handler\n");
        exit(1);
    }
    if (signal(SIGTERM, (void (*)(int)) interrupt_handler) == SIG_ERR) {
        perror("Simulation Engine ERROR: couldn't start SIGTERM handler\n");
        exit(1);
    }
    if (signal(SIGQUIT, (void (*)(int)) interrupt_handler) == SIG_ERR) {
        perror("Simulation Engine ERROR: couldn't start SIGTERM handler\n");
        exit(1);
    }
    if (signal(SIGTSTP, (void (*)(int)) interrupt_handler) == SIG_ERR) {
        perror("Simulation Engine ERROR: couldn't start SIGTSTP handler\n");
        exit(1);
    }
    if (signal(SIGIO, (void (*)(int)) interrupt_handler) == SIG_ERR) {
        perror("Simulation Engine ERROR: couldn't start SIGIO handler\n");
        exit(1);
    }
    if (signal(SIGURG, (void (*)(int)) interrupt_handler) == SIG_ERR) {
        perror("Simulation Engine ERROR: couldn't start SIGURG handler\n");
        exit(1);
    }
    backup_flag = FALSE;
    break_point_flag = FALSE;
    terminate_flag = FALSE;
    itimer_value.it_interval.tv_sec = 0;
    itimer_value.it_interval.tv_usec = 0;
    itimer_value.it_value.tv_sec = CHECKPOINT_SECS;
    itimer_value.it_value.tv_usec = 0;
    setup_alarm();
#ifdef DEBUG
    fprintf(stderr, "... Engine: End init_control\n");
#endif
}



static struct sockaddr_in gi_sock_addr;
static int interact_by_socket = FALSE;


#ifdef DEBUGsocket
#define write_bbb_in_socket \
    if ( write(out_sock_id,bbb,GI_SIM_BUF_SIZE) < GI_SIM_BUF_SIZE ) { \
	fprintf(stderr,"Engine ERROR: couldn't write socket %d\n", errno ); \
	exit(1); \
      } \
  fprintf(stderr,"... engine sending :%s\n",bbb);
#else
#define write_bbb_in_socket \
    if ( write(out_sock_id,bbb,GI_SIM_BUF_SIZE) < GI_SIM_BUF_SIZE ) { \
	fprintf(stderr,"Engine ERROR: couldn't write socket %d\n", errno ); \
	exit(1); \
      }
#endif


static char *sim_eng_err = "Simulation Engine ERROR: ";

int main(int argc,
         char **argv,
         char **envp) {
    int pp;
    unsigned long kk;
    int seed_flag = FALSE, sss;

#ifdef DEBUG
    fprintf(stderr, "Start of engine\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "%sno net name\n", sim_eng_err);
        exit(1);
    }
    sprintf(netname, argv[1]);
    trace_flag = FALSE;
    target_time = 0.0;
    /*#ifndef SunOS5x*/
    bzero((char *) &gi_sock_addr, sizeof(gi_sock_addr));
    /*#endif*/
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii-- > 0) {
            s_p = *(a_p++);
            if (*s_p == '-') {
                switch (*++s_p) {
                case 'G' :
                    interact_by_socket = TRUE;
                    gi_sock_addr.sin_family = AF_INET;
                    s_p = *(a_p++); --ii;
                    sscanf(s_p, "%d", &pp);
                    gi_sock_addr.sin_port = htons((u_short)pp);
                    s_p = *(a_p++); --ii;
#ifdef DEBUG
                    fprintf(stderr,
                            "  ... Engine: option -G port=%d , INETaddr=%s\n",
                            gi_sock_addr.sin_port, s_p);
#endif
                    kk = inet_network(s_p);
                    kk = kk >> 16;
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: kk=%ld\n", kk);
                    fprintf(stderr, "  ... Engine: inet_addr=%d\n", inet_addr(s_p));
#endif
                    /*gi_sock_addr.sin_addr = inet_makeaddr(kk, inet_addr(s_p));*/
                    gi_sock_addr.sin_addr.s_addr = inet_addr(s_p);
#ifdef DEBUG
                    fprintf(stderr,
                            "  ... Engine: interact with port %d on %s (net=%s)\n",
                            pp, s_p,
                            inet_ntoa(*((struct in_addr *)(&(gi_sock_addr.sin_addr)))));
#endif
                    break;
                case 'T' :
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: trace flag on\n");
#endif
                    trace_flag = TRUE;
                    break;
                case 'S' :
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: continue seed flag on\n");
#endif
                    seed_flag = TRUE;
                    break;
                case 's' :
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: seed flag on\n");
#endif
                    s_p = *(a_p++); --ii;
                    sscanf(s_p, "%d", &sss);
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: set_seed(%d)\n", sss);
#endif
                    set_seed(sss);
                    seed_flag = TRUE;
                    break;
                case 't' :
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: target time\n");
#endif
                    sscanf(s_p + 1, "%lg", &target_time);
                    break;
                case 'r' :
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: record traces\n");
#endif
                    record_traces = TRUE;
                    break;
                default :
#ifdef DEBUG
                    fprintf(stderr, "  ... Engine: other argument '%c'?\n", *s_p);
#endif
                    break;
                }
            }
        }
    }
    if (! seed_flag)
        set_seed(0);
    if (gethostname(hostname, 128)) {
        fprintf(stderr, "%scouldn't get hostname\n", sim_eng_err);
        exit(1);
    }
    my_pid = getpid();
#ifdef DEBUG
    fprintf(stderr, "  ... Engine: hostname='%s', my_pid=%d\n",
            hostname, my_pid);
#endif
    if (interact_by_socket) {
        if ((out_sock_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            fprintf(stderr, "%scouldn't create socket\n", sim_eng_err);
            exit(1);
        }
#ifdef DEBUG
        fprintf(stderr, "  ... Engine: created socket %d\n", out_sock_id);
#endif
#ifdef SunOS3x
        if (connect(out_sock_id, (char *)&gi_sock_addr, sizeof(gi_sock_addr))
#else
        if (connect(out_sock_id, (struct sockaddr *)&gi_sock_addr,
                    sizeof(gi_sock_addr))
#endif
                == -1) {
#ifdef SunOS4x
            fprintf(stderr, "%s couldn't connect socket %d\n", sim_eng_err, errno);
            perror(NULL);

#else
            fprintf(stderr,
                    "%scouldn't connect socket %s\n", sim_eng_err, strerror(errno));
            exit(1);
#endif
        }


#ifdef DEBUG
        fprintf(stderr, "  ... Engine: socket %d connected\n", out_sock_id);
#endif
        /*
        if ( fcntl(out_sock_id,F_SETFL,FASYNC) == -1 ) {
          fprintf(stderr,
        	  "%scouldn't set FASYNC flag %d\n", sim_eng_err, errno );
            exit(1);
          }
        #ifdef DEBUG
           fprintf(stderr,"  ... Engine: socket %d set O_NONBLOCK\n", out_sock_id );
        #endif
             */
        if (fcntl(out_sock_id, F_SETOWN, my_pid) == -1) {
            fprintf(stderr,
                    "%scouldn't F_SETOWN %d\n", sim_eng_err, errno);
            exit(1);
        }
#ifdef DEBUG
        fprintf(stderr, "  ... Engine: socket %d set F_SETOWN %d\n",
                out_sock_id, my_pid);
#endif
        sprintf(bbb, "%d", my_pid);
#ifdef DEBUG
        fprintf(stderr, "  ... Engine: sending my_pid=%d\n", my_pid);
#endif
        write_bbb_in_socket
#ifdef DEBUG
        fprintf(stderr, "  ... Engine: my_pid=%d sent\n", my_pid);
#endif
    }
    else
        out_sock_id = 0;
#ifdef DEBUG
    fprintf(stderr, "  ... Engine: start init_pn\n");
#endif
    init_pn();
#ifdef DEBUG
    fprintf(stderr, "  ... Engine: start init_events\n");
#endif
    init_events(seed_flag);
#ifdef DEBUG
    fprintf(stderr, "  ... Engine: start init_control\n");
#endif
    init_control();

    while (! terminate_flag) {
        char *cp;
        int which;

        bbb[0] = '\0';
        if (interact_by_socket) {
#ifdef DEBUG
            fprintf(stderr, "... Engine: reading socket\n");
#endif
            if (read(out_sock_id, bbb, GI_SIM_BUF_SIZE) < GI_SIM_BUF_SIZE) {
                finish_events();
                /* putchar('Q'); */
                STORE_COMPACT(CMD_Q, stdout);
                fflush(stdout);
#ifdef DEBUG
                fprintf(stderr, "End of engine: couldn't read any more command\n");
#endif
                exit(0);
            }
#ifdef DEBUGsocket
            fprintf(stderr, "... engine receiving:%s\n", bbb);
#endif
        }
        else {
            fprintf(stderr, "Enter command:\n");
            fgets(bbb, GI_SIM_BUF_SIZE, stdin);
        }
#ifdef DEBUG
        fprintf(stderr, "... Engine received=%s\n", bbb);
#endif
        for (cp = bbb ;
                ((*cp < 'a') || (*cp > 'z')) && ((*cp < 'A') || (*cp > 'Z')) ;
                ++cp);
        switch (*cp++) {
        case 'B' : /* BACKWARD STEP SIMULATION */
            sscanf(cp + 1, "%d", &allowed_steps);
#ifdef DEBUG
            fprintf(stderr, "  ... Engine: received B %d\n", allowed_steps);
#endif
            backward_sim(0.0, trace_flag);
            if (cur_priority == 0) {
                /* putchar('R'); */
                STORE_COMPACT(CMD_R, stdout);
                fflush(stdout);
            }
            break;
        case 'F' : /* FORWARD STEP SIMULATION */
            sscanf(cp + 1, "%d", &allowed_steps);
#ifdef DEBUG
            fprintf(stderr, "  ... Engine: received F %d\n", allowed_steps);
#endif
            setup_alarm();
            forward_sim(trace_flag);
            clear_alarm();
            if (cur_priority == 0) {
                /* putchar('R'); */
                STORE_COMPACT(CMD_R, stdout);
                fflush(stdout);
            }
            break;
        case 'I' : /* ASYNCHRONOUS INTERRUPT */
            break_point_flag = TRUE;
#ifdef DEBUG
            fprintf(stderr, "  ... Engine: received asynchr. interrupt\n");
#endif
            break;
        case 'L' : /* REQUEST EVENT LIST */
#ifdef DEBUG
            fprintf(stderr, "  ... Engine: received 'L' command\n");
#endif
            if (cur_priority >= skip_immediate) {
                allowed_steps = 1;
                setup_alarm();
                forward_sim(FALSE);
                clear_alarm();
            }
            if (interact_by_socket) {
#ifdef DEBUG_graphics
                out_filtered_event_list(stderr, FALSE, 0, TRUE);
#else
                out_filtered_event_list(stderr, FALSE, out_sock_id, TRUE);
#endif
            }
            break;
        case 'Q' : /* QUIT WITHOUT BACKUP */
            /* putchar('Q'); */
            STORE_COMPACT(CMD_Q, stdout);
            fflush(stdout);
            clear_alarm();
            break_point_flag = TRUE;
            terminate_flag = TRUE;
            break;
        case 'S' : /* REQUEST CURRENT STATE */
#ifdef DEBUG
            fprintf(stderr, "  ... Engine: received 'S' command\n");
#endif
            if (cur_priority >= skip_immediate) {
                allowed_steps = 1;
                setup_alarm();
                forward_sim(FALSE);
                clear_alarm();
            }
            if (interact_by_socket) {
#ifdef DEBUG_graphics
                out_filtered_simulation_state(stderr, 0, TRUE);
#else
                out_filtered_simulation_state(stderr, out_sock_id, TRUE);
#endif
                /* if ( cur_priority == 0 ) { */
                /* putchar('R'); */
                STORE_COMPACT(CMD_R, stdout);
                fflush(stdout);
                /* } */
            }
            else {
                /* putchar('s'); */
                /*
                	          STORE_COMPACT( CMD_s, stdout );
                                  STORE_DOUBLE( &cur_time, stdout );
                		  pipe_marking(); fflush(stdout);
                */
            }
            break;
        case 'T' : /* SET TRACE FLAG */
            trace_flag = TRUE;
            break;
        case 'V' : /* SET OBJECTS VISIBLE */
            sscanf(cp + 1, "%d", &which);
            if (*cp == 'p')
                place_visibility(which, TRUE);
            else {
                if (which < 0) {
                    skip_immediate = -which;
                    which = 0;
                }
                else
                    skip_immediate = 255;
                trans_visibility(which, TRUE);
            }
            break;
        case 'b' : /* BACKWARD CONTINUOUS SIMULATION */
            sscanf(cp, "%lg", &target_time);
            allowed_steps = -1;
            backward_sim(target_time, trace_flag);
            break;
        case 'c' : /* SET CLOCK */
            sscanf(cp, "%lg", &target_time);
            setup_alarm();
            allowed_steps = -1;
            set_clock(target_time);
            clear_alarm();
            if (cur_priority == 0) {
                /* putchar('R'); */
                STORE_COMPACT(CMD_R, stdout);
                fflush(stdout);
            }
            break;
        case 'f' : /* FORWARD CONTINUOUS SIMULATION */
            setup_alarm();
            allowed_steps = -1;
            forward_sim(trace_flag);
            clear_alarm();
            break;
        case 'l' : /* MODIFY EVENT LIST */
            sscanf(cp, "%d", &which);
#ifdef DEBUG
            fprintf(stderr,
                    " ... Engine: putting transition %d first in event_list\n",
                    which);
#endif
            modify_ev_list(which);
            break;
        case 'p' : /* PURGE HISTORY */
            sscanf(cp, "%lg", &target_time);
            purge_history(target_time);
            break;
        case 'q' : /* QUIT WITH BACKUP */
            backup_flag = TRUE;
            break_point_flag = TRUE;
            terminate_flag = TRUE;
            break;
        case 's' : /* RESET CURRENT STATE */
            break;
        case 't' : /* CLEAR TRACE FLAG */
            trace_flag = FALSE;
            break;
        case 'v' : /* SET OBJECTS INVISIBLE */
            sscanf(cp + 1, "%d", &which);
            if (*cp == 'p')
                place_visibility(which, FALSE);
            else {
                if (which < 0) {
                    skip_immediate = -which;
                    which = 0;
                }
                else
                    skip_immediate = 255;
                trans_visibility(which, FALSE);
            }
            break;
        default :
            fprintf(stderr,
                    "%sunrecognized command '%c'\n", sim_eng_err,
                    *(cp - 1));
            terminate_flag = TRUE;
            break;
        }
        if (backup_flag && cur_time > max_checkpoint_time)
            do_checkpoint();
        clear_alarm();
    }

    finish_events();
#ifdef DEBUG
    fprintf(stderr, "End of engine\n");
#endif
    return 0;
}
