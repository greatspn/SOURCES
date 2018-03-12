%{
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#include "../INCLUDE/const.h"
# include "../INCLUDE/struct.h"
# include "../INCLUDE/var_ext.h"
# include "../INCLUDE/macros.h"
# include "../INCLUDE/service.h"

#define yyparse parser
#define yylex jjlex
#define yywrap jjwrap

extern char *jjtext;
int yylex();
void yyerror (char const *s);

extern void check_predicate();
extern void push_expression();
extern void push_predicate();
extern double evaluate_expression();
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

int read_domain[MAX_PLACE_DOMAIN];
int base[MAX_PLACE_DOMAIN];
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

#ifdef SWN
Obj_p start_list = NULL;
Obj_p ob_ptr = NULL;
#endif


Pred_p prd_h = NULL;
Pred_p prd_t = NULL;

Expr_p pcode_h = NULL;
Expr_p pcode_t = NULL;

struct MARKBUF * mpt = NULL;

#ifdef SWN
Dyn_p dpt = NULL;
#endif

char temp[MAX_BUFFER][MAX_TAG_SIZE];

char class_read[MAX_TAG_SIZE];
char class_name[MAX_TAG_SIZE];
char sbc_name[MAX_TAG_SIZE];
char buffer[MAX_TAG_SIZE];
char error_name[MAX_TAG_SIZE];
%}
%start grammar

%token DSC O US CO S CARET D POINT ESCL ID OBRCS CBRCS DISTANCE VBAR
%token MA FU NUM STRING OBRCK CBRCK MINUS PLUS COMMA
%token NE EQ LT GT LE GE OPAR CPAR COLON MOLT DIV 
%token OR AND NOT MD PD TD WHEN EVER MAXT MINT INV SQRT POW COMB FACT
%token SEMICOLON REALNUM NEWLINE UNDERSCORE
%token DIESIS FLUSH 

%%
grammar   :  MA 
             marking
           | DSC
	     dynamic_subclass
           | CO
	     colors
	   | FU
	     function
	   | MD
	     mdrate
	   | PD
	     placedom
	   | TD
	     transdom
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
colors	   : 
	{
	 if(parse_DEF == COLOR_CLASS)
          {
	   /* controllo ripetizione nome classi di colori */
	   for(ind=0;ind<ncl;ind++)
            {
             if(strcmp(tabc[ind].col_name,bufname)==0)
	      {/* Errore */
	       out_error(ERROR_CLASS_REDECLARATION,0,0,0,0,bufname,NULL);
	      }/* Errore */
	    }
	   tabc[ncl].col_name = (char *)ecalloc(strlen(bufname)+1,sizeof(char));
	   strcpy(tabc[ncl].col_name,bufname);
	 }
	}
       prefix
	{
	 if(parse_DEF == COLOR_CLASS)
	  tabc[ncl].type=str_val;
	}
       namelist
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   tot_sbc_num+=n_sbcl;
	   tabc[ncl].sbc_num=n_sbcl;
	   tabc[ncl].sbclist=(Static_p)ecalloc(n_sbcl,sizeof(struct STATICSBC));
	   for(ind=0;ind<tabc[ncl].sbc_num;ind++)
	    {
	     tabc[ncl].sbclist[ind].name =
	       (char *)ecalloc(strlen(temp[ind])+1,sizeof(char));
	     strcpy(tabc[ncl].sbclist[ind].name,temp[ind]);
	     tabc[ncl].sbclist[ind].card = 0;
	     tabc[ncl].sbclist[ind].dynlist=NULL;
	    }
	  }
         if(parse_DEF == FALSE || parse_DEF == COLOR_CLASS)
	  ncl++ ;
	}
	   | 
        {
	 parsing_flag = STATIC_SUBCLASS;
        }
	     static
	   ;
namelist   : namelist COMMA STRING
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	  /* controllo ripetizione nome sottoclassi */
	   for(ind=0;ind<n_sbcl;ind++)
            {
             if(strcmp(temp[ind],str_in)==0)
	      {/* Errore */
	       out_error(ERROR_TWICE_THE_SAME_STATIC,0,0,0,0,str_in,NULL);
	      }/* Errore */
            }
	   strcpy(temp[n_sbcl],str_in);
	   n_sbcl++;
	  }
	}
	   | STRING
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   strcpy(temp[0],str_in);
	   n_sbcl=1;
	  }
	}
	   ;
static	   : 
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   for(i=0,stop=FALSE;!stop && i<ncl;i++)
	    if((ind=already_exist(bufname,i,STAT))!=0) stop=TRUE;
	   if(stop==FALSE)
	    {
	     not_decl=TRUE;
	     sbc_pun=(struct SBCBUF*)emalloc(sizeof(struct SBCBUF));
	     strcpy(sbc_pun->name,bufname);
	    }
	   else
	    {
	     tabc[i-1].sbclist[ind-1].name =
	      (char *)ecalloc(strlen(bufname)+1,sizeof(char));
	     strcpy(tabc[i-1].sbclist[ind-1].name,bufname);
	    }
	  }
	}
	   OBRCS objects CBRCS
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   if(not_decl)
	    {
	     sbc_pun->card=numobj;
	     sbc_pun->obj_list=start_list;
	     insert_buf(sbc_pun);
	    }
	   else
	    {
	     if(tabc[i-1].sbclist[ind-1].card==0)
	      {
	       sbc_def_num++;
	       tabc[i-1].sbclist[ind-1].obj_list=start_list;
	       if((disj=intersection(tabc[i-1].sbclist,ind-1,tabc[i-1].sbc_num))<0)
	        {/* Errore */
                 disj++;disj=-disj;
	         out_error(ERROR_INTERSECTION,0,0,i-1,0,GET_STATIC_NAME(i-1,ind-1),GET_STATIC_NAME(i-1,disj));
	        }/* Errore */
	       tabc[i-1].sbclist[ind-1].card=numobj;
	      }
	     else
	      {/* Errore */
	       out_error(ERROR_REDEFINITION,0,0,i-1,ind-1,NULL,NULL);
	      }/* Errore */
	    }
	   start_list=NULL;
	   not_decl=FALSE;
	  }
	}
	  | STRING
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   for(i=0,stop=FALSE;!stop && i<ncl;i++)
	    if((ind=already_exist(bufname,i,STAT))!=0) stop=TRUE;
	   if(stop==FALSE)
	    {
	     not_decl=TRUE;
	     sbc_pun=(struct SBCBUF *)emalloc(sizeof(struct SBCBUF));
	     strcpy(sbc_pun->name,bufname);
	     strcpy(sbc_pun->obj_name,str_in);
	     sbc_pun->obj_list=NULL;
	    }
	   else
	    {
	     tabc[i-1].sbclist[ind-1].name =
	         (char *)ecalloc(strlen(bufname)+1,sizeof(char));
	     tabc[i-1].sbclist[ind-1].obj_name =
	         (char *)ecalloc(strlen(str_in)+1,sizeof(char));
	     strcpy(tabc[i-1].sbclist[ind-1].name,bufname);
	     strcpy(tabc[i-1].sbclist[ind-1].obj_name,str_in);
	     tabc[i-1].sbclist[ind-1].obj_list=NULL;
	    }
	  }
	}
	   OBRCS NUM 
        { 
	 if(parse_DEF == COLOR_CLASS)
	  low_val=str_val; 
	}
	   MINUS NUM CBRCS
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   if(str_val<low_val)
	    {/* Errore */
	     out_error(ERROR_OBJECT_DESCRIPTION,0,0,0,0,bufname,NULL);
	    }/* Errore */
	   if(not_decl)
	    {
	     sbc_pun->card=str_val-low_val+1;
	     sbc_pun->low=low_val;
	     insert_buf(sbc_pun);
	    }
	   else
	    {
	     if(tabc[i-1].sbclist[ind-1].card==0)
	      {
	       sbc_def_num++;
	       tabc[i-1].sbclist[ind-1].card=str_val-low_val+1;
	       tabc[i-1].sbclist[ind-1].low=low_val;
	       if((disj=intersection(tabc[i-1].sbclist,ind-1,tabc[i-1].sbc_num))<0)
	        {/* Errore */
                 disj++;disj=-disj;
	         out_error(ERROR_INTERSECTION,0,0,i-1,0,GET_STATIC_NAME(i-1,ind-1),GET_STATIC_NAME(i-1,disj));
	        }/* Errore */
	      }
	     else
	      {/* Errore */
	       out_error(ERROR_REDEFINITION,0,0,i-1,ind-1,NULL,NULL);
	      }/* Errore */
	    }
	   not_decl=FALSE;
	  }
	}
	   ;
objects    : objects COMMA STRING
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   if(check_obj_name(str_in,start_list)==-1)
	    {/* Errore */
	     out_error(ERROR_OBJECT_REDECLARATION,0,0,0,0,str_in,NULL);
	    }/* Errore */
	   ob_ptr=(Obj_p)emalloc(sizeof(struct OBJ));
	   ob_ptr->nome = (char *)ecalloc(strlen(str_in)+1,sizeof(char));
	   ob_ptr->next=start_list;
	   start_list=ob_ptr;
	   strcpy(ob_ptr->nome,str_in);
	   numobj++;
	  }
	}
	   | STRING
	{
	 if(parse_DEF == COLOR_CLASS)
	  {
	   ob_ptr=(Obj_p)emalloc(sizeof(struct OBJ));
	   ob_ptr->nome = (char *)ecalloc(strlen(str_in)+1,sizeof(char));
	   ob_ptr->next=start_list;
	   start_list=ob_ptr;
	   strcpy(ob_ptr->nome,str_in);
	   numobj=1;
	  }
	}
	   ;
prefix	   : O
	{ 
	 if(parse_DEF == COLOR_CLASS)
	  n_ord++; 
	} 
	   | US 
	{ 
	 if(parse_DEF == COLOR_CLASS)
	  n_unord++;
        }
	   ;
placedom   : placedom COMMA STRING
	{/* Parsificazione del dominio di un posto */
	 if((ind=get_index(str_in))==-1)
	  {/* Errore */
	   out_error(ERROR_UNEXISTING_CLASS,npl,0,0,0,str_in,NULL);
	  }/* Errore */
	 tabp[npl].comp_num++;
	 if(tabp[npl].comp_num > MAX_PLACE_DOMAIN)
	  {/* Errore */
	   out_error(ERROR_LOW_MAX_PLACE,npl,0,0,0,NULL,NULL);
	  }/* Errore */
	 read_domain[tag] = ind;
	 tag++;
	}/* Parsificazione del dominio di un posto */
	   | STRING
	{/* Parsificazione del dominio di un posto */
	 if((ind=get_index(str_in))==-1)
	  {/* Errore */
	   out_error(ERROR_UNEXISTING_CLASS,npl,0,0,0,str_in,NULL);
	  }/* Errore */
	 tag = 0;
	 tabp[npl].comp_num = 1;
	 read_domain[tag] = ind;
	 tag++;
	}/* Parsificazione del dominio di un posto */
	   ;
transdom   : OBRCK
	{
	 tabt[ntr].guard = NULL;
	 prd_h = prd_t = NULL;
	}
	     pred
	{
	 gen(TYPEEND,"");

	 tabt[ntr].guard = prd_h;

#ifdef ESYMBOLIC
	tabt[ntr].trans_type=ASYM_T;
#endif
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

dynamic_subclass :
	{
#ifdef COLOURED
	 fprintf(stdout,"Dynamic subclass definition is skipped \n");
         exit(1);
#endif
#ifdef SYMBOLIC
	 parsing_flag = DYNAMIC_SUBCLASS;
#endif
	}
	     OPAR STRING
	{
#ifdef COLOURED
	 fprintf(stdout,"Dynamic subclass definition is skipped \n");
         exit(1);
#endif
#ifdef SYMBOLIC
	 strcpy(class_read,str_in);
#endif
	}
	     COLON coefficient_type CPAR
	{
#ifdef COLOURED
	 fprintf(stdout,"Dynamic subclass definition is skipped \n");
         exit(1);
#endif
#ifdef SYMBOLIC
	 for(i=0,stop=FALSE;!stop && i<ncl;i++)
	  if((ind=already_exist(class_read,i,STAT))!=0)
	   stop=TRUE;
	 if(!stop)
	  {/* Bufferizzare */
	   mpt=(struct MARKBUF *)emalloc(sizeof(struct MARKBUF));
	   mpt->card=card_val;
	   strcpy(mpt->name,bufname);
	   strcpy(mpt->sbc,class_read);
	   mpt->next=mbuf;
	   mbuf=mpt;
	  }/* Bufferizzare */
	 else
	  {/* Inserimento */
	   if(!already_exist(bufname,i-1,ind-1))
	    {
	     dpt=(Dyn_p)emalloc(sizeof(struct DYNSC));
	     dpt->next=tabc[i-1].sbclist[ind-1].dynlist;
	     dpt->card=card_val;
	     dpt->name=(char  *)ecalloc(strlen(bufname)+1,sizeof(char));
	     strcpy(dpt->name,bufname);
	     tabc[i-1].sbclist[ind-1].dynlist=dpt;
	    }
	   else
	    out_error(ERROR_DYNAMIC_REDECLARATION,0,0,0,0,bufname,NULL);
	  }/* Inserimento */
#endif
	}
	   ;
marking    : objmark
	   | shortmark
	   ;
objmark	   :  coefficient LT oblist GT
	{
	 if(count!=tabp[pl_ind].comp_num-1)
	  out_error(ERROR_LESSER_MARKING_COMPONENTS,pl_ind,0,0,0,NULL,NULL);
	 do
	  {
	   optimized_set_token_marking(cart_place_ptr,coef_val*sign,pl_ind,NULL,TOTAL);
	  }
	 while(optimized_nextvec(cart_place_ptr,tabp[pl_ind].comp_num,base));
	 sign=1;
	}
	   | objmark plusop coefficient LT oblist GT
	{
	 if(count!=tabp[pl_ind].comp_num-1)
	  out_error(ERROR_LESSER_MARKING_COMPONENTS,pl_ind,0,0,0,NULL,NULL);
	 do
	  {
	   optimized_set_token_marking(cart_place_ptr,coef_val*sign,pl_ind,NULL,TOTAL);
	  }
	 while(optimized_nextvec(cart_place_ptr,tabp[pl_ind].comp_num,base));
	 sign=1;
	}
	   ;
oblist	   : oblist COMMA
	{
	 if(count==tabp[pl_ind].comp_num-1)
	   out_error(ERROR_GREATER_MARKING_COMPONENTS,pl_ind,0,0,0,NULL,NULL);
	 count++;
	}
	     obj
	|
	{
	 count=0;
	}
	     obj
	   ;
obj	   : STRING
	{
	 cl=tabp[pl_ind].dominio[count];
	 objid = get_obj_id(str_in,cl);
	 if(objid == UNKNOWN)
	  {/* Errore */
	   out_error(ERROR_UNKNOWN_ELEMENT_IN_MARKING,pl_ind,0,cl,0,str_in,NULL);
	  }/* Errore */
	 cart_place_ptr->up[count] = cart_place_ptr->low[count] = base[count] = objid;
        }
	   | S
	{
	 cl = tabp[pl_ind].dominio[count];
	 cart_place_ptr->low[count]=base[count]=0;
	 cart_place_ptr->up[count]=fill_with_elements(count,cl);
	}
	   | S NUM
	{
	 cl = tabp[pl_ind].dominio[count];
	 if(str_val <1 || str_val >tabc[cl].sbc_num)
	  {/* Errore */
	   out_error(ERROR_WRONG_STATIC_INDEX_IN_MARKING,pl_ind,0,cl,0,NULL,NULL);
	  }/* Errore */
	 str_val--;
	 cart_place_ptr->low[count] = base[count] = GET_STATIC_OFFSET(cl,str_val);
	 cart_place_ptr->up[count] = set_number_of_elements(count,cl,str_val);
	}
	   | S STRING 
	{
	 cl = tabp[pl_ind].dominio[count];
	 if((sbc_ind=already_exist(str_in,cl,STAT))==0)
	  {/* Errore */
	   out_error(ERROR_WRONG_STATIC_NAME_IN_MARKING,pl_ind,0,cl,0,str_in,NULL);
	  }/* Errore */
	 sbc_ind--;
	 cart_place_ptr->low[count] = base[count] = GET_STATIC_OFFSET(cl,sbc_ind);
	 cart_place_ptr->up[count] = set_number_of_elements(count,cl,sbc_ind);
	}
           ;
shortmark  :  coefficient S
	{
	 for(i=0,tag=1;i<tabp[pl_ind].comp_num;i++)
	  {
	   cl = tabp[pl_ind].dominio[i];
	   cart_place_ptr->low[i] = base[i] = 0;
	   cart_place_ptr->up[i] = fill_with_elements(i,cl);
           for(j=0;j<=cart_place_ptr->up[i];j++)
#ifdef SYMBOLIC
#ifdef SIMULATION
            if(IS_UNORDERED(cl))
	     cart_place_ptr->mark[i][j] =  get_dynamic_from_total(cl,j);
	    else
	     cart_place_ptr->mark[i][j] =  j;
#endif
#ifdef REACHABILITY
            if(IS_UNORDERED(cl) || (IS_ORDERED(cl) && GET_STATIC_SUBCLASS(cl)==1))
	     cart_place_ptr->mark[i][j] =  get_dynamic_from_total(cl,j);
	    else
	     cart_place_ptr->mark[i][j] =  j;
#endif
#endif
#ifdef COLOURED
	    cart_place_ptr->mark[i][j] =  j;
#endif
	  }
	 do
          {
	  set_token_marking(cart_place_ptr,coef_val*sign,pl_ind,NULL,TOTAL,UNKNOWN);
          }
	 while(nextvec(cart_place_ptr,tabp[pl_ind].comp_num));
	 sign=1;
	}
	   ;
function   : funlist
	|  coefficient  S 
	{
	 if(IS_NEUTRAL(pl_ind))
	  out_error(ERROR_NEUTRAL_PLACE_AND_FUNCTION,pl_ind,ntr,0,0,NULL,NULL);
	 ptr=(Coeff_p)emalloc(sizeof(struct ENN_COEFF));
	 ptr->enn_coef=coef_val;
	 ptr->guard=NULL;
	 ptr->next=NULL;
	 ptr->coef=(Func_p)ecalloc(tabp[pl_ind].comp_num,sizeof(struct COEFF));
	 for(i=0;i<tabp[pl_ind].comp_num;i++)
	  {
	   ptr->coef[i].xsucc_coef=(int *)ecalloc(3,sizeof(int));
	   cl=tabp[pl_ind].dominio[i];
#ifndef ESYMBOLIC 
	   ptr->coef[i].sbc_coef=(int *)ecalloc(tabc[cl].sbc_num,sizeof(int));
#else 
  	   ptr->coef[i].sbc_coef=(int *)ecalloc( get_max_cardinality(),sizeof(int));
#endif
	   for(sbc_ind=0;sbc_ind<tabc[cl].sbc_num;sbc_ind++)
	    ptr->coef[i].sbc_coef[sbc_ind]=1;
	   ptr->coef[i].xsucc_coef[0]=0;
	   ptr->coef[i].xsucc_coef[1]=0;
	   ptr->coef[i].xsucc_coef[2]=0;
	  }
         fun_ptr=ptr;
	}
	   ;
funlist    : coefficient 
	{
	 if(IS_NEUTRAL(pl_ind))
	  out_error(ERROR_NEUTRAL_PLACE_AND_FUNCTION,pl_ind,ntr,0,0,NULL,NULL);
	 ptr=(Coeff_p)emalloc(sizeof(struct ENN_COEFF));
	 ptr->enn_coef=coef_val * enn_sign;
	 enn_sign=1;
	 ptr->coef=(Func_p)ecalloc(tabp[pl_ind].comp_num ,sizeof(struct COEFF));
	 for(ind=0;ind<tabp[pl_ind].comp_num;ind++)
	  {
	   ptr->coef[ind].xsucc_coef=(int *)ecalloc(3*MAX_COL_RIP,sizeof(int));
#ifndef ESYMBOLIC
	   ptr->coef[ind].sbc_coef=(int *)ecalloc(tabc[tabp[pl_ind].dominio[ind]].sbc_num,sizeof(int));
#else
	  ptr->coef[ind].sbc_coef=(int *)ecalloc( get_max_cardinality(),sizeof(int));
#endif
	  }
	 ptr->next=fun_ptr;/* Settare fun_ptr a NULL prima di parse */
	 fun_ptr=ptr;
	 ptr->guard = NULL;
	}
         opzguard LT funzseq GT
	{
	 if(count!=tabp[pl_ind].comp_num-1)
	  out_error(ERROR_LESSER_FUNCTION_COMPONENTS,pl_ind,ntr,0,0,NULL,NULL);
	}
	   | funlist plusop
	{
	 enn_sign=sign;
	 sign=1;
         pcode_h = pcode_t = NULL;
	}
	      coefficient 
	{
	 if(IS_NEUTRAL(pl_ind))
	  out_error(ERROR_NEUTRAL_PLACE_AND_FUNCTION,pl_ind,ntr,0,0,NULL,NULL);
	 ptr=(Coeff_p)emalloc(sizeof(struct ENN_COEFF));
	 ptr->enn_coef=coef_val * enn_sign;
	 enn_sign=1;
	 ptr->coef=(Func_p)ecalloc(tabp[pl_ind].comp_num ,sizeof(struct COEFF));
	 for(ind=0;ind<tabp[pl_ind].comp_num;ind++)
	  {
	   ptr->coef[ind].xsucc_coef=(int *)ecalloc(3*MAX_COL_RIP,sizeof(int));
#ifndef ESYMBOLIC 
	//   ptr->coef[ind].sbc_coef=(int *)ecalloc(tabc[cl].sbc_num,sizeof(int));
	   ptr->coef[ind].sbc_coef=(int *)ecalloc(tabc[tabp[pl_ind].dominio[ind]].sbc_num,sizeof(int));
#else 
  	   ptr->coef[ind].sbc_coef=(int *)ecalloc( get_max_cardinality(),sizeof(int));
#endif	 
          
	  }
	 ptr->next=fun_ptr;
	 fun_ptr=ptr;
	 ptr->guard = NULL;
	}
	   opzguard LT funzseq GT
	{
	 if(count!=tabp[pl_ind].comp_num-1)
	  out_error(ERROR_LESSER_FUNCTION_COMPONENTS,pl_ind,ntr,0,0,NULL,NULL);
	}
	   ;
opzguard   : OBRCK 
	{
	 ptr->guard = NULL;
	 prd_h = prd_t = NULL;
	}
	     pred
	{
	 gen(TYPEEND,"");
	 ptr->guard = prd_h;

#ifdef ESYMBOLIC
	tabt[ntr].trans_type=ASYM_T;
#endif

	}
	     CBRCK
           |
	   ;
funzseq    :
	{
	 count=0;
	}
	     kernel
	   | funzseq COMMA 
        {
	 if(count==tabp[pl_ind].comp_num-1)
	  out_error(ERROR_GREATER_FUNCTION_COMPONENTS,pl_ind,ntr,0,0,NULL,NULL);
	 count++;
        }
	     kernel
	   ;
kernel	   : term
	   | kernel plusop term
	   ;
term	   : 
	    sterm
	   | 
	    nterm
	   | 
	    CARET coefficient STRING
	{
	 cl = tabp[pl_ind].dominio[count];
	 if(tabc[cl].type != ORDERED)
	  out_error(ERROR_ILLEGAL_PREDECESSOR,pl_ind,ntr,cl,0,NULL,NULL);
	 proj_num = get_proj_num(str_in,cl);
	 switch(proj_num)
	  {
	   case ERROR_SHARED_NAME :
		out_error(ERROR_SHARED_NAME,pl_ind,ntr,cl,0,str_in,NULL);
	   case ERROR_AMBIGUITY_IN_ARC :
		out_error(ERROR_AMBIGUITY_IN_ARC,pl_ind,ntr,0,0,NULL,NULL);
	   default :
		ptr->coef[count].xsucc_coef[3*(proj_num-1)+2]+=coef_val* sign;
		sign=1;
		break;
	  }
	}
	   | 
	   xterm
	   ;
sterm	   :  coefficient  S NUM
	{
#ifdef ESYMBOLIC
	tabt[ntr].trans_type=ASYM_T;
#endif
	 cl = tabp[pl_ind].dominio[count];
	 if(str_val<1 || str_val>tabc[cl].sbc_num)
	  out_error(ERROR_WRONG_STATIC_INDEX_IN_FUNCTION,pl_ind,ntr,cl,0,NULL,NULL);
	 if(input_flag && tabc[cl].sbclist[str_val-1].card==1)
	  {/* Trasformazione possibile da sincronizzazione a proiezione */
	   sprintf(sbc_name,"%ds!c%d",tabp[pl_ind].dominio[count],str_val-1);
	   strcpy(str_in,sbc_name);
	   if(!already_transformed(sbc_name,tabp[pl_ind].dominio[count]))
	    {/* Produzione del predicato */
	     if(tabt[ntr].guard == NULL)
	      {/* Creare struttura dati per il predicato */
	       tabt[ntr].guard = NULL;
	       prd_h = prd_t = NULL;
	       gen(TYPED,sbc_name);
	       sprintf(sbc_name,"%d",str_val-1);
	       gen(TYPENUM,sbc_name);
	       gen(TYPEEQ,"");
	       gen(TYPEEND,"");
	       tabt[ntr].guard = prd_h;
	      }/* Creare struttura dati per il predicato */
             else
	      {/* Struttura dati esistente */
	       prd_h = prd_t;
	       prd_t = tabt[ntr].guard;
	       for(;prd_t->next->type !=TYPEEND ;prd_t = prd_t->next);
	       gen(TYPED,sbc_name);
	       sprintf(sbc_name,"%d",str_val-1);
	       gen(TYPENUM,sbc_name);
	       gen(TYPEEQ,"");
	       gen(TYPEAND,"");
	       gen(TYPEEND,"");
	       prd_t->next = prd_h;
	      }/* Struttura dati esistente */
	    }/* Produzione del predicato */
	   proj_num=get_proj_num(str_in,tabp[pl_ind].dominio[count]);
           switch(proj_num)
	    {
	     case ERROR_SHARED_NAME :
		  out_error(ERROR_SHARED_NAME,pl_ind,ntr,cl,0,str_in,NULL);
	     case ERROR_AMBIGUITY_IN_ARC :
		  out_error(ERROR_AMBIGUITY_IN_ARC,pl_ind,ntr,0,0,NULL,NULL);
	     default :
		  ptr->coef[count].xsucc_coef[3*(proj_num-1)]+=coef_val* sign;
		  break;
            } 
	  }/* Trasformazione possibile da sincronizzazione a proiezione */
         else
	 ptr->coef[count].sbc_coef[str_val-1]+=coef_val * sign;
	 sign=1;
	}
	   | coefficient  S STRING
	{
#ifdef ESYMBOLIC
	tabt[ntr].trans_type=ASYM_T;
#endif
	 cl = tabp[pl_ind].dominio[count];
	 if((sbc_ind=already_exist(str_in,cl,STAT))==0)
	  out_error(ERROR_WRONG_STATIC_NAME_IN_FUNCTION,pl_ind,ntr,cl,0,str_in,NULL);
	 if(input_flag && tabc[tabp[pl_ind].dominio[count]].sbclist[sbc_ind-1].card==1)
	  {/* Trasformazione possibile da sincronizzazione a proiezione */
	   sprintf(sbc_name,"%ds!c%d",tabp[pl_ind].dominio[count],sbc_ind-1);
	   strcpy(str_in,sbc_name);
	   if(!already_transformed(sbc_name,tabp[pl_ind].dominio[count]))
	    {/* Produzione del predicato */
	     if(tabt[ntr].guard==NULL)
	      {/* Creare struttura dati per il predicato */
	       tabt[ntr].guard = NULL;
	       prd_h = prd_t = NULL;
	       gen(TYPED,sbc_name);
	       sprintf(sbc_name,"%d",sbc_ind-1);
	       gen(TYPENUM,sbc_name);
	       gen(TYPEEQ,"");
	       gen(TYPEEND,"");
	       tabt[ntr].guard = prd_h;
	      }/* Creare struttura dati per il predicato */
             else
	      {/* Struttura dati esistente */
	       prd_h = prd_t;
	       prd_t = tabt[ntr].guard;
	       for(;prd_t->next->type != TYPEEND ; prd_t = prd_t->next);
	       gen(TYPED,sbc_name);
	       sprintf(sbc_name,"%d",sbc_ind-1);
	       gen(TYPENUM,sbc_name);
	       gen(TYPEEQ,"");
	       gen(TYPEAND,"");
	       gen(TYPEEND,"");
	       prd_t->next = prd_h;
	      }/* Struttura dati esistente */
	    }/* Produzione del predicato */
	   proj_num=get_proj_num(str_in,tabp[pl_ind].dominio[count]);
           switch(proj_num)
	    {
	     case ERROR_SHARED_NAME :
		  out_error(ERROR_SHARED_NAME,pl_ind,ntr,cl,0,str_in,NULL);
	     case ERROR_AMBIGUITY_IN_ARC :
		  out_error(ERROR_AMBIGUITY_IN_ARC,pl_ind,ntr,0,0,NULL,NULL);
	     default :
		  ptr->coef[count].xsucc_coef[3*(proj_num-1)]+=coef_val* sign;
		  break;
            } 
	  }/* Trasformazione possibile da sincronizzazione a proiezione */
         else
	  ptr->coef[count].sbc_coef[sbc_ind-1]+=coef_val * sign;
	 sign=1;
	}
	   | coefficient  S
	{
#ifndef ESYMBOLIC
        for(i=0;i<tabc[tabp[pl_ind].dominio[count]].sbc_num;i++)
	   ptr->coef[count].sbc_coef[i]+=coef_val * sign;
	 sign=1;
#else
	for(i=0;i<get_max_cardinality();i++)
           ptr->coef[count].sbc_coef[i]+=coef_val * sign;
	 sign=1;
#endif

	}
	   ;
nterm	   : coefficient  ESCL STRING
	{
	 cl = tabp[pl_ind].dominio[count];
	 if(tabc[cl].type != ORDERED)
	  out_error(ERROR_ILLEGAL_SUCCESSOR,pl_ind,ntr,cl,0,NULL,NULL);
	 proj_num=get_proj_num(str_in,tabp[pl_ind].dominio[count]);
	 switch(proj_num)
	  {
	   case ERROR_SHARED_NAME :
		out_error(ERROR_SHARED_NAME,pl_ind,ntr,cl,0,str_in,NULL);
	   case ERROR_AMBIGUITY_IN_ARC :
		out_error(ERROR_AMBIGUITY_IN_ARC,pl_ind,ntr,0,0,NULL,NULL);
	   default :
		ptr->coef[count].xsucc_coef[3*(proj_num-1)+1]+=coef_val* sign;
		sign=1;
		break;
	  }
	}
	   ;
xterm	   :  coefficient  STRING
	{
	 proj_num=get_proj_num(str_in,tabp[pl_ind].dominio[count]);
         switch(proj_num)
	  {
	   case ERROR_SHARED_NAME :
		out_error(ERROR_SHARED_NAME,pl_ind,ntr,cl,0,str_in,NULL);
	   case ERROR_AMBIGUITY_IN_ARC :
		out_error(ERROR_AMBIGUITY_IN_ARC,pl_ind,ntr,0,0,NULL,NULL);
	   default :
		ptr->coef[count].xsucc_coef[3*(proj_num-1)]+=coef_val* sign;
		sign=1;
		break;
          } 
	}
	   ;
coefficient : coefficient_type | { coef_val = card_val = 1;} ;
coefficient_type  : 
            NUM
        {
         coef_val = card_val = str_val;
#ifdef PIPPO
        }
	   | UNDERSCORE STRING
	{
         coef_val = card_val = get_marking_parameter_value(str_in);
         if(coef_val == UNKNOWN)
          out_error(ERROR_UNKNOWN_MARKING_PARAMETER,pl_ind,ntr,cl,0,str_in,NULL);
#endif
	}
           | VBAR STRING
        {
         strcpy(class_name,str_in);
        }
             POINT STRING
        {
         strcpy(sbc_name,str_in);
        }
             VBAR
        {
         /* Controllo su esistenza della classe */
	 if((ind=get_index(class_name))==-1)
	  out_error(ERROR_MISSING_CLASS,0,0,0,0,class_name,NULL);
	 else
          {  
           /* Controllo su esistenza sottoclasse */
	   if((sbc_ind=already_exist(sbc_name,ind,STAT))==0)
	    out_error(ERROR_MISSING_STATIC,0,0,0,0,sbc_name,NULL);
	   else 
            coef_val = card_val = tabc[ind].sbclist[sbc_ind-1].card;
          }
        }
           | VBAR STRING VBAR
        {
         /* Controllo su esistenza della classe */
         if((ind=get_index(str_in))==-1)
          {
           /* Controllo su esistenza sottoclasse */
           for(i=0,stop=FALSE;!stop && i<ncl;i++)
            if((ind=already_exist(str_in,i,STAT))!=0) stop=TRUE;
	   if(stop==FALSE)
	    out_error(ERROR_MISSING_CLASS,0,0,0,0,str_in,NULL);
	   else 
            coef_val = card_val = tabc[i-1].sbclist[ind-1].card;
           /* Questa cardinalita'e'quella della prima sottoclasse */
           /* trovata con quel nome                               */
          }
         else 
          coef_val = card_val = get_class_card(ind);
        }
	   ;
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
	 fprintf(stdout,"Predicate operator < is not allowed in SWN specification\n");
         exit(1);
	}
	   | LE
        {
	 fprintf(stdout,"Predicate operator <= is not allowed in SWN specification\n");
         exit(1);
	}
	   | GT
        {
	 fprintf(stdout,"Predicate operator > is not allowed in SWN specification\n");
         exit(1);
	}
	   | GE
        {
	 fprintf(stdout,"Predicate operator >= is not allowed in SWN specification\n");
         exit(1);
	}
	   ;
predop:   NUM
	{
	 sprintf(sbc_name,"%d",atoi(jjtext)-1);
	 gen(TYPENUM,sbc_name);
	}
      |    STRING
	{
	 gen(TYPESTRING,str_in);
	}
      |    D
	   OPAR
	   STRING
	   CPAR
	{
	 gen(TYPED,str_in);
	}
      |    ESCL
	   STRING
	{
	 gen(TYPESUCCESSOR,str_in);
	}
      |    CARET
	   STRING
	{
	 gen(TYPEPREDECESSOR,str_in);
	}
      |    DISTANCE
	   OPAR
	   STRING
	{
#ifdef SYMBOLIC
	 fprintf(stdout,"SWN predicate definition DISTANCE is not allowed in SWN symbolic solution\n");
         exit(1);
#endif
#ifdef COLOURED
	 gen(TYPEDISTANCE,str_in);
#endif
	}
	   COMMA
	   STRING 
	   CPAR
	{
#ifdef SYMBOLIC
	 fprintf(stdout,"SWN predicate definition DISTANCE is not allowed in SWN symbolic solution\n");
         exit(1);
#endif
#ifdef COLOURED
	 gen(TYPEDISTANCE,str_in);
#endif
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
