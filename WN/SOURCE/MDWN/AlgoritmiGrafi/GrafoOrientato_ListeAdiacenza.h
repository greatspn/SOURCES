/******************************************************************************\
*
* File:          GrafoOrientato_ListeAdiacenza.h
* Creation date: April 20, 2006
* Author:        Francesco Iovine, Massimiliano Macchia
*
* Purpose:       Fig. 11.2 , 11.4(c)
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _GRAFOORIENTATO_LISTEADIACENZA_H
#define _GRAFOORIENTATO_LISTEADIACENZA_H

#include "Grafo_ListeAdiacenza.h"
#include "GrafoOrientato.h"


namespace asd {



/*
Rappresentazione di un grafo orientato con liste di adiacenza.
*/
class GrafoOrientato_ListeAdiacenza : public Grafo_ListeAdiacenza
    , public GrafoOrientato {
    /* dati ereditati da Grafo */
//  private:  // dati private
//    bool tipo;  // tipo di grafo: 'orientato' o 'non_orientato'.
//    unsigned vertexCount;
//    unsigned edgeCount;
//
//  public:  // dati public
//    std::list< vertice* > lista_vertici;
//    std::list< arco* >    lista_archi;

public:  // operazioni
    GrafoOrientato_ListeAdiacenza();
    virtual ~GrafoOrientato_ListeAdiacenza();

    unsigned grado(vertice *v) const;
    unsigned outDegree(vertice *v) const;
    bool sonoAdiacenti(vertice *x, vertice *y) const;
    void verticiAdiacenti(vertice *v, std::vector<vertice *> &vec) const;
    void verticiAdiacenti(vertice *v, std::vector<adiacente> &vec) const;

public:	// operazioni uso interno
    void inserisciArco(arco *e);
    void eliminaArco(arco *e);

public:
    //typedef std::list< adiacente > adj_list;
    //typedef mappa LDA_map;
    //typedef adj_list::iterator adj_iterator;
    //typedef adj_list::const_iterator adj_const_iterator;

public:  // typedefs
    //typedef std::list< vertice* >::iterator vertice_iterator;
    //typedef std::list< vertice* >::const_iterator vertice_const_iterator;
    //typedef std::list< vertice* >::reverse_iterator vertice_rev_iterator;
    //typedef std::list< vertice* >::const_reverse_iterator vertice_const_rev_iterator;
    //typedef std::list< arco* >::iterator arco_iterator;
    //typedef std::list< arco* >::const_iterator arco_const_iterator;
    //typedef std::list< arco* >::reverse_iterator arco_rev_iterator;
    //typedef std::list< arco* >::const_reverse_iterator arco_const_rev_iterator;
};



/* PUBLIC */

/*
Costruttore.
*/
GrafoOrientato_ListeAdiacenza::GrafoOrientato_ListeAdiacenza() {

}



/*
Distruttore.
*/
GrafoOrientato_ListeAdiacenza::~GrafoOrientato_ListeAdiacenza() {

}



/*
Restituisce il numero di archi incidenti sul vertice v.
*/
unsigned GrafoOrientato_ListeAdiacenza::grado(vertice *v) const {
    return m.contaOccorrenze(v) + m.dimensioneLista(v);
}



/*
Restituisce il grado in uscita del vertice v.
*/
unsigned GrafoOrientato_ListeAdiacenza::outDegree(vertice *v) const {
    return m.dimensioneLista(v);
}



/*
Restituisce true se esiste l'arco (x, y), e false altrimenti.
*/
bool GrafoOrientato_ListeAdiacenza::sonoAdiacenti(vertice *x, vertice *y) const {
    return Grafo_ListeAdiacenza::sonoAdiacenti(x, y);
}



/*
Riempie il vector vec con i vertici adiacenti a v.
*/
void GrafoOrientato_ListeAdiacenza::verticiAdiacenti(vertice *v, std::vector<vertice *> &vec) const {
    Grafo_ListeAdiacenza::verticiAdiacenti(v, vec);
}



/*
Riempie il vector vec con oggetti 'adiacente'.
*/
void GrafoOrientato_ListeAdiacenza::verticiAdiacenti(vertice *v, std::vector<adiacente> &vec) const {
    Grafo_ListeAdiacenza::verticiAdiacenti(v, vec);
}



/*
Inserisce l'arco e nella rappresentazione.
*/
void GrafoOrientato_ListeAdiacenza::inserisciArco(arco *e) {
    m.inserisciArco(e->sorg, e->dest, e->costo);
}



/*
Elimina l'arco e dalla rappresentazione interna.
*/
void GrafoOrientato_ListeAdiacenza::eliminaArco(arco *e) {
    m.eliminaArco(e->sorg, e->dest, e->costo);
}

}  // namespace asd


#endif /* _GRAFOORIENTATO_LISTEADIACENZA_H */



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
