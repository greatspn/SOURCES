%{
/************* INCLUDES AND EXTERNALS *************************************/
#include <string.h>
#include "TRANS.y.h"

extern int mon_yyinput(char* tamp,int taille_max_tamp);

#undef YY_INPUT
#define YY_INPUT(t,r,tm)(r= mon_yyinput(t,tm))
#define  min(a,b)((a <= b) ? a : b)

extern char* Ptr_Cur;
extern int Lim_Pos;

//int mon_yyinput(char* tamp,int taille_max_tamp)
//{ 
 // int n= min(taille_max_tamp, Lim_Pos-(int)Ptr_Cur);
//  if(n>0){
 //         memcpy(tamp,Ptr_Cur,n);
 //         Ptr_Cur +=n;
       
 //        }
 
//  return n;
//}
 
extern void flush_buffer(); //{ yy_flush_buffer(YY_CURRENT_BUFFER);}

%}
 
%%
"MAXC"                                {return(MCOMP);}
"MAXIMUM"                             {return(MAX);}
"#"                                   {return (D);}
"or"                                  {return(OR);} 
"and"                                 {return(AND);}
"!"                                   {return(NOT);} 
"="                                   {return(EQ);}
";"                                   {return(';');}
"}"                                   {return('}');}
"{"                                   {return('{');} 
"["                                   {return('[');}
"]"                                   {return(']');} 
","                                   {return(',');} 
")"                                   {return(')');}  
"("                                   {return('(');}

[0-9]+                                { Translval.j=atoi(yytext); return(IDENT);}
[a-zA-Z_][a-zA-Z0-9_]*	              {	Translval.s=strdup(yytext); return(ID);}
[ \t]                                 {} 
[ \n]		                      {}
                            
%%
