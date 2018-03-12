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

#include "../lib_header/Grafo.h"
//#include "Albero.h"
//#include "HeapFibonacci.h"
//#include "Coda.h"
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <list>
#include <utility>

#define for_each_vertice_in_ordinamento_topologico(u,ord)                                  \
        u = *(ord.begin());                                                                \
        for (std::list<vertice*>::iterator it = ord.begin(); it != ord.end(); ++it, u = *it)

using std::cerr;
namespace asd {

/*
Algoritmi per trovare l'albero dei cammini minimi di un grafo orientato.
*/
vertice_array<struct rew_act> BellmanFordMIN(const Grafo &G, vertice *s);
vertice_array<struct rew_act> BellmanFordMAX(const Grafo &G, vertice *s);
vertice_array<std::string> info;
arco_array<std::string> info1;
/*
Figura 13.4
Algoritmo di Bellman e Ford per il calcolo delle distanze a partire da una sorgente s
in un grafo orientato G con n vertici ed m archi.
*/
struct rew_act {
    double reward;
    std::string action;
};

vertice_array<struct rew_act> BellmanFordMin(const Grafo &G, vertice *s) {
    unsigned n = G.numVertici();
    vertice *v, *u;
    arco *e;
    double w;

    /* inizializza D tale che Dsv = +infinito per v != s, e Dss = 0 */
    vertice_array<struct rew_act> Ds;
    for_each_vertice(v, G) Ds[v].reward = INFINITO;
    Ds[s].reward = 0;
    Ds[s].action = "";
    for (unsigned i = 1; i <= n; ++i) {
        /* for each ( (u,v) in E ) do */
        for_each_arco(e, G) {
            u = e->sorg;  v = e->dest;  w = e->costo;  // e = (u,v) con costo w
            //cout<<"Nome:"<<info[];
            if (Ds[u].reward != INFINITO) {
                if (Ds[u].reward + w < Ds[v].reward) Ds[v].reward = Ds[u].reward + w;  // rilassamento
                if (Ds[u].action != "")
                    Ds[v].action = Ds[u].action + "," + info1[e];
                else
                    Ds[v].action = " " + info1[e];
            }
        }
    }
    for_each_arco(e, G) {
        u = e->sorg;
        v = e->dest;
        if (Ds[v].reward > Ds[u].reward + e->costo) {
            cerr << "Graph contains a negative-weight cycle\n\n";
            exit(1);
        }
    }
    return Ds;
}

vertice_array <struct rew_act>BellmanFordMax(const Grafo &G, vertice *s) {
    unsigned n = G.numVertici();
    vertice *v, *u;
    arco *e;
    double w;
    /* inizializza D tale che Dsv = +infinito per v != s, e Dss = 0 */
    vertice_array<struct rew_act> Ds;
    for_each_vertice(v, G) Ds[v].reward = (INFINITO) * -1;
    Ds[s].reward = 0;
    Ds[s].action = "";
    for (unsigned i = 1; i <= n; ++i) {
        for_each_arco(e, G) {
            u = e->sorg;  v = e->dest;  w = e->costo;  // e = (u,v) con costo w
            if (Ds[u].reward != (INFINITO) * -1) {
                if (Ds[u].reward + w > Ds[v].reward) Ds[v].reward = Ds[u].reward + w;  // rilassamento
                if (Ds[u].action != "")
                    Ds[v].action = Ds[u].action + "," + info1[e];
                else
                    Ds[v].action = " " + info1[e];
            }
        }
    }
    for_each_arco(e, G) {
        u = e->sorg;
        v = e->dest;
        if (Ds[v].reward < Ds[u].reward + e->costo) {
            cerr << "Graph contains a positive-weight cycle\n\n";
            exit(1);
        }
    }
    return Ds;
}
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
