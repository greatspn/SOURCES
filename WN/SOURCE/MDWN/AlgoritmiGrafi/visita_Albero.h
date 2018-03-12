/******************************************************************************\
*
* File:          visita_Albero.h
* Creation date: June 17, 2005 00:05
* Author:        Francesco Iovine
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _VISITA_ALBERO_H
#define _VISITA_ALBERO_H

#include "Albero.h"
#include "Pila.h"
#include "Coda.h"


namespace asd {

/*
Algoritmi di visita di un albero
rappresentato con una struttura collegata basata su liste di figli.
*/
template<class Chiave>
void visitaDFS(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *));

template<class Chiave>
void visitaDFSRicorsiva_preordine(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *));

template<class Chiave>
void visitaDFSRicorsiva_postordine(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *));

template<class Chiave>
void visitaBFS(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *));



/*
Figura 3.13
Visita in profondit�iterativa (preordine) di un albero
rappresentato con una struttura collegata basata su liste di figli.
L'insieme dei nodi aperti S e' rappresentato mediante una Pila.
*/
template<class Chiave>
void visitaDFS(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *)) {
    typedef typename asd::nodo<Chiave>::lista_figli_rev_iterator lista_figli_rev_iterator;

    nodo<Chiave> *u, *v;

    Pila<nodo<Chiave>*> S;
    S.push(r);
    while (!(S.isEmpty())) {
        u = S.pop();
        if (u != 0) {
            (*visita)(u);
            for_each_rev_figlio(v, u) S.push(v);
        }
    }
}



/*
Figura 3.14
Visita in profondit�ricorsiva (preordine) di un albero
rappresentato con una struttura collegata basata su liste di figli.
I nodi aperti vengono mantenuti sfruttando
la pila dei record di attivazione delle chiamate ricorsive.
*/
template<class Chiave>
void visitaDFSRicorsiva_preordine(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *)) {
    typedef typename asd::nodo<Chiave>::lista_figli_iterator lista_figli_iterator;

    if (r == 0) return;
    (*visita)(r);
    nodo<Chiave> *v;
    for_each_figlio(v, r) visitaDFSRicorsiva_preordine(v, visita);
}



/*
Figura 3.14
Visita in profondit�ricorsiva (postordine) di un albero
rappresentato con una struttura collegata basata su liste di figli.
I nodi aperti vengono mantenuti sfruttando
la pila dei record di attivazione delle chiamate ricorsive.
*/
template<class Chiave>
void visitaDFSRicorsiva_postordine(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *)) {
    typedef typename asd::nodo<Chiave>::lista_figli_iterator lista_figli_iterator;

    if (r == 0) return;
    nodo<Chiave> *v;
    for_each_figlio(v, r) visitaDFSRicorsiva_postordine(v, visita);
    (*visita)(r);
}



/*
Figura 3.15
Visita in ampiezza di un albero
rappresentato con una struttura collegata basata su liste di figli.
L'insieme dei nodi aperti C e' rappresentato mediante una Coda.
*/
template<class Chiave>
void visitaBFS(nodo<Chiave> *r, void (*visita)(nodo<Chiave> *)) {
    typedef typename asd::nodo<Chiave>::lista_figli_iterator lista_figli_iterator;

    nodo<Chiave> *u, *v;

    Coda<nodo<Chiave>*> C;
    C.enqueue(r);
    while (!(C.isEmpty())) {
        u = C.dequeue();
        if (u != 0) {
            (*visita)(u);
            for_each_figlio(v, u) C.enqueue(v);
        }
    }
}

}  // namespace asd


#endif /* _VISITA_ALBERO_H */



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
