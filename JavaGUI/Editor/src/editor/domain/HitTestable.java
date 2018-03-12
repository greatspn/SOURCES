/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.geom.Rectangle2D;

/** An object that can be hit tested against a rectangle.
 *
 * @author elvio
 */
public interface HitTestable {
    
    public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                      boolean includeDecors);
}
