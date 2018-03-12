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

using namespace general;

bool reward = false, mte = false, mat = false;


int main(int argc, char *argv[]) {

    map <std::string, map <std::string,  list <class Elem> > > trans;
    int step = 0;


    if (argv[1] == NULL) {
        std::cerr << "\n\nUSE: GDTMC <model name> -H \n -H for finite horizon\n\n";
        exit(EXIT_FAILURE);
    }

    cout << "\n\n =========================================================\n";
    cout << "|	         MDP to DTMC converter                |\n";
    cout << " =========================================================\n";
    cout << "\n If you find any bug, send an email to beccuti@di.unito.it\n";

    /*open files*/
    std::string tmp = std::string(argv[1]) + ".otp";
    std::ifstream fp_write(tmp.c_str(), std::ifstream::in);
    if (!fp_write) {
        cerr << "\n*****Error opening the input stream .otp****" << endl;
        exit(EXIT_FAILURE);
    }

    tmp = std::string(argv[1]) + ".oar";
    std::ifstream fp_writeoar(tmp.c_str(), std::ifstream::in);

    if (!fp_writeoar) {
        cerr << "\n*****Error opening the input stream .oar****" << endl;
        exit(EXIT_FAILURE);
    }


    tmp = std::string(argv[1]) + ".mte";
    std::ifstream fp_mte(tmp.c_str(), std::ifstream::in);
    if (!fp_mte) {
        cerr << "\n*****I cannot open the input stream .mte****" << endl;
    }
    else {
        mte = true;
    }
    tmp = std::string(argv[1]) + ".mat";
    std::ifstream fp_mat(tmp.c_str(), std::ifstream::in);
    if (!fp_mat) {
        cerr << "\n*****I cannot open the input stream .mat****" << endl;
    }
    else {
        mat = true;
    }

    /*open files*/


    set< class State >IntState;

    std::ifstream fp_readInput;

    cout << "\t START INITIALIZATION" << endl;
    initialize(2, argv);
    cout << "\t END INITIALIZATION" << endl;

//use for compute reward
    if ((argc > 2) && (argv[2][1] == 'r')) {
        reward = true;
        name2id();
    }

//use for compute reward
    if ((argc > 2) && (argv[2][1] == 'H')) {
        tmp = std::string(argv[1]) + ".step";
        fp_readInput.open(tmp.c_str(), std::ifstream::in);
        if (!fp_readInput) {
            std::cout << "\n\n the file " << argv[1] << ".step is not found\n\n I'm creating it\n\n";
            std::ofstream fp_readOutput;
            fp_readOutput.open(tmp.c_str(), ofstream::out);
            if (!fp_readOutput) {
                exit(EXIT_FAILURE);
            }
            else {
                fp_readOutput << "Esempio DTMC: " << argv[1] << "\n*!*\nProb.: 0.0000000\n0\n0\nT1 1";
                fp_readOutput.close();
                exit(0);
            }

        }
        else {
            try {
                step = read_Input(fp_readInput, IntState);
            }
            catch (ExceptionIO obj) {
                cerr << obj.get() << endl << endl;
                exit(1);
            }
        }
    }
//
    /*if (!fp_write)
    	{
    	std::cerr<<"\n\n the file " <<argv[1]<<".otp is not found\n\n";
    	exit(EXIT_FAILURE);
    	}
    if (!fp_writeoar)
    	{
    	std::cerr<<"\n\n the file " <<argv[1]<<".oar is not found\n\n";
    	exit(EXIT_FAILURE);
    	}
    //if ((argv[2][1]=='H')&&(!fp_readInput))
    //	{
    //	std::cerr<<"\n\n the file " <<argv[1]<<".step is not found\n\n";
    //	exit(EXIT_FAILURE);
    //	}
    if (!fp_mte)
    	{
    	std::cerr<<"\n\n the file " <<argv[1]<<".mte is not found\n\n";
    	exit(EXIT_FAILURE);
    	}
    */
    try {
        build_transitions(fp_writeoar, trans);
        if ((argc > 2) && (argv[2][1] == 'H')) {
            //
            //buildDTMCh(fp_write,fp_mte,trans,argv[1]);
            buildDTMChI(fp_write, fp_mte, trans, argv[1], IntState, step);
            //
        }
        else if ((argc > 2) && (argv[2][1] == 'h')) {
            buildDTMCh(fp_write, fp_mte, trans, argv[1]);
        }
        else {
            buildDTMC(fp_write, fp_mte, fp_mat, trans, argv[1]);
        }
    }
    catch (ExceptionIO obj) {
        cerr << "\n*****Error:*****\n" << obj.get() << endl;
    }

//close files
    fp_write.close();
    if (mte)
        fp_mte.close();
    if (mat)
        fp_mat.close();
    fp_writeoar.close();
//close files
}

