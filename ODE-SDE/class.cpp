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
#define MAX_DOUBLE std::numeric_limits<double>::max()
#ifdef AUTOMATON
extern void reading_automaton( AUTOMA::automaton& a, char * name);
extern double next_clock_automaton;
#endif
double MININC=0;
namespace SDE {


//long int seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

std::mt19937_64 generator;//(seed);
std::normal_distribution<double> distribution;//(0.0,1.0);


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
bool SystEq::readSLUBounds(const string& file){
	ifstream in(file.c_str(), std::ifstream::in);
	if (!in)
	{
		cerr<<"\nError:  it is not possible to open the soft bound file "<<file<< "\n";
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
	in.close();
	return true;
}



/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : readInitialMarking reads initial marking from a file*/
/**************************************************************/
bool SystEq::readInitialMarking(const string& file){
	ifstream in(file.c_str(), std::ifstream::in);
	if (!in)
	{
		cerr<<"\nError:  it is not possible to open the initila marking file "<<file<< "\n\n";
		return false;
	}

	std::string name;
	cout<<"\n\n-----------------------------------------------------------\n";
	cout<<"\t\t  Reading initial marking"<<endl;
	cout<<"-----------------------------------------------------------\n";
	double buffer=-1;
	int num=0;

	while(in){
		buffer=-1;
		in>>buffer;
		if (buffer>=0.0){
			Value[num]=buffer;
#if DEBUG==1
			cout<<"Place "<<num<<" marking:"<<buffer<<endl;
#endif
			num++;
		}

	}
	if ((num)!=nPlaces){
		cerr<<"\nError:  initial marking file does not cointain a marking for each place\n\n";
		return false;
	}
//updating p-semiflow
	unsigned int i=headDerv;
	while (i!=DEFAULT){
		double value =0.0, coff=0.0;
		int place=-1;
		for (int j=0;j<VEq[i].getSizeP()-1;j++){
			VEq[i].getPsemflw(j,place,coff);
			value+=Value[place]*coff;
		}
		value+=Value[i];
		VEq[i].setPsemflw(VEq[i].getSizeP()-1,value);
		i=VEq[i].getNext();
	}
	cout<<"-----------------------------------------------------------\n";
	return true;
}








/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : readInitialMarking reads initial marking from a file*/
/**************************************************************/
bool SystEq::readParameter(const string& file){
	ifstream in(file.c_str(), std::ifstream::in);
	if (!in)
	{
		cerr<<"\nError:  it is not possible to open the parameter file "<<file<< "\n\n";
		return false;
	}

	std::string name;
	cout<<"\n\n-----------------------------------------------------------\n";
	cout<<"\t\t  Reading Parameter"<<endl;
	cout<<"-----------------------------------------------------------\n";


	try{
		string name="";
		double value=0.0;
		while(!in.eof()){

			in>>name>>value;
			if ((name!="")&& (value!=-1)){
				map<std::string,int>::iterator it;
				if ((it=NumTrans.find(name))!=NumTrans.end()){
					cout<<"New rate of transition "<<name<<" is "<<value<<endl;
					Trans[it->second].rate=value;
				}
				else{
					if ((it=NumPlaces.find(name))!=NumPlaces.end()){
						cout<<"New marking of place "<<name<<" is "<<value<<endl;
						Value[it->second]=value;
					}
					else{
						cout<<"Warning: "<<name<<" is not a valid  transition or place name"<<endl;
					}
				}
				name="";
				value=-1.0;
			}
		}

	}catch (exception& e){
		cout << "Exception: " << e.what() << endl;
		return false;
	}

    //updating p-semiflow
	unsigned int i=headDerv;
	while (i!=DEFAULT){
		double value =0.0, coff=0.0;
		int place=-1;
		for (int j=0;j<VEq[i].getSizeP()-1;j++){
			VEq[i].getPsemflw(j,place,coff);
			value+=Value[place]*coff;
		}
		value+=Value[i];
		VEq[i].setPsemflw(VEq[i].getSizeP()-1,value);
		i=VEq[i].getNext();
	}
	cout<<"-----------------------------------------------------------\n";
	return true;
}







/**************************************************************/
/* NAME :  Class SystEqMin*/
/* DESCRIPTION : getMin returns the minimum value among the places*/
/**************************************************************/

SystEqMin::~SystEqMin() {}

inline void SystEqMin::getValTranFire()
{

	for(int t=0; t<nTrans; t++)
	{
		unsigned int size=Trans[t].InPlaces.size();
		EnabledTransValueCon[t]=0.0;
		EnabledTransValueDis[t]=0.0;

		if (Trans[t].FuncT!=nullptr){
#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#else
			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#endif 
		}
		else {
			if (size==0)
				EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;
			else
			{
				double tmpC;
				double tmpD;
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

}


/**************************************************************/
/* NAME :  Class SystEqMas*/
/* DESCRIPTION : getProd returns the producd value among the places*/
/**************************************************************/

SystEqMas::~SystEqMas() {}

inline void SystEqMas::getValTranFire()
{
	for(int t=0; t<nTrans; t++)
	{
		EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;
       // cout<<" T:"<<NameTrans[t]<<endl;
		if (Trans[t].FuncT!=nullptr){
#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#else
			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#endif 		

		}
		else {

			if (Trans[t].InPlaces.size()>0)
			{
				for (unsigned int k=0; k<Trans[t].InPlaces.size(); k++)//for all variables in the components
				{
					double valD,valC;
					valD=valC=ValuePrv[Trans[t].InPlaces[k].Id];
		//			cout<<"\t"<<NamePlaces[Trans[t].InPlaces[k].Id]<<" card:"<<Trans[t].InPlaces[k].Card<<" num:"<<ValuePrv[Trans[t].InPlaces[k].Id]<<endl;
					double fatt=1;
					for (int i=2;i<=Trans[t].InPlaces[k].Card;i++)
					{
						valD*=(ValuePrv[Trans[t].InPlaces[k].Id]-i+1);
						valC*=ValuePrv[Trans[t].InPlaces[k].Id];
						fatt*=(double)i;

					}

					if (valD>0)
						EnabledTransValueDis[t]*=valD/fatt;
					else
						EnabledTransValueDis[t]=0.0;

					EnabledTransValueCon[t]*=valC/fatt;

				}
			}
		}
		//cout<<"\t"<< NameTrans[t]<<" "<<" Tot:"<<EnabledTransValueCon[t]<<endl;
	}
}


/**************************************************************/
/* NAME :  Class SystEqMas*/
/* DESCRIPTION : getProd returns the producd value among the places
                     considering its input vector ValuePrv as marking*/
/**************************************************************/

inline void SystEqMas::getValTranFire(double* ValuePrv)
{

	for(int t=0; t<nTrans; t++)
	{
		EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;

      //cout<<" T:"<<NameTrans[t]<<endl;
		//If the transition is exponential general
		if (Trans[t].FuncT!=nullptr && Trans[t].timing != NON_EXP_GEN)
		{ 

#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#else
			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#endif 	

		}

		else
		{

			//check if the transition is non exponential general and solver not SSA
			if(Trans[t].timing == NON_EXP_GEN && solve != Solve_SSA){
				throw Exception("You can't use non exponential transition with other solver than SSA");
			}

			//this code is in common between non exp gen e standard exp transition
			if (Trans[t].InPlaces.size()>0)
			{

                    for (unsigned int k=0; k<Trans[t].InPlaces.size(); k++)//for all variables in the components
                    {
                    	double valD,valC;
                    	valD=valC=ValuePrv[Trans[t].InPlaces[k].Id];
                    	double fatt=1;
                    	// card is the multiplicity of the cardinality
                    	for (int i=2; i<=Trans[t].InPlaces[k].Card; i++)
                    	{
                    		valD*=(ValuePrv[Trans[t].InPlaces[k].Id]-i+1);
                    		valC*=ValuePrv[Trans[t].InPlaces[k].Id];
                    		fatt*=(double)i;

                    	}

                    	if (valD>0)
                    		EnabledTransValueDis[t]*=valD/fatt;
                    	else
                    		EnabledTransValueDis[t]=0.0;
                    	EnabledTransValueCon[t]*=valC/fatt;
                       // cout<<"\t"<<valC/fatt<<" Tot:"<<EnabledTransValueCon[t]<<endl;
                    }
                }
            }
        }
    }



/**************************************************************/
/* NAME :  Class SystEqMin*/
/* DESCRIPTION : getMin returns the minimum value among the places
                     considering its input vector ValuePrv as marking*/
/**************************************************************/

    inline void SystEqMin::getValTranFire(double* ValuePrv)
    {

    	for(int t=0; t<nTrans; t++)
    	{
    		unsigned int size=Trans[t].InPlaces.size();
    		EnabledTransValueCon[t]=0.0;
    		EnabledTransValueDis[t]=0.0;
    		//if the transition is exponential general
    		if (Trans[t].FuncT!=nullptr && Trans[t].timing != NON_EXP_GEN)
    		{

#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
    			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#else
    			EnabledTransValueDis[t]=EnabledTransValueCon[t]=Trans[t].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,t,time);
#endif 	
    		}
    		else
    		{

    			if(Trans[t].timing == NON_EXP_GEN && solve != Solve_SSA){
    				throw Exception("You can't use non exponential transition with other solver than SSA");
    			}

    			//this code is in common between non exp gen e standard exp transition
    			if (size==0){
    				EnabledTransValueDis[t]=EnabledTransValueCon[t]=1.0;
    			}
    			else
    			{
    				double tmpC;
    				double tmpD;
                    //if (Trans[t].InPlaces.size()>0)
                    //{
    				EnabledTransValueCon[t] =ValuePrv[Trans[t].InPlaces[0].Id]/Trans[t].InPlaces[0].Card;
    				EnabledTransValueDis[t] =trunc(EnabledTransValueCon[t]);
                    for (unsigned int k=1; k<size&&(EnabledTransValueCon[t]!=0.0); k++)//for all variable in the components
                    {
                    	tmpC=ValuePrv[Trans[t].InPlaces[k].Id]/Trans[t].InPlaces[k].Card;
                    	tmpD=trunc(tmpC);
                    	if ( EnabledTransValueCon[t]>tmpC)
                    	{
                    		EnabledTransValueCon[t]=tmpC;
                    		EnabledTransValueDis[t]=tmpD;
                    	}

                    }

                }
            }

        }
    }


/**************************************************************/
/* NAME :  Class updateFutureEventList*/
/* DESCRIPTION : updateFutureEventList updates the future event list during the SSA with non exponential general
				transition using the value of Enabling */
/**************************************************************/

    void SystEq::updateFutureEventList(int tran, int prev_fired, double time){


    	int number_events = EnabledTransValueDis[tran] - PreviousEnabledTransValueDis[tran];

    	//if the transition is already fired one event has already been popped out
    	if(tran==prev_fired){
    		number_events++;
    	}

    	while(number_events>0)
    	{

#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
    		double time_event = Trans[tran].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,tran,time);
#else
    		double time_event = Trans[tran].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,tran,time);
#endif 	
    		time_event = time_event + time;
    		Event *event_distribution = new Event(time_event, tran);
    		future_event_list.pushHeap(event_distribution);
    		addEventAtTheEnd(event_distribution, tran);
    		Trans[tran].events_size++;
    		number_events--;
    	}

		//remove events if there are less tokens; the chosen transition is not updated here
		// so I can do the loop normally
    	while(number_events < 0 && Trans[tran].events_size > 0 && future_event_list.getLenght() > 0)
    	{
    		int index = uniform(0, Trans[tran].events_size-1, generator);
    		Event *event_removed = deleteEventInPosition(index, tran);
    		future_event_list.removeHeap(event_removed);
    		number_events++;
    		Trans[tran].events_size--;
    	}

    	//update with new/old value for next loop
    	PreviousEnabledTransValueDis[tran] = EnabledTransValueDis[tran];

    }

/**************************************************************/
/* NAME :  Class addEventAtTheEnd*/
/* DESCRIPTION : add a new element at the end of the event list of a particular transition */
/**************************************************************/

    void SystEq::addEventAtTheEnd(Event* event, int tran){

    	if(Trans[tran].events_size==0){
    		Trans[tran].first_event = event;
    		Trans[tran].last_event = event;
    		return;
    	}

    	Trans[tran].last_event->setNext(event);
    	event->setPrevious(Trans[tran].last_event);
    	Trans[tran].last_event = event;
    }


    

/**************************************************************/
/* NAME :  Class deleteEventInPosition*/
/* DESCRIPTION : delete the i-th event in the event list updating the pointers */
/**************************************************************/

    Event* SystEq::deleteEventInPosition(int event_index, int tran){

    	Event *temp1 = Trans[tran].first_event, *temp2 = Trans[tran].first_event;

    // Deleting the head.
    	if (event_index == 0) {
    		if(Trans[tran].first_event != NULL) {
     	  		// Update head
    			Trans[tran].first_event = Trans[tran].first_event->getNext();
    			//if it's not the last element of the list
    			if(Trans[tran].first_event != NULL)
    				Trans[tran].first_event ->setPrevious(NULL);
    		}
    	}
    	if(event_index==Trans[tran].events_size-1){
    		Trans[tran].last_event = Trans[tran].last_event->getPrevious();
    	}


    // Traverse the list to
    // find the node to be deleted.
    	while (event_index > 0) {

        // Update temp2
    		temp2 = temp1;

        // Update temp1
    		temp1 = temp1->getNext();

    		event_index--;
    	}

    	Event *next = temp1->getNext();

    // Change the next pointer of the previous node.
    	temp2->setNext(next);

    	if(next != NULL)
    		(next)->setPrevious(temp2);

    	return temp1;

    }

/**************************************************************/
/* NAME :  Class deleteEvent*/
/* DESCRIPTION : delete the event passed as parameter in the event list updating the pointers */
/**************************************************************/

    void SystEq::deleteEvent(Event* event){

    	int tran = event ->getIndexTran();
    	bool head_or_tail = false;


    // Deleting the head.
    	if (event == Trans[tran].first_event) {

        // Update head
    		Trans[tran].first_event = Trans[tran].first_event->getNext();
    		//it it's not the last element of the list
    		if(Trans[tran].first_event != NULL)
    			Trans[tran].first_event ->setPrevious(NULL);
    		head_or_tail = true;
    	}


    	if(event == Trans[tran].last_event){
    		Trans[tran].last_event = Trans[tran].last_event->getPrevious();
    		head_or_tail = true;
    	}

    	if(head_or_tail)
    		return;

    	Event *next = event->getNext();
    	Event *prev = event->getPrevious();

    // Change the next pointer of the previous node.
    	prev->setNext(next);
    	if(next !=NULL)
    		next->setPrevious(prev); 


    }


/**************************************************************/
/* NAME :  Class setSizeFutureEventList*/
/* DESCRIPTION : set the initial size of the future event list to limitate the reallocation
				when the infinite server is used */
/**************************************************************/

    inline void SystEqMin::setSizeFutureEventList(int nTrans, int &size_expTran, int &size_notExpTran){

    	int future_event_list_size = 0;

    	for(int t=0; t<nTrans; t++)
    	{
    		if(Trans[t].timing == NON_EXP_GEN) {

    			PreviousEnabledTransValueDis[t] = 0;
    			size_notExpTran++;
				for (unsigned int k=0; k<Trans[t].InPlaces.size(); k++)//for all variables in the components
				{
					future_event_list_size += (floor(Value[k]/Trans[t].InPlaces[k].Card));
				}
			}
			else
				size_expTran++;
		}

		future_event_list.setHeapSize(future_event_list_size);
	}



/**************************************************************/
/* NAME :  Class setSizeFutureEventList*/
/* DESCRIPTION : set the initial size of the future event list to limitate the reallocation
				when the mass action is used */
/**************************************************************/

	inline void SystEqMas::setSizeFutureEventList(int nTrans, int &size_expTran, int &size_notExpTran){

		int future_event_list_size = 0;

		for(int t=0; t<nTrans; t++)
		{
			if(Trans[t].timing == NON_EXP_GEN) {

				PreviousEnabledTransValueDis[t] = 0;
				size_notExpTran++;
			for (unsigned int k=0; k<Trans[t].InPlaces.size(); k++)//for all variables in the components
			{
				future_event_list_size *= Value[k];
			}
		}
		else
			size_expTran++;
	}
	future_event_list.setHeapSize(future_event_list_size);
}




/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : Constructor taking in input  the total number of transitions and places, and two vector with the names of places and transitions */
/**************************************************************/

SystEq::SystEq(int nPlaces,int nTrans, string NamePlaces[],  string NameTrans[], double itime, long int seed){

	this->nTrans=nTrans;
	this->nPlaces=nPlaces;

	Trans=new struct InfTr[nTrans];
	VEq=new class Equation[nPlaces];
	Value=(double*)malloc(sizeof(double)*nPlaces);
	ValuePrv=(double*)malloc(sizeof(double)*nPlaces);
	LBound=(double*)malloc(sizeof(double)*nPlaces);
	UBound=(double*)malloc(sizeof(double)*nPlaces);
	SLBound=(double*)malloc(sizeof(double)*nPlaces);
	SUBound=(double*)malloc(sizeof(double)*nPlaces);
	EnabledTransValueCon=(double*)malloc(sizeof(double)*nTrans);
	EnabledTransValueDis=(double*)malloc(sizeof(double)*nTrans);
	PreviousEnabledTransValueDis=(double*)malloc(sizeof(double)*nTrans);
	TransRate=(double*)malloc(sizeof(double)*(nTrans+1));


	time=this->itime=itime;
	int i=0;
	while(i<nTrans)
	{
		this->NameTrans.push_back(NameTrans[i]);
		this->NumTrans[NameTrans[i]]=i;
		++i;
	}
	i=0;
	while(i<nPlaces)
	{
		this->NamePlaces.push_back(NamePlaces[i]);
		this->NumPlaces[NamePlaces[i]]=i;
		SLBound[i]=epsilon;
		SUBound[i]=INT_MAX;
		++i;
	}
	headDirc=headDerv=DEFAULT;
	if (seed==0L)
		this->seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	else
		this->seed=seed;

	generator.seed(this->seed);
	distribution.param(std::normal_distribution<double>::param_type(0.0, 1.0));
    //distribution=&std::normal_distribution<double> (0.0,1.0);
   // =std::mt19937_64 generator(seed);
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
	free(TransRate);
	free(PreviousEnabledTransValueDis);
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
		double fireTime;

		double minfireTime;
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
 *    double fireTime;
 *
 *    double minfireTime;
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
			double inc=Trans[i].rate*EnabledTransValueCon[i];
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
	double max_drift=0.0;
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
	// FinalValueXRun=new double*[nPlaces+automaton.timers_num()];
	// #else
	FinalValueXRun=new double*[nPlaces];
	//#endif
	for (int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
	}

#ifdef AUTOMATON
	double** FinalValueTimerXRun=new double*[automaton.timers_num()];
	for (int i=0;i<automaton.timers_num();i++)
	{
		FinalValueTimerXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueTimerXRun[i][j]=0.0;
	}
#endif

	cout.precision(16);
	cout<<endl<<"Seed value: "<<seed<<endl<<endl;
	bool SIM=false;
	if (h==MAXSTEP)
		SIM=true;

	int sizeMT=Max_Time/Print_Step+2;
	double** MeanTrace=nullptr;
if (Info){ //to Initialize the matrix storing trace values
	MeanTrace=new double*[sizeMT];
	for (int i=0;i<sizeMT;i++){
		MeanTrace[i]=new double[nPlaces];
		for (int j=0;j<nPlaces;j++){
			MeanTrace[i][j]=0.0;
		}
	}
}//to Initialize the matrix storing trace values


this->Max_Run=Max_Run;
int Dsteps=0,Ssteps=0;


int run=0;

double* ValueInit=(double*)malloc(sizeof(double)*nPlaces);
//  #ifdef AUTOMATON
// double* Mean=(double*)malloc(sizeof(double)*(nPlaces+automaton.timers_num()));
// #else
double* Mean=(double*)malloc(sizeof(double)*nPlaces);
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
	time=itime;
	double NextPrintTime=itime;
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
	double tmpvalD=0.0, tmpvalSIM=0.0;
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
	out.precision(16);
	if(!out){
		throw Exception("*****Error opening output file *****\n\n");
	}
	out<<"Time";
	for (int j=0;j<nPlaces;j++){
		out<<" "<<NamePlaces[j];
	}
	out<<endl;
	for (int i=0;i<sizeMT&&(Print_Step*i<=Max_Time);i++){
		out<<Print_Step*i+itime;
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



cout.precision(16);
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
	out.precision(16);
	out<<double(tot_accepted_traces)/double(Max_Run)<<"\t"<<l<<"\t"<<u<<"\t"<<seed<<endl;
	out.close();
	if (automaton.timers_num()>0){
		ofstream out(string(argv)+".timer_auto",ofstream::out);
		cout<<string(argv)<<".timer_auto"<<endl;
		if(!out){
			throw Exception("*****Error opening output file *****\n\n");
		}
		out.precision(16);
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
	FinalValueXRun=new double*[nPlaces];
	for (int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
	}

#ifdef AUTOMATON
	double** FinalValueTimerXRun=new double*[automaton.timers_num()];
	for (int i=0;i<automaton.timers_num();i++)
	{
		FinalValueTimerXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueTimerXRun[i][j]=0.0;
	}
#endif

	cout.precision(16);
	cout<<endl<<"Seed value: "<<seed<<endl<<endl;

	int sizeMT=Max_Time/Print_Step+2;
	double** MeanTrace=nullptr;
if (Info){ //to Initialize the matrix storing trace values
	MeanTrace=new double*[sizeMT];
	for (int i=0;i<sizeMT;i++){
		MeanTrace[i]=new double[nPlaces];
		for (int j=0;j<nPlaces;j++){
			MeanTrace[i][j]=0.0;
		}
	}
}//to Initialize the matrix storing trace values

double tmpvalD[nPlaces],tmpvalS[nPlaces], tmpvalSIM[nPlaces];


//It is used to maintain the set of transitions for the simulation step.
int SetTran[nTrans+1];


this->Max_Run=Max_Run;
unsigned long Dsteps=0,Ssteps=0;

std::uniform_real_distribution<double> uniform(0.0,1.0);
int run=0;

double* ValueInit=(double*)malloc(sizeof(double)*nPlaces);
double* Mean=(double*)malloc(sizeof(double)*nPlaces);
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
	time=itime;
	double NextPrintTime=itime;
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
double maxvalD=0.0;
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
			double increment=fabs(ValuePrv[i]-tmpvalD[i])/(ValuePrv[i]==0?1:ValuePrv[i]);
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
double sqrth=sqrt(h);
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
	out.precision(16);
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
		out<<Print_Step*i+itime;
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
	out.precision(16);
	out<<double(tot_accepted_traces)/double(Max_Run)<<"\t"<<l<<"\t"<<u<<"\t"<<seed<<endl;
	out.close();

	if (automaton.timers_num()>0){
		ofstream out(string(argv)+".timer_auto",ofstream::out);
		cout<<string(argv)<<".timer_auto"<<endl;
		if(!out){
			throw Exception("*****Error opening output file *****\n\n");
		}
		out.precision(16);
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
/* DESCRIPTION : It derives the place which are not directly computed*. It stops when value is negative*/
/**************************************************************/

void  SystEq::derived(bool& neg){

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
			neg=true;
			return;
		}//normalization
		i=VEq[i].getNext();
	}

}



/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It derives the place which are not directly computed
 *               from input vector Value*/
/**************************************************************/

void  SystEq::derived(double* Value){

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
	double tmpval[nPlaces];
	this->fh=h;
	this->perc1=perc1;
	this->perc2=perc2;
	this->Max_Run=1;
	//For statistic
	FinalValueXRun=new double*[nPlaces];
	for (int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
	}


	double NextPrintTime=itime;
	ofstream out;

	if (Info)
	{
		out.open(string(argv)+".trace",ofstream::out);
		out.precision(16);
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
		double maxvalD=0.0;
		while (i!=DEFAULT)
		{
			tmpval[i]=0.0;
			for (int j=0; j<VEq[i].getSize();j++)// for all components
			{
				double val=0.0;
				if ((!NotEnable(VEq[i].getIdTrans(j)))&&((val=EnabledTransValueCon[VEq[i].getIdTrans(j)])>0))//if transition is not disable by inhibitor arc
				{
					tmpval[i]+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*val;
				}
			}

			double increment=fabs(ValuePrv[i]-tmpval[i]);
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
/* DESCRIPTION : It solves the ODE system using  Dormand and Princ method - ode45 */
/**************************************************************/

void  SystEq::SolveODE45(double h, double perc1,double Max_Time,bool Info,double Print_Step,char *argv){


	//k1
	double k1[nPlaces];
	//k2
	double k2[nPlaces];
	//k3
	double k3[nPlaces];
	//k4
	double k4[nPlaces];
	//k5
	double k5[nPlaces];
	//k6
	double k6[nPlaces];
	//k6
	double k7[nPlaces];

	double ValueA1[nPlaces];

	const double C3d40 = 3.0/40.0;
	const double C9d40 = 9.0/40.0;
	const double C40d45 =40.0/45.0;
	const double C56d15 =56.0/15.0;
	const double C32d9 =32.0/9.0;
	const double C19372d6561 =19372.0/6561.0;
	const double C25360d2187 =25360.0/2187.0;
	const double C64448d6561 =64448.0/6561.0;
	const double C212d729	=212.0/729.0;
	const double C9017d3168 =9017.0/3168.0;
	const double C335d33 =335.0/33.0;
	const double C46732d5247 =46732.0/5247.0;
	const double C49d176 =49.0/176.0;
	const double C5103d18656 =5103.0/18656.0;
	const double C35d384 =35.0/384.0;
	const double C500d1113 =500.0/1113.0;
	const double C125d192 =125.0/192.0;
	const double C2187d6784 =2187.0/6784.0;
	const double C11d84 =11.0/84.0;
	const double C5179d57600 =5179.0/57600.0;
	const double C7571d16695 =7571.0/16695.0;
	const double C393d640 =393.0/640.0;
	const double C92097d339200 =92097.0/339200.0;
	const double C187d2100 =187.0/2100.0;

	this->fh=h;
	this->Max_Run=1;
	//For statistic
	FinalValueXRun=new double*[nPlaces];
	for (int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
	}


	double NextPrintTime=itime;
	ofstream out;

	if (Info)
	{
		out.open(string(argv)+".trace",ofstream::out);
		out.precision(16);
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


	cout<<"Time:"<<time<<"(Step: "<<Print_Step<<")"<<endl;
	if (Info)
		out<<time;
	for (int i=0; i<nPlaces; i++)
	{
		if (Info)
			out<<" "<<Value[i];
		ValuePrv[i]=Value[i];
	}
	if (Info)
		out<<endl;
	NextPrintTime+=Print_Step;
	if ((Info)&&(h>NextPrintTime))
		h=Print_Step;
	int current_attempt=0;
	while (time<Max_Time)
	{
		StepODERK5(ValuePrv,k1,h);
		unsigned int i=headDirc;
		while (i!=DEFAULT)
		{
			Value[i]=ValuePrv[i]+(k1[i]/5.0);
			i=VEq[i].getNext();//next place that is computed directly
		}
		derived();// it derives all the rest of places




		StepODERK5(Value,k2,h);
		i=headDirc;
		while (i!=DEFAULT)
		{
			Value[i]=ValuePrv[i]+(k1[i]*C3d40+k2[i]*C9d40);
			i=VEq[i].getNext();//next place that is computed directly
		}
		derived();// it derives all the rest of places

		StepODERK5(Value,k3,h);
		i=headDirc;

		while (i!=DEFAULT)
		{
			Value[i]=ValuePrv[i]+(k1[i]*C40d45-k2[i]*C56d15+k3[i]*C32d9);
			i=VEq[i].getNext();//next place that is computed directly
		}
		derived();// it derives all the rest of places

		StepODERK5(Value,k4,h);
		i=headDirc;
		while (i!=DEFAULT)
		{
			Value[i]=ValuePrv[i]+(k1[i]* C19372d6561-k2[i]*C25360d2187+k3[i]*C64448d6561-k4[i]*C212d729);
			i=VEq[i].getNext();//next place that is computed directly
		}
		derived();// it derives all the rest of places


		StepODERK5(Value,k5,h);

		i=headDirc;
		while (i!=DEFAULT)
		{
			Value[i]=ValuePrv[i]+(k1[i]*C9017d3168-k2[i]*C335d33+k3[i]*C46732d5247+k4[i]*C49d176-k5[i]*C5103d18656);
			i=VEq[i].getNext();//next place that is computed directly
		}
		derived();// it derives all the rest of places

		StepODERK5(Value,k6,h);

		i=headDirc;
		while (i!=DEFAULT)
		{
			Value[i]=ValuePrv[i]+(k1[i]*C35d384+k3[i]*C500d1113+k4[i]*C125d192-k5[i]*C2187d6784+k6[i]*C11d84);
			i=VEq[i].getNext();//next place that is computed directly
		}
		derived();// it derives all the rest of places

		StepODERK5(Value,k7,h);


		double error=0.0,tmp_error_ass;
		i=headDirc;
		while (i!=DEFAULT)
		{
			ValueA1[i]=ValuePrv[i]+(k1[i]*C5179d57600+k3[i]*C7571d16695+k4[i]*C393d640-k5[i]*C92097d339200+k6[i]*C187d2100+k7[i]/40.0);
			tmp_error_ass= fabs(Value[i]-ValueA1[i]);
			error = (error > tmp_error_ass) ? error : tmp_error_ass;
			// cout<<"\t"<<Value[i]<<endl;
			i=VEq[i].getNext();//next place that is computed directly

		}
		derived();// it derives all the rest of places
		//cout<<"error:"<<error<<endl;
		if (error<=perc1){
			time+=h;


			if ((Info) && (time==NextPrintTime))
				out<<time;
			for (int i=0; i<nPlaces; i++)
			{
				if ((Info) && (time==NextPrintTime))
					out<<" "<<Value[i];
				ValuePrv[i]=Value[i];
			}


			if(time==NextPrintTime){
				cout<<"\tTime:"<<time<<"(Step: "<<Print_Step<<", Last_Step:"<<h<<")"<<endl;
				NextPrintTime+=Print_Step;
				if (Info)
					out<<endl;
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
		//cout<<"update error:"<<error<<" "<<perc1/error<<" "<<fabs(perc1/error)<<" "<<(double)std::pow((double)fabs(perc1/(error)), 0.25)<<endl;
		// Calculate new step-length
		double delta  = 0.84 *std::pow((double )fabs(perc1/(error)), 2.0);
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
				if ((time+h>NextPrintTime)){
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
/* DESCRIPTION : It solves the ODE system using  Runge-Kutta-Fehlberg method*/
/**************************************************************/

void  SystEq::SolveODERKF(double h, double perc1,double Max_Time,bool Info,double Print_Step,char *argv){


	//k1
	double k1[nPlaces];
	//k2
	double k2[nPlaces];
	//k3
	double k3[nPlaces];
	//k4
	double k4[nPlaces];
	//k5
	double k5[nPlaces];
	//k6
	double k6[nPlaces];

	const double C3d32 = 3.0/32.0;
	const double C9d32 = 9.0/32.0;
	const double C1932d2197 =1932.0/2197.0;
	const double C7200d2197 =7200.0/2197.0;
	const double C7296d2197 =7296.0/2197.0;
	const double C439d216 =439.0/216.0;
	const double C3680d513 =3680.0/513.0;
	const double C845d4104 =845.0/4104.0;
	const double C8d27	=8.0/27.0;
	const double C3544d2565 =3544.0/2565.0;
	const double C1859d4104 =1859.0/4104.0;
	const double C11d40 =11.0/40.0;
	const double C2d55 =2.0/55.0;
	const double C25d216= 25.0/216.0;
	const double C1408d2565= 1408.0/2565.0;
	const double C2197d4104=2197.0/4104.0;
	const double C1d36= 1.0/36.0;
	const double C128d4275= 128.0/4275.0;
	const double C2197d75240= 2197.0/75240.0;
	const double C1d50=1/50;

	this->fh=h;
	this->Max_Run=1;
	//For statistic
	FinalValueXRun=new double*[nPlaces];
	for (int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
	}


	double NextPrintTime=itime;
	ofstream out;

	if (Info)
	{
		out.open(string(argv)+".trace",ofstream::out);
		out.precision(16);
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


	cout<<"Time:"<<time<<"(Step: "<<Print_Step<<")"<<endl;
	if (Info)
		out<<time;
	for (int i=0; i<nPlaces; i++)
	{
		if (Info)
			out<<" "<<Value[i];
		ValuePrv[i]=Value[i];
	}
	if (Info)
		out<<endl;
	NextPrintTime+=Print_Step;
	if ((Info)&&(h>NextPrintTime))
		h=Print_Step;
	int current_attempt=0;

	while (time<Max_Time)
	{

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

		double error=0.0,tmp_error_ass;
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


			if ((Info) && (time==NextPrintTime))
				out<<time;
			for (int i=0; i<nPlaces; i++){
				if ((Info) && (time==NextPrintTime))
					out<<" "<<Value[i];
				ValuePrv[i]=Value[i];
			}
			if(time==NextPrintTime){
				cout<<"\tTime:"<<time<<"(Step: "<<Print_Step<<", Last_Step:"<<h<<")"<<endl;
				NextPrintTime+=Print_Step;
				if (Info)
					out<<endl;
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
		//cout<<"update error:"<<error<<" "<<perc1/error<<" "<<fabs(perc1/error)<<" "<<(double)std::pow((double)fabs(perc1/(error)), 0.25)<<endl;
		// Calculate new step-length
		double delta  = 0.84 *std::pow((double )fabs(perc1/(error)), 0.25);
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
				if ((time+h>NextPrintTime)){
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
inline void  SystEq::StepODERK5( double* Yn, double* Kn, const double& h){

	//compute transition enable value in Yn
	getValTranFire(Yn);

	unsigned int i=headDirc;
	while (i!=DEFAULT)
	{
		Kn[i]=0.0;
		for (int j=0; j<VEq[i].getSize();j++)// for all components
		{
			double val=0.0;
			if ((!NotEnable(VEq[i].getIdTrans(j)))&&((val=EnabledTransValueCon[VEq[i].getIdTrans(j)])>0))//if transition is not disable by inhibitor arc
			{
				Kn[i]+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*val;
			}
		}
		Kn[i]*=h;
		i=VEq[i].getNext();//next place that is computed directly
	}
	//derived(Kn);// it derives all the rest of places. It is not needed here!!!
}

/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It computes a single step of Kutta-Merson integration*/
/**************************************************************/
void SystEq::fex(double t, double *y, double *ydot, void *data){

	//compute transition enable value in Yn
	derived(y);
	time=t;
	getValTranFire(y);
	unsigned int i=headDirc;
	while (i!=DEFAULT)
	{
		ydot[i]=0.0;
		for (int j=0; j<VEq[i].getSize();j++)// for all components
		{
			double val=0.0;
			if ((!NotEnable(VEq[i].getIdTrans(j)))&&((val=EnabledTransValueCon[VEq[i].getIdTrans(j)])>0))//if transition is not disable by inhibitor arc
			{
				ydot[i]+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*val;//observe that rate==1 for general transition
			}
		}
		i=VEq[i].getNext();//next place that is computed directly
	}
	//derived(ydot,y);// it derives all the rest of places
}


/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It solves the ODE system using  LSODA method*/
/**************************************************************/
void SystEq::SolveLSODE(double h,double perc1,double perc2,double Max_Time,bool Info,double Print_Step,char *argv){


	double          rwork1, rwork5, rwork6, rwork7;
	double          atol[nPlaces+1], rtol[nPlaces+1], y[nPlaces+1], t, tout=Print_Step+itime;
	int             iwork1, iwork2, iwork5, iwork6, iwork7, iwork8, iwork9;
	int             neq = nPlaces;
	int             itol=2, itask=1, istate=1, iopt=0, jt=2;

	iwork1 = iwork2 = iwork5 = iwork6 = iwork7 = iwork8 = iwork9 = 0;
	rwork1 = rwork5 = rwork6 = rwork7 = 0.0;


	this->Max_Run=0;
	solve = Solve_LSODE;
	//For statistic
	FinalValueXRun=new double*[nPlaces];
	for (int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
	}


	ofstream out;
#ifdef CGLPK
	vector <ofstream> outflux(vec_fluxb.size());	
#endif
	if (Info)
	{
		out.open(string(argv)+".trace",ofstream::out);
		out.precision(16);
		if(!out)
		{
			throw Exception("*****Error opening output file storing TRACE *****\n\n");

		}
		out<<"Time";

#ifdef CGLPK
		for (unsigned int i=0;i<vec_fluxb.size();++i){
			outflux[i].open(string(argv)+"-"+to_string(i)+".flux",ofstream::out);
			outflux[i].precision(16);
			if(!outflux[i]){
				throw Exception("*****Error opening output file storing FLUXES*****\n\n");
			}
			outflux[i]<<"Time"<<" Obj_"<<i;
		}	
#endif

		for (int i=0;i<nPlaces;i++)
			out<<" "<<NamePlaces[i];
#ifdef CGLPK
		for (unsigned int i=0;i<vec_fluxb.size();++i){
			vec_fluxb[i].printFluxName(outflux[i]);		
			if (Variability){
				vec_fluxb[i].printFluxNameMinMax(outflux[i]);	
			}
		}
#endif

		out<<endl<<itime<<" ";
	}
	cout.precision(16);

	//initialize time
	t=itime;

	y[0]=0.0;

	for (int j=1;j<=nPlaces;j++){
		y[j]=Value[j-1];
		if (Info){
			out<<" "<<y[j];
		}
		atol[j]=perc1;
		rtol[j]=perc2;
	}

	if (Info){
#ifdef CGLPK
		getValTranFire(y+1);
		for (unsigned int i=0;i<vec_fluxb.size();++i){
			outflux[i]<<endl<<itime<<" ";
			vec_fluxb[i].printObject(outflux[i]);
			vec_fluxb[i].printValue(outflux[i]);
			if (Variability){
				vec_fluxb[i].printLowerMax(outflux[i]);	
			}
		}	
#endif
//		out<<endl;
	}
	while(tout<=Max_Time){
		lsoda(*this,neq, y, &t, tout, itol, rtol, atol, itask, &istate, iopt, jt,
			iwork1, iwork2, iwork5, iwork6, iwork7, iwork8, iwork9,
			rwork1, rwork5, rwork6, rwork7, 0);
//***********************************************************************************
//Observe istate=1 reset the integration. It assures that the next step starts really from tout however it slows the process.
//It is necessary only when you want to stop integration and we want to be sure that it will start exactly from that previous endpoint. 
		//istate=1;
//***********************************************************************************		
		if (Info){
			out<<endl<<tout<<" ";
		}
		derived(y+1);
		if (Info){
			for (int j=1;j<=nPlaces;j++){
				out<<y[j]<<" ";
			}
			

#ifdef CGLPK
			time=tout;
			getValTranFire(y+1);
			for (unsigned int i=0;i<vec_fluxb.size();++i){
				outflux[i]<<endl<<tout<<" ";
				vec_fluxb[i].printObject(outflux[i]);
				vec_fluxb[i].printValue(outflux[i]);
				if (Variability){
					vec_fluxb[i].printLowerMax(outflux[i]);	
				}
			}
#endif	
//			out<<endl;
		}		
		tout+=Print_Step;


		
		if (istate <= 0){
			throw   Exception("*****Error during the integration step*****\n\n");

		}
	}
	/*
	lsoda(*this,neq, y, &t, Max_Time, itol, rtol, atol, itask, &istate, iopt, jt,
					iwork1, iwork2, iwork5, iwork6, iwork7, iwork8, iwork9,
					rwork1, rwork5, rwork6, rwork7, 0);
	if (Info){
		out<<Max_Time<<" ";
	}
	cout<<"\t Time:"<<Max_Time<<endl;
	derived(y+1);
	if (Info){
        for (int j=1;j<=nPlaces;j++){
			out<<y[j]<<" ";
		}
		out<<endl;
	}
	if (istate <= 0){
		throw   Exception("*****Error during the integration step*****\n\n");

	}
*/ 
	cout<<"\nSolution at time "<<Max_Time<<":\n";
	for (int i=0;i<nPlaces;i++)//store resul ode and print final time for each trace
	{
		FinalValueXRun[i][0]= y[i+1];
		cout<<"\t"<<NamePlaces[i]<<": "<<y[i+1]<<"\n";
	}
	cout<<endl;
}

/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It computes the Tau. It requires that  getValTranFire() must be called before.*/
/**************************************************************/
int SystEq::getComputeTau(int SetTranExp[], double& nextTimePoint,double t){

	double tau=0.0;
	int lo = 0;
	double general_tau = 0.0;
	double sumRate=0.0;
	int size = 0;


	if (SetTranExp[0]!=0)
	{
		size= SetTranExp[0];

		for (int i=1;i<=size;++i){

			if (EnabledTransValueDis[SetTranExp[i]]!=0){
            /*It is not necessary since rate of generic transition is one by default!!!
            if (Trans[SetTran[i]].GenFun==""){
                sumRate+=EnabledTransValueDis[SetTran[i]]*Trans[SetTran[i]].rate;

                }
            else{
                sumRate+=EnabledTransValueDis[SetTran[i]];
               // cout<<"trans "<<NameTrans[SetTran[i]]<<" "<<EnabledTransValueDis[SetTran[i]]<<endl;
                }

           /// cout<<NameTrans[i]<<" "<<sumRate<<endl;

                   */
				sumRate+=EnabledTransValueDis[SetTranExp[i]]*Trans[SetTranExp[i]].rate;

			}

			TransRate[i-1]=sumRate;
		//cout<<"i "<< i<<" sumRate:"<<sumRate<<NameTrans[SetTranExp[i]]<<endl;
		}

	}

	if (sumRate==0.0 && future_event_list.getLenght() == 0)
		return -1;
	std::exponential_distribution<> ExpD(sumRate);

	tau=(ExpD(generator)+t);
    //cout<<"\nReaction:"<<tau<<" "<<tau-t<<endl;

	if(future_event_list.getLenght() > 0) {
		general_tau = future_event_list.topWeight();
		if(general_tau<tau){
			if(general_tau >= nextTimePoint)
				return -1;
			Event* top_list = future_event_list.popHeap();
			lo = top_list -> getIndexTran(); 
			deleteEvent(top_list);
			future_event_list.removeHeap(top_list);
			Trans[lo].events_size--;
			nextTimePoint = general_tau;
			return lo;			
		}
	}

	if(tau >= nextTimePoint)
		return -1;
	nextTimePoint=tau;
	std::uniform_real_distribution<> UnfRealD(0.0,1.0);
	double val=UnfRealD(generator)*sumRate;

    	//int trans=1;
   		// cout<<"\nval "<<val<<endl;

	int hi = size - 1;

	while (lo < hi) {
		int mid = lo + (hi - lo)/2;
		if (TransRate[mid] < val)
			lo = mid + 1;
		else
			hi = mid;
	}
		//}

	return SetTranExp[lo+1];
}

/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It computes the sixth-order approximation of the derivative according to the Richardson's Extrapolation formula.*/
/**************************************************************/

double SystEq::RichardsonExtrap(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, double hstep, int ider )
{
	double ValuePrv_tmp =  ValuePrv[ider];

    // let's calculate the first part of the formula with h
	ValuePrv[ider] =  ValuePrv[ider] + hstep;
	// cout << "hstep = " << hstep << " ValuePrv_tmp = " << ValuePrv_tmp <<"; ValuePrv = " << ValuePrv[ider] <<"\n";
//	double fh = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#ifdef CGLPK
     //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
	double fh = Trans[T].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#else
	double fh = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#endif 	

	ValuePrv[ider] = ValuePrv[ider] + hstep ;
	// cout << "hstep = " << hstep << " ValuePrv_tmp = " << ValuePrv_tmp <<"; ValuePrv = " << ValuePrv[ider] <<"\n";
	//double f2h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
	double f2h = Trans[T].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#else
	double f2h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#endif 	

	ValuePrv[ider] = ValuePrv_tmp - hstep;
	//double fmh = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
	double fmh = Trans[T].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#else
	double fmh = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#endif 	


	ValuePrv[ider] = ValuePrv[ider] - hstep;
//	double fm2h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
	double fm2h = Trans[T].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#else
	double fm2h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#endif 	

	double s1 = (- f2h + 8*fh - 8*fmh + fm2h) / (12*hstep) ;

	// let's calculate the second part of the formula with 2h
	ValuePrv[ider] = ValuePrv_tmp + hstep + hstep + hstep + hstep;
//
//	double f4h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
	double f4h = Trans[T].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#else
	double f4h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#endif 	  

	ValuePrv[ider] = ValuePrv_tmp - hstep - hstep - hstep - hstep;
//	double fm4h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#ifdef CGLPK
 //!If CGLPK is defined then the vector of pointers to flux balance problems is passed as input parameter.
	double fm4h = Trans[T].FuncT(ValuePrv,vec_fluxb,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#else
	double fm4h = Trans[T].FuncT(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time);
#endif 	

	double s2 = (- f4h + 8*f2h - 8*fm2h + fm4h) / (12*(hstep + hstep)) ;

	// Set the value of ValuePrv as at the beginning
	ValuePrv[ider] = ValuePrv_tmp;
/* cout << " fh = " << fh << "; f2h= " << f2h << "; f4h= " << f4h <<"\n";
cout << " fmh = " << fmh << "; fm2h= " << fm2h  << "; fm4h= " << fm4h  <<"\n";
cout << " s1 = " << s1 << "; s2= " << s2 <<"\n";*/
	return (16*s1 - s2 ) / 15 ;

}
/*
double SystEq::S(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, int hstep, int ider)
{

	double fh = f(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time,hstep,ider);
	double f2h =f(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time,hstep+hstep,ider);
	double fmh =f(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time,hstep,ider, true);
	double fm2h =f(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time,hstep+hstep,ider, true);

	return (- f2h + 8*fh - 8*fmh + fm2h) / 12*hstep ;
}

double SystEq::L(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, int hstep, int ider)
{
	return (16*S(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time,hstep,ider) - S(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time,hstep+hstep,ider) ) / 15 ;
}


double SystEq::DerivApproximation(double *ValuePrv, map <string,int>& NumTrans, map <string,int>& NumPlaces,const vector<string> & NameTrans, const struct InfTr* Trans, const int T, const double& time, int hstep, int ider)
{
	// I will write the while here;

	return RichardsonExtrap(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,T,time,hstep,ider);
}
*/

/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It computes the Tau according to Gillespie algorithm. It requires that  getValTranFire() must be called before.*/
/**************************************************************/

double SystEq::getComputeTauGillespie(int SetTran[],double t, double hstep){

	bool first=true; //boolean value if it is the first time we call this function -> this allow the computation only one single time of the total propensity a_0
	double a_0=0;
	//double eps =0.1;
	double tau=t;
	//double f=0;//
	if (SetTran[0]!=0){

		int size= SetTran[0];//number of stochastic transitions

		if(first==true){
			for(int h=1;h<=size;h++){

            /*It is not necessary since rate of generic transition is one by default!!!

            if (Trans[SetTran[h]].GenFun==""){
                    a_0+=EnabledTransValueDis[SetTran[h]]*Trans[SetTran[h]].rate;
                }
            else
                {
                    a_0+=EnabledTransValueDis[SetTran[h]];
                }
            */
				a_0+=EnabledTransValueDis[SetTran[h]]*Trans[SetTran[h]].rate;
			}
			first=false;
		}


		for (int k=1;k<=size;k++){// fixing a specific transition j among the stochastic ones (SetTran)
			int j=SetTran[k];
			if ((EnabledTransValueDis[j]!=0)){
                //if (Trans[j].FuncT==nullptr){
                    int N=Trans[j].InPlaces.size();// numero specie coinvolte nella reazione j-esima
                    double mu=0;
                    double sigma2=0;
                    double tmp;
                    int idp =0;
                    for(int i=0;i<N;i++){
                    	idp=Trans[j].InPlaces[i].Id;
                        if(Trans[j].GenFun!=""){// check if j is a generic transition
                        	DerivTAUG[idp]=RichardsonExtrap(ValuePrv,NumTrans,NumPlaces,NameTrans,Trans,j,time,hstep,idp);
						// cout << "time = " << t << " Val= "<< ValuePrv[idp] << " DerivGen = " << DerivTAUG[idp] << "\n";
                        }
                        else{
                        	DerivTAUG[idp]=(EnabledTransValueDis[j]*Trans[j].rate)*(Trans[j].InPlaces[i].Card/ValuePrv[idp]);
						// cout << "time = " << t << " Val= "<< ValuePrv[idp] << " Deriv = " << DerivTAUG[idp] << "\n";
                        }

                    }

                    for (int kk=1;kk<=size;kk++)//all the transitions
                    {
                    	int jp=SetTran[kk];
                    	double f=0;
                    	if (EnabledTransValueDis[jp]!=0){
                    		for(int i=0;i<N;i++){
                    			idp=Trans[j].InPlaces[i].Id;
                    			f+=DerivTAUG[idp]*VEq[idp].getIncDecTran(jp);
                    		}
                    		mu+=f*EnabledTransValueDis[jp]*Trans[jp].rate;
                    		sigma2+=f*f*EnabledTransValueDis[jp]*Trans[jp].rate;
                    	}
                    }
                    tmp=min(epsTAU*a_0/fabs(mu),epsTAU*a_0*epsTAU*a_0/sigma2);//}
                    if (tmp<tau){
                    	tau=tmp;
                    }
                }
            }
        }
        else{
        	throw Exception("*****There are no possible reactions, hence the time step is null*****\n\n");
        	tau=0;
        }
        return tau;
    }


/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It solves the ODE system using  LSODA method*/
/**************************************************************/
    void SystEq::SolveHLSODE(double h,double perc1,double perc2,double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv){



    	double          rwork1, rwork5, rwork6, rwork7;
    	double          atol[nPlaces+1], rtol[nPlaces+1], y[nPlaces+1], t, tout=Print_Step+itime;
    	int             iwork1, iwork2, iwork5, iwork6, iwork7, iwork8, iwork9;
    	int             neq = nPlaces;
    	int             itol=2, itask=1, istate=1, iopt=0, jt=2;

    	iwork1 = iwork2 = iwork5 = iwork6 = iwork7 = iwork8 = iwork9 = 0;
    	rwork1 = rwork5 = rwork6 = rwork7 = 0.0;


    	this->Max_Run=Max_Run;
	//For statistic
    	solve = Solve_HLSODE;
    	FinalValueXRun=new double*[nPlaces];
    	double Mean[nPlaces];
    	std::fill(Mean, Mean+nPlaces, 0.0);

	//For negative marking
    	double ValuePrev[nPlaces+1];
    	std::fill(ValuePrev, ValuePrev+nPlaces+1, 0.0);


    	cout<<endl<<"Seed value: "<<seed<<endl<<endl;

    	ofstream out;
    	if (Info)
    	{
    		out.open(string(argv)+".trace",ofstream::out);
    		out.precision(16);
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

    	for (int i=0;i<nPlaces;i++)
    	{
    		FinalValueXRun[i]=new double[Max_Run+1];
    		for (int j=0;j<Max_Run+1;j++)
    			FinalValueXRun[i][j]=0.0;
    		atol[i]=perc1;
    		rtol[i]=perc2;
    	}
    	atol[nPlaces]=perc1;
    	rtol[nPlaces]=perc2;


    	int SetTran[nTrans+1];
    	std::fill(SetTran, SetTran + nTrans + 1, 0);


//disable discrite transition from fluid computation
    	for (int i=0;i<nTrans;i++)
    	{
    		if (Trans[i].discrete){
    			SetTran[++SetTran[0]]=i;
    			Trans[i].enable=false;
    		}
    	}

    	int run=0;







    	while (run<Max_Run){

    		if (run%100==0){
    			cout<<"\r\t START RUN..."<<run<<" ";
    			cout.flush();
    		}
		//Initialization for each run
    		ValuePrev[0]=y[0]=0.0;

    		if (Info){
    			out<<itime;
    		}
    		for (int j=1;j<=nPlaces;j++){
    			y[j]=ValuePrev[j]=Value[j-1];
    			if (Info){
    				out<<" "<<y[j];
    			}
    		}
    		if (Info){
    			out<<endl;;
    		}

    		double nextTimePoint=tout=Print_Step+itime;
    		istate=1;
    		t=itime;

    		bool neg=false;
    		double tmpt=t;

    		while(t<=Max_Time){
    			if (!neg){
    				time=nextTimePoint;
    				getValTranFire(y+1);

    			}
    			neg=false;
        //compute tau
    			int Tran=getComputeTau(SetTran,nextTimePoint,t);
/*
            cout<<"TIME:"<<nextTimePoint<<" ";
            if (Tran!=-1)
            cout<<NameTrans[Tran]<<endl;
            else
            cout<<endl;
*/


        //tmpt=t;
    			lsoda(*this,neq, y, &t, nextTimePoint, itol, rtol, atol, itask, &istate, iopt, jt,
    				iwork1, iwork2, iwork5, iwork6, iwork7, iwork8, iwork9,
    				rwork1, rwork5, rwork6, rwork7, 0);
        //t=nextTimePoint;
            //check if the selected descrete transition can fire (no negative markings)

    			if (Tran!=-1){
    				int size=(Trans[Tran].Places).size();
    				for (int i=0;i<size&&!neg;++i){
    					y[(Trans[Tran].Places[i]).Id+1]+=(Trans[Tran].Places[i]).Card;
    					if   (y[(Trans[Tran].Places[i]).Id+1]<0)
    						neg=true;
    				}
    				istate=1;
    			}
    			if (neg){
    				for (int j=0;j<=nPlaces;j++){
    					y[j]=ValuePrev[j];
    				}
    				t=tmpt;
    				nextTimePoint= (tmpt+(nextTimePoint-tmpt)/2);
    			}
    			else
    			{
    				for (int j=0;j<=nPlaces;j++){
    					ValuePrev[j]=y[j];
    				}
    				tmpt=t;
    				derived(y+1);

    				if (tout==nextTimePoint){
    					if (Info){
    						out<<nextTimePoint;
    						for (int j=1;j<=nPlaces;j++){
    							out<<" "<<y[j];
    						}
    						out<<endl;
    					}

    					nextTimePoint=(tout+=Print_Step);
    				}
    				else
    					nextTimePoint=tout;
    			}

    			if (istate <= 0){
    				throw   Exception("*****Error during the integration step*****\n\n");

    			}
    		}

        for (int i=0;i<nPlaces;i++)//store resul ode
        {
        	Mean[i]+=FinalValueXRun[i][run]= y[i+1];
        }

        ++run;
    }

	//Print final time for each trace
    cout<<"\n\nSolution at time "<<Max_Time<<":\n";
	for (int i=0;i<nPlaces;i++)//save initial state
	{
		cout<<"\t"<<NamePlaces[i]<<"~= "<<Mean[i]/run<<"\n";
	}
	cout<<endl;

}




/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It solves the ODE system using  SSA method*/
/**************************************************************/
void SystEq::SolveSSA(double h,double perc1,double perc2,double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv){


	this-> Max_Run=Max_Run;
	FinalValueXRun = new double*[nPlaces];
	double Mean[nPlaces];
	std::fill(Mean, Mean + nPlaces, 0.0);
	double tout;
	solve = Solve_SSA;
	int size_expTran = 0;
	int size_notExpTran = 1;
	int Tran_previous = -1;

	setSizeFutureEventList(nTrans, size_expTran, size_notExpTran);

	//double ValuePrev[nPlaces] {0.0};

	double ValueInit[nPlaces];

	cout<<endl<<"Seed value: "<<seed<<endl<<endl;


	ofstream out;
#ifdef CGLPK
	vector <ofstream> outflux(vec_fluxb.size());	
#endif
	if (Info)
	{
		out.open(string(argv)+".trace",ofstream::out);
		out.precision(16);
		if(!out){
			throw Exception("*****Error opening output file***\n\n");
		}
		out<<"Time";

#ifdef CGLPK
		for (unsigned int i=0;i<vec_fluxb.size();++i){
			outflux[i].open(string(argv)+to_string(i)+".flux",ofstream::out);
			outflux[i].precision(16);
			if(!outflux[i]){
				throw Exception("*****Error opening output file storing FLUXES*****\n\n");
			}
			outflux[i]<<"Time"<<" Obj_"<<i;
		}	
#endif

		for(int i=0;i<nPlaces;i++)
			out<<" "<<NamePlaces[i];
#ifdef CGLPK
		for (unsigned int i=0;i<vec_fluxb.size();++i){
			vec_fluxb[i].printFluxName(outflux[i]);		
			if (Variability){
				vec_fluxb[i].printFluxNameMinMax(outflux[i]);	
			}
		}	
#endif
		out<<endl;
	}

	cout.precision(16);
	for(int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for(int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
		ValueInit[i]=Value[i];
	}


	//int SetTran[nTrans+1];
	int SetTranExp[size_expTran+1];
	std::fill(SetTranExp, SetTranExp + size_expTran + 1, 0);
	int SetTranNotExp[size_notExpTran+1];
	std::fill(SetTranNotExp, SetTranNotExp + size_notExpTran + 1, 0);

	//defining exp/not exp transition list
	for(int i=0;i<nTrans;i++)
	{
	//All transitions
		if (Trans[i].timing == NON_EXP_GEN){
			SetTranNotExp[++SetTranNotExp[0]]=i; 
		}
		else 
		SetTranExp[++SetTranExp[0]]=i;
	}

	int run=0;

	while (run<Max_Run){


		if(run%100==0){
			cout<<"\r\t START RUN... "<<run<<" ";
			cout.flush();
		}

		//Initialization for each run
		if(Info){
			out<<endl<<itime;
		}


		for (int j=0;j<nPlaces;j++){
			Value[j]=ValuePrv[j]=ValueInit[j];
			if(Info){
				out<<" "<<ValuePrv[j];
			}
		}

		if(Info){
#ifdef CGLPK
			getValTranFire(Value);
			for (unsigned int i=0;i<vec_fluxb.size();++i){
				outflux[i]<<endl<<itime<<" ";
				vec_fluxb[i].printObject(outflux[i]);
				vec_fluxb[i].printValue(outflux[i]);
				if (Variability){
					vec_fluxb[i].printLowerMax(outflux[i]);	
				}
			}	
#endif
			out<<endl;
		}

		double nextTimePoint=tout=Print_Step+itime;

		//istate=1;
		time=itime;

		while(time<=Max_Time){


            //time=nextTimePoint;
			getValTranFire(Value);

		//update of future event list
			for(int t=1;t<size_notExpTran;t++)
			{
				updateFutureEventList(SetTranNotExp[t], Tran_previous, time);
			}

      		//this returns the correct transition to fire
			int Tran=getComputeTau(SetTranExp, nextTimePoint,time);
			Tran_previous = Tran;


      //cout<<Tran<<" "<<endl;
			if (Tran!=-1){
				int size=(Trans[Tran].Places).size();
				for (int i=0;i<size;++i){
					Value[(Trans[Tran].Places[i]).Id]+=(Trans[Tran].Places[i]).Card;
				}
			}

			time=nextTimePoint;
			if(tout==nextTimePoint){
				if(Info){

					out<<nextTimePoint;
					for(int j=0; j<nPlaces;j++){

						out<<" "<<Value[j];
					}
#ifdef CGLPK

					for (unsigned int i=0;i<vec_fluxb.size();++i){
						outflux[i]<<endl<<tout<<" ";
						vec_fluxb[i].printObject(outflux[i]);
						vec_fluxb[i].printValue(outflux[i]);
						if (Variability){
							vec_fluxb[i].printLowerMax(outflux[i]);	
						}
					}	
#endif
					out<<endl;
				}
				nextTimePoint=(tout+=Print_Step);
			}
			else
				nextTimePoint=tout;

		}

		for (int i=0;i<nPlaces;i++)
		{
			Mean[i]+=FinalValueXRun[i][run]=Value[i];
		}

		Tran_previous = -1;
		//initialization for future event list
		if(future_event_list.getLenght() > 0){

			future_event_list.deleteHeap();

			for(int t=1; t<size_notExpTran; t++){
				PreviousEnabledTransValueDis[t] = 0;
				Trans[SetTranNotExp[t]].first_event = NULL;
				Trans[SetTranNotExp[t]].events_size = 0;
			}

		}

		++run;

	}

	future_event_list.deleteHeap();

	cout<<"\n\nSolution at time "<<Max_Time<<":\n";
}





/**************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It solves the ODE system using  Tau-leaping method*/
/**************************************************************/
void SystEq::SolveTAUG(double Max_Time,int Max_Run,bool Info,double Print_Step,char *argv){


	this-> Max_Run=Max_Run;
	FinalValueXRun = new double*[nPlaces];
	double Mean[nPlaces];
	std::fill(Mean, Mean + nPlaces, 0.0);
	solve = Solve_TAUG;
	//double tout;

	//double ValuePrev[nPlaces] {0.0};


	double ValueInit[nPlaces];

	int firing[nTrans];
	std::fill(firing, firing + nTrans, 0);
	cout<<endl<<"Seed value: "<<seed;
	cout<<endl<<"Epsilon TAU-leaping: "<<epsTAU<<endl<<endl;

	ofstream out;
#ifdef CGLPK
	vector <ofstream> outflux(vec_fluxb.size());	
#endif
	if (Info)
	{
		out.open(string(argv)+".trace",ofstream::out);
		out.precision(16);
		if(!out){
			throw Exception("*****Error opening output file***\n\n");
		}
		out<<"Time";
#ifdef CGLPK
		for (unsigned int i=0;i<vec_fluxb.size();++i){
			outflux[i].open(string(argv)+to_string(i)+".flux",ofstream::out);
			outflux[i].precision(16);
			if(!outflux[i]){
				throw Exception("*****Error opening output file storing FLUXES*****\n\n");
			}
			outflux[i]<<"Time"<<" Obj_"<<i;
		}	
#endif		
		for(int i=0;i<nPlaces;i++)
			out<<" "<<NamePlaces[i];	
#ifdef CGLPK
		for (unsigned int i=0;i<vec_fluxb.size();++i){
			vec_fluxb[i].printFluxName(outflux[i]);		
			if (Variability){
				vec_fluxb[i].printFluxNameMinMax(outflux[i]);	
			}
		}
#endif		
//		out<<endl;
	}

	cout.precision(16);
	for(int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		for(int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
		ValueInit[i]=Value[i];
	}


	int SetTran[nTrans+1];
	std::fill(SetTran, SetTran + nTrans + 1, 0);
	//disable discrete transition from fluid computation
	for(int i=0;i<nTrans;i++)
	{
	//All transitions
	//if (Trans[i].discrete){
		SetTran[++SetTran[0]]=i;
		Trans[i].enable=false;
		//}
	}

	int run=0;
	double hstep = 0.00001;

	while (run<Max_Run){

		if(run%100==0){
			cout<<"\r\t START RUN..."<<run<<" ";
			cout.flush();
		}
		//Initialization for each run
		if(Info){
			out<<endl<<itime;
		}

		for (int j=0;j<nPlaces;j++){
			ValuePrv[j]=ValueInit[j];
			if(Info){
				out<<" "<<ValuePrv[j];
			}
		}
		if(Info){
#ifdef CGLPK
			getValTranFire(Value);
			for (unsigned int i=0;i<vec_fluxb.size();++i){
				outflux[i]<<endl<<itime<<" ";
				vec_fluxb[i].printObject(outflux[i]);
				vec_fluxb[i].printValue(outflux[i]);
				if (Variability){
					vec_fluxb[i].printLowerMax(outflux[i]);	
				}
			}
#endif			
			out<<endl;;
		}

		double nextTimePoint=itime,tout=Print_Step+itime;
		//istate=1;
		double t=MAX_DOUBLE,tmpt=itime;

		bool neg=false;
		DerivTAUG = new double[nPlaces];

		while(nextTimePoint<=Max_Time){

			if(!neg){
				time=nextTimePoint;
				getValTranFire();
			}

			neg=false;

		//compute tau

			double tau=getComputeTauGillespie(SetTran,t, hstep);

			nextTimePoint+=tau;
			if (nextTimePoint>tout){
				tau=tout-tmpt;
				nextTimePoint=tout;
			}

			//cout<<"Tau: "<< tau<<endl;
			//cout<<"TIME:"<<nextTimePoint<<endl;

			if(tau==-1){
				throw Exception("*****Error during the tau computation*****\n\n");

			}




			for (int i=0;i<nTrans;i++){//oggi i=1 old
				//oggi if(EnabledTransValueDis[i]!=0){
				if(EnabledTransValueDis[i]!=0){
					//oggi std::poisson_distribution<>PoisD(tau*EnabledTransValueDis[i]*Trans[i].rate);
					if (Trans[i].GenFun==""){
						std::poisson_distribution<>PoisD(tau*EnabledTransValueDis[i]*Trans[i].rate);
						firing[i]=PoisD(generator);
					}
					else{
						std::poisson_distribution<>PoisD(tau*EnabledTransValueDis[i]);
                          firing[i]=PoisD(generator);//oggi
                      }
                  }
                  else
					firing[i]=0;//oggi
			//cout<<"Firing: "<<firing[i];
			}
			//cout<<endl;

			unsigned int i = headDirc;
			while(i!=DEFAULT && !neg)
			{
				double tmpvalSIM=0.0;
				for(int j=0; j<VEq[i].getSize();j++)//for all components
				{
					//if(!NotEnable(VEq[i].getIdTrans(j)))
					//{
					//	tmpvalSIM+=VEq[i].getIncDec(j)*firing[VEq[i].getIdTrans(j)];
					//}
					if(firing[VEq[i].getIdTrans(j)]!=0)
					{
						tmpvalSIM+=VEq[i].getIncDec(j)*firing[VEq[i].getIdTrans(j)];
					}

				}//for all components
				if (ValuePrv[i]+tmpvalSIM<0){
					neg=true;
				}
				else{
					Value[i]=ValuePrv[i]+tmpvalSIM;
					i=VEq[i].getNext();


				}


			}
			if (!neg)
                derived(neg);//it derives all the rest of places
            if(neg){
            	t=tau=tau/2;
            	nextTimePoint=tmpt;
            }
            else
            {
            	t=MAX_DOUBLE;
            	for(int j=0;j<=nPlaces;j++){
            		ValuePrv[j]=Value[j];
            	}
				//tmpt=t;
            	if(tout==nextTimePoint){
            		if(Info){

            			out<<endl<<nextTimePoint;
            			for(int j=0; j<nPlaces;j++){

            				out<<" "<<Value[j];
            			}
#ifdef CGLPK
            			for (unsigned int i=0;i<vec_fluxb.size();++i){
            				outflux[i]<<endl<<tout<<" ";
            				vec_fluxb[i].printObject(outflux[i]);
            				vec_fluxb[i].printValue(outflux[i]);
            				if (Variability){
            					vec_fluxb[i].printLowerMax(outflux[i]);	
            				}
            			}
#endif						
//					out<<endl;
            		}
            		tout+=Print_Step;
            	}
            	tmpt=nextTimePoint;

            }

        }

        for (int i=0;i<nPlaces;i++)
        {
        	Mean[i]+=FinalValueXRun[i][run]=Value[i];
        }

        ++run;

    }
}




/***************************************************************/
/* NAME :  Class SystEq*/
/* DESCRIPTION : It estimates the integration step for Euler method*/
/**************************************************************/

void  SystEq::HeuristicStep(double h,double perc1,double perc2,double Max_Time,bool Info,double Print_Step,char *argv){

	//assign factor for step
	double tmpval[nPlaces];
	this->fh=h;
	this->perc1=perc1;
	this->perc2=perc2;
	this->Max_Run=1;
	//For statistic
	FinalValueXRun=new  double*[nPlaces];
	double* ValueInit=(double*)malloc(sizeof(double)*nPlaces);

	for (int i=0;i<nPlaces;i++)
	{
		FinalValueXRun[i]=new double[Max_Run+1];
		ValueInit[i]=Value[i];
		for (int j=0;j<Max_Run+1;j++)
			FinalValueXRun[i][j]=0.0;
	}


	double NextPrintTime=itime;
	ofstream out;

	if (Info)
	{
		out.open(string(argv)+".trace",ofstream::out);
		out.precision(16);
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
		time=itime;
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
			double maxvalD=0.0;
			while (i!=DEFAULT)
			{
				tmpval[i]=0.0;
				for (int j=0; j<VEq[i].getSize();j++)// for all components
				{
					double val=0.0;
					if ((!NotEnable(VEq[i].getIdTrans(j)))&&((val=EnabledTransValueCon[VEq[i].getIdTrans(j)])>0))//if transition is not disable by inhibitor arc
					{
						tmpval[i]+=VEq[i].getIncDec(j)*Trans[VEq[i].getIdTrans(j)].rate*val;
					}
				}

				double increment=fabs(ValuePrv[i]-tmpval[i])/(ValuePrv[i]==0?1:ValuePrv[i]);
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
	Trans[num].discrete=T.discrete;
	Trans[num].GenFun=T.GenFun;
	Trans[num].FuncT=T.FuncT;
	Trans[num].dist[0] = std::exponential_distribution<double> (T.rate);
	Trans[num].timing = T.timing;
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


	it=T.Places.begin();
	while(it!=T.Places.end())
	{
		Trans[num].Places.push_back(*it);
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
	cout<<endl;
	for (int i=0; i<nPlaces;i++)
	{//for all places
		Freq.clear();
		for (int j=0;j<Max_Run;j++)
		{//for all run
			Freq[round(FinalValueXRun[i][j])]++;
		}

		map<int,int>::iterator it=Freq.begin();

		cout.precision(16);
		cout<<"Place "<<NamePlaces[i]<<endl;
		while(it!=Freq.end())
		{
			cout<<"\t"<<it->first<<" "<<(double)it->second/Max_Run<<endl;
			++it;
		}

	}
#endif

	ofstream out(string(argv)+".mtx",ofstream::out);
	out.precision(16);
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
	if (Trans[IdTran].GenFun!="")
		cout<<IncDec<<"*"<<Trans[IdTran].GenFun<<"()";
	else{
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
				/*
					int fatt=1;
					for (int i=2;i<=it->Card;i++){
						fatt*=i;
						cout<<"*("<<NamePlaces[it->Id]<<"-"<<i-1<<")";
					}
					cout<<"/"<<fatt;
					*/
					cout<<"^"<<it->Card<<"/"<<it->Card<<"!";
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
}


};

