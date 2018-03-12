/***********************************************************************
 *  File: simul_decl.h
 *  Date: Paris, July 17 1990
 *  Programmer: Giovanni Chiola
 ***********************************************************************/

#include <math.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#if !defined(__linux__) && !defined(_HPUX_SOURCE)
#	include <sys/sockio.h>
#else
/*#	include <sys/socketio.h>*/
#	include <sys/ioctl.h>
#endif

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>

extern FILE *fopen();
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

extern char netname[];
extern char hostname[];
extern int my_pid;
extern int out_sock_id;
#define GI_SIM_BUF_SIZE 32

#define CMD_s (long)((trans_num<<1)+1)
#define CMD_R (long)((trans_num<<1)+2)
#define CMD_b (long)((trans_num<<1)+3)
#define CMD_c (long)((trans_num<<1)+4)
#define CMD_p (long)((trans_num<<1)+5)
#define CMD_E (long)((trans_num<<1)+6)
#define CMD_Q (long)((trans_num<<1)+7)

