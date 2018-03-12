/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

/**
 *
 * @author elvio
 */
public interface PlaceableObject {
    
//    public abstract double getWidth();
//    public abstract double getHeight();
    
    public MovementHandle getCenterHandle(NetPage thisPage);
    
    // Return the Editable object when the user double clicks on the object
    public abstract EditableCell getCentralEditable(double px, double py);
}
