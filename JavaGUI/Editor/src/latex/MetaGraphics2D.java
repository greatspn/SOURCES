package latex;

import java.awt.Color;
import java.awt.Composite;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.Paint;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.RenderingHints.Key;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.font.FontRenderContext;
import java.awt.font.GlyphVector;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.image.BufferedImageOp;
import java.awt.image.ImageObserver;
import java.awt.image.RenderedImage;
import java.awt.image.renderable.RenderableImage;
import java.text.AttributedCharacterIterator;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class MetaGraphics2D extends Graphics2D {

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

    
    // Internal graphics delegate
    private Graphics2D g;
    // Should draw() commands be sent to the graphics object
    private final boolean doDraw;
    // Should log the received commands?
    private final boolean doLog;
    
    // The vector of encoded commands & arguments
    private final ArrayList<Integer> commands = new ArrayList<Integer>();
    private void writeCmd(int cmd) {
        commands.add(cmd);
    }
    private void writeCmd(int cmd, int i1) {
        writeCmd(cmd);
        commands.add(i1);
    }
    private void writeCmd(int cmd, int i1, int i2) {
        writeCmd(cmd, i1);
        commands.add(i2);
    }
    private void writeCmd(int cmd, int i1, int i2, int i3) {
        writeCmd(cmd, i1, i2);
        commands.add(i3);
    }
    private void writeCmd(int cmd, int i1, int i2, int i3, int i4) {
        writeCmd(cmd, i1, i2, i3);
        commands.add(i4);
    }
    private int dtoi(double v) {
        return ftoi((float)v);
    }
    private int ftoi(float v) {
        return Float.floatToIntBits(v);
    }
    
    abstract class DataTable<E> {
        private final Map<String, Integer> map = new HashMap<>();
        ArrayList<E> table = new ArrayList<>();
        public int getID(E obj) {
            int id;
            String repr = makeRepr(obj);
            if (map.containsKey(repr)) {
                id = map.get(repr);
            } else {
                id = table.size();
                table.add(obj);
                map.put(repr, id);
            }
            return id;
        }
        protected abstract String makeRepr(E obj);
        public int size() { return table.size(); }
        public E get(int i) { return table.get(i); }
        public String getRepr(int i) { return makeRepr(table.get(i)); }
        public void print(String header, String id) {
            System.err.println(header+" TABLE:");
            for (int i = 0; i < size(); i++) {
                System.err.println(id + i + " = " + get(i));
            }
            System.err.println();
        }
    }
    
    class ColorTable extends DataTable<Color> {
        @Override
        protected String makeRepr(Color obj) {
            StringBuilder sb = new StringBuilder();
            sb.append(obj.getRed());
            sb.append("-");            
            sb.append(obj.getGreen());
            sb.append("-");            
            sb.append(obj.getBlue());
            return sb.toString();
        }
    }
    ColorTable colorTbl = new ColorTable();
    
    class StringTable extends DataTable<String> {
        @Override
        protected String makeRepr(String obj) {
            return obj;
        }
    }
    StringTable stringTbl = new StringTable();
    
    class FontTable extends DataTable<Font> {
        @Override
        protected String makeRepr(Font obj) {
            StringBuilder sb = new StringBuilder();
            sb.append(obj.getName());
            sb.append("-"); 
            sb.append(obj.getStyle());
            sb.append("-"); 
            sb.append(obj.getSize());
            return sb.toString();
        }
    }
    FontTable fontTbl = new FontTable();
    
    class ShapeTable extends DataTable<Shape> {
        @Override
        protected String makeRepr(Shape obj) {
            StringBuilder sb = new StringBuilder();
            if (obj instanceof Rectangle2D) {
                Rectangle2D r = (Rectangle2D)obj;
                sb.append("RECT2D: ");
                sb.append(r.getX()).append(",").append(r.getY()).append(",").append(r.getWidth()).append(",").append(r.getHeight());
            }
            else throw new UnsupportedOperationException("Unsupported shape.");
            return sb.toString();
        }
    }
    ShapeTable shapeTbl = new ShapeTable();
    
    class AffineTransformTable extends DataTable<AffineTransform> {
        @Override
        protected String makeRepr(AffineTransform obj) {
            StringBuilder sb = new StringBuilder();
            double[] matrix = new double[6];
            obj.getMatrix(matrix);
            for (int i=0; i<matrix.length; i++) {
                if (i > 0)
                    sb.append("-");
                sb.append(matrix[i]);
            }
            return sb.toString();
        }
    }
    AffineTransformTable affTranTbl = new AffineTransformTable();
    
    class GlyphVectorTable extends DataTable<GlyphVector> {
        @Override
        protected String makeRepr(GlyphVector obj) {
            StringBuilder sb = new StringBuilder();
            int[] glyphs = obj.getGlyphCodes(0, obj.getNumGlyphs(), null);
            Font font = obj.getFont();
            int fontId = fontTbl.getID(font);
            sb.append(fontId);
            sb.append(":");
            sb.append(affTranTbl.getID(obj.getFontRenderContext().getTransform()));
            sb.append(":");
            for (int g : glyphs) {
                sb.append(g);
                sb.append("-");
            }
            return sb.toString();
        }
    }
    GlyphVectorTable gliphVecTbl = new GlyphVectorTable();

    public MetaGraphics2D(Graphics2D g, boolean doDraw, boolean doLog) {
        this.g = g;
        this.doDraw = doDraw;
        this.doLog = doLog;
    }
    
    public void printObjectTables() {
        colorTbl.print("COLOR", "CLR");
        stringTbl.print("STRING", "STR");
        fontTbl.print("FONT", "FONT");
        shapeTbl.print("SHAPE", "SHP");
        affTranTbl.print("AFFINE TRANSFORM", "TRAN");
        
        System.err.println("\nCOMMANDS: ");
        for (int i=0; i<commands.size(); i++)
            System.err.print(commands.get(i)+" "+(19==(i%20) ? "\n" : ""));
        System.err.println("\n\n");
        System.err.println("TOTAL COMMANDS: "+commands.size()+"\n");
    }
    
    public void saveCommands(MetaCommands mc) {
        // Save commands
        mc.cmdBuf = new int[commands.size()];
        for (int i=0; i<commands.size(); i++)
            mc.cmdBuf[i] = commands.get(i);
        
        // Save colors
        mc.colorTable = new Color[colorTbl.size()];
        for (int i=0; i<colorTbl.size(); i++)
            mc.colorTable[i] = colorTbl.get(i);
        
        // Save the string table
        mc.stringTable = new String[stringTbl.size()];
        for (int i=0; i<stringTbl.size(); i++)
            mc.stringTable[i] = stringTbl.get(i);
        
        // Save the font table
        mc.fontNames = new String[fontTbl.size()];
        mc.fontStyles = new int[fontTbl.size()];
        mc.fontSizes = new int[fontTbl.size()];
        for (int i=0; i<fontTbl.size(); i++) {
            mc.fontNames[i] = fontTbl.get(i).getName();
            mc.fontStyles[i] = fontTbl.get(i).getStyle();
            mc.fontSizes[i] = fontTbl.get(i).getSize();
        }
        mc.fontTableLazy = new Font[mc.fontNames.length]; // empty table
        
        // Save shape table
        mc.shapeTable = new Shape[shapeTbl.size()];
        for (int i=0; i<shapeTbl.size(); i++)
            mc.shapeTable[i] = shapeTbl.get(i);
        
        // Save affine transformation matrixes
        mc.affTranTable = new AffineTransform[affTranTbl.size()];
        for (int i=0; i<affTranTbl.size(); i++)
            mc.affTranTable[i] = affTranTbl.get(i);
            
        // Save glyph vectors
        mc.glyphVecTable = new GlyphVector[gliphVecTbl.size()];
        for (int i=0; i<gliphVecTbl.size(); i++)
            mc.glyphVecTable[i] = gliphVecTbl.get(i);
    }
    
//    public void writeToStream(ObjectOutputStream stream) throws IOException {
//        // Write commands
//        int[] cmds = new int[commands.size()];
//        for (int i=0; i<commands.size(); i++)
//            cmds[i] = commands.get(i);
//        stream.writeObject(cmds);
//        
//        // Write colors as an array of rgb values
//        int[] rgb = new int[colorTbl.size()];
//        for (int i=0; i<colorTbl.size(); i++) {
//            rgb[i] = colorTbl.get(i).getRGB();
//        }
//        stream.writeObject(rgb);
//        
//        // Write strings
//        String[] strings = new String[stringTbl.size()];
//        for (int i=0; i<stringTbl.size(); i++)
//            strings[i] = stringTbl.get(i);
//        stream.writeObject(strings);
//        
//        // Write fonts as three arrays
//        String[] names = new String[fontTbl.size()];
//        int[] styles = new int[fontTbl.size()];
//        int[] sizes = new int[fontTbl.size()];
//        for (int i=0; i<fontTbl.size(); i++) {
//            names[i] = fontTbl.get(i).getName();
//            styles[i] = fontTbl.get(i).getStyle();
//            sizes[i] = fontTbl.get(i).getSize();
//        }
//        stream.writeObject(names);
//        stream.writeObject(styles);
//        stream.writeObject(sizes);
//        
//        // Write shapes
//        stream.writeInt(shapeTbl.size());
//        for (int i=0; i<shapeTbl.size(); i++) {
//            Shape shape = shapeTbl.get(i);
//            if (shape instanceof Rectangle2D) {
//                Rectangle2D r = (Rectangle2D)shape;
//                stream.writeChar('R');
//                stream.writeDouble(r.getX());
//                stream.writeDouble(r.getY());
//                stream.writeDouble(r.getWidth());
//                stream.writeDouble(r.getHeight());
//            }
//            else {
//                unsupported();
//            }
//        }
//        
//        // Write transformations
//        double[] matrix = new double[6];
//        stream.writeInt(affTranTbl.size());
//        for (int i=0; i<affTranTbl.size(); i++) {
//            affTranTbl.get(i).getMatrix(matrix);
//            for (int j=0; j<6; j++)
//                stream.writeDouble(matrix[j]);
//        }
//        
//        // Write glyph vectors
//        stream.writeInt(gliphVecTbl.size());
//        for (int i=0; i<gliphVecTbl.size(); i++) {
//            GlyphVector gv = gliphVecTbl.get(i);
//            int fontId = fontTbl.getID(gv.getFont());
//            int[] codes = gv.getGlyphCodes(0, gv.getNumGlyphs(), null);
//            int affTrn = affTranTbl.getID(gv.getFontRenderContext().getTransform());
//            stream.writeInt(fontId);
//            stream.writeObject(codes);
//            stream.writeInt(affTrn);
//        }
//        
//        // Write the ending token
//        stream.writeObject("END");
//    }

    public void unsupported() {
        throw new MetaGfxException("Unsupported Graphics2D method.");
    }

    public enum RenderingHintKeyConv {

        RHKC_ALPHA_INTERPOLATION(RenderingHints.KEY_ALPHA_INTERPOLATION, "KEY_ALPHA_INTERPOLATION"),
        RHKC_ANTIALIASING(RenderingHints.KEY_ANTIALIASING, "KEY_ANTIALIASING"),
        RHKC_COLOR_RENDERING(RenderingHints.KEY_COLOR_RENDERING, "KEY_COLOR_RENDERING"),
        RHKC_DITHERING(RenderingHints.KEY_DITHERING, "KEY_DITHERING"),
        RHKC_FRACTIONALMETRICS(RenderingHints.KEY_FRACTIONALMETRICS, "KEY_FRACTIONALMETRICS"),
        RHKC_INTERPOLATION(RenderingHints.KEY_INTERPOLATION, "KEY_INTERPOLATION"),
        RHKC_RENDERING(RenderingHints.KEY_RENDERING, "KEY_RENDERING"),
        RHKC_STROKE_CONTROL(RenderingHints.KEY_STROKE_CONTROL, "KEY_STROKE_CONTROL"),
        RHKC_TEXT_ANTIALIASING(RenderingHints.KEY_TEXT_ANTIALIASING, "KEY_TEXT_ANTIALIASING");
        public String name;
        public Key key;

        RenderingHintKeyConv(Key key, String name) {
            this.key = key;
            this.name = name;
        }

        public static String keyToName(Key key) {
            for (int i = 0; i < values().length; i++) {
                if (values()[i].key.equals(key)) {
                    return values()[i].name;
                }
            }
            return null;
        }
    }

    public enum RenderingHintValueConv {

        RHVC_ALPHA_INTERPOLATION_DEFAULT(RenderingHints.VALUE_ALPHA_INTERPOLATION_DEFAULT, "VALUE_ALPHA_INTERPOLATION_DEFAULT "),
        RHVC_ALPHA_INTERPOLATION_QUALITY(RenderingHints.VALUE_ALPHA_INTERPOLATION_QUALITY, "VALUE_ALPHA_INTERPOLATION_QUALITY "),
        RHVC_ALPHA_INTERPOLATION_SPEED(RenderingHints.VALUE_ALPHA_INTERPOLATION_SPEED, "VALUE_ALPHA_INTERPOLATION_SPEED "),
        RHVC_ANTIALIAS_DEFAULT(RenderingHints.VALUE_ANTIALIAS_DEFAULT, "VALUE_ANTIALIAS_DEFAULT "),
        RHVC_ANTIALIAS_OFF(RenderingHints.VALUE_ANTIALIAS_OFF, "VALUE_ANTIALIAS_OFF "),
        RHVC_ANTIALIAS_ON(RenderingHints.VALUE_ANTIALIAS_ON, "VALUE_ANTIALIAS_ON "),
        RHVC_COLOR_RENDER_DEFAULT(RenderingHints.VALUE_COLOR_RENDER_DEFAULT, "VALUE_COLOR_RENDER_DEFAULT "),
        RHVC_COLOR_RENDER_QUALITY(RenderingHints.VALUE_COLOR_RENDER_QUALITY, "VALUE_COLOR_RENDER_QUALITY "),
        RHVC_COLOR_RENDER_SPEED(RenderingHints.VALUE_COLOR_RENDER_SPEED, "VALUE_COLOR_RENDER_SPEED "),
        RHVC_DITHER_DEFAULT(RenderingHints.VALUE_DITHER_DEFAULT, "VALUE_DITHER_DEFAULT "),
        RHVC_DITHER_DISABLE(RenderingHints.VALUE_DITHER_DISABLE, "VALUE_DITHER_DISABLE "),
        RHVC_DITHER_ENABLE(RenderingHints.VALUE_DITHER_ENABLE, "VALUE_DITHER_ENABLE "),
        RHVC_FRACTIONALMETRICS_DEFAULT(RenderingHints.VALUE_FRACTIONALMETRICS_DEFAULT, "VALUE_FRACTIONALMETRICS_DEFAULT "),
        RHVC_FRACTIONALMETRICS_OFF(RenderingHints.VALUE_FRACTIONALMETRICS_OFF, "VALUE_FRACTIONALMETRICS_OFF "),
        RHVC_FRACTIONALMETRICS_ON(RenderingHints.VALUE_FRACTIONALMETRICS_ON, "VALUE_FRACTIONALMETRICS_ON "),
        RHVC_INTERPOLATION_BICUBIC(RenderingHints.VALUE_INTERPOLATION_BICUBIC, "VALUE_INTERPOLATION_BICUBIC "),
        RHVC_INTERPOLATION_BILINEAR(RenderingHints.VALUE_INTERPOLATION_BILINEAR, "VALUE_INTERPOLATION_BILINEAR "),
        RHVC_INTERPOLATION_NEAREST_NEIGHBOR(RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR, "VALUE_INTERPOLATION_NEAREST_NEIGHBOR "),
        RHVC_RENDER_DEFAULT(RenderingHints.VALUE_RENDER_DEFAULT, "VALUE_RENDER_DEFAULT "),
        RHVC_RENDER_QUALITY(RenderingHints.VALUE_RENDER_QUALITY, "VALUE_RENDER_QUALITY "),
        RHVC_RENDER_SPEED(RenderingHints.VALUE_RENDER_SPEED, "VALUE_RENDER_SPEED "),
        RHVC_STROKE_DEFAULT(RenderingHints.VALUE_STROKE_DEFAULT, "VALUE_STROKE_DEFAULT "),
        RHVC_STROKE_NORMALIZE(RenderingHints.VALUE_STROKE_NORMALIZE, "VALUE_STROKE_NORMALIZE "),
        RHVC_STROKE_PURE(RenderingHints.VALUE_STROKE_PURE, "VALUE_STROKE_PURE "),
        RHVC_TEXT_ANTIALIAS_DEFAULT(RenderingHints.VALUE_TEXT_ANTIALIAS_DEFAULT, "VALUE_TEXT_ANTIALIAS_DEFAULT "),
        RHVC_TEXT_ANTIALIAS_OFF(RenderingHints.VALUE_TEXT_ANTIALIAS_OFF, "VALUE_TEXT_ANTIALIAS_OFF "),
        RHVC_TEXT_ANTIALIAS_ON(RenderingHints.VALUE_TEXT_ANTIALIAS_ON, "VALUE_TEXT_ANTIALIAS_ON ");
        public String name;
        public Object value;

        RenderingHintValueConv(Object value, String name) {
            this.value = value;
            this.name = name;
        }

        public static String valueToName(Object value) {
            for (int i = 0; i < values().length; i++) {
                if (values()[i].value.equals(value)) {
                    return values()[i].name;
                }
            }
            return null;
        }
    }

    //==========================================================================
    //=====================  Grahpics 2D implementation  =======================
    //==========================================================================

    @Override
    public void draw(Shape s) {
        unsupported();
    }

    @Override
    public boolean drawImage(Image img, AffineTransform xform, ImageObserver obs) {
        unsupported();
        return false;
    }

    @Override
    public void drawImage(BufferedImage img, BufferedImageOp op, int x, int y) {
        unsupported();
    }

    @Override
    public void drawRenderedImage(RenderedImage img, AffineTransform xform) {
        unsupported();
    }

    @Override
    public void drawRenderableImage(RenderableImage img, AffineTransform xform) {
        unsupported();
    }

    @Override
    public void drawString(String str, int x, int y) {
        int strId = stringTbl.getID(str);
        if (doLog)
            System.err.println("drawString STR" + strId + "=\""+str+"\" " + x + " " + y);
        writeCmd(DRAW_STRING, strId, x, y);
        if (doDraw) {
            g.drawString(str, x, y);
        }
    }

    @Override
    public void drawString(String str, float x, float y) {
        unsupported();
    }

    @Override
    public void drawString(AttributedCharacterIterator iterator, int x, int y) {
        unsupported();
    }

    @Override
    public void drawString(AttributedCharacterIterator iterator, float x, float y) {
        unsupported();
    }

    @Override
    public void drawGlyphVector(GlyphVector gv, float x, float y) {
        int gvId = gliphVecTbl.getID(gv);
        //System.err.println(" "+gv);
        if (doLog)
            System.err.println("drawGlyphVector GV"+gvId+" "+x+" "+y);
        writeCmd(DRAW_GLYPH_VECTOR, gvId, ftoi(x), ftoi(y));
        if (doDraw)
            g.drawGlyphVector(gv, x, y);
    }

    @Override
    public void fill(Shape s) {
        int shapeId = shapeTbl.getID(s);
        if (doLog)
            System.err.println("fill SHP" + shapeId);
        writeCmd(FILL_SHAPE, shapeId);
        if (doDraw) {
            g.fill(s);
        }
    }

    @Override
    public boolean hit(Rectangle rect, Shape s, boolean onStroke) {
        //System.err.println("hit " + getID(rect) + " " + getID(s) + " " + onStroke);
        return g.hit(rect, s, onStroke);
    }

    @Override
    public GraphicsConfiguration getDeviceConfiguration() {
        //System.err.println("getDeviceConfiguration");
        return g.getDeviceConfiguration();
    }

    @Override
    public void setComposite(Composite comp) {
        unsupported();
    }

    @Override
    public void setPaint(Paint paint) {
        unsupported();
    }

    @Override
    public void setStroke(Stroke s) {
        unsupported();
    }

    @Override
    public void setRenderingHint(Key hintKey, Object hintValue) {
        String keyName = RenderingHintKeyConv.keyToName(hintKey);
        String valueName = RenderingHintValueConv.valueToName(hintValue);
        if (doLog)
            System.err.println("setRenderingHint " + keyName + " " + valueName);
        g.setRenderingHint(hintKey, hintValue);
    }

    @Override
    public Object getRenderingHint(Key hintKey) {
        //System.err.println("getRenderingHint "+getID(hintKey));
        return g.getRenderingHint(hintKey);
    }

    @Override
    public void setRenderingHints(Map<?, ?> hints) {
        Iterator<?> it = hints.keySet().iterator();
        while (it.hasNext()) {
            Key key = (Key) it.next();
            String keyName = RenderingHintKeyConv.keyToName(key);
            String valueName = RenderingHintValueConv.valueToName(hints.get(key));
            if (doLog)
                System.err.println("setRenderingHint " + keyName + " " + valueName);
        }
        g.setRenderingHints(hints);
    }

    @Override
    public void addRenderingHints(Map<?, ?> hints) {
        unsupported();
        g.addRenderingHints(hints);
    }

    @Override
    public RenderingHints getRenderingHints() {
        return g.getRenderingHints();
    }

    @Override
    public void translate(int x, int y) {
        unsupported();
    }

    @Override
    public void translate(double tx, double ty) {
        if (doLog)
            System.err.println("translate " + tx + " " + ty);
        writeCmd(TRANSLATE, dtoi(tx), dtoi(ty));
        g.translate(tx, ty);
    }

    @Override
    public void rotate(double theta) {
        if (doLog)
            System.err.println("rotate " + theta);
        writeCmd(ROTATE, dtoi(theta));
        g.rotate(theta);
    }

    @Override
    public void rotate(double theta, double x, double y) {
        if (doLog)
            System.err.println("rotate " + x + " " + y);
        writeCmd(ROTATE_XY, dtoi(theta), dtoi(x), dtoi(y));
        g.rotate(theta, x, y);
    }

    @Override
    public void scale(double sx, double sy) {
        if (doLog)
            System.err.println("scale " + sx + " " + sy);
        writeCmd(SCALE, dtoi(sx), dtoi(sy));
        g.scale(sx, sy);
    }

    @Override
    public void shear(double shx, double shy) {
        unsupported();
    }

    @Override
    public void transform(AffineTransform Tx) {
        unsupported();
    }

    @Override
    public void setTransform(AffineTransform Tx) {
        int atId = affTranTbl.getID(Tx);
        if (doLog)
            System.err.println("setTransform TRAN" + atId + " = " + Tx);
        writeCmd(SET_TRANSFORM, atId);
        g.setTransform(Tx);
    }

    @Override
    public AffineTransform getTransform() {
        return g.getTransform();
    }

    @Override
    public Paint getPaint() {
        return g.getPaint();
    }

    @Override
    public Composite getComposite() {
        unsupported();
        return g.getComposite();
    }

    @Override
    public void setBackground(Color color) {
        unsupported();
    }

    @Override
    public Color getBackground() {
        return g.getBackground();
    }

    @Override
    public Stroke getStroke() {
        return g.getStroke();
    }

    @Override
    public void clip(Shape s) {
        unsupported();
    }

    @Override
    public FontRenderContext getFontRenderContext() {
        return g.getFontRenderContext();
    }

    @Override
    public Graphics create() {
        unsupported();
        return null; //new MetaGraphics2D(g, doDraw);
    }

    @Override
    public Color getColor() {
        return g.getColor();
    }

    @Override
    public void setColor(Color c) {
        int clrId = colorTbl.getID(c);
        if (doLog)
            System.err.println("setColor CLR" + clrId);
        writeCmd(SET_COLOR, clrId);
        g.setColor(c);
    }

    @Override
    public void setPaintMode() {
        unsupported();
        g.setPaintMode();
    }

    @Override
    public void setXORMode(Color c1) {
        unsupported();
    }

    @Override
    public Font getFont() {
        return g.getFont();
    }

    @Override
    public void setFont(Font font) {
        int fontId = fontTbl.getID(font);
        if (doLog)
            System.err.println("setFont FONT" + fontId);
        writeCmd(SET_FONT, fontId);
        g.setFont(font);
    }

    @Override
    public FontMetrics getFontMetrics(Font f) {
        return g.getFontMetrics();
    }

    @Override
    public Rectangle getClipBounds() {
        return g.getClipBounds();
    }

    @Override
    public void clipRect(int x, int y, int width, int height) {
        unsupported();
    }

    @Override
    public void setClip(int x, int y, int width, int height) {
        unsupported();
    }

    @Override
    public Shape getClip() {
        unsupported();
        return g.getClip();
    }

    @Override
    public void setClip(Shape clip) {
        unsupported();
        g.setClip(clip);
    }

    @Override
    public void copyArea(int x, int y, int width, int height, int dx, int dy) {
        unsupported();
    }

    @Override
    public void drawLine(int x1, int y1, int x2, int y2) {
        unsupported();
    }

    @Override
    public void fillRect(int x, int y, int width, int height) {
        if (doLog)
            System.err.println("fillRect " + x + " " + y + " " + width + " " + height);
        writeCmd(FILL_RECT, x, y, width, height);
        if (doDraw) {
            g.fillRect(x, y, width, height);
        }
    }

    @Override
    public void clearRect(int x, int y, int width, int height) {
        unsupported();
    }

    @Override
    public void drawRoundRect(int x, int y, int width, int height,
                              int arcWidth, int arcHeight) {
        unsupported();
    }

    @Override
    public void fillRoundRect(int x, int y, int width, int height,
                              int arcWidth, int arcHeight) {
        unsupported();
    }

    @Override
    public void drawOval(int x, int y, int width, int height) {
        unsupported();
    }

    @Override
    public void fillOval(int x, int y, int width, int height) {
        unsupported();
    }

    @Override
    public void drawArc(int x, int y, int width, int height, int startAngle,
                        int arcAngle) {
        unsupported();
    }

    @Override
    public void fillArc(int x, int y, int width, int height, int startAngle,
                        int arcAngle) {
        unsupported();
    }

    @Override
    public void drawPolyline(int[] xPoints, int[] yPoints, int nPoints) {
        unsupported();
    }

    @Override
    public void drawPolygon(int[] xPoints, int[] yPoints, int nPoints) {
        unsupported();
    }

    @Override
    public void fillPolygon(int[] xPoints, int[] yPoints, int nPoints) {
        unsupported();
    }

    @Override
    public boolean drawImage(Image img, int x, int y, ImageObserver observer) {
        unsupported();
        return false;
    }

    @Override
    public boolean drawImage(Image img, int x, int y, int width, int height,
                             ImageObserver observer) {
        unsupported();
        return false;
    }

    @Override
    public boolean drawImage(Image img, int x, int y, Color bgcolor,
                             ImageObserver observer) {
        unsupported();
        return false;
    }

    @Override
    public boolean drawImage(Image img, int x, int y, int width, int height,
                             Color bgcolor, ImageObserver observer) {
        unsupported();
        return false;
    }

    @Override
    public boolean drawImage(Image img, int dx1, int dy1, int dx2, int dy2,
                             int sx1, int sy1, int sx2, int sy2, ImageObserver observer) {
        unsupported();
        return false;
    }

    @Override
    public boolean drawImage(Image img, int dx1, int dy1, int dx2, int dy2,
                             int sx1, int sy1, int sx2, int sy2, Color bgcolor,
                             ImageObserver observer) {
        unsupported();
        return false;
    }

    @Override
    public void dispose() {
        g.dispose();
    }
}
