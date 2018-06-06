/*
  This is a C version of the LSODA library. I acquired the original
  source code from this web page:

    http://www.ccl.net/cca/software/SOURCES/C/kinetics2/index.shtml

  I merged several C files into one and added a simpler interface. I
  also made the array start from zero in functions called by lsoda(),
  and fixed two minor bugs: a) small memory leak in freevectors(); and
  b) misuse of lsoda() in the example.

  The original source code came with no license or copyright
  information. I now release this file under the MIT/X11 license. All
  authors' notes are kept in this file.

  - Heng Li <lh3lh3@gmail.com>
 */

/* The MIT License

   Copyright (c) 2009 Genome Research Ltd (GRL).

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/


/* Contact: Heng Li <lh3@sanger.ac.uk> */
#include <cmath>



namespace SDE
{



/************
 * idamax.c *
 ************/

int idamax(int n, double* dx, int incx);
/* Purpose : Find largest component of double vector dx


   --- Input ---

   n    : number of elements in input vector
   dx   : double vector with n+1 elements, dx[0] is not used
   incx : storage spacing between elements of dx


   --- Output ---

   idamax : smallest index, 0 if n <= 0


   Find smallest index of maximum magnitude of dx.
   idamax = first i, i=1 to n, to minimize fabs( dx[1-incx+i*incx] ).

*/

/***********
 * dscal.c *
 ***********/

void dscal(int n, double da, double* dx, int incx);

/* Purpose : scalar vector multiplication

   dx = da * dx


   --- Input ---

   n    : number of elements in input vector
   da   : double scale factor
   dx   : double vector with n+1 elements, dx[0] is not used
   incx : storage spacing between elements of dx


   --- Output ---

   dx = da * dx, unchanged if n <= 0


   For i = 0 to n-1, replace dx[1+i*incx] with
   da * dx[1+i*incx].

*/

/**********
 * ddot.c *
 **********/

double ddot(int n, double* dx, int incx, double* dy, int incy);
/*
   Purpose : Inner product dx . dy


   --- Input ---

   n    : number of elements in input vector(s)
   dx   : double vector with n+1 elements, dx[0] is not used
   incx : storage spacing between elements of dx
   dy   : double vector with n+1 elements, dy[0] is not used
   incy : storage spacing between elements of dy


   --- Output ---

   ddot : dot product dx . dy, 0 if n <= 0


   ddot = sum for i = 0 to n-1 of
   dx[lx+i*incx] * dy[ly+i*incy] where lx = 1 if
   incx >= 0, else lx = (-incx)*(n-1)+1, and ly
   is defined in a similar way using incy.

*/

/***********
 * daxpy.c *
 ***********/

/*
From tam@dragonfly.wri.com Wed Apr 24 15:48:31 1991
Return-Path: <tam>
Date: Wed, 24 Apr 91 17:48:43 CDT
From: tam@dragonfly.wri.com
To: whitbeck@sanjuan.wrc.unr.edu
*/

void daxpy(int n, double da, double* dx, int incx, double* dy, int incy);
/*
   Purpose : To compute

   dy = da * dx + dy


   --- Input ---

   n    : number of elements in input vector(s)
   da   : double scalar multiplier
   dx   : double vector with n+1 elements, dx[0] is not used
   incx : storage spacing between elements of dx
   dy   : double vector with n+1 elements, dy[0] is not used
   incy : storage spacing between elements of dy


   --- Output ---

   dy = da * dx + dy, unchanged if n <= 0


   For i = 0 to n-1, replace dy[ly+i*incy] with
   da*dx[lx+i*incx] + dy[ly+i*incy], where lx = 1
   if  incx >= 0, else lx = (-incx)*(n-1)+1 and ly is
   defined in a similar way using incy.

*/

/***********
 * dgesl.c *
 ***********/

void dgesl(double** a, int n, int* ipvt, double* b, int job);
/*
   Purpose : dgesl solves the linear system
   a * x = b or Transpose(a) * x = b
   using the factors computed by dgeco or degfa.


   On Entry :

      a    : double matrix of dimension ( n+1, n+1 ),
             the output from dgeco or dgefa.
             The 0-th row and column are not used.
      n    : the row dimension of a.
      ipvt : the pivot vector from degco or dgefa.
      b    : the right hand side vector.
      job  : = 0       to solve a * x = b,
             = nonzero to solve Transpose(a) * x = b.


   On Return :

      b : the solution vector x.


   Error Condition :

      A division by zero will occur if the input factor contains
      a zero on the diagonal.  Technically this indicates
      singularity but it is often caused by improper argments or
      improper setting of the pointers of a.  It will not occur
      if the subroutines are called correctly and if dgeco has
      set rcond > 0 or dgefa has set info = 0.


   BLAS : daxpy, ddot
*/

/***********
 * dgefa.c *
 ***********/

void dgefa(double** a, int n, int* ipvt, int* info);

/*
   Purpose : dgefa factors a double matrix by Gaussian elimination.

   dgefa is usually called by dgeco, but it can be called directly
   with a saving in time if rcond is not needed.
   (Time for dgeco) = (1+9/n)*(time for dgefa).

   This c version uses algorithm kji rather than the kij in dgefa.f.
   Note that the fortran version input variable lda is not needed.


   On Entry :

      a   : double matrix of dimension ( n+1, n+1 ),
            the 0-th row and column are not used.
            a is created using NewDoubleMatrix, hence
            lda is unnecessary.
      n   : the row dimension of a.

   On Return :

      a     : a lower triangular matrix and the multipliers
              which were used to obtain it.  The factorization
              can be written a = L * U where U is a product of
              permutation and unit upper triangular matrices
              and L is lower triangular.
      ipvt  : an n+1 integer vector of pivot indices.
      *info : = 0 normal value,
              = k if U[k][k] == 0.  This is not an error
                condition for this subroutine, but it does
                indicate that dgesl or dgedi will divide by
                zero if called.  Use rcond in dgeco for
                a reliable indication of singularity.

                Notice that the calling program must use &info.

   BLAS : daxpy, dscal, idamax
*/

/***********
 * lsoda.c *
 ***********/

/*
From tam@dragonfly.wri.com Wed Apr 24 01:35:52 1991
Return-Path: <tam>
Date: Wed, 24 Apr 91 03:35:24 CDT
From: tam@dragonfly.wri.com
To: whitbeck@wheeler.wrc.unr.edu
Subject: lsoda.c
Cc: augenbau@sparc0.brc.uconn.edu


I'm told by Steve Nichols at Georgia Tech that you are interested in
a stiff integrator.  Here's a translation of the fortran code LSODA.

Please note
that there is no comment.  The interface is the same as the FORTRAN
code and I believe the documentation in LSODA will suffice.
As usual, a free software comes with no guarantee.

Hon Wah Tam
Wolfram Research, Inc.
tam@wri.com
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define max( a , b )  ( (a) > (b) ? (a) : (b) )
#define min( a , b )  ( (a) < (b) ? (a) : (b) )

#define ETA 2.2204460492503131e-16

 void     stoda(int neq, double *y, class SystEq& f, void *_data);
 void     correction(int neq, double *y, class SystEq&  f,  int *corflag, double pnorm, double *del, double *delp, double *told,
						   int *ncf, double *rh, int *m, void *_data);
void     prja(int neq, double *y, class SystEq& f, void *_data);
void     terminate(int *istate);
void     terminate2(double *y, double *t);
void     successreturn(double *y, double *t, int itask, int ihit, double tcrit, int *istate);
void     freevectors(void); /* this function does nothing */
void     _freevectors(void);
void     ewset(int itol, double *rtol, double *atol, double *ycur);
void     resetcoeff(void);
void     solsy(double *y);
void     endstoda(void);
void     orderswitch(double *rhup, double dsm, double *pdh, double *rh, int *orderflag);
void     intdy(double t, int k, double *dky, int *iflag);
void     corfailure(double *told, double *rh, int *ncf, int *corflag);
void     methodswitch(double dsm, double pnorm, double *pdh, double *rh);
void     cfode(int meth);
void     scaleh(double *rh, double *pdh);
double   fnorm(int n, double **a, double *w);
double   vmnorm(int n, double *v, double *w);

/* Terminate lsoda due to illegal input. */
void terminate(int *istate);

/* Terminate lsoda due to various error conditions. */
void terminate2(double *y, double *t);

/*
   The following block handles all successful returns from lsoda.
   If itask != 1, y is loaded from yh and t is set accordingly.
   *Istate is set to 2, the illegal input counter is zeroed, and the
   optional outputs are loaded into the work arrays before returning.
*/

void successreturn(double *y, double *t, int itask, int ihit, double tcrit, int *istate);

/*
c-----------------------------------------------------------------------
c this is the march 30, 1987 version of
c lsoda.. livermore solver for ordinary differential equations, with
c         automatic method switching for stiff and nonstiff problems.
c
c this version is in double precision.
c
c lsoda solves the initial value problem for stiff or nonstiff
c systems of first order ode-s,
c     dy/dt = f(t,y) ,  or, in component form,
c     dy(i)/dt = f(i) = f(i,t,y(1),y(2),...,y(neq)) (i = 1,...,neq).
c
c this a variant version of the lsode package.
c it switches automatically between stiff and nonstiff methods.
c this means that the user does not have to determine whether the
c problem is stiff or not, and the solver will automatically choose the
c appropriate method.  it always starts with the nonstiff method.
c
c authors..
c                linda r. petzold  and  alan c. hindmarsh,
c                computing and mathematics research division, l-316
c                lawrence livermore national laboratory
c                livermore, ca 94550.
c
c references..
c 1.  alan c. hindmarsh,  odepack, a systematized collection of ode
c     solvers, in scientific computing, r. s. stepleman et al. (eds.),
c     north-holland, amsterdam, 1983, pp. 55-64.
c 2.  linda r. petzold, automatic selection of methods for solving
c     stiff and nonstiff systems of ordinary differential equations,
c     siam j. sci. stat. comput. 4 (1983), pp. 136-148.
c-----------------------------------------------------------------------
c summary of usage.
c
c communication between the user and the lsoda package, for normal
c situations, is summarized here.  this summary describes only a subset
c of the full set of options available.  see the full description for
c details, including alternative treatment of the jacobian matrix,
c optional inputs and outputs, nonstandard options, and
c instructions for special situations.  see also the example
c problem (with program and output) following this summary.
c
c a. first provide a subroutine of the form..
c               subroutine f (neq, t, y, ydot)
c               dimension y(neq), ydot(neq)
c which supplies the vector function f by loading ydot(i) with f(i).
c
c b. write a main program which calls subroutine lsoda once for
c each point at which answers are desired.  this should also provide
c for possible use of logical unit 6 for output of error messages
c by lsoda.  on the first call to lsoda, supply arguments as follows..
c f      = name of subroutine for right-hand side vector f.
c          this name must be declared external in calling program.
c neq    = number of first order ode-s.
c y      = array of initial values, of length neq.
c t      = the initial value of the independent variable.
c tout   = first point where output is desired (.ne. t).
c itol   = 1 or 2 according as atol (below) is a scalar or array.
c rtol   = relative tolerance parameter (scalar).
c atol   = absolute tolerance parameter (scalar or array).
c          the estimated local error in y(i) will be controlled so as
c          to be less than
c             ewt(i) = rtol*abs(y(i)) + atol     if itol = 1, or
c             ewt(i) = rtol*abs(y(i)) + atol(i)  if itol = 2.
c          thus the local error test passes if, in each component,
c          either the absolute error is less than atol (or atol(i)),
c          or the relative error is less than rtol.
c          use rtol = 0.0 for pure absolute error control, and
c          use atol = 0.0 (or atol(i) = 0.0) for pure relative error
c          control.  caution.. actual (global) errors may exceed these
c          local tolerances, so choose them conservatively.
c itask  = 1 for normal computation of output values of y at t = tout.
c istate = integer flag (input and output).  set istate = 1.
c iopt   = 0 to indicate no optional inputs used.
c rwork  = real work array of length at least..
c             22 + neq * max(16, neq + 9).
c          see also paragraph e below.
c lrw    = declared length of rwork (in user-s dimension).
c iwork  = integer work array of length at least  20 + neq.
c liw    = declared length of iwork (in user-s dimension).
c jac    = name of subroutine for jacobian matrix.
c          use a dummy name.  see also paragraph e below.
c jt     = jacobian type indicator.  set jt = 2.
c          see also paragraph e below.
c note that the main program must declare arrays y, rwork, iwork,
c and possibly atol.
c
c c. the output from the first call (or any call) is..
c      y = array of computed values of y(t) vector.
c      t = corresponding value of independent variable (normally tout).
c istate = 2  if lsoda was successful, negative otherwise.
c          -1 means excess work done on this call (perhaps wrong jt).
c          -2 means excess accuracy requested (tolerances too small).
c          -3 means illegal input detected (see printed message).
c          -4 means repeated error test failures (check all inputs).
c          -5 means repeated convergence failures (perhaps bad jacobian
c             supplied or wrong choice of jt or tolerances).
c          -6 means error weight became zero during problem. (solution
c             component i vanished, and atol or atol(i) = 0.)
c          -7 means work space insufficient to finish (see messages).
c
c d. to continue the integration after a successful return, simply
c reset tout and call lsoda again.  no other parameters need be reset.
c
c e. note.. if and when lsoda regards the problem as stiff, and
c switches methods accordingly, it must make use of the neq by neq
c jacobian matrix, j = df/dy.  for the sake of simplicity, the
c inputs to lsoda recommended in paragraph b above cause lsoda to
c treat j as a full matrix, and to approximate it internally by
c difference quotients.  alternatively, j can be treated as a band
c matrix (with great potential reduction in the size of the rwork
c array).  also, in either the full or banded case, the user can supply
c j in closed form, with a routine whose name is passed as the jac
c argument.  these alternatives are described in the paragraphs on
c rwork, jac, and jt in the full description of the call sequence below.
c
c-----------------------------------------------------------------------
*/

void lsoda(class SystEq& f, int neq, double *y, double *t, double tout, int itol, double *rtol, double *atol,
		   int itask, int *istate, int iopt, int jt,
		   int iwork1, int iwork2, int iwork5, int iwork6, int iwork7, int iwork8, int iwork9,
		   double rwork1, double rwork5, double rwork6, double rwork7, void *_data);
/*
void 
lsoda(f, neq, y, t, tout, itol, rtol, atol, itask, istate,
      iopt, jt, iwork1, iwork2, iwork5, iwork6, iwork7, iwork8,
      iwork9, rwork1, rwork5, rwork6, rwork7, _data)
	_lsoda_f        f;
	void           *_data;

	int             neq, itol, itask, *istate, iopt, jt;
	int             iwork1, iwork2, iwork5, iwork6, iwork7, iwork8, iwork9;
	double         *y, *t, tout, *rtol, *atol;
	double          rwork1, rwork5, rwork6, rwork7;
*/
/*
   If the user does not supply any of these values, the calling program
   should initialize those untouched working variables to zero.

   ml = iwork1
   mu = iwork2
   ixpr = iwork5
   mxstep = iwork6
   mxhnil = iwork7
   mxordn = iwork8
   mxords = iwork9

   tcrit = rwork1
   h0 = rwork5
   hmax = rwork6
   hmin = rwork7
*/

void stoda(int neq, double *y, class SystEq& f, void *_data);
/*
   stoda performs one step of the integration of an initial value
   problem for a system of ordinary differential equations.
   Note.. stoda is independent of the value of the iteration method
   indicator miter, when this is != 0, and hence is independent
   of the type of chord method used, or the Jacobian structure.
   Communication with stoda is done with the following variables:

   jstart = an integer used for input only, with the following
            values and meanings:

               0  perform the first step,
             > 0  take a new step continuing from the last,
              -1  take the next step with a new value of h,
                  n, meth, miter, and/or matrix parameters.
              -2  take the next step with a new value of h,
                  but with other inputs unchanged.

   kflag = a completion code with the following meanings:

             0  the step was successful,
            -1  the requested error could not be achieved,
            -2  corrector convergence could not be achieved,
            -3  fatal error in prja or solsy.

   miter = corrector iteration method:

             0  functional iteration,
            >0  a chord method corresponding to jacobian type jt.

*/

void ewset(int itol, double *rtol, double *atol, double *ycur);

void intdy(double t, int k, double *dky, int *iflag);
/*
   Intdy computes interpolated values of the k-th derivative of the
   dependent variable vector y, and stores it in dky.  This routine
   is called within the package with k = 0 and *t = tout, but may
   also be called by the user for any k up to the current order.
   ( See detailed instructions in the usage documentation. )

   The computed values in dky are gotten by interpolation using the
   Nordsieck history array yh.  This array corresponds uniquely to a
   vector-valued polynomial of degree nqcur or less, and dky is set
   to the k-th derivative of this polynomial at t.
   The formula for dky is

             q
   dky[i] = sum c[k][j] * ( t - tn )^(j-k) * h^(-j) * yh[j+1][i]
            j=k

   where c[k][j] = j*(j-1)*...*(j-k+1), q = nqcur, tn = tcur, h = hcur.
   The quantities nq = nqcur, l = nq+1, n = neq, tn, and h are declared
   static globally.  The above sum is done in reverse order.
   *iflag is returned negative if either k or t is out of bounds.
*/

void cfode(int meth);
/*
   cfode is called by the integrator routine to set coefficients
   needed there.  The coefficients for the current method, as
   given by the value of meth, are set for all orders and saved.
   The maximum order assumed here is 12 if meth = 1 and 5 if meth = 2.
   ( A smaller value of the maximum order is also allowed. )
   cfode is called once at the beginning of the problem, and
   is not called again unless and until meth is changed.

   The elco array contains the basic method coefficients.
   The coefficients el[i], 1 < i < nq+1, for the method of
   order nq are stored in elco[nq][i].  They are given by a generating
   polynomial, i.e.,

      l(x) = el[1] + el[2]*x + ... + el[nq+1]*x^nq.

   For the implicit Adams method, l(x) is given by

      dl/dx = (x+1)*(x+2)*...*(x+nq-1)/factorial(nq-1),   l(-1) = 0.

   For the bdf methods, l(x) is given by

      l(x) = (x+1)*(x+2)*...*(x+nq)/k,

   where   k = factorial(nq)*(1+1/2+...+1/nq).

   The tesco array contains test constants used for the
   local error test and the selection of step size and/or order.
   At order nq, tesco[nq][k] is used for the selection of step
   size at order nq-1 if k = 1, at order nq if k = 2, and at order
   nq+1 if k = 3.
*/

void scaleh(double *rh, double *pdh);

void prja(int neq, double *y, class SystEq& f, void *_data);
/*
   prja is called by stoda to compute and process the matrix
   P = I - h * el[1] * J, where J is an approximation to the Jacobian.
   Here J is computed by finite differencing.
   J, scaled by -h * el[1], is stored in wm.  Then the norm of J ( the
   matrix norm consistent with the weighted max-norm on vectors given
   by vmnorm ) is computed, and J is overwritten by P.  P is then
   subjected to LU decomposition in preparation for later solution
   of linear systems with p as coefficient matrix.  This is done
   by dgefa if miter = 2, and by dgbfa if miter = 5.
*/

double vmnorm(int n, double *v, double *w);
/*
   This function routine computes the weighted max-norm
   of the vector of length n contained in the array v, with weights
   contained in the array w of length n.

   vmnorm = max( i = 1, ..., n ) fabs( v[i] ) * w[i].
*/

double fnorm(int n, double **a, double *w);
/*
   This subroutine computes the norm of a full n by n matrix,
   stored in the array a, that is consistent with the weighted max-norm
   on vectors, with weights stored in the array w.

      fnorm = max(i=1,...,n) ( w[i] * sum(j=1,...,n) fabs( a[i][j] ) / w[j] )
*/

void correction(int neq, double *y, class SystEq& f, int *corflag, double pnorm, double *del, double *delp, double *told,
					   int *ncf, double *rh, int *m, void *_data);
/*
   *corflag = 0 : corrector converged,
              1 : step size to be reduced, redo prediction,
              2 : corrector cannot converge, failure flag.
*/

void corfailure(double *told, double *rh, int *ncf, int *corflag);

void solsy(double *y);
/*
   This routine manages the solution of the linear system arising from
   a chord iteration.  It is called if miter != 0.
   If miter is 2, it calls dgesl to accomplish this.
   If miter is 5, it calls dgbsl.

   y = the right-hand side vector on input, and the solution vector
       on output.
*/

void methodswitch(double dsm, double pnorm, double *pdh, double *rh);
/*
   We are current using an Adams method.  Consider switching to bdf.
   If the current order is greater than 5, assume the problem is
   not stiff, and skip this section.
   If the Lipschitz constant and error estimate are not polluted
   by roundoff, perform the usual test.
   Otherwise, switch to the bdf methods if the last step was
   restricted to insure stability ( irflag = 1 ), and stay with Adams
   method if not.  When switching to bdf with polluted error estimates,
   in the absence of other information, double the step size.

   When the estimates are ok, we make the usual test by computing
   the step size we could have (ideally) used on this step,
   with the current (Adams) method, and also that for the bdf.
   If nq > mxords, we consider changing to order mxords on switching.
   Compare the two step sizes to decide whether to switch.
   The step size advantage must be at least ratio = 5 to switch.
*/

void endstoda();
/*
   This routine returns from stoda to lsoda.  Hence freevectors() is
   not executed.
*/

void orderswitch(double *rhup, double dsm, double *pdh, double *rh, int *orderflag);
/*
   Regardless of the success or failure of the step, factors
   rhdn, rhsm, and rhup are computed, by which h could be multiplied
   at order nq - 1, order nq, or order nq + 1, respectively.
   In the case of a failure, rhup = 0. to avoid an order increase.
   The largest of these is determined and the new order chosen
   accordingly.  If the order is to be increased, we compute one
   additional scaled derivative.

   orderflag = 0  : no change in h or nq,
               1  : change in h but not nq,
               2  : change in both h and nq.
*/

void resetcoeff();
/*
   The el vector and related constants are reset
   whenever the order nq is changed, or at the start of the problem.
*/

void freevectors(void);
/* this function does nothing. */

void _freevectors(void);

/*****************************
 * more convenient interface *
 *****************************/

int n_lsoda(double y[], int n, double *x, double xout, double eps, const double yscal[],class SystEq& devis, void *data);

void n_lsoda_terminate(void);
};
