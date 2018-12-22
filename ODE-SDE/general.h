#ifndef __STR_H__
	#define __STR_H__
	#include <string.h> 
#endif

#ifndef __VEC_H__
	#define __VEC_H__
	#include <vector>
#endif
#ifndef __IOS_H__
	#define __IOS_H__
	#include <iostream>
#endif

#include <stdlib.h>

namespace general{

using namespace std;

//!Exception for Input/Output
class ExceptionIO{

std::string mess;
public:
	//! Empty constructors 
	ExceptionIO() {mess="";};
	//! constructor takes in input a message
	ExceptionIO(std::string mess) {this->mess=mess;};
	//! \name Get of methods use to access at the data structures
  	//@{
	//!it returns message
	std::string get(void) {return mess;};
	//@}
};

class Parser{
protected:
	//it redefines the << operator
	std::string delims;
	vector <std::string> token;

public:
	//! Empty constructors 
	Parser() {delims="";};
	//! constructor takes the list of delimiters plus the string that must be parsed
	Parser(const std::string& delims, const std::string& buffer);
	//! constructor takes the list of delimiters plus the string (char*) that must be parsed
	Parser(const std::string& delims, char* buffer);
	//! \name Set of methods use to update the data structures
  	//@{
	//!it uses to parse a new string with new delimiters
	void update(const std::string& delims, const std::string& buffer);
	//!it uses to parse a new string(char*) with new delimiters
	void update(const std::string& delims,  char* buffer);
	//!it updates the string token  in position ith with a input string. 
	void set(const int&,std::string);
	//@}
	//! \name Get of methods use to access at the data structures
  	//@{
	//!it returns the index of the occurrence of the string test in token vector the  otherwise -1
	int find(const std::string& test);
	//!it returns the index of the occurrence of the string test or the string test2 in token vector   otherwise -1. Observe that it returns the first index that satisfies the condition. 
	int findD(const std::string& test,const std::string& test2);
	//!it returns the index of the occurrence of the string test in token vector the  otherwise -1. Observe it tests on string with the same size. 
	int findS(const std::string& test);
	//!it returns the token in position i
	std::string get(int i);
	//!it returns the number of tokens
	unsigned int size(void) {return token.size();};
        //!it returns a new string obtained removing all the dilimiter char
	std::string get_string(void);
	//!it returns a new string obtained removing all the delimiter chars and splitting all the block with a new delimitator passed as paramiter
	std::string get_string(char delim);
	//@}
	friend ostream& operator<<(ostream& out,class Parser& data);

};

//!This class uses delimiters as  a unique string this is different by the normal use (e.i. strtok) 
//!Es. string=pippi delim=po  the algorithm does not split the string
class ParserB:Parser 
{

public:
	//! Empty constructors 
	ParserB() {Parser();};
	//! constructor takes the list of delimiters plus the string that must be parsed
	ParserB(const std::string& delims, const std::string& buffer);
	//! \name Set of methods use to update the data structures
  	//@{
	//!it uses to parse a new string with new delimiters
	void update(const std::string& delims, const std::string& buffer);
	//@}
	//! \name Get of methods use to access at the data structures
  	//@{
	//!it returns the index of the occurrence of the string test in token vector the  otherwise -1
	int find(const std::string& test){return Parser::find(test);};
	//!it returns the index of the occurrence of the string test or the string test2 in token vector   otherwise -1. Observe that it returns the first index that satisfies the condition. 
	int findD(const std::string& test,const std::string& test2){return Parser::findD(test,test2);};
	//!it returns the index of the occurrence of the string test in token vector the  otherwise -1. Observe it tests on string with the same size. 
	int findS(const std::string& test){return Parser::findS(test);};
	//!it returns the token in position i
	std::string get(int i){return Parser::get(i);};
	//!it returns the number of tokens
	unsigned int size(void) {return Parser::size();};
        //!it returns a new string obtained removing all the dilimiter char
	std::string get_string(void){ return Parser::get_string();};
	//@}
};

  
}
