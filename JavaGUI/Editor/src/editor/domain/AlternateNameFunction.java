/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import static editor.domain.NetObject.NUM_STYLES;
import static editor.domain.NetObject.STYLE_BOLD;
import static editor.domain.NetObject.STYLE_BOLD_ITALIC;
import static editor.domain.NetObject.STYLE_ITALIC;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/** This class decides which alternate name should be applied to a node,
 * like a plain name, a LaTeX label or a special formatting function.
 *
 * @author elvio
 */
public enum AlternateNameFunction implements Serializable {
    NUMBERS_AS_SUBSCRIPTS("Default", "default", false),
    PLAIN("Plain ID", "plain", false),
    LATEX_TEXT("Custom LaTeX", "latex", true);
    
    // Name fo the function, as shown in the combo box of the editor panel
    public final String nameOfFunction;
    // Name fo the function, as saved in the PNPRO file format
    public final String xmlName;
    // Does this function require an alternate text label?
    public final boolean requiresAlternateText;

    private AlternateNameFunction(String nameOfFunction, String xmlName, boolean requiresAlternateText) {
        this.nameOfFunction = nameOfFunction;
        this.xmlName = xmlName;
        this.requiresAlternateText = requiresAlternateText;
    }
    
//    public static AlternateNameFunction fromXmlName(String fnName) {
//        if (fnName == null)
//            return NUMBERS_AS_SUBSCRIPTS; // Default
//        for (AlternateNameFunction anf : values())
//            if (anf.xmlName.equals(fnName))
//                return anf;
//        throw new IllegalStateException("Invalid AlternateNameFunction.");
//    }

    @Override
    public String toString() {
        return nameOfFunction;
    }
    
    public String prepareLatexText(String id, String altText, int style) {
        switch (this) {
            case PLAIN:
                return prepareId_plain(id, style);
                
            case NUMBERS_AS_SUBSCRIPTS:
                return prepareId_NumberAsSubscript(id, style);
                
            case LATEX_TEXT:
                assert altText != null;
                return altText;
                
            default:
                throw new IllegalStateException();
        }
    }
    
    //--------------------------------------------------------------------------
    // Methods that actually formats the id/altName to the final LaTeX string
    //--------------------------------------------------------------------------
    
    public static final Set<String> SPECIAL_LATEX_NAMES;
    static {
        HashSet<String> sln = new HashSet<>();
        final String latexNames[] = {
            "Alpha", "alpha", "Beta", "beta", "Gamma", "gamma", "Delta", "delta", 
            "Epsilon", "epsilon", "varepsilon", "Zeta", "zeta", "Eta", "eta", 
            "Theta", "theta", "vartheta", "Iota", "iota", "Kappa", "kappa", "varkappa", 
            "Lambda", "lambda", "Mu", "mu", "Nu", "nu", "Xi", "xi", "Omicron", "omicron", 
            "Pi", "pi", "varpi", "Rho", "rho", "varrho", "Sigma", "sigma", "varsigma", 
            "Tau", "tau", "Upsilon", "upsilon", "Phi", "phi", "varphi", "Chi", "chi", 
            "Psi", "psi", "Omega", "omega"
        };
        sln.addAll(Arrays.asList(latexNames));
        SPECIAL_LATEX_NAMES = sln;
    }
    
    private static final ArrayList<Map<String, String>> numbersAsSubscriptsStringTabs;
    private static final ArrayList<Map<String, String>> plainStringTabs;
    
    static {
        numbersAsSubscriptsStringTabs = new ArrayList<>();
        plainStringTabs = new ArrayList<>();
        for (int i=0; i<NUM_STYLES; i++) {
            numbersAsSubscriptsStringTabs.add(new HashMap<String, String>());
            plainStringTabs.add(new HashMap<String, String>());
        }
    }

    private static String getLatexTextModeOpen(int style) {
        switch (style) {
            case STYLE_ITALIC:
                return "\\mathit{";
            case STYLE_BOLD:
                return "\\mathbf{";
            case STYLE_BOLD_ITALIC:
                return "\\mathbf{\\mathit{";
            default: // STYLE_ROMAN
                return "\\mathrm{";
        }
    }
    
    private static String getLatexTextModeClose(int style) {
        switch (style) {
            case STYLE_BOLD_ITALIC:
                return "}}";
            default: // STYLE_ROMAN, STYLE_ITALIC, STYLE_BOLD
                return "}";
        }
    }

    private static String format_id(String id, int style, boolean numberAsSubscripts) {
        // Special rules for +/* operators in names
        id = id.replace("_plus_", "+");
        id = id.replace("_times_", "*");
        StringBuilder builder = new StringBuilder();
        int pos = 0;
        boolean prevIsAlpha = false;
        while (pos < id.length()) {
            int to = pos;
            boolean isNumber = Character.isDigit(id.charAt(to));
            boolean isAlpha = Character.isAlphabetic(id.charAt(to));
            
            if (isNumber) {
                while (to < id.length() && Character.isDigit(id.charAt(to)))
                    to++;
            }
            else if (isAlpha) {
                while (to < id.length() && Character.isAlphabetic(id.charAt(to)))
                    to++;                
            }
            else { // symbols
                to++;
            }
            
            if (isNumber) {
                builder.append((numberAsSubscripts && prevIsAlpha) ? "_{": "{");
                builder.append(id, pos, to);
                builder.append("}");
            }
            else if (isAlpha) {
                boolean isBuilderEmpty = (builder.length() == 0);
                String text = id.substring(pos, to);
                boolean isSpecialName = numberAsSubscripts && SPECIAL_LATEX_NAMES.contains(text);
                
                if (!isSpecialName && !isBuilderEmpty) {
                    char firstCh = text.charAt(0);
                    if (firstCh >= 'a' && firstCh <= 'z' && 0==(style & STYLE_ITALIC))
                        builder.append("\\hspace{1pt}");
                }
                builder.append(getLatexTextModeOpen(style));
                if (isSpecialName) {
                    builder.append("\\"); // This is a special latex name
                    builder.append(text);
                }
                else {
                    for (int i=pos; i<to; i++)
                        if (id.charAt(i) == '_')
                            builder.append("\\_");
                        else
                            builder.append(id.charAt(i));
                }
                builder.append(getLatexTextModeClose(style));
                
                char lastCh = text.charAt(text.length()-1);
                if (!isSpecialName && lastCh >= 'a' && lastCh <= 'z' && 0!=(style & STYLE_ITALIC))
                    builder.append("\\hspace{1pt}"); // Correct a spacing imperfection made by jLaTeXMath
            }
            else { // symbols
                char sym = id.charAt(pos);
                switch (sym) {
                    case '<':
                        builder.append("\\langle");
                        break;
                        
                    case '>':
                        builder.append("\\rangle");
                        break;
                        
                    case '_':
                        builder.append("\\_");
                        break;
                        
                    case '*':
                        builder.append("{\\cdot}");
                        break;
                        
                    case '+':
                        builder.append("{+}");
                        break;
                        
                    default:
                        builder.append(sym);
                        break;
                }
            }
            
            pos = to;
            prevIsAlpha = isAlpha;
        }
        return builder.toString();
    }
    
    private static String prepareId_NumberAsSubscript(String id, int style) {
        String vis = numbersAsSubscriptsStringTabs.get(style).get(id);
        if (vis != null)
            return vis;
        vis = format_id(id, style, true);
        numbersAsSubscriptsStringTabs.get(style).put(id, vis);
        return vis;
    }
        
    private static String prepareId_plain(String id, int style) {
        String vis = plainStringTabs.get(style).get(id);
        if (vis != null)
            return vis;
        vis = format_id(id, style, false);
        plainStringTabs.get(style).put(id, vis);
        return vis;
    }
}
