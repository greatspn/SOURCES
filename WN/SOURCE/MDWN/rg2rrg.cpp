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


#include "./class.h"
#include <time.h>

namespace RG2RRG {
bool prob = false, averageTokens = false;
int horizon = 0;
map<std::string, struct EL> transName2Id, placesName2Id;
//cancellare
map<std::string, list<class Elem *> > dati;
//cancellare
map<std::string, list<class Elemento> > RG;
list<std::string> visit_make;
//cancellare
map<std::string, double>reachV;
//cancellare

int Min = 0;
bool MDP = true;
extern bool DEAD_ACTIVE;
bool LMDP = false, TMDP = false;
bool PRISM = false;
}

using namespace RG2RRG;
int main(int argc, char *argv[]) {
    time_t time_1, time_2, time_3, time_4;
    bool fast = false;
    prob = false;
    time(&time_1);
    if (argv[1] == NULL) {
        std::cerr << "\n\nUSE: RG2RRG net [Options]\n\tOptions:\n\t\t -a\t\t compute the average number of tokens in place" << _NUM_TOKEN_PLACE << ";\n\t\t -M\t\t enable output for PRISM;\n\t\t -f\t\t fast generation using cache;\n\t\t -p\t\t compute the probability that the place " << _PROB_PLACE << " is marking;\n\t\t -h [int]\t set the value of finite horizon;\n\t\t -L\t\t output format for lumpability;\n\t\t -T\t\t output in textual format; \n\t\t -D\t\t menage  DEAD markings.\n\n";
        exit(EXIT_FAILURE);
    }


    cout << "\n\n =========================================================\n";
    cout << "|	         (S)RG to (S)RRG converter                |\n";
    cout << " =========================================================\n";
    cout << "\n If you find any bug, send an email to beccuti@di.unito.it\n";
    for (int i = 2; i <= argc; i++)
        if (argc > i) {
            switch (argv[i][1]) {
            case 'a':
                averageTokens = true;
                cout << "\n\t      <enable option to compute the average number of tokens in place "  << _NUM_TOKEN_PLACE << ">\n";
                break;

            case 'f':
                fast = true;
#if FAST

                cout << "\n\t      <enable option fast generation using cache>\n";
#endif
#if !FAST
                cout << "\n\t      <enable option fast generation>\n";
#endif
                break;
            case 'p':
                prob = true;
                cout << "\n\t      <enable option for computing probability that the place " << _PROB_PLACE << " is marking >\n";
                break;
            case 'h':
                char *temp;
                temp = argv[i] + 2;
                horizon = atoi(temp);
                cout << "\n\t      <enable option for finite horizon >\n";
                break;
            case 'P':
                MDP = false;
                break;
            case 'L':
                LMDP = true;
                break;
            case 't':
            case 'T':
                TMDP = true;
                break;
            case 'M':
                PRISM = true;
                cout << "\n\t      <enable PRISM generation>\n";
                break;
            case 'd':
            case 'D':
                DEAD_ACTIVE = true;
                cout << "\n\t      <enable option to accept DEAD markings >\n";
                break;
            default:
                cerr << "Error in the program options" << endl;
                exit(EXIT_FAILURE);
            }
        }

    /*INITIALITATION REWARD*/
    try {
        readreward(argv);
    }
    catch (ExceptionIO) {
        cerr << "\n*****Error opening input stream .reward*****" << endl;
    }
    catch (ExceptionOper(obj)) {
        cerr << obj.get() << endl;
    }
    try {
        markingposition(argv);
    }
    catch (ExceptionIO) {
        cerr << "\n*****Error opening input stream .cvrsoff*****" << endl;
    }

    /*INITIALITATION REWARD*/
    cout << "\n =========================================================\n\n";
    cout << "START EXECUTION ON " << argv[1] << endl;

    /*INITIALITATION NET*/
    cout << "\t START INITIALIZATION" << endl;
//need by initialize
    std::string net = std::string(argv[1]) + ".event";
    ofstream out(net.c_str(), ofstream::out);
    out << " ";
    out.close();
//need by initialize
    initialize(2, argv);
    cout << "\t END INITIALIZATION" << endl;
    /*INITIALITATION NET*/

    name2id();
    time(&time_2);
    net = std::string(argv[1]) + ".mdwn";
    ifstream in(net.c_str(), ifstream::in);

    ofstream out_prob;
    ofstream out_avT;
    if (prob) {
        net = std::string(argv[1]) + ".mte"; //marking with m( _PROB_PLACE)!=0
        out_prob.open(net.c_str(), ofstream::out);
        if (!out_prob) {
            cerr << "\n*****Error opening output stream .mte*****" << endl;
            exit(EXIT_FAILURE);
        }
    }
    if (averageTokens) {
        net = std::string(argv[1]) + ".mat"; //marking with m(_NUM_TOKEN_PLACE)!=0
        out_avT.open(net.c_str(), ofstream::out);
        if (!out_avT) {
            cerr << "\n*****Error opening output stream .mte*****" << endl;
            exit(EXIT_FAILURE);
        }
    }
//open files


    if (!in) {
        cerr << "\n*****Error opening input stream .mdwn*****" << endl;
        exit(EXIT_FAILURE);
    }
    net = std::string(argv[1]) + ".RGmdp";
    out.open(net.c_str(), ofstream::out);
    if (!out) {
        cerr << "\n*****Error opening output stream .RGmdp*****" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "\t START PARSER" << endl;
    parser(in, RG);
    cout << "\t END PARSER" << endl;

    net = std::string(argv[1]) + ".cvrs";
    ifstream in_marking(net.c_str(), ofstream::in);
    if (!in_marking) {
        cerr << "\n*****Error opening input stream .cvrs*****" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "\t START GENERATION MDP" << endl;
    if (!fast)
        genRGmdp(out, in_marking, "T1", RG);
    else
        genRGmdpFast(out, in_marking, "T1", RG);
    cout << "\t END GENERATION MDP" << endl;
//Close the stream
    in.close();
    out.close();
//Close the stream
    time(&time_3);

    if (LMDP) {
        net = std::string(argv[1]) + ".lmdp";
        out.open(net.c_str(), ofstream::out);
        if (!out) {
            cerr << "\n*****Error opening output stream .xml*****" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "\t START LUMPING MDP" << endl;
        writexLMDP(out, in_marking, RG);
        cout << "\t END LUMPING MDP" << endl;
        out.close();
    }
    else if (TMDP) {
        net = std::string(argv[1]) + ".tmdp";
        out.open(net.c_str(), ofstream::out);
        if (!out) {
            cerr << "\n*****Error opening output stream .xml*****" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "\t START WRITING MDP" << endl;
        writeTextMDP(out, out_prob, out_avT, in_marking, RG);
        if (prob) {
            out_prob.close();
        }
        out.close();
        cout << "\t END WRITING MDP" << endl;
    }

    if (PRISM) {
        net = std::string(argv[1]) + ".nm";
        out.open(net.c_str(), ofstream::out);
        if (!out) {
            cerr << "\n*****Error opening output stream .nm*****" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "\t START WRITING MDP" << endl;
        writeMDPxPrism(out, in_marking, RG);
        cout << "\t END WRITING MDP" << endl;
    }
    else if (MDP) {
        if (!TMDP) {
            cout << "\t START WRITING MDP" << endl;
            //open files
            net = std::string(argv[1]) + ".xml";
            out.open(net.c_str(), ofstream::out);
            if (!out) {
                cerr << "\n*****Error opening output stream .xml*****" << endl;
                exit(EXIT_FAILURE);
            }
            writeMDP(out, out_prob, out_avT, in_marking, RG);
            cout << "\t END WRITING MDP" << endl;
        }
        if (prob) {
            out_prob.close();
        }
    }
    else {
        //open file
        net = std::string(argv[1]) + ".pomdp";
        out.open(net.c_str(), ofstream::out);
        if (!out) {
            cerr << "\n*****Error opening output stream .pomdp*****" << endl;
            exit(EXIT_FAILURE);
        }
        //open file
        cout << "\t START WRITING POMDP" << endl;
        writePOMDP(in_marking, out, RG);
        cout << "\t END WRITING POMDP" << endl;
    }



//Close the stream
    in.close();
    out.close();

    in_marking.close();
//Close the stream

//delete(in);
//delete(out);
//delete(in_marking);

    time(&time_4);

    net = std::string(argv[1]) + ".trans";
    out.open(net.c_str(), ofstream::out);
    if (!out) {
        cerr << "\n*****Error opening output stream .trans*****" << endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < ntr; i++) {
        out << i << " " << tabt[i].trans_name << endl;
    }


    cout << "END EXECUTION" << endl;
    cout << "\n=============================== TIME ================================\n\n\t";
    cout << "Total time required: " << (time_4 - time_1) << "s\n\t Time Initialitation Net: " << (time_2 - time_1) << "s\n\t Time RG_MDP(RG_POMDP): " << (time_3 - time_2) << "s\n\t Time MDP(POMDP) writing:" << (time_4 - time_3);
    cout << "\n\n=============================== TIME ================================\n\n";
    exit(EXIT_SUCCESS);

}
