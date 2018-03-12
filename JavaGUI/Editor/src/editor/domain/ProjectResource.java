/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.io.XmlExchangeable;
import java.io.Serializable;
import java.util.UUID;
import org.w3c.dom.Element;

/** A class that contains unmodifiable data
 *  that are stored in the project' resource table.
 *
 * @author elvio
 */
public class ProjectResource implements Serializable, XmlExchangeable {
    
    private final UUID uniqueResourceID;

    public ProjectResource() {
        uniqueResourceID = UUID.randomUUID();
    }

    public ProjectResource(UUID uniqueResourceID) {
        this.uniqueResourceID = uniqueResourceID;
    }
    
    public UUID getResourceID() {
        return uniqueResourceID;
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "uuid", "uniqueResourceID", null, UUID.class);
    }
}
