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


namespace FBGLPK{


LPprob::LPprob(const char* fileProb){

    try{
        ifstream in(fileProb, std::ifstream::in);
        if (!in){
            throw Exception(string("FLUX BALANCE: error opening input file:")+fileProb);
        }

        lp = glp_create_prob();
        glp_set_prob_name(lp,fileProb);


        class general::Parser parser;
        char delimC[] = "\t, ;\"";
        string buffer("");
        //reading flux names
        getline(in,buffer);
        parser.update(delimC,buffer);
        for (unsigned int i=0;i<parser.size();++i){
             ReactionsNamesId.insert({{ parser.get(i),i+1}});
              ReactionsNamesOrd.push_back(parser.get(i));
        }
        //reading flux names

        getline(in,buffer);
        parser.update(delimC,buffer);
        if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error second line: model dimension and type");
            }


        sizeRow=atoi(parser.get(0).c_str());
        sizeCol=atoi(parser.get(1).c_str());        
        sizeVet= sizeCol*sizeRow;
        int typeOBJ=setTypeObj(parser.get(2));

         if (ReactionsNamesId.size()!=sizeCol){
             
             throw Exception("FLUX BALANCE: error first line: the number of reaction names is different by the number of columns in Flux Balance problem");
        }

        // allocate memory for ia, ja, ar,
        ia=(int*)malloc(sizeof(int)*(sizeVet+1));
        ja=(int*)malloc(sizeof(int)*(sizeVet+1));
        ar=(double*)malloc(sizeof(double)*(sizeVet+1));
        Value=(double*)malloc(sizeof(double)*(sizeVet+1));

        //set optimization direction
        glp_set_obj_dir(lp,typeOBJ);

        //set number of rows
        glp_add_rows(lp, sizeRow);
        //set number of columns
        glp_add_cols(lp, sizeCol);

        //set obj coefficients


        getline(in,buffer);
        parser.update(delimC,buffer);

        if (parser.size()!=sizeCol){
              throw Exception("FLUX BALANCE: error wrong number of objective coefficients");
        }
        //set obj coefficients
        for (unsigned int i=0;i<parser.size();++i){
            glp_set_obj_coef(lp, i+1, atof(parser.get(i).c_str()));
        }

        // BOUNDS:
        // GLP_FR −∞ < x < +∞ Free (unbounded) variable -> the two parameters are ignored
        // GLP_LO lb ≤ x < +∞ Variable with lower bound -> the second parameter is ignored
        // GLP_UP −∞ < x ≤ ub Variable with upper bound -> the first parameter is ignored
        // GLP_DB lb ≤ x ≤ ub Double-bounded variable
        // GLP_FX lb = x = ub Fixed variable
        
        // the problem is S*x = 0 
        
        //set row bounds: s1*x1+s2*x2 = (< or >) ub (or lb) 
        unsigned int i;
        for (i=0;i<sizeRow&&!in.eof();++i){
            getline(in,buffer);
            parser.update(delimC,buffer);
            if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error wrong row bound format: [GLP_FR,GLP_LO,GLP_UP,GLP_DB,GLP_F] double double");
            }
           glp_set_row_bnds(lp,i+1, setTypeBound(parser.get(0)) , atof(parser.get(1).c_str()), atof(parser.get(2).c_str()));
        }
        if (i!=sizeRow)
            throw Exception("FLUX BALANCE: error less number of row bounds");

        //set colomn bound: the bounds of the lb<x<ub
        for (i=0;i<sizeCol&&!in.eof();++i){
            getline(in,buffer);
            parser.update(delimC,buffer);
            if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error wrong column bound format: [GLP_FR,GLP_LO,GLP_UP,GLP_DB,GLP_F] double double");
            }
           glp_set_col_bnds(lp,i+1, setTypeBound(parser.get(0)) , atof(parser.get(1).c_str()), atof(parser.get(2).c_str()));
        }
        if (i!=sizeCol)
            throw Exception("FLUX BALANCE: error less number of column bounds");

        //set ia ja ar
          for (i=0;i<sizeVet&&!in.eof();++i){
            getline(in,buffer);
            parser.update(delimC,buffer);
            if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error wrong column bound format: int int double");
            }
            ia[i+1]=atoi(parser.get(0).c_str());
            ja[i+1]=atoi(parser.get(1).c_str());
            ar[i+1]=atof(parser.get(2).c_str());

            //cout << "ia[" << i+1 << "]=" << ia[i+1] << endl;
            //cout << "ja[" << i+1 << "]=" << ja[i+1] << endl;
            //cout << "ar[" << i+1 << "]=" << ar[i+1] << endl;
        }
        if (i!=sizeVet)
            throw Exception("FLUX BALANCE: error wrong number of column bounds");
        glp_load_matrix(lp, sizeVet, ia, ja, ar);
        filename=string(fileProb);
    }
    catch (exception& e){
        cout << "Exception: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (Exception& e){
        cout << "Exception: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}


LPprob::LPprob(const char* FileProb,const char* FileInVar, const char* FileOutVar,int typeOBJ,const char* FluxName,const int gamma){
//creating LP problem
updateLP(FileProb,1,typeOBJ,FluxName);
//
 try{
    //opening input file
    in_var.open(FileInVar, std::ifstream::in);
    if (!in_var)
        throw Exception(string("FLUX BALANCE: error opening input file:")+std::string(FileInVar));
    if(in_var.eof())
        throw Exception(string("FLUX BALANCE: error input file:")+std::string(FileInVar)+string("is empty"));
    
    this->gamma=gamma;

    string buffer;
    getline(in_var,buffer);
    //reading flux names;
    //cout<<buffer<<endl

    //opening output file
    out_var.open(FileOutVar, std::ofstream::out);
    if (!in_var)
        throw Exception(string("FLUX BALANCE: error opening input file:")+std::string(FileOutVar));
    }
 catch (exception& e){
    cout << "\n Exception: " << e.what() << endl;
    exit(EXIT_FAILURE);
    }
 catch (Exception& e){
    cout << "\n Exception: " << e.what() << endl;
    exit(EXIT_FAILURE);
    }   
 }


void LPprob::updateLP( const char* fileProb,int variability, int typeOBJ,const char* FluxName){

    try{
        string var_obj_eq="";
        ifstream in(fileProb, std::ifstream::in);
        if (!in){
            throw Exception(string("FLUX BALANCE: error opening input file:")+fileProb);
        }
        
        lp = glp_create_prob();
        glp_set_prob_name(lp,fileProb);


        class general::Parser parser;
        char delimC[] = "\t, ;\"";
        string buffer("");
        
        //reading flux names
        getline(in,buffer);
        parser.update(delimC,buffer);
        for (unsigned int i=0;i<parser.size();++i){
            ReactionsNamesId.insert({{ parser.get(i),i+1 }});
            ReactionsNamesOrd.push_back(parser.get(i));
        }
        //reading flux names

        getline(in,buffer);
        parser.update(delimC,buffer);
        if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error first line: model dimension and type");
            }
       

        sizeRow=atoi(parser.get(0).c_str());
        sizeCol=atoi(parser.get(1).c_str()); 

//for variability
        if  (variability){
            std::unordered_map<std::string,unsigned int>::iterator it=ReactionsNamesId.find(string(FluxName));
            if (it==ReactionsNamesId.end())
                throw Exception(std::string(FluxName)+string(" is not a valid flux name"));
            else
                flux_var=it->second;
 ///add
        }  
//for variability

        sizeVet= sizeCol*sizeRow;
        cout<<"sizeVet"<<sizeVet<<" "<<sizeRow<<" "<<sizeCol<<" "<<variability<<" "<<sizeVet+1+variability*sizeCol<<endl;
        if (!variability)
        typeOBJ=setTypeObj(parser.get(2));

            
        if (ReactionsNamesId.size()!=sizeCol){
             throw Exception("FLUX BALANCE: error first line: the number of reaction names is different by the number of columns in Flux Balance problem");
        }


        // allocate memory for ia, ja, ar,
        ia=(int*)malloc(sizeof(int)*(sizeVet+1+variability*sizeCol));
        ja=(int*)malloc(sizeof(int)*(sizeVet+1+variability*sizeCol));
        ar=(double*)malloc(sizeof(double)*(sizeVet+1+variability*sizeCol));
        Value=(double*)malloc(sizeof(double)*(sizeVet+1+variability*sizeCol));

        //set optimization direction
        glp_set_obj_dir(lp,typeOBJ);

        //set number of rows
        glp_add_rows(lp, sizeRow+variability);
        //set number of columns
        glp_add_cols(lp, sizeCol);

  


        //set obj coefficients
        getline(in,buffer);

        if (!variability){
            parser.update(delimC,buffer);
            if (parser.size()!=sizeCol){
              throw Exception("FLUX BALANCE: error wrong number of objective coefficients");
            }
            for (unsigned int i=0;i<parser.size();++i){
                glp_set_obj_coef(lp, i+1, atof(parser.get(i).c_str()));
            }
        }else{
        //variability
        //resetting obj function coefficient
            var_obj_eq=buffer;
            for (unsigned int i=0;i<sizeCol;++i){
                glp_set_obj_coef(lp, i+1, (i==flux_var)?1:0);
            }
        //variability
        }
        // BOUNDS:
        // GLP_FR −∞ < x < +∞ Free (unbounded) variable -> the two parameters are ignored
        // GLP_LO lb ≤ x < +∞ Variable with lower bound -> the second parameter is ignored
        // GLP_UP −∞ < x ≤ ub Variable with upper bound -> the first parameter is ignored
        // GLP_DB lb ≤ x ≤ ub Double-bounded variable
        // GLP_FX lb = x = ub Fixed variable
        
        // the problem is S*x = 0 
        
        //set row bounds: s1*x1+s2*x2 = (< or >) ub (or lb) 
        unsigned int i;
        for (i=0;i<sizeRow&&!in.eof();++i){
            getline(in,buffer);
            parser.update(delimC,buffer);
            if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error wrong row bound format: [GLP_FR,GLP_LO,GLP_UP,GLP_DB,GLP_F] double double");
            }
           glp_set_row_bnds(lp,i+1, setTypeBound(parser.get(0)) , atof(parser.get(1).c_str()), atof(parser.get(2).c_str()));
        }
        if (i!=sizeRow)
            throw Exception("FLUX BALANCE: error wrong number of row bounds");

        //set colomn bound: the bounds of the lb<x<ub
        for (i=0;i<sizeCol&&!in.eof();++i){
            getline(in,buffer);
            parser.update(delimC,buffer);
            if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error wrong column bound format: [GLP_FR,GLP_LO,GLP_UP,GLP_DB,GLP_F] double double");
            }
           glp_set_col_bnds(lp,i+1, setTypeBound(parser.get(0)) , atof(parser.get(1).c_str()), atof(parser.get(2).c_str()));
        }
        if (i!=sizeCol)
            throw Exception("FLUX BALANCE: error wrong number of column bounds");

        //set ia ja ar
          for (i=0;i<sizeVet&&!in.eof();++i){
            getline(in,buffer);
            parser.update(delimC,buffer);
            if (parser.size()!=3){
                throw Exception("FLUX BALANCE: error wrong column bound format: int int double");
            }
            ia[i+1]=atoi(parser.get(0).c_str());
            ja[i+1]=atoi(parser.get(1).c_str());
            ar[i+1]=atof(parser.get(2).c_str());

            //cout << "ia[" << i+1 << "]=" << ia[i+1] << endl;
            //cout << "ja[" << i+1 << "]=" << ja[i+1] << endl;
            //cout << "ar[" << i+1 << "]=" << ar[i+1] << endl;
        }
        if (i!=sizeVet)
            throw Exception("FLUX BALANCE: error wrong number of column bounds");
        if (variability){
            //adding new row based on old obj
            parser.update(delimC,var_obj_eq);
            if (parser.size()!=sizeCol)
              throw Exception("FLUX BALANCE: error wrong number of coefficients for varibility previous obj");

            int i=sizeVet+1; 
            for (unsigned int j=0;j<parser.size();++j){
                //cout<<"val: "<<i<<" "<<sizeRow+variability<<" "<<j+1<<" "<<atof(parser.get(i).c_str())<<endl;
                ia[i]=sizeRow+variability;
                ja[i]=j+1;
                ar[i]=atof(parser.get(i).c_str());
                ++i;
                }
        }
        //adding new row based on old obj
        glp_load_matrix(lp, sizeVet+variability*sizeCol, ia, ja, ar); 
        filename=string(fileProb);     
    }
    catch (exception& e){
        cout << "Exception: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (Exception& e){
        cout << "Exception: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void LPprob::solveVariability(){
    string buffer;
    out_var<<"Time Obj"<<endl;
    try{
        while (!in_var.eof()){
        getline(in_var,buffer); 
        class general::Parser par(" ",buffer);
        //cout<<"tot:"<<par.size()<<" sizeCol"<<sizeCol<<endl;
        if (par.size()!=0){
         for (unsigned int i=sizeCol+2,j=1; i<par.size();i=i+2,++j){ //+2 is due to time and obj
            //updating bound
            update_bound(j,get_bound_type(j),atof(par.get(i).c_str()),atof(par.get(i+1).c_str()));
            }
         //updating new equation based on old obj
         glp_set_col_bnds(lp,sizeRow+1, GLP_LO, atof(par.get(0).c_str())*gamma, atof(par.get(0).c_str())*gamma);
         solve();
         out_var<<par.get(0)<<" "<< glp_get_obj_val(lp)<<endl;   
        }
        }
     }
 catch (exception& e){
    cout << "\n Exception: " << e.what() << endl;
    exit(EXIT_FAILURE);
    }
 catch (Exception& e){
    cout << "\n Exception: " << e.what() << endl;
    exit(EXIT_FAILURE);
    }       
}

int LPprob::setTypeBound(string typeString){
    int type;
    if (typeString.compare("GLP_FR")==0)
        type=GLP_FR;
    else  if (typeString.compare("GLP_LO")==0)
        type=GLP_LO;
    else  if (typeString.compare("GLP_UP")==0)
        type=GLP_UP;
    else  if (typeString.compare("GLP_DB")==0)
        type=GLP_DB;
    else
        type=GLP_FX;
    return type;
}


}
