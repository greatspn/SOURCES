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

    Result_p enabled_head = NULL;   /* Puntatori alla lista delle tr. abil. */

    int tro;            /* 1 marc. gia' raggiunta, 0 altrimenti */
    int marcatura = 0;  /* contatore per le marcature       */
    int h = 0;          /* per bilanciamento nella insert_tree  */
    unsigned long tang = 0;     /* contatori tipi di marc. raggiunte    */
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
extern bool FLUXB;
extern std::vector<std::string> flux_names;
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
        cerr << "Warning: it is not possible to open input file .pin\n\n";
        //exit(EXIT_FAILURE);
        ErrorLU=true;
        return 0;
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
            B[row] += net_mark[col - 1].total*val;
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
        cerr << "Warning: it is not possible to open input file .bnd\n\n";
        ErrorLU=true;
        return;
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
void build_ODECompact(ofstream &out, std::string path, std::string net)
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

}
hout << "namespace SDE {\n";
for (int i = 0; i < npl; i++)
{
    hout << "#define " << tabp[i].place_name << "_place " << i<<endl;
}
set<std::string> function_names;
for (int tt = 0; tt < ntr; tt++)
{
    if (tabt[tt].general_function!=NULL  || (tabt[tt].timing == TIMING_DETERMINISTIC && tabt[tt].general_function == NULL /*dirac*/)){
       if (!FLUXB)
        hout<<"double "<<tabt[tt].trans_name<<"_general(double *Value, map <std::string,int>& NumTrans,  map <std::string,int>& NumPlaces,const vector <string>& NameTrans, const struct InfTr* Trans, const int Tran, const double& Time);\n";
    else
        hout<<"double "<<tabt[tt].trans_name<<"_general(double *Value,vector<class FBGLPK::LPprob>& vec_fluxb,  map <std::string,int>& NumTrans,  map <std::string,int>& NumPlaces,const vector <string>& NameTrans, const struct InfTr* Trans, const int Tran, const double& Time);\n";
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
out << " int SOLVE = 7, runs=1;\n";
out << " long int seed = 0;\n";
out << " bool OUTPUT=false;\n";
   //variability 
if (FLUXB){
    out << " bool VARIABILITY=false;\n";
}
    //variability
out << " std::string fbound=\"\", finit=\"\", fparm=\"\";\n";
out << " double hini = 1e-6, atolODE = 1e-6, rtolODE=1e-6, ftime=1.0, stime=0.0, itime=0.0, epsTAU=0.1;\n\n";
out << " cout<<\"\\n\\n =========================================================\\n\";\n";
out << " cout<<\"|	              ODE/SDE Solver                       |\\n\";\n";
out << " cout<<\" =========================================================\\n\";\n";
out << " cout<<\"\\n If you find any bug, send an email to beccuti@di.unito.it\\n\";\n\n";

    //automaton
if (AUTOMATON)
{
    out << " if (argc<3)\n\t{\n \t";
    out << " std::cerr<<\"\\n\\nUSE:" << net << "_solve <out_file> <automaton_file> [OPTION ]<type> <hini>  <atol> <rtol> <runs> <ftime> <stime>   -B <bound_file> \";\n\t";

}
else
{
    out << " if (argc<2)\n\t{\n \t";
    out << " std::cerr<<\"\\n\\nUSE:" << net << "_solve <out_file> [OPTIONS]\";\n\t";
}
    //automaton
out<<"std::cerr<<\"\\n\\n\\tOPTIONS\\n\";\n\t";
out << " std::cerr<<\"\\n\\t -type <type>:\\t\\t ODE-E or ODE-RKF or ODE45 or LSODA or HLSODA or (H)SDE or HODE or SSA or TAUG or STEP. Default: LSODA \";\n\t";
out << " std::cerr<<\"\\n\\t -hini <double>:\\t Initial step size. Default: 1e-6\";\n\t";
out << " std::cerr<<\"\\n\\t -atol <double>:\\t Absolute error tolerance. Dafault: 1e-6\";\n\t";
out << " std::cerr<<\"\\n\\t -rtol <double>:\\t Relative error tolerance. Dafault: 1e-6\";\n\t";
out << " std::cerr<<\"\\n\\t -taueps <double>:\\t Epsilon value for Tau-leaping algorithm. Dafault: 0.1\";\n\t";
out << " std::cerr<<\"\\n\\t -runs <int>:\\t\\t Integer number corresponding to runs (only used in SSA,TAUG, HODE,HLSODA). Default: 1\";\n\t";
out << " std::cerr<<\"\\n\\t -ftime <double>:\\t Double number used to set the upper bound of the evolution time. Dafault: 1\";\n\t";
out << " std::cerr<<\"\\n\\t -stime <double>:\\t Double number used to set the step in the output. Default: 0.0 (no output)\";\n\t";
out << " std::cerr<<\"\\n\\t -itime <double>:\\t Double number used to set the initial simulation time. Default: 0.0 \";\n\t";
out << " std::cerr<<\"\\n\\t -b <bound_file>:\\t Soft bound are defined in the file <bound_file>\";\n\t";
out << " std::cerr<<\"\\n\\t -seed <double>:\\t Seed of random number generator\";\n\t";
    //variability 
if (FLUXB){
    out << " std::cerr<<\"\\n\\t -var:\\t Enable output for variability analysis of fluxes\";\n\t";
}
      //variability
out << " std::cerr<<\"\\n\\t -init <init_file>:\\t The file <initial_file> contains the initial marking. Default:  initial marking in the orginal net\";\n\t";
out << " std::cerr<<\"\\n\\t -parm <parm_file>:\\t The file <parm_file> contains a set of pairs with format <transition name> <value> or <place name> <value>.\\n\\t\\t\\t\\t For transition  the value is used to set a new rate value, while for place  it is used to set a new initial marking.\";\n\t";

    //automaton
if (AUTOMATON)
    out << " std::cerr<<\"\\n\\t <automaton_file>:\\t automaton is defined in the file <automaton>\\n\";";
out <<" std::cerr<<endl<<endl;";
    //automaton
out << "\n\t exit(EXIT_FAILURE);\n\t}\n\n";


if (AUTOMATON)
    out<<" int ii=3;\n";
else
    out<<" int ii=2;\n";
out<<" for (; ii<argc; ii++){\n";
out<<"\t if (strcmp(\"-type\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out << "\t\t\t if ((strcmp(argv[ii],\"ODE-E\")==0)||(strcmp(argv[ii],\"ode-e\")==0)) SOLVE = 1;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"ODE-RKF\")==0)||(strcmp(argv[ii],\"ode-rkf\")==0)) SOLVE = 5;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"ODE45\")==0)||(strcmp(argv[ii],\"ode45\")==0)) SOLVE = 6;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"LSODA\")==0)||(strcmp(argv[ii],\"lsoda\")==0)) SOLVE = 7;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"STEP\")==0)||(strcmp(argv[ii],\"step\")==0)) SOLVE = 4;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"SSA\")==0)||(strcmp(argv[ii],\"ssa\")==0)){\n";
                //out << "\t\t\t\t cout<<\"\\t using simulation\"<<endl;\n\t\t\t\t epsilon=10000000000;\n\t\t\t\t hini=MAXSTEP; \n\t\t\t\t SOLVE=3;\n\t\t\t }\n";
out << "\n\t\t\t\t SOLVE=3;\n\t\t\t }\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"HODE\")==0)||(strcmp(argv[ii],\"hode\")==0)) SOLVE = 2;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"HLSODA\")==0)||(strcmp(argv[ii],\"hlsoda\")==0)) SOLVE = 8;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"SDE\")==0)||(strcmp(argv[ii],\"sde\")==0) || (strcmp(argv[ii],\"HSDE\")==0)||(strcmp(argv[ii],\"hsde\")==0) ) SOLVE = 0;\n";
out << "\t\t\t else if ((strcmp(argv[ii],\"TAUG\")==0)||(strcmp(argv[ii],\"taug\")==0)) SOLVE = 9;\n";
out<<"\t\t\t else{\n";
out<< "\t\t\t\t std::cerr<<\"\\n\\tError:  -type  <value>\\n\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";
 //hini code
out<<"\t if (strcmp(\"-hini\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t hini=atof(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -hini  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";
 //atol code
out<<"\t if (strcmp(\"-atol\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t atolODE=atof(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -atol  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";
 //rtol code
out<<"\t if (strcmp(\"-rtol\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t rtolODE=atof(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -rtol  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";
 //runs code
out<<"\t if (strcmp(\"-runs\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t runs=atoi(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -runs  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";
 //ftime code
out<<"\t if (strcmp(\"-ftime\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t ftime=atof(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -ftime  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";

//stime code
out<<"\t if (strcmp(\"-stime\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t OUTPUT=true;\n";
out<<"\t\t\t stime=atof(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -stime  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";

//itime code
out<<"\t if (strcmp(\"-itime\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t itime=atof(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -itime <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";

//bound file code
out<<"\t if (strcmp(\"-b\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t fbound=string(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -b  <file_name>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";
//epsilon TAUleaping
out<<"\t if (strcmp(\"-taueps\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t epsTAU=atof(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -taueps  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";

//seed of random number generator
out<<"\t if (strcmp(\"-seed\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t seed=atol(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -seed  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";

//variability
//variability analysis
if (FLUXB){
    out<<"\t if (strcmp(\"-var\", argv[ii])==0){\n";
    out<<"\t\t VARIABILITY=true;\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";
}
//variability

//initial file code
out<<"\t if (strcmp(\"-init\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t finit=string(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -init  <file_name>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";

out<<"\t if (strcmp(\"-parm\", argv[ii])==0){\n";
out<<"\t\t if (++ii<argc){\n";
out<<"\t\t\t fparm=string(argv[ii]);\n\t\t }\n";
out<<"\t\t else{\n";
out<< "\t\t\t std::cerr<<\"\\nError:  -parm  <file_name>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
out<<"\t\t continue;\n";
out<<"\t }\n";

out<< "\t\t\t std::cerr<<\"\\nError:  unknown parameter \"<<argv[ii]<<\"\\n\\n\";\n\t\t\t exit(EXIT_FAILURE);\n";
out<<" }\n\n\n";


out<<" if (stime==0.0)  stime=ftime;\n\n";

out << " time(&time_1);\n\n";

out << " cout<<\"\\n=====================INPUT PARAMETERS======================\\n\";\n";
out << " cout<<\"\\n\\tCompact CPP code: On\\n\";\n";
out << " cout<<\"\\tType solution: \"<<SOLVE<<\"\\n\";\n";
if (MASSACTION)
    out << " cout<<\"\\tTransition policy: Genelarized Mass Action policy\\n\";\n";
else
    out << " cout<<\"\\tTransition policy: Minimum\\n\";\n";
out << " cout<<\"\\tSolution final time: \"<<ftime<<\"\\n\";\n";
out << " cout<<\"\\tInitial size step: \"<<hini<<\"\\n\";\n";
out << " cout<<\"\\tInitial  time: \"<<itime<<\"\\n\";\n";
out << " cout<<\"\\tAbosolute tolerance: \"<<atolODE<<\"\\n\";\n";
out << " cout<<\"\\tRelative tolerance: \"<<rtolODE<<\"\\n\";\n";
    //out << " if ((strcmp(argv[2],\"ODE\")!=0)&&(strcmp(argv[2],\"ode\")!=0)){\n";
out << " cout<<\"\\tEpsilon value for TAU-leaping: \"<<epsTAU<<\"\\n\";\n";
out << " cout<<\"\\tSolution runs: \"<<runs<<\"\\n\";\n";
out << " if (fbound!=\"\") cout<<\"\\tBound file: \"<<fbound<<\"\\n\";\n";
out << " if (finit!=\"\") cout<<\"\\tInitial marking file: \"<<finit<<\"\\n\";\n";
out << " if (fparm!=\"\") cout<<\"\\tInitial parameter file: \"<<fparm<<\"\\n\";\n";
    //Variability
if (FLUXB){
    out << " if (VARIABILITY) cout<<\"\\tEnable variability analysis.\\n\";\n";
}
    //Variability
    //automaton
if (AUTOMATON)
    out << " cout<<\"\\tAutomaton input: \"<<argv[2]<<\"\\n\";\n";
    //automaton
out << " cout<<\"\\tDetailed output: \"<<stime<<\"\\n\";\n";
out << " cout<<\"\\n===========================================================\\n\";";
out << " cout<<\"\\n\\nSTART EXECUTION...\"<<endl;\n\n";

cout << "\n\nSTART EXECUTION..." << endl;
out << " struct InfPlace pt;\n";
out << " struct InfTr t;\n";
out << " Equation eq;\n Elem el;\n";
if (MASSACTION)
    out << " SystEqMas se(" << npl << "," << ntr << ",places,transitions,itime,seed);\n";
else
    out << " SystEqMin se(" << npl << "," << ntr << ",places,transitions,itime,seed);\n";
out << " vector< struct InfPlace> Vpl;\n\n";

    //Opening transition file
out<< "\n string fileT=(string(argv[0])+\".transODE\");\n";
out<< " ifstream finT(fileT.c_str(), std::ifstream::in);\n";
out<< " if (!finT){\n";
out<< "\t cerr<<\"Error:  it is not possible to open output file for transition description \"<<fileT<< \"\\n\";\n";
out<< "\t return false;\n\t}\n";


std::string fileT=(path+".solver.transODE");
ofstream foutT(fileT.c_str(), std::ofstream::out);
if (!foutT)
{
    cerr<<"\nError:  it is not possible to open output file for transition description "<<fileT<< "\n";
    exit(EXIT_FAILURE);
}


map <std::string,bool> mapOfFunctions;
if (!FLUXB)
    out<<" map <std::string,double (*)(double *Value, map <string,int>& NumTrans, map <string,int>& NumPlaces, const vector <string>& NameTrans, const struct InfTr* Trans,  const int Tran, const double& Time)> mapOfFunctions;\n";
else
    out<<" map <std::string,double (*)(double *Value,  vector<class FBGLPK::LPprob>& vec_fluxb, map <string,int>& NumTrans, map <string,int>& NumPlaces, const vector <string>& NameTrans, const struct InfTr* Trans,  const int Tran, const double& Time)> mapOfFunctions;\n";

out<<" mapOfFunctions[std::string(\"nullptr\")]= nullptr;\n";

for (int tt = 0; tt < ntr; tt++)
{
    std::string GenFun="nullptr";
    std::string FuncT="nullptr";
    std::string rate=std::to_string(tabt[tt].mean_t);
    std::string timing= std::to_string(tabt[tt].timing);

    if (tabt[tt].general_function!=NULL)
    {
        //cout<<tabt[tt].general_function<<endl;
        std::string tmp_st(tabt[tt].general_function);
        char* stoken=strtok((char*)tmp_st.c_str(),delims);
            //to remove FN
        stoken=strtok(NULL,delims);
        while (stoken!=NULL){
            if (isdigit(stoken[0]))
             rate =stoken;
         else{
            GenFun=std::string("")+tabt[tt].trans_name+"_general";
            FuncT=std::string("&")+tabt[tt].trans_name+"_general";
            if (mapOfFunctions.find(FuncT)==mapOfFunctions.end()){
                                //out<<"mapOfFunctions[FuncT]=std::string(&)+std::string(stoken);\n";
                out<<"mapOfFunctions[\""+FuncT+"\"]=&"+std::string("&")+tabt[tt].trans_name+"_general"+";\n";
                mapOfFunctions[FuncT]=true;
            }
        }
        stoken=strtok(NULL,delims);
    }
    rate="1.0";
}
foutT<<" "<<GenFun<<" "<<FuncT<<" "<<rate<<timing<<"\n";
l_ptr = GET_INPUT_LIST(tt);

while (l_ptr != NULL)
{
    pp = GET_PLACE_INDEX(l_ptr);
    TP[tt][pp] = -l_ptr->molt;
    TPI[tt][pp] = -l_ptr->molt;
    foutT<<pp<<" "<<l_ptr->molt<<"\n";
    l_ptr = NEXT_NODE(l_ptr);
}
        //as separetor
foutT<<-1<<" "<<-1<<"\n";
        //as separetor
l_ptr = GET_INHIBITOR_LIST(tt);
while (l_ptr != NULL)
{
    pp = GET_PLACE_INDEX(l_ptr);
            //TP[tt][pp]+=l_ptr->molt;
    foutT<<pp<<" "<<l_ptr->molt<<"\n";
           // out << " pt.Id = " << pp << ";\n pt.Card = " << l_ptr->molt << ";\n t.InhPlaces.push_back(pt);\n";
    l_ptr = NEXT_NODE(l_ptr);
}
        //as separetor
foutT<<-1<<" "<<-1<<"\n";
        //as separetor
l_ptr = GET_OUTPUT_LIST(tt);
while (l_ptr != NULL)
{
    pp = GET_PLACE_INDEX(l_ptr);
    TP[tt][pp] += l_ptr->molt;
    foutT<<pp<<"\n";
    l_ptr = NEXT_NODE(l_ptr);
}
        //as separetor
foutT<<-1<<"\n";
        //as separetor
for (int i=0;i<npl;i++){
    if (TP[tt][i]!=0){
        foutT<<i<<" "<< TP[tt][i]<<"\n";
    }
}
        //as separetor
foutT<<-1<<" "<<-1<<"\n";
        //as separetor
}
out<< "//Loop Transitions \n for (int tt = 0; tt <"<<ntr<<"; tt++){";
out << "\n\t t.InPlaces.clear();\n\t t.InhPlaces.clear();\n\t t.InOuPlaces.clear();\n\t t.Places.clear();\n\t std::string discrete,GenFun,FuncT;\n";
out << "\t finT>> discrete >> GenFun >>FuncT>>t.rate;\n";
out << "\t if (discrete==\"false\") t.discrete=false; else t.discrete=true;\n";
out << "\t if (GenFun==\"nullptr\") t.GenFun=\"\"; else t.GenFun=GenFun;\n";
out << "\t t.FuncT=mapOfFunctions[FuncT];\n";
    //input places
out << "\n//// Input places\n";
out << "\t do{\n";
out << "\t\t finT>>pt.Id>>pt.Card;\n";
out << "\t\t if (pt.Id!=-1) {\n";
out << "\t\t\t t.InPlaces.push_back(pt);\n"<<" \t\t\t t.InOuPlaces.insert(pt.Id);\n\t\t\t}\n";
out << "\t } while(pt.Id!=-1);\n";
    //inhibitor places
out << "\n//// Inhibitor places\n";
out << "\t do{\n";
out << "\t\t finT>>pt.Id>>pt.Card;\n";
out << "\t\t if (pt.Id!=-1) {\n";
out << "\t\t\t t.InhPlaces.push_back(pt);\n\t\t\t}\n";
out << "\t } while(pt.Id!=-1);\n";
    //output places
out << "\n//// Ouput places\n";
out << "\t do{\n";
out << "\t\t finT>>pt.Id;\n";
out << "\t\t if (pt.Id!=-1) {\n";
out << "\t\t\t t.InOuPlaces.insert(pt.Id);\n\t\t\t}\n";
out << "\t } while(pt.Id!=-1);\n";
out << "\t do{\n";
out << "\t\t finT>>pt.Id>>pt.Card;\n";
out << "\t\t if (pt.Id!=-1) {\n";
out << "\t\t\t t.Places.push_back(pt);\n\t\t\t}\n";
out << "\t } while(pt.Id!=-1);\n\n";
out << "\t se.InsertTran(tt,t);\n";
out<< " }\n";
//to remove implicit places
if (ErrorLU)
{
    cout<<"\n\tWarning: no imlicit places are considered.\n\n";
    implPlace.clear();
}
//to remove implicit places

out<< " finT.close();\n";
out<< " mapOfFunctions.clear();\n";

//Places
    //Opening transition file
out<< "\n string fileP=(string(argv[0])+\".placesODE\");\n";
out<< " ifstream finP(fileP.c_str(), std::ifstream::in);\n";
out<< " if (!finP){\n";
out<< "\t cerr<<\"Error:  it is not possible to open output file for place description \"<<fileT<< \"\\n\";\n";
out<< "\t return false;\n\t}\n";

std::string fileP=(path+".solver.placesODE");
ofstream foutP(fileP.c_str(), std::ofstream::out);
if (!foutP)
{
    cerr<<"\nError:  it is not possible to open output file for place description "<<fileT<< "\n";
    exit(EXIT_FAILURE);
}


out<< "//Loop Places \n for (int pp = 0; pp <"<<npl<<"; pp++){\n";
out << "\t eq.clear();\n";
out << "\t int IncDec,tran;\n";
out << "\t do{\n";
out << "\t\t finP>>IncDec>>tran;\n";
out << "\t\t if (tran!=-2) {\n";
out << "\t\t\t el.setIncDec(IncDec);\n\t\t\t el.setIdTran(tran);\n\t\t\t eq.Insert(el);\n\t\t\t}\n";
out << "\t } while(tran!=-2);\n";
out << "\t Vpl.clear();\n";
out << "\t do{\n";
out << "\t\t finP>>pt.Id>>pt.Card;\n";
out << "\t\t if (pt.Id!=-2) {\n";
out << "\t\t\t Vpl.push_back(pt);\n\t\t\t}\n";
out << "\t } while(pt.Id!=-2);\n";
out << "\t eq.Insert(Vpl);\n";
out << "\t int total,lbound,ubound;\n";
out << "\t finP>>total>>lbound>>ubound;\n";
out << "\t se.InsertEq(pp,eq,total,lbound,ubound);\n";
out << " }";

for (pp = 0; pp < npl; pp++)
{
        for (int tt = 0; tt < ntr; tt++) //all places
        {
            if (TP[tt][pp] != 0)
            {
                foutP<<TP[tt][pp]<<" "<<tt<<"\n";
            }
        }//all places
        //as separetor
        foutP<<-2<<" "<<-2<<"\n";
        //as separetor


        if (implPlace.find(pp) != implPlace.end())// implicit place
        {

            list < pair<int, int> >::iterator it;
            for (it = implPlace[pp].begin(); it != implPlace[pp].end(); ++it)
            {
                foutP<<it->first<<" "<<it->second<<"\n";
            }

        }// implicit place
        foutP<<-2<<" "<<-2<<"\n";
        foutP<<net_mark[pp].total << " " << lbound[pp] << " " << ubound[pp]<<"\n";
    }
    //automaton

//if soft bound file is specified
    out << "\n if (fbound!=\"\") {\n\t if (!(se.readSLUBounds(fbound))) exit(EXIT_FAILURE);;\n }";

 //if soft init file is specified
    out << "\n if (finit!=\"\") {\n\t if (!(se.readInitialMarking(finit))) exit(EXIT_FAILURE);\n }";
    out << "\n if (fparm!=\"\") {\n\t if (!(se.readParameter(fparm))) exit(EXIT_FAILURE);\n }";

    //automaton
    cout << "\tDone.\n" << endl;
    out << "\n se.setEpsTAU(epsTAU);\n";
    out << "\n se.Print();\n";
    //automaton
    if (AUTOMATON)
    {
        out << "\n cout<<\"\\n\\nREADING AUTOMATON...\"<<endl;\n";
        out << " se.initialize_automaton(argv[2]);\n";
        out << " cout<<\"\\n\\nDONE.\"<<endl;\n";
    }
    //automaton

   //flux balance
    if (FLUXB){
        out << "\n cout<<\"\\n\\nREADING FLUX BALANCE PROBLEMS...\"<<endl;\n";
        for (unsigned int i=0; i<flux_names.size(); ++i){
            out << " se.initialize_fluxbalance(string(\""+flux_names[i]+"\"));\n";
        }
    //variability    
        out << " if (VARIABILITY) se.setVariability(VARIABILITY);\n";
    //variability
        out << " cout<<\"\\n\\nDONE.\"<<endl;\n";
    }
    //flux balance

    out << "\n\ntry\t{";
    out << "\n\tif (SOLVE==-1) \{\n\t\t cerr<< \"\\n\\nError: solution methods is not implemented\\nYou should use:  ODE-E or ODE-RKF or ODE45 or LSODA or SDE or HODE or HSDE or TAUG or SSA or STEP\\n\"; \n\t\t exit(EXIT_FAILURE);\n\t}\n\n ";



    out << "\n\tif (SOLVE == 1)\n\t\t se.SolveODEEuler(hini,atolODE,rtolODE,ftime,OUTPUT,stime,argv[1]);\n\t else\n\t\t if (SOLVE == 0)\n\t\t\t se.SolveSDEEuler(hini,atolODE,rtolODE,ftime,runs,OUTPUT,stime,argv[1]);\n\t\t else \n\t\t\tif (SOLVE == 3)\n\t\t\t\t se.SolveSSA(hini,atolODE,rtolODE,ftime,runs,OUTPUT,stime,argv[1]); \n\t\t\t else \n\t\t\t\t if (SOLVE == 4)\n\t\t\t\t\t  se.HeuristicStep(hini,atolODE,rtolODE,ftime,OUTPUT,stime,argv[1]);   \n\t\t\t\t else\n\t\t\t\t\t if (SOLVE == 5)\n\t\t\t\t\t  se.SolveODERKF(hini,atolODE,ftime,OUTPUT,stime,argv[1]);   \n\t\t\t\t else\n\t\t\t\t\tif (SOLVE == 6)\n\t\t\t\t\t\t se.SolveODE45(hini,atolODE,ftime,OUTPUT,stime,argv[1]);\n\t\t\t\t else\n\t\t\t\t\t if (SOLVE == 8)\n\t\t\t\t\t\t\t se.SolveHLSODE(hini,atolODE,rtolODE,ftime,runs,OUTPUT,stime,argv[1]);\n\t\t\t\t\t else \n\t\t\t\t\t\t\t if (SOLVE == 7) \n\t\t\t\t\t\t\t\t se.SolveLSODE(hini,atolODE,rtolODE,ftime,OUTPUT,stime,argv[1]);\n\t\t\t\t\t\t\t else  \n\t\t\t\t\t\t\t\t se.SolveTAUG(ftime,runs,OUTPUT,stime,argv[1]);";
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


 //To mapping places and transitions
    std::string filenamePT=path+".PlaceTransition";
    ofstream PTout(filenamePT.c_str());

    if (!PTout)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }
    PTout<<"#PLACE  ID\n";
    for (int pp = 0; pp < npl; pp++)
    {
       PTout<<tabp[pp].place_name<<"\t"<<pp<<endl;
   }
   PTout<<"#TRANSITION  ID\n";
   for (int tt = 0; tt < ntr; tt++)
   {
       PTout<<tabt[tt].trans_name<<"\t"<<tt<<endl;
   }
   PTout.close();

   cout << "===================== INFO =====================" << endl;
   cout << " Total Time: " << setprecision(7) << timeGlobal << " sec" << endl;
   cout << " Total Used Memory: " << usage1.ru_maxrss << "KB" << endl;
   cout << " Output saved in: " << net << ".cpp" << "\n";
   cout << "================================================\n" << endl;
}/* End build_ODECompact */




/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
   void 
   build_ODE(ofstream &out, std::string path, std::string net)
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
    for (int i = 0; i < npl; i++)
    {
        hout << "#define " << tabp[i].place_name << "_place " << i<<endl;
    }
    set<std::string> function_names;
    for (int tt = 0; tt < ntr; tt++)
    {

        if (tabt[tt].general_function != NULL || (tabt[tt].timing == TIMING_DETERMINISTIC && tabt[tt].general_function == NULL /*dirac*/))
        {
            //if(tabt[tt].timing == TIMING_DETERMINISTIC ){
              //  hout << TIMING_DETERMINISTIC << " sei dunque generale?" << endl; //Sono commossa.
           // }
            if (!FLUXB)
                hout<<"double "<<tabt[tt].trans_name<<"_general(double *Value, map <std::string,int>& NumTrans,  map <std::string,int>& NumPlaces,const vector <string>& NameTrans, const struct InfTr* Trans, const int Tran, const double& Time);\n";
            else
                hout<<"double "<<tabt[tt].trans_name<<"_general(double *Value,vector<class FBGLPK::LPprob>& vec_fluxb,  map <std::string,int>& NumTrans,  map <std::string,int>& NumPlaces,const vector <string>& NameTrans, const struct InfTr* Trans, const int Tran, const double& Time);\n"; 
        }
    }
    hout<<"};\n";
    hout.close();
        //for transition function rates
    out << "using namespace SDE;\nextern double epsilon;\n\n";

    //then I'll have to define the extern vector files

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
    out << " int SOLVE = 7, runs=1;\n";
    out << " long int seed = 0;\n";
    out << " bool OUTPUT=false;\n";
    //variability 
    if (FLUXB){
        out << " bool VARIABILITY=false;\n";
    }
    //variability
    out << " std::string fbound=\"\", finit=\"\", fparm=\"\";\n";
    out << " double hini = 1e-6, atolODE = 1e-6, rtolODE=1e-6, ftime=1.0, stime=0.0, itime=0.0, epsTAU=0.1;\n\n";
    out << " cout<<\"\\n\\n =========================================================\\n\";\n";
    out << " cout<<\"|                ODE/SDE Solver                       |\\n\";\n";
    out << " cout<<\" =========================================================\\n\";\n";
    out << " cout<<\"\\n If you find any bug, send an email to beccuti@di.unito.it\\n\";\n\n";

    //automaton
    if (AUTOMATON)
    {
        out << " if (argc<3)\n\t{\n \t";
        out << " std::cerr<<\"\\n\\nUSE:" << net << "_solve <out_file> <automaton_file> [OPTION ]<type> <hini>  <atol> <rtol> <runs> <ftime> <stime>   -B <bound_file> \";\n\t";

    }
    else
    {
        out << " if (argc<2)\n\t{\n \t";
        out << " std::cerr<<\"\\n\\nUSE:" << net << "_solve <out_file> [OPTIONS]\";\n\t";
    }
    //automaton
    out<<"std::cerr<<\"\\n\\n\\tOPTIONS\\n\";\n\t";
    out << " std::cerr<<\"\\n\\t -type <type>:\\t\\t ODE-E or ODE-RKF or ODE45 or LSODA or HLSODA or (H)SDE or HODE or SSA or TAUG or STEP. Default: LSODA \";\n\t";
    out << " std::cerr<<\"\\n\\t -hini <double>:\\t Initial step size. Default: 1e-6\";\n\t";
    out << " std::cerr<<\"\\n\\t -atol <double>:\\t Absolute error tolerance. Dafault: 1e-6\";\n\t";
    out << " std::cerr<<\"\\n\\t -rtol <double>:\\t Relative error tolerance. Dafault: 1e-6\";\n\t";
    out << " std::cerr<<\"\\n\\t -taueps <double>:\\t Epsilon value for Tau-leaping algorithm. Dafault: 0.1\";\n\t";
    out << " std::cerr<<\"\\n\\t -runs <int>:\\t\\t Integer number corresponding to runs (only used in SSA,TAUG, HODE,HLSODA). Default: 1\";\n\t";
    out << " std::cerr<<\"\\n\\t -ftime <double>:\\t Double number used to set the upper bound of the evolution time. Dafault: 1\";\n\t";
    out << " std::cerr<<\"\\n\\t -stime <double>:\\t Double number used to set the step in the output. Default: 0.0 (no output)\";\n\t";
    out << " std::cerr<<\"\\n\\t -itime <double>:\\t Double number used to set the initial simulation time. Default: 0.0 \";\n\t";
    out << " std::cerr<<\"\\n\\t -b <bound_file>:\\t Soft bound are defined in the file <bound_file>\";\n\t";
    out << " std::cerr<<\"\\n\\t -seed <double>:\\t Seed of random number generator\";\n\t";
    //variability 
    if (FLUXB){
        out << " std::cerr<<\"\\n\\t -var:\\t Enable output for variability analysis of fluxes\";\n\t";
    }
    //variability
    out << " std::cerr<<\"\\n\\t -init <init_file>:\\t The file <initial_file> contains the initial marking. Default:  initial marking in the orginal net\";\n\t";
    out << " std::cerr<<\"\\n\\t -parm <parm_file>:\\t The file <parm_file> contains a set of pairs with format <transition name> <value> or <place name> <value>.\\n\\t\\t\\t\\t For transition  the value is used to set a new rate value, while for place  it is used to set a new initial marking.\";\n\t";
    //automaton
    if (AUTOMATON)
        out << " std::cerr<<\"\\n\\t <automaton_file>:\\t automaton is defined in the file <automaton>\\n\";";
    out <<" std::cerr<<endl<<endl;";
    //automaton
    out << "\n\t exit(EXIT_FAILURE);\n\t}\n\n";


    if (AUTOMATON)
        out<<" int ii=3;\n";
    else
        out<<" int ii=2;\n";
    out<<" for (; ii<argc; ii++){\n";
    out<<"\t if (strcmp(\"-type\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out << "\t\t\t if ((strcmp(argv[ii],\"ODE-E\")==0)||(strcmp(argv[ii],\"ode-e\")==0)) SOLVE = 1;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"ODE-RKF\")==0)||(strcmp(argv[ii],\"ode-rkf\")==0)) SOLVE = 5;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"ODE45\")==0)||(strcmp(argv[ii],\"ode45\")==0)) SOLVE = 6;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"LSODA\")==0)||(strcmp(argv[ii],\"lsoda\")==0)) SOLVE = 7;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"STEP\")==0)||(strcmp(argv[ii],\"step\")==0)) SOLVE = 4;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"SSA\")==0)||(strcmp(argv[ii],\"ssa\")==0)){\n";
                //out << "\t\t\t\t cout<<\"\\t using simulation\"<<endl;\n\t\t\t\t epsilon=10000000000;\n\t\t\t\t hini=MAXSTEP; \n\t\t\t\t SOLVE=3;\n\t\t\t }\n";
    out << "\n\t\t\t\t SOLVE=3;\n\t\t\t }\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"HODE\")==0)||(strcmp(argv[ii],\"hode\")==0)) SOLVE = 2;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"HLSODA\")==0)||(strcmp(argv[ii],\"hlsoda\")==0)) SOLVE = 8;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"SDE\")==0)||(strcmp(argv[ii],\"sde\")==0) || (strcmp(argv[ii],\"HSDE\")==0)||(strcmp(argv[ii],\"hsde\")==0) ) SOLVE = 0;\n";
    out << "\t\t\t else if ((strcmp(argv[ii],\"TAUG\")==0)||(strcmp(argv[ii],\"taug\")==0)) SOLVE = 9;\n";
    out<<"\t\t\t else{\n";
    out<< "\t\t\t\t std::cerr<<\"\\n\\tError:  -type  <value>\\n\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";
 //hini code
    out<<"\t if (strcmp(\"-hini\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t hini=atof(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -hini  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";
 //atol code
    out<<"\t if (strcmp(\"-atol\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t atolODE=atof(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -atol  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";
 //rtol code
    out<<"\t if (strcmp(\"-rtol\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t rtolODE=atof(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -rtol  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";
 //runs code
    out<<"\t if (strcmp(\"-runs\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t runs=atoi(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -runs  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";
 //ftime code
    out<<"\t if (strcmp(\"-ftime\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t ftime=atof(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -ftime  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";

//stime code
    out<<"\t if (strcmp(\"-stime\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t OUTPUT=true;\n";
    out<<"\t\t\t stime=atof(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -stime  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";

//itime code
    out<<"\t if (strcmp(\"-itime\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t itime=atof(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -itime <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";

//bound file code
    out<<"\t if (strcmp(\"-b\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t fbound=string(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -b  <file_name>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";
//epsilon TAUleaping
    out<<"\t if (strcmp(\"-taueps\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t epsTAU=atof(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -taueps  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";

//seed of random number generator
    out<<"\t if (strcmp(\"-seed\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t seed=atol(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -seed  <value>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";

//variability
//variability analysis
    if (FLUXB){
        out<<"\t if (strcmp(\"-var\", argv[ii])==0){\n";
        out<<"\t\t VARIABILITY=true;\n";
        out<<"\t\t continue;\n";
        out<<"\t }\n";
    }
//variability

//initial file code
    out<<"\t if (strcmp(\"-init\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t finit=string(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -init  <file_name>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";

    out<<"\t if (strcmp(\"-parm\", argv[ii])==0){\n";
    out<<"\t\t if (++ii<argc){\n";
    out<<"\t\t\t fparm=string(argv[ii]);\n\t\t }\n";
    out<<"\t\t else{\n";
    out<< "\t\t\t std::cerr<<\"\\nError:  -parm  <file_name>\\n\";\n\t\t\t exit(EXIT_FAILURE);\n\t\t }\n";
    out<<"\t\t continue;\n";
    out<<"\t }\n";

    out<< "\t\t\t std::cerr<<\"\\nError:  unknown parameter \"<<argv[ii]<<\"\\n\\n\";\n\t\t\t exit(EXIT_FAILURE);\n";
    out<<" }\n\n\n";


    out<<" if (stime==0.0)  stime=ftime;\n\n";

    out << " time(&time_1);\n\n";

    out << " cout<<\"\\n=====================INPUT PARAMETERS======================\\n\";\n";
    out << " cout<<\"\\n\\tCompact CPP code: OFF\\n\";\n";
    out << " cout<<\"\\tType solution: \"<<SOLVE<<\"\\n\";\n";
    if (MASSACTION)
        out << " cout<<\"\\tTransition policy: Genelarized Mass Action policy\\n\";\n";
    else
        out << " cout<<\"\\tTransition policy: Minimum\\n\";\n";
    out << " cout<<\"\\tSolution final time: \"<<ftime<<\"\\n\";\n";
    out << " cout<<\"\\tInitial size step: \"<<hini<<\"\\n\";\n";
    out << " cout<<\"\\tInitial  time: \"<<itime<<\"\\n\";\n";
    out << " cout<<\"\\tAbosolute tolerance: \"<<atolODE<<\"\\n\";\n";
    out << " cout<<\"\\tRelative tolerance: \"<<rtolODE<<\"\\n\";\n";
    //out << " if ((strcmp(argv[2],\"ODE\")!=0)&&(strcmp(argv[2],\"ode\")!=0)){\n";
    out << " cout<<\"\\tEpsilon value for TAU-leaping: \"<<epsTAU<<\"\\n\";\n";
    out << " cout<<\"\\tSolution runs: \"<<runs<<\"\\n\";\n";
    out << " if (fbound!=\"\") cout<<\"\\tBound file: \"<<fbound<<\"\\n\";\n";
    out << " if (finit!=\"\") cout<<\"\\tInitial marking file: \"<<finit<<\"\\n\";\n";
    out << " if (fparm!=\"\") cout<<\"\\tInitial parameter file: \"<<fparm<<\"\\n\";\n";
    //Variability
    if (FLUXB){
        out << " if (VARIABILITY) cout<<\"\\tEnable variability analysis.\\n\";\n";
    }
    //Variability
    //automaton
    if (AUTOMATON)
        out << " cout<<\"\\tAutomaton input: \"<<argv[2]<<\"\\n\";\n";
    //automaton
    out << " cout<<\"\\tDetailed output: \"<<stime<<\"\\n\";\n";
    out << " cout<<\"\\n===========================================================\\n\";";
    out << " cout<<\"\\n\\nSTART EXECUTION...\"<<endl;\n\n";

    cout << "\n\nSTART EXECUTION..." << endl;
    out << " struct InfPlace pt;\n";
    out << " struct InfTr t;\n";
    out << " Equation eq;\n Elem el;\n";
    if (MASSACTION)
        out << " SystEqMas se(" << npl << "," << ntr << ",places,transitions,itime,seed);\n";
    else
        out << " SystEqMin se(" << npl << "," << ntr << ",places,transitions,itime,seed);\n";
    out << " vector< struct InfPlace> Vpl;\n\n";


    for (int tt = 0; tt < ntr; tt++)
    {


        out << "//Transition " << tabt[tt].trans_name << "\n t.InPlaces.clear();\n t.InhPlaces.clear();\n t.InOuPlaces.clear();\n t.Places.clear();\n";

        std::string enable="false";
        std::string GenFun="";
        std::string FuncT="nullptr";
        std::string rate=std::to_string(tabt[tt].mean_t);
        std::string timing= std::to_string(tabt[tt].timing);
        //if (tabt[tt].general_function != NULL)
        if (tabt[tt].general_function != NULL || (tabt[tt].timing == TIMING_DETERMINISTIC && tabt[tt].general_function == NULL /*dirac*/))
        {
            GenFun=std::string("")+tabt[tt].trans_name+"_general";
            FuncT=std::string("&")+tabt[tt].trans_name+"_general";
            rate="1.0";

        }
        out<<" t.timing = "<<timing<<";\n";
        out<<" t.discrete = "<<enable<<";\n";
        out<<" t.GenFun= \""<<GenFun<<"\";\n";
        out<<" t.FuncT=  "<<FuncT<<";\n";
        out<<" t.rate = "<<rate << ";\n";

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
//        if (implPlace.find(pp) == implPlace.end())   //explicit place
//        {
        for (int tt = 0; tt < ntr; tt++) //all places
        {
            if (TP[tt][pp] != 0)
            {
                out << " el.setIncDec(" << TP[tt][pp] << ");\n el.setIdTran(" << tt << ");\n eq.Insert(el);\n";
            }
        }//all places
//        }
//        else
        if (implPlace.find(pp) != implPlace.end())// implicit place
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

//if soft bound file is specified
    out << "\n if (fbound!=\"\") {\n\t if (!(se.readSLUBounds(fbound))) exit(EXIT_FAILURE);;\n }";

 //if soft init file is specified
    out << "\n if (finit!=\"\") {\n\t if (!(se.readInitialMarking(finit))) exit(EXIT_FAILURE);\n }";
    out << "\n if (fparm!=\"\") {\n\t if (!(se.readParameter(fparm))) exit(EXIT_FAILURE);\n }";

    //automaton
    cout << "\tDone.\n" << endl;
    out << "\n se.setEpsTAU(epsTAU);\n";
    out << "\n se.Print();\n";
    //automaton
    if (AUTOMATON)
    {
        out << "\n cout<<\"\\n\\nREADING AUTOMATON...\"<<endl;\n";
        out << " se.initialize_automaton(argv[2]);\n";
        out << " cout<<\"\\n\\nDONE.\"<<endl;\n";
    }
    //automaton

   //flux balance
    if (FLUXB){
        out << "\n cout<<\"\\n\\nREADING FLUX BALANCE PROBLEMS...\"<<endl;\n";
        for (unsigned int i=0; i<flux_names.size(); ++i){
            out << " se.initialize_fluxbalance(string(\""+flux_names[i]+"\"));\n";
        }
    //variability    
        out << " if (VARIABILITY) se.setVariability(VARIABILITY);\n";
    //variability
        out << " cout<<\"\\n\\nDONE.\"<<endl;\n";
    }
    //flux balance



    out << "\n\ntry\t{";
    out << "\n\tif (SOLVE==-1) \{\n\t\t cerr<< \"\\n\\nError: solution methods is not implemented\\nYou should use:  ODE-E or ODE-RKF or ODE45 or LSODA or SDE or HODE or HSDE or TAUG or SSA or STEP\\n\"; \n\t\t exit(EXIT_FAILURE);\n\t}\n\n ";



    out << "\n\tif (SOLVE == 1)\n\t\t se.SolveODEEuler(hini,atolODE,rtolODE,ftime,OUTPUT,stime,argv[1]);\n\t else\n\t\t if (SOLVE == 0)\n\t\t\t se.SolveSDEEuler(hini,atolODE,rtolODE,ftime,runs,OUTPUT,stime,argv[1]);\n\t\t else \n\t\t\tif (SOLVE == 3)\n\t\t\t\t se.SolveSSA(hini,atolODE,rtolODE,ftime,runs,OUTPUT,stime,argv[1]); \n\t\t\t else \n\t\t\t\t if (SOLVE == 4)\n\t\t\t\t\t  se.HeuristicStep(hini,atolODE,rtolODE,ftime,OUTPUT,stime,argv[1]);   \n\t\t\t\t else\n\t\t\t\t\t if (SOLVE == 5)\n\t\t\t\t\t  se.SolveODERKF(hini,atolODE,ftime,OUTPUT,stime,argv[1]);   \n\t\t\t\t else\n\t\t\t\t\tif (SOLVE == 6)\n\t\t\t\t\t\t se.SolveODE45(hini,atolODE,ftime,OUTPUT,stime,argv[1]);\n\t\t\t\t else\n\t\t\t\t\t if (SOLVE == 8)\n\t\t\t\t\t\t\t se.SolveHLSODE(hini,atolODE,rtolODE,ftime,runs,OUTPUT,stime,argv[1]);\n\t\t\t\t\t else \n\t\t\t\t\t\t\t if (SOLVE == 7) \n\t\t\t\t\t\t\t\t se.SolveLSODE(hini,atolODE,rtolODE,ftime,OUTPUT,stime,argv[1]);\n\t\t\t\t\t\t\t else  \n\t\t\t\t\t\t\t\t se.SolveTAUG(ftime,runs,OUTPUT,stime,argv[1]);";
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


 //To mapping places and transitions
    std::string filenamePT=path+".PlaceTransition";
    ofstream PTout(filenamePT.c_str());

    if (!PTout)
    {
        cerr << "Error: it is not possible to output file "<<filename<<"\n\n";
        exit(EXIT_FAILURE);
    }
    PTout<<"#PLACE  ID\n";
    for (int pp = 0; pp < npl; pp++)
    {
       PTout<<tabp[pp].place_name<<"\t"<<pp<<endl;
   }
   PTout<<"#TRANSITION  ID\n";
   for (int tt = 0; tt < ntr; tt++)
   {
       PTout<<tabt[tt].trans_name<<"\t"<<tt<<endl;
   }
   PTout.close();

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
        if (tabt[tt].rate_par_id>=0)
            out<<tabt[tt].trans_name << " = "<<tabrp[tabt[tt].rate_par_id].rate_name;
        else
            if (tabt[tt].mean_t!=0)
                out<<tabt[tt].trans_name << " = "<<tabt[tt].mean_t;

            if (MASSACTION)
            {
                int elem=0;
                for (int pp1 = 0; pp1 < npl; pp1++){
                //cout << "trans " << tabt[tt].trans_name << "-> " <<TPI[tt][pp1]<<"\n" ;
                    if (TPI[tt][pp1] < - 1){
                        if (elem>0){
                            out<<" * factorial("<<abs(TPI[tt][pp1])<<")";
                        }
                        else{
                            out<<" / ( factorial("<<abs(TPI[tt][pp1])<<")";
                        }
                        elem++;
                    }

                }
                if (elem>0)
                    out<<" )";
            }
            out<<"\n";

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
        out<<"##Begin ODE Terms (X all transitions)\n";
        for (int tt = 0; tt < ntr; tt++)
        {
            out<<"R_"<<tabt[tt].trans_name<<" = ";

            if (tabt[tt].general_function==NULL)
            {

                if (MASSACTION)
                {
                    out << " + " << tabt[tt].trans_name;
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
                    out << "+ " << tabt[tt].trans_name <<"*min( ";
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
                out<<general_function.substr(3,general_function.size()-4)<<"(y,\""<<tabt[tt].trans_name<<"\")";

            }


            out<<endl;
        }
        out<<"##End ODE Terms\n\n";


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
                    if (TP[tt][pp]>0)
                        out<<"+"<< TP[tt][pp]<<"*";
                    else
                        out<< TP[tt][pp]<<"*";
                    out<<"R_"<<tabt[tt].trans_name;
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
    out << "\nres1 <-lsoda(yini,Times,funODE,parms=0,hini=hini)";
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

    out << "res <- lsoda(y,Times,funODE,parms=x,hini=hini)\n";
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
        out << tabt[tt].trans_name << " = " << tabt[tt].mean_t;

        if (MASSACTION)
        {
            int elem=0;
            for (int pp1 = 0; pp1 < npl; pp1++){
                cout << "trans " << tabt[tt].trans_name << "-> " <<TPI[tt][pp1]<<"\n" ;
                if (TPI[tt][pp1] < - 1){
                    if (elem>0){
                        out<<" * factorial("<<abs(TPI[tt][pp1])<<")";
                    }
                    else{
                        out<<" / ( factorial("<<abs(TPI[tt][pp1])<<")";
                    }
                    elem++;
                }

            }
            if (elem>0)
                out<<" )";
        }
        out<<";\n";

#if DEBUG
        for (int i = 0; i < npl; i++)
            cout << "\t" << TP[tt][i];
        cout << endl;
#endif
    }

    out << "%End Transition rates\n\n";


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
                            out << " * " << tabp[pp1].place_name << "^" << abs(TPI[tt][pp1]);
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





