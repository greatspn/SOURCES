/******************************************************************************\
*
* File:          Grafo_ListeAdiacenza.h
* Creation date: April 20, 2006
* Author:        Francesco Iovine, Massimiliano Macchia
*
* Last changed:  April 25, 2006
* Modifications: Modificato utilizzando HASH_MAP invece che MAP.
* Changed by:    Francesco Iovine && Massimiliano Macchia
*
* Purpose:       Fig. 11.2
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _GRAFO_LISTEADIACENZA_H
#define _GRAFO_LISTEADIACENZA_H

#include "../lib_header/Grafo.h"


namespace asd {


class Grafo_ListeAdiacenza;

class liste_adiacenza {
public:
    __gnu_cxx::hash_map< vertice *, std::list< adiacente >, hash_v > m;

public:
    liste_adiacenza() {};
    ~liste_adiacenza() {};

    void inserisciVertice(vertice *v) { m[v] = std::list<adiacente>(); }
    void eliminaVertice(vertice *v) {
        m.erase(m.find(v));

        __gnu_cxx::hash_map< vertice *, std::list< adiacente >, hash_v >::iterator v_it = m.begin();
        __gnu_cxx::hash_map< vertice *, std::list< adiacente >, hash_v >::iterator v_end = m.end();
        std::list< adiacente >::iterator adj_it, adj_end, temp_it;
        for (; v_it != v_end; ++v_it) {
            adj_it = v_it->second.begin();
            adj_end = v_it->second.end();
            for (; adj_it != adj_end; ++adj_it)
                if (adj_it->vertex == v) {
                    temp_it = adj_it;
                    ++temp_it;
                    v_it->second.erase(adj_it);
                    adj_it = temp_it;
                }
        }
    }
    void inserisciArco(vertice *x, vertice *y, double c) { m[x].push_back(adiacente(y, c)); }
    void eliminaArco(vertice *x, vertice *y, double c) {
        std::list< adiacente >::iterator it, end;
        it = m[x].begin();
        end = m[x].end();
        for (; it != end; ++it)
            if (it->vertex == y && it->costo == c)
            { m[x].erase(it); break; }
    }
    void reset() { m.clear(); }
    bool sonoAdiacenti(vertice *x, vertice *y) const {
        std::list< adiacente >::const_iterator it = m.find(x)->second.begin();
        std::list< adiacente >::const_iterator end = m.find(x)->second.end();
        for (; it != end; ++it)
            if (it->vertex == y)
                return true;
        return false;
    }
    unsigned contaOccorrenze(vertice *v) const {
        unsigned g(0);

        __gnu_cxx::hash_map< vertice *, std::list< adiacente >, hash_v >::const_iterator v_it = m.begin();
        __gnu_cxx::hash_map< vertice *, std::list< adiacente >, hash_v >::const_iterator v_end = m.end();
        std::list< adiacente >::const_iterator adj_it, adj_end;
        for (; v_it != v_end; ++v_it) {
            adj_it = v_it->second.begin();
            adj_end = v_it->second.end();
            for (; adj_it != adj_end; ++adj_it)
                if (adj_it->vertex == v)
                    ++g;
        }

        return g;
    }
    unsigned dimensioneLista(vertice *v) const {
        return m.find(v)->second.size();
    }
};



/*
Rappresentazione di un grafo con liste di adiacenza.
*/
class Grafo_ListeAdiacenza : virtual public Grafo {
    /* dati ereditati da Grafo */
//  protected:  // dati protected
//    unsigned vertexCount;
//    unsigned edgeCount;
//
//  public:  // dati public
//    std::list< vertice* > lista_vertici;
//    std::list< arco* >    lista_archi;

public:  // operazioni
    Grafo_ListeAdiacenza();
    virtual ~Grafo_ListeAdiacenza();

    bool sonoAdiacenti(vertice *x, vertice *y) const;
    void inserisciVertice(vertice *v);
    void eliminaVertice(vertice *v);
    void reset();

    void verticiAdiacenti(vertice *v, std::vector<vertice *> &vec) const;
    void verticiAdiacenti(vertice *v, std::vector<adiacente> &vec) const;
    //void archiincidenti(vertice* v, std::vector<arco*> &vec) const;

public:
    typedef std::list< adiacente > adj_list;
    typedef liste_adiacenza LDA_map;
    typedef adj_list::iterator adj_iterator;
    typedef adj_list::const_iterator adj_const_iterator;

public:	// dati uso interno
    LDA_map m;
};



/* PUBLIC */

/*
Costruttore.
*/
Grafo_ListeAdiacenza::Grafo_ListeAdiacenza() {

}



/*
Distruttore.
*/
Grafo_ListeAdiacenza::~Grafo_ListeAdiacenza() {

}



/*
Restituisce true se esiste l'arco (x, y), e false altrimenti.
*/
bool Grafo_ListeAdiacenza::sonoAdiacenti(vertice *x, vertice *y) const {
    return m.sonoAdiacenti(x, y);
}



/*
Inserisce un vertice nella rappresentazione interna.
*/
void Grafo_ListeAdiacenza::inserisciVertice(vertice *v) {
    m.inserisciVertice(v);
}



/*
Elimina il vertice v e tutti gli archi ad esso incidenti.
*/
void Grafo_ListeAdiacenza::eliminaVertice(vertice *v) {
    m.eliminaVertice(v);
}



/*
Svuota completamente la mappa del suo contenuto.
*/
void Grafo_ListeAdiacenza::reset() {
    m.reset();
}



/*
Riempie il vector vec con i vertici adiacenti a v.
*/
void Grafo_ListeAdiacenza::verticiAdiacenti(vertice *v, std::vector<vertice *> &vec) const {
    std::list< adiacente >::const_iterator it = (m.m.find(v))->second.begin();
    std::list< adiacente >::const_iterator end = (m.m.find(v))->second.end();
    vec.clear();
    for (; it != end; ++it)
        vec.push_back(it->vertex);
}



/*
Riempie il vector vec con oggetti 'adiacente'.
*/
void Grafo_ListeAdiacenza::verticiAdiacenti(vertice *v, std::vector<adiacente> &vec) const {
    std::list< adiacente >::const_iterator it = (m.m.find(v))->second.begin();
    std::list< adiacente >::const_iterator end = (m.m.find(v))->second.end();
    vec.clear();
    for (; it != end; ++it)
        vec.push_back(*it);
}


}  // namespace asd


#endif /* _GRAFO_LISTEADIACENZA_H */



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
