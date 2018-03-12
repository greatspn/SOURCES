%{

int_val;
extern int yylex();

#define EQUAL_TO 1
#define NOTEQUAL_TO 2
#define GREATER_EQUAL 3
#define GREATER_THAN 4
#define LESS_EQUAL 5
#define LESS_THAN 6

#define SUM 11
#define SUB 12
#define MUL 13
#define DIV 14

#define L_AND 22
#define L_OR 23
%}

%start assign

%token WHEN EVER PLACE_NAME COMP_OP INTEGER MARK_PAR ARITHM_OP
%token RATE_PAR EXP ERROR COLON SEMICOLON POINT OPEN CLOSE LOGIC_OP NOT_OP

%%

assign : EVER
	    { fprintf(rafp,"{ double rr =\n");
	      fprintf(rsfp,"{ double rr =\n");
	    }
	  value SEMICOLON
	    { fprintf(rafp,";\n  return( rr );\n  }\n");
	      fprintf(rsfp,";\n  return( rr );\n  }\n");
	      yyin = eof;
	    }
       | when_clause EVER
	    { fprintf(rafp,"  else   rr =\n");
	      fprintf(rsfp,"  else   rr =\n");
	    }
	  value SEMICOLON
	    { fprintf(rafp," ;\n  return( rr );\n  }\n");
	      fprintf(rsfp," ;\n  return( rr );\n  }\n");
	      yyin = eof;
	    }
       ;

when_clause : when_clause WHEN
		  { fprintf(rafp,"  else if (\n");
		    fprintf(rsfp,"  else if (\n");
		  }
		logic_cond COLON
		  { fprintf(rafp,"\n  )  rr =\n");
		    fprintf(rsfp,"\n  )  rr =\n");
		  }
		value SEMICOLON
		  { fprintf(rafp," ;\n");
		    fprintf(rsfp," ;\n");
		  }
	    | WHEN
		  { fprintf(rafp,"{ double rr;\n");
		    fprintf(rafp,"  if (\n");
		    fprintf(rsfp,"{ double rr;\n");
		    fprintf(rsfp,"  if (\n");
		  }
		logic_cond COLON
		  { fprintf(rafp,"\n  )  rr =\n");
		    fprintf(rsfp,"\n  )  rr =\n");
		  }
		value SEMICOLON
		  { fprintf(rafp," ;\n");
		    fprintf(rsfp," ;\n");
		  }
	    ;

logic_cond : OPEN
		  { fprintf(rafp," ( ");
		    fprintf(rsfp," ( ");
		  }
		logic_cond CLOSE
		  { fprintf(rafp," ) ");
		    fprintf(rsfp," ) ");
		  }
	   | NOT_OP
		  { fprintf(rafp," ! ");
		    fprintf(rsfp," ! ");
		  }
		 logic_cond
	   | logic_cond LOGIC_OP
		    { if ( yylval == L_AND ) {
			  fprintf(rafp," && ");
			  fprintf(rsfp," && ");
			}
		      else {
			  fprintf(rafp," || ");
			  fprintf(rsfp," || ");
			}
		    }
		 logic_cond
	   | compare
	   ;

compare : PLACE_NAME
		  { fprintf(rafp,"DP[%d]", yylval );
		    fprintf(rsfp,"DP[%d]", yylval );
		  }
	      COMP_OP
		  { switch (yylval) {
		      case EQUAL_TO :
			  fprintf(rafp," == ");
			  fprintf(rsfp," == ");
			break;
		      case NOTEQUAL_TO :
			  fprintf(rafp," != ");
			  fprintf(rsfp," != ");
			break;
		      case GREATER_EQUAL :
			  fprintf(rafp," >= ");
			  fprintf(rsfp," >= ");
			break;
		      case GREATER_THAN :
			  fprintf(rafp," > ");
			  fprintf(rsfp," > ");
			break;
		      case LESS_EQUAL :
			  fprintf(rafp," <= ");
			  fprintf(rsfp," <= ");
			break;
		      case LESS_THAN :
			  fprintf(rafp," < ");
			  fprintf(rsfp," < ");
			break;
		    }
		  }
	      int_const
	;

int_const : INTEGER
		{ fprintf(rafp,"%d", int_val );
		  fprintf(rsfp,"%d", int_val );
		}
	  | MARK_PAR
		{ fprintf(rafp,"%d", (int)MP[yylval-1] );
		  fprintf(rsfp,"%d", (int)MP[yylval-1] );
		}
	  | PLACE_NAME
		{ fprintf(rafp,"DP[%d]", yylval );
		  fprintf(rsfp,"DP[%d]", yylval );
		}
	  ;

value : OPEN
		{ fprintf(rafp," ( ");
		  fprintf(rsfp," ( ");
		}
	    value CLOSE
		{ fprintf(rafp," ) ");
		  fprintf(rsfp," ) ");
		}
      | value ARITHM_OP
		{ switch (yylval) {
		    case SUM :
			fprintf(rafp," + ");
			fprintf(rsfp," + ");
		      break;
		    case SUB :
			fprintf(rafp," - ");
			fprintf(rsfp," - ");
		      break;
		    case MUL :
			fprintf(rafp," * ");
			fprintf(rsfp," * ");
		      break;
		    case DIV :
			fprintf(rafp," / ");
			fprintf(rsfp," / ");
		      break;
		  }
		}
	    value
      | real_val
      ;

real_val : real
	 | int_const
	 | RATE_PAR
		{ fprintf(rafp,"%f", RP[yylval-1] );
		  fprintf(rsfp,"%f", RP[yylval-1] );
		}
	 ;

real : rational EXP
	      { fprintf(rafp,"e%d", yylval );
	        fprintf(rsfp,"e%d", yylval );
	      }
     | rational
     ;

rational : INTEGER POINT
		{ fprintf(rafp,"%d.", int_val );
		  fprintf(rsfp,"%d.", int_val );
		}
	      mant
	 | POINT
		{ fprintf(rafp,"0.");
		  fprintf(rsfp,"0.");
		}
	      mant
	 ;

mant : INTEGER
	    { fprintf(rafp,"%d", int_val );
	      fprintf(rsfp,"%d", int_val );
	    }
     |
	    { fprintf(rafp,"0");
	      fprintf(rsfp,"0");
	    }
     ;
%%

#include "mdr_gram.y.h"
#include "comp_rate.c"

