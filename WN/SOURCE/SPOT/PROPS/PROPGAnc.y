



%{

/* .......... Declarations C ........*/
#pragma warning (disable:4786)

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "STRUCT.h"


int TEST_TYPE;
int ind;
int* arry;
int RESULTAT;
int TABLEAU[nodeenum];
char*   Ptr_Cur=NULL;
int     Lim_Pos=0;

extern void   flush_buffer();
extern int get_obj_id();
extern void* ecalloc();
extern void* emalloc();
extern int yylex();
extern FILE *yyin;
void yyerror(char *s){  printf("parsing error: %s\n", s); } 
int yywrap(){ return (1); }




int EXISTE_PLACE(char* id){
int i=0;
 
    while (i<npl){
      if (strcmp(tabp[i].place_name,id)==0) return i;
       i++;  
     }
    return NOT_PRESENT; 
} 
 


          
%}

/*........... Declarations Yacc .....*/

%union { 	

  int j;
  char* s;
  struct CTOKEN * tk;
  struct TREE_NODE * nd;
}
/*-----------------------------------------les tokens----------------------------------*/

%token <j> IDENT 
%token <s> ID
%token  DL PRS INT EDL EPRS EIN  NOT EQ LT GT OR AND DECOL MINUS 
/*-----------------------------------------les <>types----------------------------------*/
%type <s>  DECLARATIONS VARS  PROG FORMULES  INITS PDECLS PINITS PCORP
%type <tk> CONST FONC
%type <j>  PROJ OP
%type <nd> EXPR1 EXPR2 EXPR3 ATOMIC

%left IDENT 
%left OR
%left AND 
%left '('  ')'
/*----------------------------------------le debut du parse-----------------------------------*/
%start PROG


%%

OP              : EQ {$$=EQOP;}| NOT EQ {$$=NEQOP;}  | LT{} | LT EQ{$$=LQOP;}  | NOT LT{} | NOT LT EQ{$$=NLQOP;} | GT {}| GT EQ{$$=GQOP;} | NOT GT{} | NOT GT EQ{$$=NGQOP;} ;

VARS            : ID {
                       int v=GET_IND_OBJ($1);
                       if(v==UNKNOWN) {
			 printf("\nL'objet :%s n'existe pas dans les classes du réseau\n",$1); 
                         free(arry);
                         exit(1);
                       }
                       arry=realloc(arry,(ind+1)*sizeof(int));
                       arry[ind]=v; ind++;
                
                      } ',' VARS  |
                  ID {
                       int v=GET_IND_OBJ($1);
                       if(v==UNKNOWN) {
			 printf("\nL'objet :%s n'existe pas dans les classes du réseau\n",$1); 
                         free(arry);
                         exit(1);
                       }
                       arry=realloc(arry,(ind+1)*sizeof(int));
                       arry[ind]=v; ind++;      
                     };

PERS            : IDENT {
			  arry=realloc(arry,(ind+1)*sizeof(int));
                          arry[ind]=$1; ind++;
                        } ','  PERS |
                  MINUS { 
			  arry=realloc(arry,(ind+1)*sizeof(int));
                          arry[ind]=0 ; ind++; } ','  PERS |
                  IDENT { 
			  arry=realloc(arry,(ind+1)*sizeof(int));
                          arry[ind]=$1; ind++;
			} ;

PROJ            :  {ind=0;arry=NULL;} LT PERS GT '('ID')' { int i;
                                                            if ((i=EXISTE_PLACE($6))==NOT_PRESENT) {
                                                               printf("\nLa place :%s n'existe pas dans le réseau vérifier\n", $6);
			                                       exit(1);
							    }
							    $$=i;
                                                          };

CONST           : CONST OR CONST      { $1->next=$3; $$=$1; } |
                  {ind=0;arry=NULL;}   IDENT LT VARS GT {
                                                         $$= (Token_p)emalloc(sizeof(struct COLOURED_TOKEN));
							 $$->molt=$2;
							 $$->id=arry;     
							 $$->next=NULL;
		                       } |
                  {ind=0;arry=NULL;}   LT VARS GT {
		                                   $$= (Token_p)emalloc(sizeof(struct COLOURED_TOKEN));
						   $$->molt=1;
						   $$->id=arry;
						   $$->next=NULL;
		                       }         ; 

FONC            : ID {
		      int i;
		      if ((i=EXISTE_PLACE($1))==NOT_PRESENT) {
			   printf("\nLa place :%s n'existe pas dans le réseau vérifier\n", $1);
			   exit(1);
		       }
                        $$= (Token_p)emalloc(sizeof(struct COLOURED_TOKEN));
		        $$->molt=i;
			$$->id=NULL;     
			$$->next=NULL;
                     } |

                  PROJ {
  
                        $$=(Token_p)emalloc(sizeof(struct COLOURED_TOKEN));
		        $$->molt=$1;
			$$->id=arry;     
			$$->next=NULL; 
		  };
                  
PROG            : PDECLS  PINITS   PCORP;

PDECLS          : DL   DECLARATIONS  EDL  {};
PINITS          : INT   INITS        EIN  {};
PCORP           : PRS  FORMULES     EPRS {};

DECLARATIONS    : VARS ':' ID ';' DECLARATIONS  | {}  ;


INITS           : ID EQ '{' VARS '}' ';'  INITS | {} ;  

FORMULES        : ID ':' ATOMIC ';' FORMULES    | {} ;

ATOMIC          : EXPR1 OP EXPR2  {}|
                  EXPR3 OP EXPR3  {     $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
                                        $$->op=$2;
                                        $$->type=NOTYPE;
					$$->FONCT=NULL;
					$$->filsg=$1;
					$$->filsd=$3;
		  }; 

EXPR1           : DECOL '('FONC ')' {}|
                 '(' EXPR1 ')'      {}|
                  EXPR1 OR  EXPR1   {};

EXPR2           : DECOL '('FONC ')' {}|
                  IDENT             {}|  
                 '(' EXPR2 ')'      {}|
                  EXPR2 OR  EXPR2   {};

EXPR3           : FONC              {
		                        $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
                                        $$->op=NOP;
                                        $$->type=FUN;
					$$->FONCT=$1;
					$$->filsg=$$->filsd=NULL;
                                     }|
                  CONST             {
                                       $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
                                       $$->op=NOP;
                                       $$->type=TOK;
				       $$->FONCT=$1;
				       $$->filsg=$$->filsd=NULL; 
		                    }|
                 '(' EXPR3 ')'      {$$=$2; }|
                  EXPR3 AND EXPR3   {
		                        $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
                                        $$->op=ANDOP;
                                        $$->type=NOTYPE;
					$$->FONCT=NULL;
					$$->filsg=$1;
					$$->filsd=$3;
		                    }|
                  EXPR3 OR  EXPR3   {
		                        $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
                                        $$->op=OROP;
                                        $$->type=NOTYPE;
					$$->FONCT=NULL;
					$$->filsg=$1;
					$$->filsd=$3;
		                    };
                  
%%

int main(){

char c;
FILE* f;
int i=0;

   Ptr_Cur=(char*)malloc(STR_SIZE*sizeof(char));
   f=fopen("prop.txt","r+");    
  
   while((c=fgetc(f))!=EOF)  
     {
       Ptr_Cur[i]=c; 
       i++;
     }
   Lim_Pos=(int)Ptr_Cur+strlen(Ptr_Cur);   

   yyparse();
   free(Ptr_Cur);
   flush_buffer();

}















