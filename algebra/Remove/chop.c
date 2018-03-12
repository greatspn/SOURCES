#define ___CHOP___
#	include "global.h"
#include "chop.h"


void ChopPlace(float *pl_x, float *pl_y, float xx, float yy) {
    double          float_x, float_y, chop_fract;

    float_x = xx - *pl_x;
    float_y = yy - *pl_y;
    if ((chop_fract = float_x * float_x + float_y * float_y) > 0.0)
        chop_fract = sqrt(chop_fract);
    chop_fract = place_radius / (chop_fract * zoom_level);
    *pl_x = float_x * chop_fract + *pl_x;
    *pl_y = float_y * chop_fract + *pl_y;
}

void ChopTrans(float *tr_x, float *tr_y, float xx, float yy, int orient) {
    double          cos_arc, a_length;
    double          float_x, float_y;
    double          delta_x, delta_y;
    struct trans_object tt;
    float           xxx, yyy;

    tt.orient = orient;
    FNormTrans(&tt, *tr_x, *tr_y, tr_x, tr_y);
    FNormTrans(&tt, xx, yy, &xxx, &yyy);
    float_x = (*tr_x) - xxx;
    float_y = (*tr_y) - yyy;
    if ((a_length = float_x * float_x + float_y * float_y) > 0.0)
        a_length = sqrt(a_length);
    cos_arc = float_x / a_length;
    delta_x = (double) trans_length / (2.0 * zoom_level);
    delta_y = (double) titrans_height / (2.0 * zoom_level);
    float_x = *tr_x - cos_arc * delta_x;
    float_y = *tr_y - ((sign(float_y)) * delta_y);
    RNormTrans(&tt, (float) float_x, (float) float_y, tr_x, tr_y);
}

void FNormTrans(struct trans_object *trans, float tx, float ty, float *x, float *y) {
    switch (trans->orient) {
    case HORIZONTAL:
        *x = tx;
        *y = ty;
        break;
    case FORTYFIVE:
        *x = (tx + ty) * ISQRT2;
        *y = (ty - tx) * ISQRT2;
        break;
    case MINUSFORTYFIVE:
        *x = (tx - ty) * ISQRT2;
        *y = (tx + ty) * ISQRT2;
        break;
    default:
        *x = ty;
        *y = -tx;
        break;
    }
}

void RNormTrans(struct trans_object *trans, float tx, float ty, float *x, float *y) {
    switch (trans->orient) {
    case HORIZONTAL:
        *x = tx;
        *y = ty;
        break;
    case FORTYFIVE:
        *x = (tx - ty) * ISQRT2;
        *y = (tx + ty) * ISQRT2;
        break;
    case MINUSFORTYFIVE:
        *x = (tx + ty) * ISQRT2;
        *y = (ty - tx) * ISQRT2;
        break;
    default:
        *x = -ty;
        *y = tx;
        break;
    }
}
