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
        String[] latexStrings = {
        "\\mathrm{If}\\left[34 \\geq \\sqrt{3.14 \\cdot 67}, 12, 34 + "
                + "\\left\\lceil \\binom{3}{2} \\right\\rceil\\right] + \\mathrm{Round}"
                + "\\left[ \\frac{{\\left({3 + 2}\\right)}^{5}}{5 \\cdot 6!} \\right]\n",
        "\\left\\lceil \\sin{3} + \\cos{\\left({3 \\cdot 5.6}\\right)} + "
                + "{\\left({\\arctan{\\left({3.14 \\cdot 78}\\right)}}\\right)}^{4} \\right\\rceil\n",
        "L = \\int_a^b \\sqrt{ \\left|\\sum_{i,j=1}^ng_{ij}(\\gamma(t))"
                + "\\left(\\frac{d}{dt}x^i\\circ\\gamma(t)\\right)\\left(\\frac{d}{dt}x^j\\circ"
                + "\\gamma(t)\\right)\\right|}\\,dt\\\\",
        "\\frac{\\mathrm{N}_{0}}{2} + 2",
        "A_0=\\left(\\begin{array}{rl}ax & b \\\\ c & dy\\end{array}\\right)\\cdot p \\text{STAT}",
        "\\mathrm{P}_{0}\\mathrm{M}_{0}\\mathrm{K}_{4}"
                + "\\hspace{1em} \\mathrm{p}_{1}\\hspace{1pt}\\mathrm{m}_{2}\\hspace{1pt}\\mathrm{k}_{3}"
                + "\\hspace{1em} \\mathrm{p}_{1}\\mathrm{m}_{2}\\mathrm{k}_{3}",
        "\\mathit{P}_{0}\\mathit{M}_{0}\\mathit{K}_{4}"
                + "\\hspace{1em} \\mathit{p}_{1}\\hspace{1pt}\\mathit{m}_{2}\\hspace{1pt}\\mathit{k}_{3}"
                + "\\hspace{1em} \\mathit{p}_{1}\\mathit{m}_{2}\\mathit{k}_{3}",
        "34\\_56\\underscore 56",
//        latex1, 
        latex2, 
        latex3, 
        latex4
        };
        
        UnderscoreAtom.w = new SpaceAtom(TeXConstants.UNIT_EM, 0.4f, 0, 0);
        UnderscoreAtom.s = new SpaceAtom(TeXConstants.UNIT_EM, 0.06f, 0, 0);

        try {
            for (int i=0; i<latexStrings.length; i++) {
                writeLatexFormulaAsImage(latexStrings[i], "jLatexMathTest_"+i+".png", 32.0f);
            }
//            writeLatexFormulaAsImage("prova_0", "ExampleImg1.png", 100);
//            writeLatexFormulaAsImage("Prova_0", "ExampleImg2.png", 100);
//            writeLatexFormulaAsImage("x = \\frac{1}{N}", "ExampleImg3.png", 100);
//            writeLatexFormulaAsImage("abc \\\\ def", "ExampleImg4.png", 100);
//            writeLatexFormulaAsImage("\\left(x = \\frac{1}{N}\\right)", "ExampleImg5.png", 100);
        } catch (Exception ex) { ex.printStackTrace(); }
    }
    
//    private static String latex1 = "\\begin{array}{lr}\\mbox{\\textcolor{Blue}{Russian}}&\\mbox{\\textcolor{Melon}{Greek}}\\\\" +
//         "\\mbox{" + "привет мир".toUpperCase() + "}&\\mbox{" + "γειά κόσμο".toUpperCase() + "}\\\\" +
//         "\\mbox{привет мир}&\\mbox{γειά κόσμο}\\\\" +
//         "\\mathbf{\\mbox{привет мир}}&\\mathbf{\\mbox{γειά κόσμο}}\\\\" +
//         "\\mathit{\\mbox{привет мир}}&\\mathit{\\mbox{γειά κόσμο}}\\\\" +
//         "\\mathsf{\\mbox{привет мир}}&\\mathsf{\\mbox{γειά κόσμο}}\\\\" +
//         "\\mathtt{\\mbox{привет мир}}&\\mathtt{\\mbox{γειά κόσμο}}\\\\" +
//         "\\mathbf{\\mathit{\\mbox{привет мир}}}&\\mathbf{\\mathit{\\mbox{γειά κόσμο}}}\\\\" +
//         "\\mathbf{\\mathsf{\\mbox{привет мир}}}&\\mathbf{\\mathsf{\\mbox{γειά κόσμο}}}\\\\" +
//         "\\mathsf{\\mathit{\\mbox{привет мир}}}&\\mathsf{\\mathit{\\mbox{γειά κόσμο}}}\\\\" +
//         "&\\\\" +
//         "\\mbox{\\textcolor{Salmon}{Bulgarian}}&\\mbox{\\textcolor{Tan}{Serbian}}\\\\" +
//         "\\mbox{здравей свят}&\\mbox{Хелло уорлд}\\\\" +
//         "&\\\\" +
//         "\\mbox{\\textcolor{Turquoise}{Bielorussian}}&\\mbox{\\textcolor{LimeGreen}{Ukrainian}}\\\\" +
//         "\\mbox{прывітаньне Свет}&\\mbox{привіт світ}\\\\" +
//         "\\end{array}";
    private static String latex2 = "\\begin{array}{l}" +
         "\\forall\\varepsilon\\in\\mathbb{R}_+^*\\ \\exists\\eta>0\\ |x-x_0|\\leq\\eta\\Longrightarrow|f(x)-f(x_0)|\\leq\\varepsilon\\\\" +
         "\\det\\begin{bmatrix}a_{11}&a_{12}&\\cdots&a_{1n}\\\\a_{21}&\\ddots&&\\vdots\\\\\\vdots&&\\ddots&\\vdots\\\\a_{n1}&\\cdots&\\cdots&a_{nn}\\end{bmatrix}\\overset{\\mathrm{def}}{=}\\sum_{\\sigma\\in\\mathfrak{S}_n}\\varepsilon(\\sigma)\\prod_{k=1}^n a_{k\\sigma(k)}\\\\" +
         "\\sideset{_\\alpha^\\beta}{_\\gamma^\\delta}{\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}}\\\\" +
         "\\int_0^\\infty{x^{2n} e^{-a x^2}\\,dx} = \\frac{2n-1}{2a} \\int_0^\\infty{x^{2(n-1)} e^{-a x^2}\\,dx} = \\frac{(2n-1)!!}{2^{n+1}} \\sqrt{\\frac{\\pi}{a^{2n+1}}}\\\\" +
         "\\int_a^b{f(x)\\,dx} = (b - a) \\sum\\limits_{n = 1}^\\infty  {\\sum\\limits_{m = 1}^{2^n  - 1} {\\left( { - 1} \\right)^{m + 1} } } 2^{ - n} f(a + m\\left( {b - a} \\right)2^{-n} )\\\\" +
         "\\int_{-\\pi}^{\\pi} \\sin(\\alpha x) \\sin^n(\\beta x) dx = \\textstyle{\\left \\{ \\begin{array}{cc} (-1)^{(n+1)/2} (-1)^m \\frac{2 \\pi}{2^n} \\binom{n}{m} & n \\mbox{ odd},\\ \\alpha = \\beta (2m-n) \\\\ 0 & \\mbox{otherwise} \\\\ \\end{array} \\right .}\\\\" +
         "L = \\int_a^b \\sqrt{ \\left|\\sum_{i,j=1}^ng_{ij}(\\gamma(t))\\left(\\frac{d}{dt}x^i\\circ\\gamma(t)\\right)\\left(\\frac{d}{dt}x^j\\circ\\gamma(t)\\right)\\right|}\\,dt\\\\" +
         "\\begin{array}{rl} s &= \\int_a^b\\left\\|\\frac{d}{dt}\\vec{r}\\,(u(t),v(t))\\right\\|\\,dt \\\\ &= \\int_a^b \\sqrt{u'(t)^2\\,\\vec{r}_u\\cdot\\vec{r}_u + 2u'(t)v'(t)\\, \\vec{r}_u\\cdot\\vec{r}_v+ v'(t)^2\\,\\vec{r}_v\\cdot\\vec{r}_v}\\,\\,\\, dt. \\end{array}\\\\" +
         "\\end{array}";
//    private static String latex3 = "\\definecolor{gris}{gray}{0.9}" +
//         "\\definecolor{noir}{rgb}{0,0,0}" +
//         "\\definecolor{bleu}{rgb}{0,0,1}\\newcommand{\\pa}{\\left|}" +
//         "\\begin{array}{c}" +
//         "\\JLaTeXMath\\\\" +
//         "\\begin{split}" +
//         " &Тепловой\\ поток\\ \\mathrm{Тепловой\\ поток}\\ \\mathtt{Тепловой\\ поток}\\\\" +
//         " &\\boldsymbol{\\mathrm{Тепловой\\ поток}}\\ \\mathsf{Тепловой\\ поток}\\\\" +
//         "|I_2| &= \\pa\\int_0^T\\psi(t)\\left\\{ u(a,t)-\\int_{\\gamma(t)}^a \\frac{d\\theta}{k} (\\theta,t) \\int_a^\\theta c(\\xi) u_t (\\xi,t)\\,d\\xi\\right\\}dt\\right|\\\\" +
//         "&\\le C_6 \\Bigg|\\pa f \\int_\\Omega \\pa\\widetilde{S}^{-1,0}_{a,-} W_2(\\Omega, \\Gamma_1)\\right|\\ \\right|\\left| |u|\\overset{\\circ}{\\to} W_2^{\\widetilde{A}}(\\Omega;\\Gamma_r,T)\\right|\\Bigg|\\\\" +
//         "&\\\\" +
//         "&\\textcolor{magenta}{\\mathrm{Produit\\ avec\\ Java\\ et\\ \\LaTeX\\ par\\ }\\mathscr{C}\\mathcal{A}\\mathfrak{L}\\mathbf{I}\\mathtt{X}\\mathbb{T}\\mathsf{E}}\\\\" +
//         "&\\begin{pmatrix}\\alpha&\\beta&\\gamma&\\delta\\\\\\aleph&\\beth&\\gimel&\\daleth\\\\\\mathfrak{A}&\\mathfrak{B}&\\mathfrak{C}&\\mathfrak{D}\\\\\\boldsymbol{\\mathfrak{a}}&\\boldsymbol{\\mathfrak{b}}&\\boldsymbol{\\mathfrak{c}}&\\boldsymbol{\\mathfrak{d}}\\end{pmatrix}\\quad{(a+b)}^{\\frac{n}{2}}=\\sqrt{\\sum_{k=0}^n\\tbinom{n}{k}a^kb^{n-k}}\\quad \\Biggl(\\biggl(\\Bigl(\\bigl(()\\bigr)\\Bigr)\\biggr)\\Biggr)\\\\" +
//         "&\\forall\\varepsilon\\in\\mathbb{R}_+^*\\ \\exists\\eta>0\\ |x-x_0|\\leq\\eta\\Longrightarrow|f(x)-f(x_0)|\\leq\\varepsilon\\\\" +
//         "&\\det\\begin{bmatrix}a_{11}&a_{12}&\\cdots&a_{1n}\\\\a_{21}&\\ddots&&\\vdots\\\\\\vdots&&\\ddots&\\vdots\\\\a_{n1}&\\cdots&\\cdots&a_{nn}\\end{bmatrix}\\overset{\\mathrm{def}}{=}\\sum_{\\sigma\\in\\mathfrak{S}_n}\\varepsilon(\\sigma)\\prod_{k=1}^n a_{k\\sigma(k)}\\\\" +
//         "&\\Delta f(x,y)=\\frac{\\partial^2f}{\\partial x^2}+\\frac{\\partial^2f}{\\partial y^2}\\qquad\\qquad \\fcolorbox{noir}{gris}{n!\\underset{n\\rightarrow+\\infty}{\\sim} {\\left(\\frac{n}{e}\\right)}^n\\sqrt{2\\pi n}}\\\\" +
//         "&\\sideset{_\\alpha^\\beta}{_\\gamma^\\delta}{\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}}\\xrightarrow[T]{n\\pm i-j}\\sideset{^t}{}A\\xleftarrow{\\overrightarrow{u}\\wedge\\overrightarrow{v}}\\underleftrightarrow{\\iint_{\\mathds{R}^2}e^{-\\left(x^2+y^2\\right)}\\,\\mathrm{d}x\\mathrm{d}y}" +
//         "\\end{split}\\\\" +
//         "\\rotatebox{30}{\\sum_{n=1}^{+\\infty}}\\quad\\mbox{Mirror rorriM}\\reflectbox{\\mbox{Mirror rorriM}}" +
//         "\\end{array}";
    private static String latex3 = "\\definecolor{gris}{gray}{0.9}" +
         "\\definecolor{noir}{rgb}{0,0,0}" +
         "\\definecolor{bleu}{rgb}{0,0,1}\\newcommand{\\pa}{\\left|}" +
         "\\begin{array}{c}" +
         "\\JLaTeXMath\\\\" +
         "\\begin{split}" +
         " &Xxxxxx\\ Yyyyyyy\\ \\mathrm{Zzzzzzz\\ Wwwww}\\ \\mathtt{Cccccc\\ ccccc}\\\\" +
         " &\\boldsymbol{\\mathrm{Ddddddd\\ Ffffff}}\\ \\mathsf{Gggggg\\ Hhhhhh}\\\\" +
         "|I_2| &= \\pa\\int_0^T\\psi(t)\\left\\{ u(a,t)-\\int_{\\gamma(t)}^a \\frac{d\\theta}{k} (\\theta,t) \\int_a^\\theta c(\\xi) u_t (\\xi,t)\\,d\\xi\\right\\}dt\\right|\\\\" +
         "&\\le C_6 \\Bigg|\\pa f \\int_\\Omega \\pa\\widetilde{S}^{-1,0}_{a,-} W_2(\\Omega, \\Gamma_1)\\right|\\ \\right|\\left| |u|\\overset{\\circ}{\\to} W_2^{\\widetilde{A}}(\\Omega;\\Gamma_r,T)\\right|\\Bigg|\\\\" +
         "&\\\\" +
         "&\\textcolor{magenta}{\\mathrm{Produit\\ avec\\ Java\\ et\\ \\LaTeX\\ par\\ }\\mathscr{C}\\mathcal{A}\\mathfrak{L}\\mathbf{I}\\mathtt{X}\\mathbb{T}\\mathsf{E}}\\\\" +
         "&\\begin{pmatrix}\\alpha&\\beta&\\gamma&\\delta\\\\\\aleph&\\beth&\\gimel&\\daleth\\\\\\mathfrak{A}&\\mathfrak{B}&\\mathfrak{C}&\\mathfrak{D}\\\\\\boldsymbol{\\mathfrak{a}}&\\boldsymbol{\\mathfrak{b}}&\\boldsymbol{\\mathfrak{c}}&\\boldsymbol{\\mathfrak{d}}\\end{pmatrix}\\quad{(a+b)}^{\\frac{n}{2}}=\\sqrt{\\sum_{k=0}^n\\tbinom{n}{k}a^kb^{n-k}}\\quad \\Biggl(\\biggl(\\Bigl(\\bigl(()\\bigr)\\Bigr)\\biggr)\\Biggr)\\\\" +
         "&\\forall\\varepsilon\\in\\mathbb{R}_+^*\\ \\exists\\eta>0\\ |x-x_0|\\leq\\eta\\Longrightarrow|f(x)-f(x_0)|\\leq\\varepsilon\\\\" +
         "&\\det\\begin{bmatrix}a_{11}&a_{12}&\\cdots&a_{1n}\\\\a_{21}&\\ddots&&\\vdots\\\\\\vdots&&\\ddots&\\vdots\\\\a_{n1}&\\cdots&\\cdots&a_{nn}\\end{bmatrix}\\overset{\\mathrm{def}}{=}\\sum_{\\sigma\\in\\mathfrak{S}_n}\\varepsilon(\\sigma)\\prod_{k=1}^n a_{k\\sigma(k)}\\\\" +
         "&\\Delta f(x,y)=\\frac{\\partial^2f}{\\partial x^2}+\\frac{\\partial^2f}{\\partial y^2}\\qquad\\qquad \\fcolorbox{noir}{gris}{n!\\underset{n\\rightarrow+\\infty}{\\sim} {\\left(\\frac{n}{e}\\right)}^n\\sqrt{2\\pi n}}\\\\" +
         "&\\sideset{_\\alpha^\\beta}{_\\gamma^\\delta}{\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}}\\xrightarrow[T]{n\\pm i-j}\\sideset{^t}{}A\\xleftarrow{\\overrightarrow{u}\\wedge\\overrightarrow{v}}\\underleftrightarrow{\\iint_{\\mathds{R}^2}e^{-\\left(x^2+y^2\\right)}\\,\\mathrm{d}x\\mathrm{d}y}" +
         "\\end{split}\\\\" +
         "\\rotatebox{30}{\\sum_{n=1}^{+\\infty}}\\quad\\mbox{Mirror rorriM}\\reflectbox{\\mbox{Mirror rorriM}}" +
         "\\end{array}";
    private static String latex4 = "\\begin{array}{|c|l|||r|c|}" +
         "\\hline" +
         "\\text{Matrix}&\\multicolumn{2}{|c|}{\\text{Multicolumns}}&\\text{Font sizes commands}\\cr" +
         "\\hline" +
         "\\begin{pmatrix}\\alpha_{11}&\\cdots&\\alpha_{1n}\\cr\\hdotsfor{3}\\cr\\alpha_{n1}&\\cdots&\\alpha_{nn}\\end{pmatrix}&\\Large \\text{Large Right}&\\small \\text{small Left}&\\tiny \\text{tiny Tiny}\\cr" +
         "\\hline" +
         "\\multicolumn{4}{|c|}{\\Huge \\text{Huge Multicolumns}}\\cr" +
         "\\hline" +
         "\\end{array}";    
}
   
