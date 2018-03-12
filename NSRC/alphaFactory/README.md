
# alphaFactory

A software tool for the computation of alpha-factors of probability distribution functions.

The tool takes in input the PDF function f(x), the CTMC rate mu and the accuracy epsilon, 
and outputs the instantaneous and accumulated alpha-factors coefficients needed by the Uniformization function.
The tool implements the method found in the book "Performance Analysis of Communication Systems - Modeling with 
non-Markovian Stochastic Petri nets" by Reinhard German, with a modified tail truncation that is 
correct for both defectives and non-defectives PDFs.

Written by: Elvio G. Amparore, Universita' di Torino, 2017.


## How to compile

alphaFactory requires Boost-C++ (version 1.60 or greater), and optionally gmp.
It also requires a C++11 or better compiler.

alphaFactory is known to compile on MacOSX and Linux with this command:

 * Boost version:
   ```
   g++ -I/usr/local/include -L/usr/local/lib alphafactory.cpp  -std=c++11 -O2 -o alphafactory
   ```

 * GMP version:
   ```
   g++ -I/usr/local/include -L/usr/local/lib -DUSE_GMP=1 alphafactory.cpp -std=c++11 -O2 -lgmp -o alphafactory
   ```


## Usage examples

The command has this form:

```
./alphaFactory  "function"  <CTMC rate>  <accuracy>
```

The description of the function grammar is found in alphaFactory.h

These examples show how to run the tool:

```
./alphafactory '0.3*I(5) + 0.5*I(10)' 3.0 0.0000001
./alphafactory 'Erlang(0.75,4)' 1.25 0.0000001
./alphafactory "Triangular(2,6)" 0.85  0.0000001
./alphafactory "27/10 * Exp(-3 * x) * x + 2/5 * Exp(-2 * x) * Pow(x,2) + 1/10 * Exp(-1 * x) * Pow(x, 3)" 1.5 0.0000001
```

A unit test can be run using:

```
./alphaFactory test
```


## API reference

Read alphaFactory.h for the description of the exported C++ API functions.


## License

Copyright (c) 2017, Universita' di Torino, Elvio G. Amparore <amparore at di.unito.it>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


