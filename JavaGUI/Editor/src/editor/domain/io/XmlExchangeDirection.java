/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.io;

import editor.domain.Node;
import java.util.Map;
import org.w3c.dom.Document;

/**
 *
 * @author elvio
 */
public class XmlExchangeDirection {
    
    // Direction of data exchange
    private final boolean xmlToFields;

    // XML document
    private final Document doc;

    
    // Additional data for exchange
    public Map<String, Node> idToNode = null;
    public String arcKindTag = null;
    public boolean afterXmlCleanup = false;
    
    
    public boolean XmlToFields() { return xmlToFields; }
    public boolean FieldsToXml() { return !xmlToFields; }

    public Document getDocument() {
        return doc;
    }

    public XmlExchangeDirection(boolean xmlToFields, Document doc) {
        this.xmlToFields = xmlToFields;
        this.doc = doc;
    }
}
