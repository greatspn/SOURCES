/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Point2D;
import java.io.Serializable;

/** A node/edge decoration (a label, a number, etc...)
 *
 * @author Elvio
 */
public abstract class Decor extends NetObject implements Serializable, HitTestable, PlaceableObject {
    
    public enum Type {
        NAME_LABEL
    }
    
    // Position relative to the parent node/edge
    //public Point2D pos;
    
    public abstract double getWidth();
    public abstract double getHeight();
    // Get the relative X,Y offset from the parent anchor point
    public abstract double getRelativeX();
    public abstract double getRelativeY();    
    // Get the real X,Y positions in the graph
    public abstract void getEffectivePos(Point2D pt);
    
    // Tell if the decor is currently visible
    public boolean isVisible(ViewProfile vp) { return true; }
        
    // Special position anchor for edge labels
    public abstract double getEdgeK();
    public abstract void setEdgeK(double k);
    public abstract boolean drawLineBetweenDecorAndEdge();

    public abstract void paintDecor(Graphics2D g, DrawHelper dh, 
                                    //DecorHolder holder,
                                    boolean isParentGrayed,
                                    boolean isParentError);
    
    // Movement of the decor
    //public abstract MovementHandle getMovementHandle(/*DecorHolder holder*/);
    
    //public abstract boolean intersectRectangle(Rectangle2D rect);
    
    private static final Color EDGE_ATTACH_COLOR_HANDLE = new Color(240, 26, 211);
    public MovementHandle getEdgeAttachmentHandle(final Edge edge) {
        return new DraggableHandle() {
            class KCoeffPosition implements HandlePosition {
                double k;
                public KCoeffPosition(double k) {
                    this.k = k;                }
                
                @Override
                public double getRefX() { return getRefPoint().getX(); }
                @Override
                public double getRefY() { return getRefPoint().getY(); }
                @Override
                public Point2D getRefPoint() {
                    return edge.getPointAlongTheLine(new Point2D.Double(), k);
                }

            }
            @Override
            protected DraggableHandle.BoxShape getBoxShape() { return BoxShape.CIRCLE;}

            @Override
            protected Color getHandleColor() { return EDGE_ATTACH_COLOR_HANDLE; }

            @Override
            public HandlePosition savePosition() {
                return new KCoeffPosition(getEdgeK());
            }

            @Override
            public void restorePosition(HandlePosition hp) {
                setEdgeK(((KCoeffPosition)hp).k);
            }

            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                Point2D p = new Point2D.Double(x, y);
                double k = edge.getNearestK(p);
                setEdgeK(k);
            }

            @Override
            public boolean canMoveTo() {
                return true;
            }

            @Override public EditableCell getEditable() { return null; }
            
            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { 
                return POINT_GRID;
            }
        };
    }
}
