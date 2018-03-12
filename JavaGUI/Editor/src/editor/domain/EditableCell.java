/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.geom.Point2D;

/** Something that can be edited with the overlay editor (like double clicking
 * an obecjt to open a textfield that changes the object value).
 *
 * @author elvio
 */
public interface EditableCell extends HitTestable, EditableValue {
    
    public Point2D getEditorCenter();

}
