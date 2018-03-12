/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.Main;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

/** A movement handle that can be drawn as a small, draggable square
 *
 * @author Elvio
 */
public abstract class DraggableHandle implements MovementHandle {
    
    public final int DEFAULT_BOX_SIZE;
    //public static final Color HANDLE_COLOR = new Color(200,0,0);
    
    public static final Color HEAD_HANDLE_COLOR = new Color(255, 71, 71);
    public static final Color TAIL_HANDLE_COLOR = new Color(84, 255, 76);
    public static final Color MIDPOINT_HANDLE_COLOR = new Color(71, 71, 255);
    
    public enum BoxShape {
        DIAMOND, CIRCLE, LIGHT_CIRCLE, SQUARE
    }

    public DraggableHandle() {
        DEFAULT_BOX_SIZE = (int)(4 * Main.getUiSize().getScaleMultiplier());
    }
    
    protected abstract BoxShape getBoxShape();
    protected Color getHandleColor() { return MIDPOINT_HANDLE_COLOR; }
    protected boolean isEdgeHandle() { return false; }
    protected int getBoxSize() { return DEFAULT_BOX_SIZE; }
    protected Point2D getGizmoCenter() { return null; }

    
    //private static Rectangle handleRect = new Rectangle();
    @Override
    public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                      boolean includeDecors) 
    {
        HandlePosition pos = savePosition();
        return rect.contains(pos.getRefPoint());
    }
        
    public static int[] xCoords = new int[4], yCoords = new int[4];
    @Override
    public void paintHandle(Graphics2D g, DrawHelper dh, boolean isClicked) {
        
        HandlePosition pos = savePosition();
        int cx = dh.logicToScreen(pos.getRefX());
        int cy = dh.logicToScreen(pos.getRefY());
        
        AffineTransform oldAT = g.getTransform();
        Composite oldComp = g.getComposite();
        Color oldClr = g.getColor();
        
        boolean isSelected = false;
        if (this instanceof Selectable)
            isSelected = ((Selectable)this).isSelected();
        boolean filled = isClicked || isSelected;
        
        g.setTransform(dh.baseTransform);

        // Rotation gizmo
        Point2D gizmoCenter = getGizmoCenter();
        if (gizmoCenter != null) {
            int gcx = dh.logicToScreen(gizmoCenter.getX());
            int gcy = dh.logicToScreen(gizmoCenter.getY());
            Stroke oldS = g.getStroke();
            g.setComposite(NetObject.ALPHA_50);
            g.setColor(getHandleColor());
            g.setStroke(NetObject.LABEL_BOX_STROKE);
            
            g.drawLine(cx, cy, gcx, gcy);
            
            g.setStroke(oldS);
            g.setComposite(oldComp);
        }
        final int boxSize = getBoxSize();
        BoxShape shape = getBoxShape();
        if (shape == BoxShape.DIAMOND) {
            xCoords[0] = cx;              yCoords[0] = cy - boxSize;
            xCoords[1] = cx + boxSize;   yCoords[1] = cy;
            xCoords[2] = cx;              yCoords[2] = cy + boxSize;
            xCoords[3] = cx - boxSize;   yCoords[3] = cy;            
        }
        
        if (filled) {
            g.setColor(getHandleColor());
            switch (shape) {
                case DIAMOND: 
                    g.fillPolygon(xCoords, yCoords, 4); 
                    break;
                case CIRCLE:  
                case LIGHT_CIRCLE:
                    g.fillOval(cx-boxSize, cy-boxSize, 2*boxSize, 2*boxSize); 
                    break;
                case SQUARE:  
                    g.fillRect(cx-boxSize, cy-boxSize, 2*boxSize, 2*boxSize); 
                    break;
            }
        }
        else {
            g.setComposite(NetObject.ALPHA_75);
            g.setColor(Color.WHITE);
            switch (shape) {
                case DIAMOND: 
                    g.fillPolygon(xCoords, yCoords, 4); 
                    break;
                case CIRCLE:  
                case LIGHT_CIRCLE:  
                    g.fillOval(cx-boxSize, cy-boxSize, 2*boxSize, 2*boxSize); 
                    break;
                case SQUARE:  
                    g.fillRect(cx-boxSize, cy-boxSize, 2*boxSize, 2*boxSize); 
                    break;
            }
            g.setComposite(oldComp);
            
            g.setColor(getHandleColor());
            switch (shape) {
                case DIAMOND: 
                    g.drawPolygon(xCoords, yCoords, 4); 
                    break;
                case CIRCLE:  
                    g.drawOval(cx-boxSize+1, cy-boxSize+1, 2*boxSize-2, 2*boxSize-2);
                    // Fall through...
                case LIGHT_CIRCLE:
                    g.drawOval(cx-boxSize, cy-boxSize, 2*boxSize, 2*boxSize);
                    break;
                case SQUARE:
                    g.drawRect(cx-boxSize, cy-boxSize, 2*boxSize, 2*boxSize);
                    break;
            }
        }
                
        g.setColor(oldClr);
        g.setComposite(oldComp);
        g.setTransform(oldAT);
    }
    
    
    @Override public Node getNodeWithAura() { 
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
        throw new UnsupportedOperationException("No gray rule defined."); 
    }
    
    @Override
    public final void moveBy(double Dx, double Dy, boolean isMultiSelMove) {
        HandlePosition hp = savePosition();
        moveTo(hp.getRefX() + Dx, hp.getRefY() + Dy, isMultiSelMove);
    }
}
