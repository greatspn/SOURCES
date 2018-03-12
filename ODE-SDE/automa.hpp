#ifndef __IOS_H__
	#define __IOS_H__
	#include <iostream>
#endif 


#ifndef __LST__
	#define __LST__
	#include <list>
#endif

#ifndef __VCT__
	#define __VCT__
	#include <vector>
#endif

#ifndef __MAP__
	#define __MAP__
	#include <map>
#endif


#ifndef __XCP__
	#define __XCP__
	#include <exception> 
#endif

#include <limits>
#include <cmath>

#define __AUT_H__

namespace AUTOMA {


  using namespace std;

  
  
   /*!
   * This virtual class represent a generic condition
   */ 
  class cond {
    public:
       virtual void print(std::ostream &os)  = 0;
       virtual long double evaluate(const long double* value,const  double* time,const vector <double>& timers)  = 0;
       /*!
	* Enumerate for operation type
	*/
       enum op_type {
        C_NOT = 0, C_AND = 1, C_OR = 2, 
	C_MINOR = 3,  C_MAJOR= 4,  C_NEQ= 5,
        C_PLUS = 6,  C_MINUS = 7, C_TIMES = 8, C_DIV = 9,
        C_EQ = 10,  C_LMAJOR= 11,  C_LMINOR= 12,
    };
      
    enum var_type {
        STATE =0, CLOCK=1, 
    };
    
  };
  
   /*!
   * This class encodes  a logic combination of two condition
   */ 
  class logic_cond: public  cond {
    
  private:
    //! It stores the left operand
    class cond *lcond {nullptr};
     //! It stores the right operand 
    class cond *rcond {nullptr};
    //! It stores the operator type
    op_type op;
    
   /*!
    * op_to_char() returns the string corresponding to the op_type
    * @return the operator name
    */
   string op_to_char();
      
  public:
    /*!
     * logic_cond () is constructor
     * @param p operator type
     * @param l left operand
     * @param r right operand
     */  
     logic_cond (op_type p,  cond *l,  cond *r = {nullptr});
     /*!
      * evaluate() returns true if the condition is satisfied otherwise false
      * @return the condition evaluation
      */
     long double evaluate(const long double* value,const double* clock,const vector <double>& timers);
     
     /*!
      * print() print the logical condition 
      * @parm output pointer
      */     
      void print(std::ostream &os);
     
     //!~logic_cond() is  destructor
      ~logic_cond(){};
  };
  
  
   /*!
   * This class extend class cond  encoding   inequality between two expression
   */ 
  class ineq : public cond {
  
    private:
    //! It stores the left operand
    cond *lcond {nullptr};
    //! It stores the right operand 
    cond *rcond {nullptr};
    //! It stores the operator type
    op_type op;
    
    /*!
     * op_to_char() returns the string corresponding to the op_type
     * @return the operator name
     */
    string op_to_char();
      
   public:
    /*!
     * ineq () is constructor
     * @param p operator type
     * @param l left operand
     * @param r right operand
     */  
     ineq(op_type p,  class cond *l,   class cond *r = {nullptr});
     
     /*!
      * evaluate() returns true if the inequality is satisfied otherwise false
      * @return the inequality evaluation
      */
      long double evaluate(const long double* value,const double* clock,const vector <double>& timers);
      
     /*!
      * print() print inequality 
      * @parm output pointer
      */ 
     void print(std::ostream &os);
      
     //!~cond() is  destructor
      ~ineq(){};
  };
 
    /*!
   * This class extend class cond  encoding  operation on expression
   */ 
  class expr : public  cond {

    private:
    //! It stores the left operand
    class cond *lcond {nullptr};
     //! It stores the right operand 
    class cond *rcond {nullptr};
    //! It stores the operator type
    op_type op;
    
   /*!
    * op_to_char() returns the string corresponding to the op_type
    * @return the operator name
    */
   string op_to_char();
    
  public:
    /*!
     * ineq () is constructor
     * @param p operator type
     * @param l left operand
     * @param r right operand
     */  
     expr (op_type p,  class cond *l,   class cond *r = {nullptr});
     
     /*!
      * evaluate() returns a double obtained by the expression evaluation
      * @return the value of the expression evaluation
      */
     long double  evaluate(const long double* value, const double* clock, const vector <double>& timers);
     
     /*!
      * print() print the expression 
      * @parm output pointer
      */ 
     void print(std::ostream &os);
      
     //!~cond() is  destructor
      ~expr(){};
  };
  
  
  
   /*!
   * This class extend class cond  encoding a variable
   */ 
  class term_var : public cond  {
   
    //! it stores the variable id
    int id;
    var_type v_type;
    
  public:
    /*!
     * term_var () is constructor
     * @param v variable id
     */  
    term_var(int v, var_type t) {id =v; v_type=t;};
      
    /*!
     * print() print variable name
     * @parm output pointer
     */ 
    inline void print(std::ostream &os) {
        switch (v_type) {
            case STATE:
                os<<"V"<<id;
            break;
            case CLOCK:
                os<<"@"<<id;
            break;    
        } 
    };
    
      /*!
      * evaluate() returns the variable name
      * @return variable values
      */
      long double evaluate(const long double* value,const double* clock, const vector <double>& timers);
     
     //!~term_var() is  destructor
    ~term_var(){};
  };
  
   /*!
   * This class extend class cond  encoding a value
   */ 
  class term_val : public  cond {
   
    //! it stores a value
    long double val;
    
  public:
    /*!
     * term_var () is constructor
     * @param v variable id
     */  
    term_val(long double v) {val =v;};
    
    /*!
     * print() print terminal value
     * @parm output pointer
     */ 
    inline void print(std::ostream &os) { os<<val;};
    inline void print() { cout<<val;};
    
      /*!
      * evaluate() returns the variable name
      * @return variable name
      */
      long double  evaluate(const long double* value, const double* clock,const vector <double>& timers) {return val;};
     
     //!~term_var() is  destructor
    ~term_val(){};
  };
  
  
 
   /*!
   * This class encodes an automaton transition.
   */ 
  class trans {
    
    //! store the current state.
    int reached_state {-1};
    //! it encodes the node condition
    class cond * transition_condition {nullptr};
    //! it encodes the timer that must be reseted by this transition
    vector<int> reseting;
    
  public:
     //! trans () is  empty constructor
      trans () {};
      
      /*!
       * trans () is constructor
       * @param st identifier of the reached state
       * @param tr transition condition
       */  
      trans (const int st, vector<int> rt,class cond* tr = {nullptr}){reached_state=st; transition_condition=tr; reseting=rt;};
      
      /*!
       * evaluate() returns the next name of automaton state or the empty string iff the transition condition 
       * or the state condition of the reached state is not satisfied.
       * @return the name of the next automaton state
       */
       bool evaluate(const long double* value,const double* clock,const vector <double>& timers) { if (transition_condition == nullptr) 
						return true; 
					   else 
						return transition_condition->evaluate(value,clock,timers);};
      /*!
       * get_reached_state() return the reached state.
       * @return the name of reached state.
       */
      inline int get_reached_state() {return reached_state;};
      
      /*!
       * print() print the node information
       * @parm output pointer
       */ 
      inline void print(std::ostream &os) { transition_condition->print(os); if (reseting.size()!=0){  
                                                                                os<< " - Reset( ";
                                                                                for (unsigned int i=0;i<reseting.size();i++) os<<"@"<<reseting[i]<<" ";
                                                                                os<< ")";
                                                                             } 
                                                                             os<<" -> $"<<reached_state<<endl;};
      
      
      /*! 
       * reset_timer_variable() takes as input timer vector, and it resets the corresponding automaton timer
       * @param reseting_list is a vector of timer
       */
      inline void reset_timer_variable(vector<double>& timers){
      
          for (auto it=reseting.begin(); it!=reseting.end(); ++it){
              timers[*it]=0.0;
        }
      }
      
      
      //!~trans() is  destructor
      ~trans (){};
  };
  
  
  /*!
   * This class encodes a automaton node.
   */ 
  class node {
    //! it is true if the node is a final node.
    bool final_node {false};
    //! it encodes the node condition
    class cond * node_condition {nullptr};
    //! it encodes the node transition as list
    list <class trans> node_transitions;
    
    public:
      //! node () is  empty constructor
      node () {};
      /*!
       * set_final_node() sets the current state.
       * @param fn is TRUE iff the current node is a final one otherwise FALSE.
       */      
      void  set_final_node(const bool fn) { final_node=fn;};
      
      /*!
       * is_final_node()check if the node is a final node.
       * @return TRUE iff  the current node is a final one otherwise FALSE.
       */
      bool is_final_node() { return final_node;};
      
      /*!
       * insert_node_condition() inserts a node condition iff node_condition is a nullptr.
       * @param condition encodes the node condition. 
       */ 
      void insert_node_condition(class cond* condition){node_condition=condition;};
      
      /*!
       * insert_node_transition() inserts a node node transition.
       * @param transition encodes a node transition. 
       */ 
      void insert_node_transition(list <class trans> transition) {node_transitions.assign(transition.begin(),transition.end());};
     
      /*!
       * node_evaluate() returns true iff the state 
       * condition of this  state is  satisfied.
       */
       bool node_evaluate(const long double *value, const double* clock,const vector <double>& timers){ if (node_condition==nullptr) 
						   return true;
						else
						   return node_condition->evaluate(value,clock,timers);};
       
       /*!
       * evaluate() returns the next automaton state
       * @parm value pointer to current SDE/ODE values
       * @parm time pointer to current SDE/ODE time
       * @parm current_node identifier to the current automaton node 
       * @return the number of the next automaton state
       */
       int evaluate(const long double *value,const double* clock, vector <double>& timers,const  int current_node,vector <class node>& vet_node);
    
      /*!
       * print() print the node information
       * @parm output pointer
       */ 
      void print(std::ostream &os);
      
      //!~node() is  destructor
      ~node (){};
  };
  
  
  /*!
   * This class encodes the automaton
   */ 
  class automaton {
  
  //! vet_node stores the automaton nodes using  vector.
  vector <class node> vet_node;
  //! State_string2int encodes the mapping between state name and its identifier
  map <string,int> State_string2int;
  //! current_state stores the current state.
  int current_state {-1};
  //! initial_state stores the initial state.
  int initial_state {-1};
  //! pr_value  is pointer to the current ODE/SDE value
  long double * pr_value {nullptr};
  //! pr_clock  is pointer to the current ODE/SDE time
  double * pr_clock {nullptr};
  //!places stores the variable names
  vector <string> places;
  //! npl stores the total number of variables
  int npl {0};
  //! num_nodes stores the total number of nodes
  int num_nodes {0};  
  
  //! State_string2int encodes the mapping between Timer Varibles name and its identifier
  map <string,int> Timer_string2int;
  //!Timers encodes the Timer Variables value
  vector <double> timers;
  public:
      //! automaton () is  empty constructor
      automaton () {};
      
      /*!
       * get_current_state() return the current state.
       * @return the name of current state.
       */
      int get_current_state() { return current_state;};
      
      
      /*!
       * set_current_state() sets the current state.
       * @param state stores the  name of the current state.
       */      
      void set_current_state(string state) { initial_state=current_state=State_string2int[state];};
      
      /*!
       * set_pr_value() sets a pointer to the current ODE/SDE values.
       * @param p_v points to the  the current ODE/SDE values.
       */      
      void set_pr_value(long double* p_v) { pr_value=p_v;};
      
      /*!
       * set_pr_clock() sets a pointer to the current ODE/SDE time.
       * @param p_t points to the  the current ODE/SDE time.
       */      
      void set_pr_clock( double* p_t) { pr_clock=p_t;};
         
      /*!
       * add_node_info() inserts a new node into the automaton.
       * @param name stores the node name
       * @param p_cond is a pointer to the node condition.
       * @param l_trans is a pointer to the node transition list.
       */
      void add_node_info(const string& name, class cond* p_cond, list <class trans> l_trans);
      
      /*!
       * add_state(string name) adds a new state in the automaton
       * @param name stores the node name
       */
      void add_state(string name, string type = "");
      
      /*!
       * add_timer_var(string name) adds a new state in the automaton
       * @param name stores Timer variable name
       */
      void add_timer_var(string name);
      
      /*!
       * is_valid_node(string name) returns true if  name is a valid node name otherwise false 
       * @param name stores  a possible  node name
       */
      bool is_valid_node(string name){ return State_string2int.find(name)==State_string2int.end()?false:true;}
      /*!
       * is_final_node()check if the current node is a final node.
       * @return TRUE iff  the current node is a final one otherwise FALSE.
       */
      bool is_current_node_final_node(){return vet_node[current_state].is_final_node();};
      
      /*!
       * node_int(string name) returns the node number. If names is a not valid node name return -1
       * @param name stores  a possible  node name
       */
      int node_int(string name){ return (State_string2int.find(name)==State_string2int.end())?-1:State_string2int[name];}
      
      /*!
       * sync() computes the synchronization between ODE/SDE and automaton.
       * @return true if the synchronization is performed otherwise false
       */
       bool syncr();
       
       /*!
       * reset() resets the automaton in the initial states.
       */
       void reset(void) {current_state=initial_state;
          int size_timer=timers.size();
          for (int i=0; i<size_timer; ++i){
              timers[i]=0.0;
          }
       };
       
      /*!
       * set_variable_info() inserts all the ODE/SDE variable information (i.e. names, values, total number) 
       * @param name is a pointer to  the variable names
       * @param value is a pointer to the variable values.
       * @param num stores the number of variables.
       */
      void set_variable_info(vector <string> name, long double* value,double* clock,int num){places=name;  pr_value=value;  npl=num; pr_clock=clock;};
      
      /*!
       * search_state_variable() returns for the input state variable name  its position in the vector storing 
       * all the state variables name, otherwise -1 
       * @param name is a variable number
       * @return  its position in the vector storing all the state variables name.
       */
      int search_state_variable(const string& name);
      
      /*!
       * search_timer_variable() returns for the input timer variable name  its position in the vector storing 
       * all the timer variables name, otherwise -1 
       * @param name is a variable number
       * @return  its position in the vector storing all the timer variables name.
       */
      int search_timer_variable(const string& name);
      
      
      /*! 
       * reset_timer_variable() takes as input a index timer vector, and it resets the corresponding automaton timer
       * @param reseting_list is a vector of timer indexes that will 
       */
      inline void reset_timer_variable(const vector<int>& reseting_list){
      
          for (auto it=reseting_list.begin(); it!=reseting_list.end(); ++it){
              timers[*it]=0.0;
        }
      }
      
      /*! 
       * reset_timer_variable() takes as input a index timer vector, and it resets the corresponding automaton timer
       * @param step is a vector of timer indexes that will 
       */
      inline void inc_timer_variable(const double step){
      
          int size_timer=timers.size();
          for (int i=0; i<size_timer; ++i){
              timers[i]+=step;
        }
      }
          
      /*! 
       * reset_timer_variable() takes as input a index timer vector, and it resets the corresponding automaton timer
       * @param step is a vector of timer indexes that will 
       */  
      
      inline double get_timer_variable(const unsigned int i){
          if ((i>timers.size()))
              return -1;
          else
            return timers[i];
    }
      
     /*! 
      * timers_num()  returns the numbers of timer variables
      * @return the numbers of timer variables  
      */
      inline int timers_num(){
          return timers.size();
      }
      
      /*!
       * print() print the automaton
       * @parm output pointer
       */ 
      void print(std::ostream &os);  
      
      
      /*!
       * printTimers() print the automaton
       * @parm output pointer
       */ 
      void printTimers(std::ostream &os) { for (auto it=Timer_string2int.begin(); it!=Timer_string2int.end() ;++it) {os<<it->first<<":"<<timers[it->second]<<" "; }};
      
      /*!
       * evaluate() valuates the automaton state and updates it 
       */
      void evaluate();
      //!~automaton() is empty destructor
      ~automaton () {};
  private:
    
  };
 
  
  
   /*!
   * This class extend the standard exception class
   */ 
  class my_exception: public  exception {
    //!msg stores the error message
    string msg;
  public:
    /*!
     * my_exception () is constructor
     * @param m stores the error message
     */ 
    my_exception (string m) {msg = m;};
    /*!
     * what () returns the error message
     * @param m stores the error message
     */    
    const char* what() {return msg.c_str();};
    //!~my_exception() is empty destructor
    ~my_exception() {};
    
};
};
