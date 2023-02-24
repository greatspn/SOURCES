/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.Util;
import editor.domain.DrawHelper;
import editor.domain.NetObject;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;

/**
 *
 * @author Elvio
 */
public class NetToolBase implements MouseListener, MouseMotionListener, KeyListener {

    protected int dragCounter = 0; // 0 = not dragging. Counter due to multiple mouse button drags.
    protected int startDragX = 0, startDragY = 0;
    protected int currDragX = 0, currDragY = 0;
    protected final NetEditorPanel.Tool activeTool;
    protected final NetEditorPanel editor;
    
    // Avoid spurious clicks from previous tool instances
    protected int seenPressed = 0;
    // Modifiers mask (Ctrl, Shift, Alt, ...)
    private int lastSeenModifiers = 0;

    protected NetToolBase(NetEditorPanel editor, NetEditorPanel.Tool activeTool) {
        this.editor = editor;
        this.activeTool = activeTool;
    }

    public final void myMousePressed(MouseEvent e) {
        seenPressed |= (1 << e.getButton());
        lastSeenModifiers = e.getModifiersEx();
        
        if (dragCounter == 0) {
//        assert !isDragging;
            startDragX = e.getX();
            startDragY = e.getY();
            currDragX = startDragX;
            currDragY = startDragY;
        }
        dragCounter++;
        mousePressed(e);
    }
    
    public final void myMouseDragged(MouseEvent e) {
        lastSeenModifiers = e.getModifiersEx();
        if (0 != (lastSeenModifiers & MouseEvent.BUTTON1_DOWN_MASK) && 
            0 == (seenPressed & (1 << MouseEvent.BUTTON1)))
            return;
//        if (0 == (seenPressed & (1 << e.getButton())))
//            return; // Spurious mouse event
        assert dragCounter > 0;
        currDragX = e.getX();
        currDragY = e.getY();
        mouseDragged(e);
    }
    
    public final void myMouseReleased(MouseEvent e) {
        lastSeenModifiers = e.getModifiersEx();
        if (0 == (seenPressed & (1 << e.getButton())))
            return; // Spurious mouse event
        assert dragCounter > 0;
        dragCounter--;
        mouseReleased(e);
    }
    
    public final void myMouseClicked(MouseEvent e) {
        lastSeenModifiers = e.getModifiersEx();
        if (0 == (seenPressed & (1 << e.getButton())))
            return; // Spurious mouse event
        mouseClicked(e);
    }
    
    public void myMouseMoved(MouseEvent e) { 
        lastSeenModifiers = e.getModifiersEx();
        mouseMoved(e);
    }

    public void myMouseEntered(MouseEvent e) { 
        lastSeenModifiers = e.getModifiersEx();
        mouseEntered(e);
    }

    public void myMouseExited(MouseEvent e) { 
        lastSeenModifiers = e.getModifiersEx();
        mouseExited(e);
    }

    //public void toolEnabled(NetEditorPanel.Tool tool) { }
    public void toolDisabled() { }
    public void paintBeforeNet(Graphics2D g, DrawHelper dh) { }
    public void paintAfterNet(Graphics2D g, DrawHelper dh) { }
    public void topWindowActivated(boolean activated) { }
    
    @Override public void mouseClicked(MouseEvent e) { }    
    @Override public void mousePressed(MouseEvent e) { }
    @Override public void mouseReleased(MouseEvent e) { }
    @Override public void mouseEntered(MouseEvent e) { }
    @Override public void mouseExited(MouseEvent e) { }
    @Override public void mouseDragged(MouseEvent e) { }
    @Override public void mouseMoved(MouseEvent e) { }
    @Override public void keyTyped(KeyEvent e) { }
    @Override public void keyPressed(KeyEvent e) { }
    @Override public void keyReleased(KeyEvent e) { }
    
    private boolean isUserTogglingGridByPressinAlt() {
        // User pressing CMD/ALT while dragging disables the grid snapping
        int mask = (Util.isOSX() ? MouseEvent.META_DOWN_MASK | MouseEvent.ALT_DOWN_MASK 
                    : MouseEvent.ALT_DOWN_MASK);
        return (0 != (lastSeenModifiers & mask));
    }
    
    protected Point2D getLogicPointFromMousePos(MouseEvent e, NetObject.MeshGridSize mgs) {
        double LX = NetObject.screenToLogic(e.getX(), editor.currPage.viewProfile.zoom);
        double LY = NetObject.screenToLogic(e.getY(), editor.currPage.viewProfile.zoom);
        return snapPointToGrid(new Point2D.Double(LX, LY), mgs);
    }
        
//    private final static double GRID_ANGLE_DEGREES = 15.0;
    protected Point2D snapPointToGrid(Point2D pt, NetObject.MeshGridSize mgs) {
        if (mgs != null) {
            boolean gridSnapping = (editor.isGridSnappingActive());
            if (isUserTogglingGridByPressinAlt())
                gridSnapping = !gridSnapping;
            if (mgs == NetObject.NODE_GRID) {
                if (gridSnapping)
                    pt.setLocation(Math.round(pt.getX()), Math.round(pt.getY()));
            }
            else if (mgs == NetObject.POINT_GRID) {
                if (gridSnapping)
                    pt.setLocation(Math.round(pt.getX() * 2.0) / 2.0,
                                   Math.round(pt.getY() * 2.0) / 2.0);
            }
            else {
                // Rotation handle
                double theta = -Math.atan2(pt.getY() - mgs.rotationCenter.getY(),
                                           pt.getX() - mgs.rotationCenter.getX());
                //theta += Math.PI / 2.0;
                if (gridSnapping)
                    theta = NetObject.DEG_TO_RAD * (mgs.gridSnapping * Math.round
                                                    (theta * NetObject.RAD_TO_DEG / mgs.gridSnapping));
                
                pt.setLocation(mgs.rotationCenter.getX() + mgs.rotationShaft * Math.cos(theta),
                               mgs.rotationCenter.getY() + mgs.rotationShaft * Math.sin(theta));
            }
        }
        return pt;
    }
    
//    protected double snapValueToGrid(double value, NetObject.MeshGridSize mgs) {
//        if (editor.isGridSnappingActive() && mgs != null) {
//            if (mgs == NetObject.NODE_GRID) {
//                return Math.round(value);
//            }
//            else if (mgs == NetObject.POINT_GRID) {
//                return Math.round(value * 2.0) / 2.0;
//            }
//        }
//        return value;
//    }
//    
    
    private static String formatDoubleToShow(double d) {
        if(d == (int) d)
            return ""+(int)d;
        else
            return String.format("%.2f", d);
    }
    
    protected static final Color GRID_COLOR = new Color(0, 148, 255);
    protected static final Stroke GRID_STROKE = new BasicStroke(1.0f, BasicStroke.CAP_SQUARE, 
                                                                BasicStroke.JOIN_MITER, 1.0f, 
                                                                new float[] {2.0f, 3.0f}, 0.0f);
    protected static final Font GRID_FONT = new Font("SansSerif", 0, 12);
    protected void paintGridLines(Graphics2D g, DrawHelper dh, double x, double y,
                                  NetObject.MeshGridSize mgs) 
    {
        boolean isRotation = (mgs != null && mgs.isRotation());
        boolean shouldPaint = (editor.isGridSnappingActive() || isRotation);
        if (isUserTogglingGridByPressinAlt())
            shouldPaint = !shouldPaint;
        if (!shouldPaint)
            return;
        Composite oldComp = g.getComposite();
        Font oldFont = g.getFont();
        g.setComposite(NetObject.ALPHA_50);
        g.setColor(GRID_COLOR);
        g.setFont(GRID_FONT);
        Stroke oldStroke = g.getStroke();
        g.setStroke(GRID_STROKE);
        int scrX = dh.logicToScreen(x);
        int scrY = dh.logicToScreen(y);
        final int SNAP = Math.max(editor.netPanel.getWidth(), editor.netPanel.getHeight());
        AffineTransform oldAT = g.getTransform();
        g.setTransform(dh.baseTransform);
        if (!isRotation) {
            // position grid
            g.drawLine(scrX, 0, scrX, scrY+SNAP);
            g.drawLine(0, scrY, scrX+SNAP, scrY);
            int len = dh.logicToScreen(3);
            g.drawString("["+formatDoubleToShow(x)+", "+formatDoubleToShow(y)+"]", 
                         scrX + len, scrY + len);
        }
        else {
            // rotation gizmo
            int cX = dh.logicToScreen(mgs.rotationCenter.getX());
            int cY = dh.logicToScreen(mgs.rotationCenter.getY());
            int len = dh.logicToScreen(mgs.rotationShaft);
            g.drawLine(cX, 0, cX, SNAP);
            g.drawLine(0, cY, SNAP, cY);
            g.drawLine(cX, cY, cX + (int)((scrX-cX) * 1.2), cY + (int)((scrY-cY) * 1.2));
            double theta = -Math.atan2(scrY-cY, scrX-cX);
            int theta2 = (int)Math.round(theta * NetObject.RAD_TO_DEG);
            if (theta2 < 0)
                theta2 += 360;
            g.drawArc(cX-len/2, cY-len/2, len, len, 0, theta2);
            g.drawString(theta2+"˚", cX + (int)(len/2.5), cY - (int)(len/2.5));
        }
        g.setTransform(oldAT);
        g.setColor(Color.BLACK);
        g.setStroke(oldStroke);            
        g.setComposite(oldComp);   
        g.setFont(oldFont);
    }
}
