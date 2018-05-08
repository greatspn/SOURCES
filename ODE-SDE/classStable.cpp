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


#include "class.hpp"

double epsilon=1e-20;
double epsilon1=0.00000000001;
double ep=0.9;
double delta=0.00001;
double slow=false;
#ifdef AUTOMATON 
extern void reading_automaton( AUTOMA::automaton& a, char * name); 
extern double next_clock_automaton;
#endif
double MININC=0;
namespace SDE {
    
    
    
    
    long int seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    
    std::mt19937_64 generator(seed);
    std::normal_distribution<double> distribution(0.0,1.0);
    
    
    #ifdef AUTOMATON    
    double binomlow(int i,int j , double l){
        return boost::math::binomial_distribution<>::find_lower_bound_on_p(i,j,l);
    }
    
    double binomup(int i,int j , double l){
        return boost::math::binomial_distribution<>::find_upper_bound_on_p(i,j,l);
    }
    #endif
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : from a name returns its index*/
    /**************************************************************/ 
    int SystEq::search(string& name){
        int i=0;
        while (i<(signed)NamePlaces.size()){
            if (NamePlaces[i]==name)
                return i;
            ++i;
        }
        return -1;
    }
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : readSLUBounds reads softbound from a file*/
    /**************************************************************/ 
    bool SystEq::readSLUBounds(char *argv){
        ifstream in(argv, std::ifstream::in);
        if (!in)
        {
            cerr<<"\nError:  it is not possible to open the soft bound file "<<argv<< "\n";
            return false;
        }
        int i=0; 
        std::string name;
        cout<<"\n\n-----------------------------------------------------------\n"; 
        cout<<"\t\t  Reading soft bounds"<<endl;
        cout<<"-----------------------------------------------------------\n";
        while(in)
        {
            name="";  
            in>>name;
            if(name!=""){
                i=search(name);
                if (i!=-1){
                    in>>SLBound[i]>>SUBound[i];
                    LBound[i]=SLBound[i];
                    UBound[i]=SUBound[i];
                    cout<<"\t"<<NamePlaces[i]<<"\tLB: "<<LBound[i]<<"\tUB: "<<UBound[i]<<"\tSLB: "<<SLBound[i]<<"\tSUB: "<<SUBound[i]<<endl;
                }
                else
                    cerr<<"\nError:  place  "<<name<< " is not presented in the net. It will be ignored\n";
            }  
        }
        cout<<"-----------------------------------------------------------\n";
        return true;  
    }
    
    /**************************************************************/
    /* NAME :  Class SystEqMin*/
    /* DESCRIPTION : getMin returns the minimum value among the places*/
    /**************************************************************/  
    
    inline void SystEqMin::getValTranFire()
    {
        for(int t=0; t<nTrans; t++)
        {
            unsigned int size=Trans[t].InPlaces.size();
            EnabledTransValueCon[t]=0.0;
            EnabledTransValueDis[t]=0.0;
            if (size==0)
                EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;
            else
            {
                long double tmpC;
                long double tmpD;
                //if (Trans[t].InPlaces.size()>0)
                //{
                EnabledTransValueCon[t] =ValuePrv[Trans[t].InPlaces[0].Id]/Trans[t].InPlaces[0].Card; 
                EnabledTransValueDis[t] =trunc(EnabledTransValueCon[t]); 
                for (unsigned int k=1; k<size&&(EnabledTransValueCon[t]!=0.0); k++)//for all variable in the components
                {
                    tmpC=ValuePrv[Trans[t].InPlaces[k].Id]/Trans[t].InPlaces[k].Card;
                    tmpD=trunc(tmpC);
                    if ( EnabledTransValueCon[t]>tmpC){
                        EnabledTransValueCon[t]=tmpC;
                        EnabledTransValueDis[t]=tmpD;
                        
                    }
                    
                }
                
            }
            
        }
        
    }
    
    
    /**************************************************************/
    /* NAME :  Class SystEqMas*/
    /* DESCRIPTION : getProd returns the producd value among the places*/
    /**************************************************************/  
    
    inline void SystEqMas::getValTranFire()
    {
        for(int t=0; t<nTrans; t++)
        {
            EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;
            if (Trans[t].InPlaces.size()>0)
            { 
                for (unsigned int k=0; k<Trans[t].InPlaces.size(); k++)//for all variables in the components
                {
                    long double valD,valC;
                    valD=valC=ValuePrv[Trans[t].InPlaces[k].Id];
                    long double fatt=1;
                    for (int i=2;i<=Trans[t].InPlaces[k].Card;i++)
                    {
                        valD*=(ValuePrv[Trans[t].InPlaces[k].Id]-i+1);
                        valC*=ValuePrv[Trans[t].InPlaces[k].Id];
                        fatt*=(long double)i; 
                        
                    }
                    
                    if (valD>0)
                        EnabledTransValueDis[t]*=valD/fatt;
                    else
                        EnabledTransValueDis[t]=0.0;
                    EnabledTransValueCon[t]*=valC/fatt;
                }  
            }
            
        }
    }
    
    
    /**************************************************************/
    /* NAME :  Class SystEqMas*/
    /* DESCRIPTION : getProd returns the producd value among the places
                     considering its input vector ValuePrv as marking*/
    /**************************************************************/  
    
    inline void SystEqMas::getValTranFire(long double* ValuePrv)
    {
        for(int t=0; t<nTrans; t++)
        {
            EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;
            if (Trans[t].InPlaces.size()>0)
            { 
                for (unsigned int k=0; k<Trans[t].InPlaces.size(); k++)//for all variables in the components
                {
                    long double valD,valC;
                    valD=valC=ValuePrv[Trans[t].InPlaces[k].Id];
                    long double fatt=1;
                    for (int i=2;i<=Trans[t].InPlaces[k].Card;i++)
                    {
                        valD*=(ValuePrv[Trans[t].InPlaces[k].Id]-i+1);
                        valC*=ValuePrv[Trans[t].InPlaces[k].Id];
                        fatt*=(long double)i; 
                        
                    }
                    
                    if (valD>0)
                        EnabledTransValueDis[t]*=valD/fatt;
                    else
                        EnabledTransValueDis[t]=0.0;
                    EnabledTransValueCon[t]*=valC/fatt;
                }  
            }
            
        }
    }
    
    
    
    /**************************************************************/
    /* NAME :  Class SystEqMin*/
    /* DESCRIPTION : getMin returns the minimum value among the places 
                     considering its input vector ValuePrv as marking*/
    /**************************************************************/  
    
    inline void SystEqMin::getValTranFire(long double* ValuePrv)
    {
        for(int t=0; t<nTrans; t++)
        {
            unsigned int size=Trans[t].InPlaces.size();
            EnabledTransValueCon[t]=0.0;
            EnabledTransValueDis[t]=0.0;
            if (size==0)
                EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;
            else
            {
                long double tmpC;
                long double tmpD;
                //if (Trans[t].InPlaces.size()>0)
                //{
                EnabledTransValueCon[t] =ValuePrv[Trans[t].InPlaces[0].Id]/Trans[t].InPlaces[0].Card; 
                EnabledTransValueDis[t] =trunc(EnabledTransValueCon[t]); 
                for (unsigned int k=1; k<size&&(EnabledTransValueCon[t]!=0.0); k++)//for all variable in the components
                {
                    tmpC=ValuePrv[Trans[t].InPlaces[k].Id]/Trans[t].InPlaces[k].Card;
                    tmpD=trunc(tmpC);
                    if ( EnabledTransValueCon[t]>tmpC){
                        EnabledTransValueCon[t]=tmpC;
                        EnabledTransValueDis[t]=tmpD;
                        
                    }
                    
                }
                
            }
            
        }
        
    }
    
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : Constructor taking in input  the total number of transitions and places, and two vector with the names of places and transitions */
    /**************************************************************/  
    
    SystEq::SystEq(int nPlaces,int nTrans, string NamePlaces[],  string NameTrans[]){
        
        this->nTrans=nTrans;
        this->nPlaces=nPlaces;
        
        Trans=new struct InfTr[nTrans];
        VEq=new class Equation[nPlaces];
        Value=(long double*)malloc(sizeof(long double)*nPlaces);
        ValuePrv=(long double*)malloc(sizeof(long double)*nPlaces); 
        LBound=(double*)malloc(sizeof(double)*nPlaces);
        UBound=(double*)malloc(sizeof(double)*nPlaces); 
        SLBound=(double*)malloc(sizeof(double)*nPlaces);
        SUBound=(double*)malloc(sizeof(double)*nPlaces);
        EnabledTransValueCon=(long double*)malloc(sizeof(long double)*nTrans);
        EnabledTransValueDis=(long double*)malloc(sizeof(long double)*nTrans);
        
        time=0.0;
        int i=0;
        while(i<nTrans)
        {
            this->NameTrans.push_back(NameTrans[i]);
            ++i;
        }
        i=0;
        while(i<nPlaces)
        {
            this->NamePlaces.push_back(NamePlaces[i]);
            //cout<< SUBound[i]<<endl;
            SLBound[i]=epsilon;
            SUBound[i]=INT_MAX;
            ++i;
        }
        headDirc=headDerv=DEFAULT;
    }
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : Deconstruct*/
    /**************************************************************/  
    
    SystEq::~SystEq(){
        
        
        delete[] Trans;
        delete[] VEq;
        free(LBound);
        free(UBound);
        free(SLBound);
        free(SUBound);
        free(Value);
        free(ValuePrv);
        free(EnabledTransValueCon);
        free(EnabledTransValueDis);
        if (FinalValueXRun!=nullptr)
        {
            for(int i=0;i<nPlaces;i++)
                delete[] FinalValueXRun[i];
            delete[] FinalValueXRun;
        }
    }
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : It checks if it is disable by an inhibitor arc or due to the probabilistic step*/
    /**************************************************************/
    
    inline bool SystEq::NotEnable(int Tran){
        
        
        if (!Trans[Tran].enable) return true;
        
        if (Trans[Tran].InhPlaces.size()==0) return false;
        vector <InfPlace>::iterator it= Trans[Tran].InhPlaces.begin();
        while (it!=Trans[Tran].InhPlaces.end())
        {
            if (ValuePrv[it->Id]>=it->Card)
            {
                return true;
            }
            ++it;
        }
        return false;   
    }
    
    
    
    
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : resetTrans resets the set of enable transitions*/
    /**************************************************************/ 
    inline void  SystEq::resetTrans(){
        
        for (int i=0;i<nTrans;i++)
            Trans[i].enable=true;     
        
    }
    
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : fireEnalbeTrans decides if one among the enable transitions can fire*/
    /**************************************************************/ 
    
    int SystEq::fireEnableTrans(int SetTran[],double& h ){ 
        
        
        if (SetTran[0]!=0)
        {
            int size= SetTran[0];
            long double fireTime;
            
            long double minfireTime;	
            minfireTime= Trans[SetTran[1]].dist[0](generator)/EnabledTransValueDis[SetTran[1]];
            int minTr=SetTran[1];
            for (int i=2;i<=size;++i){
                fireTime=Trans[SetTran[i]].dist[0](generator)/EnabledTransValueDis[SetTran[i]];
                if (fireTime<minfireTime)
                {
                    minfireTime=fireTime;
                    minTr=(SetTran[i]);
                }
            } 
            //the selected transition will fire in the current time step
            if  (minfireTime<h)
            {
                h=minfireTime;
                Trans[minTr].enable=true;
                return minTr;
            }
        }
        return -1;
    }
    /* 
     * int SystEq::fireEnableTrans(set<int>&SetTran,double& h ){ 
     *    
     *    
     *    if (SetTran.size()!=0)
     *    {
     *    set< int>::iterator it=SetTran.begin();
     *    long double fireTime;
     *    
     *    long double minfireTime;	
     *    minfireTime= Trans[(*it)].dist[0](generator)/EnabledTransValueDis[*it];
     *    int minTr=(*it);
     *    ++it;
     *    while (it!=SetTran.end())
     *    {  
     *    fireTime=Trans[(*it)].dist[0](generator)/EnabledTransValueDis[*it];
     *    if (fireTime<minfireTime)
     *    {
     *    minfireTime=fireTime;
     *    minTr=(*it);
}
++it;
} 
//the selected transition will fire in the current time step
if  (minfireTime<h)
{
h=minfireTime;
Trans[minTr].enable=true;
return minTr;
}
}
return -1;
}
*/
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : setBNoiseTrans generates the brown noise value for all the transition involved  the diffusion process*/
    /**************************************************************/  
    inline void  SystEq::setBNoiseTrans(int diffh){
        
        
        for (int i=0;i<nTrans;i++)
        {
            Trans[i].BrownNoise=0.0;
        }
        
        for (int j=0;j<diffh;j++)
        {
            for (int i=0;i<nTrans;i++)
            {	  
                Trans[i].BrownNoise+=distribution(generator);
            }
        }
        for (int i=0;i<nTrans;i++)
        {
            Trans[i].BrownNoise= sqrt(Trans[i].rate*EnabledTransValueCon[i])*Trans[i].BrownNoise/sqrt(diffh);
        }  
    }
    
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : setBNoiseTrans generates the brown noise value for all the transition involved  the diffusion process*/
    /**************************************************************/  
    inline bool  SystEq::setBNoiseTrans(){
        bool enalbleTransitionInDiffusion=false;
        
        for (int i=0;i<nTrans;i++)
        {
            if ((Trans[i].enable)&&(EnabledTransValueCon[i]!=0.0)){
                long double inc=Trans[i].rate*EnabledTransValueCon[i];
                Trans[i].BrownNoise=sqrt(inc)*distribution(generator); 
                enalbleTransitionInDiffusion=true;
            }
        }
        return enalbleTransitionInDiffusion;
    }
    
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : ComputeODEstep computes the Euler step for ODE*/
    /**************************************************************/  
    
    inline void SystEq::ComputeHstep(double& h){
        long double max_drift=0.0;
        //*int id_t=-1;
        for (int i=0;i<nTrans;i++){
            if ((Trans[i].enable)&&(EnabledTransValueCon[i]!=0.0)){
                if (max_drift<Trans[i].rate) 
                    max_drift=Trans[i].rate;
            }
        }
        if (slow){
            max_drift=1/(max_drift*32);
        }
        else
            max_drift=1/(max_drift*16);
        while (h>=max_drift){
            h=h/2;
            
            //h=max_drift;
        }
    }
    
    inline void SystEq::ComputeHstep(const double& prv, const double& next, double& h ){
        
       // double perc_prv;
    /*
        if (slow)
            perc_prv= prv*perc2;
        else
            perc_prv= prv*perc1;
        if (perc_prv==0)
            perc_prv=perc2;
        h=perc_prv/next;
      */  
        
        if (slow)
            h= perc2/next;
        else
            h= perc1/next;
        
        // double tmp_next=fabs(prv-next);
        //cout<<endl<<prv<<" "<<perc_prv<<"  "<<tmp_next<<" "<<h<<endl;
        //while (perc_prv<tmp_next*h){
        // h=h/2;
        //}
        // cout<<"\tslow:"<<slow<<endl;
        //cout<<"\t"<<perc_prv<<"  "<<tmp_next<<" "<<h<<endl;
        
    }
    
    
    
    /**************************************************************/
    /* NAME :  Class SystEq*/
    /* DESCRIPTION : SolveHODEEuler solves the hybrid ODE system using Euler method*/
    /**************************************************************/  
    
    void  SystEq::SolveHODEEuler(double h,double perc1,double perc2,double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv){
        
        //assign factor for step
        this->fh=h;
        this->perc1=perc1;
        this->perc2=perc2;
        this->Max_Run=Max_Run;
        //For statistic
       // #ifdef AUTOMATON 
       // FinalValueXRun=new long double*[nPlaces+automaton.timers_num()];
       // #else
        FinalValueXRun=new long double*[nPlaces];
        //#endif
        for (int i=0;i<nPlaces;i++)
        {
            FinalValueXRun[i]=new long double[Max_Run+1];
            for (int j=0;j<Max_Run+1;j++)
                FinalValueXRun[i][j]=0.0;
        }
        
        #ifdef AUTOMATON
            long double** FinalValueTimerXRun=new long double*[automaton.timers_num()];
            for (int i=0;i<automaton.timers_num();i++)
            {
                FinalValueTimerXRun[i]=new long double[Max_Run+1];
                for (int j=0;j<Max_Run+1;j++)
                    FinalValueTimerXRun[i][j]=0.0;
            }
        #endif 
        
        cout.precision(12);
        cout<<endl<<"Seed value: "<<seed<<endl<<endl;
        bool SIM=false;
        if (h==MAXSTEP)
            SIM=true;
        
        int sizeMT=Max_Time/Print_Step+2;
        long double** MeanTrace=nullptr;
        if (Info){ //to Initialize the matrix storing trace values
            MeanTrace=new long double*[sizeMT];
            for (int i=0;i<sizeMT;i++){
                MeanTrace[i]=new long double[nPlaces];
                for (int j=0;j<nPlaces;j++){
                    MeanTrace[i][j]=0.0;
                }
            }
        }//to Initialize the matrix storing trace values
        
        
        this->Max_Run=Max_Run;
        int Dsteps=0,Ssteps=0;
        
        
        int run=0;
        
        long double* ValueInit=(long double*)malloc(sizeof(long double)*nPlaces);
      //  #ifdef AUTOMATON 
       // long double* Mean=(long double*)malloc(sizeof(long double)*(nPlaces+automaton.timers_num()));
       // #else
        long double* Mean=(long double*)malloc(sizeof(long double)*nPlaces);
        //#endif
        //header for output file
        
        //It is used to maintain the set of transitions for the simulation step.
        int SetTran[nTrans+1];
        
        //cerr<<"c(";
        for (int i=0;i<nPlaces;i++)//save initial state
        {
            ValueInit[i]=Value[i];
            Mean[i]=0.0;
        } 
        
        #ifdef AUTOMATON
        int tot_accepted_traces=0;
        
        #endif  	
        int trace_step;
        while(run<Max_Run){
            if (run%100==0){
                cout<<"\r\t START RUN..."<<run<<" ";
                cout.flush();
            }
            time=0;
            double NextPrintTime=0.0;
            trace_step=0;
            for (int i=0;(i<nPlaces);i++)//return to the initial state
            {
                ValuePrv[i]=Value[i]=ValueInit[i];
            }
            
            
            #ifdef AUTOMATON
            automaton.reset();//reset automaton state for each new run
            bool accepted_trace=true;
            next_clock_automaton=Max_Time;
            bool reached=false;
            //cout<<"*****************************"<<endl;
            while ((accepted_trace=automaton.syncr())&&(time<Max_Time))
            { 
                
                #else
                while (time<Max_Time)
                { 
                    #endif
                    
                    for (int i=0;i<nPlaces;i++)
                    {//save previous step
                        // ValuePrv[i]=Value[i];
                        if (Info){
                            if ((!SIM)&&((NextPrintTime<time+epsilon1)||(NextPrintTime==0)))
                                MeanTrace[trace_step][i]+=Value[i];
                            else
                                if ((SIM)&&((NextPrintTime<time+epsilon1)||(NextPrintTime==0))){
                                    MeanTrace[trace_step][i]+=Value[i];
                                }
                        }
                        
                        ValuePrv[i]=Value[i];
                    }
                    if (Info){
                        if ((((NextPrintTime<time+epsilon1)))||(NextPrintTime==0))
                        {
                            trace_step++;
                            NextPrintTime+=Print_Step;
                        }
                    }    
                        
                        //compute transition enable value
                        getValTranFire();
                    
                    SetTran[0]=0;
                    for(int t=0; t<nTrans; t++){
                        if (!NotEnable(t)){
                            bool inserted=false;
                            for (auto it=Trans[t].InOuPlaces.begin();(it!=Trans[t].InOuPlaces.end())&&(!inserted);++it){
                                //case bound
                                int i=*it;
                                if ((Value[i]<=LBound[i] || Value[i]<=SLBound[i])||(Value[i]>=UBound[i] || Value[i]>=SUBound[i])){ 
                                    
                                    if (EnabledTransValueDis[t]>0){
                                        SetTran[++SetTran[0]]=t;
                                        inserted=true;
                                    }
                                    Trans[t].enable=false; 
                                }
                            }
                        }
                        
                    }
                    
                    
                    int t=-1;
                    
                    //reset time step
                    #if ADAPTATIVE	
                    h=MAXSTEP;
                    ComputeHstep(h);
                    #else
                    h=fh;
                    #endif	
                    
                    if (time+h>Max_Time)
                        h=Max_Time-time;
                    if ((Info)&&(time+h>NextPrintTime)){
                        h=NextPrintTime-time;
                    }
                    
                    
                    
                    if (SetTran[0]!=0)
                    {
                        Dsteps++;
                        t=fireEnableTrans(SetTran,h);  
                    }
                    
                    #ifdef AUTOMATON
                    if  ((time<next_clock_automaton)&&(time+h>next_clock_automaton)){
                        h=next_clock_automaton-time;
                        if (t!=-1){//reset transition since the step is smaller due to automaton
                            Trans[t].enable=false;
                            t=-1;
                        }
                    }
                    next_clock_automaton=Max_Time;
                    #endif	  
                    Ssteps++;
                    //select if there is an enable transition. it is identifier is saved in t. The time step is updated coherently
                    unsigned int i=headDirc;
                    while (i!=DEFAULT)// for all places
                    {
                        long double tmpvalD=0.0, tmpvalSIM=0.0;
                        for (int j=0; j<VEq[i].getSize();j++)// for all components
                        {
                            
                            if (!NotEnable(VEq[i].getIdTrans(j)))//if transition is not disable by inhibitor arc or due to probabilistic step
                            { 
                                if (t!=VEq[i].getIdTrans(j)) 
                                {// it is not the transition selected by fireEnalbeTrans  
                                    tmpvalD+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*EnabledTransValueCon[VEq[i].getIdTrans(j)];
                                }
                                else
                                {//it is the   transition selected by fireEnalbeTrans
                                    tmpvalSIM+= VEq[i].getIncDec(j);  
                                }
                            }
                        }// for all components
                        Value[i]=ValuePrv[i]+tmpvalD*h+tmpvalSIM;
                        i=VEq[i].getNext(); 
                    }
                    
                    derived();// it derives all the rest of places 
                    
                    #ifndef AUTOMATON     
                    if ((!Info)&&(h==MAXSTEP)) //when no transition are enable during SIMULATION
                        h=Print_Step;
                    #endif      
                    time+=h;
                    #ifdef AUTOMATON
                    automaton.inc_timer_variable(h);
                    #endif
                    //reset SetTran and transitions not enable
                    resetTrans();
 #if AUTOMATON                           
                    if (automaton.is_current_node_final_node()&& (reached==false)){
                            reached=true;
                            for (int i=0;i<automaton.timers_num();i++){
                                FinalValueTimerXRun[i][tot_accepted_traces]=automaton.get_timer_variable(i);
                            }
                            
                        }
#endif
                    
#if DEBUG && AUTOMATON
                       cout<<"\nTime:"<<time<<" Automaton State:"<<automaton.get_current_state()<< "\n\tTimers var:"<<endl;
                       automaton.printTimers(cout); 
                       cout<<endl; 
                       for (auto i=0;i<nPlaces;i++)
                           cout<<NamePlaces[i]<<":"<<Value[i]<<" ";
                       cout<<endl;
#endif                
                       
                }
                
                
                #ifdef AUTOMATON
                if ((accepted_trace)&&(automaton.is_current_node_final_node())){
                    tot_accepted_traces++;
                    #endif    
                    
                    
                    //Print final time for each trace
                    if ((Info)){
                        for (int i=0;i<nPlaces;i++){//save previous step
                            MeanTrace[trace_step][i]+=Value[i];
                        }
                    }
                    //Print final time for each trace    
                    
                    
                    for (int i=0;i<nPlaces;i++)//save initial state
                    {
                        #ifdef AUTOMATON
                        Mean[i]+= FinalValueXRun[i][tot_accepted_traces]= Value[i];
                        #else
                        Mean[i]+= FinalValueXRun[i][run]= Value[i];
                        #endif
                    }
                    #ifdef AUTOMATON
                }
                #endif    
                run++;
            }
            
            if ((Info)){
                ofstream out(string(argv)+".trace",ofstream::out);
                out.precision(12);
                if(!out){
                    throw Exception("*****Error opening output file *****\n\n");         
                }
                out<<"Time";
                for (int j=0;j<nPlaces;j++){
                    out<<" "<<NamePlaces[j];
                }
                out<<endl;
                for (int i=0;i<sizeMT&&(Print_Step*i<=Max_Time);i++){
                    out<<Print_Step*i;
                    for (int j=0;j<nPlaces;j++){
                        #ifdef AUTOMATON
                        out<<" "<<MeanTrace[i][j]/tot_accepted_traces;
                        #else
                        out<<" "<<MeanTrace[i][j]/run;
                        #endif
                    }
                    out<<endl;
                }
                out.close();
                for (int i=0;i<sizeMT;i++){
                    delete[] MeanTrace[i];
                }
                delete[] MeanTrace;
            }
            
            
            
            cout.precision(12);
            cout<<"\nMean at time "<<time<<":\n";
            if(time){
                for (int i=0;i<nPlaces;i++)//save initial state
                {
                    #ifdef AUTOMATON
                    cout<<"\t"<<NamePlaces[i]<<": ~"<<Mean[i]/tot_accepted_traces<<"\n";
                    #else
                    cout<<"\t"<<NamePlaces[i]<<": ~"<<Mean[i]/run<<"\n";
                    #endif 
                }
            }
                cout<<endl; 
            cout<<"\nTotal steps: "<<Ssteps<<",  simulation steps: "<<Dsteps<<"\n";
            
            #ifdef AUTOMATON
            
            double l = binomlow(Max_Run,tot_accepted_traces, (1-ALPHA)/2);
            double u = binomup(Max_Run,tot_accepted_traces, (1-ALPHA)/2);
            cout<<"\nProbability to accept the automaton paths at time "<<Max_Time<< " : ["<<l<<","<<u<<"] ("<<double(tot_accepted_traces)/double(Max_Run)<<")"<<endl;
            ofstream out(string(argv)+".prob_auto",ofstream::out);
            cout<<string(argv)<<".prob_auto"<<endl;
            if(!out){
                throw Exception("*****Error opening output file *****\n\n");         
            }
            out.precision(12);
            out<<double(tot_accepted_traces)/double(Max_Run)<<"\t"<<l<<"\t"<<u<<"\t"<<seed<<endl;
            out.close();
            if (automaton.timers_num()>0){    
                ofstream out(string(argv)+".timer_auto",ofstream::out);
                cout<<string(argv)<<".timer_auto"<<endl;
                if(!out){
                  throw Exception("*****Error opening output file *****\n\n");    
                }
                out.precision(12);
                auto num_timers=automaton.timers_num();
                for (auto j=0;j<tot_accepted_traces;++j){
                    for (auto i=0;i<num_timers;++i){
                        out<<FinalValueTimerXRun[i][j]<<" ";
                    }
                    out<<endl;
                }
                out.close();
            } 
            
            for (int i=0;i<automaton.timers_num();i++){
                free(FinalValueTimerXRun[i]);
            }
            free(FinalValueTimerXRun);
            #endif 
            
            free(Mean);
            free(ValueInit);
        }
        
        
        
        
        /**************************************************************/
        /* NAME :  Class SystEq*/
        /* DESCRIPTION : SolveSDEEuler solves the SDE system using Euler method*/
        /**************************************************************/  
        
        void  SystEq::SolveSDEEuler(double h,double perc1,double perc2,double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv){
            
            //assign factor for step
            this->fh=h;
            this->perc1=perc1;
            this->perc2=perc2;
            this->Max_Run=Max_Run;
            //For statistic
            FinalValueXRun=new long double*[nPlaces];
            for (int i=0;i<nPlaces;i++)
            {
                FinalValueXRun[i]=new long double[Max_Run+1];
                for (int j=0;j<Max_Run+1;j++)
                    FinalValueXRun[i][j]=0.0;
            }
            
#ifdef AUTOMATON
            long double** FinalValueTimerXRun=new long double*[automaton.timers_num()];
            for (int i=0;i<automaton.timers_num();i++)
            {
                FinalValueTimerXRun[i]=new long double[Max_Run+1];
                for (int j=0;j<Max_Run+1;j++)
                    FinalValueTimerXRun[i][j]=0.0;
            }
#endif            
            
            cout.precision(12);
            cout<<endl<<"Seed value: "<<seed<<endl<<endl;
            
            int sizeMT=Max_Time/Print_Step+2;
            long double** MeanTrace=nullptr;
            if (Info){ //to Initialize the matrix storing trace values
                MeanTrace=new long double*[sizeMT];
                for (int i=0;i<sizeMT;i++){
                    MeanTrace[i]=new long double[nPlaces];
                    for (int j=0;j<nPlaces;j++){
                        MeanTrace[i][j]=0.0;
                    }
                }
            }//to Initialize the matrix storing trace values
            
            long double tmpvalD[nPlaces],tmpvalS[nPlaces], tmpvalSIM[nPlaces]; 
        
                
            //It is used to maintain the set of transitions for the simulation step.
            int SetTran[nTrans+1];
            
            
            this->Max_Run=Max_Run;
            unsigned long Dsteps=0,Ssteps=0;
            
            std::uniform_real_distribution<double> uniform(0.0,1.0);
            int run=0;
            
            long double* ValueInit=(long double*)malloc(sizeof(long double)*nPlaces);
            long double* Mean=(long double*)malloc(sizeof(long double)*nPlaces);
            //header for output file
            
            
            for (int i=0;i<nPlaces;i++)//save initial state
            {
                ValueInit[i]=Value[i];
                Mean[i]=0.0;
            } 
            
            #ifdef AUTOMATON
            int tot_accepted_traces=0;
            #endif 
            
            int trace_step;
            while(run<Max_Run){
                if (run%100==0){
                    cout<<"\r\t START RUN..."<<run<<" ";
                    cout.flush();
                }
                time=0;
                double NextPrintTime=0.0;
                trace_step=0;
                for (int i=0;(i<nPlaces);i++)//return to the initial state
                {
                    Value[i]=ValueInit[i];
                }
                //bool steady_state;
                #ifdef AUTOMATON
                automaton.reset();//reset automaton state for each new run
                bool accepted_trace=true;
                next_clock_automaton=Max_Time;
                h=this->fh;
                
                bool reached=false;
                while ((accepted_trace=automaton.syncr() ) &&(time<Max_Time))
                { 
                    #else
                    while (time<Max_Time)
                    { 
                        #endif	
                        for (int i=0;i<nPlaces;i++)
                        {//save previous step
                            ValuePrv[i]=Value[i];
                        if ((Info)&&((NextPrintTime<=time+epsilon)||(NextPrintTime==0))){
                                
                                MeanTrace[trace_step][i]+=Value[i];
                            }
                            
                        }
                        if ((Info)&&((NextPrintTime<=time+epsilon)||(NextPrintTime==0)))
                        {
                            trace_step++;
                            NextPrintTime+=Print_Step;//cout<<endl;
                        }
                        
                        
                        //compute transition enable value
                        getValTranFire();
                        SetTran[0]=0;
                        //compute transition enable for simulation
                        for(int t=0; t<nTrans; t++){
                            if (!NotEnable(t)){
                                bool inserted=false;
                                for (auto it=Trans[t].InOuPlaces.begin();(it!=Trans[t].InOuPlaces.end())&&(!inserted);++it){
                                    //case bound
                                    int i=*it;
                                    if ((Value[i]<=LBound[i] || Value[i]<=SLBound[i])||(Value[i]>=UBound[i] || Value[i]>=SUBound[i])){ 
                                        
                                        if (EnabledTransValueDis[t]>0){
                                            SetTran[++SetTran[0]]=t;
                                            inserted=true;
                                        }
                                        Trans[t].enable=false; 
                                    }
                                }
                            }
                            
                        }
                        //compute transition enable for simulation
                        
                        int t=-1;
                        double tfiring=MAXSTEP;//
                        if (SetTran[0]!=0){
                            Dsteps++;
                            t=fireEnableTrans(SetTran,tfiring);// in this way we return the transition with smallest delay, tfiring stores its delay
                        } 
                        Ssteps++;
                        
                        
                        unsigned int i=headDirc; 
                        unsigned int  ind_maxvalD=0;
                        long double maxvalD=0.0;
                        while (i!=DEFAULT)// for all places
                        {
                            tmpvalD[i]=tmpvalS[i]= tmpvalSIM[i]=0.0;
                            
                            int size =  VEq[i].getSize();
                            for (int j=0; j<size;j++)// for all components
                            {
                                int IdTran=VEq[i].getIdTrans(j); 
                                if (!NotEnable(IdTran))//if transition is not disable by inhibitor arc or due to probabilistic step
                                {   
                                    if (t!=IdTran) 
                                    {// it is not the transition selected by fireEnalbeTrans  
                                        tmpvalD[i]+=VEq[i].getIncDec(j)*Trans[IdTran].rate*EnabledTransValueCon[IdTran];
                                        tmpvalS[i]+=VEq[i].getIncDec(j)*getBNoiseTran(IdTran);
                                    }
                                    else
                                    {//it is the   transition selected by fireEnalbeTrans
                                        tmpvalSIM[i]+= VEq[i].getIncDec(j);  
                                    }
                                }
                            }// for all components
                            //to compute the most important variation
                            long double increment=fabs(ValuePrv[i]-tmpvalD[i])/(ValuePrv[i]==0?1:ValuePrv[i]);
                            if (increment>maxvalD) {
                                ind_maxvalD=i;
                                maxvalD=increment;
                            }
                            //to compute the most important variation
                            i=VEq[i].getNext();
                        }
                        //computing step
                        h=fh;
                        
                        if ((!setBNoiseTrans())){
                            h=Max_Time-time;// no transitions enable in the diffusion process 
                            //so we can extend the step
                        }
                        else{
                            #if ADAPTATIVE		  
                            ComputeHstep(ValuePrv[ind_maxvalD],maxvalD,h);
                            #endif	
                            //  cout<<"h:"<<h<<endl;
                        }

                        //to check if time+h is greater than the simulation time
                        if (time+h>Max_Time){
                            h=Max_Time-time;
                        }
                   //     if (h>fh)
                    //        h=fh;
                        //cout<<time<<" "<<h<<endl;
                        //to check if time+h is greater than the simulation time
                        if ((Info)&&(time+h>NextPrintTime)){
                            h=NextPrintTime-time;
                        }
                        
                        #ifdef AUTOMATON	
                        //to check if time+h is greater than the next asynchronous automaton event
                        if ((time<next_clock_automaton)&&(time+h>next_clock_automaton)){
                            h=next_clock_automaton-time;
                            
                        }
                        next_clock_automaton=Max_Time;
                        #endif	
                        
                        //check if h is lower than tfiring
                        if (h>tfiring)
                            h=tfiring;
                        //check if h is lower than tfiring
                        
                        i=headDirc;
                        long double sqrth=sqrt(h);
                        double sum=0;
                        slow=false;
                        while (i!=DEFAULT){// for all places
                            Value[i]=ValuePrv[i]+tmpvalD[i]*h+tmpvalS[i]*sqrth+(tfiring>h?0.0L:tmpvalSIM[i]);
                            
                            if ((!slow)&&(Value[i]!=LBound[i])&&(Value[i]!=UBound[i])){	
                                if (fabs(Value[i]-LBound[i])<ep)//(Value[i]-ep<LBound[i])
                                {
                                    //  cout<<ValuePrv[i]<<" "<<Value[i]<<" "<<i<<endl;
                                    slow=true;
                                }  
                                if (fabs(Value[i]-UBound[i])<ep)//(Value[i]+ep>UBound[i])
                                { 
                                    // cout<<Value[i]<<" "<<i<<endl;
                                    slow=true;
                                }
                            }
                            
                            if (Value[i]<LBound[i])
                            {
                                //cout<<Value[i]<<endl;
                                Value[i]=LBound[i]; 
                            }
                            else
                            {
                                if (Value[i]>UBound[i]){
                                    // cout<<Value[i]<<endl;
                                    Value[i]=UBound[i];  
                                }
                            }    
                                
                                sum+=Value[i]; 
                            i=VEq[i].getNext();	  
                        }
                       
                     //   if ((!slow)&&(fabs(Value[3]-50)<ep || fabs(Value[3]-100000)<ep)){
                       //     slow=true;
                       // }
                        
                            
                        derived();// it derives all the rest of places   
                        if ((!Info)&&(h==MAXSTEP)){ //when no transition are enable during SIMULATION
                            h=Print_Step;
                        }
                        
      
                        
                        time+=h;
                        #ifdef AUTOMATON
                        automaton.inc_timer_variable(h);
                        if (automaton.is_current_node_final_node()&& (reached==false)){
                            reached=true;
                            for (int i=0;i<automaton.timers_num();i++){
                                FinalValueTimerXRun[i][tot_accepted_traces]=automaton.get_timer_variable(i);
                            }
                            
                        }
                        if (automaton.is_current_node_final_node())
                           time=Max_Time+1;
                        #endif
                        //reset SetTran and transitions not enable
                        resetTrans();   
#if DEBUG && AUTOMATON
                       cout<<"\nTime:"<<time<<" Automaton State:"<<automaton.get_current_state()<< "\n\tTimers var:"<<endl;
                       automaton.printTimers(cout); 
                       cout<<endl;
                       this->PrintValue(cout);
                       cout<<endl;
#endif
                       
                    }
                    #ifdef AUTOMATON
                    if ((accepted_trace)&&(automaton.is_current_node_final_node())){
                        tot_accepted_traces++;
                        #endif
                        
                        //Print final time for each trace
                        if ((Info)){
                            for (int i=0;i<nPlaces;i++){//save previous step
                                MeanTrace[trace_step][i]+=Value[i];
                            }
                        }
                        
                        //Print final time for each trace 
                        
                        for (int i=0;i<nPlaces;i++)//save initial state
                        {
                            #ifdef AUTOMATON
                            Mean[i]+= FinalValueXRun[i][tot_accepted_traces]= Value[i];
                            #else 
                            Mean[i]+= FinalValueXRun[i][run]= Value[i];
                            #endif        
                        }
                        #ifdef AUTOMATON
                    }
                    #endif 
                    run++;
                }
                
                if ((Info)){
                    ofstream out(string(argv)+".trace",ofstream::out);
                    out.precision(12);
                    if(!out){
                        throw Exception("*****Error opening output file *****\n\n");         
                    }
                    
                    //Header file
                    out<<"Time";
                    for (int j=0;j<nPlaces;j++){
                        out<<" "<<NamePlaces[j]<<" ";
                    }
                    out<<endl;
                    
                    
                    for (int i=0;i<sizeMT&&Print_Step*i<=Max_Time;i++){
                        out<<Print_Step*i;
                        for (int j=0;j<nPlaces;j++){
                             #ifdef AUTOMATON
                             out<<" "<<MeanTrace[i][j]/tot_accepted_traces;
                             #else
                             out<<" "<<MeanTrace[i][j]/run;
                             #endif
                        }
                        out<<endl;
                    }
                    out.close();
                    for (int i=0;i<sizeMT;i++){
                        delete[] MeanTrace[i];
                    }
                    delete[] MeanTrace;
                }
                
                
                cout<<"\nMean at time "<<time<<":\n";
                if(time)
                {
                    for (int i=0;i<nPlaces;i++)//save initial state
                    {
                        #ifdef AUTOMATON
                        cout<<"\t"<<NamePlaces[i]<<": ~"<<Mean[i]/tot_accepted_traces<<"\n";
                        #else
                        cout<<"\t"<<NamePlaces[i]<<": ~"<<Mean[i]/run<<"\n";
                        #endif      
                    }
                }
                    cout<<endl; 
                cout<<"\nTotal steps: "<<Ssteps<<",  simulation steps: "<<Dsteps<<"\n";
                
                #ifdef AUTOMATON
                cout<<"Accepted traces:"<<tot_accepted_traces<<endl;
                double l = binomlow(Max_Run,tot_accepted_traces, (1-ALPHA)/2);
                double u = binomup(Max_Run,tot_accepted_traces, (1-ALPHA)/2);
                cout<<"\nProbability to accept the automaton paths at time "<<Max_Time<< " : ["<<l<<","<<u<<"] ("<<double(tot_accepted_traces)/double(Max_Run)<<")"<<endl;
                ofstream out(string(argv)+".prob_auto",ofstream::out);
                cout<<string(argv)<<".prob_auto"<<endl;
                if(!out){
                        throw Exception("*****Error opening output file *****\n\n");         
                }
                out.precision(12);
                out<<double(tot_accepted_traces)/double(Max_Run)<<"\t"<<l<<"\t"<<u<<"\t"<<seed<<endl;
                out.close(); 
                
                if (automaton.timers_num()>0){    
                    ofstream out(string(argv)+".timer_auto",ofstream::out);
                    cout<<string(argv)<<".timer_auto"<<endl;
                    if(!out){
                        throw Exception("*****Error opening output file *****\n\n");    
                    }
                    out.precision(12);
                    auto num_timers=automaton.timers_num();
                    for (auto j=0;j<tot_accepted_traces;++j){
                        for (auto i=0;i<num_timers;++i){
                            out<<FinalValueTimerXRun[i][j]<<" ";
                        }
                        out<<endl;
                    }
                    out.close();
                }  
                for (int i=0;i<automaton.timers_num();i++){
                    free(FinalValueTimerXRun[i]);
                }
                free(FinalValueTimerXRun);
                #endif 
                
                free(Mean);
                free(ValueInit);
            }
            
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : It derives the place which are not directly computed*/
            /**************************************************************/  
            
            void  SystEq::derived(){
                
                unsigned int i=headDerv;
                while (i!=DEFAULT)
                {
                    double value =0.0, coff=0.0;
                    int place=-1;
                    for (int j=0;j<VEq[i].getSizeP()-1;j++)
                    {
                        VEq[i].getPsemflw(j,place,coff);
                        value+=Value[place]*coff;
                    }
                    VEq[i].getPsemflw((VEq[i].getSizeP()-1),place,coff);
                    Value[i] = coff - value;
                    if (Value[i]<0)
                    {//normalization
                        Value[i]=0;
                        for (int j=0;j<VEq[i].getSizeP()-1;j++)
                        {
                            VEq[i].getPsemflw(j,place);
                            Value[place]= Value[place]/value*coff;
                        } 
                    }//normalization
                    i=VEq[i].getNext(); 
                }
                
            }
            
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : It derives the place which are not directly computed 
             *               from input vector Value*/
            /**************************************************************/  
            
            void  SystEq::derived(long double* Value){
                
                unsigned int i=headDerv;
                while (i!=DEFAULT)
                {
                    double value =0.0, coff=0.0;
                    int place=-1;
                    for (int j=0;j<VEq[i].getSizeP()-1;j++)
                    {
                        VEq[i].getPsemflw(j,place,coff);
                        value+=Value[place]*coff;
                    }
                    VEq[i].getPsemflw((VEq[i].getSizeP()-1),place,coff);
                    Value[i] = coff - value;
                    
                    if (Value[i]<0)
                    {//normalization
                        Value[i]=0;
                        for (int j=0;j<VEq[i].getSizeP()-1;j++)
                        {
                            VEq[i].getPsemflw(j,place);
                            Value[place]= Value[place]/value*coff;
                        } 
                    }//normalization
                    i=VEq[i].getNext(); 
                }
                
            }
            
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : It solves the ODE system using Euler method*/
            /**************************************************************/  
            
            void  SystEq::SolveODEEuler(double h,double perc1,double perc2,double Max_Time,bool Info,double Print_Step,char *argv){

                //assign factor for step
                long double tmpval[nPlaces]; 
                this->fh=h;
                this->perc1=perc1;
                this->perc2=perc2;
                this->Max_Run=Max_Run;
                //For statistic
                FinalValueXRun=new long double*[nPlaces];
                for (int i=0;i<nPlaces;i++)
                {
                    FinalValueXRun[i]=new long double[Max_Run+1];
                    for (int j=0;j<Max_Run+1;j++)
                        FinalValueXRun[i][j]=0.0;
                }
                
                
                double NextPrintTime=0.0;
                ofstream out;
                
                if (Info)
                {
                    out.open(string(argv)+".trace",ofstream::out);
                    out.precision(12);
                    if(!out) 
                    {
                        throw Exception("*****Error opening output file *****\n\n");  
                        
                    }
                    out<<"Time";
                    for (int i=0;i<nPlaces;i++)
                        out<<" "<<NamePlaces[i];
                    out<<endl;
                } 
                cout.precision(16);
                while (time<Max_Time)
                { 
                    //Initialize current step
                    if (((NextPrintTime<time+epsilon)||(NextPrintTime==0))){
                        if (Info)
                            out<<time;
                        cout<<"Time:"<<time<<"(Step: "<<Print_Step<<")"<<endl;
                    }       
                    
                    for (int i=0; i<nPlaces; i++)
                    {
                        if ((Info) &&((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                            out<<" "<<Value[i];
                        ValuePrv[i]=Value[i];
                    }
                    if (((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                    {
                        if (Info)
                            out<<endl;
                        NextPrintTime+=Print_Step;
                        
                        
                    }
                    //compute transition enable value
                    getValTranFire();
                    #if ADAPTATIVE	
                    h=MAXSTEP;
                    ComputeHstep(h);
                    #else
                    h=fh;
                    #endif	
                    
                    if (time+h>Max_Time)
                    {
                        h=Max_Time-time;
                    }
                    else
                    {
                        if ((Info)&&(time+h>NextPrintTime)){
                            h=NextPrintTime-time;
                        }
                    }    
                        //Aggiunto per fare dei test!!!!   
                    unsigned int i=headDirc;
                    unsigned int  ind_maxvalD=0;
                    long double maxvalD=0.0;
                    while (i!=DEFAULT)
                    {
                        tmpval[i]=0.0;
                        for (int j=0; j<VEq[i].getSize();j++)// for all components
                        {
                            long double val=0.0;
                            if ((!NotEnable(VEq[i].getIdTrans(j)))&&((val=EnabledTransValueCon[VEq[i].getIdTrans(j)])>0))//if transition is not disable by inhibitor arc 
                            { 
                                tmpval[i]+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*val;
                            }
                        }
                        
                        long double increment=fabs(ValuePrv[i]-tmpval[i]);
                        if (increment>maxvalD) {
                            ind_maxvalD=i;
                            maxvalD=increment;
                        }
                        i=VEq[i].getNext();//next place that is computed directly
                    }
                    
                    #if ADAPTATIVE		  
                    ComputeHstep(ValuePrv[ind_maxvalD],maxvalD,h);
                    #endif	
                    
                    i=headDirc;
                    while (i!=DEFAULT)
                    {
                        Value[i]=ValuePrv[i]+tmpval[i]*h;
                        if ( Value[i]<0){
                            cout<<"Value "<<i<<" "<<Value[i]<< " Prv:"<<ValuePrv[i]<<" Var:"<<tmpval[i]<<" h:"<<h<<" inc"<<ind_maxvalD<<endl;
                         throw   Exception("*****Error negative value, try to reduce the step *****\n\n");   
                        }
                        i=VEq[i].getNext();//next place that is computed directly
                    }
                    derived();// it derives all the rest of places
                    time+=h;
                }
                
                
                
                for (int i=0;i<nPlaces;i++)//store resul ode
                {
                    FinalValueXRun[i][0]= Value[i];
                }
                
                //Print final time for each trace
                if ((Info)&&((NextPrintTime<=time)||(NextPrintTime==0)))
                {
                    out<<time;
                    for (int i=0;i<nPlaces;i++){//save previous step
                        out<<" "<<Value[i];
                    }
                    out<<endl; 
                }
                
                
                
                //Print final time for each trace  
                cout<<"Solution at time "<<time<<":\n";
                for (int i=0;i<nPlaces;i++)//save initial state
                {
                    cout<<"\t"<<NamePlaces[i]<<": "<<Value[i]<<"\n";
                }
                cout<<endl;  
            }
            
            
            
            
               
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : It solves the ODE system using Euler method*/
            /**************************************************************/  
            
            void  SystEq::SolveODERK5(double h, double perc1,double Max_Time,bool Info,double Print_Step,char *argv){

 
                //k1
                long double k1[nPlaces]; 
                //k2
                long double k2[nPlaces];
                //k3
                long double k3[nPlaces];
                //k4
                long double k4[nPlaces];
                //k5
                long double k5[nPlaces];
                //k6
                long double k6[nPlaces];
          
                const long double C3d32 = 3.0/32.0;
                const long double C9d32 = 9.0/32.0;
                const long double C1932d2197 =1932.0/2197.0;
                const long double C7200d2197 =7200.0/2197.0;
                const long double C7296d2197 =7296.0/2197.0;
                const long double C439d216 =439.0/216.0;
                const long double C3680d513 =3680.0/513.0;
                const long double C845d4104 =845.0/4104.0;
                const long double C8d27	=8.0/27.0;
                const long double C3544d2565 =3544.0/2565.0;
                const long double C1859d4104 =1859.0/4104.0;
                const long double C11d40 =11.0/40.0;
                const long double C2d55 =2.0/55.0;
                const long double C25d216= 25.0/216.0;
                const long double C1408d2565= 1408.0/2565.0;
                const long double C2197d4104=2197.0/4104.0;
                const long double C1d36= 1.0/36.0;
                const long double C128d4275= 128.0/4275.0;
				const long double C2197d75240= 2197.0/75240.0; 
				const long double C1d50=1/50;
                
                this->fh=h;
                this->Max_Run=Max_Run;
                //For statistic
                FinalValueXRun=new long double*[nPlaces];
                for (int i=0;i<nPlaces;i++)
                {
                    FinalValueXRun[i]=new long double[Max_Run+1];
                    for (int j=0;j<Max_Run+1;j++)
                        FinalValueXRun[i][j]=0.0;
                }
                
                
                double NextPrintTime=0.0;
                ofstream out;
                
                if (Info)
                {
                    out.open(string(argv)+".trace",ofstream::out);
                    out.precision(12);
                    if(!out) 
                    {
                        throw Exception("*****Error opening output file *****\n\n");  
                        
                    }
                     out<<"Time";
                    for (int i=1;i<nPlaces;i++)
                        out<<" "<<NamePlaces[i];
                    out<<endl;
                } 
                cout.precision(16);
                
            
               if (Info){
                   
                    out<<"Time ";
                    for (int i=0;i<nPlaces;i++)
                        out<<" "<<NamePlaces[i];
                    out<<endl;
                    out<<time;
               }
               cout<<"Time:"<<time<<"(Step: "<<Print_Step<<")"<<endl;
               for (int i=0; i<nPlaces; i++)
                    {
                        if (Info)
                            out<<" "<<Value[i];
                        ValuePrv[i]=Value[i];
                    }
                if (Info)
                            out<<endl;    
                NextPrintTime+=Print_Step;
                int current_attempt=0; 
                while (time<Max_Time)
                { 
              /*      //Initialize current step
                    if (((NextPrintTime<time+epsilon)||(NextPrintTime==0))){
                        if (Info)
                            out<<time;
                        cout<<"Time:"<<time<<"(Step: "<<Print_Step<<")"<<endl;
                    }       
                    
                    for (int i=0; i<nPlaces; i++)
                    {
                        if ((Info) &&((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                            out<<" "<<Value[i];
                        ValuePrv[i]=Value[i];
                    }
                    if (((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                    {
                        if (Info)
                            out<<endl;
                        NextPrintTime+=Print_Step;
                        
                        
                    }
                */    
                    //cout<<"Time: "<<time<<"  h:"<<h<<endl;
                    StepODERK5(ValuePrv,k1,h);
                    unsigned int i=headDirc;
                    while (i!=DEFAULT)
                    {
                        Value[i]=ValuePrv[i]+(k1[i]/4.0);
                        i=VEq[i].getNext();//next place that is computed directly
                    }
                    derived();// it derives all the rest of places
                    
                    
                    
                    
                    StepODERK5(Value,k2,h);
                    i=headDirc;
                    while (i!=DEFAULT)
                    {
                        Value[i]=ValuePrv[i]+(k1[i]*C3d32+k2[i]*C9d32);
                        i=VEq[i].getNext();//next place that is computed directly
                    }
                    derived();// it derives all the rest of places
                    
                    StepODERK5(Value,k3,h);
                    i=headDirc;
                    while (i!=DEFAULT)
                    {
                        Value[i]=ValuePrv[i]+(k1[i]*C1932d2197-k2[i]*C7200d2197+k3[i]*C7296d2197);
                        i=VEq[i].getNext();//next place that is computed directly
                    }
                    derived();// it derives all the rest of places
                    
                    StepODERK5(Value,k4,h);
                    i=headDirc;
                    while (i!=DEFAULT)
                    {
                        Value[i]=ValuePrv[i]+(k1[i]*C439d216-k2[i]*8+k3[i]*C3680d513-k4[i]*C845d4104);
                        i=VEq[i].getNext();//next place that is computed directly
                    }
                    derived();// it derives all the rest of places
                    
                    
                    StepODERK5(Value,k5,h);
                   
                    i=headDirc;
                    while (i!=DEFAULT)
                    {
                    	Value[i]=ValuePrv[i]+(-k1[i]*C8d27+k2[i]*2-k3[i]*C3544d2565+k4[i]*C1859d4104-k5[i]*C11d40);
                    	i=VEq[i].getNext();//next place that is computed directly
                    }
                    derived();// it derives all the rest of places
                                     
                    StepODERK5(Value,k6,h);
                    
                    long double error=0.0,tmp_error_ass;
                    i=headDirc;
                    while (i!=DEFAULT)
                    {
                        tmp_error_ass=fabs(k1[i]*C1d36-k3[i]*C128d4275-k4[i]*C2197d75240+k5[i]*C1d50+k6[i]*C2d55);
                        Value[i]=ValuePrv[i]+(k1[i]*C25d216+k3[i]*C1408d2565+k4[i]* C2197d4104-k5[i]/5.0); 
                        //ValueA1[i]=ValuePrv[i]+(k1[i]*16.0/135.0+k3[i]*6656.0/12825.0+k4[i]*28561.0/56430.0-k5[i]*9.0/50.0+k6[i]*2.0/55.0);
						//tmp_error_ass= fabs(Value[i]-ValueA1[i]);		
                        error = (error > tmp_error_ass) ? error : tmp_error_ass;
                       // cout<<"\t"<<Value[i]<<endl;
                        i=VEq[i].getNext();//next place that is computed directly
                        
                    }
                    derived();// it derives all the rest of places
                    //cout<<"error:"<<error<<endl;
                    if (error<=perc1){
                        time+=h;
    
                        //Initialize current step
                        if (((NextPrintTime<time+epsilon)||(NextPrintTime==0))){
                            if (Info)
                                out<<time;
                            cout<<"Time:"<<time<<"(Step: "<<Print_Step<<", Last_Step:"<<h<<")"<<endl;
                        }        
                    
                        for (int i=0; i<nPlaces; i++)
                        {
                            if ((Info) &&((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                                out<<" "<<Value[i];
                            ValuePrv[i]=Value[i];
                        }
                        if (((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                        {
                            if (Info)
                                out<<endl;
                            NextPrintTime+=Print_Step;
                        
                        }
                     current_attempt=0;       
                    } 
                    else {
                            
                            current_attempt++;
                            if (current_attempt>max_attempt){
                                throw   Exception("*****Error max attempt reached without satisfying tolerant, increase max_attempt in class.hpp*****\n\n");   

                            }                   
                    }
                    

                    // Prevent small truncation error from rounding to zero
                    if (error == 0.) 
                    	error = 1.0e-30;
                    //cout<<"update error:"<<error<<" "<<perc1/error<<" "<<fabs(perc1/error)<<" "<<(long double)std::pow((long double)fabs(perc1/(error)), 0.25)<<endl;
                    // Calculate new step-length 
                    long double delta  = 0.84 *std::pow((long double )fabs(perc1/(error)), 0.25);
                    //cout<<"delta:"<<delta<<endl;
                    if (delta <=0.1)
                    	h=h*0.1;
                    else
                    	if (delta >=4)
                    		h=h*4;
                    	else
                    		h=h*delta;
                    //cout<<"h:"<<h<<endl;
                    
                    if (time+h>Max_Time)
                    {
                        h=Max_Time-time;
                    }
                    else
                    {
                        if ((Info)&&(time+h>NextPrintTime)){
                            h=NextPrintTime-time;
                           
                        }
                    }    

                }
                
                
                
                for (int i=0;i<nPlaces;i++)//store resul ode
                {
                    FinalValueXRun[i][0]= Value[i];
                }
                
                //Print final time for each trace
                if ((Info)&&((NextPrintTime<=time)||(NextPrintTime==0)))
                {
                    out<<time;
                    for (int i=0;i<nPlaces;i++){//save previous step
                        out<<" "<<Value[i];
                    }
                    out<<endl; 
                }
                
                
                
                //Print final time for each trace  
                cout<<"Solution at time "<<time<<":\n";
                for (int i=0;i<nPlaces;i++)//save initial state
                {
                    cout<<"\t"<<NamePlaces[i]<<": "<<Value[i]<<"\n";
                }
                cout<<endl;  
            }
            
            
             /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : It computes a single step of Kutta-Merson integration*/
            /**************************************************************/ 
            inline void  SystEq::StepODERK5(long double* Yn, long double* Kn, const double& h){              
                    
                    //compute transition enable value in Yn
                    getValTranFire(Yn);
                   
                    unsigned int i=headDirc;
                    while (i!=DEFAULT)
                    {
                        Kn[i]=0.0;
                        for (int j=0; j<VEq[i].getSize();j++)// for all components
                        {
                            long double val=0.0;
                            if ((!NotEnable(VEq[i].getIdTrans(j)))&&((val=EnabledTransValueCon[VEq[i].getIdTrans(j)])>0))//if transition is not disable by inhibitor arc 
                            { 
                                Kn[i]+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*val;
                            }
                        }
                        Kn[i]*=h;
                        i=VEq[i].getNext();//next place that is computed directly
                    }
                    
                }
 
 
 
 
 
 
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : It estimates the integration step for Euler method*/
            /**************************************************************/  
            
            void  SystEq::HeuristicStep(double h,double perc1,double perc2,double Max_Time,bool Info,double Print_Step,char *argv){
                
                //assign factor for step
                long double tmpval[nPlaces]; 
                this->fh=h;
                this->perc1=perc1;
                this->perc2=perc2;
                this->Max_Run=Max_Run;
                //For statistic
                FinalValueXRun=new long double*[nPlaces];
                long double* ValueInit=(long double*)malloc(sizeof(long double)*nPlaces);
                
                for (int i=0;i<nPlaces;i++)
                {
                    FinalValueXRun[i]=new long double[Max_Run+1];
                    ValueInit[i]=Value[i];
                    for (int j=0;j<Max_Run+1;j++)
                        FinalValueXRun[i][j]=0.0;
                }
                
                
                double NextPrintTime=0.0;
                ofstream out;
                
                if (Info)
                {
                    out.open(string(argv)+".trace",ofstream::out);
                    out.precision(12);
                    if(!out) 
                    {
                        throw Exception("*****Error opening output file *****\n\n");  
                        
                    }
                } 
                cout.precision(16);
                double Max_error;
                double eT=perc2;
                int MaxIter=20;
                int Iter=0;
                do 
                {
                    cout<<"Euler Step: "<<this->perc1<<endl;
                    Max_error=-1;
                    time=0.0;
                    for (int i=0;(i<nPlaces);i++)//return to the initial state
                    {
                        Value[i]=ValueInit[i];
                        
                    }
                    while (time<Max_Time)
                    { 
                        //Initialize current step
                        if ((Info)&&((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                            out<<time;
                        for (int i=0; i<nPlaces; i++)
                        {
                            if ((Info) &&((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                                out<<" "<<Value[i];
                            ValuePrv[i]=Value[i];
                        }
                        if ((Info)&&((NextPrintTime<time+epsilon)||(NextPrintTime==0)))
                        {
                            out<<endl;
                            NextPrintTime+=Print_Step;
                            
                            
                        }
                        //compute transition enable value
                        getValTranFire();
                        #if ADAPTATIVE	
                        h=MAXSTEP;
                        ComputeHstep(h);
                        #else
                        h=fh;
                        #endif	
                        
                        if (time+h>Max_Time)
                        {
                            h=Max_Time-time;
                        }
                        else
                        {
                            if ((Info)&&(time+h>NextPrintTime)){
                                h=NextPrintTime-time;
                            }
                        }    
                        //Aggiunto per fare dei test!!!!   
                        unsigned int i=headDirc;
                        unsigned int  ind_maxvalD=0;
                        long double maxvalD=0.0;
                        while (i!=DEFAULT)
                        {
                            tmpval[i]=0.0;
                            for (int j=0; j<VEq[i].getSize();j++)// for all components
                            {
                                long double val=0.0;
                                if ((!NotEnable(VEq[i].getIdTrans(j)))&&((val=EnabledTransValueCon[VEq[i].getIdTrans(j)])>0))//if transition is not disable by inhibitor arc 
                                { 
                                    tmpval[i]+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*val;
                                }
                            }
                            
                            long double increment=fabs(ValuePrv[i]-tmpval[i])/(ValuePrv[i]==0?1:ValuePrv[i]);
                            //cout<<increment<<endl;
                            if (increment>maxvalD) {
                                ind_maxvalD=i;
                                maxvalD=increment;
                            }
                            i=VEq[i].getNext();//next place that is computed directly
                        }
                        
                        #if ADAPTATIVE		  
                        ComputeHstep(ValuePrv[ind_maxvalD],maxvalD,h);
                        #endif	
                        //
                       // cout<<h<<" "<<maxvalD<<endl;
                       // exit(1);
                        i=headDirc;
                        while (i!=DEFAULT)
                        {
                            Value[i]=ValuePrv[i]+tmpval[i]*h;
                            i=VEq[i].getNext();//next place that is computed directly
                        }
                        derived();// it derives all the rest of places
                        time+=h;
                    }
                    
                    
                    Max_error=-1;
                    for (int i=0;i<nPlaces;i++)//store resul ode
                    {
                       
                        //cout<<"\t\t==="<<Value[i]<<" "<<FinalValueXRun[i][0]<<endl;
                        double num=Value[i]-FinalValueXRun[i][0]; 
                        //cout<<"\t\t***"<<num<<endl;
                        if (FinalValueXRun[i][0] !=0)
                            num = num/FinalValueXRun[i][0];
                        num=fabs(num);
                        //cout<<"\t\t***"<<num<<endl;
                        if (Max_error<num){
                            Max_error=num;
                             // cout<<"\tMax_error"<<Max_error<<" "<<Value[i]<<" "<<FinalValueXRun[i][0]<<endl;
                        }
                        FinalValueXRun[i][0]= Value[i];
                    }
                    
                    cout<<"Max_error_rel: "<<Max_error<<" at iteration "<<Iter<<endl;
                    //Print final time for each trace
                    if ((Info)&&((NextPrintTime<=time)||(NextPrintTime==0)))
                    {
                        out<<time;
                        for (int i=0;i<nPlaces;i++){//save previous step
                            out<<" "<<Value[i];
                        }
                        out<<endl; 
                    }
                    
                    
                    
                    //Print final time for each trace  
                    cout<<"Solution at time "<<time<<":\n";
                    for (int i=0;i<nPlaces;i++)//save initial state
                    {
                        cout<<"\t"<<NamePlaces[i]<<": "<<Value[i]<<"\n";
                    }
                    cout<<endl;  
                    ++Iter;
                    this->perc1=this->perc1/2;
                }
                while ((Max_error>eT) && (Iter<MaxIter));
                if (Max_error<eT){
                    cout<<"Euler step: "<<this->perc1*2<<endl;
                }
                else
                {
                 cout<<"Error Euler step not found: "<<this->perc1*2 << " with error "<<Max_error<<endl;
                }
            }
            
            
            
            
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : InsetTran stores the transition information*/
            /**************************************************************/ 
            
            void SystEq::InsertTran(int num, struct InfTr T){
                if ((num<0)||(num>nTrans-1))
                {
                    throw Exception("Error id transition is not corrected\n\n");
                }
                Trans[num].rate=T.rate;
                Trans[num].enable=true; 
                Trans[num].dist[0] = std::exponential_distribution<double> (T.rate);
                vector<InfPlace>::iterator it=T.InhPlaces.begin();
                //inhibitor places
                while(it!=T.InhPlaces.end())
                {
                    Trans[num].InhPlaces.push_back(*it);
                    ++it;
                }
                //input places
                it=T.InPlaces.begin();
                while(it!=T.InPlaces.end())
                {
                    Trans[num].InPlaces.push_back(*it);
                    ++it;
                }
                Trans[num].InOuPlaces=T.InOuPlaces;
            }
            
            
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : InsetEq stores an equation*/
            /**************************************************************/ 
            
            void SystEq::InsertEq(int num, class Equation& Eq,double InitValue,double LBound, double UBound){
                if ((num<0)||(num>nPlaces-1))
                {
                    throw Exception("Error id equation/place is not corrected\n\n");
                }
                
                VEq[num]=Eq; 
                Value[num]=InitValue;
                this->LBound[num]=LBound;
                this->UBound[num]=UBound;
                if (Eq.isDirectlyComputed())
                {
                    VEq[num].setNext(headDirc);
                    headDirc=num;
                }
                else
                {
                    VEq[num].setNext(headDerv);
                    headDerv=num; 
                }
            }
            
            
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : PrintStatistic print the places' distribution*/
            /**************************************************************/ 
            void SystEq::PrintStatistic(char *argv){
                if (FinalValueXRun==NULL)
                    throw Exception("*****Error no solution  computed *****\n\n");
                #if DEBUG 
                map<int,int> Freq;
                for (int i=0; i<nPlaces;i++)
                {//for all places
                    Freq.clear(); 
                    for (int j=0;j<Max_Run;j++)
                    {//for all run
                        Freq[round(FinalValueXRun[i][j])]++;
                    }
                    
                    map<int,int>::iterator it=Freq.begin();
                    
                    cout.precision(12);
                    cout<<"Place "<<NamePlaces[i]<<endl;
                    while(it!=Freq.end())
                    {
                        cout<<"\t"<<it->first<<" "<<(double)it->second/Max_Run<<endl;
                        ++it;
                    }
                    
                }
                #endif
                
                ofstream out(string(argv)+".mtx",ofstream::out);
                out.precision(12);
                out<<NamePlaces[0];
                for (int i=1;i<nPlaces;i++)
                    out<<" "<<NamePlaces[i];
                out<<endl;
                for (int j=0;j<Max_Run;j++)
                {//for all places
                    for (int i=0; i<nPlaces;i++)
                    {//for all run
                        out<<FinalValueXRun[i][j]<<" "; 
                    }
                    out<<endl;
                }  
                
                out.close();
                
            }
            
            
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : Print all the system information*/
            /**************************************************************/ 
            
            
            void SystEq::Print(){
                cout<<"\n\n =========================================================\n";
                cout<<"|\t\t      System equation\t\t\t  |\n";
                cout<<" =========================================================\n\n";
                if ((NamePlaces.size()==(unsigned)nPlaces)&&(NameTrans.size()==(unsigned)nTrans))//check name
                {
                    unsigned int i=headDirc;
                    while (i!=DEFAULT)
                    {
                        std::cout<<"\td"<<NamePlaces[i]<<"/dt ="; 
                        VEq[i].Print(NamePlaces,NameTrans,Trans,typeTfunction);
                        std::cout<<"\t"<<NamePlaces[i]<<"_"<<time<<" = "<<Value[i]<<endl<<endl;// current computed value
                        i=VEq[i].getNext();
                    }
                    i=headDerv;
                    int place;
                    double coff; 
                    while (i!=DEFAULT)
                    {
                        cout<<"\t"<<NamePlaces[i]<<" = ";
                        for (int j=0;j<VEq[i].getSizeP()-1;j++)
                        {
                            VEq[i].getPsemflw(j,place,coff);
                            coff*=-1;
                            if (coff>=0)
                                cout<<"+";
                            cout<<coff<<"*"<<NamePlaces[place];
                        }
                        VEq[i].getPsemflw(VEq[i].getSizeP()-1,place,coff);
                        if (coff>=0)
                            cout<<"+";
                        cout<<coff<<endl;
                        i=VEq[i].getNext(); 
                    }
                    cout<<endl;
                    cout<<"\tTransitions' rate\n";
                    for (int i=0;i<nTrans;i++)
                        cout<<"\t\t"<<NameTrans[i]<<": "<<Trans[i].rate<<endl;
                    cout<<"===========================================================\n\n";
                }
                
            }
            //automaton   
            #ifdef AUTOMATON
            /**************************************************************/
            /* NAME :  Class SystEq*/
            /* DESCRIPTION : Initialize automaton*/
            /**************************************************************/ 
            void SystEq::initialize_automaton(char * name){
                
                try{
                    automaton.set_variable_info(NamePlaces,Value,&time,nPlaces);
                    reading_automaton(automaton,name);
                    automaton.print(cout);
                }
                catch (my_exception e){
                    cerr << "\n*****"<<e.what()<<endl<< endl;
                    exit(EXIT_FAILURE);
                }
                
            }
            #endif
            //automaton 
            
            /**************************************************************/
            /* NAME :  Class Equation*/
            /* DESCRIPTION : Copy Constructor*/
            /**************************************************************/
            
            Equation::Equation(const Equation& Eq){
                
                
                vector <class Elem>::const_iterator it = Eq.LElem.begin();
                while (it!= Eq.LElem.end())
                {
                    LElem.push_back(*it);
                    ++it;
                }
                vector < struct InfPlace >::const_iterator it1 = Eq.PSemiflow.begin();
                while (it1!= Eq.PSemiflow.end())
                {
                    PSemiflow.push_back(*it1);
                    ++it1;
                }
                next=Eq.next;
            }
            
            
            /**************************************************************/
            /* NAME :  Class Equation*/
            /* DESCRIPTION : Constructor taking in input the list of places in the P-semiflow*/
            /**************************************************************/
            Equation::Equation(vector <struct InfPlace> places){
                
                vector <struct InfPlace>::iterator it = places.begin();
                while (it!= places.end())
                {
                    PSemiflow.push_back(*it);
                    ++it;
                } 
                next=DEFAULT;
            }
            
            
            /**************************************************************/
            /* NAME :  Class Equation*/
            /* DESCRIPTION : Insertion taking in input the list of places in the P-semiflow*/
            /**************************************************************/
            void Equation::Insert(vector <struct InfPlace> places){
                
                vector <struct InfPlace>::iterator it = places.begin();
                while (it!= places.end())
                {
                    PSemiflow.push_back(*it);
                    ++it;
                } 
                next=DEFAULT;
            }
            
            
            /**************************************************************/
            /* NAME :  Class Eq*/
            /* DESCRIPTION : It prints the equation  */
            /**************************************************************/
            
            inline void Equation::Print(vector< std::string >& NamePlaces, vector< std::string >& NameTrans, struct InfTr* Trans,std::string typeTfunction)
            {
                vector <class Elem>::const_iterator it = LElem.begin();
                while (it!= LElem.end())
                {
                    (*it).Print(NamePlaces,NameTrans,Trans,typeTfunction);
                    ++it;
                }
                cout<<endl;
            }
            
            
            /**************************************************************/
            /* NAME :  Class Elem*/
            /* DESCRIPTION : Constructor*/
            /**************************************************************/
            
            Elem::Elem(int IncDec, int IdTran){
                
                this->IncDec=IncDec;
                this->IdTran=IdTran;
            }
            /**************************************************************/
            /* NAME :  Class Elem*/
            /* DESCRIPTION : Copy Constructor*/
            /**************************************************************/
            
            Elem::Elem(const Elem& El){
                
                
                IncDec=El.IncDec;
                IdTran=El.IdTran;
            }
            
            
            
            
            
            /**************************************************************/
            /* NAME :  Class Elem*/
            /* DESCRIPTION : It prints an  equation element  */
            /**************************************************************/
            
            inline void Elem::Print(vector <string>& NamePlaces, vector <string>& NameTrans, struct InfTr* Trans,std::string typeTfunction)const
            {
                if (IncDec>0)
                    cout<<" +";  
                else
                    cout<<" ";
                cout<<IncDec<<"*"<<NameTrans[IdTran]<<"*"<<typeTfunction<<"( ";
                
                vector <struct InfPlace>::const_iterator it = Trans[IdTran].InPlaces.begin();
                if ((it== Trans[IdTran].InPlaces.end()))
                    cout<<"1.0";
                while (it!= Trans[IdTran].InPlaces.end())
                {
                    cout<<NamePlaces[it->Id];
                    if (it->Card!=1)
                    {
                        if (typeTfunction!="Prod")
                            cout<<"/"<<it->Card;
                        else{
                            int fatt=1;
                            for (int i=2;i<=it->Card;i++){
                                fatt*=i;
                                cout<<"*("<<NamePlaces[it->Id]<<"-"<<i-1<<")";
                            }
                            cout<<"/"<<fatt;
                        }
                    }
                    ++it;
                    if (it!=Trans[IdTran].InPlaces.end())
                        cout<<", ";
                }
                cout<<" )";
                if (Trans[IdTran].InhPlaces.size()>0)
                {
                    cout<<"*1_(";
                    it = Trans[IdTran].InhPlaces.begin();
                    while (it!= Trans[IdTran].InhPlaces.end())
                    {
                        cout<<NamePlaces[it->Id];
                        if (it->Card>1)
                        {
                            cout<<"<"<<it->Card;
                        }
                        ++it;
                        if (it!=Trans[IdTran].InhPlaces.end())
                            cout<<" && ";
                    }
                    cout<<")";
                }
            }
            
            
        };
        
