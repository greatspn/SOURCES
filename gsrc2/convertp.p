program ccc(input,output);

#include "../gsrc/gconst.h"

type  plarange= 0..maxplace;
      trarange= 0..maxtrans;
      grorange= 0..maxgroup;
      mparange= 0..maxmpar;
      tokrange= 0..maxtoken;
      gphrange= 0..maxgraph;
      mrkrange= 0..maxmark;
      stackrng= 0..maxstack;
      marking = -maxmpar..maxtoken;

#include "../gsrc/gfiles.h"

	gftype = file of filgrph;

var graph : gftype ;
     ithrough : integer;
     inoenabl : integer;
     ivanflg : integer;
     itomark  : integer;
     netname : packed array [1..100] of char;
     i, j : integer;
     alphanum : packed set of char;

begin
    if argc<2 then begin
	writeln; writeln('ERROR: net name missing !');
      end
    else argv(1,netname);
    alphanum:=['a'..'z','A'..'Z','0'..'9','/']; i:=1;
    repeat
      if netname[i] in alphanum then begin
	  i:=i+1
	end
    until not (netname[i] in alphanum);
    netname[i]:='.';
    i:=i+1;
    netname[i]:='r';
    i:=i+1;
    netname[i]:='g';
    i:=i+1;
    netname[i]:='r';
    for j:=i+1 to 100 do begin
	netname[j]:=' '
      end;
    rewrite(graph,netname);
    while not eof(input) do begin
        readln(ithrough); readln(inoenabl); readln(ivanflg); readln(itomark);
	with graph^ do begin
	    tomark:=itomark; through:=ithrough; vanflg:=(ivanflg<>0);
	    noenabl:=inoenabl; loopflg:=false
	  end;
	put(graph);
      end
end.
