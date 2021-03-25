/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package latex;

import editor.domain.NetObject;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import javax.swing.JComponent;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;

/** A component that shows a LaTeX Formula
 *
 * @author elvio
 */
public class JLatexComponent extends JComponent implements Scrollable {
    private LatexFormula formula;
    
    public JLatexComponent() {
        setPreferredSize(new Dimension(40, 30));
    }

    public void setFormula(LatexFormula formula) {
        assert formula != null;
        this.formula = formula;
        // initialize latex commands, do not save them in the global cache
        formula.initializeGraphicsCommands(false);
        
        setPreferredSize(new Dimension(formula.getWidth(), formula.getHeight()));
        repaint(); // drawing has changed
        revalidate(); // area has changed
    }
    
    public LatexFormula getFormula() {
        return formula;
    }

    @Override
    public void paint(Graphics g) {
        super.paint(g); 
//        System.out.println("paint!");

        Graphics2D g2 = (Graphics2D)g;
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                            RenderingHints.VALUE_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
                            RenderingHints.VALUE_INTERPOLATION_BICUBIC);
        g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
                            RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
                            RenderingHints.VALUE_FRACTIONALMETRICS_ON);

        // Draw the background
        g.setColor(editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR);
        g.fillRect(0, 0, getWidth(), getHeight());

        if (formula != null) {
            g.setColor(Color.BLACK);
            formula.draw(g2, 0, 0, 1.0, false);
        }
    }

    @Override
    public Dimension getPreferredScrollableViewportSize() {
        return getPreferredSize();
    }

    @Override
    public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) {
        return NetObject.getUnitToPixels();
    }

    @Override
    public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction) {
        if (orientation == SwingConstants.HORIZONTAL)
            return (int)visibleRect.getWidth();
        return (int)visibleRect.getHeight();
    }

    @Override
    public boolean getScrollableTracksViewportWidth() {
        return false;
    }

    @Override
    public boolean getScrollableTracksViewportHeight() {
        return false;
    }
}
