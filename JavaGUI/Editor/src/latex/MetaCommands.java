/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package latex;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.font.FontRenderContext;
import java.awt.font.GlyphVector;
import java.awt.font.LineMetrics;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;

/** Sequence of graphics commands, exchanged with the MathProvider.
 * Used to encode the commands needed to draw a LaTeX formula.
 *
 * @author elvio
 */
class MetaCommands implements Serializable {
    
    //--------------------------------------------------------------------------
    // Keep the list of commands in sync with MetaGraphics2D
    //--------------------------------------------------------------------------
    private static final int DRAW_STRING = 10;
    private static final int FILL_SHAPE= 11;
    private static final int TRANSLATE = 12;
    private static final int ROTATE = 13;
    private static final int ROTATE_XY = 14;
    private static final int SCALE = 15;
    private static final int SET_TRANSFORM = 16;
    private static final int SET_COLOR = 17;
    private static final int SET_FONT = 18;
    private static final int FILL_RECT = 19;
    private static final int DRAW_GLYPH_VECTOR = 20;
    
    
    
    //================================================
    // Data tables and commands
    //================================================
    
    // General informations
    String latex;
    float latexSize;
    int width;
    int height;
    int depth;
    int lineHeight;
    int lineDepth;
    
    // The encoded command buffer
    int[] cmdBuf;
    
    // Resource tables
    Color[] colorTable;
    String[] stringTable;
    Shape[] shapeTable;
    AffineTransform[] affTranTable;
    
    // Special management for Font resources (with late loading)
    String[] fontNames;
    int[] fontStyles;
    int[] fontSizes;
    transient Font[] fontTableLazy;
    
    // Glyph vectors
    transient GlyphVector[] glyphVecTable;
    int[] gvFontIds;
    int[][] gvGlyphCodes;
    int[] gvAffineTransforms;
    
    
    double itod(int i) {
        return (double)Float.intBitsToFloat(cmdBuf[i]);
    }
    float itof(int i) {
        return Float.intBitsToFloat(cmdBuf[i]);
    }
    
    public MetaCommands() { }

    public MetaCommands(String latex, float latexSize, int width, int height, int depth, 
                        int lineHeight, int lineDepth) 
    {
        this.latex = latex;
        this.latexSize = latexSize;
        this.width = width;
        this.height = height;
        this.depth = depth;
        this.lineHeight = lineHeight;
        this.lineDepth = lineDepth;
    }
    
    
        
//    public static MetaCommands loadFromStream(String latex, float size, ObjectInputStream stream) 
//            throws IOException, ClassNotFoundException, RuntimeException
//    {
//        MetaCommands mc = new MetaCommands();
//        mc.latex = latex;
//        mc.latexSize = size;
//                
//        // Read header
//        mc.width = stream.readInt();
//        mc.height = stream.readInt();
//        mc.depth = stream.readInt();
//        mc.lineHeight = stream.readInt();
//        mc.lineDepth = stream.readInt();
//        
//        // Read commands
//        mc.cmdBuf = (int[])stream.readObject();
//        
//        // Read colors
//        int[] rgb = (int[])stream.readObject();
//        mc.colorTable = new Color[rgb.length];
//        for (int i=0; i<rgb.length; i++) {
//            if (rgb[i] == Color.WHITE.getRGB())
//                mc.colorTable[i] = Color.WHITE;
//            else if (rgb[i] == Color.BLACK.getRGB())
//                mc.colorTable[i] = Color.BLACK;
//            else
//                mc.colorTable[i] = new Color(rgb[i]);
//        }
//        
//        // Read strings
//        mc.stringTable = (String[])stream.readObject();
//        
//        // Read and recreate fonts
//        mc.fontNames = (String[])stream.readObject();
//        mc.fontStyles = (int[])stream.readObject();
//        mc.fontSizes = (int[])stream.readObject();
//        mc.fontTableLazy = new Font[mc.fontNames.length]; // empty table
////        for (int i=0; i<mc.fontTable.length; i++) {
////            mc.fontTable[i] = new Font(names[i], styles[i], sizes[i]);
////        }
//        
//        // Read shapes
//        int numShapes = stream.readInt();
//        mc.shapeTable = new Shape[numShapes];
//        for (int i=0; i<numShapes; i++) {
//            char type = stream.readChar();
//            switch (type) {
//                case 'R':
//                    double x = stream.readDouble();
//                    double y = stream.readDouble();
//                    double w = stream.readDouble();
//                    double h = stream.readDouble();
//                    mc.shapeTable[i] = new Rectangle2D.Double(x, y, w, h);
//                    break;
//                default:
//                    throw new UnsupportedOperationException("Unknown shape type.");
//            }
//        }
//        
//        // Read affine transformation matrixes
//        int numAffTrn = stream.readInt();
//        mc.affTranTable = new AffineTransform[numAffTrn];
//        double[] matrix = new double[6];
//        for (int i=0; i<numAffTrn; i++) {
//            for (int j=0; j<6; j++)
//                matrix[j] = stream.readDouble();
//            mc.affTranTable[i] = new AffineTransform(matrix);
//        }
//                
//        // Read glyph vectors
//        int numGlyphVecs = stream.readInt();
//        mc.gvFontIds = new int[numGlyphVecs];
//        mc.gvGlyphCodes = new int[numGlyphVecs][];
//        mc.gvAffineTransforms = new int[numGlyphVecs];
//        mc.glyphVecTable = null;
//        for (int i=0; i<numGlyphVecs; i++) {
//            mc.gvFontIds[i] = stream.readInt();
//            mc.gvGlyphCodes[i] = (int[])stream.readObject();
//            mc.gvAffineTransforms[i] = stream.readInt();
//        }
//        
//        // Read the ending token
//        String endTok = (String)stream.readObject();
//        if (!endTok.equals("END"))
//            throw new IllegalStateException("Out of synchronization with the MathProvider.");
//        
//        //---------------------------------
////        ByteArrayOutputStream baos = new ByteArrayOutputStream();
////        ObjectOutputStream oos = new ObjectOutputStream(baos);
////        oos.writeObject(mc);
////        
////        ByteArrayInputStream bais = new ByteArrayInputStream(baos.toByteArray());
////        ObjectInputStream ois = new ObjectInputStream(bais);
////        MetaCommands mc2 = (MetaCommands)ois.readObject();
////        return mc2;
//
//        return mc;
//    }
    
    private static final FontRenderContext defaultFRC = 
            new FontRenderContext(null, RenderingHints.VALUE_TEXT_ANTIALIAS_ON, 
                    RenderingHints.VALUE_FRACTIONALMETRICS_ON);
    
    public static MetaCommands drawSimpleString(String str, int size) {
//        System.out.println("drawSimpleString(\""+str+"\", "+size+")");
        MetaCommands mc = new MetaCommands();
        mc.cmdBuf = new int[2 + 4];
        int i = 0;
        // set font
        mc.fontNames = new String[] { "Times" };
        mc.fontSizes = new int[] { size };
        mc.fontStyles = new int[] { 0 };
        mc.cmdBuf[i++] = SET_FONT;
        mc.cmdBuf[i++] = 0;

        Font f = mc.getFont(0);
        LineMetrics lm = f.getLineMetrics("Mj", defaultFRC);
        Rectangle2D bounds = f.getStringBounds(str, defaultFRC);
        mc.width = (int)Math.ceil(bounds.getWidth());
        mc.height = (int)Math.ceil(bounds.getHeight());
        mc.depth = (int)(lm.getAscent());
        mc.lineHeight = (int)lm.getHeight();
        mc.lineDepth = mc.depth;

        // draw string
        mc.stringTable = new String[] { str };
        mc.cmdBuf[i++] = DRAW_STRING;
        mc.cmdBuf[i++] = 0;
        mc.cmdBuf[i++] = 0;
        mc.cmdBuf[i++] = mc.depth;
        
        return mc;
    }
    
    
    private Font getFont(int fontId) {
        if (fontTableLazy == null)
            fontTableLazy = new Font[fontNames.length];
        if (fontTableLazy[fontId] == null) {
            // Lazily load the Latex font
            Font base = LatexProviderImpl.createLatexFont(fontNames[fontId]);
//            if (fontNames[fontId].startsWith("jlm_")) {
//                LatexProvider.preloadLatexFont(fontNames[fontId]);
//            }
            Font derived = base.deriveFont(fontStyles[fontId], fontSizes[fontId]);
            fontTableLazy[fontId] = derived;
        }
        return fontTableLazy[fontId];
    }
    
    private GlyphVector getGlyphVector(int j) {
        if (glyphVecTable == null) {
            glyphVecTable = new GlyphVector[gvFontIds.length];
            for (int i=0; i < glyphVecTable.length; i++) {
                Font font = getFont(gvFontIds[i]);
                AffineTransform at = affTranTable[gvAffineTransforms[i]];
                FontRenderContext frc = new FontRenderContext(at, true, true);
                glyphVecTable[i] = font.createGlyphVector(frc, gvGlyphCodes[i]);
            }

        }
        return glyphVecTable[j];
    }
    
    public void doDraw(Graphics2D g, boolean doLog) {
        if (doLog)
            System.out.println("----- doDraw -----");
        Color defaultColor = g.getColor();
        AffineTransform baseTransform = g.getTransform();
        int i = 0;
        while (i < cmdBuf.length) {
            switch (cmdBuf[i++]) {
                case DRAW_STRING: {
                    String str = stringTable[cmdBuf[i++]];
                    int x = cmdBuf[i++];
                    int y = cmdBuf[i++];
                    g.drawString(str, x, y);
                    if (doLog)
                        System.out.println("drawString \""+str+"\" "+x+" "+y);
                    break;
                }
                    
                case FILL_SHAPE: {
                    Shape sh = shapeTable[cmdBuf[i++]];
                    g.fill(sh);
                    if (doLog)
                        System.out.println("fill "+sh);
                    break;
                }
                    
                case TRANSLATE: {
                    double tx = itod(i++);
                    double ty = itod(i++);
                    g.translate(tx, ty);
                    if (doLog)
                        System.out.println("translate "+tx+" "+ty);
                    break;
                }
                    
                case ROTATE: {
                    double theta = itod(i++);
                    g.rotate(theta);
                    if (doLog)
                        System.out.println("rotate "+theta);
                    break;
                }
                    
                case ROTATE_XY: {
                    double theta = itod(i++);
                    double x = itod(i++);
                    double y = itod(i++);
                    g.rotate(theta, x, y);
                    if (doLog)
                        System.out.println("rotate "+theta+" "+x+" "+y);
                    break;                    
                }
                    
                case SCALE: {
                    double sx = itod(i++);
                    double sy = itod(i++);
                    g.scale(sx, sy);
                    if (doLog)
                        System.out.println("scale "+sx+" "+sy);
                    break;                                        
                }
                    
                case SET_TRANSFORM: {
                    int atId = cmdBuf[i++];
                    AffineTransform aft = affTranTable[atId];
                    if (baseTransform != null) {
                        g.setTransform(baseTransform);
                        g.transform(aft);
                    }
                    else 
                        g.setTransform(aft);
                    if (doLog)
                        System.out.println("setTransform TRAN"+atId+" = "+aft);
                    break;
                }
                    
                case SET_COLOR: {
                    Color c = colorTable[cmdBuf[i++]];
                    if (c.getRGB() == Color.BLACK.getRGB())
                        c = defaultColor; // Use the default color instead of (0,0,0)
                    g.setColor(c);
                    if (doLog)
                        System.out.println("setColor "+c);
                    break;
                }
                    
                case SET_FONT:{
                    Font f = getFont(cmdBuf[i++]);
                    g.setFont(f);
                    if (doLog)
                        System.out.println("setFont "+f);
                    break;
                }
                    
                case FILL_RECT: {
                    int x = cmdBuf[i++];
                    int y = cmdBuf[i++];
                    int w = cmdBuf[i++];
                    int h = cmdBuf[i++];
                    g.fillRect(x, y, w, h);
                    if (doLog)
                        System.out.println("fillRect "+x+" "+y+" "+w+" "+h);
                    break;
                }
                    
                case DRAW_GLYPH_VECTOR: {
                    GlyphVector glyphs = getGlyphVector(cmdBuf[i++]);                    
                    float x = itof(cmdBuf[i++]);
                    float y = itof(cmdBuf[i++]);
                    g.drawGlyphVector(glyphs, x, y);
                    if (doLog)
                        System.out.println("drawGlyphVector "+glyphs+" "+x+" "+y);
                    break;
                }
                    
                default:
                    throw new IllegalArgumentException("Wrong command serialization.");
            }
        }
        
        g.setColor(defaultColor);
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public int getDepth() {
        return depth;
    }

    public int getLineHeight() {
        return lineHeight;
    }

    public int getLineDepth() {
        return lineDepth;
    }
}
