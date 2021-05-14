/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor;

import de.rototor.pdfbox.graphics2d.PdfBoxGraphics2D;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Ellipse2D;
import java.io.IOException;
import javax.swing.JLabel;
import org.apache.pdfbox.pdmodel.PDDocument;
import org.apache.pdfbox.pdmodel.PDPage;
import org.apache.pdfbox.pdmodel.PDPageContentStream;
import org.apache.pdfbox.pdmodel.common.PDRectangle;
import org.apache.pdfbox.pdmodel.graphics.form.PDFormXObject;
import org.apache.pdfbox.util.Matrix;
import org.scilab.forge.jlatexmath.Box;
import org.scilab.forge.jlatexmath.CharAtom;
import org.scilab.forge.jlatexmath.DefaultTeXFont;
import org.scilab.forge.jlatexmath.TeXConstants;
import org.scilab.forge.jlatexmath.TeXEnvironment;
import org.scilab.forge.jlatexmath.TeXFont;
import org.scilab.forge.jlatexmath.TeXFormula;
import org.scilab.forge.jlatexmath.TeXIcon;

/**
 *
 * @author elvio
 */
public class TestPDFBox {
    
    public static void writeLaTeXformula(Graphics2D g2, String latex, float size) {
        TeXFormula.externalFontMap.remove(Character.UnicodeBlock.BASIC_LATIN);
        TeXFormula formula = new TeXFormula(latex);
        
        int style = TeXConstants.STYLE_DISPLAY;
        TeXIcon icon = formula.createTeXIcon(style, size);
        System.out.println("baseLine = "+icon.getBaseLine());
        System.out.println("box = "+icon.getBox());
        System.out.println("iconWidth = "+icon.getIconWidth());
        System.out.println("iconHeight = "+icon.getIconHeight());
        System.out.println("iconDepth = "+icon.getIconDepth());
        System.out.println("trueIconWidth = "+icon.getTrueIconWidth());
        System.out.println("trueIconHeight = "+icon.getTrueIconHeight());
        System.out.println("trueIconDepth = "+icon.getTrueIconDepth());
        System.out.println("insets = "+icon.getInsets());
        System.out.println("isColored = "+icon.isColored);
        
        TeXFont texFont = new DefaultTeXFont(size);
        TeXEnvironment env = new TeXEnvironment(style, texFont);
        CharAtom atom = new CharAtom('M', "mathnormal");
        Box box = atom.createBox(env);
        System.out.println("EM = "+box.getHeight() * size);
//        System.out.println("EM = "+ SpaceAtom.getFactor(TeXConstants.UNIT_EM, env));
        System.out.println("Axis height = "+texFont.getAxisHeight(style) * size);
        System.out.println("scaleFactor = "+texFont.getScaleFactor());
        System.out.println("Xheight = "+texFont.getXHeight(env.getStyle(), env.getLastFontId()) * size);
        
        TeXFormula formula2 = new TeXFormula("A_0");
        TeXIcon icon2 = formula2.createTeXIcon(TeXConstants.STYLE_DISPLAY, size);
        System.out.println("expected height = "+icon2.getIconHeight());

        g2.setColor(Color.white);
        g2.fillRect(0, 0, icon.getIconWidth(), icon.getIconHeight());

        JLabel jl = new JLabel();
        jl.setForeground(new Color(0, 0, 0));
        icon.paintIcon(jl, g2, 0, 0);
        
        // Draw the baseline
        g2.setColor(Color.red);
        int y = icon.getIconHeight() - icon.getIconDepth();
        g2.fillRect(0, y, icon.getIconWidth(), 1);
    }
    
    public static void main(String[] args) throws IOException {
        PDDocument document = new PDDocument();
        
        float xBorder = 10, yBorder = 10;
        float pageWidth = 400, pageHeight = 200;
        PDRectangle pageRect = new PDRectangle(pageWidth + 2*xBorder, pageHeight + 2*yBorder);
        PDPage page = new PDPage(pageRect);
        document.addPage(page);
        
        PdfBoxGraphics2D pdfBoxGraphics2D = new PdfBoxGraphics2D(document, pageWidth, pageHeight);
        
        Graphics2D g2d = pdfBoxGraphics2D;
        
        Ellipse2D.Double circle =
                new Ellipse2D.Double(0, 0, 400, 200);
        Color oldColour = g2d.getColor();

        g2d.setColor(Color.RED);
        g2d.fill(circle);
        g2d.setColor(Color.BLACK);
        g2d.draw(circle);

        
        String latex = "\\mathrm{If}\\left[34 \\geq \\sqrt{3.14 \\cdot 67}, 12, 34 + \\left\\lceil \\binom{3}{2} \\right\\rceil\\right] +\n" + "\\mathrm{Round}\\left[ \\frac{{\\left({3 + 2}\\right)}^{5}}{5 \\cdot 6!} \\right]\n";
        writeLaTeXformula(g2d, latex, 12);
        
        
        pdfBoxGraphics2D.dispose();
        PDFormXObject xform = pdfBoxGraphics2D.getXFormObject();

        // Build a matrix to place the form
        Matrix matrix = new Matrix();
        // Note: As PDF coordinates start at the bottom left corner, we move up from there.
        matrix.translate(xBorder, yBorder);
        PDPageContentStream contentStream = new PDPageContentStream(document, page);
        contentStream.transform(matrix);

        // Now finally draw the form. As we not do any scaling, the form drawn has a size of 5,5 x 5,5 inches,
        // because PDF uses 72 DPI for its lengths by default. If you want to scale, skew or rotate the form you can
        // of course do this. And you can also draw the form more then once. Think of the XForm as a stamper.
        contentStream.drawForm(xform);

        contentStream.close();

        document.save("prova.pdf");
        document.close();
    }
}
