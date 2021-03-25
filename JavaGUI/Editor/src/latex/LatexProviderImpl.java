/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package latex;

import editor.Main;
import editor.domain.AlternateNameFunction;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.image.BufferedImage;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import javax.swing.JLabel;
import org.scilab.forge.jlatexmath.DefaultTeXFontParser;
import org.scilab.forge.jlatexmath.ParseException;
import org.scilab.forge.jlatexmath.SpaceAtom;
import org.scilab.forge.jlatexmath.TeXConstants;
import org.scilab.forge.jlatexmath.TeXFormula;
import org.scilab.forge.jlatexmath.TeXIcon;
import org.scilab.forge.jlatexmath.UnderscoreAtom;

/** Opens and keeps running the runtime connection with the MathProvider
 *
 * @author elvio
 */
public class LatexProviderImpl extends LatexProvider {
    
    // Where jLaTeX fonts are located in the JAR.
    // This is needed to load fonts stored in a MetaCommand object.
    private static final String[][] jLatexFontTable = {
        { "jlm_cmex10",   "fonts/base/jlm_cmex10.ttf" },
        { "jlm_cmmi10",   "fonts/base/jlm_cmmi10.ttf" },
        { "jlm_cmmib10",  "fonts/base/jlm_cmmib10.ttf" },
        { "jlm_eufb10",   "fonts/euler/jlm_eufb10.ttf" },
        { "jlm_eufm10",   "fonts/euler/jlm_eufm10.ttf" },
        { "jlm_cmr10",    "fonts/latin/jlm_cmr10.ttf" },
        { "jlm_jlmbi10",  "fonts/latin/jlm_jlmbi10.ttf" },
        { "jlm_jlmbx10",  "fonts/latin/jlm_jlmbx10.ttf" },
        { "jlm_jlmi10",   "fonts/latin/jlm_jlmi10.ttf" },
        { "jlm_jlmr10",   "fonts/latin/jlm_jlmr10.ttf" },
        { "jlm_jlmsb10",  "fonts/latin/jlm_jlmsb10.ttf" },
        { "jlm_jlmsbi10", "fonts/latin/jlm_jlmsbi10.ttf" },
        { "jlm_jlmsi10",  "fonts/latin/jlm_jlmsi10.ttf" },
        { "jlm_jlmss10",  "fonts/latin/jlm_jlmss10.ttf" },
        { "jlm_jlmtt10",  "fonts/latin/jlm_jlmtt10.ttf" },
        { "jlm_cmbsy10",  "fonts/maths/jlm_cmbsy10.ttf" },
        { "jlm_cmsy10",   "fonts/maths/jlm_cmsy10.ttf" },
        { "jlm_msam10",   "fonts/maths/jlm_msam10.ttf" },
        { "jlm_msbm10",   "fonts/maths/jlm_msbm10.ttf" },
        { "jlm_rsfs10",   "fonts/maths/jlm_rsfs10.ttf" },
        { "jlm_special",  "fonts/maths/jlm_special.ttf" },
        { "jlm_stmary10", "fonts/maths/jlm_stmary10.ttf" },
        { "jlm_cmbx10",   "fonts/latin/optional/jlm_cmbx10.ttf" },
        { "jlm_cmbxti10", "fonts/latin/optional/jlm_cmbxti10.ttf" },
        { "jlm_cmss10",   "fonts/latin/optional/jlm_cmss10.ttf" },
        { "jlm_cmssbx10", "fonts/latin/optional/jlm_cmssbx10.ttf" },
        { "jlm_cmssi10",  "fonts/latin/optional/jlm_cmssi10.ttf" },
        { "jlm_cmti10",   "fonts/latin/optional/jlm_cmti10.ttf" },
        { "jlm_cmtt10",   "fonts/latin/optional/jlm_cmtt10.ttf" },
        { "jlm_dsrom10",  "fonts/maths/optional/jlm_dsrom10.ttf" },
    };
    
    // Keep track of the globally installed JLaTeXMath fonts
    public static Map<String, java.awt.Font> preloadedFont = new HashMap<>();
    
    public static java.awt.Font createLatexFont(String faceName) {
        if (preloadedFont.containsKey(faceName))
            return preloadedFont.get(faceName);
        
        for (int i=0; i<jLatexFontTable.length; i++) {
            if (faceName.equals(jLatexFontTable[i][0])) {
                Font font = DefaultTeXFontParser.createFont(jLatexFontTable[i][1]);
                
                preloadedFont.put(faceName, font);
                
                return font;
            }
        }
        throw new RuntimeException("Missing font info for: "+faceName);
    }
        
    // The LaTeX cache.
    // Stores the precomputed meta-commands (LaTeX strings already processed and stored
    // in the disk cache). The cache is loaded at startup with initializeProvider()
    private final TypesetCache typesetCacheLRU = new TypesetCache();


    public static void initializeProvider() {
        LatexProviderImpl lp = new LatexProviderImpl();
        LatexProvider.initialize(lp);
        
        // Modify underscore shape in jLaTeXMath
        UnderscoreAtom.w = new SpaceAtom(TeXConstants.UNIT_EM, 0.4f, 0, 0); // was 0.6f
        UnderscoreAtom.s = new SpaceAtom(TeXConstants.UNIT_EM, 0.06f, 0, 0);
        
        // Load cached pre-rendered latex strings
        lp.typesetCacheLRU.loadCacheFromFile(lp.getCacheFile());
//        TeXFormula.registerFonts(true);
    }
    
    @Override
    public synchronized void quit() {
        typesetCacheLRU.storeCacheToFile(getCacheFile());
        System.out.println("LaTeX cache saved to disk.");
    }
    
    public LatexProviderImpl() {
    }

    // Get the graphics command corresponding to a LaTeX formula, or a
    // LatexParseException if the formula is not parseable.
    @Override
    MetaCommands typesetLatexFormula(String latex, float size, boolean useCache) 
    {
        MetaCommands mc = null;
        if (useCache)
            mc = typesetCacheLRU.searchCache(latex, size);
        if (mc != null)
            return mc;
        mc = typesetLatexFormulaImpl(latex, size);
        if (mc == null) {
            mc = MetaCommands.drawSimpleString(deLatexify(latex), (int)size);
        }
        if (useCache)
            typesetCacheLRU.addToCache(latex, size, mc);
        return mc;
    }
    
    private final BufferedImage dummyImage = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB);
    private final JLabel dummyLabel = new JLabel();
    
    private synchronized MetaCommands typesetLatexFormulaImpl(String latex, float size) 
    {
        //System.err.println("processLatexFormula "+latex+"  "+size);
        long startTime = System.currentTimeMillis();
        boolean doLog = false;
        
        // Sanitize LaTeX input
        String sanitizedLatex = sanitizeInput(latex);
//            if (sentLatex.length() > 0 && sentLatex.charAt(0)=='#')
//                sentLatex = '\\' + sentLatex;
//            sentLatex = sentLatex.replaceAll("([^\\\\])#", "$1\\\\#");
        if (!sanitizedLatex.equals(latex))
            System.out.println("sanitizedLatex = <"+sanitizedLatex+">");

        // Prepare the LaTeX formula
        TeXFormula formula;
        try {
            // Avoid bug in jLatexMath engine.
            TeXFormula.externalFontMap.remove(Character.UnicodeBlock.BASIC_LATIN);
            // Prepare and parse the (sanitized) latex text
            formula = new TeXFormula(sanitizedLatex);
        }
        catch (ParseException pe) {
            // Invalid LaTeX object.
            Main.logException(pe, true);
            return null;
        }
        int style = TeXConstants.STYLE_DISPLAY;
        TeXIcon icon = formula.createTeXIcon(style, size);
        //TeXIcon icon = formula.new TeXIconBuilder().setStyle(TeXConstants.STYLE_DISPLAY).setSize(size).build();
        icon.setInsets(new Insets(0, 0, 0, 0)); // top, left, bottom, right
        
        // Prepare the Graphics2D target
        Graphics2D g2x = dummyImage.createGraphics();
        MetaGraphics2D g2 = new MetaGraphics2D(g2x, false/*doDraw*/, doLog);

        // Draw the LaTeX formula into the Graphics2D object
        try {
            dummyLabel.setForeground(Color.BLACK);
            icon.paintIcon(dummyLabel, g2, 0, 0);
        } catch (MetaGfxException e) {
            Main.logException(e, true);
        }
        
        // Compute the expected line height
        TeXFormula exFormula = new TeXFormula("A^0_0");
        TeXIcon exIcon = exFormula.createTeXIcon(TeXConstants.STYLE_DISPLAY, size);

        // Save the graphic commands in a meta-command object
        MetaCommands mc = new MetaCommands(latex, size,
            icon.getIconWidth(), icon.getIconHeight(), icon.getIconDepth(),
            exIcon.getIconHeight(), exIcon.getIconDepth());
        g2.saveCommands(mc);
        
        double totTime = (System.currentTimeMillis() - startTime) / 1000.0;
        System.out.println("LaTeX formula \""+latex+"\" typesetted in " + totTime + " sec.");
        
        if (doLog) {
            g2.printObjectTables();
            System.err.println("\n");
        }        
        return mc;
    }
    
    // Remove LaTeX commands from a string
    private static String deLatexify(String str) {
        StringBuilder sb = new StringBuilder();
        int i = 0;
        while (i < str.length()) {
            char ch = str.charAt(i++);
            switch (ch) {
                case '\\': { // start of LaTeX command 
                    int i0 = i;
                    while (i < str.length() && Character.isAlphabetic(str.charAt(i)))
                        i++;
                    if (i < str.length() && i == i0)
                        i++; // single character command, like \!
                    String cmd = str.substring(i0, i);
                    if (AlternateNameFunction.SPECIAL_LATEX_NAMES.contains(cmd)) {
                        // symbol like 'pi', 'Psi', etc...
                        sb.append(cmd);
                    }
                    else {
                        switch (cmd) {
                            case "mathit":
                            case "mathrm":
                            case "mathbf":
                            case "textbf":
                            case "textit":
                            case "textrm":
                            case "text":
                            case "bf":
                            case "it":
                                i++; // skip '{'
                                break;

                            case "hspace":
                                while (i < str.length() && str.charAt(i) != '}')
                                    i++;
                                break;
                                
                            case "langle":       sb.append('<');   break;
                            case "rangle":       sb.append('>');   break;
                            case "{":            sb.append('{');   break;
                            case "}":            sb.append('}');   break;
                            case "_":            sb.append('_');   break;
                            case " ":            sb.append(' ');   break;
                            case "^":            sb.append('^');   break;
                            case "#":            sb.append('#');   break;
                            case "sharp":        sb.append('#');   break;
                            case "rightarrow":   sb.append("->");   break;
                            case "setminus":     sb.append('\\');   break;
                            case "cdot":         sb.append('*');   break;
                            case "times":        sb.append('*');   break;
                            case "geq":          sb.append(">=");   break;
                            case "leq":          sb.append("<=");   break;
                            case "in":           sb.append("in");   break;
                            case "not":          sb.append("!");   break;

                                // ignore directly
                            case "!":
                            case ",":
                            case "left":
                            case "right":
                                break;

                            default:
                                System.out.println("Unknown LaTeX command: \""+cmd
                                        +"\" in string: \""+str+"\" at "+i0+"-"+i);
                                sb.append(cmd);
                        }
                    }
                }
                break;
                                    
                case '}':
                case '{':
                    break;
                    
                default:
                    sb.append(ch);
            }
        }
        System.out.println("deLatexify ["+str+"] -> ["+sb.toString()+"]");
        return sb.toString();
    }
    
    // Converts a general expression (with invalid LaTeX characters like &,^,...)
    // into a LaTeX string. This can be used when normal LaTeX conversions fails.
    public static String basicLatexForm(String input) {
        StringBuilder sb = new StringBuilder();
        boolean textMode = false;
        int i = 0;
        while (i < input.length()) {
            char ch = input.charAt(i++);
            
            // Text mode
            if (Character.isAlphabetic(ch) || ch==' ') {
                if (!textMode) {
                    sb.append("\\mathrm{");
                    textMode = true;
                }
                sb.append(ch==' ' ? "\\hspace{4pt}" : ch);
            }
            else {
                if (textMode) {
                    sb.append("}");
                    textMode = false;
                }
                
                switch (ch) {
                    case '&':
                    case '^':
                    case '_':
                    case '~':
                    case '}':
                    case '{':
                    case '#':
                    //case '!':
                        sb.append("\\").append(ch);
                        break;
                        
                    default:
                        sb.append(ch);
                        break;
                }
            }
        }
        
        if (textMode) {
            sb.append("}");
            textMode = false;
        }
        return sb.toString();
    } 
    
    
    private String sanitizeInput(String latexInput) {
        StringBuilder sb = new StringBuilder();
        
        boolean prevWasBackslash = false;
        for (int i=0; i<latexInput.length(); i++) {
            char ch = latexInput.charAt(i);
            switch (ch) {
                case '#':
                case '~':
                case '@':                    
                    if (!prevWasBackslash) {
                        sb.append('\\');
                    }
            }
            sb.append(ch);
            
            prevWasBackslash = (ch == '\\');
        }
        
        return sb.toString();
    }
    
    
    public synchronized boolean checkLatexFormula(String latex) {
        try {
            TeXFormula formula = new TeXFormula(latex);
            return true;
        }
        catch (ParseException pe) {
            Main.logException(pe, true);
            return false;
        }
    } 
    
    
    //-----------------------------------------------------------------------
    // Internal MetaCommands cache implementation
    //-----------------------------------------------------------------------

    // A key in the cache Hash table
    private static class TypesetKey implements Serializable{
        private final String latex;
        private final float size;

        public TypesetKey(String latex, float size) {
            this.latex = latex;
            this.size = size;
        }
        @Override
        public boolean equals(Object o) {
            if (!(o instanceof TypesetKey))
                return false;
            TypesetKey tk = (TypesetKey)o;
            return latex.equals(tk.latex) && size == tk.size;
        }
        
        @Override
        public int hashCode() {
            return latex.hashCode() ^ Float.floatToIntBits(size);
        }
    }
        
    // The LRU cache class
    private class TypesetCache extends LinkedHashMap<TypesetKey, SoftReference<MetaCommands>> {

        public TypesetCache() {
            super(65, 0.75f, true/* keep LRU order */);
        }
        
        @Override
        protected boolean removeEldestEntry(final Map.Entry<TypesetKey, SoftReference<MetaCommands>> eldest) {
            return super.size() > Main.getMaxLatexCacheEntries();
        }
        
        public synchronized MetaCommands searchCache(String latex, float size) {
            TypesetKey tk = new TypesetKey(latex, size);
            SoftReference<MetaCommands> refMc = get(tk);
            if (refMc == null)
                return null;
            MetaCommands mc = refMc.get();
            if (mc == null) {
                // The garbage collector has reclaimed this cache entry. Delete it.
                typesetCacheLRU.remove(tk);
            }
            return mc;
        }

        public synchronized void addToCache(String latex, float size, MetaCommands mc) {
            typesetCacheLRU.put(new TypesetKey(latex, size), 
                             new SoftReference<>(mc));
        }
        
        public synchronized void loadCacheFromFile(File f) {
            long startTime = System.currentTimeMillis();
            try {
                if (!f.canRead()) {
                    System.out.println("LaTeX cache is empty.");
                    return;
                }
                ObjectInputStream input = new ObjectInputStream(new BufferedInputStream(new FileInputStream(f)));
                while (input.readChar() == 'C') {
                    TypesetKey key = (TypesetKey)input.readObject();
                    MetaCommands mc = (MetaCommands)input.readObject();
                    typesetCacheLRU.put(key, new SoftReference<>(mc));
                }
                input.close();
                double totalLoadTime = (System.currentTimeMillis() - startTime) / 1000.0;
                System.out.println("LaTeX cache has "+size()+" entries, loaded in "+totalLoadTime+" seconds.");
            }
            catch (java.io.InvalidClassException e) { /*cache class has changed.*/ }
            catch (Exception e) {
                System.out.println("Could not load the LaTeX cache.");
                Main.logException(e, false);
            }
        }
        
        public synchronized void storeCacheToFile(File f) {
            try {
                ObjectOutputStream output = new ObjectOutputStream(new BufferedOutputStream(new FileOutputStream(f, false)));
                Set<Entry<TypesetKey, SoftReference<MetaCommands>>> set = entrySet();
                Iterator<Entry<TypesetKey, SoftReference<MetaCommands>>> it = set.iterator();
                while (it.hasNext()) {
                    Entry<TypesetKey, SoftReference<MetaCommands>> entry = it.next();
                    MetaCommands mc = entry.getValue().get();
                    if (mc != null) {
                        output.writeChar('C');
                        output.writeObject(entry.getKey());
                        output.writeObject(mc);
                    }
                }
                output.writeChar('Q');
                output.close();
            }
            catch (IOException e) {
                System.out.println("Could not store the LaTeX cache.");
                Main.logException(e, true);
                f.delete();
            }
        }
    }
        
    private File getCacheFile() {
        String workingDirectory;
        String OS = (System.getProperty("os.name")).toUpperCase();
        if (OS.contains("WIN")) {
            workingDirectory = System.getenv("AppData");
        }
        else if (OS.contains("MAC")) {
            workingDirectory = System.getProperty("user.home");
            workingDirectory += "/Library/Caches";            
        }
        else { // Linux ?
            workingDirectory = System.getProperty("user.home");
        }
        assert (workingDirectory != null);
        return new File(workingDirectory, ".latexCache");
    }
    
    public void clearCache() {
        typesetCacheLRU.clear();
        File diskCache = getCacheFile();
        diskCache.delete();
    }
    
    public int getNumCacheEntries() {
        return typesetCacheLRU.size();
    }
}
