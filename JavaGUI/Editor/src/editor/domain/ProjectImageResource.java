/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import editor.Main;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLContent;
import java.awt.Image;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.Serializable;
import java.util.Arrays;
import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import org.w3c.dom.Element;

/**
 *
 * @author elvio
 */
public class ProjectImageResource extends ProjectResource implements Serializable {
    
    byte[] imageData;    
    transient Image image = null;

    public ProjectImageResource(byte[] imageData) {
        this.imageData = imageData;
    }

    public ProjectImageResource() { }

    
    @Override
    public boolean equals(Object obj) {
        if (obj instanceof ProjectImageResource) {
            ProjectImageResource pir = (ProjectImageResource)obj;
            return Arrays.equals(imageData, pir.imageData);
        }
        return super.equals(obj);
    }
    
    private static final ImageIcon MISSING = Main.loadImageIcon("missing_image.png");

    public Image getImage() {
        if (image == null) {
            try {
                image = ImageIO.read(new ByteArrayInputStream(imageData));
            }
            catch (IOException e) {
                Main.logException(e, true);
            }
            if (image == null)
                image = MISSING.getImage();
        }
        return image;
    }

    public byte[] getImageData() {
        return imageData;
    }
    
    public String getImageDataBase64() {
        return javax.xml.bind.DatatypeConverter.printBase64Binary(imageData);
    }
    public void setImageDataBase64(String base64data) {
        imageData = javax.xml.bind.DatatypeConverter.parseBase64Binary(base64data);
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        bindXMLContent(this, el, exDir, "@ImageDataBase64", "");
    }
}
