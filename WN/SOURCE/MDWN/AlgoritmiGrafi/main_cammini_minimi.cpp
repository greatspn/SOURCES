/******************************************************************************\
*
* File:          main_cammini_minimi.cpp
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Last changed:  April 20, 2006
* Modifications: Modificato utilizzando la nuova struttura dati polimorfica Grafo.
* Changed by:    Francesco Iovine && Massimiliano Macchia
*
* Purpose:       Programma di prova che mostra il funzionamento degli algoritmi
*                per il calcolo dei cammini minimi di un grafo orientato, 
*                utilizzando, come esempio, quello di figura 13.10.
*
* License:       See the end of this file for license information
*
\******************************************************************************/

#include "cammini_minimi.h"

// Includere soltanto il tipo di grafo orientato desiderato.

//#include <grafo/GrafoOrientato_ListaArchi.h>
#include "GrafoOrientato_ListeAdiacenza.h"
//#include <grafo/GrafoOrientato_ListeIncidenza.h>
//#include <grafo/GrafoOrientato_MatriceAdiacenza.h>
//#include <grafo/GrafoOrientato_MatriceIncidenza.h>

//#include "Albero.h"
//#include "visita_Grafo.h"
//#include "visita_Albero.h"

#include <iostream>
#include <map>

using std::cout;
using std::endl;
using std::map;
using std::pair;

using namespace asd;


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

vertice* v;
arco* e;

cout << "\n\nBULDING TEST GRAPH\n\n";

map< char, vertice* > vMap;
map< char*, arco* > eMap;

vMap['A'] = grafo.aggiungiVertice();
info[ vMap['A'] ] = 'A';
vMap['B'] = grafo.aggiungiVertice();
info[ vMap['B'] ] = 'B';
vMap['C'] = grafo.aggiungiVertice();
info[ vMap['C'] ] = 'C';
vMap['D'] = grafo.aggiungiVertice();
info[ vMap['D'] ] = 'D';
vMap['E'] = grafo.aggiungiVertice();
info[ vMap['E'] ] = 'E';
vMap['F'] = grafo.aggiungiVertice();
info[ vMap['F'] ] = 'F';
vMap['G'] = grafo.aggiungiVertice();
info[ vMap['G'] ] = 'G';

eMap["A->B"] = grafo.aggiungiArco(vMap['A'], vMap['B'], 7);
eMap["A->C"] = grafo.aggiungiArco(vMap['A'], vMap['C'], 14);
eMap["B->C"] = grafo.aggiungiArco(vMap['B'], vMap['C'], 6);
eMap["A->D"] = grafo.aggiungiArco(vMap['A'], vMap['D'], 30);
eMap["D->C"] = grafo.aggiungiArco(vMap['D'], vMap['C'], 10);
eMap["C->E"] = grafo.aggiungiArco(vMap['C'], vMap['E'], 1);
eMap["E->F"] = grafo.aggiungiArco(vMap['E'], vMap['F'], -6);
//eMap["F->E"] = grafo.aggiungiArco(vMap['F'], vMap['E'], -36);
eMap["E->G"] = grafo.aggiungiArco(vMap['E'], vMap['G'], 9);
eMap["F->G"] = grafo.aggiungiArco(vMap['F'], vMap['G'], -34);
eMap["G->F"] = grafo.aggiungiArco(vMap['G'], vMap['F'], 10);



cout << "\n\nVertexes list:\n "; for_each_vertice(v,grafo) cout << info[v] << " ";
cout << "\n\nArcs list:\n ";
for_each_arco(e,grafo) cout << "(" << info[e->sorg] << ", " << info[e->dest] << ", " << e->costo << ") ";

cout << "\n\nNumber of vertexes:\n " << grafo.numVertici();
cout << "\n\nNumber of arcs: \n" << grafo.numArchi();

/*
cout << "\n\nVISITA BFS DEL GRAFO a partire dal nodo 'A'";
Albero< vertice* > alberoBFS;
visitaBFS(grafo, vMap['A'], alberoBFS);
cout << "\nVisita in ampiezza dell'albero BFS risultante: "; visitaBFS(alberoBFS.padre(), stampa_nodo);
cout << "\nVisita in profondita' dell'albero BFS risultante: "; visitaDFS(alberoBFS.padre(), stampa_nodo);

cout << "\n\nVISITA DFS DEL GRAFO a partire dal nodo 'A'";
Albero< vertice* > alberoDFS;
visitaDFS(grafo, vMap['A'], alberoDFS);
cout << "\nVisita in ampiezza dell'albero DFS risultante: "; visitaBFS(alberoDFS.padre(), stampa_nodo);
cout << "\nVisita in profondita' dell'albero DFS risultante: "; visitaDFS(alberoDFS.padre(), stampa_nodo);
*/


cout << "\n\n\nBELLMAN-FORD ALGORITHM\n\n";
vertice_array<int> distanzeBellmanFord;
distanzeBellmanFord = BellmanFordMin(grafo, vMap['A']);
cout << "\nMIN path from 'A': ";
cout << "\n\t(A->A)(" << distanzeBellmanFord[ vMap['A'] ] << ")  ";
cout << "\n\t(A->B)(" << distanzeBellmanFord[ vMap['B'] ] << ")  ";
cout << "\n\t(A->C)(" << distanzeBellmanFord[ vMap['C'] ] << ")  ";
cout << "\n\t(A->D)(" << distanzeBellmanFord[ vMap['D'] ] << ")  ";
cout << "\n\t(A->E)(" << distanzeBellmanFord[ vMap['E'] ] << ")  ";
cout << "\n\t(A->F)(" << distanzeBellmanFord[ vMap['F'] ] << ")  ";
cout << "\n\t(A->G)(" << distanzeBellmanFord[ vMap['G'] ] << ")  ";

cout<<"\n\n************************************************************\n\n";


distanzeBellmanFord = BellmanFordMax(grafo, vMap['A']);
cout << "\nMAX path from 'A': ";
cout << "\n\t(A->A)(" << distanzeBellmanFord[ vMap['A'] ] << ")  ";
cout << "\n\t(A->B)(" << distanzeBellmanFord[ vMap['B'] ] << ")  ";
cout << "\n\t(A->C)(" << distanzeBellmanFord[ vMap['C'] ] << ")  ";
cout << "\n\t(A->D)(" << distanzeBellmanFord[ vMap['D'] ] << ")  ";
cout << "\n\t(A->E)(" << distanzeBellmanFord[ vMap['E'] ] << ")  ";
cout << "\n\t(A->F)(" << distanzeBellmanFord[ vMap['F'] ] << ")  ";
cout << "\n\t(A->G)(" << distanzeBellmanFord[ vMap['G'] ] << ")  ";

cout<<"\n\n************************************************************\n\n";
/*
cout << "\n\n\nALGORITMO DISTANZE ACICLICO";
vertice_array<int> distanzeAciclicoTopologico;
distanzeAciclicoTopologico = distanzeAciclico(grafo, vMap['A']);
cout << "\ndistanze dal vertice 'A': ";
cout << "\n\t(A->A)(" << distanzeAciclicoTopologico[ vMap['A'] ] << ")  ";
cout << "\n\t(A->B)(" << distanzeAciclicoTopologico[ vMap['B'] ] << ")  ";
cout << "\n\t(A->C)(" << distanzeAciclicoTopologico[ vMap['C'] ] << ")  ";
cout << "\n\t(A->D)(" << distanzeAciclicoTopologico[ vMap['D'] ] << ")  ";
cout << "\n\t(A->E)(" << distanzeAciclicoTopologico[ vMap['E'] ] << ")  ";
cout << "\n\t(A->F)(" << distanzeAciclicoTopologico[ vMap['F'] ] << ")  ";
cout << "\n\t(A->G)(" << distanzeAciclicoTopologico[ vMap['G'] ] << ")  ";



cout << "\n\n\nALGORITMO DI DIJKSTRA";
cout << "\nFigura 13.10  Esecuzione dell'algoritmo di Dijkstra con sorgente s = 'A'";
vertice_array<int> distanzeDijkstra;
Albero< vertice* > albero_dijkstra;
distanzeDijkstra = Dijkstra(grafo, vMap['A'], albero_dijkstra);
cout << "\ndistanze dal vertice 'A': ";
cout << "\n\t(A->A)(" << distanzeDijkstra[ vMap['A'] ] << ")  ";
cout << "\n\t(A->B)(" << distanzeDijkstra[ vMap['B'] ] << ")  ";
cout << "\n\t(A->C)(" << distanzeDijkstra[ vMap['C'] ] << ")  ";
cout << "\n\t(A->D)(" << distanzeDijkstra[ vMap['D'] ] << ")  ";
cout << "\n\t(A->E)(" << distanzeDijkstra[ vMap['E'] ] << ")  ";
cout << "\n\t(A->F)(" << distanzeDijkstra[ vMap['F'] ] << ")  ";
cout << "\n\t(A->G)(" << distanzeDijkstra[ vMap['G'] ] << ")  ";
cout << "\nvisita BFS dell'albero dei cammini minimi: "; visitaBFS(albero_dijkstra.padre(), stampa_nodo);
cout << "\nvisita DFS dell'albero dei cammini minimi: "; visitaDFS(albero_dijkstra.padre(), stampa_nodo);



cout << "\n\n\nALGORITMO DI FLOYD-WARSHALL";
map<pair<vertice*, vertice*>, int> distanzeFloydWarshall;
pair<vertice*, vertice*> p;
distanzeFloydWarshall = FloydWarshall(grafo);

cout << "\ndistanze dal vertice A: ";
p.first = vMap['A'];  p.second = vMap['A'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(A->A)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['A'];  p.second = vMap['B'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(A->B)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['A'];  p.second = vMap['C'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(A->C)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['A'];  p.second = vMap['D'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(A->D)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['A'];  p.second = vMap['E'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(A->E)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['A'];  p.second = vMap['F'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(A->F)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['A'];  p.second = vMap['G'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(A->G)(" << distanzeFloydWarshall[ p ] << ")  ";

cout << "\ndistanze dal vertice B: ";
p.first = vMap['B'];  p.second = vMap['A'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(B->A)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['B'];  p.second = vMap['B'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(B->B)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['B'];  p.second = vMap['C'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(B->C)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['B'];  p.second = vMap['D'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(B->D)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['B'];  p.second = vMap['E'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(B->E)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['B'];  p.second = vMap['F'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(B->F)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['B'];  p.second = vMap['G'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(B->G)(" << distanzeFloydWarshall[ p ] << ")  ";

cout << "\ndistanze dal vertice C: ";
p.first = vMap['C'];  p.second = vMap['A'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(C->A)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['C'];  p.second = vMap['B'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(C->B)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['C'];  p.second = vMap['C'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(C->C)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['C'];  p.second = vMap['D'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(C->D)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['C'];  p.second = vMap['E'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(C->E)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['C'];  p.second = vMap['F'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(C->F)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['C'];  p.second = vMap['G'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(C->G)(" << distanzeFloydWarshall[ p ] << ")  ";

cout << "\ndistanze dal vertice D: ";
p.first = vMap['D'];  p.second = vMap['A'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(D->A)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['D'];  p.second = vMap['B'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(D->B)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['D'];  p.second = vMap['C'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(D->C)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['D'];  p.second = vMap['D'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(D->D)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['D'];  p.second = vMap['E'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(D->E)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['D'];  p.second = vMap['F'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(D->F)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['D'];  p.second = vMap['G'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(D->G)(" << distanzeFloydWarshall[ p ] << ")  ";

cout << "\ndistanze dal vertice E: ";
p.first = vMap['E'];  p.second = vMap['A'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(E->A)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['E'];  p.second = vMap['B'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(E->B)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['E'];  p.second = vMap['C'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(E->C)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['E'];  p.second = vMap['D'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(E->D)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['E'];  p.second = vMap['E'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(E->E)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['E'];  p.second = vMap['F'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(E->F)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['E'];  p.second = vMap['G'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(E->G)(" << distanzeFloydWarshall[ p ] << ")  ";

cout << "\ndistanze dal vertice F: ";
p.first = vMap['F'];  p.second = vMap['A'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(F->A)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['F'];  p.second = vMap['B'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(F->B)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['F'];  p.second = vMap['C'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(F->C)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['F'];  p.second = vMap['D'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(F->D)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['F'];  p.second = vMap['E'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(F->E)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['F'];  p.second = vMap['F'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(F->F)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['F'];  p.second = vMap['G'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(F->G)(" << distanzeFloydWarshall[ p ] << ")  ";

cout << "\ndistanze dal vertice G: ";
p.first = vMap['G'];  p.second = vMap['A'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(G->A)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['G'];  p.second = vMap['B'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(G->B)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['G'];  p.second = vMap['C'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(G->C)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['G'];  p.second = vMap['D'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(G->D)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['G'];  p.second = vMap['E'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(G->E)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['G'];  p.second = vMap['F'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(G->F)(" << distanzeFloydWarshall[ p ] << ")  ";
p.first = vMap['G'];  p.second = vMap['G'];
if (distanzeFloydWarshall[ p ] != INFINITO) cout << "\n\t(G->G)(" << distanzeFloydWarshall[ p ] << ")  ";


*/
cout << endl;
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
