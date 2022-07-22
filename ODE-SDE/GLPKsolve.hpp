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
#ifndef __STDL__
    #define __STDL__
    #include <stdlib.h>
#endif

#ifndef __STDR__
    #define __STDR__
    #include <stdarg.h>
#endif

#ifndef __IOS_H__
	#define __IOS_H__
	#include <iostream>
#endif

#ifndef __FSTREAM__
	#define __FSTREAM__
	#include <fstream>
#endif

#ifndef __UNORDERED_MAP__
    #define __UNORDERED_MAP__
    #include <unordered_map>
#endif

#ifndef __GLPK__
    #define __GLPK__
    #include <glpk.h>
#endif

#ifndef __GEN_H__
	#define __GEN_H__
	#include "general.h"
#endif

namespace FBGLPK{

  using namespace std;

    //!Exception
    struct Exception{
        std::string mess;
        public:
        //! Empty constructors
        Exception() {mess="";};
        //! constructor takes in input a message
        Exception(std::string mess) {this->mess=mess;};
        //! \name Get of methods use to access at the data structures
        //@{
        //!It returns message
        std::string what(void) {return mess;};
        //@}
    };

 
  //!Class LPprob
  /*!
   * This class implement LP problem exploiting GLPK
   */
    class LPprob{

        //!Pointer to LP problem encoded by GLPK
         glp_prob *lp {nullptr};
         //!Pointer to row indexes
         int *ia {nullptr};
         //!Pointer to column indexes
         int *ja {nullptr};
         //!Pointer to real problem values. It is connected to ia e ja as follows assuming the probem described by [Min CX, AX=B, Lbound<B<Ubound] then ai[i]=n  aj[i]=m ar[i]=v means A[n,m]=v
         double *ar {nullptr};

         double *Value {nullptr};
         //!It encodes the size of ia,ja,ar
         unsigned int sizeCol {0}; // Number of reactions
         unsigned int sizeRow {0}; // Number of metabolites
         unsigned int sizeVet {0};
         //!It is true when the LP problem is solved
         bool solved {false};
         //!It is the file name storing the Flux Balance problem 
         string filename {""};
         //!It stores the flux names and their numeric id
         unordered_map <string, unsigned int> ReactionsNamesId;
         //!It stores the flux names  ordered as stored in the file
         vector <string> ReactionsNamesOrd;
        public:     
        //! Empty Constructor.
        LPprob(){};
           //! Empty Constructor.
        LPprob(const LPprob& t){
            if (t.filename!=""){
                this->updateLP(t.filename.c_str());
            }
        };
        //! Constructor by file. It takes as input a file describing the LP problem
        LPprob( const char * FileProb);
        //! Constructor by file. It takes as input a file describing the LP problem
        void updateLP( const char * FileProb);
        //! Solve the LP problem
        void solve(){
            cout<<"\n\n-------------------------------------------------------"<<endl;
            glp_simplex(lp, NULL);
            solved=true;
            cout<<"-------------------------------------------------------\n"<<endl;
        };
        //! Return obj function value
        double getOBJ(){
            if (!solved) solve();
            return glp_get_obj_val(lp);
        };
        //! Return variable values
         double* getVariables(){
            if (!solved) solve();
            for (unsigned int i=1;i<=sizeCol;++i){
            Value[i]= glp_get_col_prim(lp, i);
            }
            return Value;
        };
        //! Return lower bound value
        double getLwBounds(int indexR){
	    double LB = glp_get_col_lb(lp, indexR);
            return LB;
        };
        //! Return uppper bound value
        double getUpBounds(int indexR){
	    double UB = glp_get_col_ub(lp, indexR);
            return UB;
        };
        //! Print the last GLPK solution
        void print(){
            if (!solved) solve();
            cout<<"Obj value:"<< getOBJ()<<endl<<endl;
            getVariables();
            auto it=ReactionsNamesOrd.begin();
            for (unsigned int i=1;i<=sizeCol;++i,++it){
                cout<<*it<<":"<<Value[i]<<endl;

            }
        };
        
        //! Print the flux values
        void printValue(ofstream& out){
            getVariables();
            for (unsigned int i=1;i<=sizeCol;++i){
                out<<" "<<Value[i];
            }
        };
        //! Print the flux names
        void printFluxName(ofstream& out){
            for (auto it=ReactionsNamesOrd.begin();it!=ReactionsNamesOrd.end();++it){
                out<<" "<<*it;
            }
        };

        //!Update the bound values of a specified flux
        void update_bound(int indexR, string TypeBound, double Lb, double Ub){
            glp_set_col_bnds(lp, indexR, setTypeBound(TypeBound) , Lb, Ub);
            cout<<"Bounds of "<< indexR <<" is updated as: ["<<Lb<<";"<<Ub<<"]"<<endl;
        };
        //! Return the numeric id of a flux. If the name is not present return -1
        int fromNametoid(const string& name){
            auto it=ReactionsNamesId.find(name);
            if (it!=ReactionsNamesId.end())
                return it->second;
            else
                return -1;

        }
        
        //! Deconstruct
        ~LPprob() {
            //--count;
            if (sizeVet){
            free(ia);
            free(ja);
            free(ar);
            glp_delete_prob(lp);
            //if (count==0)
            //    glp_free_env();
            };
        };
        private:
        //! It returns the bound type according to following values GLP_FR,GLP_LO,GLP_UP,GLP_DB,GLP_F
        int setTypeBound(string typeString);
        //! It returns the optimization direction according to  GLP_MAX,  GLP_MIN
        int setTypeObj(string typeString){
            int type;
            if (typeString.compare("GLP_MAX")==0)
                type=GLP_MAX;
            else
                type=GLP_MIN;
            return type;
        }
    };
}
