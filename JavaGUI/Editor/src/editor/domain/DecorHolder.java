/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

/** An object that may have a set of decorations attached to.
 *
 * @author elvio
 */
public interface DecorHolder {
    // Enumeration
    public int getNumDecors();
    public Decor getDecor(int i);
    
    // Placement
    //public void getDecorAnchorPoint(Point2D out, double K);
}
