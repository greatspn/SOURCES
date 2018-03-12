/******************************************************************************\
*
* File:          visita_Grafo.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Purpose:       Fig.11.7, Fig.11.9, Fig.11.13, Fig.11.14, Fig.11.15
*
* License:       See the end of this file for license information
*
* Modifications:
*   1)  Modificato per compatibilita' con la nuova struttura dati Grafo.
*       April 20, 2006. Francesco Iovine && Massimiliano Macchia
*   2)  Aggiunto l'algortmo 'fortementeConnesso'
*       October 2, 2006. Massimiliano Macchia
*
\******************************************************************************/
#ifndef _VISITA_GRAFO_H
#define _VISITA_GRAFO_H

#include "Grafo.h"
#include "GrafoOrientato.h"
#include "Albero.h"
#include "Coda.h"
#include "Pila.h"

#include <ext/hash_set>



namespace asd {

/*
Algoritmi di visita di un grafo.
*/
void visitaBFS(const Grafo &G, vertice *s, Albero< vertice * > &T);
void visitaDFS(const Grafo &G, vertice *s, Albero< vertice * > &T);
bool connessoGrafo(const Grafo &G);
bool fortementeConnesso(const GrafoOrientato &G);


/*
Procedure interne.
*/
void visitaDFSRicorsiva(const Grafo &G, vertice *v, Albero< vertice * > &T,
                        vertice_array<bool> &marcato,
                        vertice_array< nodo<vertice *>* > &nodi);

void visitaFortementeConnesso(vertice *v, unsigned &contatore,
                              unsigned &numComponenti, Pila<vertice *> &P,
                              __gnu_cxx::hash_set<vertice *, hash_v> &Set,
                              __gnu_cxx::hash_map<vertice *, unsigned, hash_v> &numeroDFS,
                              __gnu_cxx::hash_map<vertice *, unsigned, hash_v> &fuga,
                              const GrafoOrientato &G);



/* ALGORITMI DI VISITA DI UN GRAFO */

/*
Figura 11.7
Algoritmo di visita in ampiezza in un grafo G a partire dal vertice s.
*/
void visitaBFS(const Grafo &G, vertice *s, Albero< vertice * > &T) {
    vertice *u, *v;
    vertice_array<bool> marcato;
    vertice_array< nodo<vertice *>* > nodi;

    /* rendi tutti i vertici non marcati  */
    for_each_vertice(v, G) marcato[v] = false;

    /* T <- albero formato da un solo nodo */
    T.clear();
    nodi[s] = T.aggiungiNodo(s);

    Coda< vertice * > F;
    marcato[s] = true;  // marca il vertice s
    F.enqueue(s);
    while (!(F.isEmpty())) {
        u = F.dequeue();
        for_each_vertice_adiacente(v, u, G) {
            if (marcato[v] == false) {  // if( v non e' ancora marcato )
                F.enqueue(v);
                marcato[v] = true;  // marca il vertice v
                nodi[v] = T.aggiungiNodo(v, nodi[u]); //rendi u padre di v in T
            }
        }
    }

    // return T
}



/*
Figura 11.9
Algoritmo ricorsivo per la visita in profondita'
di un grafo G a partire da un vertice s.
*/
void visitaDFS(const Grafo &G, vertice *s, Albero< vertice * > &T) {
    vertice *v;
    vertice_array<bool> marcato;
    vertice_array< nodo<vertice *>* > nodi;

    /* rendi tutti i vertici non marcati  */
    for_each_vertice(v, G) marcato[v] = false;

    /* T <- albero formato da un solo nodo */
    T.clear();
    nodi[s] = T.aggiungiNodo(s);

    visitaDFSRicorsiva(G, s, T, marcato, nodi);

    // return T
}



/*
Figura 11.13
Algoritmo che verifica se un grafo G e' connesso.
*/
bool connessoGrafo(const Grafo &G) {
    /* scegli arbitrariamente un vertice s in G */
    vertice *s = (G.lista_vertici).front();

    /* T <-visitaGenerica(s) */
    Albero<vertice *> T;
    visitaBFS(G, s, T);

    if (T.numNodi() == G.numVertici())  // if( T ha n nodi ) then
        return true;                    //     return connesso
    else return false;                  // else return non connesso
}



/*
Figura 11.15
Algoritmo che verifica se un grafo orientato G e' fortemente connesso.
*/
bool fortementeConnesso(GrafoOrientato &G) {
    unsigned contatore = 0;
    unsigned numComponenti = 0;

    //strutture di supporto alla visita
    Pila<vertice *> P;
    __gnu_cxx::hash_map<vertice *, unsigned, hash_v> numeroDFS, fuga;
    __gnu_cxx::hash_set<vertice *, hash_v> Set;

    /*vertice *v;
    for_each_vertice(v,G)
    {
    	if(set.find(v) == set.end())
    	{
    		set.insert(v);
    		P.push(v);
    		visitaFortementeConnesso(v,contatore,numComponenti,P,
                                     set,numeroDFS,fuga,G);
    	}
    }*/

    //crea un nuovo vertice x ...
    vertice *x = G.aggiungiVertice();
    //... con archi (x,v) per tutti i vertici v
    vertice *v;
    for_each_vertice(v, G) {
        G.aggiungiArco(x, v);
    }

    //al posto di un albero T utilizziamo un hash_set Set
    Set.insert(x);
    P.push(x);

    visitaFortementeConnesso(x, contatore, numComponenti, P,
                             Set, numeroDFS, fuga, G);

    G.rimuoviVertice(x);

    if (numComponenti == G.numVertici()) return true;
    else return false;
}



/* PROCEDURE INTERNE */

/*
Figura 11.9
Algoritmo ricorsivo per la visita in profondita'
di un grafo G a partire da un vertice v.
*/
void visitaDFSRicorsiva(const Grafo &G, vertice *v, Albero< vertice * > &T,
                        vertice_array<bool> &marcato,
                        vertice_array< nodo<vertice *>* > &nodi) {
    vertice *w;
    marcato[v] = true;  // marca il vertice s
    for_each_vertice_adiacente(w, v, G) { // for_each ( arco (v,w) ) do
        if (marcato[w] == false) {  // if( v non e' marcato )
            nodi[w] = T.aggiungiNodo(w, nodi[v]); //aggiungi l'arco(v,w) all'albero T
            visitaDFSRicorsiva(G, w, T, marcato, nodi);
        }
    }
}



/*
Figura 11.14
Algoritmo ricorsivo per la visita del grafo
e la individuazione delle componenti
fortemente connesse del grafo G.
*/
void visitaFortementeConnesso(vertice *v, unsigned &contatore,
                              unsigned &numComponenti, Pila<vertice *> &P,
                              __gnu_cxx::hash_set<vertice *, hash_v> &Set,
                              __gnu_cxx::hash_map<vertice *, unsigned, hash_v> &numeroDFS,
                              __gnu_cxx::hash_map<vertice *, unsigned, hash_v> &fuga,
                              const GrafoOrientato &G) {
    numeroDFS[v] = contatore;
    fuga[v] = contatore;
    ++contatore;

    vertice *w;
    for_each_vertice_adiacente(w, v, G) {
        if (Set.find(w) == Set.end()) {
            Set.insert(w);
            P.push(w);
            visitaFortementeConnesso(w, contatore, numComponenti, P, Set, numeroDFS, fuga, G);
            if (fuga[w] < fuga[v]) fuga[v] = fuga[w];
        }
        else if (numeroDFS[w] < fuga[v]) fuga[v] = numeroDFS[w];
    }

    if (fuga[v] == numeroDFS[v]) {
        ++numComponenti;

        vertice *u;
        do {
            u = P.pop();
            numeroDFS[u] = G.numVertici() + 1;
        }
        while (u != v);
    }
}

}  // namespace asd


#endif /* _VISITA_GRAFO_H */



/******************************************************************************\
 * Copyright (C) 2005-2006 ASD Team

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

