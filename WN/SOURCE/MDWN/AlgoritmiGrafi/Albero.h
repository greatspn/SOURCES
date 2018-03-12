/******************************************************************************\
*
* File:          Albero.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Purpose:       Fig.3.10
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _ALBERO_H
#define _ALBERO_H

#include <list>


#define for_each_figlio(v,u)                                      \
        v = *((u->lista_figli).begin());                          \
        for (lista_figli_iterator it = (u->lista_figli).begin();  \
             it != (u->lista_figli).end();                        \
             ++it, v = *it)

#define for_each_rev_figlio(v,u)                                       \
        v = *((u->lista_figli).rbegin());                              \
        for (lista_figli_rev_iterator it = (u->lista_figli).rbegin();  \
             it != (u->lista_figli).rend();                            \
             ++it, v = *it)


namespace asd {

template<class Chiave>
class Albero;

/*
Nodo per una rappresentazione collegata di albero,
basata su liste di figli.
*/
template<class Chiave>
class nodo {
private:
    Chiave chiave;

public:
    nodo *padre;
    std::list<nodo *> lista_figli;

    nodo(Chiave chiave_ = Chiave(), nodo *padre_ = 0)
        : chiave(chiave_)
        , padre(padre_)
    {}

    nodo(const nodo &n)
        : chiave(n.chiave)
        , padre(n.padre)
        , lista_figli(n.lista_figli)
    {}

    ~nodo()
    {}

    inline Chiave getChiave() const { return chiave; }

    typedef typename std::list< nodo<Chiave>* >::iterator lista_figli_iterator;
    typedef typename std::list< nodo<Chiave>* >::const_iterator lista_figli_const_iterator;
    typedef typename std::list< nodo<Chiave>* >::reverse_iterator lista_figli_rev_iterator;
    typedef typename std::list< nodo<Chiave>* >::const_reverse_iterator lista_figli_const_rev_iterator;

    friend class Albero<Chiave>;
};



/*
Figura 3.10
Rappresentazione collegata di albero, basata su liste di figli.
*/
template<class Chiave>
class Albero {
private:  // dati
    nodo<Chiave> *radice;
    unsigned itemsCount;

public:  // operazioni
    Albero();
    Albero(const Albero<Chiave> &t);
    ~Albero();

    unsigned numNodi() const;
    unsigned grado(nodo<Chiave> *v) const;
    nodo<Chiave> *padre(nodo<Chiave> *v = 0) const;
    nodo<Chiave> *figli(nodo<Chiave> *v) const;
    nodo<Chiave> *aggiungiNodo(const Chiave &data, nodo<Chiave> *u = 0);
    void aggiungiSottoalbero(Albero<Chiave> &a, nodo<Chiave> *u);
    void rimuoviSottoalbero(nodo<Chiave> *v, Albero<Chiave> &sottoalbero);
    void spostaSottoalbero(nodo<Chiave> *v, nodo<Chiave> *u);

    void clear();

private:  // procedure interne
    void distruggiSottoalbero(nodo<Chiave> *v);
    void setItemsCount(nodo<Chiave> *v);

public:
    typedef typename nodo<Chiave>::lista_figli_iterator lista_figli_iterator;
    typedef typename nodo<Chiave>::lista_figli_const_iterator lista_figli_const_iterator;
    typedef typename nodo<Chiave>::lista_figli_rev_iterator lista_figli_rev_iterator;
    typedef typename nodo<Chiave>::lista_figli_const_rev_iterator lista_figli_const_rev_iterator;
};



/* PUBLIC */

/*
Costruttore di default.
*/
template<class Chiave>
Albero<Chiave>::Albero()
    : radice(0)
    , itemsCount(0)
{}



/*
Costruttore di copia.
*/
template<class Chiave>
Albero<Chiave>::Albero(const Albero<Chiave> &t)
    : radice(t.radice)
    , itemsCount(t.itemsCount)
{}



/*
Distruttore.
*/
template<class Chiave>
Albero<Chiave>::~Albero() {
    distruggiSottoalbero(radice);
}



/*
Restituisce il numero di nodi presenti nell'albero.
*/
template<class Chiave>
unsigned Albero<Chiave>::numNodi() const {
    return itemsCount;
}



/*
Restituisce il numero di figli del nodo v.
*/
template<class Chiave>
unsigned Albero<Chiave>::grado(nodo<Chiave> *v) const {
    return (v->lista_figli).size();
}



/*
Restituisce il padre del nodo v nell'albero.
Restituisce la radice dell'albero se il nodo v è nullo (di default).
*/
template<class Chiave>
nodo<Chiave> *Albero<Chiave>::padre(nodo<Chiave> *v) const {
    if (v) return v->padre;
    else return radice;
}



/*
Restituisce, uno dopo l'altro, i figli del nodo v.
*/
template<class Chiave>
nodo<Chiave> *Albero<Chiave>::figli(nodo<Chiave> *v) const {
    static bool prima_chiamata = true;
    static lista_figli_iterator it;

    if (prima_chiamata) {
        it = (v->lista_figli).begin();
        if (it == (v->lista_figli).end()) return 0;
        prima_chiamata = false;
        return *it++;
    }
    else if (it == (v->lista_figli).end()) {
        prima_chiamata = true;
        return 0;
    }
    else return *it++;
}



/*
Inserisce un nuovo nodo v come figlio di u nell'albero e lo restituisce.
Se v e' il primo nodo ad essere inserito nell'albero,
esso diventa la radice (e u viene ignorato).
*/
template<class Chiave>
nodo<Chiave> *Albero<Chiave>::aggiungiNodo(const Chiave &data, nodo<Chiave> *u) {
    nodo<Chiave> *v = new nodo<Chiave>(data, u);

    if (radice == 0) radice = v;
    else if (u != 0)(u->lista_figli).push_back(v);

    ++itemsCount;
    return v;
}



/*
Inserisce nell'albero il sottoalbero a in modo che la radice di a diventi figlia di u.
*/
template<class Chiave>
void Albero<Chiave>::aggiungiSottoalbero(Albero<Chiave> &a, nodo<Chiave> *u) {
    (a.radice)->padre = u;
    (u->lista_figli).push_back(a.radice);
    itemsCount += a.itemsCount;

    a.radice = 0;
}



/*
Stacca e restituisce l'intero sottoalbero radicato in v.
L'operazione cancella dall'albero il nodo v e tutti i suoi discendenti.
*/
template<class Chiave>
void Albero<Chiave>::rimuoviSottoalbero(nodo<Chiave> *v, Albero<Chiave> &sottoalbero) {
    if (v->padre) {
        ((v->padre)->lista_figli).remove(v);
        v->padre = 0;
    }

    sottoalbero.radice = v;
    sottoalbero.setItemsCount(sottoalbero.radice);

    itemsCount -= sottoalbero.itemsCount;
}



/*
Sposta l'intero sottoalbero radicato in v.
Rende il nodo u nuovo padre di v.
*/
template<class Chiave>
void Albero<Chiave>::spostaSottoalbero(nodo<Chiave> *v, nodo<Chiave> *u) {
    if (u->padre == v || v->padre == u) return;

    if (v == radice) radice = u;
    if (v->padre) {
        for (lista_figli_iterator it = ((v->padre)->lista_figli).begin();
                it != ((v->padre)->lista_figli).end(); ++it) {
            if (*it == v) {
                ((v->padre)->lista_figli).erase(it);
                break;
            }
        }
    }

    (u->lista_figli).push_back(v);
    v->padre = u;
}



/*
Elimina tutti i nodi, rendendo l'albero vuoto.
*/
template<class Chiave>
void Albero<Chiave>::clear() {
    distruggiSottoalbero(radice);
    radice = 0;
    itemsCount = 0;
}



/* PRIVATE */

template<class Chiave>
void Albero<Chiave>::distruggiSottoalbero(nodo<Chiave> *v) {
    if (v) {
        for (lista_figli_iterator it = (v->lista_figli).begin();
                it != (v->lista_figli).end(); ++it) {
            distruggiSottoalbero(*it);
        }
        delete v;
    }
}



template<class Chiave>
void Albero<Chiave>::setItemsCount(nodo<Chiave> *v) {
    if (v) {
        for (lista_figli_iterator it = (v->lista_figli).begin(); it != (v->lista_figli).end(); ++it) {
            setItemsCount(*it);
        }
        ++itemsCount;
    }
}

}  // namespace asd


#endif /* _ALBERO_H */



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
