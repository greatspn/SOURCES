#ifndef __IOS_H__
	#define __IOS_H__
    #include <iostream>
    #include <fstream>
    #include <sstream>
#endif

#define __TREE_H__




using namespace std;


namespace tree_def {

/*
tree objects contains the root of a parsing binary tree.
*/
class tree {

	public:
	class node *root {nullptr};
	/*!
     * tree() constructor of the class
     * @param rt  root of the tree
     */
	tree(class node *rt){root=rt;};
	~tree(){};
    /*!
     * print() prints to "os" the whole parsing tree.
     * @param std::ostream &os  stream on which the tree will be printed.
     */
    void print(std::ostream &os);
    /*!
     * toString() return the whole parsing tree as a string.
     */
    std::string toString();
};


class node {

	public:
	virtual void print(std::ostream &os)  = 0;

	enum op_type {
        PLUS = 0,MINUS = 1,MULT = 2 ,DIV = 3, FUNCT = 4,
	COMPOS = 5, BASE = 6, NaO = 8,
	};

};

class expression_node: public node {

	private:
	class node *l_node {nullptr};
	class node *r_node {nullptr};
	op_type op;

	public:
	/*!
     * expression_node() constructor of the class
     * @param p  string used to express the kind of rule applied in the grammar
     * @param l  left child of the node
     * @param r  right child of the node which can be null in some rules of the grammar
     */
	expression_node(string p, class node *l, class node*r);
	~expression_node(){};
	/*!
     * print() prints to "os" the body of the grammar rule.
     * @param std::ostream &os  stream on which the body will be printed.
     */
	void print(std::ostream &os);
	/*!
     * op_to_char() returns the string corresponding to the op_type op.
     */
	string op_to_char();

};

class function_node: public node {

	private:
	class node *l_node {nullptr};
	string funct {"f"};

	public:
	/*!
     * function_node() constructor of the class
     * @param fun string used to express the operand function's name.
     * @param l  left child of the node representing the arguement of the function.
     */
	function_node(string fun, class node *l);
	~function_node(){};
	/*!
     * print() prints to "os" the body of the grammar rule.
     * @param std::ostream &os  stream on which the body will be printed.
     */
	void print(std::ostream &os);

};

class num_node: public node {
	long double val;

	public:
	/*!
     * num_node() constructor of the class
     * @param v the value of the number.
     */
	num_node(long double v) {val = v;};
	~num_node(){};
	/*!
     * print() prints to "os" the number val.
     * @param std::ostream &os  stream on which the number will be printed.
     */
	inline void print(std::ostream &os) { os<<val;};
	/*!
     * print() prints to cout the number of val.
     */
    inline void print() { cout<<val;};

};

class var_node: public node {
	int id;
    std::string array_name;
	public:
	/*!
     * var_node() constructor of the class
     * @param n double referring the index of the array position during translation.
     */
	var_node(long double n, std::string name) {id = n; array_name = name;};
	~var_node(){};
	/*!
     * print() prints to "os" the index of the corresponding place.
     * @param std::ostream &os  stream on which the index will be printed.
     */
	inline void print(std::ostream &os) {os<<array_name<<"["<<id<<"]";};

};

class composite_node: public node {

	private:
	class node *l_node {nullptr};
	class node *r_node {nullptr};

	public:
	/*!
     * composite_node() constructor of the class
     * @param l  left child of the node representing a constant number.
     * @param r  left child of the node representing the place.
     */
	composite_node(class node *l, class node*r);
	~composite_node(){};
	/*!
     * print() prints to "os" the composite expression.
     * @param std::ostream &os  stream on which the number will be printed.
     */
	void print(std::ostream &os);

};

class comment_node: public node {
	string comment;

	public:
	/*!
     * comment_node() constructor of the class
     * @param s string containing the comment.
     */
	comment_node(string s) {comment = s;};
	~comment_node(){};
	/*!
     * print() prints to "os" the comment.
     * @param std::ostream &os  stream on which the comment will be printed.
     */
	inline void print(std::ostream &os) { os<<comment;};
	/*!
     * print() prints to cout the comment.
     */
    inline void print() { cout<<comment;};
};


};
