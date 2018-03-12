(*****************************************************************************
*                                                                            *
*                         UNIVERSITA' di TORINO                              *
*                      Dipartimento di Informatica                           *
*                                                                            *
* Program purpose: SOLUTION OF MARKOV MATRIX WITH GAUSS-SEIDEL ALGORITHM.    *
* Version: Berkeley UNIX 4.2 PASCAL                                          *
* Date: July 13, 1987                                                        *
* Programmer: Giovanni Chiola                                                *
* Notes:                                                                     *
* 1. Constant definitions may be changed in order to either allow the        *
*    firing of larger Reachability Graph or reduce the program memory        *
*    requirement.                                                            *
*                                                                            *
*****************************************************************************)
program gspngaussseidel (output,mtrx,mpdf,gparams);

label 100;  (* Program termination on error condition *)

#include "../gsrc/gconst.h"

type  mrkrange= 0..maxmark;
      mtxrange= 0..maxmatrx;
#include "../gsrc/gfiles.h"

var mpdname,mtrname : packed array [1..100] of char;
    gparams : text;
    mtrx    : packed file of filmtrx;    (* The reduced Tangible Markov chain's sparse matrix *)
    mpdf    : packed file of real;       (* Markings' Steady-state Probability Distribution *)
    toptan  : integer;
    entrlst : packed array [mrkrange] of mtxrange;
    noentr  : packed array [mrkrange] of mrkrange;
    mprob   : array [mrkrange] of real;
    rate    : array [mtxrange] of real;
    from    : packed array [mtxrange] of mrkrange;
    eps,err : real;
    exectime,maxcpu,maxiter,noiter,firstmtx : integer;

procedure exit( status : integer ); external;
procedure abort; begin exit(5) end; (* ABORT *)

(* The following procedure initializes the program data structure *)
procedure init;

var alphanum: set of char;
    c       : char;
    i,j,k   : integer;

begin (* init *)
  if argc<2 then begin
      writeln; writeln('ERROR: netname missing !'); abort
    end;
  alphanum:=['a'..'z','A'..'Z','0'..'9','/']; i:=1; argv(1,mpdname);
  repeat
    c:=mpdname[i];
    if c in alphanum then begin
        mtrname[i]:=c; i:=i+1
      end
  until not (c in alphanum);
  mpdname[i]:='.'; mtrname[i]:='.';
  i:=i+1;
  mpdname[i]:='m'; mtrname[i]:='m';
  i:=i+1;
  mpdname[i]:='p'; mtrname[i]:='t';
  i:=i+1;
  mpdname[i]:='d'; mtrname[i]:='r';
  for j:=i+1 to 100 do begin
      mpdname[j]:=' '; mtrname[j]:=' '
    end;
  reset(gparams,'gparams'); readln(gparams,eps);
  readln(gparams,maxiter); readln(gparams,maxcpu);
  reset(mtrx,mtrname);
  firstmtx:=0; toptan:=0;
  while not eof(mtrx) do begin
      toptan:=toptan+1;
(*
writeln('reading marking #', toptan:1 );
*)
      with mtrx^ do begin j:=fm; rate[firstmtx]:=-1.0/rt end; get(mtrx);
(*
writeln('        ', j:1, ' items, sumrate =', rate[firstmtx] );
*)
      entrlst[toptan]:=firstmtx; noentr[toptan]:=j; firstmtx:=firstmtx+j+1;
      if firstmtx>=maxmatrx then begin
	  exectime:=clock; writeln;
	  write('ERROR: too many matrix entries !  Raise constant MAXMATRX.'); abort
	end;
      for k:=entrlst[toptan]+1 to entrlst[toptan]+j do begin
	  with mtrx^ do begin from[k]:=fm; rate[k]:=rt end;
(*
writeln('          reading item #', (k-entrlst[toptan]):1, ', fm=', from[k]:1, ' rt=', rate[k] );
*)
	  get(mtrx)
	end (* for k *)
    end; (* while not eof(mat) *)
  noiter:=0;
  for i:=1 to toptan
    do for j:=entrlst[i]+1 to entrlst[i]+noentr[i] do begin
	  k:=from[j]; rate[j]:=rate[j]*rate[entrlst[k]]
	end;
  reset(mpdf,mpdname); get(mpdf);
  if mpdf^<0.0 then (* Take previous results *)
      for i:=1 to toptan do begin
	  get(mpdf); mprob[i]:=mpdf^/rate[entrlst[i]]
	end
    else (* Uniform probability initialization *)
      for i:=1 to toptan do mprob[i]:=1.0/(toptan);
  exectime:=clock
end; (* init *)

procedure gaussseidel;

var i,j,k,l : integer;
    r,sum   : real;

begin (* gaussseidel *)
  repeat
    noiter:=noiter+1; err:=0.0; sum:=0.0;
    for i:=1 to toptan do begin (* ROW BY COLUMN PRODUCT *)
	r:=0; k:=entrlst[i]+1; l:=k+noentr[i]-1;
	for j:=k to l do r:=r+rate[j]*mprob[from[j]];
	err:=err+abs(r-mprob[i]); mprob[i]:=r; sum:=sum+r
      end; (* for i *)
    (* PROBABILITY NORMALIZATION *) r:=1.0/sum;
    for i:=1 to toptan do mprob[i]:=mprob[i]*r
  until (err<eps) or (noiter>=maxiter) or ((clock-exectime)>maxcpu);
  sum:=0.0;
  for i:=1 to toptan do begin
      r:=mprob[i]*rate[entrlst[i]]; sum:=sum+r; mprob[i]:=r
    end;
  r:=1.0/sum;
  rewrite(mpdf); mpdf^:=-err; put(mpdf); mpdf^:=-1.0; put(mpdf);
  for i:=1 to toptan do begin
      mpdf^:=r*mprob[i]; put(mpdf)
    end; (* for i *)
end; (* gaussseidel *)

begin  (***   MAIN PROGRAM   ***)
  init;
  gaussseidel;
100: (* PROGRAM TERMINATION *)
  exectime:=clock-exectime;
  writeln; writeln('Program GSPNGAUSSSEIDEL terminated.  Elapsed CPU time',exectime:7,' msec.'); writeln
end.
