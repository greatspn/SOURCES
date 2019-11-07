/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.ProjectResource;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLContent;
import java.io.Serializable;
import java.util.Base64;
import org.w3c.dom.Element;

/** A serialized text document (log of a tool-chain computation).
 *
 * @author elvio
 */
public class DocumentResource extends ProjectResource implements Serializable {
    
    public byte[] docData;

    public DocumentResource(byte[] docData) {
        this.docData = docData;
    }

    public DocumentResource() { }
    
    
    public String getDocumentDataBase64() {
        return Base64.getEncoder().withoutPadding().encodeToString(docData);
//        return javax.xml.bind.DatatypeConverter.printBase64Binary(docData);
    }
    public void setDocumentDataBase64(String base64data) {
        docData = Base64.getDecoder().decode(base64data);
//        docData = javax.xml.bind.DatatypeConverter.parseBase64Binary(base64data);
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
        
        bindXMLContent(this, el, exDir, "@DocumentDataBase64", "");
    }
}
