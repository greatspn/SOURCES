/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.Util;
import editor.Main;
import editor.domain.NetObject;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Rectangle;
import javax.swing.JComponent;
import javax.swing.UIManager;

/** The horizontal/vertical rulers drawn around the editor panel.
 *
 * @author Elvio
 */
public class Ruler extends JComponent {
    public static final int HORIZONTAL = 0;
    public static final int VERTICAL = 1;
    public final int SIZE;
    
//    public static final Color BKGND = new Color(245, 245, 245);
//    public static final Color CLR_TICK_10 = Color.BLACK;
//    public static final Color CLR_TICK_5 = new Color(128, 128, 128);
//    public static final Color CLR_TICK_1 = new Color(192, 192, 192);
    
    public final Color BKGND;
    private final Color CLR_TICK_10;
    private final Color CLR_TICK_5;
    private final Color CLR_TICK_1;
 
    public int orientation;
    private final JComponent viewportView;
    private int zoomLevel = -1;
    
    public Ruler(int o, JComponent viewportView) {
        this.SIZE = (int)(16 * Main.getUiSize().getScaleMultiplier());
        this.orientation = o;
        this.viewportView = viewportView;
        
        this.BKGND = Util.mix(UIManager.getColor("TextField.background"), new Color(0xEEEEEE), 0.80f);
        this.CLR_TICK_10 = Util.mix(BKGND, UIManager.getColor("TextField.foreground"), 0.05f);
        this.CLR_TICK_5 = Util.mix(BKGND, CLR_TICK_10, 0.25f);
        this.CLR_TICK_1 = Util.mix(BKGND, CLR_TICK_10, 0.50f);
    }

    public int getZoomLevel() {
        return zoomLevel;
    }

    public void setZoomLevel(int zl) {
        if (zl == zoomLevel)
            return;
        zoomLevel = zl;
        repaint();
    }
 
    @Override
    public Dimension getPreferredSize() {
        if (orientation == HORIZONTAL)
            return new Dimension(Math.max(viewportView.getPreferredSize().width, 10), SIZE);
        else
            return new Dimension(SIZE, Math.max(viewportView.getPreferredSize().height, 10));
    }
 
    @Override
    protected void paintComponent(Graphics g) {
        Rectangle drawHere = g.getClipBounds();
 
        // Fill clipping area with the background color.
        g.setColor(BKGND);
        g.fillRect(drawHere.x, drawHere.y, drawHere.width, drawHere.height);
        
        if (zoomLevel <= 0)
            return;
        
        double units = (NetObject.getUnitToPixels() * zoomLevel) / 100;
//        double increment = units;
//        if (units > 50)
//            increment = units / 10;
 
//        // Do the ruler labels in a small font that's black.
//        g.setFont(new Font("SansSerif", Font.PLAIN, 10));
        g.setColor(Color.black);
 
        // Some vars we need.
        int end, start, tickLength;
        //String text = null;
 
        // Use clipping bounds to calculate first and last tick locations.
        if (orientation == HORIZONTAL) {
            start = (int)Math.floor(drawHere.x / units);
            end = (int)Math.ceil(((drawHere.x + drawHere.width) / units) + 1);
            g.drawLine((int)(start * units), SIZE-1, (int)(end * units), SIZE-1);
        } else {
            start = (int)Math.floor(drawHere.y / units);
            end = (int)Math.ceil(((drawHere.y + drawHere.height) / units) + 1);
            g.drawLine(SIZE-1, (int)(start * units), SIZE-1, (int)(end * units));
        }
        
        
 
        // Make a special case of 0 to display the number
        // within the rule and draw a units label.
//        if (start == 0) {
//            text = Integer.toString(0);// + (isMetric ? " cm" : " in");
//            tickLength = 10;
//            if (orientation == HORIZONTAL) {
//                g.drawLine(0, SIZE-1, 0, SIZE-tickLength-1);
//                g.drawString(text, 2, 21);
//            } else {
//                g.drawLine(SIZE-1, 0, SIZE-tickLength-1, 0);
//                g.drawString(text, 9, 10);
//            }
//            text = null;
//            start = increment;
//        }
 
        // ticks and labels
        for (int i = start; i < end; i++) {
            int pos = (int)(i * units);
            Color clr;
            if (i % 10 == 0)  {
                tickLength = 12;
                clr = CLR_TICK_10;
            }
            else if (i % 5 == 0)  {
                tickLength = 9;
                clr = CLR_TICK_5;
                //text = Integer.toString(i/units);
            } else {
//                if (units < 5.0)
//                    continue;
                tickLength = 6;
                clr = CLR_TICK_1;
                //text = null;
            }
 
            if (tickLength != 0) {
                g.setColor(clr);
                if (orientation == HORIZONTAL) {
                    g.drawLine(pos, SIZE-2, pos, SIZE-tickLength-2);
//                    if (text != null)
//                        g.drawString(text, i-3, 21);
                } else {
                    g.drawLine(SIZE-2, pos, SIZE-tickLength-2, pos);
//                    if (text != null)
//                        g.drawString(text, 9, i+3);
                }
            }
        }
    }
}
