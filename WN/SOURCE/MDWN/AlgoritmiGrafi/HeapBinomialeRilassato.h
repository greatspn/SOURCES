/******************************************************************************\
*
* File:          HeapBinomialeRilassato.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Last changed:  February 21, 2006
* Modifications: Fixed a problem when using gcc 4.0
* Changed by:    Francesco Iovine
*
* Purpose:       Fig.8.8
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _HEAPBINOMIALERILASSATO_H
#define _HEAPBINOMIALERILASSATO_H

#include "HeapBinomiale.h"


namespace asd {

/*
Figura 8.8
Realizzazione di una coda di priorita' mediante un heap binomiale rilassato,
implementato con una rappresentazione collegata basata su liste di figli.
*/
template<class Elem, class Chiave>
class HeapBinomialeRilassato
    : public HeapBinomiale<Elem, Chiave> {
protected:  // dati
    /*  std::list<nodo_binomiale<Elem, Chiave>*> foresta;
        unsigned itemsCount; */
    nodo_binomiale<Elem, Chiave> *min;

public:  // operazioni
    HeapBinomialeRilassato();
    HeapBinomialeRilassato(const std::vector< std::pair<Elem, Chiave> > &vec);
    HeapBinomialeRilassato(const HeapBinomialeRilassato<Elem, Chiave> &h);
    ~HeapBinomialeRilassato();

    Elem findMin() const;
    nodo_binomiale<Elem, Chiave> *insert(const Elem &e, const Chiave &k);
    Elem deleteMin();
    void decreaseKey(nodo_binomiale<Elem, Chiave> *v, const Chiave &d);
    void merge(const HeapBinomialeRilassato &c1, const HeapBinomialeRilassato &c2);

protected:  // procedure interne
    void aggiornaMin();

    /*  protected:  // dati per uso interno
        std::list<nodo_binomiale<Elem, Chiave>*> lista_nodi;  //lista di nodi*/

private:  // typedefs
    typedef typename std::list<nodo_binomiale<Elem, Chiave>*>::iterator foresta_iterator;
    typedef typename std::list<nodo_binomiale<Elem, Chiave>*>::const_iterator foresta_const_iterator;
    typedef typename std::vector< std::pair<Elem, Chiave> >::const_iterator vec_iterator;
};



/* PUBLIC */

/*
Costruttore di default.
*/
template<class Elem, class Chiave>
HeapBinomialeRilassato<Elem, Chiave>::HeapBinomialeRilassato()
    : HeapBinomiale<Elem, Chiave>()
    , min(0)
{}



/*
Costruttore per la creazione di un nuovo HeapBinomialeRilassato non vuoto.
*/
template<class Elem, class Chiave>
HeapBinomialeRilassato<Elem, Chiave>::HeapBinomialeRilassato(const std::vector< std::pair<Elem, Chiave> > &vec)
    : HeapBinomiale<Elem, Chiave>() {
    for (vec_iterator it = vec.begin(); it != vec.end(); ++it)
        insert((*it).first, (*it).second);
}



/*
Costruttore di copia.
*/
template<class Elem, class Chiave>
HeapBinomialeRilassato<Elem, Chiave>::HeapBinomialeRilassato(const HeapBinomialeRilassato<Elem, Chiave> &h)
    : HeapBinomiale<Elem, Chiave>(h)
    , min(h.min)
{}



/*
Distruttore.
*/
template<class Elem, class Chiave>
HeapBinomialeRilassato<Elem, Chiave>::~HeapBinomialeRilassato()
{}



/*
Ricerca del minimo.
*/
template<class Elem, class Chiave>
Elem HeapBinomialeRilassato<Elem, Chiave>::findMin() const {
    assert(this->itemsCount != 0);

    return min->elem;
}



/*
Inserimento di un nuovo elemento.
*/
template<class Elem, class Chiave>
nodo_binomiale<Elem, Chiave> *HeapBinomialeRilassato<Elem, Chiave>::insert(const Elem &e,
        const Chiave &k) {
    /* aggiunge alla foresta un nuovo B0 con dati 'e' e 'k' */
    nodo_binomiale<Elem, Chiave> *v = new nodo_binomiale<Elem, Chiave>(e, k);
    (this->foresta).push_back(v);
    ++(this->itemsCount);

    aggiornaMin();

    (this->lista_nodi).push_back(v);
    return v;
}



/*
Estrazione del minimo.
*/
template<class Elem, class Chiave>
Elem HeapBinomialeRilassato<Elem, Chiave>::deleteMin() {
    assert(this->itemsCount != 0);

    Elem minElem = HeapBinomiale<Elem, Chiave>::deleteMin();
    aggiornaMin();

    return minElem;
}



/*
Decremento di una chiave.
*/
template<class Elem, class Chiave>
void HeapBinomialeRilassato<Elem, Chiave>::decreaseKey(nodo_binomiale<Elem, Chiave> *v,
        const Chiave &d) {
    HeapBinomiale<Elem, Chiave>::decreaseKey(v, d);
    aggiornaMin();
}



/*
Fusione di due code di priorita' in un'unica coda di priorita'.
*/
template<class Elem, class Chiave>
void HeapBinomialeRilassato<Elem, Chiave>::merge(const HeapBinomialeRilassato &c1,
        const HeapBinomialeRilassato &c2) {
    /* unisci le liste delle radici dei due heap binomiali */
    for (foresta_const_iterator it = (c1.foresta).begin(); it != (c1.foresta).end(); ++it)
        (this->foresta).push_back(*it);
    for (foresta_const_iterator it = (c2.foresta).begin(); it != (c2.foresta).end(); ++it)
        (this->foresta).push_back(*it);

    this->itemsCount += (c1.itemsCount + c2.itemsCount);

    aggiornaMin();
}



// PRIVATE

/*
Aggiorna 'min'.
*/
template<class Elem, class Chiave>
void HeapBinomialeRilassato<Elem, Chiave>::aggiornaMin() {
    nodo_binomiale<Elem, Chiave> *r;

    min = (this->foresta).front();

    for_each_radice(r) {
        if (r->chiave < min->chiave) {
            min = r;
        }
    }
}

}  // namespace asd


#endif /* _HEAPBINOMIALERILASSATO_H */



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
