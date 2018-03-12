/******************************************************************************\
*
* File:          GrafoOrientato_ListeIncidenza.h
* Creation date: April 20, 2006
* Author:        Francesco Iovine, Massimiliano Macchia
*
* Purpose:       Fig. 11.2 , 11.4(d)
*
* License:       See the end of this file for license information
*
\******************************************************************************/
#ifndef _GRAFOORIENTATO_LISTEINCIDENZA_H
#define _GRAFOORIENTATO_LISTEINCIDENZA_H

#include <grafo/Grafo_ListeIncidenza.h>
#include <grafo/GrafoOrientato.h>


namespace asd {



/*
Rappresentazione di un grafo orientato con liste di incidenza.
*/
class GrafoOrientato_ListeIncidenza : public Grafo_ListeIncidenza
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
    GrafoOrientato_ListeIncidenza();
    virtual ~GrafoOrientato_ListeIncidenza();

    bool sonoAdiacenti(vertice *x, vertice *y) const;
    unsigned inDegree(vertice *v) const;
    unsigned outDegree(vertice *v) const;
    void verticiAdiacenti(vertice *v, std::vector<vertice *> &vec) const;
};



/* PUBLIC */

/*
Costruttore.
*/
GrafoOrientato_ListeIncidenza::GrafoOrientato_ListeIncidenza() {

}



/*
Distruttore.
*/
GrafoOrientato_ListeIncidenza::~GrafoOrientato_ListeIncidenza() {

}



/*
Restituisce true se esiste l'arco (x, y), e false altrimenti.
*/
bool GrafoOrientato_ListeIncidenza::sonoAdiacenti(vertice *x, vertice *y) const {
    return m.sonoAdiacenti_direct(x, y);
}



/*
Restituisce il grado in ingresso del vertice v.
*/
unsigned GrafoOrientato_ListeIncidenza::inDegree(vertice *v) const {
    return m.contaDest(v);
}



/*
Restituisce il grado in uscita del vertice v.
*/
unsigned GrafoOrientato_ListeIncidenza::outDegree(vertice *v) const {
    return m.contaSorg(v);
}



/*
Riempie il vector vec con i vertici adiacenti a v.
*/
void GrafoOrientato_ListeIncidenza::verticiAdiacenti(vertice *v, std::vector<vertice *> &vec) const {
    std::list< arco * >::const_iterator it = (m.m.find(v))->second.begin();
    std::list< arco * >::const_iterator end = (m.m.find(v))->second.end();
    vec.clear();
    for (; it != end; ++it)
        if ((*it)->sorg == v)
            vec.push_back((*it)->dest);
}



}  // namespace asd


#endif /* _GRAFOORIENTATO_LISTEINCIDENZA_H */



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
