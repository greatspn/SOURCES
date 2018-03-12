/******************************************************************************\
*
* File:          CodaPriorita.h
* Creation date: September 16, 2005
* Author:        Francesco Iovine
*
* Purpose:       Fig.8.1
*
* License:       See the end of this file for license information
*
* Modifications:
*   1)  Commentati tutti i metodi della classe base.
*       October 2, 2006. Francesco Iovine
*
\******************************************************************************/
#ifndef _CODAPRIORITA_H
#define _CODAPRIORITA_H


namespace asd {

/*
Figura 8.1
Il tipo di dato CodaPriorita
*/
template<class Elem, class Chiave>
class CodaPriorita {
    /* dati:
          un insieme S di n elementi di tipo 'elem'
          a cui sono associate chiavi di tipo 'chiave'
          prese da un universo totalmente ordinato. */

public:  // operazioni
    virtual ~CodaPriorita() {}

//    Elem findMin() const;
//    void* insert(const Elem&, const Chiave&);
//    void delete_(void*);
//    Elem deleteMin();
//    void increaseKey(void*, const Chiave&);
//    void decreaseKey(void*, const Chiave&);
//    void merge(const CodaPriorita<Elem, Chiave>& c1,
//               const CodaPriorita<Elem, Chiave>& c2);
};

}  // namespace asd


#endif /* _CODAPRIORITA_H */



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
