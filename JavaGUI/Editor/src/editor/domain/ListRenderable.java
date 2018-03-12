/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import javax.swing.Icon;

/** An item that can be shown in a JList with a text and an icon
 *
 * @author elvio
 */
public interface ListRenderable {
    
    public String getDescription();
    
    public Icon getIcon16();
    
    public int getTextSize();
}
