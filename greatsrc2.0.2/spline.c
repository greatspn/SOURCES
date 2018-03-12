#define ___SPLINE___
#	include "global.h"
#	include "postscript.h"
#	include "circle.h"
#	include "showgdi.h"
#	include "chop.h"
#	include "draw.h"
#include "spline.h"

void DrawSpline(struct arc_object *arc, int op, DrawingSurfacePun ds, int x_off, int y_off) {
    struct coordinate *point;
    int             nospl;
    float           xtran, ytran, xpros, ypros;
    float           fx2, fy2, fx3, fy3, xe, ye, ixpros, iypros, xctrans,
                    yctrans;
    float           center_x, center_y;
    float           x1, y1, x2, y2, x3, y3, xend, yend;
    char            string[5];
    int             xloc, yloc;

    /* count number of spline */
    for (point = arc->point->next->next, nospl = 1;
            point != NULL; point = point->next, nospl++);

    xctrans = arc->trans->center.x;
    yctrans = arc->trans->center.y;
    point = arc->point;
    xtran = ABS(point->x);
    ytran = ABS(point->y);
    point = point->next;
    xpros = ABS(point->x);
    ypros = ABS(point->y);

    switch (arc->type) {
    case TO_PLACE:
        break;
    case TO_TRANS:
        xe = xtran * zoom_level;
        ye = ytran * zoom_level;
        FNormTrans(arc->trans, xe, ye, &xe, &ye);
        FNormTrans(arc->trans, xtran, ytran, &x1, &y1);
        FNormTrans(arc->trans, xctrans, yctrans, &x2, &y2);
        x3 = xe;
        y3 = ye + ARROW_L * zoom_level * sign(y1 - y2);
        y1 = y1 + ARROW_L * sign(y1 - y2);
        RNormTrans(arc->trans, xe, ye, &xe, &ye);
        RNormTrans(arc->trans, x3, y3, &x3, &y3);
        RNormTrans(arc->trans, x1, y1, &xtran, &ytran);
        DrawArrow(x3 - x_off * zoom_level, y3 - y_off * zoom_level,
                  xe - x_off * zoom_level, ye - y_off * zoom_level, op, ds);
        break;
    case INHIBITOR:
        FNormTrans(arc->trans, xtran, ytran, &center_x, &center_y);
        FNormTrans(arc->trans, xctrans, yctrans, &x2, &y2);
        y3 = not_radius * sign(center_y - y2);
        x1 = center_x;
        y1 = center_y;
        center_y += y3;
        y3 += y3;
        y1 += y3;
        RNormTrans(arc->trans, center_x * zoom_level, center_y * zoom_level,
                   &xe, &ye);
        RNormTrans(arc->trans, x1, y1, &xtran, &ytran);
        Circle(xe - x_off * zoom_level, ye - y_off * zoom_level, not_radius, op, ds);
        break;
    default:
        break;
    }

    DrawCurv(&xend, &yend, xtran, ytran, xpros, ypros, arc, op, ds, x_off, y_off);

    if (nospl == 1) {
        xe = xend * zoom_level;
        ye = yend * zoom_level;
        ixpros = xpros * zoom_level;
        iypros = ypros * zoom_level;
        if (arc->type == TO_PLACE)
            DrawArrow(xe - x_off * zoom_level, ye - y_off * zoom_level,
                      ixpros - x_off * zoom_level, iypros - y_off * zoom_level, op, ds);
        else
            DrawFloatVector(xe - x_off * zoom_level, ye - y_off * zoom_level,
                            ixpros - x_off * zoom_level, iypros - y_off * zoom_level, op, ds);
        xe = arc->point->x * zoom_level;
        ye = arc->point->y * zoom_level;
        if (arc->color != NULL) {
            gdiDrawText(ds, (int)((xe + ixpros) * 0.5 + (arc->colpos.x - x_off)*zoom_level),
                        (int)((ye + iypros) * 0.5 + (arc->colpos.y - y_off)*zoom_level),
                        op, canvas_font, arc->color);
        }
        else if (arc->lisp != NULL) {
            gdiDrawText(ds, (int)((xe + ixpros) * 0.5 + (arc->colpos.x - x_off)*zoom_level),
                        (int)((ye + iypros) * 0.5 + (arc->colpos.y - y_off)*zoom_level),
                        op, canvas_font, arc->lisp->tag);
        }
        if (arc->mult > 1) {
            sprintf(string, "_%1d", arc->mult);
            gdiDrawText(ds, (int)((xe + ixpros) / 2 - x_off * zoom_level),
                        (int)((ye + iypros) / 2 - y_off * zoom_level),
                        op, canvas_font, string);
        }
        return;
    }
    point = arc->point->next;
    if (arc->mult < 0) {
        xe = xend * zoom_level;
        ye = yend * zoom_level;
        DrawFloatVector(xe - x_off * zoom_level, ye - y_off * zoom_level,
                        (ABS(point->x) - x_off) * zoom_level,
                        (ABS(point->y) - y_off) * zoom_level,
                        op, ds);
        xloc = (ABS(point->x) - x_off) * zoom_level;
        yloc = (ABS(point->y) - y_off) * zoom_level;
        if (arc->mult == -1)
            sprintf(string, "%s", arc->place->tag);
        else
            sprintf(string, "%1d*%s", -(arc->mult), arc->place->tag);
        gdiDrawText(ds, (int)xloc, (int)yloc, op, canvas_font, string);
        for (; point->next->next != NULL; point = point->next);
        xloc = (ABS(point->x) - x_off) * zoom_level;
        yloc = (ABS(point->y) - y_off) * zoom_level;
        if (arc->mult == -1)
            sprintf(string, "%s", arc->trans->tag);
        else
            sprintf(string, "%1d*%s", -(arc->mult), arc->trans->tag);
        gdiDrawText(ds, (int)xloc, (int)yloc, op, canvas_font, string);
        if (arc->type == TO_PLACE)
            DrawArrow((ABS(point->x) - x_off) * zoom_level,
                      (ABS(point->y) - y_off) * zoom_level,
                      (ABS(point->next->x) - x_off) * zoom_level,
                      (ABS(point->next->y) - y_off) * zoom_level, op, ds);
        else
            DrawFloatVector((ABS(point->x) - x_off) * zoom_level,
                            (ABS(point->y) - y_off) * zoom_level,
                            (ABS(point->next->x) - x_off) * zoom_level,
                            (ABS(point->next->y) - y_off) * zoom_level, op, ds);
        return;
    }
    /* compute intermediate points' spline */
    for (; --nospl; point = point->next) {
        x1 = xend;
        y1 = yend;
        x2 = ABS(point->x);
        y2 = ABS(point->y);
        x3 = ABS(point->next->x);
        y3 = ABS(point->next->y);
        CalcPoint(x1, y1, x2, y2, x3, y3, &xend, &yend, op, ds, x_off, y_off);
    }
    xe = xend * zoom_level;
    ye = yend * zoom_level;
    fx2 = x2 * zoom_level;
    fy2 = y2 * zoom_level;
    fx3 = x3 * zoom_level;
    fy3 = y3 * zoom_level;
    if (arc->color != NULL) {
        gdiDrawText(ds, (int)((fx2 + fx3) * 0.5 + (arc->colpos.x - x_off)*zoom_level),
                    (int)((fy2 + fy3) * 0.5 + (arc->colpos.y - y_off)*zoom_level),
                    op, canvas_font, arc->color);
    }
    else if (arc->lisp != NULL) {
        gdiDrawText(ds, (int)((fx2 + fx3) * 0.5 + (arc->colpos.x - x_off)*zoom_level),
                    (int)((fy2 + fy3) * 0.5 + (arc->colpos.y - y_off)*zoom_level),
                    op, canvas_font, arc->lisp->tag);
    }
    if (arc->mult > 1) {
        sprintf(string, "_%1d", arc->mult);
        gdiDrawText(ds, (int)((xe + fx3) / 2 - x_off * zoom_level),
                    (int)((ye + fy3) / 2 - y_off * zoom_level),
                    op, canvas_font, string);
    }
    if (arc->type == TO_PLACE)
        DrawArrow(xe - x_off * zoom_level, ye - y_off * zoom_level,
                  fx3 - x_off * zoom_level, fy3 - y_off * zoom_level, op, ds);
    else
        DrawFloatVector(xe - x_off * zoom_level, ye - y_off * zoom_level,
                        fx3 - x_off * zoom_level, fy3 - y_off * zoom_level, op, ds);
}


static void DrawCurv(float *xend, float *yend, float xtran, float ytran, float xnext, float ynext,
                     struct arc_object *arc, int op, DrawingSurfacePun ds, int x_off, int y_off)
/* draw spline at the transition side  */
{
    int             clockwise;
    float           xc, yc, xs, ys, ixend, iyend, xctrans, yctrans;
    float           fxc, fyc, fxtran, fytran, fxctrans, fyctrans, rad;
    float           dcnext, dnexte, alphacos, alphasin, vnextcx, vnextcy;
    float           vnextex, vnextey, nextex, nextey, malphasin;
    float           distpc, distps;
    float           fsign, fxnext, fynext, f1, f2;

    xctrans = ABS(arc->trans->center.x);
    yctrans = ABS(arc->trans->center.y);
    FNormTrans(arc->trans, xctrans, yctrans, &fxctrans, &fyctrans);
    FNormTrans(arc->trans, xtran, ytran, &fxtran, &fytran);
    FNormTrans(arc->trans, xnext, ynext, &fxnext, &fynext);
    if ((ABS(fxctrans - fxtran)) < 0.1) {
        *xend = xtran;
        *yend = ytran;
        return;
    }
    fsign = sign((fxnext - fxtran));
    fxc = fxctrans + fsign * RADIUS;
    fyc = fytran;
    rad = fsign * (fxc - fxtran);
    f1 = fxnext - fxc;
    f2 = fynext - fyc;
    if ((distpc = f1 * f1 + f2 * f2) > 0.0)
        distpc = (float) sqrt(distpc);
    if (distpc < (rad + RO)) {
        f1 = fxnext - fxtran;
        f2 = fynext - fytran;
        if ((distps = f1 * f1 + f2 * f2) > 0.0)
            distps = (float) sqrt(distps);
        rad = distps * RED_FACTOR;
        fxc = fxtran + fsign * rad;
    }
    f1 = fxc - fxnext;
    f2 = fyc - fynext;
    if ((dcnext = f1 * f1 + f2 * f2) > 0.0)
        dcnext = (float) sqrt(dcnext);
    if ((dnexte = dcnext * dcnext - rad * rad) > 0.0)
        dnexte = (float) sqrt(dnexte);
    alphacos = dnexte / dcnext;
    malphasin = rad / dcnext;
    vnextcx = (fxc - fxnext) / dcnext;
    vnextcy = (fyc - fynext) / dcnext;
    if ((fxtran > fxctrans && fytran > fyctrans) ||
            (fxtran < fxctrans && fyctrans > fytran)) {
        /* counter-clockwise rotation */
        clockwise = FALSE;
        alphasin = -malphasin;
        CalcVer(alphasin, alphacos, vnextcx, vnextcy, &vnextex, &vnextey);
    }
    else {
        /* clockwise rotation */
        clockwise = TRUE;
        alphasin = malphasin;
        CalcVer(alphasin, alphacos, vnextcx, vnextcy, &vnextex, &vnextey);
    }
    nextex = vnextex * dnexte;
    nextey = vnextey * dnexte;
    RNormTrans(arc->trans, fxnext + nextex, fynext + nextey, xend, yend);
    RNormTrans(arc->trans, fxc * zoom_level, fyc * zoom_level, &xc, &yc);
    RNormTrans(arc->trans, fxtran * zoom_level, fytran * zoom_level, &xs, &ys);
    ixend = *xend * zoom_level;
    iyend = *yend * zoom_level;
    if (clockwise)
        DrawCircle(xc - x_off * zoom_level, yc - y_off * zoom_level,
                   xs - x_off * zoom_level, ys - y_off * zoom_level,
                   ixend - x_off * zoom_level, iyend - y_off * zoom_level,
                   (rad * zoom_level), op, ds);

    else
        DrawCircle(xc - x_off * zoom_level, yc - y_off * zoom_level,
                   ixend - x_off * zoom_level, iyend - y_off * zoom_level,
                   xs - x_off * zoom_level, ys - y_off * zoom_level,
                   (rad * zoom_level), op, ds);
}


static void CalcVer(float alphasin, float alphacos, float vproscx, float vproscy, float *vprosex, float *vprosey)
/* rotation matrix */
{
    *vprosex = alphacos * vproscx - (alphasin * vproscy);
    *vprosey = alphasin * vproscx + alphacos * vproscy;
}
/*
float calc( x, y)
	float x,y;
{
    float temp;

    temp=(float)((atan2((double)y,(double)x)/(double)3.141592653)*(double)11520.);
	if(temp < 0.)
       temp=23040. + temp;

    return temp;
}
*/


static void DrawCircle(float xc, float yc, float xs, float ys,
                       float ixend, float iyend, float rad, int op, DrawingSurfacePun ds) {
    float           x, y;
    float           rsq;
    float           deltax, deltay, inc, dec, signx, signy;
    int             endquad;

    if (PSflag) {
        PSSpline(xc, yc, xs, ys, ixend, iyend) ;
        return;
    }

    if (rad <= 1.5)
        return;

    xs -= xc;
    ys -= yc;
    ixend -= xc;
    iyend -= yc;
    rsq = rad * rad;
    x = xs;
    y = ys;
    inc = 1.0;
    dec = -inc;
    if ((ixend >= 0.0) && (iyend > 0.0))
        endquad = 1;
    else if ((ixend < 0.0) && (iyend >= 0.0))
        endquad = 2;
    else if ((ixend <= 0.0) && (iyend < 0.0))
        endquad = 3;
    else
        endquad = 4;

    SetFunction(op, ds);

    for (;;) {
        /* CLOCKWISE ROTATION */
        if ((x > 0.0) && (y >= 0.0)) {
            deltax = dec;
            deltay = inc;
            signx = 1.0;
            signy = 1.0;
        }
        else if ((x <= 0.0) && (y > 0.0)) {
            deltax = dec;
            deltay = dec;
            signx = -1.0;
            signy = 1.0;
        }
        else if ((x < 0.0) && (y <= 0.0)) {
            deltax = inc;
            deltay = dec;
            signx = -1.0;
            signy = -1.0;
        }
        else {
            deltax = inc;
            deltay = inc;
            signx = 1.0;
            signy = -1.0;
        }
        if (ABS(x) < ABS(y)) {
            x += deltax;
            if (x < -rad)
                x = -rad;
            else if (x > rad)
                x = rad;
            if ((y = rsq - x * x) > 0.0)
                y = signy * sqrt(y);
            else
                y = 0.0;
        }
        else {
            y += deltay;
            if (y < -rad)
                y = -rad;
            else if (y > rad)
                y = rad;
            if ((x = rsq - y * y) > 0.0)
                x = signx * sqrt(x);
            else
                x = 0.0;
        }
        /* TEST FOR TERMINATION */
        switch (endquad) {
        case 1:
            if (x <= ixend && y >= 0.0)
                goto end_draw;
            break;
        case 2:
            if (y <= iyend && x <= 0.0)
                goto end_draw;
            break;
        case 3:
            if (x >= ixend && y <= 0.0)
                goto end_draw;
            break;
        default:
            if (y >= iyend && x >= 0.0)
                goto end_draw;
            break;
        }
        XDrawPoint(ds->display, ds->drawable, ds->gc, (int)(xc + x), (int)(yc + y)) ;
    }
end_draw:
    return;
}


static void CalcPoint(float x1, float y1, float x2, float y2, float x3, float y3,
                      float *xend, float *yend, int op, DrawingSurfacePun ds, int x_off, int y_off) {
    float           xs, ys, xc, yc;
    float           ix1, iy1, ixend, iyend;
    float           d21, d23, ls2, lc2, fxc, fyc, fxs, fys;
    float           v12x, v12y, v32x, v32y;
    float           c2xr, c2yr, s2x, s2y, e2x, e2y;
    float           lc2r, dist, vc2xr, vc2yr;
    float           rad, dx12, dy12, dx23, dy23;
    float           cosalpha, sinalpha;

    ix1 = x1 * zoom_level;
    iy1 = y1 * zoom_level;
    v12x = x1 - x2;
    dx12 = v12x * v12x;
    v12y = y1 - y2;
    dy12 = v12y * v12y;
    v32x = x3 - x2;
    dx23 = v32x * v32x;
    v32y = y3 - y2;
    dy23 = v32y * v32y;
    if ((d21 = dx12 + dy12) > 0.0)
        d21 = (float) sqrt(d21);
    if ((d23 = dx23 + dy23) > 0.0)
        d23 = (float) sqrt(d23);
    v12x = v12x / d21;
    v12y = v12y / d21;
    v32x = v32x / d23;
    v32y = v32y / d23;
    c2xr = (v12x + v32x);
    c2yr = (v12y + v32y);
    lc2r = c2xr * c2xr + c2yr * c2yr;
    if (lc2r <= 0.0) {
        xs = x2 * zoom_level;
        ys = y2 * zoom_level;
        DrawFloatVector(ix1 - x_off * zoom_level, iy1 - y_off * zoom_level,
                        xs - x_off * zoom_level, ys - y_off * zoom_level, op, ds);
        *xend = x2;
        *yend = y2;
        return;
    }
    lc2r = (float) sqrt(lc2r);
    vc2xr = c2xr / lc2r;
    vc2yr = c2yr / lc2r;
    cosalpha = ABS(vc2xr * v12x + vc2yr * v12y);
    if ((sinalpha = 1.0 - cosalpha * cosalpha) > 0.0)
        sinalpha = (float) sqrt(sinalpha);
    lc2 = RO;
    dist = d23 / 2;
    if (d21 < dist)
        dist = d21;
    dist = dist / cosalpha;
    if (lc2 > dist)
        lc2 = dist;
    rad = lc2 * sinalpha;
    ls2 = lc2 * cosalpha;
    fxc = x2 + vc2xr * lc2;
    fyc = y2 + vc2yr * lc2;
    s2x = v12x * ls2;
    s2y = v12y * ls2;
    fxs = x2 + s2x;
    fys = y2 + s2y;
    e2x = v32x * ls2;
    e2y = v32y * ls2;
    *xend = x2 + e2x;
    *yend = y2 + e2y;
    xs = fxs * zoom_level;
    ys = fys * zoom_level;
    DrawFloatVector(ix1 - x_off * zoom_level, iy1 - y_off * zoom_level,
                    xs - x_off * zoom_level, ys - y_off * zoom_level, op, ds);
    xc = fxc * zoom_level;
    yc = fyc * zoom_level;
    ixend = *xend * zoom_level;
    iyend = *yend * zoom_level;

    if (((v12x * v32y) - (v32x * v12y)) <= 0.0) {
        DrawCircle(xc - x_off * zoom_level, yc - y_off * zoom_level,
                   xs - x_off * zoom_level, ys - y_off * zoom_level,
                   ixend - x_off * zoom_level, iyend - y_off * zoom_level,
                   (rad * zoom_level), op, ds);
    }
    else {
        DrawCircle(xc - x_off * zoom_level, yc - y_off * zoom_level,
                   ixend - x_off * zoom_level, iyend - y_off * zoom_level,
                   xs - x_off * zoom_level, ys - y_off * zoom_level,
                   (rad * zoom_level), op, ds);
    }
}
