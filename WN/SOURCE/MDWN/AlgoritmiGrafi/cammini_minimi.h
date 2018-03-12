/******************************************************************************\
*
* File:          cammini_minimi.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Purpose:       Fig.13.4, Fig.13.6, Fig.13.7, Fig.13.11, Fig.13.12
*
* License:       See the end of this file for license information
*
* Modifications:
*   1)  Modificato per compatibilita' con la nuova struttura dati Grafo.
*       April 20, 2006. Francesco Iovine && Massimiliano Macchia
*
\******************************************************************************/
#ifndef _CAMMINIMINIMI_H
#define _CAMMINIMINIMI_H

#include "Grafo.h"
#include "Albero.h"
//#include "HeapFibonacci.h"
//#include "Coda.h"

#include <list>
#include <map>


#define for_each_vertice_in_ordinamento_topologico(u,ord)                                  \
        u = *(ord.begin());                                                                \
        for (std::list<vertice*>::iterator it = ord.begin(); it != ord.end(); ++it, u = *it)


namespace asd {

/*
Algoritmi per trovare l'albero dei cammini minimi di un grafo orientato.
*/
vertice_array<int> BellmanFord(const Grafo &G, vertice *s);
std::list<vertice *> ordinamentoTopologico(const Grafo &G);
vertice_array<int> distanzeAciclico(const Grafo &G, vertice *s);
vertice_array<int> Dijkstra(const Grafo &G, vertice *s, Albero< vertice * > &T);
std::map< std::pair<vertice *, vertice *>, int > FloydWarshall(const Grafo &G);



/*
Figura 13.4
Algoritmo di Bellman e Ford per il calcolo delle distanze a partire da una sorgente s
in un grafo orientato G con n vertici ed m archi.
*/
vertice_array<int> BellmanFordMin(const Grafo &G, vertice *s) {
    unsigned n = G.numVertici();
    vertice *v, *u;
    arco *e;
    int w;

    /* inizializza D tale che Dsv = +infinito per v != s, e Dss = 0 */
    vertice_array<int> Ds;
    for_each_vertice(v, G) Ds[v] = INFINITO;
    Ds[s] = 0;
    for (unsigned i = 1; i <= n; ++i) {
        /* for each ( (u,v) in E ) do */
        for_each_arco(e, G) {
            u = e->sorg;  v = e->dest;  w = e->costo;  // e = (u,v) con costo w
            if (Ds[u] != INFINITO) {
                if (Ds[u] + w < Ds[v]) Ds[v] = Ds[u] + w;  // rilassamento
            }
        }
    }
    for_each_arco(e, G) {
        u = e->sorg;
        v = e->dest;
        if (Ds[v] > Ds[u] + e->costo) {
            //std::cerr<<"Graph contains a negative-weight cycle\n\n";
            exit(1);
        }
    }
    return Ds;
}

vertice_array<int> BellmanFordMax(const Grafo &G, vertice *s) {
    unsigned n = G.numVertici();
    vertice *v, *u;
    arco *e;
    int w;
    /* inizializza D tale che Dsv = +infinito per v != s, e Dss = 0 */
    vertice_array<int> Ds;
    for_each_vertice(v, G) Ds[v] = (INFINITO) * -1;
    Ds[s] = 0;
    for (unsigned i = 1; i <= n; ++i) {
        for_each_arco(e, G) {
            u = e->sorg;  v = e->dest;  w = e->costo;  // e = (u,v) con costo w
            if (Ds[u] != (INFINITO) * -1) {
                if (Ds[u] + w > Ds[v]) Ds[v] = Ds[u] + w;  // rilassamento
            }
        }
    }
    for_each_arco(e, G) {
        u = e->sorg;
        v = e->dest;
        if (Ds[v] < Ds[u] + e->costo) {
            //std::cerr<<"Graph contains a positive-weight cycle\n\n";
            exit(1);
        }
    }
    return Ds;
}


vertice_array<char> info;


}  // namespace asd


#endif /* _CAMMINIMINIMI_H */



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
