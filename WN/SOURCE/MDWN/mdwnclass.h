/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
 *   beccuti@mfn.unipmn.it  *
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
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <list>
#include <utility>
#include <stdlib.h>
#ifndef __GEN_H__
#define __GEN_H__
#include "general.h"
#endif


//FOR DEBUG
#define TEST1 0
#define TEST2 0
#define TEST3 0
//FOR DEBUG


#define STEMP 1

#define _COLORDEF "@c"
#define _MARKINGDEF "@m"
//!Id controllable components
#define _COMP_CON_COD "CC"
//!Id passive components
#define _COMP_NOTCON_COD "NC"
//!Id not controllable components
#define _COMP_GL_COD "GC"

#define _PLACE_POSITION 1
#define _TRANS_POSITION 3


namespace ParserDefNet {

using namespace std;

using namespace general;

//!Exception for components
struct ExceptionComponents {};
//!Struct NOOBJC
/*!
Used to store the net information
*/
struct NOOBJS {
//!number of marking parameters
    unsigned int NM;
//! number of places
    unsigned int NP;
//!number of rate parameters
    unsigned int NR;
//! number of transitions
    unsigned int NT;
//! number of groups
    unsigned int NG;
//! number of levels
    unsigned int NL;
};

enum Type {
    PROB = 0,
    NOND = 1
};

//!Struct TCS
/*!
Used to store the transition information
*/
struct TCS {
//!type transition: RUN/STOP
    std::string type;
//!id component
    std::string comp;
//! arc expression
    std::string arc;
};


//!Class Component
/*!
This class is used to save the domain and the arc label of a component.
*/
class component {
    std::string domain;
    std::string arclabel;
    int pos;//!initial marking
    int place;//!place position
public:
    //! Empty constructors
    component() {domain = ""; arclabel = ""; pos = -1; place = -1;};
    //! constructor takes in input the domain and the arc label component
    component(const string &domain, const string &arclabel) {this->domain = domain; this->arclabel = arclabel; pos = -1; place = -1;};
    //! constructor takes in input the arc label component
    component(const string &arclabel) {domain = ""; this->arclabel = arclabel; pos = -1; place = -1;};
    //! constructor takes in input the arc label component and the position
    component(const string &arclabel, int pos) {domain = ""; this->arclabel = arclabel; this->pos = pos; place = -1; };
    component(int place) {domain = ""; arclabel = ""; pos = -1; this->place = place;};
    //!Destructor
    ~component() {};
    //! \name Get of methods use to access at the data structures
    //@{
    //!returns domain
    std::string get_domain() {return domain;};
    //!returns arclabel
    std::string get_arclabel() {return arclabel;};
    //!returns pos
    int get_pos() {return pos;};
    //!returns place
    int get_place() {return place;};
    //@}
    //! \name Set of methods use to modify  the data structures
    //@{
    //!sets the domain
    void set_domain(const std::string &domain) {this->domain = domain;};
    //!sets the place position
    void set_place(const int &place) {this->place = place;};
    //@}
};

//!This function reads the file .def recovering the component types. The components types are saved CompAtt(active components) and CompPas (passive components).
void readComponent(ifstream &, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas);
void readComponentNew(ifstream &in, map<std::string, class component> &CompAtt, map <std::string, class component> &CompPas);
//!This function takes in input the arclabel and the color classes and returns the domain string
std::string arclabelTodomain(std::string , map <std::string, std::string> &color);

//!This function  builds the new file .net inserting the run and stop places (calling AddPlace()) and updating the transition with the appropriated arc (calling change_trans()) and inserting the transitions Nd2Pr and Pr2Nd. The  type is using to distinguish between the probabilistic and non deterministic net
void create_net(ifstream &in, ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, Type type);
void create_netNew(ifstream &in, ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, Type type);

//!This function  sets  the correct  number of place and transition
struct NOOBJS AddPlaceTransNum(ofstream &out, std::string buffer, const unsigned int &sizeCompAtt, const unsigned int &sizeCompPas);

//!This function  adds the places RUN and STOP
void AddPlace(ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, int num_place, Type type);

//!This function  adds the transition Pr2Nd
void AddTransPrToNd(ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, const struct NOOBJS &noobjs);

//!This function  adds the transition Nd2Pr
void AddTransNdtoPr(ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, const struct NOOBJS &noobjs);

//!This function splits the string temp in blocks <type,Comp,String>
void parser_string(std::string temp, list<struct TCS> &list_TCS);
void parser_stringNew(std::string temp, list<struct TCS> &list_TCS);
//!This function updates the transition with appropiated arcs
void change_trans(ifstream &in, ofstream &out, std::string buffer, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, list<struct TCS> &ListTCS, Type type);

}//end namespace ParserDefNet
