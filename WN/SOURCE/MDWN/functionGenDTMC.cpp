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


static int S_step = 0;

map<std::string, int> MarkingReward, transName2Id;
map<int, int> TReward;

extern bool reward;

void name2id() {
    for (int i = 0; i < ntr; i++) {
        transName2Id[tabt[i].trans_name] = i;
    }
}

void build_transitions(ifstream &in, map <std::string, map <std::string,  list <class Elem> > > &trans) {
    bool fine = false;
    std::string buffer;
    Parser parser;
    char delim[] = "--> :=";

//reward case: read the transition reward
    if (reward) {
        build_TReward(TReward);
    }
//reward case: read the transition reward

    while ((in) && (!fine)) {
        getline(in, buffer, '\n');
        if (buffer == "Transitions :")
            fine = true;
    }
    if (!fine) {
        ExceptionIO obj("Error Transition  has not found");
        throw (obj);
    }
    else {
        while (in) {
            getline(in, buffer, '\n');
            parser.update(delim, buffer);
            //istringstream stream1(parser.get(4)+"-"+parser.get(5));
            std::string prob;
            if (parser.get(4)[parser.get(4).size() - 1] == 'e')
                prob = parser.get(4) + "-" + parser.get(5);
            else
                prob = parser.get(4);
//cout<<atof(pippo.c_str())<<endl;
            //double prob=0.0;
            //stream1>>prob;
            trans[parser.get(0)][parser.get(1)].push_front(Elem(parser.get(2), prob));
            /***********************************************************************************/
#if TEST1
            out << trans[parser.get(0)][parser.get(1)].front().get_mark() << " " << trans[parser.get(0)][parser.get(1)].front().get_prob() << endl;
#endif
            /***********************************************************************************/
        }
    }
}

//provvisorio leggere da file!!!
void build_TReward(map <int, int> &TReward) {
    TReward[transName2Id["AssignA1"]] = 10;
    cout << "AssignA1 " << transName2Id["AssignA1"] << endl;
    TReward[transName2Id["AssignP1"]] = 10;
    cout << "AssignP1 " << transName2Id["AssignP1"] << endl;
    TReward[transName2Id["AssignG2"]] = 10000;
    cout << "AssignA2 " << transName2Id["AssignG2"] << endl;
    TReward[transName2Id["AssignAA2"]] = 10;
    cout << "AssignAA2 " << transName2Id["AssignAA2"] << endl;
    TReward[transName2Id["AssignAP2"]] = 10;
    cout << "AssignAP2 " << transName2Id["AssignAP2"] << endl;
}






void buildDTMCh(std::ifstream &in, std::ifstream &inTE, map <std::string, map <std::string,  list <class Elem> > > &tr, char *argv) {
    bool fine = false;
    set <std::string> tovisite, newmark, mark, arc, TEmark, TEreached;
    Parser parser, parser1;
    char delim[] = " :";
    char delim1[] = "S";
    std::string buffer;
    buffer = "T1S0";
    tovisite.insert(buffer);
    /* Inserisco le marcature che voglio testare*/
    while ((inTE)) {
        getline(inTE, buffer, '\n');
        if (buffer != "") {
            TEmark.insert(buffer);
        }
    }
    /* Inserisco le marcature che voglio testare*/
    getline(in, buffer, '\n');
//leggo il numero di passi
    S_step = atoi(buffer.c_str());
//leggo il numero di passi
    for (int i = 1; i <= S_step; i++) {
        fine = false;
        while ((in) && (!fine)) {
            getline(in, buffer, '\n');
            parser.update(delim, buffer);
            if ((parser.get(0) == "Process") && (parser.get(1) == "time"))
                fine = true;
        }
        if (!fine) {
            ExceptionIO obj("Error Process Time  is not found");
            throw (obj);
        }
        else {
            int pos = in.tellg();
            while (!tovisite.empty()) {
                std::string marking(*tovisite.begin());
                //tovisite.pop();
                tovisite.erase(tovisite.begin());
                mark.insert(marking);
                fine = false;
                while ((in) && (!fine)) {
                    getline(in, buffer, '\n');
                    parser.update(delim, buffer);
                    parser1.update(delim1, marking);
                    if (parser.get(0) == parser1.get(0)) {
                        fine = true;
                        list <class Elem>::iterator iter;
                        iter = tr[parser.get(0)][parser.get(1)].begin();
                        while (iter != tr[parser.get(0)][parser.get(1)].end()) {
                            ostringstream s1;
                            s1 << iter->get_mark() << "S" << i;
                            //
                            std::string temp = s1.str();
                            if (TEmark.find(iter->get_mark()) != TEmark.end()) {
                                //raggiungo un elemento TE
                                TEreached.insert(s1.str());
                            }//raggiungo un elemento TE
                            //
                            mark.insert(s1.str());
                            //if (marking=="T1")
                            //	marking="T1S0";
                            arc.insert(marking + ":" + s1.str() + ":" + iter->get_prob());
                            newmark.insert(s1.str());
                            iter++;
                        }
                    }
                }
                in.seekg(pos);
            }
            tovisite = newmark;
            newmark.clear();
        }
    }
    try {
        writeMDPh(mark, TEreached, arc, argv);
    }
    catch (ExceptionIO obj) {
        throw (obj);
    }

}

int read_Input(std::ifstream &fp_readInput, set < class State> &IntState) {

//double prob=0.0;
    std::string state, buffer;
    Parser parser;
    char delim[] = " ";
    while ((fp_readInput) && (buffer != "*!*")) {
        getline(fp_readInput, buffer, '\n');
    }
    if (!fp_readInput) {
        ExceptionIO obj("\n*****Error file .step*****");
        throw (obj);
    }
    getline(fp_readInput, buffer, '\n');
    getline(fp_readInput, buffer, '\n');
    int step = atoi(buffer.c_str());
    getline(fp_readInput, buffer, '\n');
    S_step = atoi(buffer.c_str());
    bool fine = false;
    while ((fp_readInput) && (!fine)) {
        getline(fp_readInput, buffer, '\n');
        //fp_readInput>>state>>prob;
        parser.update(delim, buffer);
        if ((parser.get(0) == "Done") || (parser.get(0) == "")) {
            fine = true;
        }
        else
            //class State temp(state,prob);
            IntState.insert(State(parser.get(0), atof(parser.get(1).c_str())));
    }
    return step;
}

void buildDTMChI(std::ifstream &in, std::ifstream &inTE, map <std::string, map <std::string,  list <class Elem> > > &tr, char *argv, set < class State> &IntState, int step) {
    bool fine = false;
    set <std::string> tovisite, newmark, mark, arc, TEmark, TEreached;
    Parser parser, parser1;
    char delim[] = " :";
    char delim1[] = "S";
    std::string buffer;
//buffer="T1S0";
    set < class State>::iterator iter1 = IntState.begin();
    class State temp();
    while (iter1 != IntState.end()) {

        buffer = ((class State)(*iter1)).get_mark();
        //buffer=temp.get_mark();
        tovisite.insert(buffer);
        iter1++;
    }
    /* Inserisco le marcature che voglio testare*/
    while ((inTE)) {
        getline(inTE, buffer, '\n');
        if (buffer != "") {
            TEmark.insert(buffer);
        }
    }
    /* Inserisco le marcature che voglio testare*/
    buffer = "";
    getline(in, buffer, '\n');
//leggo il numero di passi
    S_step = atoi(buffer.c_str());
//leggo il numero di passi
//for (int i=step;i<=1;i++)
//	{

    fine = false;
    while ((in) && (!fine)) {
        getline(in, buffer, '\n');
        parser.update(delim, buffer);
        if ((parser.get(0) == "Process") && (parser.get(1) == "time") && (step == atoi(parser.get(2).c_str())))
            fine = true;
    }
    if (!fine) {
        ExceptionIO obj("Error Process Time  is not found");
        throw (obj);
    }
    else {
        int pos = in.tellg();
        while (!tovisite.empty()) {
            std::string marking(*tovisite.begin());
            //tovisite.pop();
            tovisite.erase(tovisite.begin());
            mark.insert(marking);
            fine = false;
            while ((in) && (!fine)) {
                getline(in, buffer, '\n');
                parser.update(delim, buffer);
                parser1.update(delim1, marking);
                if (parser.get(0) == parser1.get(0)) {
                    fine = true;
                    list <class Elem>::iterator iter;
                    iter = tr[parser.get(0)][parser.get(1)].begin();
                    while (iter != tr[parser.get(0)][parser.get(1)].end()) {
                        ostringstream s1;
                        s1 << iter->get_mark() << "S" << step + 1; //
                        std::string temp = s1.str();
                        if (TEmark.find(iter->get_mark()) != TEmark.end()) {
                            //raggiungo un elemento TE
                            TEreached.insert(s1.str());
                        }//raggiungo un elemento TE
                        mark.insert(s1.str());
                        arc.insert(marking + ":" + s1.str() + ":" + iter->get_prob());
                        newmark.insert(s1.str());
                        iter++;
                    }
                }
            }
            in.seekg(pos);
        }
        tovisite = newmark;
        newmark.clear();
    }

    try {
        writeMDPhI(mark, TEreached, arc, argv, IntState, step);
    }
    catch (ExceptionIO obj) {
        throw (obj);
    }

}

void buildDTMC(std::ifstream &in, std::ifstream &inTE, std::ifstream &inAT, map <std::string, map <std::string,  list <class Elem> > > &tr, char *argv) {

    set <std::string> tovisite, newmark, mark, arc, TEmark;
    set <pair < std::string , int> > ATmark;
    Parser parser, parser1;
    char delim[] = " :";
//char delim1[]="S";
    std::string buffer;
//buffer="T1";
    tovisite.insert("T1");
    /* Inserisco le marcature che voglio testare*/

    while ((inTE)) {
        getline(inTE, buffer, '\n');
        if (buffer != "") {
            TEmark.insert(buffer);
        }
    }
    /* Inserisco le marcature che voglio testare*/

    /* Inserisco le marcature che voglio testare*/

    while ((inAT)) {
        getline(inAT, buffer, '\n');
        class general::Parser parser(delim, buffer);
        if (parser.size() == 2) {
            ATmark.insert(make_pair(parser.get(0), atoi(parser.get(1).c_str())));
        }
    }
    /* Inserisco le marcature che voglio testare*/

    getline(in, buffer, '\n');
    getline(in, buffer, '\n');
    getline(in, buffer, '\n');
    getline(in, buffer, '\n');
//fine=false;
    while ((in)) {
        getline(in, buffer, '\n');
        parser.update(delim, buffer);
        if ((parser.get(0) != "") && (parser.get(0) != "Optimal")) {
            mark.insert(parser.get(0));
            list <class Elem>::iterator iter;
            iter = tr[parser.get(0)][parser.get(1)].begin();
            while (iter != tr[parser.get(0)][parser.get(1)].end()) {
                arc.insert(parser.get(0) + ":" + iter->get_mark() + ":" + iter->get_prob());
                iter++;
            }
            if (reward) {
                Parser parserReward;
                char delimReward[] = ";()";
                parserReward.update(delimReward, parser.get(1));
                int somma = 0;
                for (unsigned int i = 0; i < parserReward.size(); i++) {
                    if (TReward.find(atoi(parserReward.get(i).c_str())) != TReward.end()) {
                        somma = somma + TReward[atoi(parserReward.get(i).c_str())];
                    }
                }
                if (somma > 0)
                    MarkingReward[parser.get(0)] = somma;
            }

        }
    }

    cout << MarkingReward.size() << endl;
    map<std::string, int>::iterator  iter1 = MarkingReward.begin();
    while (iter1 != MarkingReward.end()) {
        cout << "\t" << iter1->first << " " << iter1->second << "\n";
        iter1++;
    }


    try {
        writeMDP(mark, TEmark, ATmark, arc, argv);
    }
    catch (ExceptionIO obj) {
        throw (obj);
    }
}


void writeMDPhI(set<std::string> &mark, set<std::string> &TEreached, set<std::string> &arc, char *argv, set < class State> &IntState, const int &step) {
    Parser parser;
    char delim[] = "S";

    std::string net = std::string(argv) + ".sm";
    ofstream out(net.c_str(), ofstream::out);
    if (!out) {
        ExceptionIO obj("\n*****Error opening output stream .sm*****");
        throw (obj);
    }
    out << "print(\"Esempio DTMC:" << net << "\");\n dtmc mod1 :=\n\t{\n \t state ";
    set <std::string>::iterator iter = mark.begin();
    out << " " << *iter;
    iter++;
    while (iter != mark.end()) {
        if (*iter != "") {
            out << "," << *iter;
            iter++;
        }
    }
    set < class State>::iterator iter1 = IntState.begin();

    if (iter1 != IntState.end()) {
        out << ";\n \t init(" << ((class State)*iter1).get_mark() << ":" << ((class State)*iter1).get_prob();
        iter1++;
    }
    while (iter1 != IntState.end()) {
        out << "," << ((class State)*iter1).get_mark() << ":" << ((class State)*iter1).get_prob();
        iter1++;
    }
    out << ");\n \t arcs(";
    iter = arc.begin();
    out << *iter;
    iter++;
    while (iter != arc.end()) {
        if (*iter != "::") {
            out << ", " << *iter;
            iter++;
        }
    }
    out << ");\n";

    iter = mark.begin();
    while (iter != mark.end()) {
        out << "real P" << *iter << "S := prob_at(in_state(" << *iter << ")," << step + 1 << ");\n";
        iter++;
    }
//int i=0;
//while (i<=S_step)
//	{
//	iter=TEreached.begin();
//		while(iter!=TEreached.end())
//			{
    //parser.update(delim,*iter);
    //if (atoi(parser.get(1).c_str())==i)
    //	{
//				out<<"real P"<<*iter<<"S := prob_at(in_state("<<*iter<<"),"<< step+1 <<");\n";
    //	}
//			iter++;
//			}
//	i=i+1;
//	}

    out << "\t};\n";
//i=0;
//while (i<=S_step)
//	{
    out << "\nreal x := 0.0";
    iter = TEreached.begin();
    while (iter != TEreached.end()) {
        //parser.update(delim,*iter);
        //if (atoi(parser.get(1).c_str())==i)
        //	{
        out << " + mod1.P" << *iter << "S";
        //	}
        //out<<"\nprint(\"\\nProb. transient:"<<*iter<< ";\",mod1.P"<<*iter<<"A"<<i<<",\"\\n\");";
        iter++;
    }
    //out<<";\nprint(\"\\n***************************\");\n";
    out << ";\nprint(\"\\n*!*\\nProb.: \",x);";
    out << ";\nprint(\"\\n" << step + 1 << " \");";
    out << ";\nprint(\"\\n" << step + 10 << " \");";
    iter = mark.begin();
    while (iter != mark.end()) {
        parser.update(delim, *iter);
        if (atoi(parser.get(1).c_str()) == (step + 1)) {
            out << ";\nprint(\"\\n" << *iter << " \",mod1.P" << *iter << "S);";

        }
        //out<<"\nprint(\"\\nProb. transient:"<<*iter<< ";\",mod1.P"<<*iter<<"A"<<i<<",\"\\n\");";
        iter++;
    }
//	i=i+100;
//	}
    out << "\nprint(\"\\n\");\n";
}




void writeMDPh(set<std::string> &mark, set<std::string> &TEreached, set<std::string> &arc, char *argv) {
    Parser parser;
    char delim[] = "S";

    std::string net = std::string(argv) + ".sm";
    ofstream out(net.c_str(), ofstream::out);
    if (!out) {
        ExceptionIO obj("\n*****Error opening output stream .sm*****");
        throw (obj);
    }
    out << "print(\"Esempio DTMC:" << net << "\");\n dtmc mod1 :=\n\t{\n \t state ";
    set <std::string>::iterator iter = mark.begin();
    out << " " << *iter;
    iter++;
    while (iter != mark.end()) {
        if (*iter != "") {
            out << "," << *iter;
            iter++;
        }
    }
    out << ";\n \t init(T1S0:1.0);\n \t arcs(";
    iter = arc.begin();
    out << *iter;
    iter++;
    while (iter != arc.end()) {
        if (*iter != "::") {
            out << ", " << *iter;
            iter++;
        }
    }
    out << ");\n";

    int i = 0;
    while (i <= S_step) {
        iter = TEreached.begin();
        while (iter != TEreached.end()) {
            parser.update(delim, *iter);
            if (atoi(parser.get(1).c_str()) == i) {
                out << "real P" << *iter << "S" << i << " := prob_at(in_state(" << *iter << ")," << i << ");\n";
            }
            iter++;
        }
        i = i + 1;
    }
    out << "\t};\n";
    i = 0;
    while (i <= S_step) {
        out << "\nreal x" << i << " := 0.0";
        iter = TEreached.begin();
        while (iter != TEreached.end()) {
            parser.update(delim, *iter);
            if (atoi(parser.get(1).c_str()) == i) {
                out << " + mod1.P" << *iter << "S" << i;
            }
            //out<<"\nprint(\"\\nProb. transient:"<<*iter<< ";\",mod1.P"<<*iter<<"A"<<i<<",\"\\n\");";
            iter++;
        }
        //out<<";\nprint(\"\\n***************************\");\n";
        out << ";\nprint(\"\\nProb. step" << i << "; \",x" << i << ",\"\");";
        i = i + 1;
    }
    out << ";\nprint(\"\\n\");\n";
}



void writeMDP(set<std::string> &mark, set<std::string> &TEmark, set < pair <std::string, int > > &ATmark, set<std::string> &arc, char *argv) {

    std::string net = std::string(argv) + ".sm";
    ofstream out(net.c_str(), ofstream::out);
    if (!out) {
        ExceptionIO obj("\n*****Error opening output stream .sm*****");
        throw (obj);
    }


    out << "print(\"Esempio DTMC:" << net << "\");\n dtmc mod1 :=\n\t{\n \t state ";
    set <std::string>::iterator iter = mark.begin();
    out << *iter;
    iter++;
    cout << mark.size() << endl;
    while (iter != mark.end()) {
        if (*iter != "") {
            out << "," << *iter;
            iter++;
        }
    }
    out << ";\n \t init(T1:1.0);\n \t arcs(";
    iter = arc.begin();
    out << *iter;
    iter++;
    while (iter != arc.end()) {
        if (*iter != "::") {
            out << ", " << *iter;
            iter++;
        }
    }



    out << ");\n";

    /*marking probability*/
    set < pair <std::string, int > >::iterator iter1;

    for (int i = 800; i < 10000; i = i + 400) {
        iter = TEmark.begin();
        while (iter != TEmark.end()) {
            out << "real P" << *iter << "A" << i << " := prob_at(in_state(" << *iter << ")," << i << ");\n";
            iter++;
        }
        iter1 = ATmark.begin();
        while (iter1 != ATmark.end()) {
            out << "real T" << iter1->first << "A" << i << " := prob_at(in_state(" << iter1->first << ")," << i << ");\n";
            iter1++;
        }
    }


    /*marking probability*/
#if STEADY
    iter = TEmark.begin();
    while (iter != TEmark.end()) {
        out << "real P" << *iter << "B := prob_ss(in_state(" << *iter << "));\n";
        iter++;
    }
    iter1 = ATmark.begin();
    while (iter1 != ATmark.end()) {
        out << "real T" << iter1->first << "B := prob_ss(in_state(" << iter1->first << "));\n";
        iter1++;
    }

    /*marking probability*/
#endif

    if (reward) {
        //reward case
        map<std::string, int>::iterator  iter = MarkingReward.begin();
        while (iter != MarkingReward.end()) {
            out << "real P" << iter->first << "C := prob_at(in_state(" << iter->first << "),99);\n";
            iter++;
        }
        iter = MarkingReward.begin();
        while (iter != MarkingReward.end()) {
            out << "real P" << iter->first << "D := prob_ss(in_state(" << iter->first << "));\n";
            iter++;
        }
    }//reward case
    /*	out<<"real m1 := avg_ss(";
    	map<string,int>::iterator  iter=MarkingReward.begin();
    	int i=0;
    	while (iter!=MarkingReward.end())
    		{
    		out<<"cond(in_state("<<iter->first<<"),"<<iter->second<<",";
    		iter++;
       		i++;
    		}
    	out<<"0";
    	for (int j=0;j<i;j++)
    		{
    		out<<")";
    		}
    	out<<");\n";
    	}//reward case
    */
    out << "\t};\n";


    /* compute probability */
    for (int i = 800; i < 10000; i = i + 400) {
        out << "\nreal x" << i << " := 0.0";

        iter = TEmark.begin();
        while (iter != TEmark.end()) {
            out << " + mod1.P" << *iter << "A" << i;

            iter++;
        }
        out << ";\nprint(\"\\n----------------------------------------------\\n\");";
        out << ";\nprint(\"Step" << i << "\");";
        out << ";\nprint(\"\\n\\tProb. : \",x" << i << ",\"\");";
//----------//
        out << "\nreal y" << i << " := 0.0";
        iter1 = ATmark.begin();
        while (iter1 != ATmark.end()) {
            out << " + " << iter1->second << "* mod1.T" << iter1->first << "A" << i;
            iter1++;
        }
        out << ";\nprint(\"\\n\\tAverage number of tokens: \",y" << i << ",\"\");";
    }
    out << ";\nprint(\"\\n\");\n";

#if STEADY
    out << "real xB := 0.0";
    iter = TEmark.begin();
    while (iter != TEmark.end()) {
        out << " + mod1.P" << *iter << "B";
        //out<<"\nprint(\"\\nProb. steady state:"<<*iter<< ";\",mod1.P"<<*iter<<"B,\"\\n\");";
        iter++;
    }
    out << ";\nreal yB := 0.0";
    iter1 = ATmark.begin();
    while (iter1 != ATmark.end()) {
        out << " + " << iter1->second << " * mod1.T" << iter1->first << "B";
        //out<<"\nprint(\"\\nProb. steady state:"<<*iter<< ";\",mod1.P"<<*iter<<"B,\"\\n\");";
        iter1++;
    }

    if (reward) {
        out << ";\nreal xC := 0.0";
        map<std::string, int>::iterator  iter1 = MarkingReward.begin();
        while (iter1 != MarkingReward.end()) {
            out << " + mod1.P" << iter1->first << "C*" << iter1->second;
            //out<<"\nprint(\"\\nProb. steady state:"<<*iter<< ";\",mod1.P"<<*iter<<"B,\"\\n\");";
            iter1++;
        }
        out << ";\nreal xD := 0.0";
        iter1 = MarkingReward.begin();
        while (iter1 != MarkingReward.end()) {
            out << " + mod1.P" << iter1->first << "D*" << iter1->second;
            //out<<"\nprint(\"\\nProb. steady state:"<<*iter<< ";\",mod1.P"<<*iter<<"B,\"\\n\");";
            iter1++;
        }
    }
    out << ";\nprint(\"\\n\\n**********************************************\\n\");";
    out << ";\nprint(\"Steady state\\n\");";
    out << ";\nprint(\"\\tProbability \",xB,\"\");" << endl;
    out << ";\nprint(\"\\n\\tAverage number of tokens \",yB,\"\");" << endl;
    out << ";\nprint(\"\\n**********************************************\\n\\n\");";
    if (reward) {
        out << "print(\"\\nReward steady state \",xC,\"\\n\");" << endl;
        out << "print(\"\\nReward steady state \",xD,\"\\n\");" << endl;
    }
    //out<<";\nprint(\"\\n\");";
#endif
}


