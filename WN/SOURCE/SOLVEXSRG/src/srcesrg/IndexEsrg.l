%{
/************* INCLUDES AND EXTERNALS *************************************/
#include <string.h>
#include "IndexEsrg.y.h"

int mon_yyinput_index(char* tamp,int taille_max_tamp);

#undef YY_INPUT
#define YY_INPUT(t,r,tm)(r= mon_yyinput_index(t,tm))
#define  min(a,b)((a <= b) ? a : b)

extern char* Ptr_Cur_index;
extern int Lim_Pos_index;

int mon_yyinput_index(char* tamp,int taille_max_tamp)
{ 
  int n= min(taille_max_tamp, Lim_Pos_index-(int)Ptr_Cur_index);
  if(n>0){
          memcpy(tamp,Ptr_Cur_index,n);
          Ptr_Cur_index +=n;
       
         }
 
  return n;
}
 
void flush_buffer_index(){ yy_flush_buffer(YY_CURRENT_BUFFER);}

%}
 
%%


"%"                                   {return(NBEVENT);} 
"#"                                   {return(NBORDTOK);} 
"-"                                   {return(MINUS);} 
"="                                   {return(EQ);}
"<"                                   {return(LT);} 
">"                                   {return(GT);} 
"*"                                   {return(IDEN);}

";"                                   {return(';');}
"}"                                   {return('}');}
"{"                                   {return('{');} 
":"                                   {return(':');} 
"["                                   {return('[');}
"]"                                   {return(']');} 
","                                   {return(',');} 
")"                                   {return(')');}  
"("                                   {return('(');}

[0-9]+                                { Indexlval.j=atoi(yytext); return(IDENT);}
[a-zA-Z_][a-zA-Z0-9_]*	              {	Indexlval.s=strdup(yytext); return(ID);}
[ \t]                                 {} 
[ \n]		                      {}
                            
%%
