/***************************************************************************
 *   Copyright (C) 2021 by Marco Beccuti				                   *
 *   marco.beccuti@unito.it						                           *
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

#ifndef __CGLPK_H__
	#define __CGLPK_H__
	#include "GLPKsolve.hpp"
#endif

/*
#ifndef OMP_H_
    #define OMP_H_
    #include <omp.h>
#endif
*/

#include <sys/time.h>
#include <sys/resource.h>

using namespace FBGLPK;

int main(int argc, char *argv[]){

    clock_t startGlobal,endGlobal;
    startGlobal=clock();
cout<<"\n\n =========================================================\n";
cout<<"| 	          Variability analysis based on GLPK     	          |\n";
cout<<" =========================================================\n";
cout<<"\n If you find any bug, send an email to marco.beccuti@unito.it\n";

if (argc<5)
	{
	std::cerr<<"\n\nUSE: VARIABILITY <out_file> <Flux_Balance_Problem> <Solutions> <FLUX> <gamma>\n\n";
		exit(EXIT_FAILURE);
	}


//omp_set_num_threads(2);
try{
    class LPprob lmax(argv[2],argv[3],string(string(argv[1])+"_MAX").c_str(),GLP_MAX,argv[4],atof(argv[5]));
    class LPprob lmin(argv[2],argv[3],string(string(argv[1])+"_MIN").c_str(),GLP_MIN,argv[4],atof(argv[5]));   
    cout<<"\nSolution:\n\n";
    lmax.solveVariability();
    lmin.solveVariability();   
}
catch (Exception& e){
        cout << "\n\n Exception: " << e.what() << endl<<endl;
}
endGlobal=clock();
cout<<"\n=========================== TIME ===========================\n\n";
cout<<"\t Execution time: "<<((double)(endGlobal-startGlobal))/CLOCKS_PER_SEC<<"s."<<endl;
cout<<"\n============================================================\n\n";
}
