/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package latex;

import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.geom.AffineTransform;
import java.io.Serializable;
import javax.swing.Icon;

/** An immutable LaTeX formula that can be drawn on a Graphics target.
 * Uses the LatexProvider as backend to generate and draw the graphics sequence.
 *
 * @author elvio
 */
public class LatexFormula implements Serializable {
    // The latex formula
    private final String latex;
    // Size of the formula
    private final float size;
    // Transient graphic commands
    private transient MetaCommands graphicCommands;

    public LatexFormula(String latex, float size) {
        this.latex = latex;
        this.size = size;
    }
    
    
    private void retrieveCommands() {
        if (graphicCommands == null) {

            // Retrieve the graphic LaTeX representation
            LatexProvider lp = LatexProvider.getProvider();
            try {
                graphicCommands = lp.typesetLatexFormula(latex, size);
            }
            catch (Exception lpe) {
                try {
                    graphicCommands = lp.typesetLatexFormula(LatexProvider.DEFAULT_LATEX_ERROR, size);
                }
                catch (Exception lpe2) {
                    graphicCommands = null;
                }
            }
        }
    }
    
    public void draw(Graphics2D g, double textX, double textY, 
                     double scaleFact,  boolean doLog) 
    {
//        System.out.println("latex.draw: \""+latex+"\" textX="+textX+" textY="+textY+" scaleFact="+scaleFact);
        retrieveCommands();
        if (graphicCommands != null) {
            AffineTransform oldTransform = g.getTransform();
            g.translate(textX, textY);
            g.scale(scaleFact, scaleFact);
            graphicCommands.doDraw(g, doLog);
            g.setTransform(oldTransform);            
        }
    }

    public String getLatex() {
        return latex;
    }

    public float getSize() {
        return size;
    }
    
    public int getWidth() {
        retrieveCommands();
        if (graphicCommands != null)
            return graphicCommands.getWidth();
        return 1;
    }
    
    public int getHeight() {
        retrieveCommands();
        if (graphicCommands != null)
            return graphicCommands.getHeight();
        return 1;
    }
    
    public int getDepth() {
        retrieveCommands();
        if (graphicCommands != null)
            return graphicCommands.getDepth();
        return 1;
    }

    public int getLineHeight() {
        retrieveCommands();
        if (graphicCommands != null)
            return graphicCommands.getLineHeight();
        return 1;
    }

    public int getLineDepth() {
        retrieveCommands();
        if (graphicCommands != null)
            return graphicCommands.getLineDepth();
        return 1;
    }

    // Get the LatexFormula as an Icon (can be passed, for instance, to a JLabel)
    public Icon getAsIcon(final float scaleFactor) {
        return new Icon() {
            @Override public void paintIcon(Component comp, Graphics g, int x, int y) {
                Graphics2D g2 = (Graphics2D)g.create();
                g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                                    RenderingHints.VALUE_ANTIALIAS_ON);
                g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
                                    RenderingHints.VALUE_INTERPOLATION_BICUBIC);
                g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
                                    RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
                g2.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
                                    RenderingHints.VALUE_FRACTIONALMETRICS_ON);
                
                draw(g2, x, y, scaleFactor, false);
            }

            @Override public int getIconWidth() {
                return (int)(getWidth() * scaleFactor);
            }

            @Override public int getIconHeight() {
                return (int)(getHeight() * scaleFactor);
            }
        };
    }    
}
