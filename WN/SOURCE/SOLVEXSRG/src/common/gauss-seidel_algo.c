
#include <stdio.h>
#include <stdlib.h>
#include "structSolv.h"

#define EPS 0.0000001
#define MAXITER 1000
#define true 1
#define false 0


// Free of the transition matrix
void
free_mtrx(mtrx *U,
          unsigned long nb) {
    unsigned long i;
    for (i = 0; i < nb; i++)
        free(U[i].first);
    free(U);
}


// printing of the transition matrix
void
print_mtrx(mtrx *U,
           unsigned long nb) {

    unsigned long i, j;

    for (i = 1; i < nb; i++) {
        printf("********* Dest = %d *********** \n", i);
        for (j = 0; j < U[i].ninn; j++)
            printf(" source = %d , rate= %g \n ",
                   U[i].first[j].fm, U[i].first[j].rt);
    }

}



// Gauss-seidel algo. to solve the system pi*U=0.
// This implem. is widely inspired from the code
// of G. Chiola in GreatSPN core.

double *
gauss_seidel(double *mprob,
             mtrx *column,
             unsigned long top_tan) {
    register unsigned long ii;
    register unsigned long rn;
    register unsigned long i;
    itm *r_p;
    mtrx *c_p;
    double *res_p;
    double rr, r, sum, err = 0, eps =EPS ;
    unsigned long  noiter = 0,
                   maxiter = MAXITER,
                   max_i = top_tan + 1;
   
    do {
        /* Loop until accuracy is met */
        i = max_i; ++noiter;  err = sum = 0.0;
        for (res_p = mprob + 1, c_p = column + 1 ; --i ; ++res_p, ++c_p) {
            /* For each unknown */
            if ((ii = c_p->ninn)) {
                r_p = c_p->first;
                r = r_p->rt;
                rn = r_p->fm;
                r *= mprob[rn];
                while (--ii) {
                    rr = (++r_p)->rt;
                    rn = r_p->fm;
                    rr *= mprob[rn];
                    r += rr;
                }

                if (r == 0.0)
                    r = 0.01 * *res_p;
            }
            else
                r = 0.01 * *res_p;
            rr = r - *res_p;
            err += ((rr >= 0.0) ? rr : -rr);
            *res_p = r;
            sum += r;
        }/* For each unknown */

        if ((rr = sum - 1.0) > 0.01 || rr < -0.01) {
            rr = 1.0 / sum;
            for (i = max_i, res_p = mprob ; --i ;) {
                *++res_p *= rr;
            }
        }
    if (noiter%100==0)    
      printf("Err: %.14f(%.8f) Iteration: %d\n",err,eps,noiter);
    } /* Loop until accuracy is met */
    while (err > eps && noiter < maxiter);
    printf("Err: %.14f Iteration: %d\n",err,noiter);
    return mprob;
}

