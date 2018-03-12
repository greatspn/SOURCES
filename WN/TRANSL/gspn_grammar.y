%{
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include "../INCLUDE/const.h"
# include "../INCLUDE/struct.h"
# include "../INCLUDE/var_ext.h"
# include "../INCLUDE/service.h"
# include "../INCLUDE/macros.h"

#define yyparse parser
#define yylex jjlex
#define yywrap jjwrap

extern char *jjtext;
int yylex();
void yyerror (char const *s);

extern void check_predicate();
extern void push_expression();
extern void push_predicate();
extern int evaluate_expression();
extern int optimized_set_token_marking();
extern int optimized_nextvec();
extern int set_token_marking();
extern int get_dynamic_from_total();
extern int nextvec();
extern int get_pl_in();
extern int get_index();
extern int get_marking_parameter_offset();
extern int fill_with_elements();
extern int already_exist();
extern int intersection();
extern int check_obj_name();
extern int get_class_card();
extern int get_obj_id();
extern int set_number_of_elements();
extern int get_proj_num();
extern int already_transformed();
extern void *ecalloc();
extern char *emalloc();
extern void emit();
extern void gen();
extern void out_error();
extern void insert_buf();

int ind,i,j,sbc_ind,tag ;
int objid;
int n_sbcl,numobj,count ;
int not_decl,stop,eqfl,relfl,distfl ;
int low_val,enn_sign=1,sign=1,prsign=1;
int proj_num;
int coef_val,card_val;
int pl,cl,disj;
int addm0,totm0;

int parsing_flag = UNKNOWN;
int exp_on = TRUE;

MDrate_p pmd = NULL;
MDrate_p mdptr = NULL;


Pred_p prd_h = NULL;
Pred_p prd_t = NULL;

Expr_p pcode_h = NULL;
Expr_p pcode_t = NULL;

struct MARKBUF * mpt = NULL;

char temp[MAX_BUFFER][MAX_TAG_SIZE];

char class_read[MAX_TAG_SIZE];
char class_name[MAX_TAG_SIZE];
char sbc_name[MAX_TAG_SIZE];
char buffer[MAX_TAG_SIZE];
char error_name[MAX_TAG_SIZE];
%}
%start grammar

%token DSC O US CO S CARET D PD ESCL DISTANCE POINT ID OBRCS CBRCS VBAR
%token MA FU NUM STRING OBRCK CBRCK MINUS PLUS COMMA
%token NE EQ LT GT LE GE OPAR CPAR COLON MOLT DIV 
%token OR AND NOT MD TD WHEN EVER MAXT MINT INV SQRT POW COMB FACT
%token SEMICOLON REALNUM NEWLINE UNDERSCORE
%token DIESIS FLUSH 

%%
grammar   :  MA 
             marking
	   | FU
	     function
	   | MD
	     mdrate
	   | TD
	     transguard
	   ;
mdrate	   : condlist EVER 
	     COLON
	{
	 pmd = (MDrate_p)emalloc(sizeof(struct MD_RATE));
	 pmd->next = mdptr;
	 mdptr = pmd;
	 pmd->guard = NULL;
	 pmd->code = NULL;
	 prd_h = prd_t = NULL;
	 pcode_h = pcode_t = NULL;
	}
	     kexpression SEMICOLON
	{
	 if(exp_on)
          {
           emit(TYPEEND,"",0.0,0);
	   pmd->code = pcode_h;
          } 
         else
	  pmd->code = NULL;
	 tabt[nmd].md_rate_val = mdptr;
	 mdptr = NULL;
	}
           |
            EVER
	    COLON
	{
	 pmd=(MDrate_p)emalloc(sizeof(struct MD_RATE));
	 pmd->next = mdptr;
	 mdptr = pmd;
	 pmd->guard = NULL;
	 pmd->code = NULL;
	 prd_h = prd_t = NULL;
	 pcode_h = pcode_t = NULL;
	}
	     kexpression SEMICOLON
	{
         if(exp_on)
          {
	   emit(TYPEEND,"",0.0,0);
	   pmd->code = pcode_h;
          }
         else
          pmd->code = NULL;
	 tabt[nmd].md_rate_val = mdptr;
	 mdptr = NULL;
	}
	   ;
condlist   : condlist  WHEN 
	{
	 pmd = (MDrate_p)emalloc(sizeof(struct MD_RATE));
	 pmd->next = mdptr;
	 mdptr = pmd;
	 pmd->guard = NULL;
	 pmd->code = NULL;
	 prd_h = prd_t = NULL;
	 pcode_h = pcode_t = NULL;
	}
	     OBRCK
	     pred
	{
	 gen(TYPEEND,"");
	 pmd->guard = prd_h;
	 check_predicate(pmd->guard,MDRATE);
	}
	     CBRCK
	     COLON kexpression SEMICOLON
	{
         if(exp_on)
          {
	   emit(TYPEEND,"",0.0,0);
	   pmd->code = pcode_h;
          }
         else
          pmd->code = NULL;
	}
	   |  WHEN
	{
	 pmd=(MDrate_p)emalloc(sizeof(struct MD_RATE));
	 pmd->next=mdptr;
	 mdptr=pmd;
	 pmd->guard = NULL;
	 pmd->code = NULL;
	 prd_h = prd_t = NULL;
	 pcode_h = pcode_t = NULL;
	}
	     OBRCK
	     pred
	{
	 gen(TYPEEND,"");
	 pmd->guard = prd_h;
	 check_predicate(pmd->guard,MDRATE);
	}
	     CBRCK
	     COLON kexpression SEMICOLON
	{
         if(exp_on)
          {
	   emit(TYPEEND,"",0.0,0);
	   pmd->code = pcode_h;
          }
         else
          pmd->code = NULL;
	}
	   ;
kexpression   :  kexpression plusop vterm
	{
	 emit(sign==1 ? TYPEPLUS : TYPEMINUS,"",0.0,0);
	}
	   | vterm
	   ;
vterm	   : vterm mulop vfatt
	{
	 emit(prsign==1 ? TYPEMOLT : TYPEDIV,"",0.0,0);
	}
	   | vfatt
	   ;
vfatt	   : 
           INV OPAR kexpression CPAR
	{
         if(parse_DEF != MDRATE)
          {
           fprintf(stdout,"Error: INV rule, token %s floating in expression\n",str_in);
           exit(1);
          }
	 emit(TYPEINV,"",0.0,0);
	}
           | SQRT OPAR kexpression CPAR
	{
	 emit(TYPESQRT,"",0.0,0);
	}
           | POW OPAR kexpression  COMMA kexpression CPAR
	{
	 emit(TYPEPOW,"",0.0,0);
	}
           | FACT OPAR kexpression CPAR
        {
         emit(TYPEFACT,"",0.0,0);
        }
           | COMB OPAR kexpression  COMMA kexpression CPAR
        {
         emit(TYPECOMB,"",0.0,0);
        }
	   | OPAR kexpression CPAR
	   | REALNUM
	{
         if(parse_DEF != MDRATE)
          {
           fprintf(stdout,"Error: REALNUM rule token %s floating in expression\n",str_in);
           exit(1);
          }
	 emit(TYPEREALNUM,"",real_val,0);
	}
           | NUM
        {
         emit(TYPENUM,str_in,atof(str_in),str_val);
        }
	   | STRING
	{
	 emit(TYPEPAR,str_in,0.0,0);
	}
           | VBAR STRING
        {
#ifdef GSPN
	 fprintf(stdout,"Coloured marking definition is not allowed in GSPN specification\n");
         exit(1);
#endif
#ifdef SWN
         strcpy(class_name,str_in);
#endif
        }
             POINT STRING
        {
#ifdef GSPN
	 fprintf(stdout,"Colour class cardinality is not allowed in GSPN specification\n");
         exit(1);
#endif
#ifdef SWN
         strcpy(sbc_name,str_in);
#endif
        }
             VBAR
        {
#ifdef GSPN
	 fprintf(stdout,"Colour class cardinality is not allowed in GSPN specification\n");
         exit(1);
#endif
#ifdef SWN
         /* Controllo su esistenza della classe */
	 if((ind=get_index(class_name))==-1)
	  out_error(ERROR_MISSING_CLASS,0,0,0,0,class_name,NULL);
	 else
          {  
           /* Controllo su esistenza sottoclasse */
	   if((sbc_ind=already_exist(sbc_name,ind,STAT))==0)
	    out_error(ERROR_MISSING_STATIC,0,0,0,0,sbc_name,NULL);
	   else 
	    emit(TYPENUM,"",0.0,tabc[ind].sbclist[sbc_ind-1].card);
          }
#endif
        }
           | VBAR STRING VBAR
        {
#ifdef GSPN
	 fprintf(stdout,"Colour class cardinality is not allowed in GSPN specification\n");
         exit(1);
#endif
#ifdef SWN
         /* Controllo su esistenza della classe */
         if((ind=get_index(str_in))==-1)
          {
           /* Controllo su esistenza sottoclasse */
           for(i=0,stop=FALSE;!stop && i<ncl;i++)
            if((ind=already_exist(str_in,i,STAT))!=0) stop=TRUE;
	   if(stop==FALSE)
	    out_error(ERROR_MISSING_CLASS,0,0,0,0,str_in,NULL);
	   else 
	    emit(TYPENUM,"",0.0,tabc[i-1].sbclist[ind-1].card);
           /* Questa cardinalita'e'quella della prima sottoclasse */
           /* trovata con quel nome                               */
          }
         else 
	  emit(TYPENUM,"",0.0,get_class_card(ind));
#endif
        }
	   | MAXT OPAR kexpression COMMA kexpression CPAR
	{
	 emit(TYPEMAX,"",0.0,0);
	}
	   | MINT OPAR kexpression COMMA  kexpression CPAR
	{
	 emit(TYPEMIN,"",0.0,0);
	}
	   | DIESIS STRING
	{
#ifdef GSPN
	 emit(TYPEMARK,str_in,0.0,0);
#endif
#ifdef SWN
	 fprintf(stdout,"Place marking #%s is not allowed in SWN specification\n",str_in);
         exit(1);
#endif
	}
	   ;
plusop	   : PLUS
	{
	 sign = 1;
	}
	   | MINUS
	{
	 sign = -1;
	}
	   ;
mulop	   : MOLT
	{
	 prsign = 1;
	}
	   | DIV
	{
	 prsign = 0;
	}
	   ;
transguard   : OBRCK
	{
	 tabt[ntr].guard = NULL;
	 prd_h = prd_t = NULL;
	}
	     pred
	{
	 gen(TYPEEND,"");
	 tabt[ntr].guard = prd_h;
	}
	     CBRCK opzRESET
           |
             DIESIS
	{
	 tabt[ntr].reset = TRUE;
	}
           |
	   ;
opzRESET   : DIESIS
	{
	 tabt[ntr].reset = TRUE;
	}
           |
           ;
marking : {totm0=addm0=0;enn_sign=1;} opzexpression opzguard | 
                     marking plusop { enn_sign = sign; sign=1;} opzexpression opzguard;
opzexpression: LT kexpression GT
               {
                emit(TYPEEND,"",0.0,0);
                if(parse_DEF == INITIAL_MARKING)
                 {
                  addm0 = evaluate_expression(pcode_h,NULL) * enn_sign;
                  push_expression(pcode_h,pcode_t);
                 }
                else if(parse_DEF==FUNCTION)
                 ptr->expression = pcode_h;
                pcode_h = pcode_t = NULL;
               }
            |
               { 
                if(parse_DEF == INITIAL_MARKING) 
                 addm0 = enn_sign;
               }
            ; 
opzguard   : OBRCK 
	{
	 prd_h = prd_t = NULL;
	}
	     pred
	{
         gen(TYPEEND,"");
         if(parse_DEF==FUNCTION)
          ptr->guard = prd_h;
         else if(parse_DEF==INITIAL_MARKING)
          {
	   addm0 *= verify_predicate_constraint(prd_h,NULL);
           push_predicate(prd_h,prd_t);
	   prd_h = prd_t = NULL;
           totm0 += addm0;
          }
	}
	     CBRCK
           |
          {
           if(parse_DEF==INITIAL_MARKING)
            totm0 += addm0;
          }
	   ;
function : {
            ptr=(Coeff_p)emalloc(sizeof(struct ENN_COEFF));
            enn_sign=1;
            ptr->enn_coef = enn_sign;
	    ptr->guard = NULL;
            ptr->expression = NULL;
	    ptr->next = fun_ptr;/* Settare fun_ptr a NULL prima di parse */
	    fun_ptr = ptr;
           } opzexpression opzguard |
           function plusop 
           {
	    enn_sign = sign;
	    sign=1;
            ptr=(Coeff_p)emalloc(sizeof(struct ENN_COEFF));
            ptr->enn_coef = enn_sign;
	    ptr->guard = NULL;
            ptr->expression = NULL;
	    ptr->next = fun_ptr;/* Settare fun_ptr a NULL prima di parse */
	    fun_ptr = ptr;
           } opzexpression opzguard;
pred	   : pred
	     OR 
	     pterm
        {
	 gen(TYPEOR,"");
	}
	   | pterm
	   ;
pterm	   : pterm 
	     AND 
	     pfatt
	{
	 gen(TYPEAND,"");
	}
	   | pfatt
	   ;
pfatt	   : OPAR 
	     pred
	     CPAR
	   | predop
	     relop
	     predop
	   {
	    gen(relfl," ");
	   }
	   ;
relop      : EQ
        {
	 relfl = TYPEEQ;
	}
           | NE
        {
	 relfl = TYPENE;
	}
	   | LT
        {
	 relfl = TYPELT;
	}
	   | LE
        {
	 relfl = TYPELE;
	}
	   | GT
        {
	 relfl = TYPEGT;
	}
	   | GE
        {
	 relfl = TYPEGE;
	}
	   ;
predop: kexpression 
        {
	 emit(TYPEEND,"",0.0,0);
         gen(TYPENUM,"");
         pcode_h = pcode_t = NULL;
        }
           ; 
%%
//#include "../SUPPORT/lex.c"
void yyerror(char const *s) {
    fprintf(stdout, "Syntax error while parsing\n");
    switch (parsing_flag) {
    /* Tipo di errore */
    case TRANSITION :
        fprintf(stdout, "predicate of transition %s\n", TRANS_NAME(ntr));
        break;
    case PLACE :
        fprintf(stdout, "color domain of place %s\n", PLACE_NAME(npl));
        break;
    case MDRATE :
        fprintf(stdout, "MD rate definition of transition %s\n", TRANS_NAME(nmd));
        break;
    case FUNCTION :
        fprintf(stdout, "color function labeling arc connecting\n");
        fprintf(stdout, "place %s and transition %s\n", PLACE_NAME(pl_ind), TRANS_NAME(ntr));
        break;
    case COLOR_CLASS :
        fprintf(stdout, "color class definition %s\n", error_name);
        break;
    case STATIC_SUBCLASS :
        fprintf(stdout, "static subclass definition %s\n", error_name);
        break;
    case DYNAMIC_SUBCLASS :
        fprintf(stdout, "dynamic subclass definition %s\n", error_name);
        break;
    case INITIAL_MARKING :
        fprintf(stdout, "initial marking definition %s\n", error_name);
        break;
    default:
        break;
    }/* Tipo di errore */
    exit(1);
}
