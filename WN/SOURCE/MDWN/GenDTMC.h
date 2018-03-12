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

#if !(__GEN_H__)
#define __GEN_H__
#include "general.h"
#endif
#include <set>
#include <map>
#include <list>
#include <utility>
#include <fstream>
#include <sstream>
#include <stdlib.h>

//FOR DEBUG
#define TEST1 0
#define TEST2 0
#define TEST3 0
//FOR DEBUG

#define STEP 1
#define STEADY 0

extern "C" {
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"
    extern int  initialize(int  argc,  char **);
    extern int  ntr;
}


using namespace general;
class Elem {
    std::string mark, prob;
public:
    //! Empty constructors
    Elem() {mark = ""; prob = "";};
    //! constructor takes in input a marking and a probability
    Elem(const std::string &mark, const std::string &prob) {this->mark = mark; this->prob = prob;};
    //!Destructor
    ~Elem() {};
    //! \name Get of methods use to access at the data structures
    //@{
    //!returns marking
    std::string get_mark() {return mark;};
    //!returns prob
    std::string get_prob() {return prob;};
    //@}
    //! \name Set of methods use to modify  the data structures
    //@{
    //!sets the marking
    void set_mark(const std::string &mark) {this->mark = mark;};
    //!sets the place position
    void set_prob(const std::string &mark) {this->prob = prob;};
    //@}
};

class State {
    std::string mark;
    double prob;
public:
    //! Empty constructors
    State() {mark = ""; prob = 0.0;};
    //! constructor takes in input a marking and a probability
    State(const std::string &mark, const double &prob) {this->mark = mark; this->prob = prob;};
    //!Destructor
    ~State() {};
    //! \name Get of methods use to access at the data structures
    //@{
    //!returns marking
    std::string get_mark() {return mark;};
    //!returns prob
    double get_prob() {return prob;};
    //@}
    //! \name Set of methods use to modify  the data structures
    //@{
    //!sets the marking
    void set_mark(const std::string &mark) {this->mark = mark;};
    //!sets the place position
    void set_prob(const double &mark) {this->prob = prob;};
    friend bool operator<(class State st1, class State st2) {return st1.prob < st2.prob;};
    //@}
};

//!It reads the MDP transition
void build_transitions(std::ifstream &, map <std::string, map <std::string,  list <class Elem> > > &);
//!It builds the DTMC (steady state analysis).
void buildDTMC(std::ifstream &, std::ifstream &, std::ifstream &,  map <std::string, map <std::string,  list <class Elem> > > &, char *);
//!It writes the DTMC in a file following the smart format (steady state analysis).
void writeMDP(set<std::string> &, set<std::string> &, set <pair < std::string , int> > &, set<std::string> &, char *);
//!It builds the DTMC steady state (transient analysis).
void buildDTMCh(std::ifstream &, std::ifstream &, map <std::string, map <std::string,  list <class Elem> > > &, char *);
//!It builds the DTMC steady state (transient analysis) by input.Observe IntState contans the initial states with their probabilities and step the actual step (time sep).
void buildDTMChI(std::ifstream &in, std::ifstream &inTE, map <std::string, map <std::string,  list <class Elem> > > &tr, char *argv, set < class State> &IntState, int step);
//!It writes the DTMC in a file following the smart format (transient analysis).
void writeMDPh(set<std::string> &, set<std::string> &, set<std::string> &, char *);
//!It writes the DTMC in a file following the smart format (transient analysis) by input.Observe IntState contans the initial states with their probabilities and step the actual step (time step)
void writeMDPhI(set<std::string> &mark, set<std::string> &TEreached, set<std::string> &arc, char *argv, set < class State> &IntState, const int &step);
//!It reads the initial state with thier probabililities and returns the actual step.
int read_Input(std::ifstream &fp_readInput, set < class State> &IntState);
//!It reads the transition rewards
void build_TReward(map <int, int> &TReward);
void name2id();


