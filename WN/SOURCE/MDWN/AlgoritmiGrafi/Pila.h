/******************************************************************************\
*
* File:          Pila.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Purpose:       Fig.3.6
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _PILA_H
#define _PILA_H

#include <list>


namespace asd {

/*
Figura 3.6
Il tipo di dato Pila.
*/
template<class Elem>
class Pila {
private:
    std::list<Elem> S;

public:
    Pila() {}
    ~Pila() {};

    bool isEmpty() const;
    void push(const Elem &e);
    Elem pop();
    Elem top() const;
};



/* PUBLIC */

/*
Restituisce true se S e' vuota, e false altrimenti.
*/
template<class Elem>
bool Pila<Elem>::isEmpty() const {
    return S.empty();
}



/*
Aggiunge e come ultimo elemento di S.
*/
template<class Elem>
void Pila<Elem>::push(const Elem &e) {
    S.push_back(e);
}



/*
Toglie da S l'ultimo elemento e lo restituisce.
*/
template<class Elem>
Elem Pila<Elem>::pop() {
    Elem e = S.back();
    S.pop_back();
    return e;
}



/*
Restituisce l'ultimo elemento di S (senza toglierlo da S).
*/
template<class Elem>
Elem Pila<Elem>::top() const {
    return S.back();
}

}  // namespace asd


#endif /* _PILA_H */



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
