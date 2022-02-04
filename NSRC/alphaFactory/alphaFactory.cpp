//=============================================================================
// Copyright (c) 2017, Universita' di Torino, Elvio G. Amparore ,amparore@di.unito.it>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//=============================================================================
//
//  alphaFactory
//
//  A software tool for the computation of alpha-factors for general 
//  events in a DSPN net. Takes in input the event function f_g(x),
//  the CTMC rate and the accuracy, and outputs the coefficients needed
//  for the Uniformization function.
//
//  Written by: Elvio G. Amparore, Universita' di Torino, 2017.
//
// Compile with: (requires boost C++)
//  clang++ -I/usr/local/include -L/usr/local/lib alphafactors.cpp 
//          -std=c++11 -O2 -o alphafactors
//=============================================================================

#include <iostream>
#include <memory>
#include <utility>
#include <map>
#include <stdexcept>
#include <algorithm>
#include "boost/variant.hpp"
#include <boost/version.hpp>

#ifdef USE_GMP
#   include <boost/multiprecision/gmp.hpp>
	namespace mprec = boost::multiprecision;
    template<int PREC>
    using base_mpfloat = mprec::number<mprec::gmp_float<PREC>>;
#else
#   include <boost/multiprecision/cpp_dec_float.hpp>
	namespace mprec = boost::multiprecision;
	template<int PREC>
	using base_mpfloat = mprec::number<mprec::cpp_dec_float<PREC>>;
#endif

#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/expint.hpp>

using namespace boost;
using namespace std;

#define MPFLOAT_PRECISION   1024

// typedef mp::mpf_float_500  mpfloat;
// typedef mp::number<mp::cpp_dec_float<MPFLOAT_PRECISION>> mpfloat;
typedef base_mpfloat<MPFLOAT_PRECISION> mpfloat;

struct Symbol;
struct Term;
struct Function;
template<typename T> struct Any;

template<typename T> 
struct Any {
	T* ptr;
	Any() : ptr(nullptr) { }
	Any(T* p) : ptr(p) { }
	Any(const Any&) = default;
	Any(Any&&) = default;
	Any& operator=(const Any&) = default;
	bool operator==(const Any&) const { throw; }
};
struct AnyExpr;
typedef boost::variant<Term, Symbol, Function, 
                       Any<Term>, Any<Symbol>, Any<Function>, 
                       AnyExpr> expr;

//=============================================================================

const char DIRACT_DELTA = 'D';
const char RECT = 'R';
const char UNIFORM = 'U';
const char TRIANGULAR = 'T';
const char ERLANG = 'L';
const char TRUNCATED_EXP = 'N';
const char PARETO = 'P';

struct OpNameType {
	const char *name;
	char op;
	size_t min_arity, max_arity;
};
static const OpNameType g_ops[] = {
	OpNameType{ "Exp", 'e', 1, 1 },
	OpNameType{ "Log", 'l', 1, 1 },
	OpNameType{ "Add", '+', 2, 1000 },
	OpNameType{ "Sub", '-', 2, 2 },
	OpNameType{ "Prod", '*', 2, 1000 },
	OpNameType{ "Divide", '/', 2, 2 },
	OpNameType{ "Pow", '^', 2, 2 },
	// Special functions for cdf/pdf description
	OpNameType{ "DiracDelta", DIRACT_DELTA, 1, 1 },
	OpNameType{ "Rect", RECT, 2, 2 },
	OpNameType{ "Uniform", UNIFORM, 2, 2 },
	OpNameType{ "Triangular", TRIANGULAR, 2, 2 },	
	OpNameType{ "Erlang", ERLANG, 2, 2 },
	OpNameType{ "TruncatedExp", TRUNCATED_EXP, 2, 2 },
	OpNameType{ "Pareto", PARETO, 2, 2 }
};

const OpNameType& OpDescr(char op) {
	for (const OpNameType& ont : g_ops)
		if (ont.op == op)
			return ont;
	cerr << "Operator with code " << op << "does not exists." << endl;
	throw;
}

//=============================================================================

struct Term {
	Term() { }
	Term(mpfloat f) : value(f) {}
	Term(const Term&) = default;
	Term(Term&&) = default;
	Term& operator=(const Term&) = default;
	bool operator==(const Term& x) const { return value == x.value; }

	mpfloat value;
};

struct Symbol {
	Symbol() { }
	Symbol(std::string f) : name(f) {}
	Symbol(const Symbol&) = default;
	Symbol(Symbol&&) = default;
	Symbol& operator=(const Symbol&) = default;
	bool operator==(const Symbol& x) const { return name == x.name; }

	std::string name;
};

struct AnyExpr {
	expr* ptr;
	AnyExpr() : ptr(nullptr) { }
	AnyExpr(expr* p) : ptr(p) { }
	AnyExpr(const AnyExpr&) = default;
	AnyExpr(AnyExpr&&) = default;
	AnyExpr& operator=(const AnyExpr&) = default;
	bool operator==(const AnyExpr&) const { throw; }
};

struct Function {
	Function() { }
	Function(char _op, expr e1) : op(_op), operands({e1}) { _check(); }
	Function(char _op, expr e1, expr e2) : op(_op), operands({e1,e2}) { _check(); }
	Function(char _op, expr e1, expr e2, expr e3) : op(_op), operands({e1,e2,e3}) { _check(); }
	Function(char _op, expr e1, expr e2, expr e3, expr e4) : op(_op), operands({e1,e2,e3,e4}) { _check(); }
	Function(char _op, std::vector<expr> _ex) : op(_op), operands(_ex) { _check(); }
	Function(const Function&) = default;
	Function(Function&&) = default;
	Function& operator=(const Function&) = default;
	bool operator==(const Function& x) const { 
		return op == x.op && operands == x.operands;
	}

	char op;
	std::vector<expr> operands;

	void _check() {
		const OpNameType& ont = OpDescr(op);
		if (operands.size() < ont.min_arity || operands.size() > ont.max_arity)
			throw "Invalid operand size.";
	}
};

//=============================================================================

mpfloat beta(int k, const mpfloat& v);
mpfloat eta(int k, mpfloat q, int m, mpfloat l, mpfloat a);
mpfloat factorial(int m);
mpfloat IncompleteEulerGammaN(mpfloat a, mpfloat z);
mpfloat gamma(int k, mpfloat q, int m, mpfloat l, mpfloat a);
mpfloat gamma_aInf(int k, mpfloat q, int m, mpfloat l);
mpfloat alpha(int k, const mpfloat& q, const expr& ex);
mpfloat moment(int k, const expr& f);

expr integrate(const expr& ex);
expr simplify(const expr& ex);
mpfloat evaluate(const expr& ex);

//=============================================================================

expr mkTerm(mpfloat f) { return Term(f); }

//=============================================================================

ostream& operator << (ostream& os, const Term& t) {
	return os << t.value;
}
ostream& operator << (ostream& os, const Symbol& x) {
	return os << x.name;
}
ostream& operator << (ostream& os, const Function& fn) {
	os << OpDescr(fn.op).name << "(";
	for (size_t i=0; i<fn.operands.size(); i++)
		os << (i==0 ? "" : ", ") << fn.operands[i]; 
	return os << ")";
}
ostream& operator << (ostream& os, const AnyExpr& e) {
	return os << "Any<expr>";
}
template<typename T>
ostream& operator << (ostream& os, const Any<T>& e) {
	return os << "Any<>";
}

//=============================================================================

class matcher : public boost::static_visitor<bool> {
	expr match;

	// bool match_ok(const expr& what) { *match=what; return true; }

	template<typename T, typename Arg>
	bool any_match(const Arg& arg) {
		if (match.type() == typeid(Any<T>)) {
			*boost::get<Any<T>>(match).ptr = arg;
			return true;
		}
		return false;
	}

	template<typename Arg>
	bool any_expr_match(const Arg& arg) {
		if (match.type() == typeid(AnyExpr)) {
			*boost::get<AnyExpr>(match).ptr = arg;
			return true;
		}
		return false;
	}

public:
	matcher(expr _match) : match(_match) { }

	bool operator()(const Term& t) {
		if (any_expr_match(t)) 
			return true;
		if (any_match<Term>(t))
			return true;
		if (match.type() == typeid(Term)) {
			if (boost::get<Term>(match).value == t.value)
				return true;
		}
		return false;
	}
	bool operator()(const Symbol& sym) {
		if (any_expr_match(sym)) 
			return true;
		if (any_match<Symbol>(sym))
			return true;
		if (match.type() == typeid(Symbol)) {
			if (boost::get<Symbol>(match).name == sym.name)
				return true;
		}
		return false;
	}
	bool operator()(const Function& fn) {
		if (any_expr_match(fn)) 
			return true;
		if (any_match<Function>(fn))
			return true;
		if (match.type() == typeid(Function)) {
			Function& f = boost::get<Function>(match);
			if (f.op == fn.op && f.operands.size() == fn.operands.size()) {
				for (size_t i=0; i<f.operands.size(); i++) { // Recursive match
					matcher m(f.operands[i]);
					if (!boost::apply_visitor(m, fn.operands[i]))
						return false;
				}
				return true;
			}
		}
		return false;
	}

	bool operator()(const AnyExpr& e) { throw; }
	template<typename T> bool operator()(const Any<T>& e) { throw; }
};

//=============================================================================

// takes a product of terms and prepend a constant factor of 1
expr prepend_one(const expr& ex) {
	if (ex.type() != typeid(Function))
		throw;
	if (boost::get<Function>(ex).op != '*')
		throw;
	std::vector<expr> newops = boost::get<Function>(ex).operands;
	newops.insert(newops.begin(), mkTerm(1));
	return Function('*', newops);
}

//=============================================================================

int require_int(mpfloat a) {
	int n = (int)a;
	if (n != a) {
		cerr << "Parameter " << a << " is expected to be an integer." << endl;
		throw;
	}
	return n;
}

//=============================================================================

// Determines if a polynomial term has shape: c * Exp(l * x) * x^h * Rect[0, a]
bool is_poly_x_explx_xh_R0a(const expr& ex, mpfloat& c, mpfloat& l, bool& has_exp, 
	                        mpfloat& h, bool& has_Rect, mpfloat& a) 
{
	c = 1;
	l = 0;
	h = 0;

	auto check_has_c = [&c](const expr& e) -> bool {
		if (e.type() == typeid(Term)) {
			c *= boost::get<Term>(e).value;
			return true;
		}
		return false;
	};
	auto check_has_exp = [&l](const expr& e) -> bool {
		Symbol x;
		matcher m(Function('e', Any<Symbol>(&x)));
		if (boost::apply_visitor(m, e)) {
			l += 1;
			return true;
		}
		Term lt;
		matcher m2(Function('e', Function('*', Any<Term>(&lt), Any<Symbol>(&x))));
		if (boost::apply_visitor(m2, e)) {
			l += lt.value;
			return true;
		}
		return false;
	};
	auto check_has_xh = [&h](const expr& e) -> bool {
		if (e.type() == typeid(Symbol)) {
			h += 1;
			return true;
		}
		Symbol x;
		Term ht;
		matcher m2(Function('^', Any<Symbol>(&x), Any<Term>(&ht)));
		if (boost::apply_visitor(m2, e)) {
			h += ht.value;
			return true;
		}
		return false;
	};
	auto check_has_Rect = [&a](const expr& e) -> bool {
		Term at;
		matcher m(Function(RECT, mkTerm(0), Any<Term>(&at)));
		if (boost::apply_visitor(m, e)){
			a = at.value;
			return true;
		}
		return false;
	};

	has_exp=false;
	bool has_xh=false;
	has_Rect=false;
	// product of multiple terms
	if (ex.type() == typeid(Function)) {
		const Function& fex = boost::get<Function>(ex);
		if (fex.op == '*') {
			for (const expr& opr : fex.operands) {
				bool matched = check_has_c(opr);
				if (matched)
					continue;

				matched = check_has_exp(opr);
				has_exp |= matched;
				if (matched)
					continue;

				matched = check_has_xh(opr);
				has_xh |= matched;
				if (matched)
					continue;

				matched = check_has_Rect(opr);
				has_Rect |= matched;
				if (matched)
					continue;

				// unmatched operand
				return false;
			}
		}
		else {
			// single term
			check_has_c(ex);
			has_exp = check_has_exp(ex);
			has_xh = check_has_xh(ex);
			has_Rect = check_has_Rect(ex);
		}
	}

	return (has_exp || has_xh || has_Rect);
}

//=============================================================================

expr integrate(const expr& ex) {
// 	expr __integrate(const expr& ex);
// 	expr t = __integrate(ex);
// 	cout << "integrate("<<ex<<")  =  " << t << endl;
// 	return t;
// }

// expr __integrate(const expr& ex) {


	// integrate[x]  ->  1/2 x^2
	{
		Symbol x;
		matcher m((Any<Symbol>(&x)));
		if (boost::apply_visitor(m, ex))
			return Function('*', mkTerm(0.5), Function('^', x, mkTerm(2)));
	}

	// integrate[ term ]  ->  term * x
	{
		Term t;
		matcher m((Any<Term>(&t)));
		if (boost::apply_visitor(m, ex))
			return Function('*', t, Symbol("x"));
	}

	// integrate[ e^x ]  ->  e^x
	{
		Symbol x;
		matcher m(Function('e', Any<Symbol>(&x)));
		if (boost::apply_visitor(m, ex)) 
			return ex;
	}

	// integrate[ e^(k * x) ]  ->  1/k * e^(k * x)
	{
		Symbol x;
		Term k;
		matcher m(Function('e', Function('*', Any<Term>(&k), Any<Symbol>(&x))));
		if (boost::apply_visitor(m, ex)) 
			return Function('*', mkTerm(1/k.value), Function('e', Function('*', k.value, x)));
	}

	// integrate[x^m] -> 1/(m+1) * x ^ (m+1)
	{
		Term t;
		Symbol x;
		matcher m(Function('^', Any<Symbol>(&x), Any<Term>(&t)));
		if (boost::apply_visitor(m, ex)) 
			return Function('*', mkTerm(1.0/(t.value+1)), 
				                 Function('^', x, mkTerm(t.value+1)));
	}

	// integrate[ DiracDelta(t) ]  ->  1
	{
		Term t;
		matcher m(Function(DIRACT_DELTA, Any<Term>(&t)));
		if (boost::apply_visitor(m, ex)) 
			return mkTerm(1);
	}

	// integrate[ x * DiracDelta(t) ]  ->  t
	{
		Term t;
		Symbol x;
		matcher m(Function('*', Any<Symbol>(&x), Function(DIRACT_DELTA, Any<Term>(&t))));
		if (boost::apply_visitor(m, ex)) 
			return t;
	}

	// integrate[ (c * x * DiracDelta(t)) ]  ->  c * t
	{
		Term c, t;
		Symbol x;
		matcher m(Function('*', Any<Term>(&c), Any<Symbol>(&x), Function(DIRACT_DELTA, Any<Term>(&t))));
		if (boost::apply_visitor(m, ex)) 
			return mkTerm(c.value * t.value);
	}

	// ---- rectangular signal ----

	// integrate[ c * Exp(l * x) * x^h * Rect[0, a] ]
	{
		mpfloat c, l, h, a;
		bool has_exp, has_Rect;
		if (is_poly_x_explx_xh_R0a(ex, c, l, has_exp, h, has_Rect, a)) {

			if (!has_exp) {
				if (!has_Rect) {
					cerr << ex << ": Integral of c * x^h  does not converge on [0, infinity)." << endl;
					throw;
				}
				return mkTerm(pow(a, 1+h) * c / (h+1));
			}

			// c * Exp(l * x) * x^h * Rect(0, a)  
			//    ->  c * (-l)^(-h-1) * (Gamma(h+1) - IncompleteGamma(h+1, -a * l))
			mpfloat ig = 0;
			if (has_Rect)
				ig = IncompleteEulerGammaN(h+1, -a * l);
			// cout << "integrate "<<ex<<" c="<<c<<" l="<<l<<" h="<<h<<" a="<<a<<"  ig="<<ig<<endl;
			
			return mkTerm(c / pow(abs(l), h+1) * (factorial(require_int(h)) - ig));
		}
	}

	// // integrate[ R(a, b) ]  ->  integrate[ R(0, b) ] - integrate[ R(0, a) ]
	{
		Term a, b;
		matcher m(Function(RECT, Any<Term>(&a), Any<Term>(&b)));
		if (boost::apply_visitor(m, ex) && a.value != 0)
			return Function('-', integrate(Function(RECT, mkTerm(0), b)), 
				                 integrate(Function(RECT, mkTerm(0), a)));
	}

	// integrate[ <...> * R(a, b) ]  ->  integrate[ <...> * R(0, b) ] - integrate[ <...> * R(0, a) ]
	{
		if (ex.type()==typeid(Function)) {
			const Function& f = boost::get<Function>(ex);
			if (f.op == '*') {
				Term a, b;
				matcher m(Function(RECT, Any<Term>(&a), Any<Term>(&b)));
				if (boost::apply_visitor(m, f.operands.back()) && a.value != 0) {
					std::vector<expr> opsA = f.operands, opsB;
					opsA.pop_back();
					opsB = opsA;
					opsA.push_back(Function(RECT, mkTerm(0), a));
					opsB.push_back(Function(RECT, mkTerm(0), b));
					return Function('-', integrate(Function('*', opsB)),
				                         integrate(Function('*', opsA))); 
				} 
			}
		}
	}


	//---- low priority rules ----

	// integrate[ term * f(x) ]  ->  term * integrate[f(x)]
	{
		Term t;
		expr e;
		matcher m(Function('*', Any<Term>(&t), AnyExpr(&e)));
		if (boost::apply_visitor(m, ex)) 
			return Function('*', t, integrate(e));
	}

	// integrate[ f1(x) - f2(x) ]  ->  integrate[f1(x)] - integrate[f2(x)]
	{
		expr e1, e2;
		matcher msub(Function('-', AnyExpr(&e1), AnyExpr(&e2)));
		if (boost::apply_visitor(msub, ex))
			return Function('-', integrate(e1), integrate(e2));
	}

	// integrate[ sum of terms ]  ->  sum of integrate[terms]
	{
		if (ex.type() == typeid(Function)) {
			const Function& f = boost::get<Function>(ex);
			if (f.op == '+') {
				std::vector<expr> int_opr;
				for (const expr& opr : f.operands)
					int_opr.push_back(integrate(opr));
				return Function('+', int_opr);
			}
		}
	}

	cerr << "Cannot integrate " << ex << endl;
	throw;
}

//=============================================================================

int grade_of(const expr& ex) {
	if (ex.type() == typeid(Term))
		return 0;
	else if (ex.type() == typeid(Symbol))
		return 2;
	else if (ex.type() == typeid(Function)) {
		const Function& f = boost::get<Function>(ex);
		switch (f.op) {
			case DIRACT_DELTA:
			case RECT:
				return 3; // special functions
			default:
				return 1;
		}
	}
	else throw;
}

expr simplify(const expr& ex) {
// 	expr __simplify(const expr& ex);
// 	expr s = __simplify(ex);
// 	cout << "  simplify " << ex << "  ->  ";
// 	if (s == ex)
// 		cout << "unchanged" << endl;
// 	else
// 		cout << s << endl;
// 	return s;
// }

// expr __simplify(const expr& ex) {
	if (ex.type() == typeid(Term) || ex.type() == typeid(Symbol))
		return ex;

	try {
		mpfloat value = evaluate(ex);
		return mkTerm(value);
	}
	catch(...) { /* not evaluatable */ }

	// Reorder & simplify terms
	if (ex.type() == typeid(Function)) {
		const Function& f = boost::get<Function>(ex);
		if (f.op == '*' || f.op == '+') {
			// Aggregate binary +/* operations into a single n-ary operation
			std::vector<expr> operands;
			std::function<void (const expr&)> aggreg;
			aggreg = [&](const expr& e) {
				if (e.type() != typeid(Function) ||
				    boost::get<Function>(e).op != f.op) 
				{
					operands.push_back(simplify(e));
					return;
				}
				for (const expr& eop : boost::get<Function>(e).operands)
					aggreg(simplify(eop));
			};
			aggreg(ex);
			assert(operands.size() >= 2);
			// Reorder terms: first constants, sub-expressions, symbols and finally special functions
			sort(operands.begin(), operands.end(), [](const expr& e1, const expr& e2){
				return grade_of(e1) < grade_of(e2);
			});
			// Add/multiply the constant terms
			while (operands.size() >= 2 && 
				   operands[0].type() == typeid(Term) && 
				   operands[1].type() == typeid(Term)) 
			{
				if (f.op == '*') {
					operands[1] = mkTerm(boost::get<Term>(operands[0]).value * 
						               boost::get<Term>(operands[1]).value);
				}
				else if (f.op == '+') {
					operands[1] = mkTerm(boost::get<Term>(operands[0]).value + 
						               boost::get<Term>(operands[1]).value);
				}
				else throw;
				operands.erase(operands.begin());
			}
			// Remove neutral terms
			if (operands[0].type() == typeid(Term)) {
				if (f.op == '*') {
					if (boost::get<Term>(operands[0]).value == 0)
						return mkTerm(0);
					if (boost::get<Term>(operands[0]).value == 1)
						operands.erase(operands.begin());
				}
				else if (f.op == '+') {
					if (boost::get<Term>(operands[0]).value == 0)
						operands.erase(operands.begin());
				}
			}
			// No add/product, actually....
			if (operands.size() == 1)
				return operands[0];

			// Transform symbols into powers:   x*x -> x^2
			{
				Symbol x;
				Term t;
				matcher powm(Function('^', Any<Symbol>(&x), Any<Term>(&t)));
				for (size_t i=0; i<operands.size()-1; i++) {
					if (operands[i+1].type() == typeid(Symbol)) {						
						if (operands[i] == operands[i+1]) {
							operands[i] = Function('^', operands[i+1], mkTerm(2));
							operands.erase(operands.begin() + i + 1);
							i--;
						}
						else if (boost::apply_visitor(powm, operands[i])) {
							operands[i] = Function('^', operands[i+1], mkTerm(t.value + 1));
							operands.erase(operands.begin() + i + 1);
							i--;
						}
					}
				}		
			}	
			
			// Apply:  a * b * (c + d)  ->  a*b*c + a*b*d
			if (f.op == '*') {
				auto it = operands.begin();
				while (it != operands.end()) {
					if (it->type() == typeid(Function) && boost::get<Function>(*it).op == '+') {
						Function sum = boost::get<Function>(*it);
						operands.erase(it);
						for (size_t i=0; i<sum.operands.size(); i++) {
							sum.operands[i] = Function('*', 
								                       operands.size()==1 ? operands[0] : Function('*', operands), 
								                       sum.operands[i]);
						}
						// cout << "sum = " << sum << endl;
						return simplify(sum);
					}
					it++;
				}
			}

			return Function(f.op, operands);
		}
	}

	// convert Uniform[a, b]  ->  1/(b-a) * Rect[a,b]
	{
		Term a, b;
		matcher m(Function(UNIFORM, Any<Term>(&a), Any<Term>(&b)));
		if (boost::apply_visitor(m, ex)) 
			return Function('*', mkTerm(1 / (b.value - a.value)), Function(RECT, a, b));
	}

	// convert Triangular[a, b]  ->  4/((a-b)^2) * (x-a) * Rect(a, (a+b)/2) - 
	//                              -4/((a-b)^2) * (x-b) * Rect((a+b)/2, b)
	{
		Term a, b;
		matcher m(Function(TRIANGULAR, Any<Term>(&a), Any<Term>(&b)));
		if (boost::apply_visitor(m, ex)) {
			expr t = Function('+', Function('*', mkTerm(4 / pow((b.value - a.value), 2)), 
				                                 Function('+', Symbol("x"), mkTerm(-a.value)),
				                                 Function(RECT, a, mkTerm((a.value + b.value) / 2))),
								   Function('*', mkTerm(-4 / pow((b.value - a.value), 2)), 
								 	             Function('+', Symbol("x"), mkTerm(-b.value)),
				                                 Function(RECT, mkTerm((a.value + b.value) / 2), b)));
			return simplify(t);
		}
	}

	// convert Erlang(lambda, rr) ->  (lambda^rr) / (rr-1)!  * x ^ (rr-1) * Exp(-lambda * x)
	{
		Term lambda, rr;
		matcher m(Function(ERLANG, Any<Term>(&lambda), Any<Term>(&rr)));
		if (boost::apply_visitor(m, ex)) {
			expr t = Function('*', mkTerm(pow(lambda.value, rr.value) / factorial(require_int(rr.value - 1))),
				                   Function('^', Symbol("x"), mkTerm(rr.value - 1)),
				                   Function('e', Function('*', mkTerm(-lambda.value), Symbol("x"))));
			return simplify(t);
		}		
	}

	// convert TruncatedExp(l, t) -> l Exp(-l x) R[0, t] + Exp(-l t) DiracDelta[t]
	{
		Term l, a;
		matcher m(Function(TRUNCATED_EXP, Any<Term>(&l), Any<Term>(&a)));
		if (boost::apply_visitor(m, ex)) {
			expr t = Function('+', Function('*', mkTerm(l.value), 
				                                 Function('e', Function('*', mkTerm(-l.value), Symbol("x"))),
				                                 Function(RECT, mkTerm(0), a)),
			                       Function('*', mkTerm(exp(-l.value * a.value)), 
			                       	             Function(DIRACT_DELTA, a)));
			return simplify(t);
		}		

	}

	// simplify[ x ^ 0 ]  ->  1
	{
		Symbol x;
		matcher m(Function('^', Any<Symbol>(&x), mkTerm(0)));
		if (boost::apply_visitor(m, ex)) 
			return mkTerm(1);
	}

	// simplify[ f ^ 1 ]  ->  f
	{
		expr f;
		matcher m(Function('^', AnyExpr(&f), mkTerm(1)));
		if (boost::apply_visitor(m, ex)) 
			return simplify(f);
	}

	// simplify[ f * (a + b) ]  ->  a * f + b * f
	{
		expr f, a, b;
		matcher m(Function('*', AnyExpr(&f), Function('+', AnyExpr(&a), AnyExpr(&b))));
		if (boost::apply_visitor(m, ex)) 
			return simplify(Function('+', Function('*', a, f), Function('*', b, f)));
	}

	// simplify[ (a + b) * f ]  ->  a * f + b * f
	{
		expr f, a, b;
		matcher m(Function('*', Function('+', AnyExpr(&a), AnyExpr(&b)), AnyExpr(&f)));
		if (boost::apply_visitor(m, ex)) 
			return simplify(Function('+', Function('*', a, f), Function('*', b, f)));
	}

	// recursively descend functions
	if (ex.type() == typeid(Function)) {
		const Function& f = boost::get<Function>(ex);
		Function newf;
		newf.op = f.op;
		bool changed = false;
		for (size_t i=0; i<f.operands.size(); i++) {
			newf.operands.push_back(simplify(f.operands[i]));
			if (!(newf.operands[i] == f.operands[i]))
				changed = true;
		}
		return (changed ? simplify(newf) : newf);
	}

	cerr << "Cannot simplify " << ex << endl;
	throw;
}

//=============================================================================

mpfloat evaluate(const expr& ex) {
	if (ex.type() == typeid(Term))
		return boost::get<Term>(ex).value;

	if (ex.type() == typeid(Symbol))
		throw "Cannot evaluate a symbol.";

	if (ex.type() == typeid(Function)) {
		const Function& f = boost::get<Function>(ex);
		switch (f.op) {
			case '+': {
					mpfloat v = 0;
					for (const expr& a : f.operands)
						v += evaluate(a);
					return v;
				}
			case '-':
				assert(f.operands.size() == 2);
				return evaluate(f.operands[0]) - evaluate(f.operands[1]);
			case '*':{
					mpfloat v = 1;
					for (const expr& a : f.operands)
						v *= evaluate(a);
					return v;
				}
			case '/':
				assert(f.operands.size() == 2);
				return evaluate(f.operands[0]) / evaluate(f.operands[1]);
			case '^':
				assert(f.operands.size() == 2);
				return pow(evaluate(f.operands[0]), evaluate(f.operands[1]));
			case 'e':
				assert(f.operands.size() == 1);
				return exp(evaluate(f.operands[0]));
			case 'l':
				assert(f.operands.size() == 1);
				return log(evaluate(f.operands[0]));
			default:
				// cerr << "Don't known how to evaluate function " << OpDescr(f.op).name << endl;
				throw "Unknown function";
		}
	}

	// cerr << "Cannot evaluate expression " << ex << endl;
	throw "Cannot evaluate.";
}

//=============================================================================

static map<pair<int, mpfloat>, mpfloat> s_beta_mem; // memoized buffer

mpfloat beta(int k, const mpfloat& v) {
	auto mem_key = make_pair(k, v);
	if (s_beta_mem.count(mem_key) == 1)
		return s_beta_mem[mem_key];

	mpfloat result;
	if (k == 0)
		result = exp(-v);
	else
		result = (beta(k-1, v) * v) / k;

	s_beta_mem[mem_key] = result;
	return result;
}

//=============================================================================

static map<tuple<int, mpfloat, int, mpfloat, mpfloat>, mpfloat> s_eta_mem; // memoized buffer

mpfloat eta(int k, mpfloat q, int m, mpfloat l, mpfloat a) {
	auto mem_key = make_tuple(k, q, m, l, a);
	if (s_eta_mem.count(mem_key) == 1)
		return s_eta_mem[mem_key];

	mpfloat result;
	if (k == 0)
		result = pow(a, m) / (q + l);
	else
		result = eta(k - 1, q, m, l, a) * q / (q + l);

	s_eta_mem[mem_key] = result;
	return result;
}

//=============================================================================

static map<int, mpfloat> s_factorial_mem;

mpfloat factorial(int m) {
	if (m == 0 || m == 1)
		return 1;

	if (s_factorial_mem.count(m) == 0) {
		mpfloat f = m * factorial(m - 1);
		s_factorial_mem[m] = f;
	}
	return s_factorial_mem[m];
}

//=============================================================================

// template<int PREC>
// mpfloat compute_positive_tgamma_at_precision(mpfloat a, mpfloat z) {
// 	static_assert(PREC <= MPFLOAT_PRECISION, "Using an inconsisten precision for tgamma.");
// 	typedef base_mpfloat<PREC> comp_float;

// 	// Compute tgamma using a precision of PREC
// 	// This will raise an oveflow exception if the precision is not enough.
// 	// cout << "tgamma("<<a<<", "<<z<<") " << endl;
// 	comp_float result = boost::math::tgamma(comp_float(a), comp_float(z));
// 	// cout << "compute_positive_tgamma("<<a<<", "<<z<<") = "<<result<<"  PREC=cpp_dec_float<"<<PREC<<">"<<endl;
// 	// Convert back to the precision of an mpfloat type
// 	return mpfloat(result.str(0, std::ios_base::scientific));
// }


// // End of template recursion
// struct EOR { };

// template<int PREC>
// mpfloat compute_positive_tgamma(mpfloat a, mpfloat z, EOR) {
// 	cout << "tgamma("<<a<<", "<<z<<") computation cannot be handled.\n"
// 	     << "Increase the size of MPFLOAT_PRECISION.\n" <<endl;
// 	throw std::overflow_error("Not enough precision to compute a tgamma().");
// }

// template<int PREC>
// mpfloat compute_positive_tgamma(mpfloat a, mpfloat z, int) {
// 	static_assert(PREC <= MPFLOAT_PRECISION, "Wrong recursive template definition.");
// 	typedef mp::number<mp::cpp_dec_float<PREC>> comp_float;
// 	try {
// 		// Compute tgamma using a precision of PREC
// 		comp_float result = boost::math::tgamma(comp_float(a), comp_float(z));
// 		cout << "compute_positive_tgamma("<<a<<", "<<z<<") = "<<result<<"  PREC=cpp_dec_float<"<<PREC<<">"<<endl;
// 		// Convert back to the precision of an mpfloat type
// 		return mpfloat(result.str(0, std::ios_base::scientific));
// 	}
// 	catch (std::overflow_error ovf) {
// 		// PREC is not enough - recompute doubling the precision
// 		typedef typename std::conditional<4 * PREC <= MPFLOAT_PRECISION, int, EOR>::type NextPrecType;
// 		return compute_positive_tgamma<4 * PREC>(a, z, NextPrecType());
// 	}
// }


static map<tuple<mpfloat, mpfloat>, mpfloat> s_pos_tgamma_mem; // memoized buffer

// Compute the tgamma function using an increasing precision in case of overflow
mpfloat compute_positive_tgamma(mpfloat a, mpfloat z) {
	auto mem_key = make_tuple(a, z);
	if (s_pos_tgamma_mem.count(mem_key) == 1)
		return s_pos_tgamma_mem[mem_key];

	try {
		// cout << "compute_positive_tgamma("<<a<<", "<<z<<") " << endl;
		mpfloat result = mpfloat(boost::math::tgamma(double(a), double(z)));
		// cout << "compute_positive_tgamma("<<a<<", "<<z<<") = "<<result<<"  <>"<<endl;

		s_pos_tgamma_mem[mem_key] = result;
		return result;
	}
	catch (std::overflow_error ovf) {

		mpfloat a_prev = a - 1;
		mpfloat result = a_prev * compute_positive_tgamma(a_prev, z);
		// cout << "compute_positive_tgamma("<<a<<", "<<z<<") = "<<result<<"  <recursive>"<<endl;

		s_pos_tgamma_mem[mem_key] = result;
		return result;

		// try { return compute_positive_tgamma_at_precision<32>(a, z); }
		// catch (std::overflow_error ovf) {
		// 	try { return compute_positive_tgamma_at_precision<128>(a, z); }
		// 	catch (std::overflow_error ovf) {
		// 		try { return compute_positive_tgamma_at_precision<512>(a, z); }
		// 		catch (std::overflow_error ovf) {
		// 			// tgamma is too hard to compute, rethrow the exception
	 //     			throw ovf;
		// 		}
		// 	}
		// }
		// throw ovf;
	}
}

//=============================================================================

mpfloat IncompleteEulerGammaN(mpfloat a, mpfloat z) {
	// Using full precision is both very time consuming and totally useless 
	// for our purpose. tgamma and expint can be computed using less precision.
	if (a == 0) {
		return mpfloat(-boost::math::expint(-double(z)));
	}
	if (a > 0) {
		// the boost::tgamma is only defined for a>=0
		return compute_positive_tgamma(a, z);

		// mpfloat sum_betas = 0;
		// mpfloat fact_k = 1;
		// for (int k=0; k<a; k++) {
		// 	fact_k *= (k>0 ? k : 1);
		// 	assert(fact_k == factorial(k));
		// 	sum_betas += pow(z, k) / fact_k;
		// }
		// return factorial(a-1) * exp(-z) * sum_betas;
	}
	else { // a < 0
		return (IncompleteEulerGammaN(a + 1, z) - pow(z, a) * exp(-z)) / a;
	}

}

//=============================================================================

static map<tuple<int, mpfloat, int, mpfloat, mpfloat>, mpfloat> s_gamma_mem; // memoized buffer

mpfloat gamma(int k, mpfloat q, int m, mpfloat l, mpfloat a) {
	// cout << "gamma("<<k<<", "<<q<<", "<<m<<", "<<l<<", "<<a<<")"<<endl;
	auto mem_key = make_tuple(k, q, m, l, a);
	if (s_gamma_mem.count(mem_key) == 1)
		return s_gamma_mem[mem_key];

	mpfloat result;
	if (k == 0) {
		// m!/((q+l)^(m+1))(1-Sum[beta[h,(q+l)a],{h,0,m}])
		mpfloat sum_betas = 0;
		for (int h=0; h<=m; h++)
			sum_betas += beta(h, (q+l)*a);

		result = (factorial(m) / pow(q+l, m+1)) * (1 - sum_betas);
	}
	else {
		// ((k+m)/k)*(q/(q+l))*gamma[k-1,q,m,l,a] - eta[k,q,m,l,a]*beta[k,(q+l)a]
		result = (mpfloat(k+m)/k) * (q/(q+l)) * gamma(k-1,q,m,l,a) - eta(k,q,m,l,a) * beta(k,(q+l)*a);
	}

	s_gamma_mem[mem_key] = result;
	return result;
}

//=============================================================================

static map<tuple<int, mpfloat, int, mpfloat>, mpfloat> s_gamma_aInf_mem; // memoized buffer

// Same as gamma(k, q, m, l, infinity)
mpfloat gamma_aInf(int k, mpfloat q, int m, mpfloat l) {
	auto mem_key = make_tuple(k, q, m, l);
	if (s_gamma_aInf_mem.count(mem_key) == 1)
		return s_gamma_aInf_mem[mem_key];

	mpfloat result;
	if (k == 0) { // m!/((q+l)^(m+1))
		result = (factorial(m) / pow(q+l, m+1));
	}
	else { // ((k+m)/k)*(q/(q+l))*gamma_aInf[k-1,q,m,l]
		result = (mpfloat(k+m)/k) * (q/(q+l)) * gamma_aInf(k-1,q,m,l);
	}

	s_gamma_aInf_mem[mem_key] = result;
	return result;
}


//=============================================================================

mpfloat alpha(int k, const mpfloat& q, const expr& ex) {
	// alpha[ k, q, f1 + f2 ]  ->  alpha[ k, q, f1 ] + alpha[ k, q, f2 ] 
	{
		if (ex.type() == typeid(Function) && boost::get<Function>(ex).op == '+') {
			mpfloat a = 0;
			for (const expr& o : boost::get<Function>(ex).operands)
				a += alpha(k, q, o);
			return a;
		}
	}

	// alpha[ k, q, t * DiracDelta(a) ]  ->  t * beta[ k, q * a ] 
	{
		Term t, a;
		matcher m(Function('*', Any<Term>(&t), Function(DIRACT_DELTA, Any<Term>(&a))));
		if (boost::apply_visitor(m, ex)) 
			return t.value * beta(k, q * a.value);
	}

	// alpha[ k, q, DiracDelta(a) ]  ->  beta[ k, q * a ] 
	{
		Term a;
		matcher m(Function(DIRACT_DELTA, Any<Term>(&a)));
		if (boost::apply_visitor(m, ex)) 
			return beta(k, q * a.value);
	}

	// alpha[k,q,<term> * R[a,b]]  ->  alpha[k,q,<term> * R[0,b]] - alpha[k,q,<term> * R[0,a]];
	if (ex.type() == typeid(Function) && boost::get<Function>(ex).op == '*') {
		const Function& f = boost::get<Function>(ex);
		Term a, b;
		matcher m(Function(RECT, Any<Term>(&a), Any<Term>(&b)));
		if (boost::apply_visitor(m, f.operands.back()) && a.value != 0) {
			if (a.value >= b.value) {
				cerr << "Cannot evaluate a rectangular impulse R["<<a.value<<", "<<b.value<<"] with a > b." << endl;
				throw "Invalid rectangular impulse.";
			}
			std::vector<expr> opsA = f.operands, opsB;
			opsA.pop_back();
			opsB = opsA;
			opsA.push_back(Function(RECT, mkTerm(0), a));
			opsB.push_back(Function(RECT, mkTerm(0), b));
			return alpha(k, q, Function('*', opsB)) - alpha(k, q, Function('*', opsA));
		} 
	}

	// alpha[k,q,R[a,b]]  ->  alpha[k,q,R[0,b]] - alpha[k,q,R[0,a]];
	{
		Term a, b;
		matcher m(Function(RECT, Any<Term>(&a), Any<Term>(&b)));
		if (boost::apply_visitor(m,ex) && a.value != 0) {
			return alpha(k, q, Function('*', Function(RECT, mkTerm(0), b))) - 
			       alpha(k, q, Function('*', Function(RECT, mkTerm(0), a)));
		} 
	}

	// alpha[k,q, c * Exp(l * x) * x^h * Rect[0, a] ]
	{
		mpfloat c, l, h, a;
		bool has_exp, has_Rect;
		if (is_poly_x_explx_xh_R0a(ex, c, l, has_exp, h, has_Rect, a)) {

			// cout << "alpha k="<<k<<" q="<<q<<" "<<ex<<"   =>  c="<<c<<", l="<<l<<" h="<<h<<" a="<<a<<endl;
			if (has_Rect) {
				return c * gamma(k, q, require_int(h), -l, a);
			}
			else { // a -> infinity
				// cout << "    = " << (c * gamma_aInf(k, q, require_int(h), -l)) << endl;
				return c * gamma_aInf(k, q, require_int(h), -l);
			}
		}
	}

	// alpha[k, q, Pareto[kappa, a]]  ->  -(exp(-q) * kappa^k * a * q^k) / ((k - a) k!)
	{

		Term kappa, a;
		matcher m(Function(PARETO, Any<Term>(&kappa), Any<Term>(&a)));
		if (boost::apply_visitor(m, ex)) {
			// cout << "k="<<k<<" q="<<q<<" kappa="<<kappa.value<<" a="<<a.value<<endl;
			// cout << "  Gamma="<<IncompleteEulerGammaN(k - require_int(a.value), kappa.value * q)<<endl;
			return a.value * pow(kappa.value, a.value) * 
		           pow(q, a.value) / factorial(k) *
		           IncompleteEulerGammaN(k - require_int(a.value), kappa.value * q);
	    }
	}

	cerr << "Cannot compute alpha("<<k<<", "<<q<<", "<<ex<<")." << endl;
	throw;
}

//=============================================================================

mpfloat moment(int k, const expr& f) {

	// Special case for the Pareto distribution
	// moment[0,pareto[kappa_,alpha_]]  ->  1
	// moment[1,pareto[kappa_,alpha_]]  ->  (alpha * kappa)/(alpha - 1)
	{
		Term kappa, a;
		matcher m(Function(PARETO, Any<Term>(&kappa), Any<Term>(&a)));
		if (boost::apply_visitor(m, f)) {
			if (k == 0)
				return 1;
			else if (k == 1) {
				if (a.value <= 1) {
					cerr << "Asking for the mean of a Pareto distribution of shape a="
					     <<a.value<<", which is infinite." << endl;
				}
				return (kappa.value * a.value) / (a.value - 1);
			}
			else
				throw;
		}
	}

	// moment(k, f) = integral( f * x^k )
	Symbol X("x");
	return evaluate(integrate(simplify(Function('*', f, Function('^', X, mkTerm(k))))));
}

//=============================================================================

std::vector<pair<mpfloat, mpfloat>>
compute_alpha_factors(const expr& f, const mpfloat& q, const mpfloat& accuracy, bool verbose) {
	std::vector<pair<mpfloat, mpfloat>> factors;
	// Compute area and mean value of f
	mpfloat area_f = moment(0, f);
	mpfloat expectation_f = moment(1, f);
	if (abs(area_f - 1) > accuracy) {
		cerr << "Warning: function " << f << " is not a density function, area = " << area_f << endl;
	}
	if (verbose) {
		cout << "======\n";
		cout << "Area:        " << area_f << endl;
		cout << "Expectation: " << expectation_f << endl;
	}

	// lim_aCFk = lim_{k -> infty} aCF(k, q)
	mpfloat lim_aCFk = (1 - area_f) / q;
	// cumulative function values
	mpfloat aCFk = 1/q;
	// errors
	mpfloat err1 = area_f, err2 = expectation_f;
	if (err2 == numeric_limits<mpfloat>::infinity())
		err2 = 0;

	int k = 0;
	while ((err1 > accuracy || err2 > accuracy) && err1 > 0 && err2 > 0) {
		// k-th alpha factor
		mpfloat afk = alpha(k, q, f);
		aCFk -= (afk / q);

		// cout << "aCFk["<<k<<"]="<<aCFk<<"   lim_aCFk="<<lim_aCFk<<endl;

		factors.push_back(make_pair(afk, aCFk));
		// cout << "alpha["<<k<<", "<<q<<", "<<f<<"] = "<<afk<<"  "<<aCFk<<endl;
		err1 -= afk;
		err2 -= (aCFk - lim_aCFk);
		k++;
	}
	return factors;
}

//=============================================================================

inline bool isfloat(char ch) { return isdigit(ch) || ch=='.' || ch=='-' || ch=='e' || ch=='E'; }
inline bool isfloat_start(char ch) { return isdigit(ch) || ch=='.' || ch=='-'; }

struct Token {
	const char* tok;
	Token(const char* t) : tok(t) {}
};
istream& operator >> (istream& is, const Token& t) {
	while (isspace(is.peek()))
		is.get();
	size_t len = strlen(t.tok);
	for (size_t i=0; i<len; i++) {
		char ch = is.get();
		if (ch != t.tok[i]) {
			cerr << "Syntax error. Found " << ch << endl;
			exit(-1); 
		}
	}
	return is;
}

inline void skip_spaces(istream& is) {
	while (is && isspace(is.peek()))
		is.get();
}

//=============================================================================
// The parsed grammar is the following:
//
//       <expr_term> ::=  <floating-point-number>
//                     |  Symbol
//                     |  '(' <expr_sum> ')'
//                     |  <Function> '(' <comma separated arguments> ')'
//
//        <expr_sum> ::=  <expr_product> ('+' <expr_product>)*
//
//   <expr_product>  ::=  <expr_term> ('*' <expr_term>)*
//
// and is implemented with a simple recursive parser.
//=============================================================================
expr parse_sum(istream& is);
expr parse_product(istream& is);
expr parse_term(istream& is);

expr parse_term(istream& is) {
	skip_spaces(is);
	int ch = is.peek();
	if (ch == '(') {
		is.get();
		expr t = parse_sum(is);
		skip_spaces(is);
		is >> Token(")");
		return t;
	}
	if (isfloat_start(ch)) {
		std::string term;
		term += is.get();
		while (is && isfloat(is.peek()))
			term += is.get();

		return mkTerm(mpfloat(term.c_str()));
	}
	if (isalpha(ch)) {
		std::string fn;
		while (is && isalpha(is.peek()))
			fn += is.get();
		skip_spaces(is);

		if (fn == "Fract")  	 fn = "Divide";
		if (fn == "I")           fn = "DiracDelta";
		if (fn == "R")  		 fn = "Rect";

		for (const OpNameType op : g_ops) {
			if (fn == op.name) {
				std::vector<expr> operands;
				int paren, sep;
				paren = is.get();
				if (paren!='(' && paren!='[')
					throw "parse error, expected '(' or '['.";
				paren = (paren=='(' ? ')' : ']');
				do {
					operands.push_back(parse_sum(is));
					skip_spaces(is);
					sep = is.get();
					if (sep!=',' && sep!=paren)
						throw "parse error, expected ',' or ')'.";
				}
				while (sep == ',');

				if (operands.size() < op.min_arity || operands.size() > op.max_arity) {
					cerr << "Function " << fn << " cannot have " << operands.size() << " parameters.";
				}
				return Function(op.op, operands);
			}
		}
		if (fn == "x") {
			return Symbol("x");
		}
		cerr << "Unknown function " << fn << endl;
		throw "Unknown function name.";
	}
	throw "Parse error.";
}

expr parse_product(istream& is) {
	expr e = parse_term(is);

	while (is) {
		skip_spaces(is);
		int op = is.peek();
		if (op != '*' && op != '/')
			return e;
		is.get();

		expr e2 = parse_term(is);
		if (e.type() == typeid(Function)) {
			Function& fn = boost::get<Function>(e);
			if (fn.op == op && op == '*') { // same operand as on the left
				fn.operands.push_back(e2);
				continue;
			}
		}
		e = Function(op, e, e2);
	}
	return e;
}

expr parse_sum(istream& is) {
	expr e = parse_product(is);

	while (is) {
		skip_spaces(is);
		int op = is.peek();
		if (op != '+' && op != '-')
			return e;
		is.get();

		expr e2 = parse_product(is);
		if (e.type() == typeid(Function)) {
			Function& fn = boost::get<Function>(e);
			if (fn.op == op && op == '+') { // same operand as on the left
				fn.operands.push_back(e2);
				continue;
			}
		}
		e = Function(op, e, e2);
	}
	return e;
}

// Parser entry point
expr parse(const char* str) {
	istringstream iss(str);
	expr e = parse_sum(iss);
	// cout << "parsed: " << e << endl;
	if (iss)
		throw "could not parse the whole expression.";
	return e;
}

//=============================================================================

const char* verify_alpha_factors_expr(const char* fg_expr) {
	try {
		// Test parseability
		expr fgx = parse(fg_expr);
		// Test moment computation
		mpfloat m0 = moment(0, fgx);
		if (abs(m0 - 1.0) > 1.0e-6) {
			cerr << "Integral in [0, inf) of f_g(x) is " << m0 << endl;
			throw "Not a probability density function. Its integral in [0, inf) does not sum to 1.";
		}
		moment(1, fgx);
		// Compute the first alpha factor
		alpha(0, 1.0, simplify(fgx));

		return nullptr;
	}
	catch (const char * e) { return e; }
}

//=============================================================================

std::map<std::tuple<string, double, double>, 
         std::vector<pair<double, double>>> computed_factors_mem;

const std::vector<pair<double, double>>&
compute_alpha_factors_dbl(const char* fg_expr, const double q, const double accuracy) {
	auto f_key = make_tuple(string(fg_expr), q, accuracy);

	if (computed_factors_mem.count(f_key) == 0) {
		expr fg = parse(fg_expr);
		std::vector<pair<mpfloat, mpfloat>> factors;
		factors = compute_alpha_factors(simplify(fg), mpfloat(q), mpfloat(accuracy), false);
		
		std::vector<pair<double, double>> dbl_factors(factors.size());
		for (size_t f = 0; f < factors.size(); f++)
			dbl_factors[f] = make_pair(double(factors[f].first), double(factors[f].second));

		computed_factors_mem[f_key] = dbl_factors;
	}

	return computed_factors_mem[f_key];
}

//=============================================================================

#ifndef ALPHAFACTORSLIB

//=============================================================================

const char* g_banner = 
	"alphaFactory:  Alpha-factors calculator for general distribution functions.\n"
	"Written by Elvio G. Amparore. (C) Universita' di Torino, 2017.\n"
	"See the LICENSE.txt file for the ISC license terms.\n"
	"\n"
	"Computes the alpha-factors of a probability distribution function f(x).\n"
	"Based on the general idea of Reinhard German, as described in the book\n"
	" \"Modeling with Non-Markovian Stochastic Petri Nets\"\n"
	"and re-arranged by E. Amparore at University of Torino.\n"
	"\n"
	"Invokation:\n"
	"  ./alpha-factors  \"<function>\"  <ctmc_rate>  <accuracy>\n"
	"where <function> is the expression of the pdf, <ctmc_rate> the rate of\n"
	"the uniformized CTMC, and <accuracy> the error threshold.\n"
	"\n"
	"Function are expressed as these examples:\n"
	"  3 * Exp(-0.2 * x)\n"
	"  0.4 * Pow(x, 2) * Exp(-0.5 * x) * Rect(0, 4)\n"
	"  0.3 * DiracDelta(5) + 0.7 * DiracDelta(10)\n"
	"\n"
	"Language elements include:\n"
	"  1.2345               Number term.\n"
	"  +,-,*,/              Basic operands.\n"
	"  Pow(t, e)            Power of t with exponent e.\n"
	"  Exp(t)               Exponential of t\n"
	"  Log(t)               Natural logarithm of t\n"
	"\n"
	"two functions to specify a discontinuous impulse:\n"
	"  DiracDelta(t)        Deterministic Dirac impulse at time t\n"
	"  Rect(a,b)            Rectangular impulse of 1 in range (a,b)\n"
	"\n"
	"and the convenience (non-primitive) functions:\n"
	"  Uniform(a,b)         Uniform distribution in (a,b) = Rect(a,b)/(b-a)\n"
	"  Triangular(a,b)      Triangular distribution in (a,b), peak in (a+b)/2\n"
	"  Erlang(l,r)          Erlang distribution of rate l and r phases\n"
	"  TruncatedExp(l,t)    Exponential distribution of rate l truncated at time t\n"
	"  Pareto(k,a)          Pareto distribution\n"
	"\n"
	"The output starts with some initial text ended by a line with the marker '======'.\n"
	"The successive numbers are: the area of the function f(x) (usually 1 for \n"
	"non-defective pdf), the expected value and the number N of alpha factors.\n"
	"It then outputs the N alpha factors for the pdf f(x), followed by the N alpha factors\n"
	"of the inverse of the cdf (1-F(x)).\n"
	"\n"
;

//=============================================================================

const double alphaF_test1[27] = {
  0.000158461, 0.00138654, 0.0060661, 0.0176928, 0.038703, 0.0677302, 
  0.0987732, 0.123466, 0.135041, 0.13129, 0.114879, 0.0913811, 
  0.066632, 0.0448485, 0.0280303, 0.016351, 0.00894196, 0.00460248, 
  0.00223732, 0.00103034, 0.000450775, 0.000187823, 0.0000747023, 
  0.0000284193, 0.0000103612, 3.62643e-6, 1.22043e-6
};
const double alphaCF_test1[27] = {
  0.571338, 0.570546, 0.567079, 0.556969, 0.534853, 
  0.49615, 0.439708, 0.369156, 0.29199, 0.216967, 0.151321, 0.0991036,
  0.0610282, 0.0354005, 0.0193832, 0.0100398, 0.00493008, 0.00230009,
  0.00102162, 0.000432857, 0.000175271, 0.0000679435, 0.0000252565, 
  9.01689e-6, 3.0962e-6, 1.02395e-6, 3.26563e-7
};
// alpha-factors of:  0.3 I[5] + 0.7 I[10]  computed with Mathematica, accuracy 10-6, q=1.25
const double alphaF_test2[33] = {
  0.000581745, 0.00365221, 0.0115151, 0.0244143, 0.0394742, 0.0526598,
  0.0617645, 0.0674872, 0.0720061, 0.0767845, 0.0814656, 0.0843273, 
  0.0835456, 0.0782672, 0.0689595, 0.0570821, 0.0444454, 0.0326252, 
  0.0226373, 0.0148866, 0.00930218, 0.00553642, 0.00314553, 
  0.00170948, 0.000890343, 0.000445169, 0.000214023, 0.0000990844, 
  0.0000442341, 0.0000190664, 7.94433e-6, 3.20336e-6, 1.25131e-6
};
const double alphaCF_test2[33] = {
  0.799535, 0.796613, 0.787401, 0.767869, 0.73629, 
  0.694162, 0.644751, 0.590761, 0.533156, 0.471728, 0.406556, 
  0.339094, 0.272258, 0.209644, 0.154476, 0.108811, 0.0732542, 
  0.0471541, 0.0290442, 0.0171349, 0.0096932, 0.00526406, 0.00274763, 
  0.00138005, 0.000667774, 0.00031164, 0.000140421, 0.0000611539, 
  0.0000257666, 0.0000105135, 4.15806e-6, 1.59537e-6, 5.9432e-7
};
// function = 0.25 * R[2, 6],   q = 0.75,  accuracy=10e-6
const double alphaF_test3[17] = {
  0.07067372120339582, 0.16557530647024726, 0.21175625320934066, 
  0.1973538632623196, 0.14977349580104793, 0.09753786145464155, 
  0.0560078097870213, 0.02880563530616211, 0.013409858887596944, 
  0.0056962117994673826, 0.0022227067779820326, 
  0.0008014051103070652, 0.0002683766923409913, 
  0.00008386220620797023, 0.000024553480399702962, 
  6.760812844957447e-6, 1.7566204252853954e-6
};
const double alphaCF_test3[17] = {
  1.239101705062139, 1.0183346297684759, 
  0.7359929588226883, 0.4728544744729289, 0.27315648007153165, 
  0.14310599813200958, 0.06842891841598119, 0.030021404674431706, 
  0.012141592824302449, 0.004546643758345939, 
  0.0015830347210365626, 0.0005144945739604758, 
  0.00015665898417248743, 0.00004484270922852713, 
  0.000012104735362256518, 3.0903182356465894e-6, 
  7.481576685993957e-7
};
// ftriangular[2, 8], q = 0.75, accuracy = 1e-6
const double alphaF_test4[19] = {
  0.035273762096410324, 0.10475683962246518, 0.16731066643184836, 
  0.1906245756250972, 0.17312043575060904, 0.13271707902053315, 
  0.08885379957801796, 0.05311115850375989, 0.02878176398998181, 
  0.014300032265106023, 0.006569611895059384, 0.002809443533940739,
  0.0011243669453171988, 0.00042298288363698213, 
  0.00015013499660890508, 0.000050440061634290626, 
  0.000016084946670149688, 4.880826742287141e-6, 
  1.4124306413435554e-6
};
const double alphaCF_test4[19] = {
  1.2863016505381195, 1.1466258643748326,
  0.9235449757990348, 0.6693788749655719, 0.4385516272980932, 
  0.26159552193738234, 0.14312378916669172, 0.07230891116167854, 
  0.03393322584170279, 0.01486651615489476, 0.006107033628148916, 
  0.002361108916227931, 0.0008619529891383327, 
  0.0002979758109556899, 0.00009779581547714978, 
  0.000030542399964762285, 9.095804404562701e-6, 
  2.5880354148465133e-6, 7.047945597217727e-7
};
// a simple exponential:  2 E^(-2 x)   q=0.75, accuracy = 1e-6
const double alphaF_test5[11] = {
  0.7272727272727273, 0.1983471074380165, 0.05409466566491359, 0.014753090635885523, 
  0.004023570173423324, 0.0010973373200245428, 0.00029927381455214803, 
  0.00008162013124149491, 0.000022260035793134973, 6.070918852673174e-6, 1.6557051416381383e-6
};
const double alphaCF_test5[11] = { 
  0.36363636363636354, 0.09917355371900821, 0.027047332832456764, 0.007376545317942733, 
  0.0020117850867116347, 0.0005486686600122445, 0.00014963690727604713, 0.00004081006562072059, 
  0.00001113001789654063, 3.035459426309732e-6, 8.278525707922145e-7
};
// Erlang of rate lambda=9 and 3 phases,    q=2.75, accuracy = 1e-6
const double alphaF_test6[13] = {
  0.4493801951398052, 0.31552226467262917, 0.147691272825486, 0.05761007096029596, 
  0.020224812145635818, 0.006626853341335991, 0.0020679542341757702, 
  0.0006222719428066907, 0.00018204764284238294, 0.000052075094051840045, 
  0.000014625345563495504, 4.04530834734982e-6, 1.1045700097373622e-6 
};
const double alphaCF_test6[13] = { 
  0.20022538358552538, 0.08549001461366022, 0.03178409722257441, 0.010834980509739512, 
  0.0034805033658719415, 0.0010707385144770355, 0.0003187551565949372, 
  0.00009247445011977695, 0.00002627530726800134, 7.338909430968598e-6, 
  2.0206019533338694e-6, 5.49580736115753e-7, 1.479189143930758e-7
};
// Exponential (lambda=4) truncated to t=4,   q=1.75, accuracy = 1e-6
const double alphaF_test7[22] = {
  0.22231820762540075, 0.17349007390549398, 0.13661646870434177, 0.10939277660720885, 
  0.08919864485577517, 0.0732177396605748, 0.059187723446032514, 0.04603489601358085, 
  0.03384440059762005, 0.02327372720517613, 0.014899607570293332, 0.008871582236488748, 
  0.004918972641741452, 0.0025457419368255362, 0.001233281530259469, 0.000560957647360711, 
  0.00024028114864956102, 0.00009720333089771596, 0.000037238621062423384, 
  0.000013544360217788685, 4.6881271145788114e-6, 1.5476222717198126e-6
};
const double alphaCF_test7[22] = { 
  0.44438959564262814, 0.3452524105537744, 0.2671858570084363, 0.20467569894717408, 
  0.15370504474387398, 0.11186633636640267, 0.07804478011152695, 0.05173912524662361, 
  0.03239946776226929, 0.01910019507359722, 0.010586133604858173, 0.005516658041150317, 
  0.0027058165315837733, 0.0012511068533977527, 0.0005463745503923419, 0.00022582732332907852, 
  0.00008852380981504366, 0.00003297904930206311, 0.000011699837266392604, 
  3.960202856227643e-6, 1.281273076468322e-6, 3.969174926284292e-7
};

// Pareto distribution (k=12, alpha=7),    q=0.15, accuracy = 1e-6
const double alphaF_test8[24] = {
  0.12831598088484214, 0.2588803513572124, 0.2647419417243595, 0.18359322793699892, 
  0.0975801809279189, 0.042674318634619554, 0.01614195527920471, 0.005502593629162214, 
  0.0017569321291073636,0.000546601106833402, 0.0001725697780145455, 0.00005741441236452669, 
  0.000020690626572562552, 8.172893328823796e-6, 3.530306287855329e-6, 1.6507927938689505e-6, 
  8.257695061323273e-7, 4.372115971615928e-7, 2.428992823244178e-7, 1.4062627455741864e-7, 
  8.437579841824132e-8, 5.2232640003236155e-8, 3.3238952965556086e-8, 2.1677578039501926e-8
};
const double alphaCF_test8[24] = { 
  5.811226794101053, 4.08535778505297, 2.3204115068905735, 1.0964566539772473, 
  0.44592211445778795, 0.1614266568936576, 0.05381362169895952, 0.01712966417121143, 
  0.005416783310495672, 0.0017727759316063254, 0.0006223107448426887, 0.00023954799574584407, 
  0.00010161048526209372, 0.000047124529736601744, 0.00002358915448423288, 0.000012583869191773211, 
  7.078739150891029e-6, 4.163995169813744e-6, 2.544666620984292e-6, 1.6071581239348345e-6, 
  1.044652801146559e-6, 6.964352011249847e-7, 4.7484218135461077e-7, 3.303249944245979e-7
};

// Expolinomial:  27/10 E^(-3 x) x + 2/5 E^(-2 x) x^2 + 1/10 E^(-x) x^3
// q = 1.1, accuracy = 1e-6
const double alphaF_test9[32] = {
  0.2183237699383949, 0.17941278919233844, 0.13962028549592823, 0.1130848512603577, 
  0.09183663516475614, 0.07264052118478372, 0.05544977476523576, 0.040869446065427764, 
  0.029192476229361948, 0.020292461753214773, 0.013780129263765586, 0.00917160515423114, 
  0.005999190555712181, 0.003865216021222848, 0.00245753949178536, 0.0015443648032499523, 
  0.0009604844806355319, 0.0005918375349930822, 0.0003616559910791037, 0.00021934150147872436, 
  0.00013212375326180094, 0.0000790931831721027, 0.00004707877208521484, 0.000027876678021584615, 
  0.000016427254186817794, 9.637294243978593e-6, 5.630569597937739e-6, 3.277047010712572e-6, 
  1.9004627926191055e-6, 1.0984606685788835e-6, 6.329223551938669e-7, 3.6361429610718554e-7
};
const double alphaCF_test9[32] = { 
  0.7106147546014592, 0.5475122189720606, 0.42058468670303495, 0.31778027646634616, 
  0.23429242631656785, 0.1682555888758554, 0.11784670272564107, 0.08069266084797946, 
  0.05415404609401406, 0.03570635359109153, 0.023178963351304635, 0.014841140483821782, 
  0.0093873308877198, 0.0058734981411535745, 0.0036393713304396108, 0.002235403327485109, 
  0.0013622356178164437, 0.0008242014950954598, 0.0004954233213871837, 0.0002960219564065252, 
  0.00017590945344125164, 0.00010400655964843101, 0.00006120767593459933, 0.000035865241369522415, 
  0.000020931373926960786, 0.000012170197341525701, 7.051497707036848e-6, 4.07236406093451e-6, 
  2.3446706130989595e-6, 1.3460700052999744e-6, 7.706860460328227e-7, 4.4012759502629044e-7
};

//  3.075146296605449 E^(-1.3 x) x^2 R[0, 1] + 
//  0.016656809328834597 E^(-0.4 x) x^4 R[1, 3] +
//  0.08823381802878069 E^(-0.9 x) x^3 R[3, 5]          q=1.4, accuracy = 1e-6
const double alphaF_test10[21] = {
  0.17229864590173324, 0.1833588459447176, 0.1459232739927913, 0.12116253766184826, 
  0.10510515799325004, 0.08742196612875092, 0.06705726687439353, 0.0472472253097414, 
  0.030754425904461126, 0.01860534971343138, 0.010507146245758749, 0.0055569413418704895,
  0.0027593390592362483, 0.0012894437958368172, 0.0005683436410975424, 0.0002368148903564649, 
  0.00009349013097616177, 0.00003504531913704927, 0.000012500239909280681, 4.251153959473417e-6, 
  1.381103329196748e-6
};
const double alphaCF_test10[21] = { 
  0.5912152529273335, 0.4602446486811066, 0.35601373868625563, 0.2694690689277926, 
  0.19439395607547114, 0.13194969455493477, 0.08405164678751081, 0.050303628709124096, 
  0.028336181634509006, 0.015046646124915164, 0.007541541663658915,0.0035722978480371365, 
  0.0016013413771541018, 0.0006803100944135182, 0.00027435035077241646, 0.00010519685766065582, 
  0.00003841819267768312, 0.00001338582186550507, 4.457079073161725e-6, 1.4205405306807122e-6, 
  4.34038152683035e-7
};


struct aF_test {
	const char* fn;
	const double* alphaF, *alphaCF;
	size_t num_entries;
	double ctmc_q;
	double accuracy;
};
const aF_test all_tests[] = {
	{ "DiracDelta(5)", 
	  alphaF_test1, alphaCF_test1, 27, 1.75, 1e-6 },
	{ "0.3 * DiracDelta(5) + 0.7 * DiracDelta(10)", 
	  alphaF_test2, alphaCF_test2, 33, 1.25, 1e-6 },
	{ "Uniform(2, 6)", //"Prod(0.25, Rect(2, 6))", 
	  alphaF_test3, alphaCF_test3, 17, 0.75, 1e-6 },
	{ "Triangular(2,8)", //"Add(Prod(Divide(1,9), Add(x, -2), Rect(2,5)),  Prod(Divide(-1,9), Add(x, -8), Rect(5,8)))",
	  alphaF_test4, alphaCF_test4, 19, 0.75, 1e-6 },
	{ "2 * Exp(-2 * x)",
	  alphaF_test5, alphaCF_test5, 11, 0.75, 1e-6 },
	{ "Erlang(9, 3)",
	  alphaF_test6, alphaCF_test6, 13, 2.75, 1e-6 },
	{ "TruncatedExp(0.5, 4)",
	  alphaF_test7, alphaCF_test7, 22, 1.75, 1e-6 },
	{ "Pareto(12, 7)", // Cannot compute incomplete Gamma functions in negative domain.
	  alphaF_test8, alphaCF_test8, 24, 0.15, 1e-6 },
	{ "27/10 * Exp(-3 * x) * x + 2/5 * Exp(-2 * x) * Pow(x,2) + 1/10 * Exp(-1 * x) * Pow(x, 3)",
		// "Add(Prod(Divide(27,10), Exp(Prod(-3,x)), x), "
		//   "Prod(Divide(2,5), Exp(Prod(-2,x)), Pow(x,2)), "
		//   "Prod(Divide(1,10), Exp(Prod(-1,x)), Pow(x,3)))",
	  alphaF_test9, alphaCF_test9, 32, 1.1, 1.e-6 },
	{ "3.075146296605449    * Exp(-1.3 * x) * Pow(x, 2) * Rect(0, 1) + "
	  "0.016656809328834597 * Exp(-0.4 * x) * Pow(x, 4) * Rect(1, 3) + "
	  "0.08823381802878069  * Exp(-0.9 * x) * Pow(x, 3) * Rect(3, 5)",
		// "Add(Prod(3.075146296605449, Exp(Prod(-1.3, x)), Pow(x, 2), Rect(0, 1)), "
	 //       "Prod(0.016656809328834597, Exp(Prod(-0.4, x)), Pow(x, 4), Rect(1, 3)), "
	 //       "Prod(0.08823381802878069, Exp(Prod(-0.9, x)), Pow(x, 3), Rect(3, 5))) ",
		alphaF_test10, alphaCF_test10, 21, 1.4, 1e-6 }
};


void test_value(mpfloat v1, mpfloat v2) {
	if (abs(v1 - v2) < 0.000001)
		return;
	cerr << "Unexpected value " << v1 << ", expected " << v2 << "!" << endl;
	throw;
}

//=============================================================================

void unit_test() {
	// mpfloat ff(3.76);
	// cout << setprecision(numeric_limits<mpfloat>::max_digits10) << sqrt(ff) << endl;

	Symbol X("x");
	cout << "\nTesting simplify()...\n";
	const char* simplify_fn[] = {
		"Prod(3, 4, x, 2, Add(2, 5), DiracDelta(3), Exp(Prod(x, 3)))",
		"Prod(3, Prod(4, Add(3, 4), x), Add(6, 2))",
		"Add(Prod(2,3), Add(1,5), Prod(4,5))",
		"Add(0, x)", 
		"Add(x, 0)",
		"Prod(3, x, 0, Exp(x))", 
		"Prod(1,x)",
		"Prod(3, x, Add(5,6))",
		"Prod(3, Add(x, 5), DiracDelta(4))",
		"Prod(Add(Prod(0.3, DiracDelta(5)), Prod(0.7, DiracDelta(10))), x)",
		"Prod(x, 3, x)",
		"Prod(Pow(x, 4), 5, x, x, 7)",
		"Add(Prod(Divide(1,9), Add(x, -2), Rect(2,5)),  Prod(Divide(-1,9), Add(x, -8), Rect(5,8)))",
		"Triangular(2,8)"
	};
	for (const char* fnstr : simplify_fn) {
		// cout << "\nSimplify: " << fnstr << endl;
		expr e = parse(fnstr);
		expr e2 = simplify(e);
		cout << "simplify(" << e << ")  =  " << e2 << endl;
	}

	cout << "\nTesting integrate()...\n";
	const char* integrate_fn[] = {
		"Prod(3, Exp(x))",
		"Prod(3, Exp(Prod(4, x)))",
		"DiracDelta(5)",
		"Prod(3, DiracDelta(5))",
		"Prod(3, Prod(x, DiracDelta(5)))",
		// "Rect(7, 11) / Sub(11, 7)"
	};
	for (const char* fnstr : integrate_fn) {
		expr e = parse(fnstr);
		expr e2 = simplify(integrate(simplify(e)));
		cout << "integrate(" << e << ")  =  " << e2 << endl;
	}

	cout << "\nTesting eta/beta/gamma functions...\n";
	test_value(factorial(0), mpfloat(1));
	test_value(factorial(1), mpfloat(1));
	test_value(factorial(2), mpfloat(2));
	test_value(factorial(3), mpfloat(6));
	test_value(factorial(15), mpfloat("1307674368000"));

	test_value(IncompleteEulerGammaN(5, 3), mpfloat(19.566317868570529591));
	test_value(IncompleteEulerGammaN(5, 0), mpfloat(24));
	test_value(IncompleteEulerGammaN(1, 0), mpfloat(1));
	test_value(IncompleteEulerGammaN(2, 0), mpfloat(1));
	test_value(IncompleteEulerGammaN(9, 4), mpfloat(39458.626321444478531));
	// test_value(IncompleteEulerGammaN(20, 5), mpfloat(1.2164505841529114399347999768364725862221807457248e17));
	test_value(IncompleteEulerGammaN(-5.2, 1.4), mpfloat(0.006277156776673721));
	test_value(IncompleteEulerGammaN( 0.0, 2.3), mpfloat(0.032502267171621586));
	test_value(IncompleteEulerGammaN(-1.0, 2.3), mpfloat(0.011088534446988742));
	test_value(IncompleteEulerGammaN(-1.5, 2.3), mpfloat(0.006575406287319803));
	test_value(IncompleteEulerGammaN(-2.0, 2.3), mpfloat(0.003931993997942657));

	test_value(beta(5, 4.34), 0.167274);
	test_value(beta(2, 0.65), 0.110282);
	
	test_value(eta(5, 1.3, 2, 0.25, 0.8), 0.171359);
	test_value(eta(8, 10.74, 2, 1.2, 0.3), 0.00323026);
	test_value(eta(3, 0.3, 7, 0.1, 0.2), 0.0000135);

	test_value(gamma(0, 0.9, 3, 0.65, 0.5), 0.00847603);
	test_value(gamma(0, 10.74, 2, 1.2, 0.3), 0.000815474);
	test_value(gamma(5, 1.3, 2, 0.25, 0.8), 0.000217258);
	test_value(gamma(8, 10.74, 2, 1.2, 0.3), 0.0000277005);
	test_value(gamma(3, 4.3, 7, 0.1, 6.2), 4.01770600);

	cout << "\nTesting compute_alpha_factors()...\n";
	for (const aF_test& test : all_tests) 
	{
		expr f = parse(test.fn);
		cout << "\nAlpha-factors of "<<f<<" with q="<<test.ctmc_q<<endl;
		cout << "Using f_g(x) = " << simplify(f) << endl;
		std::vector<pair<mpfloat, mpfloat>> factors;
		factors = compute_alpha_factors(simplify(f), test.ctmc_q, test.accuracy, true);
		if (test.num_entries == factors.size()) {
			for (size_t i=0; i<factors.size(); i++) {
				test_value(test.alphaF[i], factors[i].first);
				test_value(test.alphaCF[i], factors[i].second);
				cout << setw(3)<<i<<": " << float(factors[i].first) << "   " 
					 << float(factors[i].second) << endl;
			}
			cout << "ALL OK." << endl;
		}
		else { 
			for (size_t i=0; i<factors.size(); i++) {
				cout << setw(3)<<i<<": " << float(factors[i].first) << "   " 
					 << float(factors[i].second) << endl;
			}
			cerr << "Different number of result entries!!" << endl;
			throw;
		}
		cout << endl;
	}
}

//=============================================================================

int main (int argc, char **argv) {
	mpfloat ctmc_q, accuracy;
	const char *expr_str;

	if (argc == 2 && 0==strcmp(argv[1], "test")) {
		unit_test();
		return 0;
	}
	if (argc != 4) {
		cout << g_banner << endl;
		return 0;
	}
	expr_str = argv[1];
	ctmc_q = mpfloat(argv[2]);
	accuracy = mpfloat(argv[3]);

	cout << "Computing alpha-factors of:\n  " << expr_str 
	     << "\nfor a uniformized CTMC of rate " << ctmc_q <<", accuracy=" << accuracy << endl;

	cout << setprecision(numeric_limits<double>::digits10);
	expr fg = parse(expr_str);
	expr sfg = simplify(fg);
	cout << "Using f_g(x) = " << sfg << endl;
	std::vector<pair<mpfloat, mpfloat>> factors;
	factors = compute_alpha_factors(sfg, ctmc_q, accuracy, true);
	cout << "Factors:     " << factors.size() << endl;

	for (size_t i=0; i<factors.size(); i++)
		cout << factors[i].first << endl;
	cout << endl;
	for (size_t i=0; i<factors.size(); i++)
		cout << factors[i].second << endl;
	cout << endl;	

	return 0;
}

//=============================================================================

#endif // ALPHAFACTORSLIB








