/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
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

#include "class.h"
#include "../lib_header/cammini_minimi.h"
#include "../lib_header/GrafoOrientato_ListeAdiacenza.h"
#include "../lib_header/Albero.h"



namespace RG2RRG{

using namespace asd;

using std::cout;
using std::endl;
using std::map;
using std::pair;
extern bool prob,averageTokens;
extern int horizon;
//cancellare
extern map<std::string,list<class Elem> > dati;
//cancellare


extern map<std::string,list<class Elem> > dati;
extern list<std::string> visit_make;
extern map<std::string, double>reachV;
extern map<std::string, struct EL> transName2Id, placesName2Id;
list <Formula> f1;
list <Formula> o1;
//variables for the Bellman-Ford algorithm 
GrafoOrientato_ListeAdiacenza grafo;
map< std::string, vertice* > vMap;
map< std::string, arco* > eMap;
map<std::string,int>MarkingPos;

extern int Min;

//variables for the Bellman-Ford algorithm 
map<std::string,list <class RAGcache_el> > RAGcache;
//for the Bellman-Ford algorithm 


template<class Chiave> void stampa_nodo(nodo<Chiave>* v)
{
    cout << info[v->getChiave()];
    if (v->padre) cout << "(->" << info[(v->padre)->getChiave()] << ")";
    cout << "  ";
}
//for the Bellman-Ford algorithm 


/**************************Class Elememento****************************************/
Elemento::Elemento(const std::string& id, std::string& id_trans, std::string& instance){
	this->id=id;
	this->id_trans=id_trans;
	this->instance=instance;
	this->prob=0.0;
	this->reward=0.0;
}

Elemento::Elemento(const std::string& id,const double& prob){
	this->id_trans="";
	this->id=id;
	this->prob=prob;
	this->reward=0.0;
}

Elemento::Elemento(const std::string& id,std::string& id_trans,const double& prob,const double& reward){
	this->id_trans=id_trans;
	this->id=id;
	this->prob=prob;
	this->reward=reward;
}


void Elemento::get(std::string& id,std::string& id_trans,std::string& instance)
{
	id_trans=this->id_trans;
	id=this->id;
	instance=this->instance;
}


void Elemento::get(std::string& id,double& prob)
{
	prob=this->prob;
	id=this->id;
}




void Elemento::get(std::string& id,std::string& id_trans,double& prob)
{
	id_trans=this->id_trans;
	id=this->id;
	prob=this->prob;
}

void Elemento::get(std::string &id,std::string& id_trans,double& prob,double& reward)
{
	id_trans=this->id_trans;
	id=this->id;
	prob=this->prob;
	reward=this->reward;
}


/****************************Class Elemento**************************************/


/**************************Class ElFormula***********************************/
ElFormula::ElFormula(const std::string& place,const char& oper,const int& value)
{
this->place=place;
this->value=value;
this->oper=oper;
}

void ElFormula::get(std::string& place,char& oper,int& value)
{
value=this->value;
oper=this->oper;
place=this->place;
}
/**************************Class ElFormula***********************************/

/**************************Class Formula***********************************/
void Formula::insert(const ElFormula& el)
{
this->listElFormula.push_front(el);
}

void Formula::write()
{
int value;
char oper;
std::string place;

cout<<"\nReward:"<<reward<<" (";
list<class ElFormula>::iterator iter;
iter=listElFormula.begin();
while (iter!=listElFormula.end())
	{
	iter->get(place,oper,value);
	if (iter==listElFormula.begin())
		cout<<place<<oper<<value;
	else
		cout<<" | "<<place<<oper<<value;
	iter++;
	}
cout<<")\n";
}

bool Formula::test(map<std::string,int>& mapMakring)
{
int value;
char oper;
bool end=true;
std::string place;
if (listElFormula.size()==0) return false;
else
	{
	list<class ElFormula>::iterator iter;
	iter=listElFormula.begin();
	while(iter!=listElFormula.end()&&(end))
		{
		iter->get(place,oper,value);
		switch (oper)
			{
			case '=':
			if (mapMakring[place]!=value) end=false;
			break;
			case '<':
			if (!(mapMakring[place]<value)) end=false;
			break;
			case '>':
			if (!(mapMakring[place]>value)) end=false;
			break;
			default:
			throw (ExceptionOper());
			}
		iter++;
		}
	return end;
	}
}

/**************************Class Formula***********************************/


 

void name2id()
{

for (int i=0;i<ntr;i++)
	{
	transName2Id[tabt[i].trans_name].id=i;
/***********************************************************************************/
#if TEST1
	cout<<tabt[i].trans_name<<" "<<i<<" "<<transName2Id[tabt[i].trans_name].reward<<endl;
#endif
	}
/***********************************************************************************/
#if TEST1
map <std::string,int >::iterator iter;
iter=transName2Id.begin();
while (iter!=transName2Id.end())
	{
	cout<<iter->first<<" "<<" id:"<<iter->second.id<<endl;
	iter++;
	}
#endif
/***********************************************************************************/
for (int i=0;i<npl;i++)
	{
	placesName2Id[tabp[i].place_name].id=i;
	transName2Id[tabp[i].place_name].reward=0;
	}
/***********************************************************************************/
#if TEST2
map <std::string,struct EL>::iterator iter;
iter=placesName2Id.begin();
while (iter!=placesName2Id.end())
	{
	cout<<"Place:"<<iter->first<<" "<<" id:"<<iter->second.id<<endl;
	iter++;
	}
#endif
/***********************************************************************************/
}






void Path(std::ofstream& out,std::string& id1,std::string& id,std::string& action,map<std::string,list<class Elemento> > &RG,double prv_reward)
{
list<class Elemento>::iterator iter1;
std::string idr,actionr,tmpaction,instance;


iter1=RG[id].begin();
while (iter1!=RG[id].end())
		{
		iter1->get(idr,tmpaction,instance);
		if (instance.compare("()")==0)
			actionr=action+";"+tmpaction;
		else
			actionr=action+";"+tmpaction+instance;	
		if (idr.find("T")!=std::string::npos)
			{
			//*out<<" "<<id1<<" ->"<<" "<<idr<<" [color=dimgray label=\" "<<actionr<<"\"];\n";
			if(vMap[idr]==NULL)
				{
				vMap[idr] = grafo.aggiungiVertice();
				info[ vMap[idr] ] = idr;
				}
			eMap[tmpaction] = grafo.aggiungiArco(vMap[id], vMap[idr],transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward+prv_reward);
			//if (instance.compare("()")==0)	
				info1[eMap[tmpaction]]=actionr;
			//else
			//	info1[eMap[tmpaction]]=actionr;
			//visit_make.push_front(idr);
			}	
		else
			{
			//if(vMap[idr]==NULL)
			//	{
			//	vMap[idr] = grafo.aggiungiVertice();
			//	info[ vMap[idr] ] = idr;
			//	}
			//eMap[tmpaction] = grafo.aggiungiArco(vMap[id], vMap[idr],transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward);
			//info1[eMap[tmpaction]]=tmpaction+instance;
			Path(out,id1,idr,actionr,RG,transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward+prv_reward);
			}
		iter1++;
		}
}


void PathFast(std::ofstream& out,std::string& id1,std::string& id,std::string& action,map<std::string,list<class Elemento> > &RG,double prv_reward, map<std::string,class RAG_el>&RAG)
{
list<class Elemento>::iterator iter1;
std::string idr,actionr,tmpaction,instance;

//cout<<"size of "<< id<<" "<<RG[id].size()<<endl;
//int i;

iter1=RG[id].begin();
while (iter1!=RG[id].end())
		{
		iter1->get(idr,tmpaction,instance);
		if (instance.compare("()")==0)
			actionr=action+";"+tmpaction;
		else
			actionr=action+";"+tmpaction+instance;	
		if (idr.find("T")!=std::string::npos)
			{
			double reward=transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward+prv_reward;
			if(RAG[idr].action=="")
					{
					RAG[idr].reward=reward;
					RAG[idr].action=actionr;
					}
			else
				if (Min)
					{
					if (RAG[idr].reward<reward)
						{
						RAG[idr].reward=reward;
						RAG[idr].action=actionr;
						}
					}
				else
					{
					if (RAG[idr].reward>reward)
						{
						RAG[idr].reward=reward;
						RAG[idr].action=actionr;
						}
				}
			}	
		else
			{
			PathFast(out,id1,idr,actionr,RG,transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward+prv_reward,RAG);
			}
		iter1++;
		}
}

void PathVeryFast(std::ofstream& out,std::string& id1,std::string& id,std::string& action,map<std::string,list<class Elemento> > &RG,double prv_reward, map<std::string,class RAG_el>&RAG)
{
list<class Elemento>::iterator iter1;
std::string idr,actionr,tmpaction,instance;

//cout<<"size of "<< id<<" "<<RG[id].size()<<endl;
//int i;
iter1=RG[id].begin();

while (iter1!=RG[id].end())
		{
		iter1->get(idr,tmpaction,instance);
		if (instance.compare("()")==0)
			actionr=action+";"+tmpaction;
		else
			actionr=action+";"+tmpaction+instance;	
		if (idr.find("T")!=std::string::npos)
			{
			double reward=transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward+prv_reward;
			std::string TmpAction(tmpaction);
			if (instance.compare("()")!=0)
							{
							TmpAction+=instance;
							}
			class RAGcache_el tmp(idr,TmpAction,transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward);
			if(RAG[idr].action=="")
					{
					RAG[idr].reward=reward;
					RAG[idr].action=actionr;
					RAGcache[id].push_back(tmp);
					}
			else
				if (Min)
					{
					if (RAG[idr].reward<reward)
						{
						RAG[idr].reward=reward;
						RAG[idr].action=actionr;
						RAGcache[id].push_back(tmp);
						}
					}
				else
					{
					if (RAG[idr].reward>reward)
						{
						RAG[idr].reward=reward;
						RAG[idr].action=actionr;
						RAGcache[id].push_back(tmp);
						}
					}
			}	
		else
			{
			if (RAGcache.find(idr)!=RAGcache.end())
				{
				list<class RAGcache_el>::iterator iter5=RAGcache[idr].begin();
				while (iter5!=RAGcache[idr].end())
					{
					double reward=transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward+prv_reward;
					if(RAG[iter5->id].action=="")
					{
					RAG[iter5->id].reward=reward+iter5->reward;
					RAG[iter5->id].action=actionr+iter5->action;
					}
					else
					if (Min)
						{
						if (RAG[iter5->id].reward<(reward+iter5->reward))
							{
							RAG[iter5->id].reward=reward+iter5->reward;
							RAG[iter5->id].action=actionr+iter5->action;
							}
						}
						else
							{
							if (RAG[iter5->id].reward>(reward+iter5->reward))
								{
								RAG[iter5->id].reward=reward+iter5->reward;
								RAG[iter5->id].action=actionr+iter5->action;
								}
							}
					iter5++;
					}
				}
			else
				{
				PathVeryFast(out,id1,idr,actionr,RG,transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward+prv_reward,RAG);
				list<class RAGcache_el>::iterator iter5=RAGcache[idr].begin();
				while (iter5!=RAGcache[idr].end())
					{
					if  (iter5->action!="")
						{
						class RAGcache_el tmp(iter5->id,iter5->action,iter5->reward);
						RAGcache[id].push_back(tmp);
						}
					iter5++;
					}
				}
			}
		iter1++;
		}

}

void genRGmdp(std::ofstream& out,std::ifstream& in_marking,std::string sour_mark,map<std::string,list<class Elemento> > &RG)
{
list<class Elemento>::iterator iter1;
std::string idr,id_trans,dest_mark,instance;
double prob;

map<std::string,int> visitOLD;
visit_make.push_front(sour_mark);
visitOLD["T1"]=1;
//int tot=0;
//int i=0;
while (!visit_make.empty())
	{
	sour_mark=visit_make.front();
	visit_make.pop_front();
	while (!RG[sour_mark].empty())
		{
		RG[sour_mark].front().get(idr,id_trans,instance);
		//Bellman-Ford algorithm 
		//vMap[idr] = grafo.aggiungiVertice();
		//info[ vMap[idr] ] = idr;
		//eMap[id_trans] = grafo.aggiungiArco(vMap[sour_mark], vMap[idr],transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward);
		if (instance.compare("()")!=0)
			{
			id_trans+=instance;
		//	info1[eMap[id_trans]]=id_trans+instance;
			}
		else
			info1[eMap[id_trans]]=id_trans;
		Path(out,sour_mark,idr,id_trans,RG,transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward);
		//Bellman-Ford algorithm 
		RG[sour_mark].pop_front();
		}
	//Bellman-Ford algorithm 
	vertice* v;
	//arco* e;
	vertice_array<struct rew_act> distanzeBellmanFord;
	if (Min)
		distanzeBellmanFord = BellmanFordMin(grafo, vMap[sour_mark]);
	else
		distanzeBellmanFord = BellmanFordMax(grafo, vMap[sour_mark]);
	for_each_vertice(v,grafo)
			{
			if (info[v].find("T")!=std::string::npos)
				{
				list<class Elemento>::iterator iter1;
				iter1=RG[info[v]].begin();
/***********************************************************************************/
#if TEST7
				out<<"Source: "<<sour_mark<<endl;
#endif
/***********************************************************************************/	
				while (iter1!=RG[info[v]].end())
					{
					iter1->get(dest_mark,prob);
					if (visitOLD[dest_mark]==0)
						{
						visit_make.push_front(dest_mark);
						visitOLD[dest_mark]=1;
						}
/*Compute reward*/
					double reward=distanzeBellmanFord[vMap[info[v]]].reward+computeMarkingReward(f1,in_marking,MarkingPos[dest_mark]);
/*Compute reward*/
					
					Elemento elem( dest_mark,distanzeBellmanFord[vMap[info[v]]].action,prob,reward);
/***********************************************************************************/
#if TEST7
					out<<"\tdestination: "<<dest_mark<<" action: "<<distanzeBellmanFord[vMap[info[v]]].action<<" probability: "<<prob<<" reward: "<<reward<<endl;
#endif
/***********************************************************************************/	
					RG[sour_mark].push_front(elem);
					iter1++;
					}
				}
			}
	grafo.clear();
	info.clear();
	info1.clear();
	vMap.clear();
	eMap.clear();
	//Bellman-Ford algorithm 
/***********************************************************************************/
//#if TEST6
	list<std::string>::iterator iter2;
	iter2=visit_make.begin();
cout<<"Visit_make: ";
	while(iter2!=visit_make.end())
		{
		cout<<*iter2<<" ";
		iter2++;
		}
	cout<<endl;
//#endif
/***********************************************************************************/	
	}
//free the RG states  that are not in MDP
cout<<"Total number of RG states: "<<RG.size()<<endl;
map<std::string,list<class Elemento> >::iterator iter3=RG.begin(),iter4;
while (iter3!=RG.end())
	{
	if (visitOLD[iter3->first]==0) 
		{
		iter4=iter3;
		iter3++;
		RG.erase(iter4);
		}
	else
	iter3++;
	}
cout<<"Total number of MDP states: "<<RG.size()<<endl;
//free the RG states  that are not in MDP
}


void genRGmdpFast(std::ofstream& out,std::ifstream& in_marking,std::string sour_mark,map<std::string,list<class Elemento> > &RG)
{
list<class Elemento>::iterator iter1;
std::string idr,id_trans,dest_mark,instance;
double prob;

map<std::string,int> visitOLD;
visit_make.push_front(sour_mark);
visitOLD["T1"]=1;
//int tot=0;
//int i=0;
map<std::string,class RAG_el>RAG;
while (!visit_make.empty())
	{
	sour_mark=visit_make.front();
	visit_make.pop_front();
	while (!RG[sour_mark].empty())
		{
		RG[sour_mark].front().get(idr,id_trans,instance);
		if (instance.compare("()")!=0)
			{
			id_trans+=instance;
			}
#if FAST
		PathVeryFast(out,sour_mark,idr,id_trans,RG,transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward,RAG);
#endif
#if !FAST
		PathFast(out,sour_mark,idr,id_trans,RG,transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward,RAG);
#endif
		RAGcache.clear();	
		RG[sour_mark].pop_front();
		}
	map<std::string,class RAG_el>::iterator iterRAG;
	iterRAG=RAG.begin();
	while(iterRAG!=RAG.end())
		{
		list<class Elemento>::iterator iter1;
		iter1=RG[iterRAG->first].begin();
		while (iter1!=RG[iterRAG->first].end())
			{
			iter1->get(dest_mark,prob);
			if (visitOLD[dest_mark]==0)
				{
				visit_make.push_front(dest_mark);
				visitOLD[dest_mark]=1;
				}
			Elemento elem(dest_mark,iterRAG->second.action,prob,iterRAG->second.reward+computeMarkingReward(f1,in_marking,MarkingPos[dest_mark]));
			if (iterRAG->second.action=="")
				exit(1);
			RG[sour_mark].push_front(elem);
			iter1++;
			}
		iterRAG++;
		}
	RAG.clear();
/***********************************************************************************/
#if TEST6
	list<std::string>::iterator iter2;
	iter2=visit_make.begin();
cout<<"Visit_make: ";
	while(iter2!=visit_make.end())
		{
		cout<<*iter2<<" ";
		iter2++;
		}
	cout<<endl;
#endif
/***********************************************************************************/	
	}
//free the RG states  that are not in MDP
cout<<"Total number of RG states: "<<RG.size()<<endl;
map<std::string,list<class Elemento> >::iterator iter3=RG.begin(),iter4;
while (iter3!=RG.end())
	{
	if (visitOLD[iter3->first]==0) 
		{
		iter4=iter3;
		iter3++;
		RG.erase(iter4);
		}
	else
	iter3++;
	}
cout<<"Total number of MDP states: "<<RG.size()<<endl;
//free the RG states  that are not in MDP
}



void writePOMDP(std::ifstream& in_marking,std::ofstream& out,map<std::string,list<class Elemento> > &RG)
{

std::string id,action;
double prob,reward;



out<<"#Generate autamatically by MDWN\ndiscount: 0.90\nvalues: cost\n";
//discount and values will be parameters in the future.
//print state list
out<<"states:";
map <std::string,list<class Elemento> >::iterator iter=RG.begin();

while (iter!=RG.end())
	{
	out<<" "<<iter->first;
	iter++;
	}
out<<"\nactions:";
set <std::string> setAction;

//reset the iterator
iter=RG.begin();
//reset the iterator

while (iter!=RG.end())
	{
	list<class Elemento>::iterator iter1;
	iter1=iter->second.begin();
	if (iter1==iter->second.end())
			{
			cerr<<"Found an state "<<iter->first<<" without associated action"<<endl;
			exit(1);
			}
	while (iter1!=iter->second.end())
		{
		iter1->get(id,action,prob,reward);
		class general::Parser par,par1;
		char delim[]=";";
		char delim1[]="()";
		std::string tmp;
		par.update(delim,action);
		for (unsigned int i=0;i<par.size();i++)
			{
			std::string tmp1= par.get(i);
			par1.update(delim1,par.get(i));
			tmp=std::string(tmp+tabt[atoi(par1.get(0).c_str())].trans_name+"Z"+par1.get(1)+"Z");
			}
		if ((setAction.insert(tmp)).second==TRUE)
			{
			out<<" "<<tmp;
			}
		iter1++;
		}
	iter++;
	}

//free memory
setAction.clear();
//free memory

//Put here the observations
out<<"\nobservations: "<<RG.size();
//Put here the observations

//reset the iterator
iter=RG.begin();
//reset the iterator
int index=0;
while (iter!=RG.end())
	{
	list<class Elemento>::iterator iter1;
	iter1=iter->second.begin();
	while (iter1!=iter->second.end())
		{
		iter1->get(id,action,prob,reward);
		out<<"O: " << computeMarkingObs(o1,in_marking,MarkingPos[id],index,RG.size())<<endl;
		std::string tmp;
		class general::Parser par,par1;
		char delim[]=";";
		char delim1[]="()";
		par.update(delim,action);
		for (unsigned int i=0;i<par.size();i++)
			{
			std::string tmp1= par.get(i);
			par1.update(delim1,par.get(i));
			tmp=std::string(tmp+tabt[atoi(par1.get(0).c_str())].trans_name+"Z"+par1.get(1)+"Z");
			}
		out<<"T: "<<tmp<<" : "<<iter->first<< " : " << id  << " " << prob<<endl;
		out<<"R: "<<tmp<<" : "<<iter->first<< " : " << id  << ":* " << reward<<".0"<<endl;
		iter1++;
		}
	iter++;
	index++;
	}

}


void writeMDP(std::ofstream& out,std::ofstream& out_prob,std::ofstream& out_avT, std::ifstream &in_marking , map<std::string,list<class Elemento> > &RG)
{
int i=0,num_token=0;
map <std::string,int> id2int;
std::string id,action;
double prob,reward;

out<<"<?xml version = \"1.0\"?>\n<!DOCTYPE MDP[\n<!ELEMENT ACTION ( LABEL, TRANSITIONS ) >\n<!ELEMENT ACTIONS ( ACTION )+ >\n<!ELEMENT ENDING_STATE ( #PCDATA ) >\n<!ELEMENT LABEL ( #PCDATA ) >\n<!ELEMENT MDP ( STATES, ACTIONS ) >\n<!ATTLIST MDP stationary NMTOKEN #REQUIRED >\n<!ATTLIST MDP  Infinite NMTOKEN #REQUIRED >\n<!ATTLIST MDP  InfiniteCriteriumType  (DISCOUNTED |AVERAGE_REWARD) #IMPLIED >\n<!ATTLIST MDP  DecompositionAlgorithmType NMTOKEN #IMPLIED >\n<!ATTLIST MDP  Horizon NMTOKEN #IMPLIED >\n<!ATTLIST MDP  Optimization_algorithm (POLICY_ITERATION | VALUE_ITERATION | LINEAR_PROGRAMMING) #IMPLIED >\n<!ELEMENT PROBABILITY ( #PCDATA ) >\n<!ELEMENT REWARD ( #PCDATA ) >\n<!ELEMENT STARTING_STATE ( #PCDATA ) >\n<!ELEMENT STATE ( LABEL ) >\n<!ELEMENT STATES ( STATE )+ >\n<!ELEMENT TRANSITIONS (TRANSITION)+ >\n<!ELEMENT TRANSITION ( STARTING_STATE, ENDING_STATE, PROBABILITY, REWARD ) >\n]>";

out<<"\n\n<MDP stationary=\"yes\" Infinite=\"true\" InfiniteCriteriumType=\"AVERAGE_REWARD\" Horizon=\""<<horizon<<"\" Optimization_algorithm=\"LINEAR_PROGRAMMING\">\n\t<STATES>";

map <std::string,list<class Elemento> >::iterator iter;
iter=RG.begin();
i=0;

while (iter!=RG.end())
	{
	out<<"\n\t\t<STATE>\n\t\t\t<LABEL>"<<iter->first<<"</LABEL>\n\t\t</STATE>";
	id2int[iter->first]=i;
	if ((prob)&&(testTE(in_marking,MarkingPos[iter->first])))
		{
		out_prob<<iter->first<<endl;
		}
	if ((averageTokens)&&((num_token=testTM(in_marking,MarkingPos[iter->first]))>0))
		{
		out_avT<<iter->first<<" "<<num_token<<endl;
		}
	iter++;
	i++;
	}
out<<"\n\t</STATES>\n\t<ACTIONS>\n";
iter=RG.begin();
i=0;
while (iter!=RG.end())
	{
	list<class Elemento>::iterator iter1;
	iter1=iter->second.begin();
	std::string compare("$");
	if (iter1==iter->second.end())
			{
			cout<<"Found an state "<<iter->first<<" without associated action"<<endl;
			exit(1);
			}
	while (iter1!=iter->second.end())
		{
		iter1->get(id,action,prob,reward);
		if (action!=compare)
			{
			//cout<<tot<<endl;
			if (compare!="$")
			out<<"\t\t\t</TRANSITIONS>\n\t\t</ACTION>\n";
			out<<"\t\t<ACTION>\n\t\t\t<LABEL>"<<action<<"{"<<i<<"}"<<"</LABEL>\n\t\t\t<TRANSITIONS>\n\t\t\t\t<TRANSITION>\n";
			out<<"\t\t\t\t\t<STARTING_STATE>"<<i<<"</STARTING_STATE>\n";
			out<<"\t\t\t\t\t<ENDING_STATE>"<<id2int[id]<<"</ENDING_STATE>\n";
			out<<"\t\t\t\t\t<PROBABILITY>"<<prob<<"</PROBABILITY>\n";
			out<<"\t\t\t\t\t<REWARD>"<<reward<<"</REWARD>\n\t\t\t\t</TRANSITION>\n";
			}
		else
			{
			out<<"\t\t\t\t<TRANSITION>\n";
			out<<"\t\t\t\t\t<STARTING_STATE>"<<i<<"</STARTING_STATE>\n";
			out<<"\t\t\t\t\t<ENDING_STATE>"<<id2int[id]<<"</ENDING_STATE>\n";
			out<<"\t\t\t\t\t<PROBABILITY>"<<prob<<"</PROBABILITY>\n";
			out<<"\t\t\t\t\t<REWARD>"<<reward<<"</REWARD>\n\t\t\t\t</TRANSITION>\n";
			}
		compare=action;
		iter1++;
		}
	out<<"\t\t\t</TRANSITIONS>\n\t\t</ACTION>\n";
	iter++;
	i++;

	}
	out<<"\t</ACTIONS>\n</MDP>";
}


double computeMarkingReward(list <Formula>& g,std::ifstream& in,int& pos)
{
//unsigned int offset=0;
double reward=0;
map<std::string,int> mapMarking;
//bool end;
in.seekg(pos);
if (in)
	{
	//in.getline(buffer,200);
	std::string str;
	getline(in,str,'\n');
/***********************************************************************************/
#if TEST1
	cout<<str<<endl;
#endif
/***********************************************************************************/
	unsigned int i=0;
	char delim[]="()";
	class general::Parser parser(delim,str);
	while(i<parser.size())
		{
		reward=atoi(parser.get(i+1).c_str())*placesName2Id[tabp[atoi((parser.get(i)).c_str())].place_name].reward+reward;
		mapMarking[tabp[atoi(parser.get(i).c_str())].place_name]=atoi(parser.get(i+1).c_str());
		i=i+2;
/*UPDATE 18/05/2009
	end=false;
	reward=0;
	while(!end)
		{

		std::string place(str.substr(offset,str.find_first_of("(",offset)-offset));
		std::string tokens(str.substr(str.find_first_of("(",offset)+1,str.find_first_of(")",offset)-(offset+2)));
		offset=str.find_first_of(")",offset+1)+1;
		reward=atoi(tokens.c_str())*placesName2Id[tabp[atoi(place.c_str())].place_name].reward+reward;
		mapMarking[tabp[atoi(place.c_str())].place_name]=atoi(tokens.c_str());
		if(offset==str.size()) end=true;
UPDATE 18/05/2009*/

/***********************************************************************************/
#if TEST1
		cout<<place<<" "<<atoi(tokens.c_str())<<"\n";
#endif
/***********************************************************************************/
		}
	}
else throw (ExceptionIO());

list <Formula >::iterator iter;
iter=g.begin();
while (iter!=g.end())
	{
	if (iter->test(mapMarking)) reward+=iter->get_reward();
	iter++;
	}
mapMarking.clear();
return reward;
}

std::string computeMarkingObs(list <Formula>& g,std::ifstream& in,const int& pos,const unsigned int& index,const unsigned& size)
{
//unsigned int offset=0;
//double reward=0;
map<std::string,int> mapMarking;
//bool end;
in.seekg(pos);
if (in)
	{
	//in.getline(buffer,200);
	std::string str;
	getline(in,str,'\n');
/***********************************************************************************/
#if TEST1
	cout<<str<<endl;
#endif
/***********************************************************************************/
	unsigned int i=0;
	char delim[]="()";
	class general::Parser parser(delim,str);
	while(i<parser.size())
		{
		mapMarking[tabp[atoi(parser.get(i).c_str())].place_name]=atoi(parser.get(i+1).c_str());
		i=i+2;
/***********************************************************************************/
#if TEST1
		cout<<place<<" "<<atoi(tokens.c_str())<<"\n";
#endif
/***********************************************************************************/
		}
	}
else throw (ExceptionIO());

list <Formula >::iterator iter;
iter=g.begin();
while (iter!=g.end())
	{
	if (iter->test(mapMarking))
			{
			std::string tmp(" ");
			if (iter->get_observ()=="T")
				{
				for (unsigned int i=0;i<size;i++)
					{
					if(i==index)
						{
						tmp=tmp+"1 ";
						}
						else
						{
						tmp=tmp+"0 ";
						}
					}
				}
			else 
				if(iter->get_observ()=="u")
					tmp="uniform";
			     	else
					{
					char delim[]=" ";
					class general::Parser parser(delim,iter->get_observ());
					tmp=iter->get_observ();
					if (parser.size()!=size)
						{//in this way the probability distribution is automatically complited
						for (unsigned int i=parser.size(); i<=size; i++)
							{
							tmp=tmp+" 0";
							}
						}//in this way the probability distribution is automatically complited	
					}
			
			mapMarking.clear();
			return tmp;
			}
	iter++;
	}
//ERROR
mapMarking.clear();
return "ERROR!!!";
}



bool testTE(std::ifstream& in,int& pos)
{
in.seekg(pos);
if (in)
	{
	//in.getline(buffer,200);
	std::string str;
	getline(in,str,'\n');
/***********************************************************************************/
#if TEST1
	cout<<str<<endl;
#endif
/***********************************************************************************/	unsigned int i=0;
	char delim[]="()";
	class general::Parser parser(delim,str);
	while(i<parser.size())
		{
		if (!strcmp(tabp[atoi(parser.get(i).c_str())].place_name, _PROB_PLACE))
			{
			return true;
			}
		i=i+2;
		}
	return false;
	}
else throw (ExceptionIO());
}





int testTM(std::ifstream& in,int& pos)
{
in.seekg(pos);
if (in)
	{
	//in.getline(buffer,200);
	std::string str;
	getline(in,str,'\n');
/***********************************************************************************/
#if TEST1
	cout<<str<<endl;
#endif
/***********************************************************************************/
	unsigned int i=0;
	char delim[]="()";
	class general::Parser parser(delim,str);
	while(i<parser.size())
		{
		if (!strcmp(tabp[atoi(parser.get(i).c_str())].place_name,_NUM_TOKEN_PLACE))
			{
			return atoi(parser.get(i+1).c_str());
			}
		i=i+2;
		}
	return 0;
	}
else throw (ExceptionIO());
return false;
}


void parser(std::ifstream& in,map<std::string,list<class Elemento> > &RG)
{
std::string sour_mark,dest_mark,id_trans,instance;
int num_reached;
double prob;

while (in)
	{
	in>>sour_mark>>num_reached;

/***********************************************************************************/
#if TEST6
cout<<sour_mark<<" "<<num_reached<<endl;
#endif
/***********************************************************************************/

	if (num_reached==-1)
		{
		in>>dest_mark>>id_trans>>instance;
/***********************************************************************************/
#if TEST6
	cout<<"\t"<<dest_mark<<" action: "<<id_trans<<" instance:"<<instance<<endl;
#endif
/***********************************************************************************/	
		Elemento elem(dest_mark,id_trans,instance);
		RG[sour_mark].push_front(elem);
		}
	else
		{
		while(num_reached!=0)
			{
			in>>dest_mark>>prob;
			Elemento elem(dest_mark,prob);
			RG[sour_mark].push_front(elem);
/***********************************************************************************/
#if TEST6
	cout<<"\t"<<dest_mark<<" prob: "<<prob<<endl;
#endif
/***********************************************************************************/	
			num_reached--;
			}
		}
	
	}


/***********************************************************************************/
#if TEST6
map <std::string,list<class Elemento> >::iterator iter;
iter=RG.begin();
while (iter!=RG.end())
	{
	list<class Elemento>::iterator iter1;
	iter1=iter->second.begin();
	while (iter1!=iter->second.end())
		{
		iter1->get(dest_mark,id_trans,prob);
		cout<<"Source:"<<iter->first<<" Destination: "<<dest_mark<<" Transition:"<<id_trans<<"  Prob:"<<prob<<endl;
		iter1++;
		}
	iter++;
	}
#endif
/***********************************************************************************/
}

void markingposition(char *argv[])
{
std::string marking;
int pos;
std::string net= std::string(argv[1])+".cvrsoff";
ifstream in(net.c_str(),ifstream::in);
if( !in) throw(ExceptionIO()); 
while (in)
	{
	in>>marking>>pos;
	MarkingPos[marking]=pos;
	}
}


void readreward(char *argv[])
{
std::string net= std::string(argv[1])+".reward";
ifstream in(net.c_str(),ifstream::in);
std::string transition,place;
char type,oper;
double reward;
int value;
Formula forum;
//file does not exist
if( !in) throw(ExceptionIO());
//file does not exist
while (in)
	{
	in>>type;
	switch (type)
		{
		case 'T':
		case 't':
			in>>transition>>reward;
			transName2Id[transition].reward=reward;
		break;
		case 'P':
		case 'p':
			in>>place>>reward;
			placesName2Id[place].reward=reward;
		break;
		//and formule
		case 'F':
		case 'f':
			in>>reward>>type;
			type='|';
			forum.set_reward(reward);
			while ((type=='|')&&(in))
				{
				in>>place>>oper>>value>>type;
				ElFormula el(place,oper,value);
				forum.insert(el);
				}
			f1.push_front(forum);
			forum.clear();		
		break;
		//and formule
//Observability
		case 'O':
		case 'o':
		//read probability distribution of observations
		double prob=0.0;
		std::string tmp;
		getline(in,tmp);
		class general::Parser par;
		char delim[]=" ";
		par.update(delim,tmp);
		unsigned int i=0;
		while (par.get(i)!="(" && par.get(i)!="T" && par.get(i)!="t")
			{
			tmp=+" "+par.get(i);
			prob=+atoi(tmp.c_str());
			i++;
			}
		if (par.get(i)=="t" || par.get(i)=="T")
			{
			prob=1;	
			tmp="T";
			}
		if (prob==0.0 || prob>1)
			{
			stringstream s1;
			s1<<"Error Obs:" << o1.size() << "probability sum > 1 or <= 0";
			throw(ExceptionOper(s1.str()));
			}
		i=i+2;
		forum.set_observ(tmp);
		while (i< (par.size()-1))
			{
			ElFormula el(par.get(i),(par.get(i+1).c_str())[0],atoi(par.get(i+2).c_str()));
			forum.insert(el);
			i=i+3;
			}
		//QUI!!!
			o1.push_front(forum);
			forum.clear();	
//Observability 
		break;
		}
	}
in.close();

/***********************************************************************************/
#if TEST5
cout<<transName2Id.size()<<endl;
map <std::string,struct EL>::iterator iter;
iter=transName2Id.begin();
while (iter!=transName2Id.end())
	{;
	cout<<"Transition:"<<iter->first<<" reward"<<iter->second.reward<<endl;
	iter++;
	}
iter=placesName2Id.begin();
while (iter!=placesName2Id.end())
	{;
	cout<<"Place:"<<iter->first<<" reward"<<iter->second.reward<<endl;
	iter++;
	}
cout<<transName2Id.size()<<endl;
//f1->write();
#endif
/***********************************************************************************/
}

}//end namespace RG2RRG
