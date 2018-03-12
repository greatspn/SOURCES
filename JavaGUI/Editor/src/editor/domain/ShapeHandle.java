/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.Graphics2D;

/** An handle that corresponds to an object shape
 *
 * @author elvio
 */
public abstract class ShapeHandle implements MovementHandle {

    @Override
    public void paintHandle(Graphics2D g, DrawHelper dh, boolean isClicked) {
        // Nothing to paint
    }

    @Override
    public Node getNodeWithAura() {
        return null;
    }
    
    @Override
    public int getMagnetWithAura() {
        return -1;
    }

    @Override
    public boolean isGrayNodeRuleActive() {
        return false;
    }

    @Override
    public boolean isNodeGrayed(Node node) {
        throw new UnsupportedOperationException("No gray rule active.");
    }
    
    @Override
    public void moveBy(double Dx, double Dy, boolean isMultiSelMove) {
        HandlePosition hp = savePosition();
        moveTo(hp.getRefX() + Dx, hp.getRefY() + Dy, isMultiSelMove);
    }
}
