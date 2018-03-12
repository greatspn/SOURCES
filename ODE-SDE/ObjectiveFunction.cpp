
#ifndef __TREE_H__
	#include "ObjectiveFunction.hpp"
#endif

namespace tree_def {

 /*
 * ***********************************
 *  Class tree
 * ***********************************
 */

 void tree::print(std::ostream &os){
 os<<"\n\n \t -TREE-\n\n";
        root->print(os);
os<<"\n\n \t - END TREE-\n\n";
 }

 std::string tree::toString(){
    std::ostringstream strm;     
    root->print(strm);
    std::string str =  strm.str();
    return str;
 }

 /*
 * ***********************************
 *  Class expression_node:public node
 * ***********************************
 */

expression_node::expression_node(string p, class node *l, class node*r){
    if(p == "+")
        op = node::op_type::PLUS;
    if(p == "-")
        op = node::op_type::MINUS;
    if(p == "*")
        op = node::op_type::MULT;
    if(p == "/")
        op = node::op_type::DIV;
    if(p == "funct")
        op = node::op_type::FUNCT;
    if(p == "base")
        op = node::op_type::BASE;
    if(p == "compos")
        op = node::op_type::COMPOS;

	l_node = l;
	r_node = r;
}

void expression_node::print(std::ostream &os){
	if(l_node&&r_node){
        l_node->print(os);
        os<<op_to_char();
        r_node->print(os);
	}
	else{
        l_node->print(os);
	}

}

string expression_node::op_to_char(){
	switch(op){
		case 0:
			return "+";
			break;
		case 1:
			return "-";
			break;
		case 2:
			return "*";
			break;
		case 3:
			return "/";
			break;
		default:
			return " ";
	}
}

 /*
 * ***********************************
 *  Class function_node:public node
 * ***********************************
 */

function_node::function_node(string fun, class node *l) {
	funct = fun;
	l_node = l;
}

void function_node::print(std::ostream &os){
	os<<funct<<"(";
	l_node->print(os);
	os<<")";
}

 /*
 * ***********************************
 *  Class composite_node:public node
 * ***********************************
 */

composite_node::composite_node(class node*l, class node*r){
	l_node = l;
	r_node = r;
}

void composite_node::print(std::ostream &os){
	l_node->print(os);
	os<<"*";
	r_node->print(os);
}

};
