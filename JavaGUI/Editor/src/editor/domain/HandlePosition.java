/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.geom.Point2D;

/** The position of an handle, to be saved and restored.
 *
 * @author elvio
 */
public interface HandlePosition {
    
    public double getRefX();
    public double getRefY();
    public Point2D getRefPoint();
}
