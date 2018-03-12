/* Declarations */

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
  
int yylex();
void yyerror(const char *);

  struct Desc{
    char *wholetag;             // whole tag;
    char *tag;                  // tag of transition (first part);
    char **labels;              // labels of transition;
    int nl;                     // number of labels;
    char *end;                  // string to distinguish;
    struct Desc *next;
  };

  struct RestSet{
    char **labels;     // Labels that are synchronized
    int nl;            // number of labels
  };

  int istrrest;

  extern int Tr_or_Pl;                    // Tr. or Pl under parsing
  extern int Tr_Rest;                     // is there restriction on labels of tr-s
  extern int Pl_Rest;                     // is there restriction on labels of pl-s
  extern struct RestSet PlaceRest;        // Labels of places to be sybchronised
  extern struct RestSet TransRest;        // Labels of transitions to be sybchronised
  extern struct Desc *PDesc;
  extern int Restriction;
  extern char *rlabels[20];

  void Copy_Name(char *str)
    {
      PDesc->tag=(char *)malloc(sizeof(char)*(strlen(str)+1));
      strcpy(PDesc->tag,str);
    }
  
  void Copy_End(char *str)
    {
      PDesc->end=(char *)malloc(sizeof(char)*(strlen(str)+1));
      strcpy(PDesc->end,str);
    }

  void Copy_Label(char *str)
    {
      int rest,ii;
      int isrest;
      struct RestSet *R;

      isrest=0;
      R=&TransRest;
      if(Tr_or_Pl==0)
	{
	  isrest=0;
	  R=&PlaceRest;
	}

      if(!isrest) { rest=0;}
      else
	{
	  rest=1;
	  for(ii=0;ii<R->nl;ii++)
	    if(strcmp(R->labels[ii],str)==0) rest=0;
	}
      if(!rest)
	{
	  if(PDesc->nl==0)
	    {PDesc->labels=(char **)malloc(sizeof(char *)*10);}
	  PDesc->labels[PDesc->nl]=(char *)malloc(sizeof(char)*(strlen(str)+1));
	  strcpy(PDesc->labels[PDesc->nl],str);
	  PDesc->nl++;
	}
    }

  void Fill_Rest(char *str)
    {
      struct RestSet *R;

      if(istrrest) { R=&TransRest; }
      else { R=&PlaceRest; }
      if(R->nl==0)
	{R->labels=(char **)malloc(sizeof(char *)*100);}
      R->labels[R->nl]=(char *)malloc(sizeof(char)*(strlen(str)+1));
      strcpy(R->labels[R->nl],str);
      R->nl++; 
    }
%}

%union {
  char *SVal; }

%token <SVal> ID
%token BAR
%token NEWLINE
%token B_WAVE
%token E_WAVE
%token EQUAL
%token PL
%token TR

%type <SVal> BAR

%%
string    :  fulltag
          |  set_of_labels
          ;
fulltag   :  ID BAR labels BAR BAR ID {Copy_Name($1); Copy_End($6);}
          |  BAR labels BAR BAR ID {Copy_End($5);}
          |  BAR labels
          |  BAR labels BAR BAR
          |  ID {Copy_Name($1);}
          |  ID BAR {Copy_Name($1);}
          |  ID BAR labels {Copy_Name($1);}
          |  ID BAR BAR ID {Copy_Name($1);Copy_End($4);}
          ;
labels    :  label
          |  labels BAR label
          ;
label     :  ID {Copy_Label($1);}
          ;
set_of_labels :  TR {istrrest=1; Tr_Rest=1;} EQUAL B_WAVE rlabels E_WAVE
              |  PL {istrrest=0; Pl_Rest=1;} EQUAL B_WAVE rlabels E_WAVE
              ;
rlabels       :  ;
              |  rlabel
              |  rlabels BAR rlabel
              ;
rlabel        :  ID {Fill_Rest($1);}
              ;
%%










































