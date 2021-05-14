/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package latex;

import common.Util;
import de.rototor.pdfbox.graphics2d.PdfBoxGraphics2D;
import editor.domain.NetObject;
import editor.domain.io.DtaFormat;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.io.File;
import java.io.IOException;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.pdmodel.PDPage;
import org.apache.pdfbox.pdmodel.PDPageContentStream;
import org.apache.pdfbox.pdmodel.common.PDRectangle;
import org.apache.pdfbox.pdmodel.graphics.form.PDFormXObject;
import org.apache.pdfbox.util.Matrix;

/** A component that shows a LaTeX Formula
 *
 * @author elvio
 */
public class JLatexComponent extends JComponent implements Scrollable {
    private LatexFormula formula;
    
    public JLatexComponent() {
        setPreferredSize(new Dimension(10, 10));
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
    
    // Copy the current latex formula to the clipboard
    public void copyToClipboard() {
        if (formula != null) {
            StringSelection stringSelection = new StringSelection(formula.getLatex());
            Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
            clipboard.setContents(stringSelection, null);
        }
    }
    
    // Save as pdf
    public void saveAsPdf(String proposedName) {
        if (formula == null)
            return;
        
        File pdfFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export LaTeX in PDF format...");
            String curDir = Util.getPreferences().get("pdf-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+proposedName+".pdf") : null);
            fileChooser.addChoosableFileFilter(DtaFormat.fileFilter);
            fileChooser.setFileFilter(DtaFormat.fileFilter);
            if (fileChooser.showSaveDialog(getRootPane()) != JFileChooser.APPROVE_OPTION)
                return;
            pdfFile = fileChooser.getSelectedFile();
            
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("pdf-export-dir", curDir);
            if (pdfFile.exists()) {
                int r = JOptionPane.showConfirmDialog(getRootPane(),
                            "The file \"" + pdfFile + "\" already exists! Overwrite it?",
                            "Overwrite file",
                            JOptionPane.YES_NO_CANCEL_OPTION,
                            JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            PDDocument document = new PDDocument();
        
            float xBorder = 0, yBorder = 0;
            float pdfScaleFactor = 1.0f;
            float width = (float)formula.getWidth() * pdfScaleFactor;
            float height = (float)formula.getHeight() * pdfScaleFactor;
//            System.out.println("width="+width+" height="+height);
            PDRectangle pageRect = new PDRectangle(width + 2 * xBorder, height + 2 * yBorder);
            PDPage pdfPage = new PDPage(pageRect);
            document.addPage(pdfPage);

            PdfBoxGraphics2D pdfBoxGraphics2D = new PdfBoxGraphics2D(document, width, height);
            pdfBoxGraphics2D.setColor(Color.BLACK);
            formula.draw(pdfBoxGraphics2D, 0, 0, 1, false);

            pdfBoxGraphics2D.dispose();
            PDFormXObject xform = pdfBoxGraphics2D.getXFormObject();

            // Build a matrix to place the form
            Matrix matrix = new Matrix();
            // Note: As PDF coordinates start at the bottom left corner, we move up from there.
            matrix.translate(xBorder, yBorder);
            PDPageContentStream contentStream = new PDPageContentStream(document, pdfPage);
            contentStream.transform(matrix);

            // Now finally draw the form. As we not do any scaling, the form drawn has a size of 5,5 x 5,5 inches,
            // because PDF uses 72 DPI for its lengths by default. If you want to scale, skew or rotate the form you can
            // of course do this. And you can also draw the form more then once. Think of the XForm as a stamper.
            contentStream.drawForm(xform);

            contentStream.close();

            document.save(pdfFile);
            document.close();
            
//            mainInterface.setStatus("PDF exported.", true);
        } 
        catch (IOException e) {
            JOptionPane.showMessageDialog(getRootPane(), 
                                          "An error happened while exporting the page in PDF format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export LaTeX in PDF format...", 
                                          JOptionPane.ERROR_MESSAGE);            
//            mainInterface.setStatus("could not export in PDF format.", true);
        }
    }
}
