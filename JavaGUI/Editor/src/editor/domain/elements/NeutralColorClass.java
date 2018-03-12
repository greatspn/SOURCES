/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

import java.awt.geom.Point2D;

/** This is not a node class. It is a special ColorClass, with a single instance,
 *  that represents the neutral color. 
 *
 * @author elvio
 */
public class NeutralColorClass extends ColorClass {
    
    // Avoid serializability.
    private class NotSerializable {}
    private NotSerializable obj = new NotSerializable();

    private NeutralColorClass() {
        super("Neutral", new Point2D.Double(0, 0), "{#}");
        isParseDataValid = true;
    }

    @Override
    public boolean isNeutralDomain() {
        return true;
    }
    
    public static NeutralColorClass INSTANCE = new NeutralColorClass();
}
