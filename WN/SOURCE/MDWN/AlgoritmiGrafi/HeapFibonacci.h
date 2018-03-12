/******************************************************************************\
*
* File:          HeapFibonacci.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Last changed:  February 21, 2006
* Modifications: Fixed a problem when using gcc 4.0
* Changed by:    Francesco Iovine
*
* Purpose:       Fig.8.10, Fig.8.11
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _HEAPFIBONACCI_H
#define _HEAPFIBONACCI_H

#include "HeapBinomialeRilassato.h"


namespace asd {

/*
Figura 8.10
Realizzazione di una coda di priorita' mediante un heap di Fibonacci,
implementato con una rappresentazione collegata basata su liste di figli.
*/
template<class Elem, class Chiave>
class HeapFibonacci
    : public HeapBinomialeRilassato<Elem, Chiave> {
protected:  // dati
    /*  std::list<nodo_binomiale<Elem, Chiave>*> foresta;
        unsigned itemsCount;
        nodo_binomiale<Elem, Chiave>* min; */

public:  // operazioni
    HeapFibonacci();
    HeapFibonacci(const std::vector< std::pair<Elem, Chiave> > &vec);
    HeapFibonacci(const HeapFibonacci<Elem, Chiave> &h);
    ~HeapFibonacci();

    void decreaseKey(nodo_binomiale<Elem, Chiave> *v, const Chiave &d);

private:  // procedure interne
    void staccaInCascata(nodo_binomiale<Elem, Chiave> *v);

    /*  protected:  // dati per uso interno
        std::list<nodo_binomiale<Elem, Chiave>*> lista_nodi;  //lista di nodi*/

private:  // typedefs
    typedef typename std::list<nodo_binomiale<Elem, Chiave>*>::iterator foresta_iterator;
};



/* PUBLIC */

/*
Costruttore di default.
*/
template<class Elem, class Chiave>
HeapFibonacci<Elem, Chiave>::HeapFibonacci()
    : HeapBinomialeRilassato<Elem, Chiave>()
{}



/*
Costruttore per la creazione di un nuovo HeapFibonacci non vuoto.
*/
template<class Elem, class Chiave>
HeapFibonacci<Elem, Chiave>::HeapFibonacci(const std::vector< std::pair<Elem, Chiave> > &vec)
    : HeapBinomialeRilassato<Elem, Chiave>(vec)
{}



/*
Costruttore di copia.
*/
template<class Elem, class Chiave>
HeapFibonacci<Elem, Chiave>::HeapFibonacci(const HeapFibonacci<Elem, Chiave> &h)
    : HeapBinomialeRilassato<Elem, Chiave>(h)
{}



/*
Distruttore.
*/
template<class Elem, class Chiave>
HeapFibonacci<Elem, Chiave>::~HeapFibonacci()
{}



/*
Decremento di una chiave.
*/
template<class Elem, class Chiave>
void HeapFibonacci<Elem, Chiave>::decreaseKey(nodo_binomiale<Elem, Chiave> *v, const Chiave &d) {
    v->chiave -= d;
    if (v->chiave < (this->min)->chiave) this->min = v;

    if (v->padre) {
        if (v->chiave < (v->padre)->chiave)  staccaInCascata(v);
    }
}



/* PRIVATE */

/*
Figura 8.11
Procedura 'staccaInCascata' per il ripristino delle proprieta'
di struttura e ordinamento in un heap di Fibonacci.
La procedura taglia l'arco tra 'v' e il suo genitore 'u'.
Se 'u' non e' radice e aveva gia' perso un figlio, stacca ricorsivamente anche 'u'
in modo che ogni nodo non-radice perda al piu' un figlio.
*/
template<class Elem, class Chiave>
void HeapFibonacci<Elem, Chiave>::staccaInCascata(nodo_binomiale<Elem, Chiave> *v) {
    /* u = padre(v) */
    nodo_binomiale<Elem, Chiave> *u = v->padre;

    /* togli 'v' dalla lista dei figli di 'u' e aggiungi 'v' alla lista delle radici dell'heap. */
    for (foresta_iterator it = (u->lista_figli).begin(); it != (u->lista_figli).end(); ++it) {
        if (*it == v) {
            (u->lista_figli).erase(it);
            break;
        }
    }
    v->padre = 0;
    (this->foresta).push_back(v);

    v->lutto = false;
    if (u->padre != 0) {  // if( u non e' una radice ) then
        if (u->lutto) staccaInCascata(u);
        else u->lutto = true;
    }
}

}  // namespace asd


#endif /* _HEAPFIBONACCI_H */



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
