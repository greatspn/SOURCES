/***************************************************************************
 *   Copyright (C) 2007 by Marco Beccuti   *
 *   beccuti@mfn.unipmn.it   *
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
#include "GenDTMC.h"

static int S_step=0;

void build_transitions(ifstream& in,map <string, map <string,  list <class Elem> > >& trans)
{
bool fine=false;
std::string buffer;
Parser parser;
char delim[]="--> :=";
while ((in)&&(!fine))
	{
	getline(in,buffer,'\n');
	if (buffer=="Transitions :")
		fine=true;
	}
if (!fine)
	{ 
	ExceptionIO obj("Error Transition  has not found");
	throw (obj);
	}
else
	{
	while (in)
		{
		getline(in,buffer,'\n');
		parser.update(delim,buffer);
		//istringstream stream1(parser.get(4)+"-"+parser.get(5));
		std::string prob;
		if (parser.get(4)[parser.get(4).size()-1]=='e')
			prob=parser.get(4)+"-"+parser.get(5);
		else
			prob=parser.get(4);	
//cout<<atof(pippo.c_str())<<endl;
		//double prob=0.0;
		//stream1>>prob;
		trans[parser.get(0)][parser.get(1)].push_front( Elem(parser.get(2),prob));
/***********************************************************************************/
#if TEST1
cout<<trans[parser.get(0)][parser.get(1)].front().get_mark()<<" "<<trans[parser.get(0)][parser.get(1)].front().get_prob() <<endl;
#endif
/***********************************************************************************/
		}
	}

}

void buildDTMCh(std::ifstream& in,std::ifstream& inTE, map <string, map <string,  list <class Elem> > >& tr,char* argv)
{
bool fine=false;
set <std::string> tovisite,newmark,mark,arc,TEmark,TEreached;
Parser parser,parser1;
char delim[]=" :";
char delim1[]="S";
std::string buffer;
buffer="T1S0";
tovisite.insert(buffer);
/* Inserisco le marcature che voglio testare*/
while ((inTE))
	{
	getline(inTE,buffer,'\n');
	if (buffer!="")
		{
		TEmark.insert(buffer);
		}
	}
/* Inserisco le marcature che voglio testare*/
getline(in,buffer,'\n');
//leggo il numero di passi
S_step=atoi(buffer.c_str());
//leggo il numero di passi
for (int i=1;i<=S_step;i++)
	{
	fine=false;
	while ((in)&&(!fine))
		{
		getline(in,buffer,'\n');
		parser.update(delim,buffer);
		if ((parser.get(0)=="Process") && (parser.get(1)=="time"))
			fine=true;
		}
	if (!fine)
		{ 
		ExceptionIO obj("Error Process Time  is not found");
		throw (obj);
		}
	else
		{
		int pos=in.tellg();
		while(!tovisite.empty())
			{
			std::string marking (*tovisite.begin());
			//tovisite.pop();
			tovisite.erase(tovisite.begin());
			mark.insert(marking);
			fine=false;
			while ((in)&&(!fine))
				{
				getline(in,buffer,'\n');
				parser.update(delim,buffer);
				parser1.update(delim1,marking);	
				if (parser.get(0)==parser1.get(0))
					{
					fine=true;
					list <class Elem>::iterator iter;
					iter=tr[parser.get(0)][parser.get(1)].begin();
					while (iter!=tr[parser.get(0)][parser.get(1)].end())
						{
						ostringstream s1;
						s1<<iter->get_mark()<<"S"<<i;
						//
						std::string temp=s1.str();
						if (TEmark.find(iter->get_mark())!=TEmark.end())
							{//raggiungo un elemento TE
							TEreached.insert(s1.str());
							}//raggiungo un elemento TE
						//
						mark.insert(s1.str());
						//if (marking=="T1")
						//	marking="T1S0";
						arc.insert(marking+":"+s1.str()+":"+iter->get_prob());
						newmark.insert(s1.str());
						iter++;
						}
					}
				}
			in.seekg(pos);		
			}
		tovisite=newmark;
		newmark.clear();
		}
	}
try
	{
	writeMDPh(mark,TEreached,arc,argv);
	}
catch(ExceptionIO obj)
	{
	throw (obj);
	}

}




void buildDTMC(std::ifstream& in,std::ifstream& inTE, map <string, map <string,  list <class Elem> > >& tr,char* argv)
{
set <std::string> tovisite,newmark,mark,arc,TEmark;
Parser parser,parser1;
char delim[]=" :";
//char delim1[]="S";
std::string buffer;
//buffer="T1";
tovisite.insert("T1");
/* Inserisco le marcature che voglio testare*/
while ((inTE))
	{
	getline(inTE,buffer,'\n');
	if (buffer!="")
		{
		TEmark.insert(buffer);
		}
	}
/* Inserisco le marcature che voglio testare*/
getline(in,buffer,'\n');
getline(in,buffer,'\n');
getline(in,buffer,'\n');
getline(in,buffer,'\n');
//fine=false;
while ((in))
	{
	getline(in,buffer,'\n');
	parser.update(delim,buffer);
	if (parser.get(0)!="")
		{
		mark.insert(parser.get(0));
		list <class Elem>::iterator iter;
		iter=tr[parser.get(0)][parser.get(1)].begin();
		while (iter!=tr[parser.get(0)][parser.get(1)].end())
			{
			arc.insert(parser.get(0)+":"+iter->get_mark()+":"+iter->get_prob());
			iter++;
			}
		}
	}
try
	{
	writeMDP(mark,TEmark,arc,argv);
	}
catch(ExceptionIO obj)
	{
	throw (obj);
	}
}

void writeMDPh(set<std::string>& mark,set<std::string>&TEreached,set<std::string>& arc,char* argv)
{
Parser parser;
char delim[]="S";

std::string net=std::string(argv)+".sm";
ofstream out(net.c_str(),ofstream::out);
if( !out) 
	{
	ExceptionIO obj("\n*****Error opening output stream .sm*****");
	throw (obj);	
  	}
out<<"print(\"Esempio DTMC:"<<net<<"\");\n dtmc mod1 :=\n\t{\n \t state ";
set <string>::iterator iter=mark.begin();
out<<" "<<*iter;
iter++;
while(iter!=mark.end())
	{
	if (*iter!="")
		{
		out<<","<<*iter;
		iter++;
		}
	}
out<<";\n \t init(T1S0:1.0);\n \t arcs(";
iter=arc.begin();
out<<*iter;
iter++;
while(iter!=arc.end())
	{
	if (*iter!="::")
		{
		out<<", "<<*iter;
		iter++;
		}
	}
out<<");\n";


for (int i=0; i<=S_step;i+10)
	{
	iter=TEreached.begin();
		if (iter!=TEreached.end())
			out<<"real P"<<i<<" := prob_at(in_state("<<*iter<<")";
		while(iter!=TEreached.end())
			{
			//parser.update(delim,*iter);
			//if (atoi(parser.get(1).c_str())==i)
				//{
				//out<<"real P"<<*iter<<"S"<<i<<" := prob_at(in_state("<<*iter<<"),"<< i <<");\n";
				out<<"|in_state("<<*iter<<")";
				//}
			iter++;
			}
		out<<","<<i<<");\n";
	}
out<<"\t};\n";
for (int i=0; i<=S_step;i+10)
	{
	out<<"\nreal x"<<i<<" := 0.0+mod1.P"<<i<<";";
	//iter=TEreached.begin();
	//while(iter!=TEreached.end())
	//	{
	//	parser.update(delim,*iter);
	//		if (atoi(parser.get(1).c_str())==i)
	//			{
	//			out<<" + mod1.P"<<*iter<<"S"<<i;
	//			}
	//	//out<<"\nprint(\"\\nProb. transient:"<<*iter<< ";\",mod1.P"<<*iter<<"A"<<i<<",\"\\n\");";
	//	iter++;
	//	}
	
	//out<<";\nprint(\"\\n***************************\");\n";
	out<<";\nprint(\"\\nProb. step"<<i<<"; \",x"<<i<<",\"\");";
	}
out<<";\nprint(\"\\n\");\n";
}

void writeMDP(set<std::string>& mark,set<std::string>&TEmark,set<std::string>& arc,char* argv)
{
std::string net=std::string(argv)+".sm";
ofstream out(net.c_str(),ofstream::out);
if( !out) 
	{
	ExceptionIO obj("\n*****Error opening output stream .sm*****");
	throw (obj);	
  	}
out<<"print(\"Esempio DTMC:"<<net<<"\");\n dtmc mod1 :=\n\t{\n \t state ";
set <string>::iterator iter=mark.begin();
out<<*iter;
iter++;
while(iter!=mark.end())
	{
	if (*iter!="")
		{
		out<<","<<*iter;
		iter++;
		}
	}
out<<";\n \t init(T1:1.0);\n \t arcs(";
iter=arc.begin();
out<<*iter;
iter++;
while(iter!=arc.end())
	{
	if (*iter!="::")
		{
		out<<", "<<*iter;
		iter++;
		}
	}
out<<");\n";

for (int i=0; i<2600;i+100)
	{
	iter=TEmark.begin();
		while(iter!=TEmark.end())
			{
			out<<"real P"<<*iter<<"A"<<i<<" := prob_at(in_state("<<*iter<<"),"<<i<<");\n";
			iter++;
			}
	}

iter=TEmark.begin();
while(iter!=TEmark.end())
	{
	out<<"real P"<<*iter<<"B := prob_ss(in_state("<<*iter<<"));\n";
	iter++;
	}

out<<"\t};\n";
for (int i=0; i<2600;i+100)
	{
	out<<"\nreal x"<<i<<" := 0.0";
	iter=TEmark.begin();
	while(iter!=TEmark.end())
		{
		out<<" + mod1.P"<<*iter<<"A"<<i;
		//out<<"\nprint(\"\\nProb. transient:"<<*iter<< ";\",mod1.P"<<*iter<<"A"<<i<<",\"\\n\");";
		iter++;
		}
	//out<<";\nprint(\"\\n***************************\");\n";
	out<<";\nprint(\"\\nProb. step"<<i<<"; \",x"<<i<<",\"\");";
	}
out<<";\nprint(\"\\n\");\n";
out<<"real xB := 0.0";
iter=TEmark.begin();
	while(iter!=TEmark.end())
		{
		out<<" + mod1.P"<<*iter<<"B";
		//out<<"\nprint(\"\\nProb. steady state:"<<*iter<< ";\",mod1.P"<<*iter<<"B,\"\\n\");";
		iter++;
		}
out<<";\nprint(\"\\nProb. steady state \",xB,\"\\n\");";
//out<<";\nprint(\"\\n\");";
}


