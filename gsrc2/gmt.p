(*****************************************************************************
*                                                                            *
*                         UNIVERSITA' di TORINO                              *
*                      Dipartimento di Informatica                           *
*                                                                            *
* Program purpose: CONVERSION OF A SYMBOLIC  GSPN's REACHABILITY GRAPH INTO  *
*                  TANGIBLE MARKINGS' REDUCED MARKOV CHAIN SPARSE MATRIX     *
*                  (linked by columns).  REVISED VERSION FOR GreatSPN.       *
* Version: Berkeley UNIX 4.2 PASCAL                                          *
* Date: August 12, 1987                                                      *
* Programmer: Giovanni Chiola                                                *
* Notes:                                                                     *
*    This is an intermediate version adapted to the new "grg.c" reachability *
*    program, under development for GreatSPN version 2.0                     *
*                                                                            *
*****************************************************************************)
program gspnmtrxred (output,net,graph,trs,vrs,mtrx,mpdf);

label 100;  (* Program termination on error condition *)

#include "../gsrc/gconst.h"

type  plarange= 0..maxplace;
      trarange= 0..maxtrans;
      grorange= 0..maxgroup;
      rparange= 0..maxrpar;
      tokrange= 0..maxtoken;
      gphrange= 0..maxgraph;
      mrkrange= 0..maxmark;
      marktyp = array [plarange] of tokrange;
      grphitem= packed record (* 8 bytes *)
		  rp : real;
		  tm : mrkrange;
		  nx : gphrange
		end; (* record *)
#include "../gsrc/gfiles.h"

var mtrname,mpdname,netname,rgrname,trsname,vrsname : packed array [1..100] of char;
    net     : text;                              (* The GSPN description *)
    graph   : file of filgrph;                   (* The Reachability Graph of the GSPN *)
    trs,vrs : packed file of tokrange;           (* The Tangible and Vanishing Reachability Sets *)
    mtrx    : packed file of filmtrx;            (* The reduced Tangible Markov chain's sparse matrix *)
    mpdf    : packed file of real;               (* Tangible Markings' Mean Sojourn Time *)
    mat     : packed file of filmtrx;            (* internal buffer for reduced TMC linked by rows *)
    trakind : packed array [trarange] of grorange;
    rateprob: array [trarange] of real;
    loadtan,loadvan       : boolean;
    rpar    : array [rparange] of real;
    noplace,notrans,nogroup,nompar,norpar,nosubnet : integer;
    currmrk : marktyp;
    firstgrph,lastgrph,i,toptan,lastan,topvan,lastvan : integer;
    entrlst : packed array [mrkrange] of gphrange;
    noentr  : packed array [mrkrange] of mrkrange;
    grpharr : array [gphrange] of grphitem;
    mflags  : packed array [mrkrange] of boolean;
    exectime: integer;

procedure exit( status : integer ); external;
procedure abort; begin exit(2) end; (* ABORT *)

#include "../gmtdef.h"

(* The following function will be passed as actual parameter of the previous one to provide the current marking *)
function givemark ( nopl : integer ) : integer;  begin if nopl>noplace then givemark:=0
							 else givemark:=currmrk[nopl]
						 end; (* givemark *)

(* The following procedure reads the GSPN description from NET file *)
procedure readnet;

var c        : char;
    i,j,maxj,k,l : integer;

procedure skipname;

var stop : boolean;
    j    : integer;

begin (* skipname *)
  if eoln(net) then readln(net);
  repeat read(net,c) until (c<>' ');
  j:=1; stop:=false;
  while (j<=maxstr) and not stop do begin
      if not stop then j:=j+1; read(net,c); stop:=(c=' ')
    end (* while *)
end; (* skipname *)

begin (* readnet *)
  (* SKIP COMMENT *) readln(net); c:=' ';
  repeat
    if eoln(net) then readln(net) else readln(net,c)
  until c='|';
  readln(net,c,nompar,noplace,norpar,notrans,nogroup,nosubnet);
  (* SKIP MARKING PARAMETERS *) for i:=1 to nompar do readln(net);
  (* SKIP PLACES *) for i:=1 to noplace do readln(net);
  (* READ REAL PARAMETERS *) for i:=1 to norpar do begin
      skipname; readln(net,rpar[i])
    end; (* for *)
  (* SKIP GROUPS *) for i:=1 to nogroup do readln(net);
  (* READ TRANSITIONS *) for i:=1 to notrans do begin
      skipname; read(net,rateprob[i],j,trakind[i]);
      (* SKIP INPUT BAG *) readln(net,maxj);
      for j:=1 to maxj do begin
	  readln(net,l,l,k);
	  for l:=1 to k do readln(net)
	end;
      (* SKIP OUTPUT BAG *) readln(net,maxj);
      for j:=1 to maxj do begin
	  readln(net,l,l,k);
	  for l:=1 to k do readln(net)
	end;
      (* SKIP INHIBITION BAG *) readln(net,maxj);
      for j:=1 to maxj do begin
	  readln(net,l,l,k);
	  for l:=1 to k do readln(net)
	end
    end (* for *)
end; (* readnet *)

(* The following procedure initializes the program data structure *)
procedure init;

var alphanum: set of char;
    c       : char;
    i,j     : integer;

begin (* init *)
  exectime:=clock;
  if argc<2 then begin
      writeln; writeln('ERROR: missing parameters !'); abort
    end
  else argv(1,netname);
  alphanum:=['a'..'z','A'..'Z','0'..'9','/']; i:=1;
  repeat
    c:=netname[i];
    if c in alphanum then begin
        mpdname[i]:=c; rgrname[i]:=c; trsname[i]:=c; vrsname[i]:=c; mtrname[i]:=c; i:=i+1
      end
  until not (c in alphanum);
  mpdname[i]:='.'; netname[i]:='.'; rgrname[i]:='.'; trsname[i]:='.'; vrsname[i]:='.'; mtrname[i]:='.';
  i:=i+1;
  mpdname[i]:='m'; netname[i]:='n'; rgrname[i]:='r'; trsname[i]:='t'; vrsname[i]:='v'; mtrname[i]:='m';
  i:=i+1;
  mpdname[i]:='p'; netname[i]:='e'; rgrname[i]:='g'; trsname[i]:='r'; vrsname[i]:='r'; mtrname[i]:='t';
  i:=i+1;
  mpdname[i]:='d'; netname[i]:='t'; rgrname[i]:='r'; trsname[i]:='s'; vrsname[i]:='s'; mtrname[i]:='r';
  for j:=i+1 to 100 do begin
      mpdname[j]:=' '; netname[j]:=' '; rgrname[j]:=' '; trsname[j]:=' '; vrsname[j]:=' '; mtrname[j]:=' '
    end;
  reset(net,netname); readnet; rewrite(mpdf,mpdname); rewrite(mat);
  reset(graph,rgrname); reset(trs,trsname); reset(vrs,vrsname);
  loadtan:=false; loadvan:=false;
  for i:=1 to notrans do if rateprob[i]<=0.0 then begin
      j:=round(-rateprob[i]);
      if (j<=norpar) then begin
	  if j=0 then begin
	      writeln; writeln('ERROR: transition number',i:4,' has undefined rate !'); abort
	    end;
	  rateprob[i]:=rpar[j]
	end (* if rateprob[i] is parametric *)
      else (* marking-dependent rate *)
	if (trakind[i]=0)or(trakind[i]=detraknd) then loadtan:=true else loadvan:=true
    end; (* for i *)
  toptan:=0; lastan:=toptan; topvan:=maxmark+1; lastvan:=topvan;
  for i:=1 to maxmark do begin
      mflags[i]:=false; entrlst[i]:=0; noentr[i]:=0
    end; (* for i *)
  firstgrph:=0; lastgrph:=0;
  exectime:=clock
end; (* init *)

(* The following procedure adds a new sparse matrix entry *)
procedure insertgraph ( frma : integer;
			toma : integer;
			rnum : real );
var i,j,k,l : integer;
    found   : boolean;

function newgraph : integer;

begin (* newgraph *)
  if firstgrph=0 then begin
      if lastgrph>=maxgraph then begin
	  writeln; writeln('ERROR: too many graph items !');
	  writeln('    Raise constant MAXGRAPH.'); abort
	end;
      lastgrph:=lastgrph+1; newgraph:=lastgrph
    end
  else begin
    newgraph:=firstgrph; firstgrph:=grpharr[firstgrph].nx
  end
end; (* newgraph *)

begin (* insertgraph *)
  i:=entrlst[frma]; found:=(i>0);
  if found then begin
      with grpharr[i] do begin
	  j:=nx; k:=tm; if k=toma then rp:=rp+rnum else found:=false
	end;
      if not found then if k>toma then begin
			    l:=newgraph; entrlst[frma]:=l; noentr[frma]:=noentr[frma]+1;
			    with grpharr[l] do begin
				nx:=i; tm:=toma; rp:=rnum
			      end
			  end
			else begin
			  while (j<>0) and (k<toma) do with grpharr[j] do begin
			      k:=tm; found:=(k=toma);
			      if found then rp:=rp+rnum
				else if k<toma then begin i:=j; j:=nx end
			    end; (* with and while *)
			  if not found then begin
			      l:=newgraph; grpharr[i].nx:=l; noentr[frma]:=noentr[frma]+1;
			      with grpharr[l] do begin
				  nx:=j; tm:=toma; rp:=rnum
				end
			    end
			end
    end
  else begin
    i:=newgraph; entrlst[frma]:=i; noentr[frma]:=1;
    with grpharr[i] do begin
	nx:=0; tm:=toma; rp:=rnum
      end
  end
end; (* insertgraph *)

(*  The following procedure converts the symbolic Reachability Graph into the reduced Tangible Markov Chain  *
 *  sparse matrix (linked by rows) and put it into MAT temporary internal file                               *)
procedure reduce;

var i,j,k,prevan : integer;
    loop,uncomp  : packed array [mrkrange] of boolean;
    sumrat       : real;
    vanm,vlp     : boolean;
kk1, kk2 : integer;

procedure updatevanpaths ( mindx : integer;
			   notr  : integer );
var i,j,k,l,m : integer;
    thr,tom   : array [trarange] of integer;
    probs     : array [trarange] of real;
    r         : real;

begin (* updatevanpaths *)
  uncomp[mindx]:=false; loop[mindx]:=false;
  for i:=1 to notr do begin
      with graph^ do begin
(*
writeln('    updatevanpaths thr=',through:1,' nen=',noenabl:1,' tom=',tomark:1,
' van=',vanflg:1,' lp=',loopflg:1);
*)
	  if loopflg then loop[mindx]:=true; j:=through; k:=noenabl; tom[i]:=tomark;
	  if vanflg then begin if tom[i]<topvan then topvan:=tom[i] end
	    else if tom[i]>toptan then toptan:=tom[i]
	end;
      r:=rateprob[j]; thr[i]:=j; get(graph);
      if r<0.0 then r:=mdeetaaar(j,givemark);
      if k>1 then probs[i]:=r*k else probs[i]:=r
    end; (* for i *)
  j:=0; k:=0; l:=1; r:=1.0; vlp:=vlp or loop[mindx];
  for i:=1 to notr do begin (* FIRING PROBABILITY NORMALIZATION *)
      m:=thr[i];
      if trakind[m]=k then r:=r+probs[i]
      else begin
	k:=trakind[m]; j:=j+1; r:=1.0/r;
	for m:=l to (i-1) do probs[m]:=r*probs[m];
	r:=probs[i]; l:=i
      end
    end; (* for i *)
  r:=1.0/r;
  for m:=l to notr do probs[m]:=probs[m]*r;
  if j>1 then begin
      r:=1.0/j; for i:=1 to notr do probs[i]:=probs[i]*r
    end;
  for i:=1 to notr do begin
      j:=tom[i];
      if (j<=toptan) and not loop[mindx] then begin (* MARKING "j" IS TANGIBLE *)
	  insertgraph(mindx,j,probs[i])
	end
      else begin (* MARKING "j" IS VANISHING *)
	  k:=entrlst[j];
	  while k<>0 do with grpharr[k] do begin
	      r:=probs[i]*rp; m:=tm; insertgraph(mindx,m,r);
	      if (m>=topvan)and((m<lastvan)or uncomp[m]) then begin
		  mflags[m]:=true; uncomp[mindx]:=true
		end;
	      k:=nx
	    end (* while k *)
      end
    end; (* for i *)
end; (* updatevanpaths *)

procedure computerate ( mindx : integer );

var i,j,k,m : integer;
    r       : real;

begin (* computerate *)
  with graph^ do begin
      i:=tomark; j:=through; k:=noenabl;
      if vanflg then begin if i<topvan then topvan:=i end
	else if i>toptan then toptan:=i
(*
;
writeln('  computerate thr=',through:1,' nen=',noenabl:1,' tom=',tomark:1,
' van=',vanflg:1,' lp=',loopflg:1);
*)
    end;
  r:=rateprob[j];
  if r<0.0 then r:=mdeetaaar(j,givemark);
  if k>1 then r:=r*k;
  if i<=toptan then (* MARKING "i" IS TANGIBLE *) begin
(*
writeln('      ok tangible');
*)
      if i<>mindx then insertgraph(mindx,i,r)
    end
  else begin (* MARKING "i" IS VANISHING *)
    k:=entrlst[i];
    while k<>0 do with grpharr[k] do begin
	m:=tm; if m<>mindx then insertgraph(mindx,m,r*rp); k:=nx
(*
;
writeln('      reducing to tangible ', m:1 );
*)
      end (* while k *)
  end
end; (* computerate *)

begin (* reduce *)
  prevan:=maxmark; vlp:=false;
  while not eof(graph) do begin
      with graph^ do begin i:=tomark; j:=through; vanm:=vanflg
(*
;
writeln('thr=',through:1,' nen=',noenabl:1,' tom=',tomark:1,
' van=',vanflg:1,' lp=',loopflg:1);
*)
      end;
      get(graph);
      if vanm then begin
	  if loadvan then for k:=1 to noplace do begin
			      currmrk[k]:=vrs^; get(vrs)
			    end; (* for *)
	  lastvan:=i;
	  if topvan>i then topvan:=i;
	end
      else begin
	if loadtan then for k:=1 to noplace do begin
			    currmrk[k]:=trs^; get(trs)
			  end; (* for *)
	lastan:=i; if lastan>toptan then toptan:=lastan
      end;
      if vanm then updatevanpaths(i,j)
      else begin
	prevan:=topvan-1;
	for k:=1 to j do begin
	    computerate(i); get(graph)
	  end; (* for k *)
	with mat^ do begin fm:=i; rt:=noentr[i] end; put(mat);
kk1 := noentr[i]; kk2 := 0;
	k:=entrlst[i]; entrlst[i]:=0; noentr[i]:=0; sumrat:=0.0;
	while k<>0 do begin
kk2 := kk2+1;
	    j:=k;
	    with grpharr[j] do begin
		k:=nx; nx:=firstgrph; sumrat:=sumrat+rp;
		with mat^ do begin fm:=tm; rt:=rp end
	      end;
	    put(mat); firstgrph:=j
	  end; (* while k *)
if kk1 <> kk2 then begin
writeln;writeln('error kk1=',kk1:1,' kk2=',kk2:1);
abort;
end;
	mpdf^:=sumrat; put(mpdf)
      end
    end; (* while not eof(graph) *)
  for i:=topvan to maxmark do begin
      k:=entrlst[i];
      while k<>0 do begin
	  j:=k;
	  with grpharr[j] do begin
	      k:=nx; nx:=firstgrph
	    end;
	  firstgrph:=j
	end (* while k *)
    end (* for i *)
end; (* reduce *)

(*  The following procedure transposes the sparse matrix contained into MAT file,    *
 *  tests whether the initial marking is Recurrent or Transient and puts the matrix  *
 *  (linked by columns) into the output file MTRX                                    *)
procedure transpose;

var goon              : boolean;
    i,j,k,l,noreachm0 : integer;
    r                 : real;
kk1, kk2 : integer;

begin (* transpose *)
  mflags[1]:=true; noreachm0:=1;
(*
writeln('  toptan=',toptan);
*)
  while not eof(mat) do begin
      with mat^ do begin i:=fm; j:=round(rt) end; get(mat);
(*
writeln('  from=',i:1,' num=',j:1);
*)
      for k:=1 to j do begin
	  with mat^ do begin l:=fm; insertgraph(l,i,rt)
(*
;
writeln('    to=',l:1,' rate=',rt);
*)
	  end;
	  if not mflags[i] then if mflags[l] then begin
				    mflags[i]:=true; noreachm0:=noreachm0+1
				  end;
	  get(mat)
	end (* for k *)
    end; (* while not eof(mat) *)
(*
writeln('  mat read');
*)
  goon:=(noreachm0<toptan);
  while goon do begin
      goon:=false;
      for i:=2 to toptan do if mflags[i] then begin
	      j:=entrlst[i];
kk1 := noentr[i]; kk2 := 0;
	      while j<>0  do with grpharr[j] do begin
kk2 := kk2+1;
		  k:=tm;
		  if not mflags[k] then begin
		      goon:=true; mflags[k]:=true; noreachm0:=noreachm0+1
		    end;
                  j:=nx
		end (* while j *)
;
if kk1 <> kk2 then begin
writeln;writeln('error in marking #', i:1, '  kk1=',kk1:1,' kk2=',kk2:1);
abort;
end;
	    end (* if mflags[i] *)
    end; (* while goon *)
(*
writeln('  reachm0=',noreachm0);
*)
  if noreachm0<toptan then begin
      writeln; writeln('WARNING: the initial marking is transient !')
    end;
  reset(mpdf,mpdname);
  for i:=1 to toptan do begin
      r:=mpdf^; get(mpdf);
      with mtrx^ do begin fm:=noentr[i]; rt:=-r end;
kk1 := noentr[i]; kk2 := 0;
(*
writeln('writing marking #', i:1, ' noentr=', noentr[i]:1, ' rt=', -r );
*)
      put(mtrx); j:=entrlst[i];
      while j<>0 do with grpharr[j] do begin
kk2 := kk2+1;
(*
writeln('        item ', j:1, ' tm=', tm:1, ' rp=', rp );
*)
	  with mtrx^ do begin fm:=tm; rt:=rp end; put(mtrx); j:=nx
	end (* while j *)
;
if kk1 <> kk2 then begin
writeln;writeln('error kk1=',kk1:1,' kk2=',kk2:1);
abort;
end;
    end (* for i *)
end; (* transpose *)

begin  (***   MAIN PROGRAM   ***)
  init;
(*
writeln('Start of reduce');
*)
  reduce;
(*
writeln('End of reduce');
*)
  reset(mat); rewrite(mtrx,mtrname);
(*
writeln('Start of transpose');
*)
  transpose;
(*
writeln('End of transpose');
*)
  rewrite(mpdf,mpdname); (* MARKING PROBABILITY INITIALIZATION *)
  mpdf^:=0.0; put(mpdf); mpdf^:=0.0; put(mpdf); mpdf^:=1.0; put(mpdf);
  for i:=2 to toptan do begin
      mpdf^:=0.0; put(mpdf)
    end;
100: (* PROGRAM TERMINATION *)
  exectime:=clock-exectime;
  writeln; writeln('Program GSPNMTRXRED terminated.  Elapsed CPU time',exectime:7,' msec.'); writeln
end.
