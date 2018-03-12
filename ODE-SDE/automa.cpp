


#ifndef __AUT_H__
	#include "automa.hpp"
#endif 




 double next_clock_automaton=0.0;
 bool check_clock=false;
 const double epsilon=std::numeric_limits<double>::epsilon();
 extern double slow;
 extern double ep;
namespace AUTOMA {


 /*
 * ************************************************************
 *  Class logic_cond public cond
 * ************************************************************
 */
  
/*
 * class logic_cond
 * constructor implementation:
 */ 
  
   logic_cond::logic_cond (op_type p,  cond *l,  cond *r ){
     op =p;
     lcond =l;
     rcond =r;
   }
   
  
 /*
 * class logic_cond
 * print() implementation
 * prints a logical condition
 */   
  void  logic_cond::print(std::ostream &os) {
    if (op != C_NOT) {
      os<<"("; 
      lcond->print(os); 
      os<<" "<<op_to_char()<<" ";
      rcond->print(os); 
      os<<")";
      
    }
    else {
      os<<"!("; 
      lcond->print(os);
      os<<")";
      
    }
    
  }
  
 /*
 * class logic_cond
 * op_to_char() implementation
 * return the current operator name
 */  
 string logic_cond::op_to_char(){
    
    switch(op){
      case 1:
	return "&&";
      break;
      case 2:
	return "||";
      break;
      default:
	 throw  my_exception (string("Error: unknown operator type for logic condition"));
    }
  }
  

/*
 * class  logic
 * evaluate() implementation
 * evaluates the logic condition
 */   
  long double logic_cond::evaluate(const long double* value,const double* clock,const vector <double>& timers){
   
    switch(op){
      case 1: //&&
	return  (lcond->evaluate(value,clock,timers)&&rcond->evaluate(value,clock,timers));
      break;
      case 2: //||
	return  (lcond->evaluate(value,clock,timers)||rcond->evaluate(value,clock,timers));
      break;
      case 0: //!
	return !(lcond->evaluate(value,clock,timers));
      break;
      default:
	 throw  my_exception (string("Error: unknown operator type for inequality"));
    } 
  }
  
 /*
 * ************************************************************
 *  Class ineq public cond
 * ************************************************************
 */
  
/*
 * class ineq
 * constructor implementation:
 */ 
  
     ineq::ineq (op_type p,  cond *l,  cond *r ){
     op =p;
     lcond =l;
     rcond =r;
   }

 /*
 * class ineq
 * print() implementation
 * prints an inequality
 */   
    void ineq::print(std::ostream &os) { 
      os<<"("; 
      lcond->print(os); 
      os<<" "<<op_to_char()<<" "; 
      rcond->print(os); os<<")"; 
    }

/*
 * class  ineq
 * op_to_char() implementation
 * return the current operator name
 */  
 string ineq::op_to_char(){
    
    switch(op){
      case 3:
	return "<";
      break;
      case 4:
	return ">";
      break;
      case 5:
	return "<>";
      break; 
      case 10:
	return "==";
      case 11:
	return ">=";
      case 12:
	return "<=";
      default:
	 throw  my_exception (string("Error: unknown operator type for inequality"));
    }
  }    
 
/*
 * class  ineq
 * evaluate() implementation
 * evaluates the inequality
 */   
  long double  ineq::evaluate(const long double* value,const double* clock,const vector <double>& timers){
   	
    double rcondT=rcond->evaluate(value,clock,timers);
    double lcondT=lcond->evaluate(value,clock,timers);
    
    //if ((check_clock)&&(*clock<next_clock_automaton)&& (next_clock_automaton>rcondT)){
    if ((check_clock)&&(lcondT<next_clock_automaton)&& (next_clock_automaton>(rcondT+(*clock-lcondT)))){
      check_clock=false;
      next_clock_automaton=rcondT+(*clock-lcondT);
    }
    
    if ((!slow) &&(fabs(rcond-lcond)<ep))
        slow=true;
   // cout.precision(16);
   // cout<<"QUIU"<<rcondT<<" "<<lcondT<<" <="<<(lcondT<=rcondT)<<" >="<<(lcondT>=rcondT)<<endl;
    switch(op){
      case 3://<
	return  lcondT<rcondT;
      break;
      case 4://>
	return  lcondT>rcondT;
      break;
      case 5: //<>
	return  (fabs(lcondT-rcondT) > epsilon);//lcondT!=rcondT;
      break;
      case 10: //==
	return  (fabs(lcondT-rcondT) < epsilon); //lcondT==rcondT;
      break;
      case 11: //>=
        //  cout.precision(16);
         // cout<<"Test>="<<lcondT<<" "<<(lcondT>rcondT)<<" "<<(fabs(lcondT-rcondT) < epsilon)<<endl;
	return (lcondT>rcondT || fabs(lcondT-rcondT) < epsilon);//lcondT>=rcondT;
      break;
      case 12: //<=
	return (lcondT<rcondT || fabs(lcondT-rcondT) < epsilon);
      break;      default:
	 throw  my_exception (string("Error: unknown operator type for inequality"));
    }
    
  }
 
 
/*
 * ************************************************************
 *  Class exp public cond
 * ************************************************************
 */
  
/*
 * class expr
 * constructor implementation:
 */ 
  
     expr::expr (op_type p,  cond *l,  cond *r ){
     op =p;
     lcond =l;
     rcond =r;
   }
   
/*
 * class expr
 * print() implementation
 * prints an expression
 */   
     void expr::print(std::ostream &os) { 
       os<<"("; 
       lcond->print(os); 
       os<<" "<<op_to_char()<<" ";
       rcond->print(os); os<<")";  
    }

/*
 * class  expr
 * op_to_char() implementation
 * returns the current operator name
 */  
  string expr::op_to_char(){
    
    switch(op){
      case 6:
	return "+";
      break;
      case 7:
	return "-";
      break;
      case 8:
	return "*";
      break;
      case 9:
	return "/";
      break;
      default:
	 throw  my_exception (string("Error: unknown operator type for expression"));
    }
  }
  
 /*
 * class  expr
 * evaluate() implementation
 * evaluates the expression
 */   
  long double  expr::evaluate(const long double* value, const double* clock,const vector <double>& timers){
   
    switch(op){
      case 6:
	return  lcond->evaluate(value,clock,timers)+rcond->evaluate(value,clock,timers);
      break;
      case 7:
	return  lcond->evaluate(value,clock,timers)-rcond->evaluate(value,clock,timers);
      break;
      case 8:
	return lcond->evaluate(value,clock,timers)*rcond->evaluate(value,clock,timers);
      break;
      case 9:
	return lcond->evaluate(value,clock,timers)/rcond->evaluate(value,clock,timers);
      break;
      default:
	 throw  my_exception (string("Error: unknown operator type for expression"));
    }
    
  }
  
  
 /*
 * ************************************************************
 *  Class term_var public cond
 * ************************************************************
 */ 
 
 
 /*
 * class term_var
 * evaluate() implementation
 * evaluates the expression
 */ 
 long double term_var::evaluate(const long double* value,const double* clock, const vector <double>& timers){
     
    switch (v_type) {
        case STATE:
              return value[id];  
        break;
        case CLOCK:
              check_clock=true;
              return timers[id];
        break;
        default:
        throw  my_exception (string("Error: unknown variable type"));
        }
        
};
/*
 * ************************************************************
 *  Class node
 * ************************************************************
 */   
    
 /*
 * class node
 * print() implementation
 * prints the node information
 */     
  
 void node::print(std::ostream &os){
 
   if (final_node)
     os<<"\t(F)\n";
   os<<"\tState condition:\n\t\t";
   if (node_condition!=nullptr)
     node_condition->print(os);
   os<<"\n\tTransitions:\n";
   auto it=node_transitions.begin();
   while (it!=node_transitions.end()){
     os<<"\t\t";
     it->print(os);
     ++it;
   }
   os<<"\n";
}
 
/*
 * class node
 * evaluate() implementation
 * returns the next automaton state 
 */     
 int node::evaluate(const long double *value,const double *clock, vector <double>& timers,const int current_node,vector <class node>& vet_node){
   
 for (auto it=node_transitions.begin();it!=node_transitions.end();++it){
   if (it->evaluate(value,clock,timers)&& (vet_node[it->get_reached_state()].node_evaluate(value,clock,timers))){
     it->reset_timer_variable(timers);//reset timers
     return it->get_reached_state();//new state
   }
 }

 //check if the automaton can remain in the current state 
 if (this->node_evaluate(value,clock,timers))
   return current_node;
 else
   return -1;
}
      
/*
 * ************************************************************
 *  Class automaton
 * ************************************************************
 */
  
/*
 * class automaton
 * add_state() implementation:
 * inserts a new node into the automaton.
 */
void automaton::add_state(string name, string type){
  
  State_string2int[name]=num_nodes; 
  vet_node.push_back(node());
  if (type.size()==1) {
      switch (type[0]){
	case '@': 
	  set_current_state(name);
	  break;
	case '#':
	  vet_node[num_nodes].set_final_node(true);
	  break;
	default:
	  throw  my_exception (string("Error: unknown state type "));
      }
    
  }
 num_nodes++; 
}

/*
 * class automaton
 * add_timer_var() implementation:
 * inserts a new timer variable into the automaton.
 */
void automaton::add_timer_var(string name){
  
  Timer_string2int[name]=timers.size(); 
  timers.push_back(0);
  
}



  
/*
 * class automaton
 * add_node_info() implementation:
 * update the node information.
 */
 void automaton::add_node_info(const string& name, class cond* p_cond, list <class trans> l_trans){
 
 if (State_string2int.find(name)==State_string2int.end()){
   throw  my_exception (string(string("Error: state "+name)+" is not defined"));
   }
 int node_num =State_string2int[name];
 vet_node[node_num].insert_node_condition(p_cond);
 vet_node[node_num].insert_node_transition(l_trans);
 }
 
/*
 * class automaton
 * print() implementation
 * print  the automaton.
 */
 void automaton::print(std::ostream &os){
   os<<"\n -------------------------------------------------------\n";
   os<<"|\t\t\tAutomaton                       |";
   os<<"\n -------------------------------------------------------\n";
   os<<"\nMapping  node name into an integer:\n";
   for (auto it= State_string2int.begin();it!=State_string2int.end();++it){
     os<<"\t$"<<it->second<<": "<<it->first;
     if (vet_node[it->second].is_final_node())
       os<<"(Final)";
     os<<endl;
   }
   os<<"\nSDE variable names:\n";
   for (int i=0;i<npl;i++){
     os<<"\tV"<<i<<": "<<places[i]<<" = "<<pr_value[i]<<endl;
   }
     
   os<<"\nCurrunt state: "<<current_state<<endl;
   
   os<<"\nTimer variables:\n";
   for (auto it=Timer_string2int.begin();it!=Timer_string2int.end();++it){
       os<<"\t@"<<it->second<<":"<<it->first<<" = "<<timers[it->second]<<"\n";
   }
   os<<"\nAutomaton description:\n";
   int i=0;
   for (auto it= vet_node.begin();it!=vet_node.end();++it){
     os<<"$"<<i++<<":"<<endl;
     it->print(os);
   }
 }

 
/*
 * class automaton
 * search_timer_variable() implementation
 *returns for the input variable name  its position in the vector storing 
 *all the variables name,  otherwise -2
 */
int automaton::search_timer_variable(const string& name){

//  if (name=="$T"){
//    return -1;
//  }
  auto it=Timer_string2int.find(name);
  if (it!=Timer_string2int.end())
      return it->second;
  else
      return -2;
}

/*
 * class automaton
 * search_state_variable() implementation
 *returns for the input variable name  its position in the vector storing 
 *all the variables name  otherwise -2
 */
int automaton::search_state_variable(const string& name){

  int i=0;
  //if (name=="$T"){
  //  return -1;
  //}
  while (i<npl){
    if (name == places[i])
      return i;
    ++i;
  }
  return -2;
}

/*
 * class automaton
 * syncr() implementation
 * computes the synchronization between ODE/SDE and automaton
 */

bool automaton::syncr(){
   //cout<<"cc:"<<current_state<<endl;
   int prv_state;
   do{
    prv_state= current_state;
    current_state=vet_node[current_state].evaluate(pr_value,pr_clock,timers,current_state,vet_node);
   }
   while((current_state!=-1)&&(prv_state!=current_state));
  //cout<<"cc:"<<current_state<<endl;
  if (current_state!=-1)
    return true;
  else
    return false;
}

/* class automaton
 * evaluate() implementation
 *  valuates the automaton state and updates it 
 */
 void automaton::evaluate(){
  if (current_state ==-1)  
    throw  my_exception (string("Error: current automaton state is not set")); 
  else
  {
    int prv_state;
    do{
    prv_state= current_state; 
    current_state=vet_node[current_state].evaluate(pr_value,pr_clock,timers,current_state,vet_node);
  
    }while(prv_state!=current_state);
   }
 }
};
