/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
 *  beccuti@mfn.unipmn.it   *
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
#include "./mdwnclass.h"


using namespace ParserDefNet;
using namespace general;

void open_nets(ifstream &in, ofstream &out, const std::string &net1, std::string net2) {
//read the components
    in.open(net1.c_str(), ifstream::in);
    if (!in) {
        cerr << "\n*****Error opening input stream .net*****" << endl;
        throw (ExceptionIO());
    }

    out.open(net2.c_str(), ios::trunc);
    if (!out) {
        cerr << "\n*****Error opening output stream .net*****" << endl;
        throw (ExceptionIO());
    }
}

int main(int argc, char *argv[]) {
    map <std::string, class component>CompAtt, CompPas;
    time_t time_1, time_2, time_3;

    time(&time_1);
    if ((argv[1] == NULL) || (argc < 3)) {
        std::cerr << "\n\nUSE: MDWN2WN probabilistic_net, non_deterministic_net\n\n";
        exit(EXIT_FAILURE);
    }
//read the components
    std::string net = std::string(argv[1]) + ".def";
    ifstream in(net.c_str(), ifstream::in);
    cout << "\n\n =========================================================\n";
    cout << "|	       MDWN/MDPN to WN/PN converter               |\n";
    cout << " =========================================================\n";
    cout << "\n If you find any bug, send an email to beccuti@di.unito.it\n";
    cout << "\n =========================================================\n\n";
    cout << "\nWorking on " << argv[1] << "\n\t(the probabilistic net)" << endl;

    if (!in) {
        cerr << "\n*****Error opening input stream .def*****\n" << endl;
        exit(EXIT_FAILURE);
    }
    try {
        readComponentNew(in, CompAtt, CompPas);
    }
    catch (ExceptionIO) {
        cout << "Error!!" << endl;
    };

    in.close();

//read the components
    ofstream out;
    net = std::string(argv[1]) + ".net";
    std::string net1 = std::string(argv[1]) + "M.net";
    try {
        open_nets(in, out, net, net1);
    }
    catch (ExceptionIO) {
        exit(EXIT_FAILURE);
    };
    try {
        create_net(in, out, CompAtt, CompPas, PROB);
    }
    catch (ExceptionIO obj) {
        cerr << "\n*****Error during the creation of the new probabilistic .net*****\n" << obj.get() << endl;
        exit(EXIT_FAILURE);
    };
//read  prob net --> build prob net + place run and stop
    out.close();
    in.close();

    cout << "\n\tGenerated correctly!!\n";
//cout<<"*****************************************************************\n";
    cout << "\nWorking on " << argv[2] << "\n\t(non deterministic net)" << endl;
    time(&time_2);
    net = std::string(argv[2]) + ".net";
    net1 = std::string(argv[2]) + "M.net";
    try {
        open_nets(in, out, net, net1);
    }
    catch (ExceptionIO) {
        exit(EXIT_FAILURE);
    };

//read  prob net --> build prob net + place run and stop
    try {
        create_net(in, out, CompAtt, CompPas, NOND);
    }
    catch (ExceptionIO obj) {
        cerr << "\n*****Error during the creation of the new non deterministic .net*****\n" << obj.get() << endl;
        exit(EXIT_FAILURE);
    }
    catch (ExceptionComponents()) {
        cerr << "\n*****Error during the creation: test all transition components*****" << endl;
        exit(EXIT_FAILURE);
    };
//read  prob net --> build prob net + place run and stop
    out.close();
    in.close();
    cout << "\n\tGenerated correctly!!\n\n";
    cout << "********************************************************************\n\n";
    cout << "\t\t\tEND EXECUTION" << endl;
    time(&time_3);
#if STEMP
    cout << "\n=============================== TIME================================\n\n\t";
    cout << "Total time required: " << (time_3 - time_1) << "s\n\tTime Probabilistic Net generation: " << (time_2 - time_1) << "s\n\tTime Non deterministic Net generation: " << (time_3 - time_2) << "s";
    cout << "\n\n=============================== TIME ===============================\n\n";
#endif
}
