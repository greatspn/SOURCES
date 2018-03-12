%{
/*      lisp-obj.lex   */
#include <string.h>

#define LISP_OBJ_LEX
#include "attrib-yacc.h"


#include "lisp-obj_yacc.y.h"

#include "var-lex.h"

#define YY_INPUT(buf,result,max_size) \
{ \
int c = (*LEXtoParsifyString++); \
result = (c == '\0') ? YY_NULL : (buf[0] = c, 1); \
}	

%}

%%
\n                    { /* unput((char)0,jjin); */ 
                        return(NEWLINE);    }
\#		      { return(DIESIS);    }
\^		      { return(CARET);      }
\{		      { return(OBRCS);      }
\}		      { return(CBRCS);      }
\!		      { return(ESCL);       }
\.		      { return(POINT);      }
\:		      { return(COLON);      }
\;		      { return(SEMICOLON);  }
\+		      { return(PLUS);       }
\-		      { return(MINUS);      }
\*		      { return(MOLT);       }
\/		      { return(DIV);        }
\,		      { return(COMMA);      }
\<		      { return(LESSER);     }
\>		      { return(GREATER);    }
\(		      { return(OPAR);       }
\)		      { return(CPAR);       }
\[		      { return(OBRCK);      }
\]		      { return(CBRCK);      }
=		      { return(EQUAL);      }
\~z/[' ']	      { return(MD);          
		      }
\~w/[' ']	      { return(PD);      
		      }
\~k/[' ']	      { return(TD);      
		      }
\~f/[' ']	      { return(FU);      
		      }
\~m/[' ']	      { return(MA);     
	              }
\~c/[' ']	      { return(CO);      
		      }
S                     { return(S);          }
\|		      { return(VBAR);       }
o/[' ']		      { return(O);      
		      }
u/[' ']		      { return(US);      
		      }
d/[(]		      { return(D);      }
ID/\n                 { return(ID);      }
id/\n                 { return(ID);     }
or		      { return(OR);      }
and		      { return(AND);      }
FLUSH		      { return(FLUSH);       }
[0-9]+		      { 
			yylval.num_int = atoi(yytext);
			return(NUM); 
		      }
[a-zA-Z][a-zA-Z_0-9]* { 
			yylval.String = Estrdup(yytext);
			return(STRING); 
		      }

%%

int yywrap(void)
{
	return (1);
}
