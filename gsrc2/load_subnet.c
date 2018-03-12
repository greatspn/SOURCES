/*
 *  program: struct.c
 *  date: September 11, 1987
 *  programmer: Giovanni Chiola
 *  address:	Dipartimento di Informatica, Universita' di Torino
 *		corso Svizzera 185, 10149 Torino, Italy.
 *
 *  purpose: structural analysis of a GSPN; produces the following files:
 *  notes:
 *    1) the net description must be in GreatSPN 1.3 format
 */


/*
#define DEBUG
*/


#include <stdio.h>

FILE *nfp, * ifp, * ofp;

#include "const.h"


typedef unsigned short ROWT;
#define ROWT_SIZE (unsigned)(sizeof(ROWT))

typedef ROWT *ROWP;
#define ROWP_SIZE (unsigned)(sizeof(ROWP))

typedef ROWP *ARCM;
#define ARCM_SIZE (unsigned)(sizeof(ARCM))

ARCM det_subnet;
ROWP det_trans, l_t_s;

struct subnet_str {
    ROWT type;
    ROWT equiv_s;
    ROWP trans;
    ROWP place;
    ROWP det_trans;
};

#define SBN_SIZE (unsigned)(sizeof(struct place_obj))

struct subnet_str *subnet_mat;

int det_num, subnet_num;

char *
ecalloc(nitm, sz)
unsigned nitm, sz;
{
    //extern char *	calloc();
    char 		*callptr;

    if ((callptr = calloc(nitm, sz)) == NULL) {
        fprintf(stderr, "ecalloc: couldn't fill request for %d, %d\n",
                nitm, sz);
        exit(2);
    }
    return (callptr);
}

load_subnet() {
    ARCM		pp1;
    ROWP		rwp;
    int			det, subnet, n_sub_d;
    int			type, t_num, p_num, shared_num, equiv_s;
    struct subnet_str   smp1;
    int                 i;

#ifdef DEBUG
    printf("  Start of load_subnet\n");
#endif

    /* read number of deterministic transitions in the net */
    /* read number of Isolated Subnet (IS) in the net */

    fscanf(nfp, "%d %d\n", &det_num, &subnet_num);

    rwp = l_t_s = (ROWP)ecalloc(trans_num, short_SIZE);
    for (i = trans_num ; i-- ; rwp++)
        *rwp = 0;
    smp1 = subnet_mat = (struct subnet_str *)ecalloc(subnet_num, SBN_SIZE);
    rwp = det_trans = (ROWP)ecalloc(det_num, short_SIZE);
    pp1 = det_subnet = (ARCM)ecalloc(det_num, ROWP_SIZE);
    for (i = det_num ; i-- ; pp1++) {
        register ROWP cp;
        int j;
        fscanf(nfp, "%d\n", &det);
        fscanf(nfp, "%d\n", &n_sub_d);
        rwp++ = (ROWT)det;
        *pp1 = cp = (ROWP)ecalloc(n_sub_d + 2, ROWT_SIZE);
        *cp++ = (ROWT)n_sub_d;
        for (j = n_sub_d ; j-- ; cp++) {
            fscanf(nfp, "%d", subnet);
            *cp = (ROWT)subnet;
        }
        *cp = (ROWT)0;
        fscanf(nfp, "\n");
    }
    for (i = subnet_num ; i-- ; smp1++) {
        register ROWP cp;
        int j;
        fscanf(nfp, "%d %d %d %d %d\n", &type, &t_num, &p_num, &shared_num, &equiv_s);
        smp1->type = (ROWT)type;
        smp1->equiv_s = equiv_s;
        smp1->trans = cp = (ROWP)ecalloc(t_num + 2, ROWT_SIZE);
        cp++ = t_num;
        for (j = t_num ; j-- ; cp++) {
            fscanf(nfp, "%d", tt);
            *cp = (ROWT)tt;
            *(l_t_s + tt - 1) = subnet_num - j;
        }
        *cp = (ROWT)0;
        fscanf(nfp, "\n");

        smp1->place = cp = (ROWP)ecalloc(p_num + 2, ROWT_SIZE);
        cp++ = p_num;
        for (j = p_num ; j-- ; cp++) {
            fscanf(nfp, "%d", pp);
            *cp = (ROWT)pp;
        }
        *cp = (ROWT)0;
        fscanf(nfp, "\n");

        smp1->del_trans = cp = (ROWP)ecalloc(shared_num + 2, ROWT_SIZE);
        cp++ = shared_num;
        for (j = shared_num ; j-- ; cp++) {
            fscanf(nfp, "%d", tt);
            *cp = (ROWT)tt;
        }
        *cp = (ROWT)0;
        fscanf(nfp, "\n");
    }
#ifdef DEBUG
    printf("  End of load_subnet\n");
#endif
}

