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
import org.w3c.dom.Element;

/** Base class of nodes and edges. Contains common graphical parameters.
 *
 * @author elvio
 */
public abstract class GraphicalElement extends SelectableObject implements XmlExchangeable {

    // Default label text scale
    private LabelDecor.Size textSize = LabelDecor.Size.NORMAL;    

    public EditableValue getLabelTextSizeEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return textSize; }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                setGfxTextSize((LabelDecor.Size)value);
            }
        };
    }

    public LabelDecor.Size getGfxTextSize() {
        return textSize;
    }
    public void setGfxTextSize(LabelDecor.Size textSize) {
        this.textSize = textSize;
    }
    
    // write gfx properties to file
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "text-size", "textSize", LabelDecor.Size.NORMAL);
    }
}
