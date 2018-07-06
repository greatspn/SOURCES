#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include<map>
#include<set>
#include <sys/resource.h>
#include <utility>
#include <limits.h>
#include <vector>
#include <unistd.h>
#include "general.h"
#ifndef __SET_H__
#define __SET_H__
#include <set>
#endif

#ifndef __LST_H__
#define __LST_H__
#include <list>
#endif


#ifndef __MAP_H__
#define __MAP_H__
#include <map>
#endif


extern "C" {


#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/ealloc.h"







    Tree_p reached_marking = NULL;
    Tree_p initial_marking = NULL;
    Tree_p current_marking = NULL;

    Result_p enabled_head = NULL;	/* Puntatori alla lista delle tr. abil. */

    int tro;			/* 1 marc. gia' raggiunta, 0 altrimenti */
    int marcatura = 0;	/* contatore per le marcature		*/
    int h = 0;			/* per bilanciamento nella insert_tree	*/
    unsigned long tang = 0;		/* contatori tipi di marc. raggiunte	*/
    unsigned long evan = 0;
    unsigned long dead = 0;
    int home = 0;
    unsigned long cont_tang;
    int count_arc = 0;


    int cur_priority;

    int *code_place = NULL;
    int *min_place = NULL;
    int *max_place = NULL;
    int *init_place = NULL;
    char IstanceName[MAX_TAG_SIZE];
    int max_priority = 0;
    time_t old_time, new_time;




    unsigned long d_ptr;
    unsigned long length;
    unsigned long f_mark;
    int f_bot;
    int f_throu;
    int f_tang;

    extern bool MASSACTION;
    extern bool AUTOMATON;

    int exceeded_markings_bound()
    {
        return FALSE;
    }
}

using namespace std;
using namespace general;
bool ErrorLU=false;

extern std::map<std::string,int> Place2Int;
extern std::string parseObjectiveFunction(const std::string& of_name,const std::string& a_name);

void LU(double **A, int *P, double *B, const int rows, const int cols)
{

    for (int i = 0; i < rows - 1; i++)
    {
        int maxPivot = i;
        for (int k = i + 1; k < rows; k++)
            if (fabs(A[k][i]) > fabs(A[i][i]))
                maxPivot = k;

        // check for singularity
        if (0 == A[maxPivot][i])
        {
            cout << "matrix is singular" << endl;
            ErrorLU=true;
            return;
        }

        //swap
        if (maxPivot != i)
        {
            swap(P[i], P[maxPivot]);
            swap(B[i], B[maxPivot]);
        }
        for (int k = i; k < cols; k++)
        {
            swap(A[i][k], A[maxPivot][k]);
        }

        //swap

        for (int k = i + 1; k < rows; k++)   // iterate down rows
        {
            // lower triangle factor is not zeroed out, keep it for L
            A[k][i] /= A[i][i]; // denominator is really for the pivot row elements
            for (int j = i + 1; j < cols; j++)  // iterate across rows
            {
                // subtract off lower triangle factor times pivot row
                A[k][j] = A[k][j] - A[i][j] * A[k][i];
            }

            B[k] = B[k] - B[i] * A[k][i] ;
        }

    }

}


int read_pinv(std::string &net, double **A, int *P, double *B, map <int, list < pair<int, int> > > &implPlace)
{
    cout << "Opening file: " << std::string(net + ".pin").c_str() << endl;
    ifstream in(std::string(net + ".pin").c_str());
    if (!in)
    {
        cerr << "Error: it is not possible to open input file .pin\n\n";
        exit(EXIT_FAILURE);
    }
    int i = 0;
    int npinv;
    in >> npinv;
    cout << "Number of P-invs: " << npinv << endl;
    A = new double* [npinv];
    P = new int [npinv];
    B = new double [npinv];
    for (int i = 0; i < npinv; i++)
    {
        P[i] = i; //row swapping permutations, initially identity
        B[i] = 0;
        A[i] = new double[npl];
        for (int j = 0; j < npl; j++)
            A[i][j] = 0.0;
    }
    int col, row = 0;
    double val = 0;
    while (in)
    {
        int nelm;
        in >> nelm;
        // cout<<"nelm "<<nelm<<endl;
        for (int i = 0; i < nelm; i++)
        {
            in >> val >> col;
            // cout << "Val"<< val<<" col"<<col<<endl;
            A[row][col - 1] = val;
            B[row] += net_mark[col - 1].total;
        }
        row++;
    }
    cout << "\n\tP-invariant: \n\t";
    for (int i = 0; i < npinv; i++)
    {

        for (int j = 0; j < npl; j++)
        {
            cout << " " << A[i][j];
        }
        cout << " | " << B[i] << "\n\t";
    }
    LU(A, P, B, npinv, npl);
    cout << "\n\tU\n\t";
    for (int i = 0; i < npinv; i++)
    {
        bool first = true;
        int pl;
        double div;
        for (int j = 0; j < npl; j++)
        {
            if (j < i)
                cout << " 0";
            else
            {
                cout << " " << A[i][j];
                if ((A[i][j] != 0))
                {
                    if (first)
                    {
                        pl = j;
                        first = false;
                        div = A[i][j];
                    }
                    else
                    {
                        implPlace[pl].push_back(make_pair(j, A[i][j] / div));
                        //cout<<pl<<" "<<j<<" "<<A[i][j]/div<<endl;

                    }
                }
            }

        }
        cout << " | " << B[i] << "\n\t";
        implPlace[pl].push_back(make_pair(-1, B[i] / div));
        //cout<<pl<<" -1 "<<B[i]/div<<endl;
    }
    cout << "\n\tL\n\t";
    for (int i = 0; i < npinv; i++)
    {
        for (int j = 0; j < npl; j++)
        {
            if (j > i)
                cout << " 0";
            else if (i == j)
                cout << " 1";
            else
                cout << " " << A[i][j];

        }
        cout << "\n\t";
    }
    cout << "\n\tP\n\t";
    for (int i = 0; i < npinv; i++)
    {
        cout << " " << P[i];
    }

    return npinv;
}

//!It takes in input a transition ID  and returns its corresponding preset.
void getPreTR(const int tID, int *preset)
{
    Node_p l_ptr = NULL;
    int pp;
    for (int ii = 0; ii < npl; ii++)
    {
        preset[ii] = 0;
    }
    l_ptr = GET_INPUT_LIST(tID);
    while (l_ptr != NULL)
    {
        pp = GET_PLACE_INDEX(l_ptr);
        preset[pp] = l_ptr->molt;
        l_ptr = NEXT_NODE(l_ptr);
    }
}

//!It takes in input a transition ID  and returns its corresponding inhibitor vector.
void getInhTR(const int tID, int *set)
{
    Node_p l_ptr = NULL;
    int pp;
    l_ptr = GET_INHIBITOR_LIST(tID);
    while (l_ptr != NULL)
    {
        pp = GET_PLACE_INDEX(l_ptr);
        set[pp] = l_ptr->molt;
        l_ptr = NEXT_NODE(l_ptr);
    }
}



/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void read_bound(std::string &net, int *lbound, int *ubound)
{
    cout << "Opening file: " << std::string(net + ".bnd").c_str() << endl;
    ifstream in(std::string(net + ".bnd").c_str());
    if (!in)
    {
        cerr << "Error: it is not possible to open input file .bnd\n\n";
        exit(EXIT_FAILURE);
    }
    int i = 0;
    while (in)
    {
        in >> (lbound[i]) >> (ubound[i]);
        if (ubound[i] == -1)
            ubound[i] = INT_MAX;
        ++i;
    }
#if DEBUG
    for (int i = 0; i < npl; i++)
        cout << "Bound place " << i << " " << lbound[i] << " " << ubound[i] << endl;
#endif
}




/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void build_ODE(ofstream &out, std::string path, std::string net)
{
    /* Init build_ODE */



    Node_p l_ptr = NULL;
    int pp;
    clock_t startGlobal, endGlobal;
    double timeGlobal;
    startGlobal = clock();
    char delims[]=": ()\n\r\t";

    cout << "\n\n------------------------------------------------" << endl;
    cout << "               Start  encoding" << endl;
    cout << "------------------------------------------------\n" << endl;


    out << "\n#include <iostream>\n#include \"class.hpp\"\n\n";
    out << "\n#include <iostream>\n#include \""<<path<<".hpp\"\n\n";

    //for transition function rates
    std::string filename=path+".hpp";
    ofstream hout(filename.c_str());

    if (!hout)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }
    hout << "namespace SDE {\n";
    set<std::string> function_names;
    for (int tt = 0; tt < ntr; tt++)
    {
        if (tabt[tt].general_function!=NULL){
            //cout<<tabt[tt].general_function
            std::string tmp_st(tabt[tt].general_function);
            char* stoken=strtok((char*)tmp_st.c_str(),delims);
            vector<std::string>token;
            while (stoken!=NULL){
                token.push_back(stoken);
                stoken=strtok(NULL,delims);
            }
            if ((token.size()>1)&&( (token[1]!="Discrete" || token[1]!="discrete" || token[1]!="DISCRETE"))){
                if (function_names.find(token[1])==function_names.end()){
                    //cout<<token[1]<<endl;
                    hout<<"double "<<token[1]<<"(double *Value, map <std::string,int>& NumTrans,  map <std::string,int>& NumPlaces,const vector <string>& NameTrans, const struct InfTr* Trans, const int Tran, const double Time);\n";
                    function_names.insert(token[1]);
                }
            }
        }
    }
    hout<<"};\n";
    hout.close();
        //for transition function rates
        out << "using namespace SDE;\nextern double epsilon;\n\n";

    out << " string places[]={";
    for (int i = 0; i < npl; i++)
    {
        if (i != 0)
            out << ",";
        out << "\"" << tabp[i].place_name << "\"";
    }
    out << "};\n";


    out << " string transitions[]={";
    for (int i = 0; i < ntr; i++)
    {
        if (i != 0)
            out << ",";
        out << "\"" << tabt[i].trans_name << "\"";
    }
    out << "};\n";

    int *lbound = (int *) malloc(sizeof(int) * npl);
    int *ubound = (int *) malloc(sizeof(int) * npl);
    memset(lbound, 0, npl);
    memset(ubound, 0, npl);
    read_bound(path, lbound, ubound);
    double **A, *B;
    int *P;
    map <int, list < pair<int, int> > > implPlace;

    int pinv=read_pinv(path,A,P,B,implPlace);


    int **TP = (int **) malloc((ntr) *  sizeof(int *));
    int **TPI = (int **) malloc((ntr) *  sizeof(int *));
    for (int tt = 0; tt < ntr; tt++)
    {
        TP[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TP[tt], 0, npl * sizeof(int));
        TPI[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TPI[tt], 0, npl * sizeof(int));
    }




    out << "\nint main(int argc, char **argv) {\n\n";
    out << " time_t time_1,time_4;\n";
    out<<  " int who = RUSAGE_SELF;\n struct rusage usage;\n";
    out << " int SOLVE = -1, OUTPUT=false; //SOLVE = 0 --> (H)SDE/SIM,  SOLVE = 1 --> ODE  SOLVE = 2 -->HODE\n\n";
    out << " cout<<\"\\n\\n =========================================================\\n\";\n";
    out << " cout<<\"|	              ODE/SDE Solver                       |\\n\";\n";
    out << " cout<<\" =========================================================\\n\";\n";
    out << " cout<<\"\\n If you find any bug, send an email to beccuti@di.unito.it\\n\";\n\n";



    //automaton
    if (AUTOMATON)
    {
        out << " if (argc<11)\n\t{\n \t";
        out << " std::cerr<<\"\\n\\nUSE:" << net << "_solve <out_file> <type> <step_factor>  <perc1> <perc2> <runs> <Max_time> <output> <step_output> <automaton_file> -B <bound_file> \";\n\t";

    }
    else
    {
        out << " if (argc<10)\n\t{\n \t";
        out << " std::cerr<<\"\\n\\nUSE:" << net << "_solve <out_file> <type> <step_factor> <perc1> <perc2> <runs> <Max_time> <output> <step_output> -B <bound_file> \";\n\t";
    }
    //automaton
    out << " std::cerr<<\"\\n\\t <type>:\\t ODE-E or ODE-RKF or ODE45 or LSODA or HLSODA or (H)SDE or HODE or SIM or STEP:\";\n\t";
    out << " std::cerr<<\"\\n\\t <hini>:\\t Initial step step size\";\n\t";
    out << " std::cerr<<\"\\n\\t <rtol>:\\t Value used to compute Step (smaller -> greater precision but slower solution)\";\n\t";
    out << " std::cerr<<\"\\n\\t <atol>:\\tValue used to compute Euler Step when the model is closed to bounds (smaller -> greater precision but slower solution)\";\n\t";
    out << " std::cerr<<\"\\n\\t <runs>:\\t integer number corresponding to runs (only for (H)SDE)\";\n\t";
    out << " std::cerr<<\"\\n\\t <Max_time>:\\t double number used to set the upper bound of the evolution time\";\n\t";
    out << " std::cerr<<\"\\n\\t <output>:\\t true/false to save traces\";\n\t";
    out << " std::cerr<<\"\\n\\t <step_output>:\\t double number used to set the step in the output\";\n\t";
    out << " std::cerr<<\"\\n\\t <bound_file>:\\t soft bound are defined in the file <bound_file>\";\n\t";
    //automaton
    if (AUTOMATON)
        out << "std::cerr<<\"\\n\\t <automaton_file>:\\t automaton is defined in the file <automaton>\\n\";";
    out <<"std::cerr<<endl<<endl;";
    //automaton
    out << "\n\t exit(EXIT_FAILURE);\n\t}\n\n";
    out << " double step=atof(argv[3]);\n";
    out << " double perc1=atof(argv[4]);\n";
    out << " double perc2=atof(argv[5]);\n";
    out << " int  runs=atoi(argv[6]);\n";
    out << " double Max_time=atof(argv[7]);\n\n";
    out << " if ((strcmp(argv[2],\"ODE-E\")==0)||(strcmp(argv[2],\"ode-e\")==0))\n\t SOLVE = 1;\n";
    out << " if ((strcmp(argv[2],\"ODE-RKF\")==0)||(strcmp(argv[2],\"ode-rkf\")==0))\n\t SOLVE = 5;\n";
    out << " if ((strcmp(argv[2],\"ODE45\")==0)||(strcmp(argv[2],\"ode45\")==0))\n\t SOLVE = 6;\n";
    out << " if ((strcmp(argv[2],\"LSODA\")==0)||(strcmp(argv[2],\"lsoda\")==0))\n\t SOLVE = 7;\n";
    out << " if ((strcmp(argv[2],\"STEP\")==0)||(strcmp(argv[2],\"step\")==0))\n\t SOLVE = 4;\n";
    out << " if ((strcmp(argv[2],\"SIM\")==0)||(strcmp(argv[2],\"sim\")==0))\n\t{\n";
    out << "\t cout<<\"\\t using simulation\"<<endl;\n\t epsilon=10000000000;\n\t step=MAXSTEP;\n\t SOLVE=3;\n\t}\n";
    out << " if ((strcmp(argv[2],\"HODE\")==0)||(strcmp(argv[2],\"hode\")==0))\n\t SOLVE = 2;\n";
    out << " if ((strcmp(argv[2],\"HLSODA\")==0)||(strcmp(argv[2],\"hlsoda\")==0))\n\t SOLVE = 8;\n";
    out << " if ((strcmp(argv[2],\"SDE\")==0)||(strcmp(argv[2],\"sde\")==0) || (strcmp(argv[2],\"HSDE\")==0)||(strcmp(argv[2],\"hsde\")==0) )\n\t SOLVE = 0;\n";

    out << " if ((!strcmp(argv[8],\"true\"))||(!strcmp(argv[6],\"TRUE\")))\n\t OUTPUT= true;\n";
    out << " double step_o=atof(argv[9]);\n";

    out << " time(&time_1);\n\n";

    out << " cout<<\"\\n=====================INPUT PARAMETERS======================\\n\";\n";
    out << " cout<<\"\\n\\tType solution: \"<<argv[2]<<\"\\n\";\n";
    if (MASSACTION)
        out << " cout<<\"\\n\\tTransition policy: Genelarized Mass Action policy\\n\";\n";
    else
        out << " cout<<\"\\n\\tTransition policy: Minimum\\n\";\n";
    out << " cout<<\"\\tSolution end time: \"<<Max_time<<\"\\n\";\n";
    out << " cout<<\"\\tSolution step: \"<<step<<\"\\n\";\n";
    out << " if ((strcmp(argv[2],\"ODE\")!=0)&&(strcmp(argv[2],\"ode\")!=0)){\n";
    out << "\tcout<<\"\\tSolution runs: \"<<runs<<\"\\n\";\n }\n";
    //automaton
    if (AUTOMATON)
        out << " cout<<\"\\tAutomaton input: \"<<argv[10]<<\"\\n\";\n";
    //automaton
    out << " cout<<\"\\tDetailed output: \"<<argv[9]<<\"\\n\";\n";
    out << " cout<<\"\\n===========================================================\\n\";";
    out << " cout<<\"\\n\\nSTART EXECUTION...\"<<endl;\n\n";

    cout << "\n\nSTART EXECUTION..." << endl;
    out << " struct InfPlace pt;\n";
    out << " struct InfTr t;\n";
    out << " Equation eq;\n Elem el;\n";
    if (MASSACTION)
        out << " SystEqMas se(" << npl << "," << ntr << ",places,transitions);\n";
    else
        out << " SystEqMin se(" << npl << "," << ntr << ",places,transitions);\n";
    out << " vector< struct InfPlace> Vpl;\n\n";
    for (int tt = 0; tt < ntr; tt++)
    {


        out << "//Transition " << tabt[tt].trans_name << "\n t.InPlaces.clear();\n t.InhPlaces.clear();\n t.InOuPlaces.clear();\n t.Places.clear();\n";

        if (tabt[tt].general_function!=NULL)
        {
            cout<<tabt[tt].general_function<<endl;
            std::string tmp_st(tabt[tt].general_function);
            char* stoken=strtok((char*)tmp_st.c_str(),delims);
            vector<std::string>token;
            while (stoken!=NULL){
                token.push_back(stoken);
                stoken=strtok(NULL,delims);
            }

            out<<" t.GenFun= \""<<token[1]<<"\";\n";
            if (!(token[1]=="Discrete" || token[1]=="discrete" || token[1]=="DISCRETE")){
                out<<" t.FuncT=  &"<<token[1]<<";\n";
                out<<" t.rate = 1.0;\n";

                }
            else
                if (token.size()>2){
                    out<<" t.FuncT=nullptr;\n";
                    out<<" t.rate = "<<token[2]<<";\n";
                    }

                else{
                    cerr << "Error: Discrete transition without rate: "<<tabt[tt].trans_name<<"\n\n";
                    exit(EXIT_FAILURE);
                    }
        }
        else{
            out<<" t.GenFun=\"\";\n t.FuncT=nullptr;\n";
            out<<" t.rate = "<<tabt[tt].mean_t << ";\n";
            }
        l_ptr = GET_INPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] = -l_ptr->molt;
            TPI[tt][pp] = -l_ptr->molt;
            out << " pt.Id = " << pp << ";\n pt.Card = " << l_ptr->molt << ";\n t.InPlaces.push_back(pt);\n"<<" t.InOuPlaces.insert("<<pp<<");\n";
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_INHIBITOR_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            //TP[tt][pp]+=l_ptr->molt;
            out << " pt.Id = " << pp << ";\n pt.Card = " << l_ptr->molt << ";\n t.InhPlaces.push_back(pt);\n";
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_OUTPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] += l_ptr->molt;
            out <<" t.InOuPlaces.insert("<<pp<<");\n";
            l_ptr = NEXT_NODE(l_ptr);
        }

        for (int i=0;i<npl;i++){
            if (TP[tt][i]!=0){
                out << " pt.Id = " << i << ";\n pt.Card = " << TP[tt][i] << ";\n t.Places.push_back(pt);\n";
            }
        }


        out << " se.InsertTran(" << tt << ",t);\n\n";
    }
//to remove implicit places
    if (ErrorLU)
    {
        cout<<"\n\tWarning: no imlicit places are considered.\n\n";
        implPlace.clear();
    }
    //to remove implicit places

    for (pp = 0; pp < npl; pp++)
    {
        out << "//Place " << tabp[pp].place_name << "\n eq.clear();\n";
        if (implPlace.find(pp) == implPlace.end())   //explicit place
        {
            for (int tt = 0; tt < ntr; tt++)
            {
                if (TP[tt][pp] != 0)
                {
                    out << " el.setIncDec(" << TP[tt][pp] << ");\n el.setIdTran(" << tt << ");\n eq.Insert(el);\n";
                }
            }//explicit place
        }
        else   // implicit place
        {
            out << " Vpl.clear();\n";
            list < pair<int, int> >::iterator it;
            for (it = implPlace[pp].begin(); it != implPlace[pp].end(); ++it)
            {
                out << " pt.Id = " << it->first << ";\n";
                out << " pt.Card = " << it->second << ";\n";
                out << " Vpl.push_back(pt);\n";
            }
            out << " eq.Insert(Vpl);\n";
        }// implicit place
        out << " se.InsertEq(" << pp << ",eq," << net_mark[pp].total << "," << lbound[pp] << "," << ubound[pp] << ");\n\n";
    }
    //automaton
    if (AUTOMATON)
        out << "\n if ((SOLVE!=3)&&(argc>10) && (argv[10][1]=='B')) {\n\tse.readSLUBounds(argv[11]);\n }";
    else
        out << "\n if ((SOLVE!=3)&&(argc>9) && (argv[9][1]=='B')) {\n\tse.readSLUBounds(argv[10]);\n }";
    //automaton
    cout << "\tDone.\n" << endl;

    out << "\n se.Print();\n";
    //automaton
    if (AUTOMATON)
    {
        out << "\n cout<<\"\\n\\nREADING AUTOMATON...\"<<endl;\n";
        out << " se.initialize_automaton(argv[10]);\n";
        out << " cout<<\"\\n\\nDONE.\"<<endl;\n";
    }
    //automaton
    out << "\n\ntry\t{";
    out << "\n\tif (SOLVE==-1) \{\n\t\t cerr<< \"\\n\\nError: solution methods \"<<argv[2]<<\" is not implemented\\nYou should use:  ODE-E or ODE-RKF or ODE45 or LSODA or SDE or HODE or HSDE or SIM or STEP\\n\"; \n\t\t exit(EXIT_FAILURE);\n\t}\n\n ";



    out << "\n\tif (SOLVE == 1)\n\t\t se.SolveODEEuler(step,perc1,perc2,Max_time,OUTPUT,step_o,argv[1]);\n\t else\n\t\t if (SOLVE == 0)\n\t\t\t se.SolveSDEEuler(step,perc1,perc2,Max_time,runs,OUTPUT,step_o,argv[1]);\n\t\t else \n\t\t\tif (SOLVE == 3)\n\t\t\t\t se.SolveHODEEuler(step,perc1,perc2,Max_time,runs,OUTPUT,step_o,argv[1]); \n\t\t\t else \n\t\t\t\t if (SOLVE == 4)\n\t\t\t\t\t  se.HeuristicStep(step,perc1,perc2,Max_time,OUTPUT,step_o,argv[1]);   \n\t\t\t\t else\n\t\t\t\t\t if (SOLVE == 5)\n\t\t\t\t\t  se.SolveODERKF(step,perc1,Max_time,OUTPUT,step_o,argv[1]);   \n\t\t\t\t else\n\t\t\t\t\tif (SOLVE == 6)\n\t\t\t\t\t\t se.SolveODE45(step,perc1,Max_time,OUTPUT,step_o,argv[1]);\n\t\t\t\t else\n\t\t\t\t\t  if (SOLVE == 8)\n\t\t\t\t\t\t\t se.SolveHLSODE(step,perc1,perc2,Max_time,runs,OUTPUT,step_o,argv[1]);\n\t\t\t\t\t else \n\t\t\t\t\t\t se.SolveLSODE(step,perc1,perc2,Max_time,OUTPUT,step_o,argv[1]);\n";
    out << "\n\tse.PrintStatistic(argv[1]);\n\t}\n catch(Exception obj)\n\t{\n\tcerr<<endl<<obj.get()<<endl;\n\t}\n\n";
    out << " time(&time_4);\n\n cout<<\"\\n\\nEND EXECUTION\"<<endl;\n cout<<\"\\nResults are saved in: \"<<argv[1]<<endl;\n";
    out << " cout<<\"\\n=========================== TIME ===========================\\n\\n\\t\";\n";
    out << " cout<<\"Total time required: \"<<(time_4-time_1)<<\"s.\"<<endl;\n";
    out << " cout<<\"\\n=========================== TIME ===========================\\n\\n\";\n";
    out << " cout<<\"\\n=========================== MEM. ===========================\\n\\n\\t\";\n";
    out << " getrusage(who,&usage);\n";
    out << " cout<<\"Total memory used: \"<<usage.ru_maxrss<<\"KB\"<<endl;\n";
    out << " cout<<\"\\n=========================== TIME ===========================\\n\\n\";\n\n}";
//     return EXIT_SUCCESS;
//
//free memory
    for (int tt = 0; tt < ntr; tt++)
    {
        free(TPI[tt]);
        free(TP[tt]);
    }
    free(lbound);
    free(ubound);
    free(TP);
    free(TPI);
    cout << "------------------------------------------------" << endl;
    cout << "                 End encoding" << endl;
    cout << "------------------------------------------------\n\n" << endl;

    int who1 = RUSAGE_SELF;
    struct rusage usage1;
    getrusage(who1, &usage1);


    endGlobal = clock();
    timeGlobal = ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC;



    cout << "===================== INFO =====================" << endl;
    cout << " Total Time: " << setprecision(7) << timeGlobal << " sec" << endl;
    cout << " Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;
    cout << " Output saved in: " << net << ".cpp" << "\n";
    cout << "================================================\n" << endl;
}/* End build_ODE */




/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void build_ODEGPU(std::string net)
{
    /* Init build_ODEGPU */

    Node_p l_ptr = NULL;
    int pp;
    clock_t startGlobal, endGlobal;
    double timeGlobal;
    startGlobal = clock();


    cout << "\n\n------------------------------------------------" << endl;
    cout << "               Start  encoding" << endl;
    cout << "------------------------------------------------\n" << endl;


//transition rate
    std::string filename=net+".c_vector";
    ofstream out(filename.c_str());

    if (!out)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }

    for (int i=0; i<ntr; i++)
        out<<tabt[i].mean_t<<"\n";
    out.close();


//initial marking
    filename=net+".M_0";
    out.open(filename.c_str());

    if (!out)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }

    for (int i=0; i<npl; i++)
        out<<net_mark[i].total<<"\t";
    out<<endl;

    out.close();

//constant marking if different by 0
    filename=net+".M_feed";

    if (!out)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }

    out.open(filename.c_str());
    for (int i=0; i<npl; i++)
        out<<"0"<<"\t";
    out<<endl;
    out.close();


    int **TPO = (int **) malloc((ntr) *  sizeof(int *));
    int **TPI = (int **) malloc((ntr) *  sizeof(int *));
    for (int tt = 0; tt < ntr; tt++)
    {
        TPO[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TPO[tt], 0, npl * sizeof(int));
        TPI[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TPI[tt], 0, npl * sizeof(int));
    }

    for (int tt = 0; tt < ntr; tt++)
    {
        l_ptr = GET_INPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            //TP[tt][pp] = -l_ptr->molt;
            TPI[tt][pp] = l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_OUTPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TPO[tt][pp] = l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }

#if DEBUG
        for (int i = 0; i < npl; i++)
            cout << "\t" << TP[tt][i];
        cout << endl;
#endif
    }


//stoichiometric matrix  left side
    filename=net+".left_side";

    if (!out)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }

    out.open(filename.c_str());
    for (int tt = 0; tt < ntr; tt++)
    {
        out<<TPI[tt][0];
        for (int pp = 1; pp < npl; pp++)
        {
            out<<"\t"<<TPI[tt][pp];
        }
        out<<"\n";
    }
    out.close();


//stoichiometric matrix  right side
    filename=net+".right_side";

    if (!out)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }

    out.open(filename.c_str());
    for (int tt = 0; tt < ntr; tt++)
    {
        out<<TPO[tt][0];
        for (int pp = 1; pp < npl; pp++)
        {
            out<<"\t"<<TPO[tt][pp];
        }
        out<<"\n";
    }
    out.close();

//free memory
    for (int tt = 0; tt < ntr; tt++)
    {
        free(TPI[tt]);
        free(TPO[tt]);
    }
    free(TPO);
    free(TPI);
    cout << "------------------------------------------------" << endl;
    cout << "                 End encoding" << endl;
    cout << "------------------------------------------------\n\n" << endl;

    int who1 = RUSAGE_SELF;
    struct rusage usage1;
    getrusage(who1, &usage1);


    endGlobal = clock();
    timeGlobal = ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC;



    cout << "===================== INFO =====================" << endl;
    cout << " Total Time: " << setprecision(7) << timeGlobal << " sec" << endl;
    cout << " Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;
    cout << "================================================\n" << endl;


}


/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void build_ODER(ofstream &out, std::string net)
{
    /* Init build_ODE */

    Node_p l_ptr = NULL;
    int pp;
    clock_t startGlobal, endGlobal;
    double timeGlobal;
    startGlobal = clock();


    cout << "\n\n------------------------------------------------" << endl;
    cout << "               Start  encoding" << endl;
    cout << "------------------------------------------------\n" << endl;

    int **TP = (int **) malloc((ntr) *  sizeof(int *));
    int **TPI = (int **) malloc((ntr) *  sizeof(int *));
    for (int tt = 0; tt < ntr; tt++)
    {
        TP[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TP[tt], 0, npl * sizeof(int));
        TPI[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TPI[tt], 0, npl * sizeof(int));
    }



    out << "\n###################################################################\n";
    out <<   "#This file is automatically generated by Greatspn                 #\n";
    out <<   "#You can report bugs  by sending an e-mail to beccuti@di.unito.it #\n";
    out <<   "###################################################################\n\n";

    out << "\nlibrary(deSolve)\n\n";
    out <<"Times <- seq(from = 0, to = FinalTime, by = step)\n";

    out << "\n##Begin parameter rates\n";
    for(int i=0; i<nrp; ++i)
        out<<tabrp[i].rate_name<<" = "<<tabrp[i].rate_val<<endl;
    out << "##End parameter rates\n";

    out << "\n##Begin Transition rates\n";
    for (int tt = 0; tt < ntr; tt++)
    {
        l_ptr = GET_INPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] = -l_ptr->molt;
            TPI[tt][pp] = -l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_OUTPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] += l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        //Encoding transition rates
        out << tabt[tt].trans_name << " = ";
        if (tabt[tt].rate_par_id>=0)
            out<<tabrp[tabt[tt].rate_par_id].rate_name<<"\n";
        else
            out<<tabt[tt].mean_t<< "\n";
#if DEBUG
        for (int i = 0; i < npl; i++)
            cout << "\t" << TP[tt][i];
        cout << endl;
#endif
    }

    out << "##End Transition rates\n\n";


    out << "funODE <- function(t,y, parms){\n\n";

    out << "##Places array\n";

    out << "##Begin Place mapping\n";
//Initialization variables
    for (pp = 0; pp < npl; pp++)
    {
        out << tabp[pp].place_name << " = y[" << pp + 1 << "]\n";

    }
    out << "##End Place mapping\n\n";
//Initialization variables

    out << "##Begin ODE system\n";
    for (pp = 0; pp < npl; pp++)
    {
        out << "d" << tabp[pp].place_name << " = ";
        bool found = false;
        for (int tt = 0; tt < ntr; tt++)
        {
            if (TP[tt][pp] != 0)
            {
                found = true;
                if (tabt[tt].general_function==NULL)
                {

                    if (MASSACTION)
                    {
                        out << " + " << tabt[tt].trans_name << " * " << TP[tt][pp];
                        for (int pp1 = 0; pp1 < npl; pp1++)
                        {
                            if (TPI[tt][pp1] < 0)
                            {
                                out << " * " << tabp[pp1].place_name << "^" << abs(TPI[tt][pp1]);
                            }
                        }
                    }
                    else
                    {
                        out << "+ " << tabt[tt].trans_name << " * " << TP[tt][pp] << "*min( ";
                        bool first = true;
                        for (int pp1 = 0; pp1 < npl; pp1++)
                        {
                            if (TPI[tt][pp1] < 0)
                            {
                                if (first)
                                {
                                    out << " " << tabp[pp1].place_name << "/" << abs(TPI[tt][pp1]);
                                    first = false;

                                }
                                else
                                    out << ", " << tabp[pp1].place_name << "/" << abs(TPI[tt][pp1]);

                            }

                        }
                        out << " )";
                    }
                }
                else
                {
                    std::string general_function(tabt[tt].general_function);
                    if (TP[tt][pp]>0)
                    {
                        out<<"+1 * "<<general_function.substr(3,general_function.size()-4);
                    }
                    else
                    {
                        out<<TP[tt][pp]<<" * "<<general_function.substr(3,general_function.size()-4);
                    }
                }

            }
        }
        if (!found)//case test loop
            out << "0";
        out << "\n";
    }
    out << "list(c(d" << tabp[0].place_name;
    for (pp = 1; pp < npl; pp++)
    {
        out << ", d" << tabp[pp].place_name;
    }

    out << "))\n";
    out << "}\n##End ODE System\n";

    //Generazione Marcatori dei Posti

    out << "\n##Setting Markers on Places:\n";
    out << "yini <- c(";
    for (pp = 0; pp < npl; pp++)
    {
        out << "y"<<pp+1 << " = " << net_mark[pp].total;
        if(pp+1!=npl)
        {
            out << ", ";
        }
    }
    out << ")\n";
    out<<"t1=Sys.time()";
    out << "\nres1 <-lsode(yini,Times,funODE,parms=0,hini=hini)";
    out <<"\ncolnames(res1)= c(\"Time\"";
    for (pp = 0; pp < npl; pp++)
    {
        out <<",\""<<tabp[pp].place_name<<"\"";
    }
    out << ")\n";
    out << "\n\n##REMEMBER TO INITIALIZE Times ARRAY.";
    out<<"\ncat(\"\\n\\nExecution time ODE:\",difftime(Sys.time(), t1, unit = \"secs\"), \"sec.\\n\")";
    out << "\n\nwrite.table(file=\"ODE_01.txt\",res1)\n\n##PLEASE REMEMBER TO DEFINE Times ARRAY";

    cout << "\tDone.\n" << endl;

//free memory
    for (int tt = 0; tt < ntr; tt++)
    {
        free(TPI[tt]);
        free(TP[tt]);
    }
    free(TP);
    free(TPI);
    cout << "------------------------------------------------" << endl;
    cout << "                 End encoding" << endl;
    cout << "------------------------------------------------\n\n" << endl;

    int who1 = RUSAGE_SELF;
    struct rusage usage1;
    getrusage(who1, &usage1);


    endGlobal = clock();
    timeGlobal = ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC;



    cout << "===================== INFO =====================" << endl;
    cout << " Total Time: " << setprecision(7) << timeGlobal << " sec" << endl;
    cout << " Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;
    cout << " Output saved in: " << net << "\n";
    cout << "================================================\n" << endl;


}/* End build_ODER */

struct transition_def
{
    //std::string name;
    double tr_min;
    double tr_max;
    double locat;
};


void build_ODEOPT(ofstream &out, std::string net, std::string trans_path, std::string obj_funct_path)
{

    /* Init build_ODE with OPT */

    Node_p l_ptr = NULL;
    int pp;
    clock_t startGlobal, endGlobal;
    double timeGlobal;
    startGlobal = clock();


    cout << "\n\n------------------------------------------------" << endl;
    cout << "               Start  encoding" << endl;
    cout << "------------------------------------------------\n" << endl << endl;
    cout << "Transition path: "<< trans_path << endl;
    cout << "Object Function path: "<< obj_funct_path << endl <<endl;

    int **TP = (int **) malloc((ntr) *  sizeof(int *));
    int **TPI = (int **) malloc((ntr) *  sizeof(int *));
    for (int tt = 0; tt < ntr; tt++)
    {
        TP[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TP[tt], 0, npl * sizeof(int));
        TPI[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TPI[tt], 0, npl * sizeof(int));
    }

    /* ---------------------------------------------------
                OPTIMIZATION TRANS. I/O & CHECK
    ------------------------------------------------------ */
    std::map<std::string,transition_def> transit;
    std::string line;
    ifstream myfile(std::string(trans_path).c_str());
    Parser parser;
    char delimC[] = "\t,; =[]{}\"";

    if (!myfile.is_open())
    {
        cerr << "Error: it is not possible to open transitions file \n\n";
        exit(EXIT_FAILURE);
    }
    else
    {
        while (!(myfile.eof()))
        {
            getline (myfile,line);
            if (line!="")
            {
                transition_def k;
                k.locat = -1;
                parser.update(delimC,line);
                std::string tr_name = parser.get(0);
                k.tr_min = atof(parser.get(1).c_str());
                k.tr_max = atof(parser.get(2).c_str());
                k.locat = atof(parser.get(3).c_str());
                //transit.push_back(k);
                transit.insert(std::make_pair(tr_name,k));
            }
        }
    }

    myfile.close();

    //generating hashmap and hashset of places and transitions names
    cout << "Places are:" << endl;
    for (pp = 0; pp < npl; pp++)
    {
        Place2Int[std::string(tabp[pp].place_name)] = pp;
        cout << tabp[pp].place_name << endl;
    }

    std::set<std::string> TransNameSet;
    std::string Trans_NA_names = "";

    cout << "\nTransitions are:" << endl;

    for(int j=0; j<ntr; j++)
    {
        TransNameSet.insert(tabt[j].trans_name);
        cout << tabt[j].trans_name << endl;
    }

    cout << "\nTransitions bounds are:\n" << endl;

    //constraint check loop
    std::map<std::string, transition_def>::iterator it;
    for( it = transit.begin(); it != transit.end(); it++)
    {
        cout << it->first<<" ";
        cout << it->second.tr_min<<" ";
        cout << it->second.tr_max<<" ";
        cout << it->second.locat;
        cout << "\n";
        //check location point constraint (between min and max values)
        if((it->second.tr_min>it->second.locat||it->second.tr_max<it->second.locat)&&it->second.locat!=0)
        {
            cerr << "Error: some value of location is out of bound between min and max research values. \n\n";
            exit(EXIT_FAILURE);
        }
        //check constraint of transition names
        if(TransNameSet.find(it->first)==TransNameSet.end())
        {
            cerr << "Error: Incorrect match between NET and transition names. \n\n";
            exit(EXIT_FAILURE);
        }
    }
    cout << "\n";
    out << "\n###################################################################\n";
    out <<   "#This file is automatically generated by Greatspn                 #\n";
    out <<   "#You can report bugs  by sending an e-mail to beccuti@di.unito.it #\n";
    out <<   "###################################################################\n\n";

#if DEBUGOPT
    out << "library(parallel)"<< endl;
    out << "library(alabama)" <<endl;
#endif

    out << "library(deSolve)" << endl;
    out << "library(GenSA)"<< endl;
    // out << "#If you want to use a different opt solver library uncomment this:"<< endl;
    //  out << "#library()" << endl;


    out << "\n##Begin Transition Rates\n";
    //out << "e=rep(NA,"<<ntr<<")\n\n";
    for (int tt = 0; tt < ntr; tt++)
    {
        l_ptr = GET_INPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] = -l_ptr->molt;
            TPI[tt][pp] = -l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_OUTPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] += l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        //Encoding transition rates

        bool flag = false;

        it = transit.find(tabt[tt].trans_name);
        if (it != transit.end())
        {
            flag= true;
        }

        //if reaction is part of OPT problem, the value becomes NA.
        if(!flag)
        {
            //out << "e["<< tt+1<< "] = " << tabt[tt].mean_t << ";\n";
            out << tabt[tt].trans_name << " = " << tabt[tt].mean_t << "\n";
        }
        else
        {
            //out << "e["<< tt+1<< "] = NA;\n";
            Trans_NA_names = Trans_NA_names + tabt[tt].trans_name + ",";
            out << tabt[tt].trans_name << " = NA #" << tabt[tt].mean_t << "\n";
        }
#if DEBUG
        for (int i = 0; i < npl; i++)
            cout << "\t" << TP[tt][i];
        cout << endl;
#endif
    }
    out << "##End Transition Rates";


    out << "\n\n##Setting number of places:\nyini = rep(0," << npl<<")" <<endl;
    out << "\n##Begin Place Mapping\n";
    for (pp = 0; pp < npl; pp++)
    {
        out << tabp[pp].place_name << " = yini[" << pp + 1 << "] = "<< net_mark[pp].total << "\n";
    }
    out << "##End Place Mapping\n\n";

    //out << "\n##Initial Marking:\n";
    /*out << "Ptini <- c(";
    for (pp = 0; pp < npl; pp++)
    {
        out << "y"<<pp+1 << " = " << net_mark[pp].total;
        if(pp+1!=npl)
        {
            out << ", ";
        }
    }
    out << ")\n";*/

    out <<"##Token Sum:\n";
    out <<"tot_token=sum(yini)\n";

    out <<"##Object value:\n";
    out<<"objvalue=.Machine$double.xmax\n";

    out <<"##Object res:\n";
    out<<"objres=rep(0,"<<npl+1<<")\n";



    out <<"\n\n##ODE Result:\n";
    out <<"res<-matrix(c(0,yini),1)";


    out <<"\n\n##LOWERBOUNDS ARRAY:\n";
    out <<"LB=c(";
    it = transit.begin();
    while(it != transit.end())
    {

        out << it->second.tr_min;

        ++it;

        if(it!=transit.end())
        {
            out << ", ";
        }
    }
    out<<")\n";

    out <<"\n##UPPERBOUNDS ARRAY:\n";
    out <<"UB=c(";
    it = transit.begin();
    while(it != transit.end())
    {

        out << it->second.tr_max;

        ++it;

        if(it!=transit.end())
        {
            out << ", ";
        }
    }

    out<<")\n";

    out <<"\n##STARTING-POINTS ARRAY:\n";
    out <<"k=c(";
    it = transit.begin();
    while(it != transit.end())
    {

        if(it->second.locat<=0)
        {
            out<<(it->second.tr_min+it->second.tr_max)/2;
        }
        else
        {
            out<<it->second.locat;
        }

        ++it;

        if(it!=transit.end())
        {
            out << ", ";
        }
    }
    out<<")\n";

    out <<"\n##ODE Time Range and Step (default values ON):\n";
    out <<"Times <- seq(from = 0, to = FinalTime, by = step)\n\n";
    out << "kv=matrix(k,ncol=length(k));\n\n";

    //ODE1 WITHOUT OPT
    out << "##Function used to calculate the objective function\n";
    out << "funODE <- function(t,y,parms) {\n\n";

    it = transit.begin();
    int count = 1;
    while(it != transit.end())
    {
        out << it->first << " <- parms["<<count<<"]\n";
        ++count;
        ++it;
    }

    out << "\n##Begin ODE1 System\n";

    for (pp = 0; pp < npl; pp++)
    {
        out << tabp[pp].place_name << " = y[" << pp + 1 << "]\n";
    }
    out << "\n";
    out<<"ddy=rep(0,"<<npl<<")\n";
    for (pp = 0; pp < npl; pp++)
    {

        out << "ddy["<<pp+1<<"] = d" << tabp[pp].place_name << " = ";
        bool found = false;
        for (int tt = 0; tt < ntr; tt++)
        {
            if (TP[tt][pp] != 0)
            {
                found = true;
                //NEW for generic function
                if (tabt[tt].general_function==NULL)
                {
                    if (MASSACTION)
                    {
                        out << " + " << tabt[tt].trans_name << " * " << TP[tt][pp];
                        for (int pp1 = 0; pp1 < npl; pp1++)
                        {
                            if (TPI[tt][pp1] < 0)
                            {
                                out << " * " << tabp[pp1].place_name << "^" << abs(TPI[tt][pp1]);
                            }

                        }
                    }
                    else
                    {
                        out << "+ " << tabt[tt].trans_name << " * " << TP[tt][pp] << "*min( ";
                        bool first = true;
                        for (int pp1 = 0; pp1 < npl; pp1++)
                        {
                            if (TPI[tt][pp1] < 0)
                            {
                                if (first)
                                {
                                    out << " " << tabp[pp1].place_name << "/" << abs(TPI[tt][pp1]);
                                    first = false;
                                }
                                else
                                {
                                    out << ", " << tabp[pp1].place_name << "/" << abs(TPI[tt][pp1]);
                                }
                            }
                        }
                        out << " )";
                    }

                }
                else
                {
                    std::string general_function(tabt[tt].general_function);
                    if (TP[tt][pp]>0)
                    {
                        out<<"+1*"<<general_function.substr(3,general_function.size()-4);
                    }
                    else
                    {
                        out<<TP[tt][pp]<<"* "<<general_function.substr(3,general_function.size()-4);
                    }
                }
            }
        }
        if (!found) //case test loop
        {
            out << "0";
        }
        out << "\n";
    }
    out << "\n#cat(\"ddy  :\",ddy,\"\\n\") \nreturn(list(ddy))\n}\n";
    out << "##END ODE1 SYSTEM\n";

    std::stringstream buf;
    std::ifstream file(obj_funct_path.c_str());
    if(!file)
    {
        cerr << "\n*****Error opening input file "<<obj_funct_path<<" *****\n" << endl;
        exit(EXIT_FAILURE);
    }
    buf<<file.rdbuf();

    std::string obj_string = parseObjectiveFunction(obj_funct_path,"y");
    std::string objlast_string = parseObjectiveFunction(obj_funct_path,"last");
    cout << "Objective function with place names is:\n";
    cout << buf.str() << endl;


    cout << "\nComputed objective function is:" << endl << obj_string <<endl;

    out << "\n##OPT FUNCT\n";
    out << "fn <-function(x,y,hini,Time) { \n";
    out << "Times <- seq(from = TimeOLD, to = Time, by = step)\n";

    out << "res <- lsode(y,Times,funODE,parms=x,hini=hini)\n";
    out << "last=tail(res,1)\n";

    out << "fn="<<objlast_string<<"\n";
    out << "if (fn<objvalue){\n";
    out <<      "assign('objvalue', fn, envir = .GlobalEnv)\n";
    out <<      "assign('objres', last, envir = .GlobalEnv) \n}";
    out << "\nreturn(fn)\n}";
    out << "\n##END OPT FUNCT\n\n";
#if DEBUGOPT
    out << "\n##Begin Inequality Constraints\n";
    out << "hin <- function(x) {\n";
    out << "hin <- rep(0, "<< (transit.size())*2<<")\n";
    int j = 1;
    int i = 0;
    for(it = transit.begin(); it != transit.end(); it++)
    {
        out << "## "<<it->second.tr_min << " < " <<  it->first << " < " << it->second.tr_max << "\n";
        out << "hin["<<j++<<"]<- x["<<i+1<<"]-"<< it->second.tr_min<<"\n";
        out << "hin["<<j++<<"]<- -x["<<i+1<<"]+"<<it->second.tr_max<<"\n";
        i++;
    }
    out << "return (hin)\n}\n";
    out << "##End Inequality Constraints\n\n";

    out << "\n##Begin Gradients Def\n";
    out << "gr <- function(x,dx,F,y) {\n";
    out << "grad(func=fn, x=x, dx=dx, F = F, parms=0)\n}\n";
    out << "##End Gradients Def\n";
#endif
    out << "##Begin External loop\n";
    out << "compute = function(Time,hini){\n\n";
    out << "q=GenSA(par=k,fn=fn,upper=UB,lower=LB,control=list(max.time=2,verbose=F,trace.mat=F),y=y,hini=hini,Time=Time)" << endl << endl;
    int k = 0;

    for(it = transit.begin(); it != transit.end(); it++)
    {
        out << it->first << " <- q$par["<<k+1<<"]\n";
        k++;
    }

    std::ostringstream inhibited_rates;
    it = transit.begin();

    while(it != transit.end())
    {
        inhibited_rates << (it->first);
        ++it;
        if(it!=transit.end())
        {
            inhibited_rates << ", ";
        }
    }

    out << "assign('objvalue',.Machine$double.xmax, envir = .GlobalEnv)\n";
    out << "assign('y', objres[2:"<<npl+1<<"], envir = .GlobalEnv)\n";
    out << "assign('TimeOLD',Time, envir = .GlobalEnv)\n";
    out << "assign('k', c("<<inhibited_rates.str()<<"), envir = .GlobalEnv)\n";



    //removing last comma
    out <<"assign('kv', rbind(kv,c("<< inhibited_rates.str() <<")), envir = .GlobalEnv)\n\n";

    out<<"cat(\"Y:\",objres,\"\\n\")\n";
    out<<"cat(\"k:\",k,\"\\n\")";

    out << "\nreturn(objres)\n}\n##END ODE SYSTEM\n";


    out<<"t1=Sys.time()\n";
    out<<"Times <- seq(from = 0, to = FinalTime, by = step)\n";
    out<<"TimesLength=length(Times)\n";
    out<<"y=yini\n";
    out<<"TimeOLD=Times[1]\n";
    out<<"res=sapply(Times[2:TimesLength],compute,hini)\n";
    out<<"res=t(res)\n";
    out<<"res=rbind(c(0,yini),res)\n";

    out <<"\ncolnames(res)= c(\"Time\"";
    for (pp = 0; pp < npl; pp++)
    {
        out <<",\""<<tabp[pp].place_name<<"\"";
    }
    out << ")\n";

    out<<"\ncat(\"\\n\\nExecution time ODE+OPT solver:\",difftime(Sys.time(), t1, unit = \"secs\"), \"sec.\\n\")";

    out << "\n\nwrite.table(file=\"Opt_01.txt\",res)\n\n##PLEASE REMEMBER TO DEFINE Times ARRAY";


//free memory
    for (int tt = 0; tt < ntr; tt++)
    {
        free(TPI[tt]);
        free(TP[tt]);
    }
    free(TP);
    free(TPI);
    cout << "\n\n";
    cout << "------------------------------------------------" << endl;
    cout << "                 End encoding" << endl;
    cout << "------------------------------------------------\n\n" << endl;

    int who1 = RUSAGE_SELF;
    struct rusage usage1;
    getrusage(who1, &usage1);


    endGlobal = clock();
    timeGlobal = ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC;



    cout << "===================== INFO =====================" << endl;
    cout << " Total Time: " << setprecision(7) << timeGlobal << " sec" << endl;
    cout << " Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;
    cout << " Output saved in: " << net << "\n";
    cout << "================================================\n" << endl;

}

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void build_ODEM(ofstream &out, std::string net)
{
    /* Init build_ODE */

    Node_p l_ptr = NULL;
    int pp;
    clock_t startGlobal, endGlobal;
    double timeGlobal;
    startGlobal = clock();


    cout << "\n\n------------------------------------------------" << endl;
    cout << "               Start  encoding" << endl;
    cout << "------------------------------------------------\n" << endl;

    int **TP = (int **) malloc((ntr) *  sizeof(int *));
    int **TPI = (int **) malloc((ntr) *  sizeof(int *));
    for (int tt = 0; tt < ntr; tt++)
    {
        TP[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TP[tt], 0, npl * sizeof(int));
        TPI[tt] = (int *) malloc((npl) * sizeof(int));
        memset(TPI[tt], 0, npl * sizeof(int));
    }



    out << "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
    out << "%This file is automatically generated by Greatspn\n";
    out << "%You can report bugs  by sending an e-mail to beccuti@di.unito.it\n";
    out << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n";

    out << "function dydt =" << net << "(t,y)\n\n";

    out << "%Transition rates\n";
    for (int tt = 0; tt < ntr; tt++)
    {
        l_ptr = GET_INPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] = -l_ptr->molt;
            TPI[tt][pp] = -l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        l_ptr = GET_OUTPUT_LIST(tt);
        while (l_ptr != NULL)
        {
            pp = GET_PLACE_INDEX(l_ptr);
            TP[tt][pp] += l_ptr->molt;
            l_ptr = NEXT_NODE(l_ptr);
        }
        //Encoding transition rates
        out << tabt[tt].trans_name << " = " << tabt[tt].mean_t << ";\n";

#if DEBUG
        for (int i = 0; i < npl; i++)
            cout << "\t" << TP[tt][i];
        cout << endl;
#endif
    }

    out << "%Transition rates\n\n";


    out << "%Place mapping\n";
//Initialization variables
    for (pp = 0; pp < npl; pp++)
    {
        out << tabp[pp].place_name << " = y(" << pp + 1 << ");\n";

    }
    out << "%Place mapping\n\n";
//Initialization variables

    out << "%ODE system\n";
    for (pp = 0; pp < npl; pp++)
    {
        out << "d" << tabp[pp].place_name << " = ";
        bool found = false;
        for (int tt = 0; tt < ntr; tt++)
        {
            if (TP[tt][pp] != 0)
            {
                found = true;
                if (MASSACTION)
                {
                    out << " + " << tabt[tt].trans_name << " * " << TP[tt][pp];
                    for (int pp1 = 0; pp1 < npl; pp1++)
                    {
                        if (TPI[tt][pp1] < 0)
                        {
                            out << " * " << tabp[pp1].place_name << "/" << abs(TPI[tt][pp1]);
                        }
                    }
                }
                else
                {
                    out << "+ " << tabt[tt].trans_name << " * " << TP[tt][pp] << "*min([ ";
                    bool first = true;
                    for (int pp1 = 0; pp1 < npl; pp1++)
                    {
                        if (TPI[tt][pp1] < 0)
                        {
                            if (first)
                            {
                                out << " " << tabp[pp1].place_name << "/" << abs(TPI[tt][pp1]);
                                first = false;

                            }
                            else
                                out << ", " << tabp[pp1].place_name << "/" << abs(TPI[tt][pp1]);

                        }

                    }
                    out << " ])";
                }

            }
        }
        if (!found)//case test loop
            out << "0";
        out << ";\n";
    }
    out << "dydt = [ d" << tabp[0].place_name;
    for (pp = 1; pp < npl; pp++)
    {
        out << "; d" << tabp[pp].place_name;
    }
    out << "];\n";
    out << "%ODE system\n\nend\n";


    cout << "\tDone.\n" << endl;

//free memory
    for (int tt = 0; tt < ntr; tt++)
    {
        free(TPI[tt]);
        free(TP[tt]);
    }
    free(TP);
    free(TPI);
    cout << "------------------------------------------------" << endl;
    cout << "                 End encoding" << endl;
    cout << "------------------------------------------------\n\n" << endl;

    int who1 = RUSAGE_SELF;
    struct rusage usage1;
    getrusage(who1, &usage1);


    endGlobal = clock();
    timeGlobal = ((double)(endGlobal - startGlobal)) / CLOCKS_PER_SEC;



    cout << "===================== INFO =====================" << endl;
    cout << " Total Time: " << setprecision(7) << timeGlobal << " sec" << endl;
    cout << " Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;
    cout << " Output saved in: " << net << ".m\n";
    cout << "================================================\n" << endl;


}/* End build_ODE */




