/******************************************************************************\
*
* File:          Coda.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Purpose:       Fig.3.7
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _CODA_H
#define _CODA_H

#include <list>


namespace asd {

/*
Figura 3.7
Il tipo di dato Coda.
*/
template<class Elem>
class Coda {
private:
    std::list<Elem> S;

public:
    Coda() {}
    ~Coda() {};

    bool isEmpty() const;
    void enqueue(const Elem &e);
    Elem dequeue();
    Elem first() const;
};



/* PUBLIC */

/*
Restituisce true se S e' vuota, e false altrimenti.
*/
template<class Elem>
bool Coda<Elem>::isEmpty() const {
    return S.empty();
}



/*
Aggiunge e come ultimo elemento di S.
*/
template<class Elem>
void Coda<Elem>::enqueue(const Elem &e) {
    S.push_back(e);
}



/*
Toglie da S il primo elemento e lo restituisce.
*/
template<class Elem>
Elem Coda<Elem>::dequeue() {
    Elem e = S.front();
    S.pop_front();
    return e;
}



/*
Restituisce il primo elemento di S (senza toglierlo da S).
*/
template<class Elem>
Elem Coda<Elem>::first() const {
    return S.front();
}

}  // namespace asd


#endif /* _CODA_H */



/******************************************************************************\
 * Copyright (C) 2006 ASD Team

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
\******************************************************************************/
