/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.gui.ResourceFactory;
import java.awt.Shape;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.RoundRectangle2D;
import javax.swing.Icon;

/** Edge attachment model: how edges attach to node borders.
 * 
 * @author elvio
 */
public enum EdgeAttachmentModel implements ListRenderable {
    CENTER_ONLY(0, "Center only", "eam_center_only"), 
    ORTHOGONAL(0, "Orthogonal", "eam_orthogonal"), 
    NSEW_POINTS(4, "N, S, E, W", "eam_nseo"),            // North, south, east west
    SQUARE_POINTS(4, "Square points", "eam_square"),     // vertices of a square
    NSEW_SQUARE_POINTS(8, "NSEW + Square", "eam_nseo_square"), // 8 magnets (NSEW + SQUARE)
    ONE_PER_SIDE(4, "One per side", "eam_side1_"), 
    TWO_PER_SIDE(8, "Two per side", "eam_side2_"), 
    THREE_PER_SIDE(12, "Three per side", "eam_side3_"), 
    FOUR_PER_SIDE(16, "Four per side", "eam_side4_"), 
    FIVE_PER_SIDE(20, "Five per side", "eam_side5_");
    
    private final int numMagnets;
    private final String description;
    private final String iconName;
    private Icon icon16;

    private EdgeAttachmentModel(int numMagnets, String description, String iconName) {
        this.numMagnets = numMagnets;
        this.description = description;
        this.iconName = iconName;
    }
    
    public int numMagnets() {
        return numMagnets;
    }

    @Override
    public String getDescription() {
        return description;
    }

    @Override
    public Icon getIcon16() {
        if (icon16 == null)
            this.icon16 = ResourceFactory.loadIcon16(iconName);
        return icon16;
    }

    @Override public int getTextSize() { return -1; /* default*/ }
    
    public boolean useOrthogonalEdgeAttachment() {
        return this == ORTHOGONAL;
    }
    
    private static final Line2D.Double sideLine = new Line2D.Double();
    public void getMagnet(Point2D out, int magNum, Shape shape) {
        assert magNum >= 0 && magNum < numMagnets;
        assert this != CENTER_ONLY;
        assert this != ORTHOGONAL;
        
        // *** Circle/ellipse case ***
        if (shape instanceof Ellipse2D) {
            Ellipse2D e = (Ellipse2D)shape;
            
            double baseAngle;
            double radiantPerStep = (2 * Math.PI) / numMagnets;
            switch (this) {
                case NSEW_POINTS:
                case NSEW_SQUARE_POINTS:
                case ONE_PER_SIDE:
                case THREE_PER_SIDE:
                case FIVE_PER_SIDE:
                    baseAngle = 0;
                    break;
                    
                case SQUARE_POINTS:
                    baseAngle = Math.PI / 4;
                    break;

                case TWO_PER_SIDE:
                    baseAngle = Math.PI / 8;
                    break;

                case FOUR_PER_SIDE:
                    baseAngle = Math.PI / 8;
                    break;
                    
                default:
                    throw new IllegalStateException("Should not be here.");
            }
            
            double theta = baseAngle + magNum * radiantPerStep;
            out.setLocation(e.getCenterX() + (e.getWidth()/2.0) * Math.cos(theta), 
                            e.getCenterY() + (e.getHeight()/2.0) * Math.sin(theta));
            return;
        }
        
        // *** Rectangle/round rectangle case ***
        double x, y, w, h, rx, ry;
        if (shape instanceof Rectangle2D) {
            Rectangle2D rect = (Rectangle2D)shape;
            x = rect.getX();
            y = rect.getY();
            w = rect.getWidth();
            h = rect.getHeight();
            rx = ry = 0;
        }
        else if (shape instanceof RoundRectangle2D) {
            RoundRectangle2D roundRect = (RoundRectangle2D)shape;
            x = roundRect.getX();
            y = roundRect.getY();
            w = roundRect.getWidth();
            h = roundRect.getHeight();
            rx = roundRect.getArcWidth() / 2;
            ry = roundRect.getArcHeight() / 2;
        }
        else throw new UnsupportedOperationException("Unsupported shape.");
        double cx = x + w/2, cy  = y + h/2;
        
        if (this == NSEW_POINTS || (this == NSEW_SQUARE_POINTS && magNum < 4)) {
            // 0=north, 1=east, 2=south, 3=west
            switch (magNum) {
                case 0:   out.setLocation(cx, y); break;
                case 1:   out.setLocation(x+w, cy); break;
                case 2:   out.setLocation(cx, y+h); break;
                case 3:   out.setLocation(x, cy); break;
                default: throw new IllegalStateException();
            }
            return;
        }
        if (this == SQUARE_POINTS || this == NSEW_SQUARE_POINTS) {
            if (this == NSEW_SQUARE_POINTS)
                magNum -= 4; // The first four points are the NSEW points
            final double K = (1.0 - 0.70710678118); // 1 - sin(45)
            // 0=top-left, 1=top-right, 2=bottom-right, 3=bottom-left
            out.setLocation((magNum==0 || magNum==3) ? x + rx*K : x + w - rx*K,
                            (magNum==0 || magNum==1) ? y + ry*K : y + h - ry*K);
            return;
        }
        // Side cases
        int numPtsPerSide = numMagnets / 4;
        int side = magNum / numPtsPerSide;
        // 0=top, 1=right, 2=bottom, 3=left
        switch (side) {
            case 0: // top
                sideLine.setLine(x + rx, y, x + w - rx, y);
                break;
            case 1: // right
                sideLine.setLine(x + w, y + ry, x + w, y + h - ry);
                break;
            case 2: // bottom
                sideLine.setLine(x + rx, y + h, x + w - rx, y + h);
                break;
            case 3: // left
                sideLine.setLine(x, y + ry, x, y + h - ry);
                break;
            default: throw new IllegalStateException();
        }
        double coeff = (1 + (magNum % numPtsPerSide)) / (double)(numPtsPerSide + 1);
        out.setLocation(NetObject.linearInterp(sideLine.x1, sideLine.x2, coeff),
                        NetObject.linearInterp(sideLine.y1, sideLine.y2, coeff));
        //return;
    }    
}
