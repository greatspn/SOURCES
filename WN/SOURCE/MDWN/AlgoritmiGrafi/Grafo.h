/******************************************************************************\
*
* File:          Grafo.h
* Creation date: April 20, 2006
* Author:        Francesco Iovine, Massimiliano Macchia
*
* Purpose:       Fig. 11.2
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _GRAFO_H
#define _GRAFO_H

#include <ext/hash_map>
#include <list>
#include <vector>
#include <climits>
#include <cstdlib>
#include <cassert>


#define INFINITO INT_MAX

#define for_each_vertice(v,G)                                                        \
        v = *((G.lista_vertici).begin());                                            \
        if(G.numVertici())                                                           \
        for (asd::Grafo::vertice_const_iterator it = (G.lista_vertici).begin();      \
             it != (G.lista_vertici).end();                                          \
             ++it, v = *it)

#define for_each_rev_vertice(v,G)                                                    \
        v = *((G.lista_vertici).rbegin());                                           \
        if(G.numVertici())                                                           \
        for (asd::Grafo::vertice_const_rev_iterator it = (G.lista_vertici).rbegin(); \
             it != (G.lista_vertici).rend();                                         \
             ++it, v = *it)

#define for_each_arco(e,G)                                                           \
        e = *((G.lista_archi).begin());                                              \
        if(G.numArchi())                                                             \
        for (asd::Grafo::arco_const_iterator it = (G.lista_archi).begin();           \
             it != (G.lista_archi).end();                                            \
             ++it, e = *it)

#define for_each_rev_arco(e,G)                                                       \
        e = *((G.lista_archi).rbegin());                                             \
        if(G.numArchi())                                                             \
        for (asd::Grafo::arco_const_rev_iterator it = (G.lista_archi).rbegin();      \
             it != (G.lista_archi).rend();                                           \
             ++it, e = *it)

#define for_each_vertice_adiacente(v,u,G)                                            \
        std::vector<vertice*> __vettore__;                                           \
        G.verticiAdiacenti(u,__vettore__);                                           \
        if(__vettore__.size()) v = *(__vettore__.begin());                           \
        for(std::vector<vertice*>::const_iterator it = __vettore__.begin();          \
            it != __vettore__.end();                                                 \
            ++it, v=*it)

#define for_each_rev_vertice_adiacente(v,u,G)                                        \
        std::vector<vertice*> __vettore__;                                           \
        G.verticiAdiacenti(u,__vettore__);                                           \
        if(__vettore__.size()) v=*(__vettore__.rbegin());                            \
        for(std::vector<vertice*>::const_reverse_iterator it = __vettore__.rbegin(); \
            it != __vettore__.rend();                                                \
            ++it, v=*it)

#define for_each_arco_incidente(e,f,G)                                               \
        std::vector<arco*> __vettore__;                                              \
        G.verticiAdiacenti(f,__vettore__);                                           \
        if(__vettore__.size()) e=*(__vettore__.begin());                             \
        for(std::vector<arco*>::const_iterator it = __vettore__.begin();             \
            it!=__vettore__.end();                                                   \
            ++it, e=*it)

#define for_each_rev_arco_incidente(e,f,G)                                           \
        std::vector<arco*> __vettore__;                                              \
        G.verticiAdiacenti(f,__vettore__);                                           \
        if(__vettore__.size()) e=*(__vettore__.rbegin());                            \
        for(std::vector<arco*>::const_reverse_iterator it = __vettore__.rbegin();    \
            it!=__vettore__.rend();                                                  \
            ++it, e=*it)

#define for_each_vertice_adiacente_a_distanza(v,u,w,G)                               \
        std::vector<adiacente> __vettore__;                                          \
        G.verticiAdiacenti(u,__vettore__);                                           \
        if(__vettore__.size()) {                                                     \
            v=(*(__vettore__.begin())).vertex;                                       \
            w=(*(__vettore__.begin())).costo;                                        \
        }                                                                            \
        for(std::vector<adiacente>::const_iterator it = __vettore__.begin();         \
            it!=__vettore__.end();                                                   \
			++it, v=(*it).vertex, w=(*it).costo)

#define for_each_rev_vertice_adiacente_a_distanza(v,u,w,G)                           \
        std::vector<adiacente> __vettore__;                                          \
        G.verticiAdiacenti(u,__vettore__);                                           \
        if(__vettore__.size()) {                                                     \
            v=(*(__vettore__.rbegin())).vertex;                                      \
            w=(*(__vettore__.rbegin())).costo;                                       \
        }                                                                            \
        for(std::vector<adiacente>::const_reverse_iterator it = __vettore__.rbegin();\
            it!=__vettore__.rend();                                                  \
            ++it, v=(*it).vertex, w=(*it).costo)



namespace asd {




class vertice;
class arco;
class hash_v;
template<class T> class vertice_array;
template<class T> class arco_array;
class adiacente;
class Grafo;



/*
Vertice di un grafo
*/
class vertice {
public:
    vertice()
    {}

    ~vertice()
    {}
};



/*
Arco di un grafo
*/
class arco {
public:
    vertice *sorg;
    vertice *dest;
    int costo;

public:
    arco(vertice *a = 0, vertice *b = 0, int costo = 0)
        : sorg(a),
          dest(b),
          costo(costo)
    {}

    ~arco()
    {}

    vertice *opposto(vertice *v) {
        if (v == sorg) return dest;
        return sorg;
    }

    bool incideSu(vertice *v) {
        if (v == sorg || v == dest) return true;
        return false;
    }
};



class hash_v: std::unary_function<vertice *, long> {
public: long operator()(vertice *x) const {
        return (long)x;
    }

public: long operator()(arco *x) const {
        return (long)x;
    }
};



/*
Contenitore associativo per vertici.
*/
template<class T>
class vertice_array
    : public __gnu_cxx::hash_map< vertice *, T, hash_v > {
public:
    vertice_array()
        : __gnu_cxx::hash_map< vertice *, T, hash_v >()
    {}

    ~vertice_array()
    {}
};



/*
Contenitore associativo per archi.
*/
template<class T>
class arco_array
    : public __gnu_cxx::hash_map< arco *, T, hash_v > {
public:
    arco_array()
        : __gnu_cxx::hash_map< arco *, T, hash_v >()
    {}

    ~arco_array()
    {}
};



class adiacente {
public:
    vertice *vertex;
    int costo;

    adiacente(vertice *vert = 0, int c = 0)
        : vertex(vert),
          costo(c)
    {}

    ~adiacente()
    {}
};



/*
Classe astratta che definisce una versione generica di grafo.
*/
class Grafo {
protected:  // dati protected
    unsigned vertexCount;
    unsigned edgeCount;

public:  // dati public
    std::list< vertice * > lista_vertici;
    std::list< arco * >    lista_archi;

    // metodi definiti
public:  // operazioni
    Grafo();
    virtual ~Grafo();

    unsigned numVertici() const;
    unsigned numArchi() const;

    vertice *estremi(arco *e) const;
    vertice *opposto(vertice *x, arco *e) const;
    vertice *aggiungiVertice();
    arco *aggiungiArco(vertice *x, vertice *y, int costo);
    void rimuoviVertice(vertice *v);
    void rimuoviArco(arco *e);
    void clear();
    void genera_random(unsigned n, unsigned m, int costo_max, bool loopfree = false);

    // metodi virtuali
public:
    virtual unsigned grado(vertice *v) const;
    virtual void archiIncidenti(vertice *v, std::vector<arco *> &vec) const;

    // metodi virtuali puri
public:
    virtual void inserisciVertice(vertice *v) = 0;
    virtual void inserisciArco(arco *e) = 0;
    virtual void eliminaVertice(vertice *v) = 0;
    virtual void eliminaArco(arco *e) = 0;
    virtual void reset() = 0;
    virtual bool sonoAdiacenti(vertice *x, vertice *y) const = 0;
    virtual bool diretto() const = 0;
    virtual void verticiAdiacenti(vertice *v, std::vector<vertice *> &vec) const = 0;
    virtual void verticiAdiacenti(vertice *v, std::vector<adiacente> &vec) const = 0;

public:  // typedefs
    typedef std::list< vertice * >::iterator vertice_iterator;
    typedef std::list< vertice * >::const_iterator vertice_const_iterator;
    typedef std::list< vertice * >::reverse_iterator vertice_rev_iterator;
    typedef std::list< vertice * >::const_reverse_iterator vertice_const_rev_iterator;
    typedef std::list< arco * >::iterator arco_iterator;
    typedef std::list< arco * >::const_iterator arco_const_iterator;
    typedef std::list< arco * >::reverse_iterator arco_rev_iterator;
    typedef std::list< arco * >::const_reverse_iterator arco_const_rev_iterator;
};



/* PUBLIC */

/*
Costruttore.
*/
Grafo::Grafo()
    : vertexCount(0)
    , edgeCount(0) {

}



/*
Distruttore.
*/
Grafo::~Grafo() {
    vertice *v;
    for_each_vertice(v, (*this)) delete v;

    arco *e;
    for_each_arco(e, (*this)) delete e;
}



/*
Restituisce il numero di vertici presenti nel grafo.
*/
unsigned Grafo::numVertici() const {
    return vertexCount;
}



/*
Restituisce il numero di archi presenti nel grafo.
*/
unsigned Grafo::numArchi() const {
    return edgeCount;
}



/*
Restituisce gli estremi x e y dell'arco e = (x, y).
*/
vertice *Grafo::estremi(arco *e) const {
    static bool prima_chiamata = true;

    if (prima_chiamata) {
        prima_chiamata = false;
        return e->sorg;
    }
    else {
        prima_chiamata = true;
        return e->dest;
    }
}



/*
Restituisce y, l'estremo dell'arco e = (x, y) diverso da x.
*/
vertice *Grafo::opposto(vertice *x, arco *e) const {
    if (e->dest != x) return e->dest;
    else return e->sorg;
}




/*
Inserisce un nuovo vertice v.
*/
vertice *Grafo::aggiungiVertice() {
    vertice *v = new vertice();

    inserisciVertice(v);
    lista_vertici.push_back(v);
    ++vertexCount;

    return v;
}



/*
Inserisce un nuovo arco tra i vertici x e y.
*/
arco *Grafo::aggiungiArco(vertice *x, vertice *y, int costo = 1) {
    arco *e = new arco(x, y, costo);

    lista_archi.push_back(e);
    inserisciArco(e);

    ++edgeCount;

    return e;
}



/*
Rimuove il vertice v dal grafo.
*/
void Grafo::rimuoviVertice(vertice *v) {
    eliminaVertice(v);

    arco_iterator i, j;
    for (i = lista_archi.begin(); i != lista_archi.end();) {
        if ((*i)->sorg == v || (*i)->dest == v) {
            j = i;
            ++j;
            delete *i;
            lista_archi.erase(i);
            --edgeCount;
            i = j;
        }
        else ++i;
    }

    vertice_iterator k;
    for (k = lista_vertici.begin(); *k != v; ++k) ;
    lista_vertici.erase(k);
    delete v;

    --vertexCount;
}



/*
Rimuove l'arco e dal grafo.
*/
void Grafo::rimuoviArco(arco *e) {
    eliminaArco(e);

    arco_iterator i;
    for (i = lista_archi.begin(); *i != e; ++i) ;
    lista_archi.erase(i);
    delete e;

    --edgeCount;
}



/*
Elimina tutti i vertici e tutti gli archi, rendendo il grafo vuoto.
*/
void Grafo::clear() {
    vertice *v;
    for_each_vertice(v, (*this)) delete(v);

    arco *e;
    for_each_arco(e, (*this)) delete(e);

    reset();

    lista_vertici.clear();
    lista_archi.clear();

    vertexCount = 0;
    edgeCount = 0;
}



/*
Genera casualmente il grafo, con n vertici, m archi,
il cui costo e' nell'intervallo [1, costo_max].
La variabile booleana 'loopfree' si riferisce
alla presenza(false) o no(true) di self-loops.
*/
void Grafo::genera_random(unsigned n, unsigned m, int costo_max, bool loopfree) {
    assert(!(n == 0 && m > 0));
    assert(!(n == 1 && m > 0 && loopfree));

    std::vector<vertice *> V(n);
    std::vector<int> deg(n);
    unsigned i, j;
    int d;

    clear();

    for (i = 0; i < n; ++i) {
        V[i] = aggiungiVertice();
        deg[i] = 0;
    }
    for (i = 0; i < m; i++) {
        deg[rand() % n]++;
    }
    for (i = 0; i < n; i++) {
        vertice *v = V[i];
        d = deg[i];
        while (d > 0) {
            j = rand() % n;
            if (loopfree && j == i) continue;
            aggiungiArco(v, V[j], 1 + (rand() % costo_max));
            d--;
        }
    }
}



/*
Restituisce il grado del vertice v
*/
unsigned Grafo::grado(vertice *v) const {
    unsigned g = 0;

    arco *e;
    for_each_arco(e, (*this))
    if (e->incideSu(v))
        ++g;

    return g;
}



/*
Riempie il vector vec con gli archi incidenti a v.
*/
void Grafo::archiIncidenti(vertice *v, std::vector<arco *> &vec) const {
    arco *e;
    for_each_arco(e, (*this))
    if (e->incideSu(v))
        vec.push_back(e);
}




}  // namespace asd


#endif /* _GRAFO_H */



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
