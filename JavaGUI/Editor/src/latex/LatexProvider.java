/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package latex;


/**
 *
 * @author elvio
 */
public abstract class LatexProvider {
    
    public static String DEFAULT_LATEX_ERROR = "\\mbox{\\textcolor{Red}{???}}";

    // The singleton instance of the LaTeX provider
    private static LatexProvider latexProvider;

    public static LatexProvider getProvider() {
        assert latexProvider != null;
        return latexProvider;
    }
    
    public static void initialize(LatexProvider p) {
        assert latexProvider == null;
        latexProvider = p;
    }
    
    abstract MetaCommands typesetLatexFormula(String latex, float size);
    
    abstract public void quit();
}
