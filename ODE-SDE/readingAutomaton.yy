%{

#include <iostream>
#include <fstream>
#include "automa.hpp"
#include <FlexLexer.h>
#include <sstream>

using namespace std;



  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>

  FlexLexer* lexer;
  stringstream Ibuffer;


  //!Error function
  void yyerror(const char *str){
    fprintf(stderr,"Error: %s  at line: %d, last word parsed:%s  \n",str,lexer->lineno()-1,lexer->YYText());
  }


  int yyFlexLexer::yywrap(){
    return 1;
  }

  int yyparse(void);


int yylex(void){
    int i = lexer->yylex();
    return i;
}
 FILE *yyin; //to set the input

using namespace AUTOMA;
automaton*  ris;
list <class trans> Trans;
vector<int> Resets;

 void reading_automaton(automaton& a, char* name){
 ris=&a;
 cout<<"\n\n\tSTART PARSING..."<<endl;
 try{
     std::ifstream file(name);
     if(!file){
      cerr << "\n*****Error opening input file "<<name<<" *****\n" << endl;
      exit(EXIT_FAILURE);
     }
     Ibuffer<<file.rdbuf();
     lexer = new yyFlexLexer(&Ibuffer);
     yyparse();
    }
    catch (my_exception e){
      cerr << "\n*****"<<e.what()<<endl<< endl;
      exit(EXIT_FAILURE);
    }
    cout<<"\n\n\tEND PARSING"<<endl;




  cout<<"\n\nEND EXECUTION"<<endl;
}


%}
%union{
  long double num;
  char *pVar;
  char *pCom;
  class cond* Cond;
}

%token <num> NUMBER
%token <pVar> STRING
%token <pCom> COMMENT
%left  <pVar> INIT FINAL
%left PLUS MINUS TIMES DIV
%token MINOR MAJOR NEQ EQ LMINOR LMAJOR
%token ENDL
%left OR
%left AND
%right NOT
%token LPARENT RPARENT LBRACE RBRACE
%token SEMICOLON COMMA ARROW   
%type <pVar> State
%token <pVar> TIMEVAR
%type <pVar> Spec
%type <Cond> Condition
%type <Cond> Expression
%type <Trans> Transition
%start Automaton
%%

Automaton: 	LBRACE  States  RBRACE SEMICOLON LBRACE  TimerVars  RBRACE SEMICOLON ENDL Graph
        | 	LBRACE  States  RBRACE SEMICOLON ENDL Graph
		| COMMENT Text  ENDL Automaton
		| COMMENT Text  ENDL
		;


Text:		STRING
		| STRING Text
		;

Graph:		Node SEMICOLON  ENDL
		| Node SEMICOLON  ENDL Graph
		| ENDL Graph
		| ENDL
		;

Node:		State  LPARENT Condition RPARENT {ris->add_node_info($1,$3,Trans);}
		| State  LPARENT Condition RPARENT LBRACE  Transition RBRACE {ris->add_node_info($1,$3,Trans); Trans.clear();}
		| State  LBRACE  Transition RBRACE {ris->add_node_info($1,nullptr,Trans); Trans.clear();}
		;

Transition:	Condition   ARROW State  { Trans.push_back(trans(ris->node_int($3),Resets,$1)); }
		| Condition     ARROW State SEMICOLON Transition { Trans.push_back(trans(ris->node_int($3),Resets,$1)); }
		| Condition  LBRACE  Resets  RBRACE  ARROW State  { Trans.push_back(trans(ris->node_int($6),Resets,$1)); Resets.clear(); }
		| Condition  LBRACE  Resets  RBRACE ARROW State SEMICOLON Transition { Trans.push_back(trans(ris->node_int($6),Resets,$1)); Resets.clear();}
		;
		
		
Condition:	Expression MINOR Expression { $$ = new  ineq(cond::op_type::C_MINOR,  $1, $3); }
		| Expression MAJOR Expression { $$ = new  ineq(cond::op_type::C_MAJOR,  $1, $3); }
		| Expression LMINOR Expression {  $$ = new  ineq(cond::op_type::C_LMINOR, $1 , $3); }
		| Expression LMAJOR Expression {  $$ = new  ineq(cond::op_type::C_LMAJOR, $1 , $3); }
		| Expression NEQ Expression { $$ = new  ineq(cond::op_type::C_NEQ,  $1, $3); }
		| Condition OR  Condition  { $$ = new   logic_cond (cond::op_type::C_OR,  $1, $3); }
		| Condition AND  Condition { $$ = new   logic_cond (cond::op_type::C_AND,  $1, $3); }
		| NOT Condition {$$ = new   logic_cond (cond::op_type::C_NOT,  $2); }
		| TIMEVAR MINOR Expression {int id=ris->search_timer_variable($1);
                                    if (id<0)   throw  my_exception (string(string("Error: variable "+string($1))+" is not defined"));
                                    else
                                        $$ = new  ineq(cond::op_type::C_MINOR, (new term_var(id,cond::var_type::CLOCK)) , $3); }
		| TIMEVAR MAJOR Expression {int id=ris->search_timer_variable($1);
                                    if (id<0)   throw  my_exception (string(string("Error: variable "+string($1))+" is not defined"));
                                    else  
                                        $$ = new  ineq(cond::op_type::C_MAJOR, (new term_var(id,cond::var_type::CLOCK)) , $3); }
		| TIMEVAR NEQ Expression {int id=ris->search_timer_variable($1);
                                    if (id<0)   throw  my_exception (string(string("Error: variable "+string($1))+" is not defined"));
                                    else 
                                        $$ = new  ineq(cond::op_type::C_NEQ, (new term_var(id,cond::var_type::CLOCK)) , $3); }
		| TIMEVAR EQ Expression {int id=ris->search_timer_variable($1);
                                    if (id<0)   throw  my_exception (string(string("Error: variable "+string($1))+" is not defined"));
                                    else  
                                        $$ = new  ineq(cond::op_type::C_EQ, (new term_var(id,cond::var_type::CLOCK)) , $3); }
		| TIMEVAR LMINOR Expression {int id=ris->search_timer_variable($1);
                                    if (id<0)   throw  my_exception (string(string("Error: variable "+string($1))+" is not defined"));
                                    else  
                                        $$ = new  ineq(cond::op_type::C_LMINOR, (new term_var(id,cond::var_type::CLOCK)) , $3); }
		| TIMEVAR LMAJOR Expression {int id=ris->search_timer_variable($1);
                                    if (id<0)   throw  my_exception (string(string("Error: variable "+string($1))+" is not defined"));
                                    else  
                                        $$ = new  ineq(cond::op_type::C_LMAJOR, (new term_var(id,cond::var_type::CLOCK)) , $3); }
		| LPARENT Condition RPARENT {$$=$2; }
		;


Expression: 	STRING  {int id=ris->search_state_variable($1); 
                         if (id>=0)   
                            $$ = new term_var(id,cond::var_type::STATE);
                         else{//looking for timer variable
                            throw  my_exception (string(string("Error: variable "+string($1))+" is not defined"));
                            }
                         }
		| NUMBER {$$ = new term_val($1);}
		| Expression TIMES Expression  { $$= new expr(cond::op_type::C_TIMES,  $1, $3); }
		| Expression DIV Expression    { $$= new expr(cond::op_type::C_DIV,  $1, $3); }
		| Expression PLUS Expression   { $$= new expr(cond::op_type::C_PLUS,  $1, $3); }
		| Expression MINUS Expression  { $$= new expr(cond::op_type::C_MINUS,  $1, $3); }
		;



States: 	State 				{ris->add_state($1);}
		| State Spec 			{ris->add_state($1,$2);}
		| State COMMA States 		{ris->add_state($1);}
		| State Spec COMMA States 	{ris->add_state($1,$2);}
		;


State:		STRING  			{$$ = $1;}
		;

Spec:		INIT 				{$$ = $1;}
		| FINAL 			{$$ = $1;}
		;

TimerVars: 	
        | TIMEVAR				{ris->add_timer_var($1);}
		| TIMEVAR COMMA TimerVars 		{ris->add_timer_var($1);}
		;
		

Resets: TIMEVAR  {int id=ris->search_timer_variable($1); if (id<0)  throw  my_exception (string(string("Error: variable "+string($1))+" is not defined")); else   Resets.push_back(id);}
        | TIMEVAR COMMA Resets  {int id=ris->search_timer_variable($1); if (id<0)  throw  my_exception (string(string("Error: variable "+string($1))+" is not defined")); else  Resets.push_back(id);}
        ;
%%
