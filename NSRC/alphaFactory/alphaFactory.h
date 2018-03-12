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
#ifndef ALPHA_FACTORY_H
#define ALPHA_FACTORY_H
//=============================================================================

// Compute the alpha factors of a general distribution.
//    @fg_expr   the expression of the general distribution function f(x)
//    @q         the rate of the CTMC
//    @accuracy  requested accuracy for the truncation of the alpha-factors.
//
//       Example of f(x) functions are the following:
//         3 * Exp(-0.2 * x)
//         0.4 * Pow(x, 2) * Exp(-0.5 * x) * Rect(0, 4)
//         0.3 * DiracDelta(5) + 0.7 * DiracDelta(10)
//       
//       Language elements include:
//         1.2345               Number term.
//         +,-,*,/              Basic operands.
//         Pow(t, e)            Power of t with exponent e.
//         Exp(t)               Exponential of t
//         Log(t)               Natural logarithm of t
//      
//       and the two functions to specify a discontinuous impulse:
//         DiracDelta(t)        Deterministic Dirac impulse at time t
//         I(t)                 Synonim for DiracDelta(t).
//         Rect(a,b)            Rectangular impulse of 1 in range (a,b)
//       
//       and the convenience (non-primitive) functions:
//         Uniform(a,b)         Uniform distribution in (a,b) = Rect(a,b)/(b-a)
//         Triangular(a,b)      Triangular distribution in (a,b), peak in (a+b)/2
//         Erlang(l,r)          Erlang distribution of rate l and r phases
//         TruncatedExp(l,t)    Exponential distribution of rate l truncated at time t
//         Pareto(k,a)          Pareto distribution
// Returns: a pair of vectors containing the alpha factors of f(x) and (1-F(x))
//
const std::vector<pair<double, double>>&
compute_alpha_factors_dbl(const char* fg_expr, const double q, const double accuracy);


// Verify that the @fg_expr text is a proper function for the computation of alpha factors.
// Verification includes:
//    - the function is tested to be synctactically correct.
//    - the area of the function is tested to be 1.0
//    - the function is tested to be symbolically integrable
//
// Returns: nullptr if @fg_expr is a valid distribution for the computation of the
//          alpha-factors. Returns an error message if one of the above tests
//          fails.
const char* verify_alpha_factors_expr(const char* fg_expr);

//=============================================================================
#endif // ALPHA_FACTORY_H
