%{
/************* INCLUDES AND EXTERNALS *************************************/
#include <string.h>
#include "PROPG.y.h"

int mon_yyinput(char* tamp,int taille_max_tamp);

#undef YY_INPUT
#define YY_INPUT(t,r,tm)(r= mon_yyinput(t,tm))
#define  min(a,b)((a <= b) ? a : b)

extern char* Ptr_Cur;
extern int Lim_Pos;

int mon_yyinput(char* tamp,int taille_max_tamp)
{ 
  int n= min(taille_max_tamp, Lim_Pos-(int)Ptr_Cur);
  if(n>0){
          memcpy(tamp,Ptr_Cur,n);
          Ptr_Cur +=n;
       
         }
 
  return n;
}
 
void flush_buffer(){ yy_flush_buffer(YY_CURRENT_BUFFER);}

%}
 
%%
"DECL"                                {return(DL);}
"INIT"                                {return(INT);}
"PROPS"                               {return(PRS);}
"EDECL"                                {return(EDL);}
"EINIT"                                {return(EIN);}
"EPROPS"                               {return(EPRS);}


"#"                                   {return(DECOL);} 
"+"                                   {return(OR);} 
"-"                                   {return(MINUS);}
"^"                                   {return(AND);}
"!"                                   {return(NOT);} 
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

[0-9]+                                { Proplval.j=atoi(yytext); return(IDENT);}
[a-zA-Z_][a-zA-Z0-9_]*	              {	Proplval.s=strdup(yytext); return(ID);}
[ \t]                                 {} 
[ \n]		                      {}
                            
%%
