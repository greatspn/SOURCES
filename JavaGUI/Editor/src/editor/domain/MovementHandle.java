/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.Graphics2D;

/** An handle that the GUI uses to move objects in a graph.
 *
 * @author Elvio
 */
public interface MovementHandle extends HitTestable {
    
    // save and restore handle position
    public HandlePosition savePosition();    
    public void restorePosition(HandlePosition hp);
    
    public void moveBy(double Dx, double Dy, boolean isMultiSelMove);
    public void moveTo(double x, double y, boolean isMultiSelMove);
    public boolean canMoveTo();
    
    // The handle box is hit at position @pt?
    //public boolean isHit(Point2D pt, int zoomLevel);
    public void paintHandle(Graphics2D g, DrawHelper dh, boolean isClicked);
    
    // Is a node currently focused by the movement?
    public Node getNodeWithAura();
    public int getMagnetWithAura();
    
    // Is a node-graying rule active
    public boolean isGrayNodeRuleActive();
    public boolean isNodeGrayed(Node node);
    
    // Editing (return null if not editable)
    public EditableCell getEditable();
    
    // MOvement granularity
    public NetObject.MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove);
}
