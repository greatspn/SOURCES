/********************************************************************
 *
 * Date: Paris, July 17, 1990
 * Program: cntrl.c
 * Description:
 * Programmer: Giovanni Chiola
 * notes:
 *
 ********************************************************************/

#include <stdlib.h>
#include <unistd.h>

/*#define DEBUG
*/

#ifdef DEBUG

#define DEBUGinterrupt

#endif

#include "simul_decl.h"

int my_pid;
int out_sock_id;
char netname[256];
char hostname[128];

static struct sockaddr_in gi_sock_addr;

char bbb[GI_SIM_BUF_SIZE];

int main(int argc,
         char **argv,
         char **envp) {
    int ii, kk;
    unsigned int jj;
    char copy_history[1024];
    struct protoent *p_ent;
    int tcp_number;
    struct hostent *h_ent;

#ifdef DEBUG
    fprintf(stderr, "Start of cntrl\n");
#endif
    if (argc < 2) {
        fprintf(stderr, "Simulation cntrl ERROR: no net name !\n");
        exit(1);
    }
    sprintf(netname, argv[1]);
    if (argc > 2) {
        char **a_p = &(argv[2]);
        char *s_p;
        unsigned ii = argc - 2;
        while (ii--) {
            s_p = *(a_p++);
            if (*s_p == '-') {
                switch (*++s_p) {
                case 'G' :
                    break;
                case 'T' :
                    break;
                case 's' :
                    break;
                case 't' :
                    break;
                }
            }
        }
    }
    p_ent = getprotobyname("tcp");
    tcp_number = p_ent->p_proto;
    if ((out_sock_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Simulation cntrl ERROR: couldn't create socket !\n");
        exit(1);
    }
    if (gethostname(hostname, 128)) {
        fprintf(stderr, "Simulation cntrl ERROR: couldn't get hostname !\n");
        exit(1);
    }
    h_ent = gethostbyname(hostname);
    gi_sock_addr.sin_family = AF_INET;
    gi_sock_addr.sin_addr = *((struct in_addr *)(*(h_ent->h_addr_list)));
    for (ii = IPPORT_RESERVED, jj = TRUE ; jj ; ++ii) {
        gi_sock_addr.sin_port = htons((u_short)ii);
        jj = bind(out_sock_id, (struct sockaddr *)&gi_sock_addr, sizeof(gi_sock_addr));
    }
    if (listen(out_sock_id, 1) == -1) {
        fprintf(stderr, "Simulation cntrl ERROR: couldn't listen socket !\n");
        exit(1);
    }
    sprintf(bbb, inet_ntoa(gi_sock_addr.sin_addr));
#ifdef DEBUG
    fprintf(stderr, "%s, h_name=%s, h_addrtype=%d, h_length=%d\n",
            bbb, h_ent->h_name, h_ent->h_addrtype, h_ent->h_length);
#endif
    sprintf(copy_history, "csh -x greatspn1.5/engine nets/%s -G %d %s -T &", netname,
            gi_sock_addr.sin_port, bbb);
    system(copy_history);
    jj = sizeof(gi_sock_addr);
    if ((kk = accept(out_sock_id, (struct sockaddr *)&gi_sock_addr, &jj)) == -1) {
        fprintf(stderr, "Simulation cntrl ERROR: couldn't accept socket !\n");
        exit(1);
    }
    read(kk, bbb, GI_SIM_BUF_SIZE);
    /*
      if (setsockopt(kk,tcp_number,TCP_NODELAY,TRUE,sizeof(int)) == -1) {
    fprintf(stderr,"Simulation cntrl ERROR: couldn't set TCP_NODELAY \n");
    exit(1);
        }
    */
    sscanf(bbb, "%d", &my_pid);
#ifdef DEBUG
    fprintf(stderr, "cntrl: read pid=%d\n", my_pid);
#endif
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sending 'Vt 0' command\n");
#endif
    sprintf(bbb, "Vt 0");
    write(kk, bbb, GI_SIM_BUF_SIZE);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sending 'Vp 0' command\n");
#endif
    sprintf(bbb, "Vp 0");
    write(kk, bbb, GI_SIM_BUF_SIZE);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sleeping 4\n");
#endif
    sleep(4);
    sprintf(bbb, "c 10000.0");
    write(kk, bbb, GI_SIM_BUF_SIZE);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sleeping 4\n");
#endif
    sleep(4);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sleeping 4\n");
#endif
    sleep(4);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sending OOB interrupt\n");
#endif
    sprintf(bbb, "I");
    send(kk, bbb, GI_SIM_BUF_SIZE, MSG_OOB);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sleeping 4\n");
#endif
    sleep(4);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sleeping 4\n");
#endif
    sleep(6);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sending 'vt 0' command\n");
#endif
    sprintf(bbb, "vt 0");
    write(kk, bbb, GI_SIM_BUF_SIZE);
#ifdef DEBUG
    fprintf(stderr, "cntrl: ... sending Q command\n");
#endif
    sprintf(bbb, "Q");
    write(kk, bbb, GI_SIM_BUF_SIZE);
#ifdef DEBUG
    fprintf(stderr, "End of cntrl\n");
#endif
    return 0;
}

