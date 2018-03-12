/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.io;

import org.w3c.dom.Element;

/** Implements XML data exchange (read/write)
 *
 * @author elvio
 */
public interface XmlExchangeable {
    
    // Bind object attributes to XML nodes
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException;
}
