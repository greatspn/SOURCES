/***************************************************************************
 *   Copyright (C) 2011 by Marco Beccuti   *
 *   beccuti@di.unito.it   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

%{
#ifndef __PRS_H__
	#define __PRS_H__
	#include "../AUTOMA/parser.h"
#endif

extern "C"
{
#include "AutoLexer.l.h"
#include <stdio.h>
#include <string.h>

	int kkparse(void);
	int kklex(void);
        int kkwrap()
        	{
                return 1;
        	}
	void kkerror(const char *str)
		{
        	fprintf(stderr,"errore: %s\n",str);
		}
}
extern FILE * kkin; //standard yacc from file

using namespace dddRS;

dddRS::RSRGAuto *rs;
map <int, map <int ,class ATRANSITION> > automa;
int source,transition,dest;
map < int, class BOUNDMARKING > tmpPre, tmpPost;
bool pre=false;
set <int> InitSet;
int Bot=-1, Top=-1;


int initAutoma(class RSRGAuto* r)
{
rs = r;
std::string net=rs->getNetName()+std::string(".automaton");
kkin = fopen(net.c_str(),"r");
if (kkin==NULL)
    {
    printf("\n\nError: opening input file: %s\n\n",net.c_str());
    exit(EXIT_FAILURE);
    }
kkparse();
fclose(kkin);
#if DEBUGPARSER
map <int, map < int ,class ATRANSITION> >::iterator iterm = automa.begin();
while (iterm!=automa.end())
	{
	cout<<"****************************************\n";
	cout<<"\t\tSource: "<<iterm->first<<endl;
	map <int, class ATRANSITION>::iterator iterl = iterm->second.begin();
	cout<<"****************************************\n";
	while (iterl!=iterm->second.end())
		{
		cout<<"\tTransition: "<<(iterl->first)<<endl;
		cout<<iterl->second;
		iterl++;
		}	
	iterm++;
	}
	cout<<endl;
#endif
return EXIT_SUCCESS;
}

%}






//YACC GRAMMAR
%union{
int num;
char var[255];
}

%token <num> NUMBER
%token <var> STRING


%%
automa:
	| 
	entry '|' exit '|' edges
	;

edges: 	|
	edges rule
	;

exit:  	NUMBER NUMBER
		{
		Top = $1;
		Bot = $2;
		}
        |
	NUMBER '-'
		{
		Top = $1;
		}
	;

entry:
	|
	entry state
	;
 

state: NUMBER 
		{
		InitSet.insert($1);
		}
	;

rule:
	source  exp trans exp destination 
		{
		ATRANSITION temp(dest);
		temp.setLBoundPre(tmpPre);
		temp.setLBoundPost(tmpPost);
		tmpPre.clear();
		tmpPost.clear();
		automa[source][transition]=temp;
  		}
  	;
source:
	NUMBER
	{
	source=$1;
	pre=true;
	} 
	;
destination:
	NUMBER
	{
	dest=$1;
	} 
	;
trans: 
	STRING
	{
	transition=rs->getTR(std::string($1));
	if (transition<0)
		{
		std::string tmp= "Error in the automaton: " + std::string($1) +std::string(" is not a valid transition\n");
		ExceptionIO obj(tmp);
		throw (obj);
		}
	pre=false;
	}
	;
exp:    '-'
	|formula
  	| '(' formula ')'
	| '(' exp '&' formula ')'
	;

formula:  
  	STRING '<' NUMBER
    		{
		 int p=rs->getPL(std::string($1));
		 if (p<0)
			{
			std::string tmp= "Error in the automaton: " + std::string($1) +std::string(" is not a valid place\n");
			ExceptionIO obj(tmp);
			throw (obj);
			}
		 if (pre)
			tmpPre[p].setMax($3-1);
		 else
			tmpPost[p].setMax($3-1);
    		}
	|
	STRING '>' NUMBER
		{
		int p=rs->getPL(std::string($1));
		if (p<0)
			{
			std::string tmp= "Error in the automaton: " + std::string($1) +std::string(" is not a valid place\n");
			ExceptionIO obj(tmp);
			throw(obj);
			} 
		if(pre)
			tmpPre[p].setMin($3+1);
		else
			tmpPost[p].setMin($3+1);
		}  
	|
  	STRING '=' NUMBER
    		{
		int p=rs->getPL(std::string($1));
		if (p<0)
			{
			std::string tmp= "Error in the automaton: " + std::string($1) +std::string(" is not a valid place\n");
			ExceptionIO obj(tmp);
			throw(obj);
			}
		if (pre)
			tmpPre[p].set($3,$3);
		else
			tmpPost[p].set($3,$3);
		}
  	;
%%  
