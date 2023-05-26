/***************************************************************************
 *   Copyright (C) 2013 by Marco Beccuti				   *
 *   beccuti@di.unito.it						   *
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



#ifndef __SSTREAM__
#define __SSTREAM__
#include <sstream>
#endif

#ifndef __IOS_H__
#define __IOS_H__
#include <iostream>
#endif

#ifndef __FST_H__
#define __FST_H__
#include <fstream>
#include <limits.h>
#endif

#ifndef __STDL__
#define __STDL__
#include <stdlib.h>
#endif

#ifndef __STDR__
#define __STDR__
#include <stdarg.h>
#endif

#ifndef __STR_H__
#define __STR_H__
#include <string.h>
#endif


#ifndef __VCT_H__
#define __VCT_H__
#include <vector>
#endif

#ifndef __LST_H__
#define __LST_H__
#include <list>
#endif

#ifndef __SET_H__
#define __SET_H__
#include <set>
#endif

#ifndef __MAP_H__
#define __MAP_H__
#include <map>
#include <unordered_map>
#endif

#ifndef __TLT_H__
#define __TLT_H__
#include <utility>
#endif

#ifndef __TIM_H__
#define __TIM_H__
#include <time.h>
#endif

#ifndef __RND_H__
#define __RND_H__
#include <random>
#include <chrono>
#endif

#ifndef __MTH_H__
#define __MTH_H__
#include <cmath>       /* exp */
#include <limits.h>
#endif

#ifndef __MEM_H__
#define __MEM_H__
#include <sys/time.h>
#include <sys/resource.h>
#endif

//automaton
#ifdef AUTOMATON
  #include "automa.hpp"
#endif
//automaton

#ifdef __DISTRIBUTION__
  #include <boost/math/distributions.hpp>
#endif


#ifndef __READ_SUP__
#define __READ_SUP__
#include "readSupport.hpp"
#endif

#ifndef __RGEX__
#define __RGEX__
#include <regex>
#endif


#ifndef __LSD_H__
#define __LSD_H__
#include "lsode.hpp"
#endif


#ifdef CGLPK
    #ifndef __CGLPK_H__
        #define __CGLPK_H__
        #include "GLPKsolve.hpp"
    #endif
#endif


#ifndef __REVERSE_HEAP_H__
#define __REVERSE_HEAP_H__
#include "reverse_heap.h"
#endif 


namespace SDE
{
  #define INCDEC 0.0
  #define DEFAULT 4294967295U
  #define DEBUG 1
  #define ADAPTATIVE 1
  #define ALPHA 0.99
  #define NON_EXP_GEN 127
  const double MAXSTEP=4294967295.0;

  using namespace std;
  using namespace CRS;
//automaton
#ifdef AUTOMATON
  using namespace AUTOMA;
#endif
//automaton

  extern vector<string> name_file;
  extern vector<Table> class_files;


  //!Exception
  struct Exception
  {
    std::string mess;
  public:
    //! Empty constructors
    Exception() {mess="";};
    //! constructor takes in input a message
    Exception(std::string mess) {this->mess=mess;};
    //! \name Get of methods use to access at the data structures
    //@{
    //!It returns message
    std::string get(void) {return mess;};
    //@}
  };

  //! enumeration used to prevent use of non-exponential general transition except in SSA
  enum solve_type {
    Solve_LSODE,
    Solve_SSA,
    Solve_HLSODE,
    Solve_TAUG
  };

#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__
#include "distribution.h"
#endif 

  //! It uses for encoding the place information
  struct InfPlace
  {
    double Card;
    int Id;
    int marking;
  };

  //! It uses for encoding the transition information (i.e. rate, inhibitor places, and its associated  negative exponential distribution)
  struct InfTr
  {
    double rate;
    //!encode the list of place connected by an inhibitor arc to the transition
    vector <InfPlace> InhPlaces;
    //!encode the list of transition input places
    vector <InfPlace> InPlaces;
    //!encode the list of places modified by the transition (it is a duplicated information)
    vector <InfPlace> Places;
    set<int> InOuPlaces;
    //!encode a negative exponential distribution
    std::exponential_distribution<double> dist[1];
    //!remember if this transition is enable in the diffusion process
    bool enable;
    //!indicate if the transition is exponential or general
    int timing;
    bool discrete {false};
    //!list of the events connected to the transition if it's general non exponential
    //vector<Event*> events;
    //!size of the events associated to the transition
    int events_size {0};
    //!Pointer to the first event of the transition
    Event* first_event {NULL};
    //!Pointer to the last event of the transition, used to add
    Event* last_event {NULL};
    //!encode the  brown noise value for the current time
    double BrownNoise;
    //!it stores for generic transition
    std::string GenFun {""};
    //!it is a pointer to a function   returns the transition intesity

   #ifdef CGLPK
    //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
    double (*FuncT)(double *Value, vector<class FBGLPK::LPprob>& vec_fluxb, map <string,int>& NumTrans, map <string,int>& NumPlaces, const vector <string>& NameTrans, const struct InfTr* Trans,  const int Tran, const double& Time) {nullptr};
    #else
    double (*FuncT)(double *Value, map <string,int>& NumTrans, map <string,int>& NumPlaces, const vector <string>& NameTrans, const struct InfTr* Trans,  const int Tran, const double& Time) {nullptr};
    #endif 
  };

  //!Class Elem
  /*!
   * This class stores an equation component.
   */

  class Elem
  {
  private:
    int IncDec; //!variable decrements or increment
    int IdTran; //!transition id used to fetch the transition information
    //vector <struct InfPlace> IdPlaces; //! a list of place/variable identifiers to fetch the place information

  public:
    //! Empty Constructor.
    Elem(){IncDec=-1; IdTran=-1;};
    //! Constructor. It takes in input all the information to create an  equation component
    Elem(int IncDec, int IdTran);
    //! Copy Constructor.
    Elem(const Elem&  El);
    //! Deconstruct
    ~Elem() {};
    //! \name Get of methods use to access at the data structures
    //@{
    //!It returns IncDec
    inline int getIncDec(){ return IncDec;};
    //!It returns IdTran
    inline int  getIdTran(){ return IdTran;};
    //@}
        //! \name Set of methods use to access at the data structures
    //@{
    //!It sets IncDec
    inline void setIncDec(int IncDec){ this->IncDec=IncDec;};
    //!It sets IdTran
    inline void  setIdTran(int IdTran){this->IdTran=IdTran;};
    //@}
    //!It prints an equation component
    void Print(vector <string>& NamePlaces, vector <string>& NameTrans,struct InfTr* Trans,string typeTfunction) const;
    //!It redefines the operator<<
    friend ostream& operator<<(ostream& out,const Elem& E){
      out<<"IncDec: "<<E.IncDec<<" IdTran: "<<E.IdTran<<endl;
      return out;
    };


  };

  //!Class Equation
  /*!
   * This class stores a single equation, each equation component is encoded on class Elem.
   */
  class Equation
  {
  private:
    //! it encodes all the  equation components for a variable
    vector <class Elem> LElem;
    //! it encodes the list of places and their cardinality used to compute this value. The last element is the P-invariant value.
    vector <struct InfPlace> PSemiflow;
    //! it is used to encode the list of places directly computed by Eurel method or those derived  by the others places in the same P-semiflow.
    unsigned int next;
  public:
    //! Empty constructor
    Equation() {next=DEFAULT; };
    //! Constructor for the places derived  by the others places in the same P-semiflow.
    Equation(vector <struct InfPlace> pl);
    //!Copy Constructor
    Equation(const Equation& Eq);
    //!Clear an equation
    inline void clear() {LElem.clear(); PSemiflow.clear(); next=DEFAULT; };
    //!Insert an equation component.
    inline void Insert(class Elem& El) {LElem.push_back(El);};
    //!Insert an equation component.
    void Insert(vector <struct InfPlace> pl);
    //!It returns the number of equation components
    inline int getSize(void){return LElem.size();};
    //!It returns the number of places in the P-semiflow
    inline int getSizeP(void){return PSemiflow.size();};
    //!It returns IncDec of the ith component
    inline int getIncDec(int i){return LElem[i].getIncDec();};
    inline int getIncDecTran(int tran){
      int N=LElem.size();
      for(int i=0;i<N; ++i){
        if(LElem[i].getIdTran()==tran)
          return LElem[i].getIncDec();
      }
      return 0;
    }

    //!It returns IdTran of the ith component
    inline int  getIdTrans(int i){ return LElem[i].getIdTran();};
    inline void getPsemflw(int i, int& place, double& coff) {place=PSemiflow[i].Id; coff=PSemiflow[i].Card;};
    inline void getPsemflw(int i, int& place) {place=PSemiflow[i].Id;};
    inline void setPsemflw(int i, double& coff) {PSemiflow[i].Card=coff;};
    //It returns the next place to be processed.
    inline unsigned int getNext() {return next;};
    //It set the next place to be processed.
    inline void setNext(unsigned int pos) {next=pos;};
    //It returns true if the place value is directly computed
    inline bool isDirectlyComputed() {return (PSemiflow.size()==0) ? true:false;};
    //!It prints the equation
    void Print(vector <string>& NamePlaces, vector <string>& NameTrans,struct InfTr* Trans,string typeTfunction);
    //!It redefines the operator<<
    friend ostream& operator<<(ostream& out, const Equation& Eq){
      //out<<"Current value: " <<Eq.value<<"\n";
      vector <class Elem>::const_iterator it = Eq.LElem.begin();
      while (it!= Eq.LElem.end())
      {
       out<<Elem(*it);
       ++it;
     }
     out<<endl;
     return out;
   }

 };


  //!Class SystEq
  /*!
   * This class stores the system equations encoded on class Equation.
   */
 class SystEq
 {
 protected:
    //! it encodes the transitions information
  struct InfTr* Trans;
    //!it encodes all the system equations
  class Equation* VEq {nullptr};
    //!it stores the current equation values (at time variable "time")
  double* Value {nullptr};
    //!it stores the previous equation values (at time variable "time")
  double* ValuePrv {nullptr};
    //!it stores the derivative values for a fixed transition (at time variable "time")
  double* DerivTAUG {nullptr};
    //!it stores the current enabling degree for continuous transition fire
  double* EnabledTransValueCon {nullptr};
    //!it stores the current enabling degree for discrete transition fire
  double* EnabledTransValueDis {nullptr};
  //!it stores the previous enabling degree values for discrete transition fire
  double* PreviousEnabledTransValueDis {nullptr};
    //!it  store the final place value of each run
  double** FinalValueXRun {nullptr};
    //!It used for TauLeaping
  double* TransRate {nullptr};
  int Max_Run {1};
    //!it stores the current time for which the equation values are computed
  double time {0.0};
    //!it stores the initial time for which the equation values are computed
  double itime {0.0};
    //!it stores the number of equation/places
  int nPlaces;
    //!it stores the number of transition
  int nTrans;
  vector <string> NameTrans;
  vector <string> NamePlaces;
  map<std::string,int> NumTrans;
  map<std::string,int> NumPlaces;
  string typeTfunction;
    //!It encodes the lower bound for each place
  double* LBound;
    //!It encodes the upper bound for each place
  double* UBound;
    //!It encodes the soft lower bounds used to extend our approach to hybrid system
  double *SLBound;
    //!It encodes the soft upper bounds used to extend our approach to hybrid system
  double *SUBound;
    //!It encodes the list of places computed directly
  unsigned int headDirc;
    //!It encodes the list of places derived by others
  unsigned int headDerv;
    //!It encodes the maximum step factor
  double fh {64};
    //!It encodes the percentage of previous value used to determine h
  double perc1 {0.5};
    //!It encodes the percentage of previous value used to determine h when the model is close to a bound
  double perc2 {0.01};
    //!It encodes the epsilon used in the TAU leap algorithm
  double epsTAU {0.01};
    //!It derived the place which are not directly computed (for ODE and SDE)
  void derived();
    //!It derived the place which are not directly computed from its input vector Value* (for ODE and SDE)
  void derived(double*);
    //!It derived the place which are not directly computed. It ends as soon as negative value is derived.
  void derived(bool&);
    //!From a place name it provides its position in NamePlaces.
  int search(string& name);
    //Max number of attempt used in RK
  int max_attempt {500};
    //!It stores the used seed
  long int seed {0};
  //!It indicates the solve type used in this execution
  solve_type solve;
    //!The heap that handles the future event list of general non exponential transition
  min_heap future_event_list {min_heap()};
  //!It removes the i-th event in the event list updating the pointers 
  Event* deleteEventInPosition(int event_index, int tran);
  //!It removes the i-th event in the event list updating the pointers 
  void deleteEvent(Event* event);
  //!add event ad the end of the list
  void addEventAtTheEnd(Event* event, int tran);

//automaton
#ifdef AUTOMATON
  class automaton automaton;
#endif
//automaton
//fluxb
#ifdef CGLPK
    //!It is a vector of pointers to flux balance problems.
    vector<class FBGLPK::LPprob> vec_fluxb;
    bool Variability {false};
#endif
//fluxb    
public:
    //! Empty Constructor
  SystEq(void){};
    //! Constructor it takes in input the total number of transitions and places, two vector with the names of places and transitions, initial simulation time,  and  a seed  of random number generator. Moreover the input parameter usedMin is true if transition infinity server policy is used, otherwise mass-product policy is used.
  SystEq(int nPlaces,int nTrans, string NameTrans[], string NamePlaces[],double itime,long int seed);
    //! Deconstruct
  virtual ~SystEq();
    //! It stores the transition information. It takes in input the transition id and its information encodes on ``struct InfTr''
  void InsertTran(int num, struct InfTr T);
    //! It stores an equation. It takes in input the equation/place id, its information encodes on class Equation, its initial value and its bounds.
  void InsertEq(int num, class Equation& Eq,double InitValue,double LBound, double UBound);
    //! It prints all the system information
  void Print();
    //! It checks if it is disable by an inhibitor arc
  bool NotEnable(int Tran);

    //!It computes the Tau taking as input the list of descrete transitions and the next time point. It returns a possible transition firing otherwise  -1. It requires that  getValTranFire() must be called before.
    //! In SSA it's necessary to pass also the set of non exponetial general transition
  int getComputeTau(int SetTranExp[], double& nextTimePoint,double t);
    //!It computes the Tau according to Gillespie algorithm. It takes as input the list of descrete transitions and the next time point. It returns a possible transition firing otherwise  -1. It requires that  getValTranFire() must be called before.
  double getComputeTauGillespie(int SetTran[],double t, double hstep);
    //! It is a pure virtual function which must be implemented.
  virtual void getValTranFire()=0;
  virtual void getValTranFire(double*)=0;
  virtual void setSizeFutureEventList(int nTrans, int &size_expTran, int &size_notExpTran)=0;
    //!It updates the future event list during the SSA with non exponential general transition using the value of Enabling
  //void updateFutureEventList(int trans);
  void updateFutureEventList(int tran, int prev_fired, double time);
  //! It checks if there is an enable transition which will fire in the current time step.
  int fireEnableTrans(  int SetTran[],double& h);
    //int fireEnableTrans(  set<int>&SetTran,double& h);
    //!It generates the brown noise value for all the transition involved  the diffusion process.
  bool setBNoiseTrans();
    //It generates the brown noise value for all the transition involved  the diffusion process
  void setBNoiseTrans(int);
    //!It automaticaly updates  Euler step according to maximum transition increment.
  void ComputeHstep(double& h);
  void ComputeHstep(const double& prv, const double& next, double& h);
    //! It sets the epsilon used in the TAU algoritm.
  void setEpsTAU(const double& epsTAU){this->epsTAU=epsTAU;};
  long int getSeed();
//automaton
#ifdef AUTOMATON
     /*!
      * initialize_automaton() it initializes automaton
      * @parm file_name name of the file storing the automaton description
      */
  void initialize_automaton(char *file_name);
#endif
//automaton
#ifdef CGLPK
      void initialize_fluxbalance(std::string flux_name){
      FBGLPK::LPprob l;
      vec_fluxb.push_back(l);
      vec_fluxb[vec_fluxb.size()-1].updateLP(flux_name.c_str()); 
      };
      inline void setVariability(bool v){Variability=v;};
#endif       
    //!It returns the brown noise value for the input transition.
  inline double getBNoiseTran(int Tran) {return Trans[Tran].BrownNoise;};
    //!It reads from a file the softh lower and upper bounds.
  bool readSLUBounds(const string& file);
    //!It reads from a file the initial marking.
  bool readInitialMarking(const string& file);
    //!It reads from a file new place markings and new transition rates
  bool readParameter(const string& file);
    //! It resets the list of enable transition
  void resetTrans();
    //! It solves the ODE system using Euler method. It takes in input the step size and the Max_Time
  void SolveODEEuler(double h,double prec1,double prec2, double Max_Time,bool Info,double Print_Step,char *argv);
    //! It solves the ODE system using Runge-Kutta-Fehlberg method. It takes in input the step size and the Max_Time
  void SolveODERKF(double h,double perc1,double Max_Time,bool Info,double Print_Step,char *argv);
    //! It solves the ODE system using  Dormand and Princ method - ode45  method. It takes in input the step size and the Max_Time
  void SolveODE45(double h,double perc1,double Max_Time,bool Info,double Print_Step,char *argv);
     //! It solves the ODE system using  LSODA method
  void SolveLSODE(double h,double perc1,double perc2,double Max_Time,bool Info,double Print_Step,char *argv);
    //!It computes a single step of Kutta-Merson integration
  void StepODERK5(double* Yn, double* Kn,const double& h);
    //!It   computes the values of the derivatives in the ODE system
  void fex(double t, double *y, double *ydot, void *data);
    //! It solves the (H)SDE system using Euler method. It takes in input the step size, the Max_Time and Max_Run. To print the trace of each run -> Info = true
  void SolveSDEEuler(double h,double prec1,double prec2,double Max_Time,int Max_Run, bool Info,double Print_Step,char *argv);
    //! It solves the HODE system using Euler method. It takes in input the step size, the Max_Time and Max_Run. To print the trace of each run -> Info = true
  void SolveHODEEuler(double h,double prec1,double prec2,double Max_Time,int Max_Run, bool Info,double Print_Step,char *argv);
    //! It solves the HODE system using Tauleaping and LSODE.It takes in input the step size, the Max_Time and Max_Run. To print the trace of each run -> Info = true
  void SolveHLSODE(double h,double perc1,double perc2,double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv);
    //! It solves the  system using SSA method.It takes in input the step size, the Max_Time and Max_Run. To print the trace of each run -> Info = true
  void SolveSSA(double h,double perc1,double perc2,double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv);
     //! It solves the ODE system using  tau-leaping method  method. It returns the computed tau.
  void SolveTAUG(double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv);
    //! It computes the sixth-order approximation of the derivative according to the Richardson's Extrapolation formula.
  double DerivApproximation(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, double hstep, int ider);
    //! They need for the sixth-order approximation of the derivative according to the Richardson's Extrapolation formula.
  double RichardsonExtrap(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, double hstep, int ider);
    /*
    double L(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, int hstep, int ider );
    double S(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, int hstep, int ider );
    */
    //! It computes an estimation for the Euler step
  void HeuristicStep(double h,double perc1,double perc2,double Max_Time,bool Info,double Print_Step,char *argv);
    //! It prints a matrix PlacesXRuns with the final computed values
  void PrintStatistic(char *argv);
  void PrintValue(std::ostream &os){ for (auto i=0; i<nPlaces; ++i) os<<NamePlaces[i]<<":"<<Value[i]<<" ";}
};

class SystEqMin:public SystEq
{
public:
  SystEqMin(int nPlaces, int nTrans, string NameTrans[], string NamePlaces[],double itime,long int seed):SystEq(nPlaces,nTrans,NameTrans,NamePlaces,itime,seed){typeTfunction="Min";};
  virtual ~SystEqMin();
    //! For each transition it returns  the min of the values of its input places
  virtual void getValTranFire() override;
     //! For each transition it returns  the min of the values of its input places considering its input vector ValuePrv as marking
  virtual void getValTranFire(double* ValuePrv) override;
  virtual void setSizeFutureEventList(int nTrans, int &size_expTran, int &size_notExpTran) override;

};

class SystEqMas:public SystEq
{
public:
  SystEqMas(int nPlaces, int nTrans, string NameTrans[], string NamePlaces[],double itime,long int seed):SystEq(nPlaces,nTrans,NameTrans,NamePlaces,itime,seed){typeTfunction="Prod";};
  virtual ~SystEqMas();
    //! For each transition it returns the product  of the values of its input places
  virtual void getValTranFire() override;
    //! For each transition it returns  the min of the values of its input places considering its input vector ValuePrv as marking
  virtual void getValTranFire(double* ValuePrv) override;
  virtual void setSizeFutureEventList(int nTrans, int &size_expTran, int &size_notExpTran) override;

};

//  double Fg(double *Value, vector <string>& NameTrans, vector <string>& NamePlaces);

}
