/******************************************************************************\
*
* File:          HeapBinomiale.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Purpose:       Fig.8.6, Fig.8.7
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _HEAPBINOMIALE_H
#define _HEAPBINOMIALE_H

#include "CodaPriorita.h"
#include "Coda.h"

#include <list>
#include <vector>
#include <cassert>
#include <iostream>


#define for_each_radice(r)                                             \
        r = *((this->foresta).begin());                                \
        for (foresta_const_iterator it = (this->foresta).begin();      \
             it != (this->foresta).end();                                      \
             ++it, r = *it)

#define for_each_figlio_binomiale(v,u)                                 \
        v = *((u->lista_figli).begin());                               \
        for (foresta_const_iterator it = (u->lista_figli).begin();     \
             it != (u->lista_figli).end();                             \
             ++it, v = *it)



namespace asd {

template<class Elem, class Chiave>
class HeapBinomiale;

template<class Elem, class Chiave>
class HeapBinomialeRilassato;

template<class Elem, class Chiave>
class HeapFibonacci;

/*
Nodo di un heap binomiale,
implementato con una rappresentazione collegata basata su liste di figli.
*/
template<class Elem, class Chiave>
class nodo_binomiale {
protected:
    Elem elem;
    Chiave chiave;
    nodo_binomiale *padre;
    std::list<nodo_binomiale *> lista_figli;
    unsigned altezza;
    bool lutto;  // per HeapFibonacci

public:
    nodo_binomiale(Elem elem_ = Elem(), Chiave chiave_ = Chiave(),
                   nodo_binomiale *padre_ = 0)
        : elem(elem_)
        , chiave(chiave_)
        , padre(padre_)
        , altezza(0)
        , lutto(false)
    {}

    nodo_binomiale(const nodo_binomiale &n)
        : elem(n.elem)
        , chiave(n.chiave)
        , padre(n.padre)
        , lista_figli(n.lista_figli)
        , altezza(n.altezza)
        , lutto(n.lutto)
    {}

    ~nodo_binomiale()
    {}

    inline Elem getElem() const { return elem; }
    inline Chiave getChiave() const { return chiave; }

    friend class HeapBinomiale<Elem, Chiave>;
    friend class HeapBinomialeRilassato<Elem, Chiave>;
    friend class HeapFibonacci<Elem, Chiave>;
};



/*
Figura 8.6
Realizzazione di una coda di priorita' mediante un heap binomiale,
implementato con una rappresentazione collegata basata su liste di figli.
*/
template<class Elem, class Chiave>
class HeapBinomiale
    : public CodaPriorita<Elem, Chiave> {
protected:  // dati
    std::list<nodo_binomiale<Elem, Chiave>*> foresta;  //lista di radici
    unsigned itemsCount;


public:  // operazioni
    HeapBinomiale();
    HeapBinomiale(const std::vector< std::pair<Elem, Chiave> > &vec);
    HeapBinomiale(const HeapBinomiale<Elem, Chiave> &h);
    ~HeapBinomiale();

    Elem findMin() const;
    nodo_binomiale<Elem, Chiave> *insert(const Elem &e, const Chiave &k);
    Elem deleteMin();
    void decreaseKey(nodo_binomiale<Elem, Chiave> *v, const Chiave &d);
    void delete_(nodo_binomiale<Elem, Chiave> *v);
    void increaseKey(nodo_binomiale<Elem, Chiave> *v, const Chiave &d);
    void merge(const HeapBinomiale<Elem, Chiave> &c1, const HeapBinomiale<Elem, Chiave> &c2);

    unsigned getItemsCount() const;
    bool isEmpty() const;

    void stampa_radici() const;
    void stampa() const;

private:  // procedure interne
    void ristruttura();
    void muoviAlto(nodo_binomiale<Elem, Chiave> *v);
    void scambia(nodo_binomiale<Elem, Chiave> *v, nodo_binomiale<Elem, Chiave> *u);

private:  // typedefs
    typedef typename std::list<nodo_binomiale<Elem, Chiave>*>::iterator foresta_iterator;
    typedef typename std::list<nodo_binomiale<Elem, Chiave>*>::const_iterator foresta_const_iterator;
    typedef typename std::vector< std::pair<Elem, Chiave> >::const_iterator vec_iterator;

protected:  // dati per uso interno
    std::list<nodo_binomiale<Elem, Chiave>*> lista_nodi;  //lista di nodi
};



/* PUBLIC */

/*
Costruttore di default.
*/
template<class Elem, class Chiave>
HeapBinomiale<Elem, Chiave>::HeapBinomiale()
    : itemsCount(0)
{}



/*
Costruttore per la creazione di un nuovo HeapBinomiale non vuoto.
*/
template<class Elem, class Chiave>
HeapBinomiale<Elem, Chiave>::HeapBinomiale(const std::vector< std::pair<Elem, Chiave> > &vec)
    : itemsCount(0) {
    for (vec_iterator it = vec.begin(); it != vec.end(); ++it)
        insert((*it).first, (*it).second);
}



/*
Costruttore di copia.
*/
template<class Elem, class Chiave>
HeapBinomiale<Elem, Chiave>::HeapBinomiale(const HeapBinomiale<Elem, Chiave> &h)
    : itemsCount(h.itemsCount)
{}



/*
Distruttore.
*/
template<class Elem, class Chiave>
HeapBinomiale<Elem, Chiave>::~HeapBinomiale() {
    for (foresta_iterator it = lista_nodi.begin(); it != lista_nodi.end(); ++it) {
        if (*it) delete(*it);
    }
}



/*
Ricerca del minimo.
*/
template<class Elem, class Chiave>
Elem HeapBinomiale<Elem, Chiave>::findMin() const {
    assert(itemsCount != 0);

    nodo_binomiale<Elem, Chiave> *r;
    nodo_binomiale<Elem, Chiave> *y = *(foresta.begin());

    /* trova il minimo y nella lista di radici */
    for_each_radice(r) {
        if (r->chiave < y->chiave)  y = r;
    }

    /* restituisci l'elemento associato ad y */
    return y->elem;
}



/*
Inserimento di un nuovo elemento.
*/
template<class Elem, class Chiave>
nodo_binomiale<Elem, Chiave> *HeapBinomiale<Elem, Chiave>::insert(const Elem &e,
        const Chiave &k) {
    /* aggiunge alla foresta un nuovo B0 con dati 'e' e 'k' */
    nodo_binomiale<Elem, Chiave> *v = new nodo_binomiale<Elem, Chiave>(e, k);
    foresta.push_back(v);
    ++itemsCount;

    /* ripristina poi la proprieta' di unicita' nella foresta
       mediante fusioni successive dei doppioni Bi */
    ristruttura();

    lista_nodi.push_back(v);
    return v;
}



/*
Estrazione del minimo.
*/
template<class Elem, class Chiave>
Elem HeapBinomiale<Elem, Chiave>::deleteMin() {
    assert(itemsCount != 0);

    foresta_iterator itmin = foresta.begin();

    /* trova l'albero Th con radice a chiave minima */
    for (foresta_iterator it = foresta.begin(); it != foresta.end(); ++it) {
        if ((*it)->chiave < (*itmin)->chiave) {
            itmin = it;
        }
    }
    Elem min_elem = (*itmin)->elem;

    /* togliendo la radice a Th, esso si spezza in h alberi binomiali T0,...,Th-1,
       che vengono aggiunti alla foresta */
    nodo_binomiale<Elem, Chiave> *v;
    for_each_figlio_binomiale(v, (*itmin)) {
        v->padre = 0;
        foresta.push_back(v);
    }
    foresta.erase(itmin);
    --itemsCount;

    /* ripristina la proprieta' di unicita' nella foresta
       mediante fusioni successive dei doppioni Bi */
    ristruttura();

    return min_elem;
}



/*
Decremento di una chiave.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::decreaseKey(nodo_binomiale<Elem, Chiave> *v, const Chiave &d) {
    /* decrementa di 'd' la chiave del nodo 'v' contenente l'elemento 'e'. */
    v->chiave -= d;

    /* ripristina poi la proprieta' di unicita' nella foresta
       mediante ripetuti scambi di nodi */
    muoviAlto(v);
}



/*
Cancellazione di un elemento.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::delete_(nodo_binomiale<Elem, Chiave> *v) {
    std::cout << "\n\tdecreaseKey(v, MENO_INFINITO);"
              << "\n\tdeleteMin();";
}



/*
Incremento di una chiave.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::increaseKey(nodo_binomiale<Elem, Chiave> *v, const Chiave &d) {
    std::cout << "\n\tChiave k = v->chiave;";
    delete_(v);
    std::cout << "\n\tinsert(v->elem, k);";
}



/*
Fusione di due code di priorita' in un'unica coda di priorita'.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::merge(const HeapBinomiale<Elem, Chiave> &c1,
                                        const HeapBinomiale<Elem, Chiave> &c2) {
    /* unisci le liste delle radici dei due heap binomiali */
    for (foresta_const_iterator it = (c1.foresta).begin(); it != (c1.foresta).end(); ++it)
        foresta.push_back(*it);
    for (foresta_const_iterator it = (c2.foresta).begin(); it != (c2.foresta).end(); ++it)
        foresta.push_back(*it);

    itemsCount += (c1.itemsCount + c2.itemsCount);

    /* i duplicati sono rimossi dalla nuova lista richiamando la procedura 'ristruttura' */
    ristruttura();
}



/*
Restituisce il numero di elementi nell'heap binomiale.
*/
template<class Elem, class Chiave>
unsigned HeapBinomiale<Elem, Chiave>::getItemsCount() const {
    return itemsCount;
}



/*
Verifica in tempo costante se un heap binomiale e' vuoto.
*/
template<class Elem, class Chiave>
bool HeapBinomiale<Elem, Chiave>::isEmpty() const {
    return (foresta.size() == 0);
}



/*
Stampa le chiavi delle radici presenti nella foresta dell'heap binomiale.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::stampa_radici() const {
    nodo_binomiale<Elem, Chiave> *r;
    for_each_radice(r)
    std::cout << "(" << r->elem << ", " << r->chiave << ")(B" << r->altezza << ")  ";
}



/*
Stampa le chiavi dell'heap binomiale visitando ogni albero mediante un visita BFS.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::stampa() const {
    nodo_binomiale<Elem, Chiave> *u, *r, *v;

    Coda<nodo_binomiale<Elem, Chiave>*> C;
    for_each_radice(r) {
        C.enqueue(r);
        while (!(C.isEmpty())) {
            u = C.dequeue();
            if (u != 0) {
                std::cout << u->chiave;
                if (u->padre) std::cout << "(->" << (u->padre)->chiave << ")";
                std::cout << "  ";
                for_each_figlio_binomiale(v, u) C.enqueue(v);
            }
        }
        std::cout << "\n";
    }
}



/* PRIVATE */

/*
Figura 8.7
Procedura 'ristruttura' per il ripristino della proprieta' di unicita' in un heap binomiale.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::ristruttura() {
    bool esistono_ancora_due_Bi = true;

    for (unsigned i = 0; esistono_ancora_due_Bi; ++i) {
        esistono_ancora_due_Bi = false;
        foresta_iterator Bi = foresta.begin();
        foresta_iterator last = foresta.end();
        if (Bi == last) return;
        foresta_iterator Bj = Bi;
        while (++Bj != last) {
            if ((*Bi)->altezza == i) {
                esistono_ancora_due_Bi = true;
                if ((*Bj)->altezza == i) {
                    if ((*Bi)->chiave < (*Bj)->chiave) {
                        (*Bj)->padre = (*Bi);
                        ++((*Bi)->altezza);
                        ((*Bi)->lista_figli).push_back((*Bj));
                        foresta.erase(Bj);
                        Bj = Bi;
                    }
                    else { /* if ((*Bj)->chiave < (*Bi)->chiave) */
                        (*Bi)->padre = (*Bj);
                        ++((*Bj)->altezza);
                        ((*Bj)->lista_figli).push_back((*Bi));
                        foresta_iterator tmp = Bi;
                        ++tmp;
                        foresta.erase(Bi);
                        Bi = tmp;
                        Bj = Bi;
                    }
                }
            }
            else {
                Bi = Bj;
            }
        }
    }
}



/*
Figura 8.4
Procedura 'muoviAlto' per il ripristino della proprieta' di ordinamento a heap.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::muoviAlto(nodo_binomiale<Elem, Chiave> *v) {
    while ((v->padre != 0) && (v->chiave < (v->padre)->chiave)) {
        scambia(v, v->padre);
    }
}



/*
Scambia di posizione i nodi 'v' ed 'u', dove 'u' e' padre di 'v'.
*/
template<class Elem, class Chiave>
void HeapBinomiale<Elem, Chiave>::scambia(nodo_binomiale<Elem, Chiave> *v, nodo_binomiale<Elem, Chiave> *u) {
    /* Ora il padre di 'v' e' il padre di 'u', il cui figlio 'u' diventa 'v' */
    v->padre = u->padre;
    if (v->padre) {
        for (foresta_iterator it = ((v->padre)->lista_figli).begin(); it != ((v->padre)->lista_figli).end(); ++it) {
            if (*it == u) *it = v;
        }
    }
    else {  // se 'u' e' una radice, elimina 'u' e metti 'v' al suo posto
        for (foresta_iterator it = foresta.begin(); it != foresta.end(); ++it) {
            if (*it == u) *it = v;
        }
    }

    /* Ora i figli di 'v' hanno come padre 'u' */
    for (foresta_iterator it = (v->lista_figli).begin(); it != (v->lista_figli).end(); ++it) {
        (*it)->padre = u;
    }

    /* Ora i figli di 'u' hanno come padre 'v'. */
    for (foresta_iterator it = (u->lista_figli).begin(); it != (u->lista_figli).end(); ++it) {
        if (*it == v) *it = u;
        (*it)->padre = v;
    }

    /* Scambio le liste dei figli di 'v' e 'u' */
    std::list< nodo_binomiale<Elem, Chiave>* > temp_lista_figli = v->lista_figli;
    v->lista_figli = u->lista_figli;
    u->lista_figli = temp_lista_figli;


    /* Scambio le altezze di 'v' e 'u' */
    unsigned temp_altezza = v->altezza;
    v->altezza = u->altezza;
    u->altezza = temp_altezza;
}

}  // namespace asd


#endif /* _HEAPBINOMIALE_H */



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
