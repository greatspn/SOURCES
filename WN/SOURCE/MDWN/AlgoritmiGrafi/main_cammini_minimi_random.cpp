/******************************************************************************\
*
* File:          main_cammini_minimi_random.cpp
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Last changed:  April 20, 2006
* Modifications: Modificato utilizzando la nuova struttura dati polimorfica Grafo.
* Changed by:    Francesco Iovine && Massimiliano Macchia
*
* Purpose:       Programma di prova che mostra il funzionamento degli algoritmi
*                per il calcolo dei cammini minimi di un grafo orientato, 
*                utilizzando il generatore 'random' del grafo.
*
* License:       See the end of this file for license information
*
\******************************************************************************/

#include <algoritmi/cammini_minimi.h>

// Includere soltanto il tipo di grafo orientato desiderato.

//#include <grafo/GrafoOrientato_ListaArchi.h>
#include <grafo/GrafoOrientato_ListeAdiacenza.h>
//#include <grafo/GrafoOrientato_ListeIncidenza.h>
//#include <grafo/GrafoOrientato_MatriceAdiacenza.h>
//#include <grafo/GrafoOrientato_MatriceIncidenza.h>

#include <albero/Albero.h>
#include <algoritmi/visita_Grafo.h>
#include <algoritmi/visita_Albero.h>

#include <iostream>
#include <map>
#include <cstdlib>
#include <ctime>

using std::cout;
using std::cin;
using std::endl;
using std::map;
using std::pair;

using namespace asd;


vertice_array<int> info;

template<class Chiave>
void stampa_nodo(nodo<Chiave>* v)
{
    cout << info[v->getChiave()];
    if (v->padre) cout << "(->" << info[(v->padre)->getChiave()] << ")";
    cout << "  ";
}


int main()
{
// Qui e' possibile specificare il tipo di grafo che si vuole utilizzare.

//GrafoOrientato_ListaArchi grafo;
GrafoOrientato_ListeAdiacenza grafo;
//GrafoOrientato_ListeIncidenza grafo;
//GrafoOrientato_MatriceAdiacenza grafo;
//GrafoOrientato_MatriceIncidenza grafo;

/* I valori da associare al grafo orientato da creare casualmente */
unsigned n                       = 100;       // numero di vertici
unsigned m                       = 2 * n;     // numero di archi
unsigned costo_max               = 1000;      // costo massimo di un arco
bool     loopfree                = false;     // presenza(false) o no(true) di self loops

/* Generazione random del grafo orientato */
srand(time(0));
grafo.genera_random(n, m, costo_max, loopfree);

unsigned idx = 0;

map< int, vertice* > vMap;

vertice* v;
for_each_vertice(v,grafo) {
     info[v] = idx;
     vMap[idx] = v;
     ++idx;
}



cout << "\nGRAFO ORIENTATO";
cout << "\n\nLista di vertici: "; for_each_vertice(v,grafo) cout << info[v] << " ";
cout << "\n\nLista di archi: ";  arco* e;
for_each_arco(e,grafo) {
    cout << "\n\t(" << info[e->sorg] << ", " << info[e->dest] << ", " << e->costo << ") ";
}
cout << "\n\nIl numero di vertici e' " << grafo.numVertici();
cout << "\nIl numero di archi e' " << grafo.numArchi();



cout << "\n\n\n-------------------------------------------------------------------\n\n\n";



unsigned i;  // indice del vertice che sara' utilizzato come sorgente negli algoritmi.
cout << "Si prega di inserire il vertice da utilizzare come sorgente "
     << "\nper gli algoritmi per il calcolo dei cammini minimi di un grafo orientato.  ";
cin >> i;



cout << "\n\n\n-------------------------------------------------------------------\n\n\n";



cout << "VISITA BFS DEL GRAFO a partire dal nodo " << i;
Albero< vertice* > alberoBFS;
visitaBFS(grafo, vMap[i], alberoBFS);
cout << "\n\nNodi alberoBFS: " << alberoBFS.numNodi();
cout << "\n\nVisita in ampiezza dell'albero BFS risultante: "; visitaBFS(alberoBFS.padre(), stampa_nodo);
cout << "\n\nVisita in profondita' dell'albero BFS risultante: "; visitaDFS(alberoBFS.padre(), stampa_nodo);

cout << "\n\n\n\nVISITA DFS DEL GRAFO a partire dal nodo 'A'";
Albero< vertice* > alberoDFS;
visitaDFS(grafo, vMap[i], alberoDFS);
cout << "\n\nNodi alberoDFS: " << alberoDFS.numNodi();
cout << "\n\nVisita in ampiezza dell'albero DFS risultante: "; visitaBFS(alberoDFS.padre(), stampa_nodo);
cout << "\n\nVisita in profondita' dell'albero DFS risultante: "; visitaDFS(alberoDFS.padre(), stampa_nodo);



cout << "\n\n\n-------------------------------------------------------------------\n\n\n";



cout << "ALGORITMO DI BELLMAN-FORD\n";
vertice_array<int> distanzeBellmanFord;

distanzeBellmanFord = BellmanFord(grafo, vMap[i]);

cout << "\ndistanze dal vertice " << i << ": ";
for (unsigned j = 0; j < n; ++j) {
    if (distanzeBellmanFord[ vMap[j] ] != INFINITO)
        cout << "\n\t(" << i << "->" << j << ")" << "(" << distanzeBellmanFord[ vMap[j] ] << ")";
    else cout << "\n\t(" << i << "->" << j << ")" << "(INFINITO)";
}



cout << "\n\n\n-------------------------------------------------------------------\n\n\n";



cout << "ALGORITMO DI DIJKSTRA\n";
vertice_array<int> distanzeDijkstra;
Albero< vertice* > albero_dijkstra;

distanzeDijkstra = Dijkstra(grafo, vMap[i], albero_dijkstra);

cout << "\ndistanze dal vertice " << i << ": ";
for (unsigned j = 0; j < n; ++j) {
    if (distanzeDijkstra[ vMap[j] ] != INFINITO)
        cout << "\n\t(" << i << "->" << j << ")" << "(" << distanzeDijkstra[ vMap[j] ] << ")";
    else cout << "\n\t(" << i << "->" << j << ")" << "(INFINITO)";
}



cout << "\n\n\n-------------------------------------------------------------------\n\n\n";



cout << "ALGORITMO DI FLOYD-WARSHALL";
map<pair<vertice*, vertice*>, int> distanzeFloydWarshall;
pair<vertice*, vertice*> p;

distanzeFloydWarshall = FloydWarshall(grafo);

cout << "\n\ndistanze dal vertice " << i << ": ";
for (unsigned j = 0; j < n; ++j) {
    p.first = vMap[i]; p.second = vMap[j];
    if (distanzeFloydWarshall[ p ] != INFINITO)
        cout << "\n\t(" << i << "->" << j << ")" << "(" << distanzeFloydWarshall[p] << ")";
    else cout << "\n\t(" << i << "->" << j << ")" << "(INFINITO)";
}



cout << "\n\n\n-------------------------------------------------------------------\n\n\n";



//system("PAUSE");
return 0;
} 



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
