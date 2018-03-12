%option nostdinit
%option prefix="jj"
%option noyywrap
%{
# include <stdio.h>
# include <string.h>
# include <assert.h>
# include "../INCLUDE/const.h"
# include "../INCLUDE/struct.h"
# include "../INCLUDE/var_ext.h"
# include "../INCLUDE/macros.h"

#ifdef GSPN
# include "gspn_grammar.y.h"
#elif defined SWN
# include "wn_grammar.y.h"
#else
# error "Missing GSPN/SWN flag."
#endif

//#ifdef WN_GRAMMAR_H
//# include WN_GRAMMAR_H
//#endif

extern char *jjtext;
extern FILE *jjin;
extern int parsing_flag;
extern int get_mpar_value(const char* name, int* value);
extern int get_rpar_value(const char* name, double* value);

//----------------------------------------------------------------------------
// Elvio 19/09/2016: replace the orrible hack of writing to the netName.parse
// file every single lexer expression, and redirect instead the lexer 
// input to the memory buffer *lexer_buffer.
// Redirection is done with #define macros that replace stdio functions.

// The lexer buffer, which stores the string that will be tokenized.
const char *lexer_buffer;

// Set a new buffer
void lexer_set_buffer(const char *b) { 
	if (b != NULL) {
	  	if (lexer_buffer != NULL) {
			fprintf(stderr, "A previous usage of the lexer has not cleared the buffer "
					"using lexer_set_buffer(NULL);\n");
		}
		//printf("Will parse: %s", b);
	}
	lexer_buffer = b;
}

int lexer_fread(char *out_ptr, size_t size, size_t nmemb, FILE *file) {
	int count = 0;
	assert(lexer_buffer != NULL && file == jjin);
	assert(size == 1); // the size>1 case is not yet implemented
	while (*lexer_buffer && count < nmemb) {
		*out_ptr++ = *lexer_buffer++;
		count++;
	}
	return count;
}

int lexer_clearerr(FILE* file) {}

int lexer_ferror(FILE* file) { 
	return '\0' != *lexer_buffer;
}

int lexer_getc(FILE* file) {
	if (*lexer_buffer)
		return *lexer_buffer++;
	return EOF;
}

int lexer_fileno(FILE *file) { 
	return -100; 
}

int lexer_isatty(int tty) {
	assert(tty == -100);
	return 0;
}

int lexer_fwrite(const void * ptr, size_t size, size_t count, FILE * stream) {
	// Actually ignore the output
	//return fwrite(ptr, size, count, stream);
	return 0;
}

// Clear standard <stdio> macros
#ifdef getc
# undef getc
#endif

// Replace <stdio> calls of the lexer with the new lexer_* calls
#define fread(a,b,c,d)		lexer_fread(a,b,c,d)
#define fwrite(a,b,c,d)		lexer_fwrite(a,b,c,d)
#define clearerr(a)			lexer_clearerr(a)
#define ferror(a)			lexer_ferror(a)
#define getc(a)				lexer_getc(a)
#define fileno(a)			lexer_fileno(a)
#define isatty(a)			lexer_isatty(a)

//----------------------------------------------------------------------------

%}

%%
\n                    { /*ungetc((char)0,jjin);*/
                        /*return(NEWLINE);*/ }
\_		      { return(UNDERSCORE);  }
\#		      { return(DIESIS);      }
\:		      { return(COLON);       }
\;		      { return(SEMICOLON);   }
\+		      { return(PLUS);        }
\-		      { return(MINUS);       }
\*		      { return(MOLT);        }
\/		      { return(DIV);         }
\%		      { return(DISTANCE);    }
\^		      { return(CARET);       }
\{		      { return(OBRCS);       }
\}		      { return(CBRCS);       }
\!		      { return(ESCL);        }
\.		      { return(POINT);       }
\,		      { return(COMMA);       }
"/="|"<>"     { return(NE);          }
\<\=                  { return(LE);          }
\<		      { return(LT);      }
\>\=                  { return(GE);          }
\>		      { return(GT);     }
\(		      { /*fprintf(stdout,"(\n");*/return(OPAR);        }
\)		      { /*fprintf(stdout,")\n");*/return(CPAR);        }
\[		      { /*fprintf(stdout,"[\n");*/return(OBRCK);       }
\]		      { /*fprintf(stdout,"]\n");*/return(CBRCK);       }
=		      { /*fprintf(stdout,"=\n");*/return(EQ);       }
\~z/[' ']	      { 
			parsing_flag = MDRATE;
			return(MD);	     
		      }
\~w/[' ']	      { 
			parsing_flag = PLACE;
			return(PD);	     
		      }
\~k/[' ']	      {
			/*fprintf(stdout,"~k\n");*/parsing_flag = TRANSITION;
			return(TD);	     
		      }
\~f/[' ']	      {
			parsing_flag = FUNCTION;
			return(FU);	     
		      }
\~j/[' ']	      { 
			parsing_flag = DYNAMIC_SUBCLASS;
			return(DSC);	     
	              }
\~m/[' ']             { 
			parsing_flag = INITIAL_MARKING;
			return(MA);	     
	              }
\~c/[' ']	      { 
			parsing_flag = COLOR_CLASS;
			return(CO);	     
		      }
S                     { return(S);           }
\|		      { return(VBAR);        }
o/[' ']		      { 
			str_val = ORDERED ;
			return(O) ;
		      }
u/[' ']		      { 
			str_val = UNORDERED;
			return(US);
		      }
d/[(]		      { /*fprintf(stdout,"d\n");*/return(D);           }
ID/\n                 { return(ID);          }
id/\n                 { return(ID);          }
fact                  { return(FACT);        }
comb                  { return(COMB);        }
sqrt	              { return(SQRT);        }
pow	              { return(POW);         }
inv	              { return(INV);         }
max	              { return(MAXT);         }
min	              { return(MINT);         }
when	              { return(WHEN);        }
ever	              { return(EVER);        }
"or"|"||"		      { return(OR);          }
"and"|"&&"		      { return(AND);	     }
FLUSH		      { return(FLUSH);	     }
[0-9]+		      { 
			str_val = atoi(jjtext) ;
			return(NUM);
		      }
[0-9]+\.[0-9]+	      { 
			real_val = strtod(jjtext,(char **)NULL) ;
			return(REALNUM);
		      }
[a-zA-Z][a-zA-Z_0-9]* { 
			if (get_mpar_value(jjtext, &str_val))
				return NUM;
			if (get_rpar_value(jjtext, &real_val))
				return REALNUM;
			strcpy(str_in,jjtext) ;
			 /*fprintf(stdout,"STRING\n");*/return(STRING);
		      }
[' ']|[\0]            {
                        /* Do NOTHING !! DO NOT ECHO ON STDOUT !!! */
                      }
%%
