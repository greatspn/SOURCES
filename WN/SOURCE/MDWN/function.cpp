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
#include "./lib_header/cammini_minimi.h"
#include "./lib_header/GrafoOrientato_ListeAdiacenza.h"
#include "./lib_header/Albero.h"
#include "MDP_minimize.cc"
#define outL 0
#define MAX_REWARD 999999999

namespace RG2RRG {

using namespace asd;

using std::cout;
using std::endl;
using std::map;
using std::pair;
extern bool prob, averageTokens;
extern int horizon;
//cancellare
extern map<std::string, list<class Elem> > dati;
//cancellare


bool DEAD_ACTIVE = false;

extern map<std::string, list<class Elem> > dati;
extern list<std::string> visit_make;
extern map<std::string, double>reachV;
extern map<std::string, struct EL> transName2Id, placesName2Id;
list <Formula> f1;
list <Formula> o1;
//variables for the Bellman-Ford algorithm
GrafoOrientato_ListeAdiacenza grafo;
map< std::string, vertice * > vMap;
map< std::string, arco * > eMap;
map<std::string, int>MarkingPos;

extern int Min;

//variables for the Bellman-Ford algorithm
map<std::string, list <class RAGcache_el> > RAGcache;
//for the Bellman-Ford algorithm


template<class Chiave> void stampa_nodo(nodo<Chiave> *v) {
    cout << info[v->getChiave()];
    if (v->padre) cout << "(->" << info[(v->padre)->getChiave()] << ")";
    cout << "  ";
}
//for the Bellman-Ford algorithm




/**************************Class Elememento****************************************/
Elemento::Elemento(const std::string &id, std::string &id_trans, std::string &instance) {
    this->id = id;
    this->id_trans = id_trans;
    this->instance = instance;
    this->prob = 0.0;
    this->reward = 0.0;
}

Elemento::Elemento(const std::string &id, const double &prob) {
    this->id_trans = "";
    this->id = id;
    this->prob = prob;
    this->reward = 0.0;
}

Elemento::Elemento(const std::string &id, std::string &id_trans, const double &prob, const double &reward) {
    this->id_trans = id_trans;
    this->id = id;
    this->prob = prob;
    this->reward = reward;
}


void Elemento::get(std::string &id, std::string &id_trans, std::string &instance) {
    id_trans = this->id_trans;
    id = this->id;
    instance = this->instance;
}


void Elemento::get(std::string &id, double &prob) {
    prob = this->prob;
    id = this->id;
}




void Elemento::get(std::string &id, std::string &id_trans, double &prob) {
    id_trans = this->id_trans;
    id = this->id;
    prob = this->prob;
}

void Elemento::get(std::string &id, std::string &id_trans, double &prob, double &reward) {
    id_trans = this->id_trans;
    id = this->id;
    prob = this->prob;
    reward = this->reward;
}


/****************************Class Elemento**************************************/


/**************************Class ElFormula***********************************/
ElFormula::ElFormula(const std::string &place, const char &oper, const int &value) {
    this->place = place;
    this->value = value;
    this->oper = oper;
}

void ElFormula::get(std::string &place, char &oper, int &value) {
    value = this->value;
    oper = this->oper;
    place = this->place;
}
/**************************Class ElFormula***********************************/

/**************************Class Formula***********************************/
void Formula::insert(const ElFormula &el) {
    this->listElFormula.push_front(el);
}

void Formula::write() {
    int value;
    char oper;
    std::string place;

    cout << "\nReward:" << reward << " (";
    list<class ElFormula>::iterator iter;
    iter = listElFormula.begin();
    while (iter != listElFormula.end()) {
        iter->get(place, oper, value);
        if (iter == listElFormula.begin())
            cout << place << oper << value;
        else
            cout << " | " << place << oper << value;
        iter++;
    }
    cout << ")\n";
}

bool Formula::test(map<std::string, int> &mapMakring) {
    int value;
    char oper;
    bool end = true;
    std::string place;
    if (listElFormula.size() == 0) return false;
    else {
        list<class ElFormula>::iterator iter;
        iter = listElFormula.begin();
        while (iter != listElFormula.end() && (end)) {
            iter->get(place, oper, value);
            switch (oper) {
            case '=':
                if (mapMakring[place] != value) end = false;
                break;
            case '<':
                if (!(mapMakring[place] < value)) end = false;
                break;
            case '>':
                if (!(mapMakring[place] > value)) end = false;
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




void name2id() {

    for (int i = 0; i < ntr; i++) {
        transName2Id[tabt[i].trans_name].id = i;
        /***********************************************************************************/
#if TEST1
        cout << tabt[i].trans_name << " " << i << " " << transName2Id[tabt[i].trans_name].reward << endl;
#endif
    }
    /***********************************************************************************/
#if TEST1
    map <std::string, int >::iterator iter;
    iter = transName2Id.begin();
    while (iter != transName2Id.end()) {
        cout << iter->first << " " << " id:" << iter->second.id << endl;
        iter++;
    }
#endif
    /***********************************************************************************/
    for (int i = 0; i < npl; i++) {
        placesName2Id[tabp[i].place_name].id = i;
        transName2Id[tabp[i].place_name].reward = 0;
    }
    /***********************************************************************************/
#if TEST2
    map <std::string, struct EL>::iterator iter;
    iter = placesName2Id.begin();
    while (iter != placesName2Id.end()) {
        cout << "Place:" << iter->first << " " << " id:" << iter->second.id << endl;
        iter++;
    }
#endif
    /***********************************************************************************/
}






void Path(std::ofstream &out, std::string &id1, std::string &id, std::string &action, map<std::string, list<class Elemento> > &RG, double prv_reward) {
    list<class Elemento>::iterator iter1;
    std::string idr, actionr, tmpaction, instance;


    iter1 = RG[id].begin();
    while (iter1 != RG[id].end()) {
        iter1->get(idr, tmpaction, instance);
        if (instance.compare("()") == 0)
            actionr = action + ";" + tmpaction;
        else
            actionr = action + ";" + tmpaction + instance;
        if (idr.find("T") != std::string::npos) {
            //*out<<" "<<id1<<" ->"<<" "<<idr<<" [color=dimgray label=\" "<<actionr<<"\"];\n";
            if (vMap[idr] == NULL) {
                vMap[idr] = grafo.aggiungiVertice();
                info[ vMap[idr] ] = idr;
            }
            eMap[tmpaction] = grafo.aggiungiArco(vMap[id], vMap[idr], transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward + prv_reward);
            //if (instance.compare("()")==0)
            info1[eMap[tmpaction]] = actionr;
            //else
            //	info1[eMap[tmpaction]]=actionr;
            //visit_make.push_front(idr);
        }
        else {
            //if(vMap[idr]==NULL)
            //	{
            //	vMap[idr] = grafo.aggiungiVertice();
            //	info[ vMap[idr] ] = idr;
            //	}
            //eMap[tmpaction] = grafo.aggiungiArco(vMap[id], vMap[idr],transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward);
            //info1[eMap[tmpaction]]=tmpaction+instance;
            Path(out, id1, idr, actionr, RG, transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward + prv_reward);
        }
        iter1++;
    }
}


void PathFast(std::ofstream &out, std::string &id1, std::string &id, std::string &action, map<std::string, list<class Elemento> > &RG, double prv_reward, map<std::string, class RAG_el> &RAG) {
    list<class Elemento>::iterator iter1;
    std::string idr, actionr, tmpaction, instance;

//cout<<"size of "<< id<<" "<<RG[id].size()<<endl;
//int i;

    iter1 = RG[id].begin();
    while (iter1 != RG[id].end()) {
        iter1->get(idr, tmpaction, instance);
        if (instance.compare("()") == 0)
            actionr = action + ";" + tmpaction;
        else
            actionr = action + ";" + tmpaction + instance;
        if (idr.find("T") != std::string::npos) {
            double reward = transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward + prv_reward;
            if (RAG[idr].action == "") {
                RAG[idr].reward = reward;
                RAG[idr].action = actionr;
            }
            else if (Min) {
                if (RAG[idr].reward < reward) {
                    RAG[idr].reward = reward;
                    RAG[idr].action = actionr;
                }
            }
            else {
                if (RAG[idr].reward > reward) {
                    RAG[idr].reward = reward;
                    RAG[idr].action = actionr;
                }
            }
        }
        else {
            PathFast(out, id1, idr, actionr, RG, transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward + prv_reward, RAG);
        }
        iter1++;
    }
}

void PathVeryFast(std::ofstream &out, std::string &id1, std::string &id, std::string &action, map<std::string, list<class Elemento> > &RG, double prv_reward, map<std::string, class RAG_el> &RAG) {
    list<class Elemento>::iterator iter1;
    std::string idr, actionr, tmpaction, instance;

//cout<<"size of "<< id<<" "<<RG[id].size()<<endl;
//int i;
    iter1 = RG[id].begin();

    while (iter1 != RG[id].end()) {
        iter1->get(idr, tmpaction, instance);
        if (instance.compare("()") == 0)
            actionr = action + ";" + tmpaction;
        else
            actionr = action + ";" + tmpaction + instance;
        if (idr.find("T") != std::string::npos) {
            double reward = transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward + prv_reward;
            std::string TmpAction(tmpaction);
            if (instance.compare("()") != 0) {
                TmpAction += instance;
            }
            class RAGcache_el tmp(idr, TmpAction, transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward);
            if (RAG[idr].action == "") {
                RAG[idr].reward = reward;
                RAG[idr].action = actionr;
                RAGcache[id].push_back(tmp);
            }
            else if (Min) {
                if (RAG[idr].reward < reward) {
                    RAG[idr].reward = reward;
                    RAG[idr].action = actionr;
                    RAGcache[id].push_back(tmp);
                }
            }
            else {
                if (RAG[idr].reward > reward) {
                    RAG[idr].reward = reward;
                    RAG[idr].action = actionr;
                    RAGcache[id].push_back(tmp);
                }
            }
        }
        else {
            if (RAGcache.find(idr) != RAGcache.end()) {
                list<class RAGcache_el>::iterator iter5 = RAGcache[idr].begin();
                while (iter5 != RAGcache[idr].end()) {
                    double reward = transName2Id[(TRANS_NAME(atoi(tmpaction.c_str())))].reward + prv_reward;
                    if (RAG[iter5->id].action == "") {
                        RAG[iter5->id].reward = reward + iter5->reward;
                        RAG[iter5->id].action = actionr + iter5->action;
                    }
                    else if (Min) {
                        if (RAG[iter5->id].reward < (reward + iter5->reward)) {
                            RAG[iter5->id].reward = reward + iter5->reward;
                            RAG[iter5->id].action = actionr + iter5->action;
                        }
                    }
                    else {
                        if (RAG[iter5->id].reward > (reward + iter5->reward)) {
                            RAG[iter5->id].reward = reward + iter5->reward;
                            RAG[iter5->id].action = actionr + iter5->action;
                        }
                    }
                    iter5++;
                }
            }
            else {
                PathVeryFast(out, id1, idr, actionr, RG, transName2Id[TRANS_NAME(atoi(tmpaction.c_str()))].reward + prv_reward, RAG);
                list<class RAGcache_el>::iterator iter5 = RAGcache[idr].begin();
                while (iter5 != RAGcache[idr].end()) {
                    if (iter5->action != "") {
                        class RAGcache_el tmp(iter5->id, iter5->action, iter5->reward);
                        RAGcache[id].push_back(tmp);
                    }
                    iter5++;
                }
            }
        }
        iter1++;
    }

}

void genRGmdp(std::ofstream &out, std::ifstream &in_marking, std::string sour_mark, map<std::string, list<class Elemento> > &RG) {
    list<class Elemento>::iterator iter1;
    std::string idr, id_trans, dest_mark, instance;
    double prob;

    map<std::string, int> visitOLD;
    visit_make.push_front(sour_mark);
    visitOLD["T1"] = 1;
//int tot=0;
//int i=0;
    while (!visit_make.empty()) {
        sour_mark = visit_make.front();
        visit_make.pop_front();
        while (!RG[sour_mark].empty()) {
            RG[sour_mark].front().get(idr, id_trans, instance);
            //Bellman-Ford algorithm
            //vMap[idr] = grafo.aggiungiVertice();
            //info[ vMap[idr] ] = idr;
            //eMap[id_trans] = grafo.aggiungiArco(vMap[sour_mark], vMap[idr],transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward);
            if (instance.compare("()") != 0) {
                id_trans += instance;
                //	info1[eMap[id_trans]]=id_trans+instance;
            }
            else
                info1[eMap[id_trans]] = id_trans;
            Path(out, sour_mark, idr, id_trans, RG, transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward);
            //Bellman-Ford algorithm
            RG[sour_mark].pop_front();
        }
        //Bellman-Ford algorithm
        vertice *v;
        //arco* e;
        vertice_array<struct rew_act> distanzeBellmanFord;
        if (Min)
            distanzeBellmanFord = BellmanFordMin(grafo, vMap[sour_mark]);
        else
            distanzeBellmanFord = BellmanFordMax(grafo, vMap[sour_mark]);
        for_each_vertice(v, grafo) {
            if (info[v].find("T") != std::string::npos) {
                list<class Elemento>::iterator iter1;
                iter1 = RG[info[v]].begin();
                /***********************************************************************************/
#if TEST7
                out << "Source: " << sour_mark << endl;
#endif
                /***********************************************************************************/
                while (iter1 != RG[info[v]].end()) {
                    iter1->get(dest_mark, prob);
                    if (visitOLD[dest_mark] == 0) {
                        visit_make.push_front(dest_mark);
                        visitOLD[dest_mark] = 1;
                    }
                    /*Compute reward*/
                    double reward = distanzeBellmanFord[vMap[info[v]]].reward + computeMarkingReward(f1, in_marking, MarkingPos[dest_mark]);
                    /*Compute reward*/

                    Elemento elem(dest_mark, distanzeBellmanFord[vMap[info[v]]].action, prob, reward);
                    /***********************************************************************************/
#if TEST7
                    out << "\tdestination: " << dest_mark << " action: " << distanzeBellmanFord[vMap[info[v]]].action << " probability: " << prob << " reward: " << reward << endl;
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
#if TEST6
        list<std::string>::iterator iter2;
        iter2 = visit_make.begin();
        cout << "Visit_make: ";
        while (iter2 != visit_make.end()) {
            cout << *iter2 << " ";
            iter2++;
        }
        cout << endl;
#endif
        /***********************************************************************************/
    }

//free the RG states  that are not in MDP
    cout << "\t\tTotal number of RRG states: " << RG.size() << endl;
    map<std::string, list<class Elemento> >::iterator iter3 = RG.begin(), iter4;
    while (iter3 != RG.end()) {
        if (visitOLD[iter3->first] == 0) {
            iter4 = iter3;
            iter3++;
            RG.erase(iter4);
        }
        else
            iter3++;
    }
    cout << "\t\tTotal number of MDP states: " << RG.size() << endl;
//free the RG states  that are not in MDP
}


void genRGmdpFast(std::ofstream &out, std::ifstream &in_marking, std::string sour_mark, map<std::string, list<class Elemento> > &RG) {
    list<class Elemento>::iterator iter1;
    std::string idr, id_trans, dest_mark, instance;
    double prob;

    map<std::string, int> visitOLD;
    map<std::string, double> RewardM;
    visit_make.push_front(sour_mark);
    visitOLD["T1"] = 1;
//int tot=0;
//int i=0;
    map<std::string, class RAG_el>RAG;
    while (!visit_make.empty()) {
        sour_mark = visit_make.front();
        visit_make.pop_front();
        while (!RG[sour_mark].empty()) {
            RG[sour_mark].front().get(idr, id_trans, instance);
            if (instance.compare("()") != 0) {
                id_trans += instance;
            }
#if FAST
            PathVeryFast(out, sour_mark, idr, id_trans, RG, transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward, RAG);
#endif
#if !FAST
            PathFast(out, sour_mark, idr, id_trans, RG, transName2Id[TRANS_NAME(atoi(id_trans.c_str()))].reward, RAG);
#endif
            RAGcache.clear();
            RG[sour_mark].pop_front();
        }
        map<std::string, class RAG_el>::iterator iterRAG;
        iterRAG = RAG.begin();
        while (iterRAG != RAG.end()) {
            list<class Elemento>::iterator iter1;
            iter1 = RG[iterRAG->first].begin();
            while (iter1 != RG[iterRAG->first].end()) {
                iter1->get(dest_mark, prob);
                if (visitOLD[dest_mark] == 0) {
                    visit_make.push_front(dest_mark);
                    visitOLD[dest_mark] = 1;
                }
                if (RewardM.find(dest_mark) == RewardM.end())
                    RewardM[dest_mark] = computeMarkingReward(f1, in_marking, MarkingPos[dest_mark]);

                Elemento elem(dest_mark, iterRAG->second.action, prob, iterRAG->second.reward + RewardM[sour_mark]);
                //cout<<iterRAG->second.reward+RewardM[dest_mark]<<endl;
                if (iterRAG->second.action == "")
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
        iter2 = visit_make.begin();
        cout << "Visit_make: ";
        while (iter2 != visit_make.end()) {
            cout << *iter2 << " ";
            iter2++;
        }
        cout << endl;
#endif
        /***********************************************************************************/
    }
//free the RG states  that are not in MDP
    cout << "\t\tTotal number of RRG states: " << RG.size() << endl;
    map<std::string, list<class Elemento> >::iterator iter3 = RG.begin(), iter4;
    while (iter3 != RG.end()) {
        if (visitOLD[iter3->first] == 0) {
            iter4 = iter3;
            iter3++;
            RG.erase(iter4);
        }
        else
            iter3++;
    }
    cout << "\t\tTotal number of MDP states: " << RG.size() << endl;
//free the RG states  that are not in MDP
}



void writePOMDP(std::ifstream &in_marking, std::ofstream &out, map<std::string, list<class Elemento> > &RG) {

    std::string id, action;
    double prob1, reward;



    out << "#Generate autamatically by MDWN\ndiscount: 0.90\nvalues: cost\n";
//discount and values will be parameters in the future.
//print state list
    out << "states:";
    map <std::string, list<class Elemento> >::iterator iter = RG.begin();

    while (iter != RG.end()) {
        out << " " << iter->first;
        iter++;
    }
    out << "\nactions:";
    map <std::string, int> setAction;

//reset the iterator
    iter = RG.begin();
//reset the iterator

    while (iter != RG.end()) {
        list<class Elemento>::iterator iter1;
        iter1 = iter->second.begin();
        if (iter1 == iter->second.end()) {
            if (!DEAD_ACTIVE) {
                cerr << "Found an state " << iter->first << " without associated action" << endl;
                exit(1);
            }
        }
        while (iter1 != iter->second.end()) {
            iter1->get(id, action, prob1, reward);
            class general::Parser par, par1;
            char delim[] = ";";
            char delim1[] = "()";
            std::string tmp;
            par.update(delim, action);
            for (unsigned int i = 0; i < par.size(); i++) {
                std::string tmp1 = par.get(i);
                par1.update(delim1, par.get(i));
                tmp = std::string(tmp + tabt[atoi(par1.get(0).c_str())].trans_name + "Z" + par1.get(1) + "Z");
            }
            /*if ((setAction.insert(tmp)).second==TRUE)
            	{
            	out<<" "<<tmp;
            	}*/
            iter1++;
        }
        iter++;
    }

//free memory
    setAction.clear();
//free memory

//Put here the observations
    out << "\nobservations: " << RG.size();
//Put here the observations

//reset the iterator
    iter = RG.begin();
//reset the iterator
    int index = 0;
    while (iter != RG.end()) {
        list<class Elemento>::iterator iter1;
        iter1 = iter->second.begin();
        while (iter1 != iter->second.end()) {
            iter1->get(id, action, prob1, reward);
            out << "O: " << computeMarkingObs(o1, in_marking, MarkingPos[id], index, RG.size()) << endl;
            std::string tmp;
            class general::Parser par, par1;
            char delim[] = ";";
            char delim1[] = "()";
            par.update(delim, action);
            for (unsigned int i = 0; i < par.size(); i++) {
                std::string tmp1 = par.get(i);
                par1.update(delim1, par.get(i));
                tmp = std::string(tmp + tabt[atoi(par1.get(0).c_str())].trans_name + "Z" + par1.get(1) + "Z");
            }
            out << "T: " << tmp << " : " << iter->first << " : " << id  << " " << prob1 << endl;
            out << "R: " << tmp << " : " << iter->first << " : " << id  << ":* " << reward << ".0" << endl;
            iter1++;
        }
        iter++;
        index++;
    }

}


void CompactAction(std::string &str) {
    ostringstream tmp;
    std::string tmp1 = "";
    char delim[] = ";{";
    char delim1[] = "()";
    int *tt = (int *)malloc(ntr * sizeof(int));

    for (int i = 0; i < ntr; i++) {
        tt[i] = 0;
    }
    class general::Parser parser(delim, str);
    for (unsigned int ii = 0; ii < parser.size(); ii++) {
        class general::Parser subparser(delim1, parser.get(ii));
        if (subparser.size() > 0) {
            tt[atoi(subparser.get(0).c_str())]++;
        }
        else {
            str = "Error";
            throw (ExceptionIO());
        }
    }
    for (int i = 0; i < ntr; i++) {
        tmp << tt[i] << "-";
        tt[i] = 0;
    }
    str = tmp.str();
    free(tt);
}

void writexLMDP(std::ofstream &out, std::ifstream &in_marking, map<std::string, list<class Elemento> > &RG) {
    class MDPmin LMDP;
    map <std::string, list<class Elemento> >::iterator iter;
    list<class Elemento>::iterator iter1;
    map <std::string, int> id2int;
    map <std::string, int> ac2int;
    iter = RG.begin();
    int i = 1, j = 1, k = 0, z = 0, n_states = 0, n_actionsXSt = 0; //different actions for states
    std::string id, action;
    double prob1 = 0.0, reward = 0.0;
    map< double, set <int> > Blocks;
    map< double, set <int> > ::iterator itermap;
    set<int>::iterator iterset;
    std::string tmp;
    MDP_weight_type wg;



//out<<RG.size()<<" ";
    while (iter != RG.end()) {
        if (iter->first != "") {
            id2int[iter->first] = i;
            Blocks[computeMarkingReward(f1, in_marking, MarkingPos[iter->first])].insert(i);
#if DEBUG
            cout << computeMarkingReward(f1, in_marking, MarkingPos[iter->first]) << endl;
#endif
            iter1 = iter->second.begin();
            tmp = "*";
            map<std::string, double> stateRate;
            while (iter1 != iter->second.end()) {
                iter1->get(id, action, prob1, reward);
                CompactAction(action);//compact the action name
                if (ac2int.find(action) == ac2int.end()) {
                    ac2int[action] = j;
                    j++;
                }
                if (action != tmp) {
                    k = k + stateRate.size();

                    stateRate.clear();
                    tmp = action;
                    z++;
                }
                iter1++;
                stateRate[id] += prob1;
            }
            k = k + stateRate.size();
            stateRate.clear();
            iter++;
            i++;
        }
    }
//add temporaney states
    for (int jj = i; jj < i + z; jj++)
        Blocks[MAX_REWARD].insert(jj);
//
    n_states = i - 1;
    n_actionsXSt = z;
    /*********************************
    Mapping my variables with grammar
    n_states+n_actionsXSt=nr_states ac2int=nr_label  nr_l_trans=n_actionsXSt nr_w_trans=k blocks=Blocks.size()
    ********************************/

#if outL
    cout << n_states + n_actionsXSt << " " << n_actionsXSt << " " << ac2int.size() << " " << k << " " << Blocks.size() << endl;
#endif
    LMDP.store_sizes(n_states + n_actionsXSt, ac2int.size(), n_actionsXSt, k, Blocks.size());
    iter = RG.begin();
    i = 1;

    z = n_states; //ok  incrermentation before printing
//int tmpn=0;
    k = 0;
    while (iter != RG.end()) {
        if (iter->first != "") {
            iter1 = iter->second.begin();
            tmp = "*";
            map<int, double> stateRate;
            while (iter1 != iter->second.end()) {
                iter1->get(id, action, prob1, reward);
                CompactAction(action);//compact the action name
                if ((id2int.find(id) != id2int.end()) && (ac2int.find(action) != ac2int.end())) {
                    wg = prob1;
                    if (action != tmp) {
                        map<int, double>::iterator it = stateRate.begin();
                        while (it != stateRate.end()) {
                            k++;
                            //cout<<"*"<<k<<" "<<z<<" "<<it->second<<" "<<it->first<<endl;
                            LMDP.store_w_transition(z, it->second, it->first);
                            it++;
                        }
                        stateRate.clear();
                        stateRate[id2int[id]] += wg;
                        z++;
                        tmp = action;
#if outL
                        cout << i << " " << ac2int[action] << " " << z << endl;
#endif
                        //LMDP.store_l_transition( i, ac2int[action], z);
                        LMDP.store_l_transition(i, ac2int[action], z);
                        //tmpn++;
#if outL
                        cout << z << " " << wg << " " << id2int[id] << endl;
#endif
                        //LMDP.store_w_transition(z,wg,id2int[id]);
                    }
                    else {
#if outL
                        cout << z << " " << wg << " " << id2int[id] << endl;
#endif
                        //LMDP.store_w_transition(z,wg,id2int[id]);
                        stateRate[id2int[id]] += wg;
                    }
                }
                iter1++;
            }
            map<int, double>::iterator it = stateRate.begin();
            while (it != stateRate.end()) {
                k++;
                //cout<<""<<k<<" "<<z<<" "<<it->second<<" "<<it->first<<endl;
                LMDP.store_w_transition(z, it->second, it->first);
                it++;
            }
            stateRate.clear();
            i++;
            iter++;
        }
    }
//it prints the block according to format (state* 0)^block
#if DEBUG
    cout << Blocks.size() << endl;
#endif
    itermap = Blocks.begin();
//?? tutti i blocchi o  solo uno??
//itermap++;
    int jj = 1;
#if outL
    cout << "\nInitial partition with 0 as separator:\n";
#endif
    while (itermap != Blocks.end()) {
#if DEBUG
        cout << "Block " << jj << ":" << itermap->first << " \n\t";
#endif
        iterset = itermap->second.begin();
        while (iterset != itermap->second.end()) {
#if DEBUG
            cout << (*iterset) << " ";
#endif
            LMDP.store_block(*iterset , jj);
#if outL
            cout << (*iterset) << " ";
#endif
            iterset++;
        }
#if outL
        cout << "0 ";
#endif
#if DEBUG
        cout << endl;
#endif
        jj++;
        itermap++;
    }
#if outL
    cout << endl;
#endif
//call minimization
    LMDP.minimize();
    unsigned nr_states, nr_labels, nr_l_trans, nr_w_trans, nr_blocks;
    LMDP.give_sizes(nr_states, nr_labels, nr_l_trans, nr_w_trans, nr_blocks);

#if outL
    cout << "\nGeneral information :\n";
    cout << "States: " << nr_states << " Labelled transitions: " << nr_l_trans << " Weighted transitions: " << nr_w_trans << " Blocks: " << nr_blocks << endl;
#endif
//write MDP
#if outL
    cout << "\nFinal partition w.r.t. initial blocks :\n";
    for (unsigned bl = 1; bl <= nr_blocks; ++bl) {
        unsigned st = LMDP.give_block_first(bl);
        iter = RG.begin();
        //for (unsigned int  i=1; i<st; i++)
        //	iter++;
        cout << "Block: " << bl << " is split in:\t";
        while (st) { cout << " " << st; st = LMDP.give_block_next();}
        cout << endl;
    }
    cout << "\nState with its corresponding final block:\n";
#endif

    set <int> setblock;
    map <int, int> ll;
    map <int, int> bl2int;
    j = 0;
    for (i = 1; i <= (signed)id2int.size(); i++) {
//#if outL
        cout << "State " << i << " Block: " << LMDP.which_block(i) << endl;
//#endif
        ll[LMDP.which_block(i)] = i;
        if (bl2int.find(LMDP.which_block(i)) == bl2int.end()) {
            bl2int[LMDP.which_block(i)] = j;
            j++;
        }
        setblock.insert(LMDP.which_block(i));
    }
    cout << "----------->>" << bl2int.size() << endl;
//output
    out << setblock.size() << endl;
    for (i = 1; i <= (signed)setblock.size(); i++)
        out << i << endl;
    map <int, int>::iterator it = ll.begin();
    map <std::string, int>::iterator it1;
    i = 0;
    while (it != ll.end()) {
        i++;
        it1 = id2int.begin();
        for (int zz = 0; zz < (it->second) - 1; zz++)
            it1++;
        iter1 = RG[it1->first].begin();
        if (iter1 == RG[it1->first].end()) {
            if (!DEAD_ACTIVE) {
                cout << "Found an state " << iter->first << " without associated action" << endl;
                exit(1);
            }
            else {
                out << "X{" << i << "}" << endl;
                out << i << " " << i << " 1" << " 0" << endl;
            }
        }
        else {
            while (iter1 != RG[it1->first].end()) {
                iter1->get(id, action, prob1, reward);

                out << action << "{" << i << "}" << endl;
                out << i << " " << bl2int[LMDP.which_block(id2int[id])] << " " << prob1 << " " << reward << endl;
                iter1++;
            }
        }
        it++;
    }
//output
    cout << "\t\tTotal number of Lumped MDP states: " << setblock.size() << endl;
    /*map <int,list<int> >::iterator it=ll.begin();
    while (it!=ll.end())
    	{
    	list<int>::iterator it1 =it->second.begin();
    	while (it1!=it->second.end())
    		{
    		cout<<*it1<<" ";
    		it1++;
    		}
    	cout<<endl;
    	it++;
    	}*/
#if outL
    cout << "\nWeighted transitions:\n";
    for (unsigned tr = 0; tr < nr_w_trans; ++tr) {
        unsigned tail, head; MDP_weight_type weight;
        LMDP.give_w_transition(tail, weight, head);
        cout << "S block: " << head << " D block:" << tail << " Rate:" << weight << endl;
    }
    cout << "\nLabelled transitions:\n";
    for (unsigned tr = 0; tr < nr_l_trans; ++tr) {
        unsigned tail, head, label;
        LMDP.give_l_transition(tail, label, head);
        cout << "S block: " << head << " D block:" << tail << " Label:" << label << endl;
    }
#endif

    LMDP.terminate();
}


void writeTextMDP(std::ofstream &out, std::ofstream &out_prob, std::ofstream &out_avT, std::ifstream &in_marking , map<std::string, list<class Elemento> > &RG) {
    int i = 0, num_token = 0;
    map <std::string, int> id2int;
    std::string id, action;
    double prob1, reward;
    out << RG.size() << endl;
//states
    map <std::string, list<class Elemento> >::iterator iter;
    iter = RG.begin();


    while (iter != RG.end()) {
        out << iter->first << endl;
        id2int[iter->first] = i;
        if ((prob) && (testTE(in_marking, MarkingPos[iter->first]))) {
            out_prob << iter->first << endl;
        }
        if ((averageTokens) && ((num_token = testTM(in_marking, MarkingPos[iter->first])) > 0)) {
            out_avT << iter->first << " " << num_token << endl;
        }
        iter++;
        i++;
    }
//states
    iter = RG.begin();
    i = 0;
    list<class Elemento>::iterator iter1;
    while (iter != RG.end()) {

        iter1 = iter->second.begin();
        if (iter1 == iter->second.end()) {
            if (!DEAD_ACTIVE) {
                cout << "Found an state " << iter->first << " without associated action" << endl;
                exit(1);
            }
            else {
                out << "X{" << i << "}" << endl;
                out << i << " " << i << " 1" << " 0" << endl;
            }
        }
        else {
            while (iter1 != iter->second.end()) {
                iter1->get(id, action, prob1, reward);
                out << action << "{" << i << "}" << endl;
                out << i << " " << id2int[id] << " " << prob1 << " " << reward << endl;
                iter1++;
            }
        }
        iter++;
        i++;
    }
}



void writeMDPxPrism(std::ofstream &out, std::ifstream &in_marking , map<std::string, list<class Elemento> > &RG) {

    std::string id, action;
    double prob1, reward;



    out << "//This MDP model is automatically generated.\n//Send an email to beccuti@di.unito.it for any bug.\n";
    out << "mdp\n\n";
    for (int i = 0; i < npl; i++) {
        out << "const int N" << i << ";\n";
    }
    out << "\n  module m1\n\n\n";
    int *placeInit = (int *) malloc(sizeof(int) * npl);
    int *placeInitS = (int *) malloc(sizeof(int) * npl);

    setinit(in_marking, placeInit);

    for (int i = 0; i < npl; i++) {
        out << "\t" << tabp[i].place_name << " : [0..N" << i << "] init " << placeInit[i] << ";\n";
    }

    map <std::string, list<class Elemento> >::iterator  iter = RG.begin();
    out << "\n\n\n";
    int i = 0;
    list<class Elemento>::iterator iter1;

    while (iter != RG.end()) {
        iter1 = iter->second.begin();
        if (iter1 == iter->second.end()) {
            if (!DEAD_ACTIVE) {
                cout << "Found an state " << iter->first << " without associated action" << endl;
                exit(1);

            }
            else {
                //out<<"\t[X"<<i<<"] ";
                out << "\t[] ";
                print_state(out, in_marking, MarkingPos[iter->first], false, placeInit, placeInitS);
                out << " ->\n\t 1 : ";
                print_state(out, in_marking, MarkingPos[iter->first], true, placeInit, placeInitS);
                out << "\n";
            }

        }
        else {
            std::string actions = "@";
            double sum = 0.0;
            while (iter1 != iter->second.end()) {
                iter1->get(id, action, prob1, reward);
                if (action != actions) {
                    if (actions != "@") {
                        out << ";\n";
                        if ((sum < 0.9999) && (sum > 1.0001)) {
                            cout << "Error transition probability does not sum to 1x (" << sum << ") " << actions << endl;
                            exit(1);
                        }
                    }
                    actions = action;
                    //out<<"\t["<<action<<"] ";
                    out << "\t[] ";
                    print_state(out, in_marking, MarkingPos[iter->first], false, placeInit, placeInitS);
                    out << " ->\n\t\t";
                    sum = 0.0;
                }
                if (sum != 0.0)
                    out << "\n\t\t+ ";
                out << prob1 << ": ";
                print_state(out, in_marking, MarkingPos[id], true, placeInit, placeInitS);
                sum += prob1;
                iter1++;
            }
            out << ";\n";
        }
        iter++;
        i++;
        out << "\n";
    }
    out << "\nendmodule\n\n";

    bool first = false;
    map <std::string, struct EL>::iterator it;
    it = placesName2Id.begin();


    while (it != placesName2Id.end()) {
        if (it->second.reward != 0) {
            if (!first) {
                out << "rewards\n";
                first = true;
            }
            out << "\t true : " << it->first << "*" << it->second.reward << ";\n";
        }
        it++;
    }
    if (true) {
        out << "endrewards\n\n";
    }
    free(placeInit);
    free(placeInitS);
}

void writeMDP(std::ofstream &out, std::ofstream &out_prob, std::ofstream &out_avT, std::ifstream &in_marking , map<std::string, list<class Elemento> > &RG) {
    int i = 0, num_token = 0;
    map <std::string, int> id2int;
    std::string id, action;
    double prob1, reward;

    out << "<?xml version = \"1.0\"?>\n<!DOCTYPE MDP[\n<!ELEMENT ACTION ( LABEL, TRANSITIONS ) >\n<!ELEMENT ACTIONS ( ACTION )+ >\n<!ELEMENT ENDING_STATE ( #PCDATA ) >\n<!ELEMENT LABEL ( #PCDATA ) >\n<!ELEMENT MDP ( STATES, ACTIONS ) >\n<!ATTLIST MDP stationary NMTOKEN #REQUIRED >\n<!ATTLIST MDP  Infinite NMTOKEN #REQUIRED >\n<!ATTLIST MDP  InfiniteCriteriumType  (DISCOUNTED |AVERAGE_REWARD) #IMPLIED >\n<!ATTLIST MDP  DecompositionAlgorithmType NMTOKEN #IMPLIED >\n<!ATTLIST MDP  Horizon NMTOKEN #IMPLIED >\n<!ATTLIST MDP  Optimization_algorithm (POLICY_ITERATION | VALUE_ITERATION | LINEAR_PROGRAMMING) #IMPLIED >\n<!ELEMENT PROBABILITY ( #PCDATA ) >\n<!ELEMENT REWARD ( #PCDATA ) >\n<!ELEMENT STARTING_STATE ( #PCDATA ) >\n<!ELEMENT STATE ( LABEL ) >\n<!ELEMENT STATES ( STATE )+ >\n<!ELEMENT TRANSITIONS (TRANSITION)+ >\n<!ELEMENT TRANSITION ( STARTING_STATE, ENDING_STATE, PROBABILITY, REWARD ) >\n]>";

    out << "\n\n<MDP stationary=\"yes\" Infinite=\"true\" InfiniteCriteriumType=\"AVERAGE_REWARD\" Horizon=\"" << horizon << "\" Optimization_algorithm=\"LINEAR_PROGRAMMING\">\n\t<STATES>";

    map <std::string, list<class Elemento> >::iterator iter;
    iter = RG.begin();
    i = 0;

    while (iter != RG.end()) {
        out << "\n\t\t<STATE>\n\t\t\t<LABEL>" << iter->first << "</LABEL>\n\t\t</STATE>";
        id2int[iter->first] = i;
        if ((prob) && (testTE(in_marking, MarkingPos[iter->first]))) {
            out_prob << iter->first << endl;
        }
        if ((averageTokens) && ((num_token = testTM(in_marking, MarkingPos[iter->first])) > 0)) {
            out_avT << iter->first << " " << num_token << endl;
        }
        iter++;
        i++;
    }
    out << "\n\t</STATES>\n\t<ACTIONS>\n";
    iter = RG.begin();
    i = 0;
    while (iter != RG.end()) {
        list<class Elemento>::iterator iter1;
        iter1 = iter->second.begin();
        std::string compare("$");
        if (iter1 == iter->second.end()) {
            if (!DEAD_ACTIVE) {
                cout << "Found an state " << iter->first << " without associated action" << endl;
                exit(1);
            }
            else {
                out << "\t\t<ACTION>\n\t\t\t<LABEL>X{" << i << "}" << "</LABEL>\n\t\t\t<TRANSITIONS>\n\t\t\t\t<TRANSITION>\n";
                out << "\t\t\t\t\t<STARTING_STATE>" << i << "</STARTING_STATE>\n";
                out << "\t\t\t\t\t<ENDING_STATE>" << i << "</ENDING_STATE>\n";
                out << "\t\t\t\t\t<PROBABILITY> 1</PROBABILITY>\n";
                out << "\t\t\t\t\t<REWARD>0</REWARD>\n\t\t\t\t</TRANSITION>\n";
            }

        }
        else {
            while (iter1 != iter->second.end()) {
                iter1->get(id, action, prob1, reward);
                if (action != compare) {
                    //cout<<tot<<endl;
                    if (compare != "$")
                        out << "\t\t\t</TRANSITIONS>\n\t\t</ACTION>\n";
                    out << "\t\t<ACTION>\n\t\t\t<LABEL>" << action << "{" << i << "}" << "</LABEL>\n\t\t\t<TRANSITIONS>\n\t\t\t\t<TRANSITION>\n";
                    out << "\t\t\t\t\t<STARTING_STATE>" << i << "</STARTING_STATE>\n";
                    out << "\t\t\t\t\t<ENDING_STATE>" << id2int[id] << "</ENDING_STATE>\n";
                    out << "\t\t\t\t\t<PROBABILITY>" << prob1 << "</PROBABILITY>\n";
                    out << "\t\t\t\t\t<REWARD>" << reward << "</REWARD>\n\t\t\t\t</TRANSITION>\n";
                }
                else {
                    out << "\t\t\t\t<TRANSITION>\n";
                    out << "\t\t\t\t\t<STARTING_STATE>" << i << "</STARTING_STATE>\n";
                    out << "\t\t\t\t\t<ENDING_STATE>" << id2int[id] << "</ENDING_STATE>\n";
                    out << "\t\t\t\t\t<PROBABILITY>" << prob1 << "</PROBABILITY>\n";
                    out << "\t\t\t\t\t<REWARD>" << reward << "</REWARD>\n\t\t\t\t</TRANSITION>\n";
                }
                compare = action;
                iter1++;
            }
        }
        out << "\t\t\t</TRANSITIONS>\n\t\t</ACTION>\n";
        iter++;
        i++;

    }
    out << "\t</ACTIONS>\n</MDP>";
}


void CwriteMDP(std::ofstream &out, std::ofstream &out_prob, std::ofstream &out_avT, std::ifstream &in_marking , map<std::string, list<class Elemento> > &RG) {
    int i = 0, num_token = 0;
    map <std::string, int> id2int;
    std::string id, action;
    double prob1, reward;

    out << "<?xml version = \"1.0\"?>\n<!DOCTYPE MDP[\n<!ELEMENT ACTION ( LABEL, TRANSITIONS ) >\n<!ELEMENT ACTIONS ( ACTION )+ >\n<!ELEMENT ENDING_STATE ( #PCDATA ) >\n<!ELEMENT LABEL ( #PCDATA ) >\n<!ELEMENT MDP ( STATES, ACTIONS ) >\n<!ATTLIST MDP stationary NMTOKEN #REQUIRED >\n<!ATTLIST MDP  Infinite NMTOKEN #REQUIRED >\n<!ATTLIST MDP  InfiniteCriteriumType  (DISCOUNTED |AVERAGE_REWARD) #IMPLIED >\n<!ATTLIST MDP  DecompositionAlgorithmType NMTOKEN #IMPLIED >\n<!ATTLIST MDP  Horizon NMTOKEN #IMPLIED >\n<!ATTLIST MDP  Optimization_algorithm (POLICY_ITERATION | VALUE_ITERATION | LINEAR_PROGRAMMING) #IMPLIED >\n<!ELEMENT PROBABILITY ( #PCDATA ) >\n<!ELEMENT REWARD ( #PCDATA ) >\n<!ELEMENT STARTING_STATE ( #PCDATA ) >\n<!ELEMENT STATE ( LABEL ) >\n<!ELEMENT STATES ( STATE )+ >\n<!ELEMENT TRANSITIONS (TRANSITION)+ >\n<!ELEMENT TRANSITION ( STARTING_STATE, ENDING_STATE, PROBABILITY, REWARD ) >\n]>";

    out << "\n\n<MDP stationary=\"yes\" Infinite=\"true\" InfiniteCriteriumType=\"AVERAGE_REWARD\" Horizon=\"" << horizon << "\" Optimization_algorithm=\"LINEAR_PROGRAMMING\">\n\t<STATES>";

    map <std::string, list<class Elemento> >::iterator iter;
    iter = RG.begin();
    i = 0;

    while (iter != RG.end()) {
        out << "\n\t\t<STATE>\n\t\t\t<LABEL>" << iter->first << "</LABEL>\n\t\t</STATE>";
        id2int[iter->first] = i;
        if ((prob) && (testTE(in_marking, MarkingPos[iter->first]))) {
            out_prob << iter->first << endl;
        }
        if ((averageTokens) && ((num_token = testTM(in_marking, MarkingPos[iter->first])) > 0)) {
            out_avT << iter->first << " " << num_token << endl;
        }
        iter++;
        i++;
    }
    out << "\n\t</STATES>\n\t<ACTIONS>\n";
    iter = RG.begin();
    i = 0;
    while (iter != RG.end()) {
        list<class Elemento>::iterator iter1;
        iter1 = iter->second.begin();
        std::string compare("$");
        if (iter1 == iter->second.end()) {
            cout << "Found an state " << iter->first << " without associated action" << endl;
            exit(1);
        }
        while (iter1 != iter->second.end()) {
            iter1->get(id, action, prob1, reward);
            if (action != compare) {
                //cout<<tot<<endl;
                if (compare != "$")
                    out << "\t\t\t</TRANSITIONS>\n\t\t</ACTION>\n";
                out << "\t\t<ACTION>\n\t\t\t<LABEL>" << i << "</LABEL>\n\t\t\t<TRANSITIONS>\n\t\t\t\t<TRANSITION>\n";
                out << "\t\t\t\t\t<STARTING_STATE>" << i << "</STARTING_STATE>\n";
                out << "\t\t\t\t\t<ENDING_STATE>" << id2int[id] << "</ENDING_STATE>\n";
                out << "\t\t\t\t\t<PROBABILITY>" << prob1 << "</PROBABILITY>\n";
                out << "\t\t\t\t\t<REWARD>" << reward << "</REWARD>\n\t\t\t\t</TRANSITION>\n";
            }
            else {
                out << "\t\t\t\t<TRANSITION>\n";
                out << "\t\t\t\t\t<STARTING_STATE>" << i << "</STARTING_STATE>\n";
                out << "\t\t\t\t\t<ENDING_STATE>" << id2int[id] << "</ENDING_STATE>\n";
                out << "\t\t\t\t\t<PROBABILITY>" << prob1 << "</PROBABILITY>\n";
                out << "\t\t\t\t\t<REWARD>" << reward << "</REWARD>\n\t\t\t\t</TRANSITION>\n";
            }
            compare = action;
            iter1++;
        }
        out << "\t\t\t</TRANSITIONS>\n\t\t</ACTION>\n";
        iter++;
        i++;

    }
    out << "\t</ACTIONS>\n</MDP>";
}


void setinit(std::ifstream &in, int *placeInit) {
    in.clear();
    in.seekg(0);
    memset(placeInit, 0, npl * sizeof(int));
    if (in) {
        std::string str;
        getline(in, str, '\n');
        char delim[] = "( )";
        class general::Parser parser(delim, str);
        for (unsigned int i = 0; i < parser.size(); i = i + 2) {
            placeInit[atoi(parser.get(i).c_str())] = atoi(parser.get(i + 1).c_str());
        }
    }
}

void print_state(ofstream &out, std::ifstream &in, int pos, bool ap, int *placeInit, int *placeInitS) {


    in.clear();
    in.seekg(pos);
    memset(placeInit, 0, npl * sizeof(int));


    if (in) {
        std::string str;
        getline(in, str, '\n');
        char delim[] = "( )";
        class general::Parser parser(delim, str);
        for (unsigned int i = 0; i < parser.size(); i = i + 2) {
            placeInit[atoi(parser.get(i).c_str())] = atoi(parser.get(i + 1).c_str());

        }
        if (ap) {
            bool nodiff = true;
            for (unsigned int i = 0; i < (unsigned)npl; i++) {
                if (placeInit[i] != placeInitS[i]) {
                    if (nodiff) {
                        nodiff = false;
                        out << "(" << tabp[i].place_name << "'=" << placeInit[i] << ")";

                    }
                    else {
                        out << " & (" << tabp[i].place_name << "'=" << placeInit[i] << ")";

                    }

                }

            }
            if (nodiff == true) {
                out << " true";
            }
        }
        else {
            placeInitS[0] = placeInit[0];
            out << "(" << tabp[0].place_name << "=" << placeInit[0] << ")";
            for (unsigned int i = 1; i < (unsigned)npl; i++) {
                placeInitS[i] = placeInit[i];
                out << " & (" << tabp[i].place_name << "=" << placeInit[i] << ")";
            }

        }

    }
}


double computeMarkingReward(list <Formula> &g, std::ifstream &in, int &pos) {
//unsigned int offset=0;
    double reward = 0;
    map<std::string, int> mapMarking;
//bool end;
    in.seekg(pos);
    if (in) {
        //in.getline(buffer,200);
        std::string str;
        getline(in, str, '\n');
        /***********************************************************************************/
#if TEST1
        cout << str << endl;
#endif
        /***********************************************************************************/
        unsigned int i = 0;
        char delim[] = "()";
        class general::Parser parser(delim, str);
        while (i < parser.size()) {
            reward = atoi(parser.get(i + 1).c_str()) * placesName2Id[tabp[atoi((parser.get(i)).c_str())].place_name].reward + reward;
            mapMarking[tabp[atoi(parser.get(i).c_str())].place_name] = atoi(parser.get(i + 1).c_str());
            i = i + 2;
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
            cout << place << " " << atoi(tokens.c_str()) << "\n";
#endif
            /***********************************************************************************/
        }
    }
    else throw (ExceptionIO());

    list <Formula >::iterator iter;
    iter = g.begin();
    while (iter != g.end()) {
        if (iter->test(mapMarking)) reward += iter->get_reward();
        iter++;
    }
    mapMarking.clear();
    return reward;
}

std::string computeMarkingObs(list <Formula> &g, std::ifstream &in, const int &pos, const unsigned int &index, const unsigned &size) {
//unsigned int offset=0;
//double reward=0;
    map<std::string, int> mapMarking;
//bool end;
    in.seekg(pos);
    if (in) {
        //in.getline(buffer,200);
        std::string str;
        getline(in, str, '\n');
        /***********************************************************************************/
#if TEST1
        cout << str << endl;
#endif
        /***********************************************************************************/
        unsigned int i = 0;
        char delim[] = "()";
        class general::Parser parser(delim, str);
        while (i < parser.size()) {
            mapMarking[tabp[atoi(parser.get(i).c_str())].place_name] = atoi(parser.get(i + 1).c_str());
            i = i + 2;
            /***********************************************************************************/
#if TEST1
            cout << place << " " << atoi(tokens.c_str()) << "\n";
#endif
            /***********************************************************************************/
        }
    }
    else throw (ExceptionIO());

    list <Formula >::iterator iter;
    iter = g.begin();
    while (iter != g.end()) {
        if (iter->test(mapMarking)) {
            std::string tmp(" ");
            if (iter->get_observ() == "T") {
                for (unsigned int i = 0; i < size; i++) {
                    if (i == index) {
                        tmp = tmp + "1 ";
                    }
                    else {
                        tmp = tmp + "0 ";
                    }
                }
            }
            else if (iter->get_observ() == "u")
                tmp = "uniform";
            else {
                char delim[] = " ";
                class general::Parser parser(delim, iter->get_observ());
                tmp = iter->get_observ();
                if (parser.size() != size) {
                    //in this way the probability distribution is automatically complited
                    for (unsigned int i = parser.size(); i <= size; i++) {
                        tmp = tmp + " 0";
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



bool testTE(std::ifstream &in, int &pos) {
    in.seekg(pos);
    if (in) {
        //in.getline(buffer,200);
        std::string str;
        getline(in, str, '\n');
        /***********************************************************************************/
#if TEST1
        cout << str << endl;
#endif
        /***********************************************************************************/	unsigned int i = 0;
        char delim[] = "()";
        class general::Parser parser(delim, str);
        while (i < parser.size()) {
            if (!strcmp(tabp[atoi(parser.get(i).c_str())].place_name, _PROB_PLACE)) {
                return true;
            }
            i = i + 2;
        }
        return false;
    }
    else throw (ExceptionIO());
}





int testTM(std::ifstream &in, int &pos) {
    in.seekg(pos);
    if (in) {
        //in.getline(buffer,200);
        std::string str;
        getline(in, str, '\n');
        /***********************************************************************************/
#if TEST1
        cout << str << endl;
#endif
        /***********************************************************************************/
        unsigned int i = 0;
        char delim[] = "()";
        class general::Parser parser(delim, str);
        while (i < parser.size()) {
            if (!strcmp(tabp[atoi(parser.get(i).c_str())].place_name, _NUM_TOKEN_PLACE)) {
                return atoi(parser.get(i + 1).c_str());
            }
            i = i + 2;
        }
        return 0;
    }
    else throw (ExceptionIO());
    return false;
}


void parser(std::ifstream &in, map<std::string, list<class Elemento> > &RG) {
    std::string sour_mark, dest_mark, id_trans, instance;
    int num_reached;
    double prob;

    while (in) {
        in >> sour_mark >> num_reached;

        /***********************************************************************************/
#if TEST6
        cout << sour_mark << " " << num_reached << endl;
#endif
        /***********************************************************************************/

        if (num_reached == -1) {
            in >> dest_mark >> id_trans >> instance;
            /***********************************************************************************/
#if TEST6
            cout << "\t" << dest_mark << " action: " << id_trans << " instance:" << instance << endl;
#endif
            /***********************************************************************************/
            Elemento elem(dest_mark, id_trans, instance);
            RG[sour_mark].push_front(elem);
        }
        else {
            while (num_reached != 0) {
                in >> dest_mark >> prob;
                Elemento elem(dest_mark, prob);
                RG[sour_mark].push_front(elem);
                /***********************************************************************************/
#if TEST6
                cout << "\t" << dest_mark << " prob: " << prob << endl;
#endif
                /***********************************************************************************/
                num_reached--;
            }
        }

    }


    /***********************************************************************************/
#if TEST6
    map <std::string, list<class Elemento> >::iterator iter;
    iter = RG.begin();
    while (iter != RG.end()) {
        list<class Elemento>::iterator iter1;
        iter1 = iter->second.begin();
        while (iter1 != iter->second.end()) {
            iter1->get(dest_mark, id_trans, prob);
            cout << "Source:" << iter->first << " Destination: " << dest_mark << " Transition:" << id_trans << "  Prob:" << prob << endl;
            iter1++;
        }
        iter++;
    }
#endif
    /***********************************************************************************/
}



void markingposition(char *argv[]) {
    std::string marking;
    int pos;
    std::string net = std::string(argv[1]) + ".cvrsoff";
    ifstream in(net.c_str(), ifstream::in);
    if (!in) throw(ExceptionIO());
    while (in) {
        in >> marking >> pos;
        MarkingPos[marking] = pos;
    }
}




void readreward(char *argv[]) {
    std::string net = std::string(argv[1]) + ".reward";
    ifstream in(net.c_str(), ifstream::in);
    std::string transition, place;
    char type, oper;
    double reward;
    int value;
    Formula forum;
//file does not exist
    if (!in) throw(ExceptionIO());
//file does not exist
    while (in) {
        in >> type;
        switch (type) {
        case 'T':
        case 't':
            in >> transition >> reward;
            transName2Id[transition].reward = reward;
            break;
        case 'P':
        case 'p':
            in >> place >> reward;
            placesName2Id[place].reward = reward;
            break;
        //and formule
        case 'F':
        case 'f':
            in >> reward >> type;
            type = '|';
            forum.set_reward(reward);
            while ((type == '|') && (in)) {
                in >> place >> oper >> value >> type;
                ElFormula el(place, oper, value);
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
            double prob = 0.0;
            std::string tmp;
            getline(in, tmp);
            class general::Parser par;
            char delim[] = " ";
            par.update(delim, tmp);
            unsigned int i = 0;
            while (par.get(i) != "(" && par.get(i) != "T" && par.get(i) != "t") {
                tmp = +" " + par.get(i);
                prob = +atoi(tmp.c_str());
                i++;
            }
            if (par.get(i) == "t" || par.get(i) == "T") {
                prob = 1;
                tmp = "T";
            }
            if (prob == 0.0 || prob > 1) {
                stringstream s1;
                s1 << "Error Obs:" << o1.size() << "probability sum > 1 or <= 0";
                throw (ExceptionOper(s1.str()));
            }
            i = i + 2;
            forum.set_observ(tmp);
            while (i < (par.size() - 1)) {
                ElFormula el(par.get(i), (par.get(i + 1).c_str())[0], atoi(par.get(i + 2).c_str()));
                forum.insert(el);
                i = i + 3;
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
    cout << transName2Id.size() << endl;
    map <std::string, struct EL>::iterator iter;
    iter = transName2Id.begin();
    while (iter != transName2Id.end()) {
        ;
        cout << "Transition:" << iter->first << " reward" << iter->second.reward << endl;
        iter++;
    }
    iter = placesName2Id.begin();
    while (iter != placesName2Id.end()) {
        ;
        cout << "Place:" << iter->first << " reward" << iter->second.reward << endl;
        iter++;
    }
    cout << transName2Id.size() << endl;
//f1->write();
#endif
    /***********************************************************************************/
}

}//end namespace RG2RRG
