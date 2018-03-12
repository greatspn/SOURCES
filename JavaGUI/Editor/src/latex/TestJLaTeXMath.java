/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package latex;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;
import javax.swing.JLabel;
import org.scilab.forge.jlatexmath.Box;
import org.scilab.forge.jlatexmath.CharAtom;
import org.scilab.forge.jlatexmath.DefaultTeXFont;
import org.scilab.forge.jlatexmath.SpaceAtom;
import org.scilab.forge.jlatexmath.TeXConstants;
import org.scilab.forge.jlatexmath.TeXEnvironment;
import org.scilab.forge.jlatexmath.TeXFont;
import org.scilab.forge.jlatexmath.TeXFormula;
import org.scilab.forge.jlatexmath.TeXIcon;
import org.scilab.forge.jlatexmath.UnderscoreAtom;

/**
 *
 * @author elvio
 */
public class TestJLaTeXMath {

    public static void writeLatexFormulaAsImage(String latex, String filename, float size)
            throws IOException, ClassNotFoundException 
    {
        System.out.println("\nwriteLatexFormulaAsImage \""+latex+"\"  as "+filename);
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

        BufferedImage image = new BufferedImage(icon.getIconWidth(), icon.getIconHeight(), BufferedImage.TYPE_INT_ARGB);
        Graphics2D g2 = image.createGraphics();
        g2.setColor(Color.white);
        g2.fillRect(0, 0, icon.getIconWidth(), icon.getIconHeight());

        JLabel jl = new JLabel();
        jl.setForeground(new Color(0, 0, 0));
        icon.paintIcon(jl, g2, 0, 0);
        
        // Draw the baseline
        g2.setColor(Color.red);
        int y = icon.getIconHeight() - icon.getIconDepth();
        g2.fillRect(0, y, icon.getIconWidth(), 1);

        File file = new File(filename);
        ImageIO.write(image, "png", file.getAbsoluteFile());
    }

    public static void main(String[] args) {
//        String latex1 = "\\mathrm{If}\\left[34 \\geq \\sqrt{3.14 \\cdot 67}, 12, 34 + "
//                + "\\left\\lceil \\binom{3}{2} \\right\\rceil\\right] + \\mathrm{Round}"
//                + "\\left[ \\frac{{\\left({3 + 2}\\right)}^{5}}{5 \\cdot 6!} \\right]\n";
//        String latex1 = "\\left\\lceil \\sin{3} + \\cos{\\left({3 \\cdot 5.6}\\right)} + "
//                + "{\\left({\\arctan{\\left({3.14 \\cdot 78}\\right)}}\\right)}^{4} \\right\\rceil\n";
//        String latex1 = "L = \\int_a^b \\sqrt{ \\left|\\sum_{i,j=1}^ng_{ij}(\\gamma(t))"
//                + "\\left(\\frac{d}{dt}x^i\\circ\\gamma(t)\\right)\\left(\\frac{d}{dt}x^j\\circ"
//                + "\\gamma(t)\\right)\\right|}\\,dt\\\\";
//        String latex1 = "\\frac{\\mathrm{N}_{0}}{2} + 2";
//        String latex1 = "A_0=\\left(\\begin{array}{rl}ax & b \\\\ c & dy\\end{array}\\right)\\cdot p \\text{STAT}";
//        String latex1 = "\\mathrm{P}_{0}\\mathrm{M}_{0}\\mathrm{K}_{4}"
//                + "\\hspace{1em} \\mathrm{p}_{1}\\hspace{1pt}\\mathrm{m}_{2}\\hspace{1pt}\\mathrm{k}_{3}"
//                + "\\hspace{1em} \\mathrm{p}_{1}\\mathrm{m}_{2}\\mathrm{k}_{3}";
//        String latex1 = "\\mathit{P}_{0}\\mathit{M}_{0}\\mathit{K}_{4}"
//                + "\\hspace{1em} \\mathit{p}_{1}\\hspace{1pt}\\mathit{m}_{2}\\hspace{1pt}\\mathit{k}_{3}"
//                + "\\hspace{1em} \\mathit{p}_{1}\\mathit{m}_{2}\\mathit{k}_{3}";
        UnderscoreAtom.w = new SpaceAtom(TeXConstants.UNIT_EM, 0.4f, 0, 0);
        UnderscoreAtom.s = new SpaceAtom(TeXConstants.UNIT_EM, 0.06f, 0, 0);

        String latex1 = "34\\_56\\underscore 56";
        try {
            writeLatexFormulaAsImage(latex1, "jLatexMathTest_00.png", 16.0f);
//            writeLatexFormulaAsImage("prova_0", "ExampleImg1.png", 100);
//            writeLatexFormulaAsImage("Prova_0", "ExampleImg2.png", 100);
//            writeLatexFormulaAsImage("x = \\frac{1}{N}", "ExampleImg3.png", 100);
//            writeLatexFormulaAsImage("abc \\\\ def", "ExampleImg4.png", 100);
//            writeLatexFormulaAsImage("\\left(x = \\frac{1}{N}\\right)", "ExampleImg5.png", 100);
        } catch (Exception ex) { ex.printStackTrace(); }
    }
}
   
