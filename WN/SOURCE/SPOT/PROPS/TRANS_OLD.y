



%{

/* .......... Declarations C ........*/
#pragma warning (disable:4786)
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"

/***************** Variables *******************/

static struct Token_Domains TD; 
extern char*   Ptr_Cur;
extern int     Lim_Pos;
int trans;
/**********************************************/

/**************** flex functions ************************/
extern int yylex();
extern FILE *yyin;
void yyerror(char *s){  printf("parsing error: %s\n", s); } 
int Transwrap(){ return (1); }
extern void   flush_buffer();
/*******************************************************/


/************** extern functions needed to compute local partitions *******/

extern void* ecalloc();
extern void* emalloc();
extern int GetIndObj();
extern PART_MAT_P GetMatRepOfTransDom();
extern PART_MAT_P GetMatRepOfPartTransDom();
extern TYPE_P** FindDisjParts2();


/************************************************************************/

int EXISTE_TRANS(char* id){
int i=0;
 
    while (i<ntr){
      if (strcmp(tabt[i].trans_name,id)==0) return i;
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
%token  NOT EQ MAX D MCOMP
/*-----------------------------------------les <> types----------------------------------*/

%type <j>  OP
%type <nd> EXPR ATOMIC 

%left IDENT 
%left OR
%left AND 
%left '('  ')'

/*----------------------------------------le debut du parse-----------------------------------*/
%start FORMULES


%%

OP              : EQ {$$=EQOP;}| NOT EQ {$$=NEQOP;}  ;
 
VARS            : ID {
                       int Cl;
                       int v=GetIndObj($1,MTCL,&Cl);
                       if(v==UNKNOWN) {
			 printf("\n The object :%s is not a valid color of the WN to be verfied \n",$1);
                         return(1);
                       }
                       TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
                       TD.Domain=realloc(TD.Domain,(TD.NbCl+1)*sizeof(int));
		       TD.Token[TD.NbCl]=v ; TD.Domain[TD.NbCl]=Cl; TD.NbCl++ ;
                       free($1);
                      } ',' VARS  |

		  IDENT {
			int Cl;
			char* st=(char*)calloc(256,sizeof(char)) ;
			sprintf(st,"%d",$1);
			int v=GetIndObj(st,MTCL,&Cl);
			if(v==UNKNOWN) {
			  printf("\n The object :%s is not a valid color of the WN to be verfied \n",$1); 
			  return(1);
			}
			TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
			TD.Domain=realloc(TD.Domain,(TD.NbCl+1)*sizeof(int));
			TD.Token[TD.NbCl]=v ; TD.Domain[TD.NbCl]=Cl; TD.NbCl++ ;
			free(st);
                       } ',' VARS |

                  ID {
			int Cl;
			int v=GetIndObj($1,MTCL,&Cl);
			if(v==UNKNOWN) {
			  printf("\n The object :%s is not a valid color of WN to be verfied \n",$1); 
			  return(1);
			}
			TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
			TD.Domain=realloc(TD.Domain,(TD.NbCl+1)*sizeof(int));
			TD.Token[TD.NbCl]=v ; TD.Domain[TD.NbCl]=Cl; TD.NbCl++ ;                   
			free($1);
                     } |
		   
		  IDENT {
			int Cl;
			char* st=(char*)calloc(256,sizeof(char)) ;
			sprintf(st,"%d",$1);
			int v=GetIndObj(st,MTCL,&Cl);
			if(v==UNKNOWN) {
			  printf("\n  L'objet :%s n'est pas une couleur valide du réseau vérifié \n",$1); 
			  return(1);
			}
			TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
			TD.Domain=realloc(TD.Domain,(TD.NbCl+1)*sizeof(int));
			TD.Token[TD.NbCl]=v ; TD.Domain[TD.NbCl]=Cl; TD.NbCl++ ;
			free(st);
		  };


COMP             :
                  IDENT {
			 if($1 >= tabt[trans].comp_num)
		           {
			     printf("\n Transition '%d' has Less than '%d' components \n",trans,$1);
			     return(1);	

			   }
			TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
			TD.Token[TD.NbCl]=$1 ; TD.NbCl++ ;
			
                        } ',' COMP |
 
		  IDENT {
	             	 if($1 >= tabt[trans].comp_num)
		           {
                             printf("\n Transition '%d' has Less than '%d' components \n",trans,$1);
			     return(1);	

			   }
			TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
			TD.Token[TD.NbCl]=$1 ; TD.NbCl++ ;
			
		   };


    
STATIC         :  D '(' IDENT ')' {
                        if($3 >= tabt[trans].comp_num)
			   {
			     printf("\n Transition '%d' has not as many components \n",trans);
			     return(1);					  
			   }
			 else
			   {
			   
			     TD.Token=realloc(TD.Token,(TD.NbCl+1)*sizeof(int));
			     TD.Domain=realloc(TD.Domain,(TD.NbCl+1)*sizeof(int));
			     TD.Token[TD.NbCl]=$3 ;  TD.Domain[TD.NbCl]= tabt[trans].dominio[$3-1] ;
			     TD.NbCl++ ;
			   }
                  } ;



FORMULES        : ID { 
                         if( (trans=EXISTE_TRANS($1))==NOT_PRESENT ) 
                           {  
			     printf("\n Transition  '%s' does'nt exist \n",$1);
			     return(1);			     
			   }
			 
                     }
                  '[' ATOMIC ']' ';'
                    {
                      tabt[trans].dyn_guard=$4;
                    } 
                   FORMULES  | ;

ATOMIC          :  EXPR AND EXPR 
                    {
		      $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
		      $$->op=ANDOP;
		      $$->type=NOTYPE;
		      $$->FONCT=NULL;
		      $$->filsg=$1;
		      $$->filsd=$3;
		      
		      if($1->PART_MAT==NULL && $3->PART_MAT==NULL )
			{ $$->PART_MAT=NULL; $$->NbElPM=NULL;}
		      else
			if($1->PART_MAT==NULL && $3->PART_MAT!=NULL)
			  { $$->PART_MAT=$3->PART_MAT; $$->NbElPM=$3->NbElPM; }
			else 
			  if($1->PART_MAT!=NULL && $3->PART_MAT==NULL)
			     { $$->PART_MAT=$1->PART_MAT; $$->NbElPM=$1->NbElPM; }
			  else 
			    {
                               
			      PART_MAT_P var,Part=calloc(2,sizeof(PART_MAT));
                               				
			      Part[0].PART_MAT= $1->PART_MAT;
			      Part[0].NbElPM= $1->NbElPM ;  
			      Part[1].PART_MAT= $3->PART_MAT;
			      Part[1].NbElPM= $3->NbElPM ; 
			      var=FindDisjParts3(Part ,2);  
			      $$->PART_MAT=var->PART_MAT;
			      $$->NbElPM=var->NbElPM;
                              free(var);free(Part);
			      FreePartMAt($1->PART_MAT, $1->NbElPM);
			      FreePartMAt($3->PART_MAT, $3->NbElPM); 
			    }
                    } |
		    EXPR OR EXPR 
                      {
		       $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
		       $$->op=OROP;
		       $$->type=NOTYPE;
		       $$->FONCT=NULL;
		       $$->filsg=$1;
		       $$->filsd=$3;
		       
		       if($1->PART_MAT==NULL && $3->PART_MAT==NULL ) { $$->PART_MAT=NULL; $$->NbElPM=NULL;}
		       else
			 if($1->PART_MAT==NULL && $3->PART_MAT!=NULL)
			   { $$->PART_MAT=$3->PART_MAT; $$->NbElPM=$3->NbElPM; }
			 else 
			   if($1->PART_MAT!=NULL && $3->PART_MAT==NULL)
			     { $$->PART_MAT=$1->PART_MAT;$$->NbElPM=$1->NbElPM;}
			   else 
			     { 
			       PART_MAT_P var,Part=calloc(2,sizeof(PART_MAT));
                               				
			       Part[0].PART_MAT= $1->PART_MAT;
			       Part[0].NbElPM= $1->NbElPM ;  
			       Part[1].PART_MAT= $3->PART_MAT;
			       Part[1].NbElPM= $3->NbElPM ; 
			       var=FindDisjParts3(Part ,2);  
			       $$->PART_MAT=var->PART_MAT;
			       $$->NbElPM=var->NbElPM;
                               free(var);
			       FreePartMAt($1->PART_MAT, $1->NbElPM);
			       FreePartMAt($3->PART_MAT, $3->NbElPM); 
			     }
		       
		     } | EXPR {$$=$1; };
	    
EXPR            : MAX '(' IDENT ')' 
                     {
		       if($3 > tabt[trans].comp_num)
			   {
			     printf("\n Transition '%d' has not as many components \n",trans);
			     return(1);					  
			   } 
		       else
			 { 
			   $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
			   $$->op=$3;
			   $$->type=MAXIMUM ;
			   $$->FONCT=NULL;
			   PART_MAT_P pm=GetMatRepOfPartTransDom(MTCL,tabt[trans].dominio[$3-1]); 
			   $$->PART_MAT=pm->PART_MAT;
			   $$->NbElPM=pm->NbElPM;
			   $$->filsg=$$->filsd=NULL;
			   free(pm);
			 }
                     } |

		    ID OP MCOMP '(' {TD.NbCl=0;TD.Token=NULL;TD.Domain=NULL;} COMP ')' 
                        {
			  
		          if($1 > tabt[trans].comp_num)
			   {
			     printf("\n Transition '%d' has less then '%d' components \n",trans,$1);
			     return(1);					  
			   }
 
                           int i,cl =tabt[trans].dominio[$1-1];
		           for(i=0;i<TD.NbCl;i++)
                            if(tabt[trans].dominio[TD.Token[i]-1]!=cl) 
			    {
                             printf("\n incompatible color domain in guard function of transition '%d' \n",trans);
			     return(1);
			    }
			    
			    $$= (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
			    $$->op=$2;
			    $$->type=MAX_COMP ;
			    $$->FONCT=NULL;
			    PART_MAT_P pm=GetMatRepOfPartTransDom(MTCL,tabt[trans].dominio[$1-1]); 
			    $$->PART_MAT=pm->PART_MAT;
			    $$->NbElPM=pm->NbElPM;
			    $$->filsg=$$->filsd=NULL;
                            $$->FONCT=(CToken_p)emalloc(sizeof(CToken));
                            $$->FONCT->mult=$1;
			   ($$->FONCT->tok_dom).Token=TD.Token;     
			   ($$->FONCT->tok_dom).Domain=NULL;
			   ($$->FONCT->tok_dom).NbCl=TD.NbCl;
			    $$->FONCT->next=NULL;
			    free(pm);
			    
                        }|
             	  
                    ID  OP  '{' {TD.NbCl=0;TD.Token=NULL;TD.Domain=NULL;}   VARS '}'
                      {
		        if($1 > tabt[trans].comp_num)
			   {
			     printf("\n Transition '%d' has less than '%d' components \n",trans,$1);
			     return(1);					  
			   } 
                        else
  		   	   { 
			    $$ = (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
			    $$->op=$1;
			    $$->type=INCLUDED ;
			    $$->FONCT=(CToken_p)emalloc(sizeof(CToken));
                            ($$->FONCT->tok_dom).Token=TD.Token;     
			    ($$->FONCT->tok_dom).Domain=TD.Domain;
			    ($$->FONCT->tok_dom).NbCl=TD.NbCl;
			    PART_MAT_P pm=GetMatRepOfTransDom(MTCL,TD); 
			    $$->PART_MAT=pm->PART_MAT;
			    $$->NbElPM=pm->NbElPM;
			    $$->filsg=$$->filsd=NULL;
			    free(pm);
			   }
                       
                      }|
		  
	
                     {TD.NbCl=0;TD.Token=NULL;TD.Domain=NULL;}  
                      STATIC OP STATIC
                        {
		         $$ = (struct TREE_NODE*)emalloc(sizeof(struct TREE_NODE));
			 $$->op=$3;
			 $$->type=DF;
			 $$->PART_MAT=NULL;
			 $$->NbElPM=NULL;
			 $$->FONCT=(CToken_p)emalloc(sizeof(CToken));
			 $$->filsg= $$->filsd=NULL;
			 $$->FONCT->mult=0;
			 ($$->FONCT->tok_dom).Token=TD.Token;     
			 ($$->FONCT->tok_dom).Domain=TD.Domain;
			 ($$->FONCT->tok_dom).NbCl=TD.NbCl;
			 $$->FONCT->next=NULL;
		        }

                
;
%%
  
int guard_parser(FILE* f){

 char c; 
 int i=0; 
 char* ptr; 
 ptr=Ptr_Cur=(char*)malloc(STR_SIZE*sizeof(char));
     
 while((c=fgetc(f))!=EOF)  
   {
     Ptr_Cur[i]=c; 
     i++;
   }
 
   Ptr_Cur[i]='\0';
   Lim_Pos=(int)Ptr_Cur+strlen(Ptr_Cur);   

   
   i=yyparse();
   free(ptr);
  

  return i;
}















