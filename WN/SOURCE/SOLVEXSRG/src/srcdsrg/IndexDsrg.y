%{

/* .......... Declarations C ........*/
#pragma warning (disable:4786)
#include "const.h"
#include "SCONSSPOT.h"
#include "struct.h"
#include "SSTRUCTSPOT.h"
#include "var_ext.h"
#include "SVAR_EXTSPOT.h"
#include "macros.h"

/***************** Variables *******************/

struct Token_Domains TD; 
char*   Ptr_Cur_index=NULL;
int     Lim_Pos_index=0;
int     ind=0;
/**********************************************/

/**************** flex functions ************************/
extern int yylex();
extern FILE *yyin;
void yyerror(char *s){  printf("parsing error: %s\n", s); } 
int Indexwrap(){ return (1); }
extern void   flush_buffer_index();
/*******************************************************/


/************** extern functions needed to compute local partitions *******/
extern int get_obj_id();
extern void* ecalloc();
extern void* emalloc();
extern int initialize ();
extern int GetIndObj();
extern  TYPE_P** ComputeLocalPartitions();
extern  TYPE_P** FindDisjParts2();
extern PART_MAT_P FindDisjParts3();
extern PART_MAT_P GetMatRepOfTransDom(); 
extern char*** GetMatRepOfClObj();
extern int initialize () ;
extern void Afficher();
extern int EXISTE_PLACE();
extern int EXISTE_TRANS();
extern int EXISTE_FUNC(int t, char* f);
/************************************************************************/

int
error_color(char* s){
  printf("\n The object %s is not a valid color of the treated WN  \n",s);
  return(1); 
}
int 
error_dec(char* s){
  printf("\n The decolorisation function is not ");
  printf(" allowed for the place '%s'  \n ",s);
  return(1);
} 
int
error_proj(char* s){
  printf("\n The projection function is not ");
  printf(" compatible with the place's '%s' color domain \n ",s);
  return (1);
} 
int 
error_place(char* s){
  printf("\n Place %s does'nt exist in the treated WN \n", s);
  return(1);
}  
int 
error_trans(char* s){
  printf("\n Transition %s does'nt exist in the treated WN \n", s);
  return(1);
} 
int
error_exp(){
  printf("\n Some Sub-expressions's Color domains are incompatible\n");
  return(1);     
} 

int 
error_trans_fun(char* tr, char* fc){
  printf("\n Transition %s has no function named %s \n",tr,fc);
  return(1);	  
} 
                
%}

/*........... Declarations Yacc .....*/

%union { 	
  int j;
  char* s;
  struct CTOKEN * tk;
  struct TREE_NODE * nd;
  struct Perf_Index* Pd;
}
/*----------------------------------les tokens-------------------------*/

%token <j> IDENT 
%token <s> ID
%token  EQ LT GT MINUS NBORDTOK  NBEVENT IDEN
/*---------------------------------les <> types------------------------*/
%type <tk> PROJ
%type <nd> CONDP CONDT CONST PROJ1
%type <j>  OP OPERP  OPERT  
%type <Pd> ATOMIC 
%left IDENT 
%left OR
%left AND 
%left '('  ')'

/*------------------------------le debut du parse---------------------*/
%start FORMULES


%%

OP     : EQ {$$=EQOP;}    | LT{$$=LOP;} |   GT {$$=GOP;}    ;

OPERP  : NBORDTOK {$$=NBTOKEN; } ;  
OPERT  : NBEVENT {$$=NBEVENT; };
VARS   : ID {
            int Cl;
	    int v=GetIndObj($1,MTCL,&Cl);
	    if(v==UNKNOWN) error_color($1); 
	    TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
	    TD.Domain=realloc(TD.Domain,(TD.NbCl+1)*sizeof(int));
	    TD.Token[TD.NbCl]=v ; TD.Domain[TD.NbCl]=Cl; TD.NbCl++ ;
	    free($1);
         } ',' VARS  |
	 ID {
	   int Cl;
	   int v=GetIndObj($1,MTCL,&Cl);

	   if(v==UNKNOWN) 
	     error_color($1);  

	   TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
	   TD.Domain=realloc(TD.Domain,(TD.NbCl+1)*sizeof(int));
	   TD.Token[TD.NbCl]=v ; TD.Domain[TD.NbCl]=Cl; TD.NbCl++ ; 
	   free($1);
	 };

CONST : {TD.NbCl=0;TD.Token=NULL;TD.Domain=NULL;}  
        LT VARS GT {      
	CToken_p pt =(CToken_p)emalloc(sizeof(CToken));
	pt->mult=1;
	(pt->tok_dom).Token=TD.Token;     
	(pt->tok_dom).Domain=TD.Domain;
	(pt->tok_dom).NbCl=TD.NbCl;
	pt->next=NULL;
	$$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
	$$->op=NOP;
	$$->type=TOK;
	$$->FONCT=pt;
	$$->filsg=$$->filsd=NULL; 
	} 
                  ; 
PROJECTING : IDEN  {
               TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
 	       TD.Token[TD.NbCl]=PROJECT ; TD.NbCl++ ;   
             }  |
             MINUS { 
	       TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
	       TD.Token[TD.NbCl]=ELIMINATE ; TD.NbCl++ ;  
	     } ;

PERS  : PROJECTING ',' PERS  | PROJECTING  ;

PROJ  : {TD.NbCl=0;TD.Token=NULL;TD.Domain=NULL; }
        '[' PERS ']' {  
        if(TD.NbCl==1 && TD.Token[0]==ELIMINATE) 
	  error_dec(tabp[ind].place_name);
	if(TD.NbCl!=tabp[ind].comp_num)
	  error_proj(tabp[ind].place_name);
	 
	$$=(CToken_p)emalloc(sizeof(CToken));
	$$->mult=ind;
	($$->tok_dom).Token=TD.Token;  
	($$->tok_dom).Domain=NULL;
	int i;   
	for(i=0,($$->tok_dom).NbCl=0;i<TD.NbCl;i++)
	  if(($$->tok_dom).Token[i]==PROJECT) {
	    ($$->tok_dom).Domain=realloc(($$->tok_dom).Domain,
					 ((($$->tok_dom).NbCl)+1)*sizeof(int));
	    ($$->tok_dom).Domain[ ($$->tok_dom).NbCl]=tabp[$$->mult].dominio[i]; 
	    ($$->tok_dom).NbCl++;
	  }  
	$$->next=NULL; 	     
       };

PROJ1 :  PROJ {
         $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
	 $$->op=NOP;
	 $$->type=PRO;
	 $$->FONCT=$1;
	 $$->PART_MAT=NULL;
	 $$->NbElPM=NULL;
	 $$->filsg=$$->filsd=NULL;   
         
        };
FORMULES :  ATOMIC { 
              PERFINDICES= (struct Perf_Index**)realloc
		           (PERFINDICES,(NbPind+1)*sizeof(struct Perf_Index*));
	      PERFINDICES[NbPind]=$1;
	      NbPind ++;
           } ';' 
	   FORMULES  | ;

ATOMIC   :  OPERP '(' ID ')' ':' {
            if ((ind=EXISTE_PLACE($3))==NOT_PRESENT)
	      error_proj($3);	    
	    else {
	      free($3);
	      TD.Token=NULL;
	      TD.Domain=NULL;
	      TD.NbCl=0;
	    }	
           }                 
           CONDP{
	     $$= (struct Perf_Index*)emalloc(sizeof(struct Perf_Index));
	     $$->pt_index=ind;
	     $$->op_type=$1;
	     $$->mean=0;
	     $$->sum=0;
	     $$->type=PL;
	     $$->cond=$7;
	   } | 
	   OPERT '(' ID ')' ':' {
	     if ((ind=EXISTE_TRANS($3))==NOT_PRESENT)
	       error_trans($3);
	     else {
	       free($3);
	       TD.Token=NULL;
	       TD.Domain=NULL;
	       TD.NbCl=0;
	     }    
	   } 
           CONDT {
	     $$= (struct Perf_Index*)emalloc(sizeof(struct Perf_Index));
	     $$->pt_index=ind;
	     $$->op_type=$1;
	     $$->type=TR;
	     $$->mean=0;
	     $$->sum=0;
	     $$->cond=$7;
	   } | 
	   OPERP '(' ID ')'{
	     int i;
	     $$= (struct Perf_Index*)emalloc(sizeof(struct Perf_Index));
	     if ((i=EXISTE_PLACE($3))==NOT_PRESENT)
	       error_place($3);  
	     free($3);
	     $$->mean=0;
	     $$->sum=0;
	     $$->type=PL  ; 
	     $$->pt_index=i;
	     $$->op_type=$1;
	     $$->cond=NULL;
	   } |
	   OPERT '(' ID ')'{
	     int i;  
	     $$= (struct Perf_Index*)emalloc(sizeof(struct Perf_Index));
	     if ((i=EXISTE_TRANS($3))==NOT_PRESENT)
	       error_trans($3);
	     free($3);
	     $$->mean=0; 
	     $$->sum=0;
	     $$->type=TR  ; 
	     $$->pt_index=i;
	     $$->op_type=$1;
	     $$->cond=NULL;
	   } ;
CONDP :  PROJ1 OP CONST {
         int i;
	 $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
	 $$->op=$2;
	 $$->type=NOTYPE;
	 
	 if((($1->FONCT)->tok_dom).NbCl!=(($3->FONCT)->tok_dom).NbCl)
	   error_exp();  
	
	 for( i=0; i<(($3->FONCT)->tok_dom).NbCl;i++)
	   if( (($1->FONCT)->tok_dom).Domain[i]!= 
	       (($3->FONCT)->tok_dom).Domain[i] )
	     error_exp();

	 $$->FONCT=$3->FONCT;
	 $$->filsg=$1;
	 $$->filsd=$3;	 
        } ;

CONDT  : ID  OP  '{' {  TD.NbCl=0;TD.Token=NULL;TD.Domain=NULL;}   VARS '}' {
         int pos;
	 if((pos=EXISTE_FUNC(ind, $1))==NOT_PRESENT )
	   error_trans_fun(tabt[ind].trans_name, $1);
	 else
	   { 
	     $$ = (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
	     $$->op=$2;
	     $$->type=INCLUDED ;
	     $$->FONCT=(CToken_p)emalloc(sizeof(CToken));
	     $$->FONCT->mult=pos+1;
	     ($$->FONCT->tok_dom).Token=TD.Token;     
	     ($$->FONCT->tok_dom).Domain=TD.Domain;
	     ($$->FONCT->tok_dom).NbCl=TD.NbCl;
	     PART_MAT_P pm=GetMatRepOfTransDom(MTCL,TD); 
	     $$->PART_MAT=pm->PART_MAT;
	     $$->NbElPM=pm->NbElPM;
	     $$->filsg=$$->filsd=NULL;
	     free(pm);
	     free($1);
	   }
       }


%%

int perf_index_parser(FILE* f){

char c;
int i=0;
 char* ptr; 
   ptr=Ptr_Cur_index=(char*)malloc(STR_SIZE*sizeof(char));
     
   while((c=fgetc(f))!=EOF)  
     {
       Ptr_Cur_index[i]=c; 
       i++;
     }

   Ptr_Cur_index[i]='\0';
   Lim_Pos_index=(int)Ptr_Cur_index+strlen(Ptr_Cur_index);   

   if(i==0)
     {
       free(ptr);
       printf("\n File of perf. indices is empty \n");
       return 1;
     }
   
   i=yyparse();
   free(ptr);

  return i;
}















