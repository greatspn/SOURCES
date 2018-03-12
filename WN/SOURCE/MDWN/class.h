/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
 *   marco@Portatile.network   *
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

#ifndef __GEN_H__
	#define __GEN_H__
	#include "general.h"
#endif

#ifndef __STR_H__
	#define __STR_H__
	#include <string> 
#endif

#ifndef __VEC_H__
	#define __VEC_H__
	#include <vector>
#endif
#ifndef __IOS_H__
	#define __IOS_H__
	#include <iostream>
#endif

#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <list>
#include <utility>
#include <stdlib.h>
//FOR DEBUG
#define TEST 0
#define TEST1 0
#define TEST2 0
#define TEST4 0
#define TEST5 0
#define TEST6 0
#define DEBUG 0
//FOR DEBUG

#define  MDP_weight_is_double 1

//VERY FAST OPTION
#define FAST 0
//VERY FAST OPTION

//Define place name to compute probability
#define _PROB_PLACE "TE|TE"
//Define place name to compute probability

//Define place name to compute the average number of tokens
#define  _NUM_TOKEN_PLACE "awake|P3"
//Define place name to compute the average number of tokens

extern "C" { 
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/const.libmcesrg"

// #include "../../INCLUDE/const.h"§
// #include "./lib_header/struct.h"
// #include "./lib_header/var_ext.h"
// #include "./lib_header/macros.h"
  extern int  initialize (int  argc,  char  **);
  extern int  ntr;
}

#ifdef MDP_weight_is_double
  #include <cfloat>
  typedef double MDP_weight_type;
  const MDP_weight_type MDP_zero_weight = 0.0, MDP_unused_weight = DBL_MAX;
  inline void MDP_round_weight( MDP_weight_type & ww ){
    ww = float( ww );   // is this set of standard values okay?
  }
#else   // weight is int
  #include <climits>
  typedef int MDP_weight_type;
  const MDP_weight_type MDP_zero_weight = 0, MDP_unused_weight = INT_MIN;
  inline void MDP_round_weight( MDP_weight_type & ){}   // trivial for int
#endif


namespace RG2RRG{


using namespace std;

//!Class Elemento
/*! 
This class is stored two string destination, action and two doubles probability and reward.
*/



class Elemento{
 std::string id,id_trans,instance;
 double prob,reward;
 public:
	//! Empty constructors 
	inline Elemento(){id=""; id_trans=""; reward=0.0; prob=0.0;};
	//! constructors take in input the id and the action 
	Elemento(const std::string& id,std::string& id_trans,const double& prob,const double& reward);
	Elemento(const std::string& id,std::string& id_trans,std::string& instance);
	Elemento(const std::string& id,const double& prob);
	//! \name Set of methods use to access at the data structures
  	//@{
 	//!returns id, action and prob
	inline void get(std::string &id,std::string& id_trans,double& prob);
	//!returns id and action
	void get(std::string &id,std::string& id_trans,std::string& instance);
	 //!returns id and prob
	void get(std::string &id,double& prob);
	//!returns id, action, prob, reward
	void get(std::string &id,std::string& id_trans,double& prob,double& reward);
  	//@} 
	//!Destructor
	~Elemento(){};
	void write(void){cout<<"id:"<<id<<" action:"<<id_trans<<" prob:"<<prob<<" reward:"<<reward<<endl;};
};

class ElFormula{
std::string place;
int value;
char oper;
public:
	//! Empty constructors 
	inline ElFormula(){value=0; oper=' ';};
	//! constructors take in input  value and  oper
	ElFormula(const std::string& place,const char& oper,const int& value);
	//! \name Set of methods use to access at the data structures
  	//@{
	//!return  value and  oper
	inline void get(std::string& place,char& oper,int &value);
	//@} 
	//!Destructor
	~ElFormula(){};
};


class Formula{
double reward;
std::string observ;
 
list <ElFormula> listElFormula;
public:
	//! Empty constructors 
	Formula(){ reward=0; observ="";};
	inline Formula(const double& reward){ this->reward=reward;};
	//! \name Set of methods use to set up  the data structures
  	//@{
	//!insert a fromula in the list
	void insert(const ElFormula& el);
	//! set the reward
	inline void set_reward(const double& reward){this->reward=reward;};
	//! set the obsevation probabilities
	inline void set_observ(const std::string& observ){this->observ=observ;};
	//@} 
	//! \name Set of methods use to access at the data structures
  	//@{
	//!return  true if the formula is verificated
	bool test(map<std::string,int>& mapMakring);
	//!return the reward value
	inline double get_reward(){ return reward;};
	//!return the observation probabilities as string
	inline std::string get_observ(){ return observ;};
	//@} 
	//! \name Set of methods to write  the data structures
  	//@{
	//! print the data structures
	void write();
	inline void clear(){ listElFormula.clear();};
	//@} 
	//!Destructor
	~Formula(){};
};



struct EL
{
 int id;
 double reward;

};

class RAG_el
{
 public:
 std::string action;
 double reward;
 RAG_el(){ reward=0; action="";};
};

class RAGcache_el
{
 public:
 std::string action;
 double reward;
 std::string id;
 RAGcache_el(){ reward=0; action=""; id="";};
 RAGcache_el(std::string id,std::string action, double reward){ this->reward=reward; this->action=action; this->id=id;};
};

//!Exception for Input/Output
struct ExceptionIO{};
//!Exception for operation
struct ExceptionOper
{
std::string mess;
public:
	//! Empty constructors 
	ExceptionOper() {mess="";};
	//! constructor takes in input a message
	ExceptionOper(std::string mess) {this->mess=mess;};
	//! \name Get of methods use to access at the data structures
  	//@{
	//!it returns message
	std::string get(void) {return mess;};
	//@}
};


//!This function creates the mapping between the id of transition and its name 
void name2id();
//!This function is used to read the RRG1 and RRG2 from a file
void parser(std::ifstream& ,map<std::string,list<class Elemento> >&);
//!This function reads from a file the reward of the net non-deterministic transitions. Format: name_transition(string) reward(double)
void readreward(char **);
//!This function reads from a file the positions of markings
void markingposition(char **);
//!This function computes the Marking reward. If we want to compute the probability of TE it save in a file the marking id with the place TE marked.
double computeMarkingReward(list <Formula>&,std::ifstream&,int&);
//!It prints the state in prism format according to its description in .cvrs
void print_state(ofstream&,std::ifstream& ,int, bool,int*,int*  );
//!It copy the initial state token distribution according to its description in .cvrs
void setinit(std::ifstream& in, int* placeInit);
//!This function computes the Observation probability distribution.
std::string computeMarkingObs(list <Formula>&,std::ifstream&,const int&,const unsigned int&,const unsigned int&);
//!This function computes the RG_MDP (FAST)
void genRGmdpFast(std::ofstream&,std::ifstream &,std::string,map<std::string,list<class Elemento> > &);
//!This function computes the RG_MDP
void genRGmdp(std::ofstream&,std::ifstream &,std::string,map<std::string,list<class Elemento> > &);
//!This function returns true if _PROB_PLACE  is marked in the input marking
bool testTE(std::ifstream&,int&);
//!This function returns the number of token in place _NUM_TOKEN_PLACE in the input marking
int testTM(std::ifstream&,int&);
//!This function writes in XML format the MDP
void writeMDP(std::ofstream&,std::ofstream&,std::ofstream&,std::ifstream &, map<std::string,list<class Elemento> > &);
//!This function writes in XML format the MDP
void CwriteMDP(std::ofstream&,std::ofstream&,std::ofstream&,std::ifstream &, map<std::string,list<class Elemento> > &);
//!This function writes  the POMDP in Cassandra's format. It takes input the marking file.
void writePOMDP(std::ifstream&,std::ofstream&, map<std::string,list<class Elemento> >&);
//!This function writes  the MDP in Valmari format.
void writexLMDP(std::ofstream& out,std::ifstream& in_marking,map<std::string,list<class Elemento> > &RG);
//!This function writes  the MDP textual format.
void writeTextMDP(std::ofstream&,std::ofstream&,std::ofstream&,std::ifstream &, map<std::string,list<class Elemento> > &);
//!This function writes  the MDP in Prism format.
void writeMDPxPrism(std::ofstream& , std::ifstream &, map<std::string,list<class Elemento> > &);
}//end name space RG2RRG
