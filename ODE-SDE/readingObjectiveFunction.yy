%{

#include <stdio.h>
#include <string.h>
#include <map>
#include <sstream>
#include <fstream>
#include "ObjectiveFunction.hpp"
#include <FlexLexer.h>
#include <exception>

FlexLexer* lexer;

std::map<std::string,int> Place2Int;

std::string ObjFuncParsed;
std::string array_name;

using namespace std;
using namespace tree_def;

tree* t;

extern "C" {
int yyparse(void);

}



void yyerror(const char *str){
    fprintf(stderr,"Error: %s  at line: %d, last word parsed:%s  \n",str,lexer->lineno()-1,lexer->YYText());
  }



int yyFlexLexer::yywrap()
{
        return 1;


}


int yylex(void){
    int i = lexer->yylex();
    return i;
}


string parseObjectiveFunction(const std::string& of_name, const std::string& a_name)
{

array_name = a_name;
ObjFuncParsed = "**** ERROR - OBJECTIVE FUNCTION HAS NOT BEEN PARSED CORRECTLY *****";
std::stringstream Ibuffer;
 try{
     std::ifstream file(of_name.c_str());
     if(!file){
      cerr << "\n*****Error opening input file "<<of_name<<" *****\n" << endl;
      exit(EXIT_FAILURE);
     }
     Ibuffer<<file.rdbuf();
     lexer = new yyFlexLexer(&Ibuffer);

     if(Ibuffer.str()[Ibuffer.str().size()-1]!='\n'){
        Ibuffer << "\n";
     }

     yyparse();
    }
    catch (exception e){
      cerr << "\n*****"<<e.what()<<endl<< endl;
      exit(EXIT_FAILURE);
    }



return ObjFuncParsed;

}


%}


%union{
  long double num;
  char *pVar;
  class node* Node;
  class tree* Tree;
}

%token <num> FLOAT
%token <pVar> PLACE
%token <pVar> PLUS
%token <pVar> MINUS
%token <pVar> MULT
%token <pVar> DIV
%token <pVar> FUNCT
%token <pVar> BOPEN
%token <pVar> BCLOSE
%token <pVar> STRING
%token ENDLINE
%type <Tree> objfunc
%type <Node> fun
%type <Node> exp
%type <Node> com
%type <Node> base
%type <Node> composite
%type <pVar> op
%type <pVar> bracketO
%type <pVar> bracketC
%type <pVar> funct




%%


/* Objfunc represents the root of the grammar which can be either a function or a comment */

objfunc: com ENDLINE {t= new tree($1);
                      ObjFuncParsed = t->toString();}
        |exp ENDLINE {t= new tree($1);
                      ObjFuncParsed = t->toString();}

	;

/* Exp represents a combination of more sub-rules including functions, base types, numbers or composite. */

exp:     exp op fun { $$ = new expression_node($2,$1,$3);}
	| fun {$$ = new expression_node("funct",$1,nullptr);}
	| base {$$ = new expression_node("base",$1,nullptr);}
	| exp op base {$$ = new expression_node($2,$1,$3);}
	| composite {$$ = new expression_node("compos",$1,nullptr);}
	| exp op composite {$$ = new expression_node($2,$1,$3);}
        ;


/* Fun rule is used to express R recognized functions such as "sin","abs","log". Has one arguement. */

fun:
        funct bracketO exp bracketC { $$= new function_node($1,$3);}
	|bracketO exp bracketC {$$= new function_node("",$2);}
        ;


/* Op rule groups common aritmethical operators. */

op:	PLUS	{//std::cout<<"+";
		 $$ = $1;}
	|MINUS	{//std::cout<<"-";
		 $$ = $1;}
	|MULT	{//std::cout<<"*";
		 $$ = $1;}
	|DIV	{//std::cout<<"/";
		 $$ = $1;}
        ;


/* Composite is used make the grammar be able to parse sub-expressions as "5x"
or generically when a float is followed by a variable without
operator of multiplication between them */

composite: FLOAT PLACE {

			    auto iter = Place2Int.find($2);

                if(iter != Place2Int.end()) {
                    $$= new composite_node(new num_node($1),new var_node(iter->second+2,array_name));
                    //std::cout<<$1<<$2;
                }
                else {
                    cerr << "\n*****Error, no such place called like "<<$2<<" *****\n" << endl;
                    exit(EXIT_FAILURE);
                }
			}
			|MINUS FLOAT PLACE {
                auto iter = Place2Int.find($3);

                if(iter != Place2Int.end()) {
                    $$= new composite_node(new num_node(-1*$2),new var_node(iter->second+2,array_name));
                    //std::cout<<$1<<$2;
                }
                else {
                    cerr << "\n*****Error, no such place called like "<<$3<<" *****\n" << endl;
                    exit(EXIT_FAILURE);
                }
        }
;


/*Base expression groups two terminal tokens: floating numbers and variables (place).
  Places are linked to their proper index in R array. */

base:	FLOAT	{//std::cout<<$1;
		 $$ = new num_node($1);}
        |PLACE	{
			    auto iter = Place2Int.find($1);
                if(iter != Place2Int.end()) {
                    $$= new var_node(iter->second+2,array_name);
                    //std::cout<<$1;
                }
                else {
                    cerr << "\n*****Error, no such place called like "<<$1<<" *****\n" << endl;
                    exit(EXIT_FAILURE);
                }
            }
;


/* Terminal token for "(". */

bracketO: BOPEN {//std::cout<<"(";
		 //$$ = $1;
}
;


/* Terminal token for ")". */

bracketC: BCLOSE {//std::cout<<")";
		 //$$ = $1;
}


/* Terminal token for function names. */

funct: FUNCT {//std::cout<<$1;
	      $$ = $1;}
;


/* Terminal token for comments. */

com: STRING 	{//std::cout<<$1<<std::endl;
		$$=new comment_node($1);}
;

%%
