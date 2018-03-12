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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef __IOS_H__
#define __IOS_H__
#include <iostream>
#endif


#ifndef __MAP_H__
#define __MAP_H__
#include <map>
#endif

#ifndef __LST_H__
#define __LST_H__
#include <list>
#endif

#ifndef __SET_H__
#define __SET_H__
#include <set>
#endif


#ifndef __QUE_H__
#define __QUE_H__
#include <queue>
#endif


#ifndef __STR_H__
#define __STR_H__
#include <string.h>
#endif

#ifndef __MDD_H__
#define __MDD_H__
#include "../SHARED/medd.h"
#endif

namespace dddRS {
using namespace std;


class BOUNDMARKING {

    int Max;
    int Min;
public:
    //! Empty constructor
    BOUNDMARKING()	{Min = -1; Max = 256;};
    //! Constructor takes in input a place id and its marking bounds.
    BOUNDMARKING(int Max, int Min)	{this->Min = Min; this->Max = Max;};
    //! \name Methods use to access at the data structures
    //@{
    //!it returns  markings' bounds.
    void get(int &Max, int &Min)		{Min = this->Min; Max = this->Max;};
    //@}
    //! \name Methods use to update the data structures
    //@{
    //!it sets  markings' bounds.
    void set(int Max, int Min)		{this->Min = Min; this->Max = Max;};
    //!it sets upper bound.
    void setMax(int Max)		{if (this->Max != 256) cerr << "Conflict in the automaton" << endl; this->Max = Max;};
    //!it sets lower bound.
    void setMin(int Min)		{if (this->Min != -1) cerr << "Conflict in the automaton" << endl; this->Min = Min;};
    //@}
};

class ATRANSITION {
    int D; // destination state
    map < int, class BOUNDMARKING > pre; //transition pre/post conditions for places
    map < int, class BOUNDMARKING > post; //transition pre/post conditions for places
public:
    //! Empty constructor
    ATRANSITION()	{D = 0;};
    //! Constructor takes in input the destination state.
    ATRANSITION(int D)	{this->D = D;};
//! \name Methods use to access at the data structures
    //@{
    //!it returns the destination state and the transition id
    int getD()		{return D;};
    //!it searchs a place in the pre condition list
    bool findPre(int P) {
        if (pre.find(P) == pre.end()) {
            return false;
        }
        else
            return true;
    }
    //!it searchs a place in the post condition list
    bool findPost(int P) {
        if (post.find(P) == post.end()) {
            return false;
        }
        else
            return true;
    }
    //!it takes in input a place id and returns true if the place belongs to the pre condition otherwise false. In Max and Min the place bounds are stored.
    bool getBoundPre(int P, int &Max, int &Min) {
        if (pre.find(P) == pre.end()) {
            return false;
        }
        else {
            pre[P].get(Max, Min);
            return true;
        }
    };
    //!it takes in input a place id and returns true if the place belongs to the post condition otherwise false. In Max and Min the place bounds are stored.
    bool getBoundPost(int P, int &Max, int &Min) {
        if (post.find(P) == post.end()) {
            return false;
        }
        else {
            post[P].get(Max, Min);
            return true;
        }
    };
    //@}
//! \name Methods use to update  the data structures
    //@{
    //!it sets the destination state
    void setS_T(int D)		{this->D = D;};
    //!it sets the pre bound for  place P
    bool setBoundPre(int P, const int &Max, const int &Min) {
        bool b = false;
        if (pre.find(P) == pre.end()) {
            b = false;
        }
        pre[P].set(Max, Min);
        return b;
    };
    //!it sets the post bound for  place P
    bool setBoundPost(int P, const int &Max, const int &Min) {
        bool b = false;
        if (post.find(P) == post.end()) {
            b = false;
        }
        post[P].set(Max, Min);
        return b;
    };
    //!it sets the pre upper bound for  place P
    bool setBoundMaxPre(int P, const int &Max) {
        bool b = false;
        if (pre.find(P) == pre.end()) {
            b = false;
        }
        pre[P].setMax(Max);
        return b;
    };
    //!it sets the post upper bound for  place P
    bool setBoundMaxPost(int P, const int &Max) {
        bool b = false;
        if (post.find(P) == post.end()) {
            b = false;
        }
        post[P].setMax(Max);
        return b;
    };
    //!it sets the pre lower bound for  place P
    bool setBoundMinPre(int P, const int &Min) {
        bool b = false;
        if (pre.find(P) == pre.end()) {
            b = false;
        }
        pre[P].setMin(Min);
        return b;
    };
    //!it sets the post lower bound for  place P
    bool setBoundMinPost(int P, const int &Min) {
        bool b = false;
        if (post.find(P) == post.end()) {
            b = false;
        }
        post[P].setMin(Min);
        return b;
    };
    //!it sets a pre bound list
    void setLBoundPre(map < int, class BOUNDMARKING > &pre) {
        this->pre = pre;
    };
    //!it sets a post bound list
    void setLBoundPost(map < int, class BOUNDMARKING > &post) {
        this->post = post;
    };

    //@}
    friend ostream &operator<<(ostream &out, class ATRANSITION &data) {
        out << "-----------------------------------------\n";
        out << "Destination: " << data.D << endl;
        map <int, class BOUNDMARKING>::iterator iter = data.pre.begin();
        int Max, Min;
        out << "Pre condition:" << endl;
        while (iter != data.pre.end()) {
            iter->second.get(Max, Min);
            out << "\t Place: " << iter->first << " Max: " << Max << " Min: " << Min << endl;
            iter++;
        }
        out << "Post condition:" << endl;
        iter = data.post.begin();
        while (iter != data.post.end()) {
            iter->second.get(Max, Min);
            out << "\t Place: " << iter->first << " Max: " << Max << " Min: " << Min << endl;
            iter++;
        }
        out << "-----------------------------------------\n";
        return out;
    };
};


};
