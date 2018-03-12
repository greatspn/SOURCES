/*********************************************************************
*                           MODULO                                   *
*                                                                    *
*   Questo modulo contiene la tabella della t_student e quella
    della normale.						     *
*                                                                    *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/service.h"
#include "../../INCLUDE/ealloc.h"

#ifdef SIMULATION

#define a   16807
#define modulo 2147483647
#define mi 127774
#define r 13971

static double q = 1.0 / modulo;
extern int seed;

static double coef_t_st[8][30] = {
    {
        .325, .289, .277, .271, .267, .265, .263, .262, .261, .260,
        .260, .259, .259, .258, .258, .258, .257, .257, .257, .257,
        .257, .256, .256, .256, .256, .256, .256, .256, .256, .256
    } ,
    {
        .727, .617, .584, .569, .559, .553, .549, .546, .543, .542,
        .540, .539, .538, .537, .536, .535, .534, .534, .533, .533,
        .532, .532, .532, .531, .531, .531, .531, .530, .530, .530
    } ,
    {
        1.376, 1.061, .978, .941, .920, .906, .896, .889, .883, .879,
        .876, .873, .870, .868, .866, .865, .863, .862, .861, .860,
        .859, .858, .858, .857, .856, .856, .855, .855, .854, .854
    } ,
    {
        3.078, 1.886, 1.638, 1.533, 1.476, 1.440, 1.415, 1.397, 1.383, 1.372,
        1.363, 1.356, 1.350, 1.345, 1.341, 1.337, 1.333, 1.330, 1.328, 1.325,
        1.323, 1.321, 1.319, 1.318, 1.316, 1.315, 1.314, 1.313, 1.311, 1.310
    } ,
    {
        6.314, 2.920, 2.353, 2.132, 2.015, 1.943, 1.895, 1.860, 1.833, 1.812,
        1.796, 1.782, 1.771, 1.761, 1.753, 1.746, 1.740, 1.734, 1.729, 1.725,
        1.721, 1.717, 1.714, 1.711, 1.708, 1.706, 1.703, 1.701, 1.699, 1.697
    } ,
    {
        12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
        2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086,
        2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042
    } ,
    {
        31.821, 6.965, 4.541, 3.747, 3.365, 3.143, 2.998, 2.896, 2.821, 2.764,
        2.718, 2.681, 2.650, 2.624, 2.602, 2.583, 2.567, 2.552, 2.539, 2.528,
        2.518, 2.508, 2.500, 2.492, 2.485, 2.479, 2.473, 2.467, 2.462, 2.457
    } ,
    {
        63.657, 9.925, 5.841, 4.604, 4.032, 3.707, 3.499, 3.355, 3.250, 3.169,
        3.106, 3.055, 3.012, 2.977, 2.947, 2.921, 2.898, 2.878, 2.861, 2.845,
        2.831, 2.819, 2.807, 2.797, 2.787, 2.779, 2.771, 2.763, 2.756, 2.750
    }
} ;
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
double random_generate() {
    /* Init random_generate */
    int h, l, la, s, hr;
    double u;

    if ((h = seed / mi)) {
        la = a * (l = seed - h * mi);
        hr = h * r;
        s = modulo - hr;
        if (la > s)
            u = (double)(seed = la - s);
        else
            u = (double)(seed = hr + la);
        return (u * q);
    }
    u = (double)(seed *= a);
    return (u * q);
}/* End random_generate */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_random_integer(int min,  int max) {
    /* Init get_random_integer */
    double y;
    int ret;

    y = random_generate();
    ret = floor(((y * (max - min + 1))));
    return (min + ret);
#ifdef WRONG
    int up = tabc[cl].card - 1;
    return (GET_RANDOM_INTEGER(0, up));
#endif
}/* End get_random_integer */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_random_object(int cl) {
    /* Init get_random_object */
    double y;
    int ret;

    y = random_generate();
    ret = floor(((y * (tabc[cl].card))));
    return (ret);
#ifdef WRONG
    int up = tabc[cl].card - 1;
    return (GET_RANDOM_INTEGER(0, up));
#endif
}/* End get_random_object */
#ifdef SYMBOLIC
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int get_random_dynamic_subclass(int cl) {
    /* Init get_random_dynamic_subclass */
    int random_object;

    random_object = get_random_object(cl);
    return (get_dynamic_from_object(cl, random_object));
}/* End get_random_dynamic_subclass */
#endif
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
double negexp(double mean) {
    /* Init negexp */
    return (((double)(-log(random_generate()) * ((double)(mean)))));
}/* End negexp */
/***********************************************************************
 *                                                                     *
 *  nome : read_Z()						       *
 *                                                                     *
 *  descrizione :                                                      *
 *                                                                     *
 *  parametri :                                                        *
 *                                                                     *
 *  return :  coefficiente .                                           *
 *                                                                     *
 ***********************************************************************/
double read_Z() {
    /* return(coef_Z[...][.........]) ; */
    return (0.5);
}
/***********************************************************************
 *                                                                     *
 *  nome : read_t_st()						      *
 *                                                                     *
 *  descrizione :  Restituisce il coefficiente relativo al grado di    *
 *                 liberta' e la confidenza inserita.                  *
 *                                                                     *
 *  parametri : conf_level (confidenza) ,degree ( grado di liberta')   *
 *                                                                     *
 *  return :  coefficiente .                                           *
 *                                                                     *
 ***********************************************************************/
double read_t_st(int conf_level , int degree) {
    if (degree > 30) {
        /* n troppo grande */
        return (coef_t_st[conf_level][29]) ;
    }/* n troppo grande */
    return (coef_t_st[conf_level][degree - 1]) ;
}
#endif
