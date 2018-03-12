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
#include "mdwnclass.h"


namespace ParserDefNet {


static  bool Gl = false;
static unsigned  int sizeCompAtt = 0, sizeCompPas = 0;


/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void readComponent(ifstream &in, map<std::string, class component> &CompAtt, map <std::string, class component> &CompPas) {
    int num_read = 0;
    std::string buffer;
    map <std::string, std::string> color;
    while (in) {
        getline(in, buffer, '\n');
        if (buffer.find(_COLORDEF) != std::string::npos) {
            /***********************************************************************************/
#if TEST2
            cout << "COLOR DEFINITION" << buffer;
#endif
            /***********************************************************************************/
            num_read++;
            std::string buffer2;
            getline(in, buffer2, '\n');
            std::string idclass = buffer.substr(1, buffer.find(" ") - 1);
            if ((buffer2.substr(0, 1) == "u") || (buffer2.substr(0, 1) == "o")) {
                color[idclass] = idclass;
                buffer2.substr(2, buffer2.find(",") - 2);
                buffer2.erase(0, 2);
                do {
                    color[buffer2.substr(0, buffer2.find(","))] = idclass;
                    buffer2 = buffer2.erase(0, buffer2.find(",") + 1);
                }
                while (buffer2.find(",") != std::string::npos);
            }
        }
        if (buffer.find(_MARKINGDEF) != std::string::npos) num_read++;
        if (buffer.find(_COMP_CON_COD) != std::string::npos) {
            /***********************************************************************************/
#if TEST1
            cout << buffer.substr(1, buffer.find(" ") - 1);
#endif
            /***********************************************************************************/
            std::string buffer3 = buffer.substr(1, buffer.find(" ") - 1);
            /***********************************************************************************/
#if TEST1
            cout << "\n\tECCO QUI\n";
#endif
            /***********************************************************************************/
            getline(in, buffer, '\n');
            /***********************************************************************************/
#if TEST1
            cout << "\t" << buffer << endl;
#endif
            /***********************************************************************************/
            CompAtt[buffer3] = component(buffer, num_read);
        }
        else {
            if (buffer.find(_COMP_NOTCON_COD) != std::string::npos) {
                /***********************************************************************************/
#if TEST1
                cout << buffer.substr(1, buffer.find(" ") - 1);
#endif
                /***********************************************************************************/
                std::string buffer3 = buffer.substr(1, buffer.find(" ") - 1);
                /***********************************************************************************/
#if TEST1
                cout << "\n\tECCO QUI\n";
#endif
                /***********************************************************************************/
                getline(in, buffer, '\n');
                /***********************************************************************************/
#if TEST1
                cout << "\t" << buffer << endl;
#endif
                /***********************************************************************************/
                CompPas[buffer3] = component(buffer, num_read);
            }
        }
    }
    if (CompAtt.size() == 0) throw (ExceptionIO());
    map <std::string, class component>::iterator iter;
    iter = CompAtt.begin();
    while (iter != CompAtt.end()) {
        try {
            iter->second.set_domain(arclabelTodomain(iter->second.get_arclabel(), color));
        }
        catch (ExceptionIO) {
            cerr << "\n*****Error in the label arc for active components, it is not possible to compute the domain!*****" << endl;
            exit(1);
        };
        iter++;
    }
    iter = CompPas.begin();
    while (iter != CompPas.end()) {
        try {
            iter->second.set_domain(arclabelTodomain(iter->second.get_arclabel(), color));
        }
        catch (ExceptionIO) {
            cerr << "\n*****Error in the label arc for passive components, it is not possible to compute the domain!*****" << endl;
            throw (ExceptionIO());
        };
        iter++;
    }
    /***********************************************************************************/
#if TEST2
    cout << "\nACTIVE COMPONENTS" << endl;
    iter = CompAtt.begin();
    while (iter != CompAtt.end()) {
        cout << iter->first << ":" << iter->second.get_domain() << ":" << iter->second.get_arclabel() << endl;
        iter++;
    }

    cout << "\nPASSIVE COMPONENTS" << endl;
    iter = CompPas.begin();
    while (iter != CompPas.end()) {
        cout << iter->first << ":" << iter->second.get_domain() << ":" << iter->second.get_arclabel() << endl;
        iter++;
    }
    cout << "\nCOLOR CLASS DEFINITION" << endl;
    map <std::string, std::string>::iterator iter1;
    iter1 = color.begin();
    while (iter1 != color.end()) {
        cout << iter1->first << ":" << iter1->second << endl;
        iter1++;
    }
#endif
    /***********************************************************************************/
    sizeCompAtt = CompAtt.size();
    sizeCompPas = CompPas.size();
}

void readComponentNew(ifstream &in, map<std::string, class component> &CompAtt, map <std::string, class component> &CompPas) {
    int num_read = 0;
    std::string buffer;
    map <std::string, std::string> color;
    char delim[] = " (,";
    class general::Parser parser;
    int line = 0;
    while (in) {
        getline(in, buffer, '\n');
        line++;
        parser.update(delim, buffer);
        if (parser.find(_COLORDEF) != -1) {
            /***********************************************************************************/
#if TEST2
            cout << "COLOR DEFINITION" << buffer;
#endif
            /***********************************************************************************/
            num_read++;
            std::string buffer2;
            getline(in, buffer2, '\n');
            line++;
            std::string idclass(parser.get(0));//qui si trova l'id della classe
            parser.update(delim, buffer2);
            if (parser.findD("u", "o") != -1) {
                color[idclass] = idclass;
                int i = 1;
                std::string ssc(parser.get(i));
                while (ssc != "") {
                    color[ssc] = idclass;
                    i++;
                    ssc = parser.get(i);
                }
            }
            //else
            //	{
            //	ExceptionIO obj("Error Color class\n File .def line "+line);
            //	throw (obj);
            //	}
        }
        else {
            if (parser.find(_MARKINGDEF) != -1) num_read++;
            if (parser.findS(_COMP_CON_COD) != -1) {
                getline(in, buffer, '\n');
                line++;
                CompAtt[parser.get(0)] = component(buffer, num_read);
            }
            else {
                if (parser.findS(_COMP_NOTCON_COD) != -1) {
                    getline(in, buffer, '\n');
                    line++;
                    CompPas[parser.get(0)] = component(buffer, num_read);
                }
            }
        }
    }

//if (CompAtt.size()==0) throw (ExceptionIO());
    map <std::string, class component>::iterator iter;
    iter = CompAtt.begin();
    int index_comp = 1;
    while (iter != CompAtt.end()) {
        try {
            iter->second.set_domain(arclabelTodomain(iter->second.get_arclabel(), color));
        }
        catch (ExceptionIO) {
            cerr << "\n*****Error in the arc label for active components, it is not possible to compute the domain!*****\n";
            cerr << " Component Controllable: " << index_comp << "\n\t Arc label: " << iter->second.get_arclabel() << " Color: " << iter->second.get_domain() << endl;
            throw (ExceptionIO());
        };
        iter++;
        index_comp++;
    }
    index_comp = 0;
    iter = CompPas.begin();
    while (iter != CompPas.end()) {
        try {
            iter->second.set_domain(arclabelTodomain(iter->second.get_arclabel(), color));
        }
        catch (ExceptionIO) {
            cerr << "\n*****Error in the label arc for passive components, it is not possible to compute the domain!*****" << endl;
            cerr << " Component Not Controllable: " << index_comp << "\n\t Arc label: " << iter->second.get_arclabel() << " Color: " << iter->second.get_domain() << endl;
            throw (ExceptionIO());
        };
        iter++;
        index_comp++;
    }
    /***********************************************************************************/
#if TEST2
    cout << "\nACTIVE COMPONENTS" << endl;
    iter = CompAtt.begin();
    while (iter != CompAtt.end()) {
        cout << iter->first << ":" << iter->second.get_domain() << ":" << iter->second.get_arclabel() << endl;
        iter++;
    }

    cout << "\nPASSIVE COMPONENTS" << endl;
    iter = CompPas.begin();
    while (iter != CompPas.end()) {
        cout << iter->first << ":" << iter->second.get_domain() << ":" << iter->second.get_arclabel() << endl;
        iter++;
    }
    cout << "\nCOLOR CLASS DEFINITION" << endl;
    map <std::string, std::string>::iterator iter1;
    iter1 = color.begin();
    while (iter1 != color.end()) {
        cout << iter1->first << ":" << iter1->second << endl;
        iter1++;
    }
#endif
    /***********************************************************************************/
    sizeCompAtt = CompAtt.size();
    sizeCompPas = CompPas.size();
}


/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



std::string arclabelTodomain(std::string arclabel, map <std::string, std::string> &color) {
    bool end = false, consider = true;
    std::string domain, temp;
    for (unsigned int i = 0; !(end) && (i < arclabel.size()); i++) {
        char c = arclabel[i];
        switch (c) {
        case '<':
            break;
        case ' ':
            break;
        case 'S':  if ((i < domain.size() - 1) && !(arclabel[i + 1] = ' ') && (consider))
                temp += arclabel[i];
            break;
//controllare caso S M,C
        case '\n':
        case '>':
            end = true;
            if (color[temp] == "") throw((ExceptionIO()));
            domain += color[temp];
            break;
        case '+':
            consider = false;
            break;
        case ',':
            domain += color[temp] + ",";
            temp.clear();
            consider = true;
            break;
        default:
            if (consider) temp += arclabel[i];
            break;
        }
    }
    return domain;
}



/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void create_net(ifstream &in, ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, Type type) {
    std::string buffer;
    pair<unsigned int, unsigned int>numPlTrans;
    list<struct TCS> ListTCS;

    if ((sizeCompAtt != CompAtt.size()) || (sizeCompPas != CompPas.size())) {
        cerr << sizeCompAtt << " " << CompAtt.size() << " " << sizeCompPas << " " << CompPas.size() << endl;
        cerr << "\n*****Error no components: Controllable or Not Controllable*****" << endl;
        throw (ExceptionComponents());
    }
    while (in) {
        getline(in, buffer, '\n');
        if (buffer.find("|<") != std::string::npos) {
            //transition first line
            std::string temp = buffer.substr(buffer.find("<"), buffer.find(" ") - buffer.find("<"));
            parser_stringNew(temp, ListTCS);
            buffer.erase(buffer.find("|"), buffer.find(" ") - buffer.find("|"));
            /***********************************************************************************/
#if TEST3
            cout << buffer << endl;
#endif
            /***********************************************************************************/
            try {
                change_trans(in, out, buffer, CompAtt, CompPas, ListTCS, type);
            }
            catch (ExceptionIO obj) {
                cerr << "\n*****Error in the transition components*****\n" << obj.get() << endl;
                throw (obj);
            }
        }//transition first line
        else {
            out << buffer << endl;
            if (buffer == "|") {
                //changes number place and transition
                getline(in, buffer, '\n');
                if (buffer[0] != 'f') throw (ExceptionIO());
                struct NOOBJS noobjs = AddPlaceTransNum(out, buffer, sizeCompAtt, sizeCompPas);
                unsigned int i = 1;
                //mark
                string info;
                while ((in) && (i <= noobjs.NM)) {
                    getline(in, buffer, '\n');
                    info += buffer + "\n";
                    if ((buffer.find(_COMP_GL_COD) != std::string::npos) && (type == PROB)) {
                        CompAtt[_COMP_GL_COD] = component();
                        Gl = true;
                        sizeCompAtt += 1;
                    }
                    //not color components
                    if ((buffer.find(_COMP_CON_COD) != std::string::npos) && (type == PROB)) {
                        std::string buffer3 = buffer.substr(0, buffer.find(" "));
                        CompAtt[buffer3] = component("", 1);
                        sizeCompAtt += 1;
                    }
                    else if ((buffer.find(_COMP_NOTCON_COD) != std::string::npos) && (type == PROB)) {
                        std::string buffer3 = buffer.substr(0, buffer.find(" "));
                        CompPas[buffer3] = component("", 1);
                        sizeCompPas += 1;
                    }
                    //not color components
                    i++;
                }
                int NumPlace = noobjs.NP + 4 * sizeCompAtt + 2 * sizeCompPas;
                if (Gl) NumPlace = NumPlace - 2; //I must insert only two GL place
                int NumTrans = 0;
                if (type == PROB)
                    NumTrans = noobjs.NT + 2;
                else
                    NumTrans = noobjs.NT;
                out << "f  " << noobjs.NM << "  " << NumPlace << "  " << noobjs.NR << "  " << NumTrans << "  " << noobjs.NG << "  " << 0 << "  " << noobjs.NL << endl;
                out << info;
                if (i <= noobjs.NM) {
                    ExceptionIO obj("Error in Marking Parameter");
                    throw (obj);
                }
                //mark
                //place
                i = 1;
                while ((in) && (i <= noobjs.NP)) {
                    getline(in, buffer, '\n');
                    out << buffer << endl;
                    i++;
                }
                if (i <= noobjs.NP) {
                    ExceptionIO obj("Error in number of places");
                    throw (obj);
                }
                //place
                AddPlace(out, CompAtt, CompPas, noobjs.NP, type);
                //rate
                i = 1;
                while ((in) && (i <= noobjs.NR)) {
                    getline(in, buffer, '\n');
                    out << buffer << endl;
                    i++;
                }
                if (i <= noobjs.NR) {
                    ExceptionIO obj("Error in number of transitions");
                    throw (obj);
                }
                //rate

                //group
                i = 1;
                while ((in) && (i <= noobjs.NG)) {
                    getline(in, buffer, '\n');
                    out << buffer << endl;
                    i++;
                }
                if (i <= noobjs.NG) {
                    ExceptionIO obj("Error in number of group");
                    throw (obj);
                }
                //group
                if (type == PROB) {
                    AddTransPrToNd(out, CompAtt, CompPas, noobjs);
                    AddTransNdtoPr(out, CompAtt, CompPas, noobjs);
                }
            }//changes number place and transition
        }
    }

    if ((sizeCompAtt != CompAtt.size()) || (sizeCompPas != CompPas.size())) {
        cerr << "\n*****Error reference to a non defined component*****" << endl;
        throw (ExceptionComponents());
    }
}


/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/

void create_netNew(ifstream &in, ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, Type type) {
    std::string buffer;
    pair<unsigned int, unsigned int>numPlTrans;
    list<struct TCS> ListTCS;

    if ((sizeCompAtt != CompAtt.size()) || (sizeCompPas != CompPas.size())) {
        cerr << "\n*****Error no components: Controllable or Not Controllable*****" << endl;
        throw (ExceptionComponents());
    }
    while (in) {
        getline(in, buffer, '\n');
        if (buffer.find("|<") != std::string::npos) {
            //transition first line
            std::string temp = buffer.substr(buffer.find("<"), buffer.find(" ") - buffer.find("<"));
            parser_stringNew(temp, ListTCS);
            buffer.erase(buffer.find("|"), buffer.find(" ") - buffer.find("|"));
            /***********************************************************************************/
#if TEST3
            cout << buffer << endl;
#endif
            /***********************************************************************************/
            try {
                change_trans(in, out, buffer, CompAtt, CompPas, ListTCS, type);
            }
            catch (ExceptionIO obj) {
                cerr << "\n*****Error in the transition components*****\n" << obj.get() << endl;
                throw (obj);
            }
        }//transition first line
        else {
            out << buffer << endl;
            if (buffer == "|") {
                //changes number place and transition
                getline(in, buffer, '\n');
                if (buffer[0] != 'f') throw (ExceptionIO());
                struct NOOBJS noobjs = AddPlaceTransNum(out, buffer, sizeCompAtt, sizeCompPas);
                unsigned int i = 1;
                //mark
                string info;
                class general::Parser parser;
                char delim[] = " (,";
                while ((in) && (i <= noobjs.NM)) {
                    getline(in, buffer, '\n');
                    info += buffer + "\n";
                    parser.update(delim, buffer);
                    if ((parser.findS(_COMP_GL_COD) != -1) && (type == PROB)) {
                        CompAtt[_COMP_GL_COD] = component();
                        Gl = true;
                        sizeCompAtt += 1;
                    }
                    //not color components
                    if ((parser.findS(_COMP_CON_COD) != -1) && (type == PROB)) {
                        //std::string buffer3=buffer.substr(0,buffer.find(" "));
                        CompAtt[parser.get(0)] = component("", 1);
                        sizeCompAtt += 1;
                    }
                    else if ((buffer.find(_COMP_NOTCON_COD) != std::string::npos) && (type == PROB)) {

                        CompPas[parser.get(0)] = component("", 1);
                        sizeCompPas += 1;
                    }
                    //not color components
                    i++;
                }
                int NumPlace = noobjs.NP + 4 * sizeCompAtt + 2 * sizeCompPas;
                if (Gl) NumPlace = NumPlace - 2; //I must insert only two GL place
                int NumTrans = 0;
                if (type == PROB)
                    NumTrans = noobjs.NT + 2;
                else
                    NumTrans = noobjs.NT;
                out << "f  " << noobjs.NM << "  " << NumPlace << "  " << noobjs.NR << "  " << NumTrans << "  " << noobjs.NG << "  " << 0 << "  " << noobjs.NL << endl;
                out << info;
                if (i <= noobjs.NM) {
                    ExceptionIO obj("Error in Marking Parameter");
                    throw (obj);
                }
                //mark
                //place
                i = 1;
                while ((in) && (i <= noobjs.NP)) {
                    getline(in, buffer, '\n');
                    out << buffer << endl;
                    i++;
                }
                if (i <= noobjs.NP) {
                    ExceptionIO obj("Error in number of places");
                    throw (obj);
                }
                //place
                AddPlace(out, CompAtt, CompPas, noobjs.NP, type);
                //rate
                i = 1;
                while ((in) && (i <= noobjs.NR)) {
                    getline(in, buffer, '\n');
                    out << buffer << endl;
                    i++;
                }
                if (i <= noobjs.NR) {
                    ExceptionIO obj("Error in number of transitions");
                    throw (obj);
                }
                //rate

                //group
                i = 1;
                while ((in) && (i <= noobjs.NG)) {
                    getline(in, buffer, '\n');
                    out << buffer << endl;
                    i++;
                }
                if (i <= noobjs.NG) {
                    ExceptionIO obj("Error in number of group");
                    throw (obj);
                }
                //group
                if (type == PROB) {
                    AddTransPrToNd(out, CompAtt, CompPas, noobjs);
                    AddTransNdtoPr(out, CompAtt, CompPas, noobjs);
                }
            }//changes number place and transition
        }
    }

    if ((sizeCompAtt != CompAtt.size()) || (sizeCompPas != CompPas.size())) {
        cerr << "\n*****Error reference to a non defined component*****" << endl;
        throw (ExceptionComponents());
    }
}


/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



struct NOOBJS  AddPlaceTransNum(ofstream &out, std::string buffer, const unsigned int &sizeCompAtt, const unsigned int &sizeCompPas) {
    stringstream stream1, stream2;
    stream1.str(buffer);
    char f;
    struct NOOBJS noobjs;
    stream1 >> f >> noobjs.NM >> noobjs.NP >> noobjs.NR >> noobjs.NT >> noobjs.NG >> noobjs.NL >> noobjs.NL;
    out << stream2.str();
    return noobjs;
}



/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void AddPlace(ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, int NumPlace, Type type) {
    map <std::string, class component>::iterator iter;
    iter = CompAtt.begin();
    while (iter != CompAtt.end()) {
        iter->second.set_place(++NumPlace);
        if (iter->first != _COMP_GL_COD) {
            if (iter->second.get_domain() != "") {
                //color components
                out << "R" << iter->first << "|R" << iter->first << " 0 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
                out << "nR" << iter->first << "|nR" << iter->first << " 0 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
            }//color components
            else {
                // non color components
                out << "R" << iter->first << "|R" << iter->first << " 0 0 0 0 0 0" << endl;
                out << "nR" << iter->first << "|nR" << iter->first << " 0 0 0 0 0 0" << endl;
            }//non color components
            if (type == PROB)
                if (iter->second.get_domain() != "") {
                    //color components
                    out << "S" << iter->first << "|S" << iter->first << "  -" << 10000 + iter->second.get_pos() << " 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
                    out << "nS" << iter->first << "|nS" << iter->first << " 0 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
                }//color components
                else {
                    // non color components
                    out << "S" << iter->first << "|S" << iter->first << " " << iter->second.get_pos() << " 0 0 0 0 0" << endl;
                    out << "nS" << iter->first << "|nS" << iter->first << " 0 0 0 0 0 0" << endl;
                }// non color components
            else if (iter->second.get_domain() != "") {
                //color components
                out << "S" << iter->first << "|S" << iter->first << " 0 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
                out << "nS" << iter->first << "|nS" << iter->first << " 0 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
            }//color components
            else {
                //non color components
                out << "S" << iter->first << "|S" << iter->first << " 0 0 0 0 0 0" << endl;
                out << "nS" << iter->first << "|nS" << iter->first << " 0 0 0 0 0 0" << endl;
            }// non color components
            NumPlace += 3;
        }
        else {
            out << "nR" << iter->first << "|nRG" << iter->first << " 0 0 0 0 0 0" << endl;
            out << "nS" << iter->first << "|nSG" << iter->first << " 0 0 0 0 0 0" << endl;
            NumPlace += 1;
        }

        iter++;
    }
    iter = CompPas.begin();
    while (iter != CompPas.end()) {
        iter->second.set_place(++NumPlace);
        if (iter->second.get_domain() != "") {
            //color components
            out << "R" << iter->first << "|R" << iter->first << " 0 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
        }//color components
        else {
            // non color components
            out << "R" << iter->first << "|R" << iter->first << " 0 0 0 0 0 0" << endl;
        }// non color components
        if (type == PROB)
            if (iter->second.get_domain() != "")
                out << "S" << iter->first << "|S" << iter->first << " -" << 10000 + iter->second.get_pos() << " 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
            else
                out << "S" << iter->first << "|S" << iter->first << " " << iter->second.get_pos() << " 0 0 0 0 0" << endl;
        else if (iter->second.get_domain() != "") {
            //color components
            out << "S" << iter->first << "|S" << iter->first << " 0 0 0 0 0 0 0 0 " << iter->second.get_domain() << endl;
        }//color components
        else {
            // non color components
            out << "S" << iter->first << "|S" << iter->first << " 0 0 0 0 0 0" << endl;
        }// non color components
        NumPlace += 1;
        iter++;
    }
}



/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void AddTransPrToNd(ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, const struct NOOBJS &noobjs) {
    if (Gl)
        out << "Pr2Nd 1.000000e+00 0 0 " << sizeCompAtt + sizeCompPas - 1 << " 0 0 0 0 0 0 0 0\n";
    else
        out << "Pr2Nd 1.000000e+00 0 0 " << sizeCompAtt + sizeCompPas << " 0 0 0 0 0 0 0 0\n";
    unsigned int step = CompAtt.size();
    map <std::string, class component>::iterator iter;
    iter = CompAtt.begin();
//input arc
    while (iter != CompAtt.end()) {
        if (iter->first != _COMP_GL_COD) {
            if (iter->second.get_domain() != "") {
                //color components
                out << "   1 " << iter->second.get_place() + 2 << " 0 0 0.000000 0.000000 " << iter->second.get_arclabel() << endl;
            }//color components
            else {
                // non color components
                out << "   1 " << iter->second.get_place() + 2 << " 0 0" << endl;
            }// non color components
        }
        iter++;
    }
    iter = CompPas.begin();
    while (iter != CompPas.end()) {
        if (iter->second.get_domain() != "") {
            //color components
            out << "   1 " << iter->second.get_place() + 1 << " 0 0 0.000000 0.000000 " << iter->second.get_arclabel() << endl;
        }//color components
        else {
            // non color components
            out << "   1 " << iter->second.get_place() + 1 << " 0 0" << endl;
        }// non color components
        iter++;
    }
//input arc
//output arc
    step = sizeCompAtt;
    out << step << endl; //output arc number
    iter = CompAtt.begin();
    while (iter != CompAtt.end()) {
        if (iter->first != _COMP_GL_COD) {
            if (iter->second.get_domain() != "") {
                //color components
                out << "   1 " << iter->second.get_place() + 1 << " 0 0 0.000000 0.000000 " << iter->second.get_arclabel() << endl;
            }//color components
            else {
                // non color components
                out << "   1 " << iter->second.get_place() + 1 << " 0 0" << endl;
            }// non color components
        }
        else {
            out << "   1 " << iter->second.get_place() << " 0 0" << endl;
        }
        iter++;
    }
//output arc
    out << "0\n";
}


/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void AddTransNdtoPr(ofstream &out, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, const struct NOOBJS &noobjs) {
    out << "Nd2Pr 1.000000e+00 0 0 " << sizeCompAtt << " 0 0 0 0 0 0 0 0\n";
    unsigned int step = sizeCompAtt;
    map <std::string, class component>::iterator iter;
    iter = CompAtt.begin();
//input arc
    while (iter != CompAtt.end()) {
        if (iter->first != _COMP_GL_COD) {
            if (iter->second.get_domain() != "") {
                //color components
                out << "   1 " << iter->second.get_place() + 3 << " 0 0 0.000000 0.000000 " << iter->second.get_arclabel() << endl;
            }//color components
            else {
                // non color components
                out << "   1 " << iter->second.get_place() + 3 << " 0 0" << endl;
            }// non color components
        }
        else {
            out << "   1 " << iter->second.get_place() + 1 << " 0 0" << endl;
        }
        iter++;
    }
//input arc
//output arc
    step = sizeCompAtt + sizeCompPas;
    if (Gl) step = step - 1;

    out << step << endl; //output arc number

    step = sizeCompAtt;
    iter = CompAtt.begin();
    while (iter != CompAtt.end()) {
        if (iter->first != _COMP_GL_COD) {
            if (iter->second.get_domain() != "") {
                //color components
                out << "   1 " << iter->second.get_place() << " 0 0 0.000000 0.000000 " << iter->second.get_arclabel() << endl;
            }//color components
            else {
                // no color components
                out << "   1 " << iter->second.get_place() << " 0 0" << endl;
            }// no color components
        }
        iter++;
    }
    step = CompPas.size();
    iter = CompPas.begin();
    while (iter != CompPas.end()) {
        if (iter->second.get_domain() != "") {
            //color components
            out << "   1 " << iter->second.get_place() << " 0 0 0.000000 0.000000 " << iter->second.get_arclabel() << endl;
        }
        else {
            // no color components
            out << "   1 " << iter->second.get_place() << " 0 0" << endl;
        }// no color components
        iter++;
    }
//output arc
    out << "0\n";
}


/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void parser_string(std::string temp, list<struct TCS> &ListTCS) {
    int i = 0;
    while (temp.find("<", i) != std::string::npos) {
        struct TCS new_elem;
        new_elem.type = temp.substr(i + 1, temp.find(",", i) - (i + 1));
        i = temp.find(",", i) + 1;
        new_elem.comp = temp.substr(i, temp.find(",", i) - i);
        i = temp.find(",", i) + 1;
        new_elem.arc = temp.substr(i, temp.find(">", i) - i);
        i = temp.find(">", i) + 1;
        ListTCS.push_front(new_elem);
    }
    /***********************************************************************************/
#if TEST3
    list<struct TCS>::iterator iter;
    iter = ListTCS.begin();
    while (iter != ListTCS.end()) {
        cout << "Tipo:" << iter->type << " componente:" << iter->comp << " arco:" << iter->arc << endl;
        iter++;
    }
#endif
    /***********************************************************************************/
}



/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void parser_stringNew(std::string temp, list<struct TCS> &ListTCS) {
//int i=0;
    char delim[] = "<>";
    char delim1[] = ",";
    class general::Parser parser(delim, temp), parser1;
    for (unsigned int i = 0; i < parser.size(); i++) {
        parser1.update(delim1, parser.get(i));
        struct TCS new_elem;
        new_elem.type = parser1.get(0);
        new_elem.comp = parser1.get(1);
        new_elem.arc = parser1.get(2);
        for (unsigned int i = 3; i < parser1.size(); i++) {
            new_elem.arc = new_elem.arc + "," + parser1.get(i);
        }
        ListTCS.push_front(new_elem);
    }
    /***********************************************************************************/
#if TEST3
    list<struct TCS>::iterator iter;
    iter = ListTCS.begin();
    while (iter != ListTCS.end()) {
        cout << "Tipo:" << iter->type << " componente:" << iter->comp << " arco:" << iter->arc << endl;
        iter++;
    }
#endif
    /***********************************************************************************/
}


/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/



void change_trans(ifstream &in, ofstream &out, std::string buffer, map <std::string, class component> &CompAtt, map <std::string, class component> &CompPas, list<struct TCS> &ListTCS, Type type) {
    int offset1, offset2; //use for the different case non det or prob
    unsigned int sizeCompPas = CompPas.size(), sizeCompAtt = CompAtt.size();
    if (type == PROB) {
        offset1 = 0;
        offset2 = 2;
    }
    else {
        offset1 = 1;
        offset2 = 3;
    }
    stringstream stream1, stream2;
    stream1.str(buffer);
    std::string name, trate, col, x1, y1, x2, y2, x3, y3, x4, y4;
    int tserv, tknd, tinp, trot, level;

    stream1 >> name >> trate >> tserv >> tknd >> tinp >> trot >> x1 >> y1 >> x2 >> y2 >> x3 >> y3 >> level >> x4 >> y4 >> col;
    int newtinp = tinp + ListTCS.size();
    stream2 << newtinp;

    char delim[] = " ";
    class general::Parser parser(delim, buffer);
    parser.set(4, stream2.str());
    /*UPDATE 11/05/2009 error on the guard
    stream2<<name<<" "<<trate<<" "<<tserv<<" "<<tknd<<" "<<newtinp<<" "<<trot<<" "<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<" "<<x3<<" "<<y3<<" "<<level<<" "<<x4<<" "<<y4<< " " <<col;
    out<<stream2.str()<<endl;
    */
    out << parser.get_string(' ') << endl;
//read and copy input arc
    int i = 1;
    while (i <= tinp) {
        getline(in, buffer, '\n');
        out << buffer << endl;
        stream1.clear();
        stream1.str(buffer);
        string x, y, col;
        int mult, place, point, level;
        stream1 >> mult >> place >> point >> level >> x >> y >> col;
        int j = 1;
        while (j <= point) {
            getline(in, buffer, '\n');
            out << buffer << endl;
            j++;
        }
        i++;
    }
//read and copy input arc
//insert input arcs
    list<struct TCS>::iterator iter;
    iter = ListTCS.begin();

    while (iter != ListTCS.end()) {
        if (iter->comp.find(_COMP_NOTCON_COD) != std::string::npos) {
            //passive case
            if (CompPas[iter->comp].get_place() == -1) {
                ExceptionIO obj("No Place " + iter->comp);
                throw (obj);
            }
            if (type == NOND) {
                ExceptionIO obj("Non deterministic behavior: " + iter->comp + " (Non controllable)");
                throw (obj);
            }
            if (CompPas[iter->comp].get_domain() != "") {
                //color components
                out << "   1 " << CompPas[iter->comp].get_place() << " 0 0 0 0 <" << iter->arc << ">" << endl;
            }//color components
            else {
                //no color components
                out << "   1 " << CompPas[iter->comp].get_place() << " 0 0" << endl;
            }//no color components
        }//passive case
        else {
            //active case
            if ((iter->comp == _COMP_GL_COD) && (type == PROB)) {
                ExceptionIO obj("Probasbilistic behavior: Global Component");
                throw (obj);
            }
            if (iter->comp != _COMP_GL_COD) {
                if (CompAtt[iter->comp].get_domain() != "") {
                    //color components
                    out << "   1 " << CompAtt[iter->comp].get_place() + offset1 << " 0 0 0 0 <" << iter->arc << ">" << endl;
                }//color components
                else {
                    //no color components
                    out << "   1 " << CompAtt[iter->comp].get_place() + offset1 << " 0 0" << endl;
                }//no color components
            }
            else {
                out << "   1 " << CompAtt[iter->comp].get_place() << " 0 0" << endl;
            }
        }//active case
        iter++;
    }
//insert input arcs
    getline(in, buffer, '\n');
    int toup = atoi(buffer.c_str());
    out << toup + ListTCS.size() << endl;

//read and copy output arc
    i = 1;
    while (i <= toup) {
        getline(in, buffer, '\n');
        out << buffer << endl;
        stream1.clear();
        stream1.str(buffer);
        string x, y, col;
        int mult, place, point, level;
        stream1 >> mult >> place >> point >> level >> x >> y >> col;
        int j = 1;
        while (j <= point) {
            getline(in, buffer, '\n');
            out << buffer << endl;
            j++;
        }
        i++;
    }
//read and copy output arc
//insert out arcs

    iter = ListTCS.begin();

    while (iter != ListTCS.end()) {
        //
        if (iter->comp.find(_COMP_NOTCON_COD) != std::string::npos) {
            //passive case
            if (CompPas[iter->comp].get_place() == -1) {
                ExceptionIO obj("No Place " + iter->comp + " (Non controllable)");
                throw (obj);
            }
            if (type == NOND) {
                ExceptionIO obj("Error non deterministic behavior for" + iter->comp);
                throw (obj);
            }
            if ((iter->type == "RUN") || (iter->type == "Run") || ((iter->type == "run"))) {
                if (CompPas[iter->comp].get_domain() != "") {
                    //color components
                    out << "   1 " << CompPas[iter->comp].get_place() << " 0 0 0 0 <" << iter->arc << ">" << endl;
                }//color components
                else {
                    //no color components
                    out << "   1 " << CompPas[iter->comp].get_place() << " 0 0" << endl;
                }//no color components
            }
            else {
                if (CompPas[iter->comp].get_domain() != "") {
                    //color components
                    out << "   1 " << CompPas[iter->comp].get_place() + 1 << " 0 0 0 0 <" << iter->arc << ">" << endl;
                }//color components
                else {
                    //no color components
                    out << "   1 " << CompPas[iter->comp].get_place() + 1 << " 0 0" << endl;
                }//no color components
            }
        }//passive case
        else {
            //active case
            if ((iter->comp == _COMP_GL_COD) && (type == PROB)) {
                ExceptionIO obj("Probasbilistic behavior: Global Component");
                throw (obj);
            }
            if ((iter->type == "RUN") || (iter->type == "Run") || (iter->type == "run")) {
                if (iter->comp != _COMP_GL_COD) {
                    if (CompAtt[iter->comp].get_domain() != "") {
                        //color components
                        out << "   1 " << CompAtt[iter->comp].get_place() + offset1 << " 0 0 0 0 <" << iter->arc << ">" << endl;
                    }//color components
                    else {
                        //no color components
                        out << "   1 " << CompAtt[iter->comp].get_place() + offset1 << " 0 0" << endl;
                    }//no color components
                }
                else
                    out << "   1 " << CompAtt[iter->comp].get_place() << " 0 0" << endl;
            }
            else if (iter->comp != _COMP_GL_COD)
                if (CompAtt[iter->comp].get_domain() != "") {
                    //color components
                    out << "   1 " << CompAtt[iter->comp].get_place() + offset2 << " 0 0 0 0 <" << iter->arc << ">" << endl;
                }//color components
                else {
                    //no color components
                    out << "   1 " << CompAtt[iter->comp].get_place() + offset2 << " 0 0" << endl;
                }//no color components
            else
                out << "   1 " << CompAtt[iter->comp].get_place() + 1 << " 0 0" << endl;
        }//active case
        iter++;
    }
//insert out arc
    ListTCS.erase(ListTCS.begin(), ListTCS.end());
    if ((sizeCompPas != CompPas.size()) || (sizeCompAtt != CompAtt.size())) {
        ExceptionIO obj("Error reference to a new component: " + name + "\n");
        throw (obj);
    }
}

}//end namespace ParserDefNet
