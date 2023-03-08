/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.Main;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ParsedColorSubclass;
import editor.domain.grammar.ExprLangLexer;
import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Dimension;
import java.awt.Stroke;
import java.awt.font.FontRenderContext;
import java.awt.geom.Dimension2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

/** Base class of net objects (nodes, edges)
 *
 * @author Elvio
 */
public class NetObject implements Serializable {
    
    //-------------------------------------------------------------------------
    // Unit conversion routines
    //-------------------------------------------------------------------------
        
    // Pixels occupied by a single UNIT on the screen at 100% zoom
    public static int getUnitToPixels() {
        switch (Main.getUiSize()) {
            case NORMAL:
                return 16;
            case LARGE:
                return 24;
            case LARGER:
                return 32;
            default: 
                throw new UnsupportedOperationException();
        }
    }

//    public int logicToScreen(double s) { return logicToScreen(s, zoomLevel); }
//    public double screenToLogic(int l) { return screenToLogic(l, zoomLevel); }

    //private static final double UNIT_SIZE_MULT = (UNIT_TO_PIXELS * 100.0);
    public static double logicToScreenFract(double s, double zoom) { 
        return (s * (zoom/100.0) * getUnitToPixels()); 
    }
    public static int logicToScreen(double s, double zoom) { 
        return (int)Math.round(logicToScreenFract(s, zoom)); 
    }
    public static double screenToLogic(int l, double zoom) { 
        return (((double)l) / getUnitToPixels() / (zoom/100.0));
    }
    
    public static Dimension logicToScreen(Dimension2D d, int zoom) {
        return new Dimension(logicToScreen(d.getWidth(), zoom), 
                             logicToScreen(d.getHeight(), zoom));
    }
//    public static Dimension2D screenToLogic(Dimension d, int zoom) {
//        return new DoubleDimension(screenToLogic(d.width, zoom), 
//                                   screenToLogic(d.height, zoom));
//    }
    
   
    
    protected static final Color SELECTION_COLOR = new Color(71, 163, 255);
    protected static final Color LOCKED_SELECTION_COLOR = new Color(163, 71, 255);
    public static final float SELECTION_STROKE_WIDTH = 0.25f;
    public static final Composite ALPHA_90 = AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.90f);
    public static final Composite ALPHA_75 = AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.75f);
    public static final Composite ALPHA_50 = AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.50f);
    public static final Composite ALPHA_25 = AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.25f);
    
    
    protected static final Color LABEL_BOX_COLOR = new Color(82, 154, 255);
    protected static final Stroke LABEL_BOX_STROKE = new BasicStroke(1.0f, BasicStroke.CAP_SQUARE, 
                                                                     BasicStroke.JOIN_MITER, 1.0f, 
                                                                     new float[] {3.0f, 3.0f}, 0.0f);
    
    // Activity aura size and colors
    public static final Color ACTIVE_COLOR = new Color(20, 180, 255);
    public static final double DEFAULT_ACTIVITY_AURA_SIZE = 0.15;
    public static final int NUM_ACTIVITY_AURA_STEPS = 5;
    
    // Highlighted elements (semiflow visualization)
    protected static final float[] FLOW_DASHES = {0.3f, 0.4f};
    protected static final float[] HIGHLIGHT_DASHES = {0.2f, 0.5f};
    protected static final int NUM_HIGHLIGHT_PHASES = 10;
    protected static final float HIGHLIGHT_DASH_PHASE_MULT = 0.07f; // sum of dash lengths / num phases
    
    
    public static final Color GRAYED_BLACK = new Color(190, 190, 190);
    
    public static FontRenderContext NET_FRC = new FontRenderContext(null, true/*isAntialiased*/, 
                                                                    true/*useFractionalMetrics*/);
    
    
    //-------------------------------------------------------------------------
    // Helper 2D management functions
    //-------------------------------------------------------------------------
    public static double linearInterp(double v1, double v2, double k) {
        return v1 + k * (v2 - v1);
    }
    public static Point2D linearInterp(Point2D p1, Point2D p2, Point2D out, double k) {
        out.setLocation(linearInterp(p1.getX(), p2.getX(), k),
                        linearInterp(p1.getY(), p2.getY(), k));
        return out;
    }
    public static Point2D linearInterp(double x1, double y1, double x2, double y2,
                                       Point2D out, double k) 
    {
        out.setLocation(linearInterp(x1, x2, k), linearInterp(y1, y2, k));
        return out;
    }
//    protected static Point2D bezierInterp(Point2D p0, Point2D p1, Point2D p2, Point2D p3, 
//                                          Point2D out, double k) 
//    {
//        // NOTE: p1 and ps are the control points
//        assert k >= 0.0 && k <= 1.0;
//        double invK = (1.0 - k);
//        double a = (invK * invK * invK);
//        double b = 3 * (invK * invK * k);
//        double c = 3 * (invK * k * k);
//        double d = (k * k * k);
//        
//        out.setLocation(a*p0.getX() + b*p1.getX() + c*p2.getX() + d*p3.getX(), 
//                        a*p0.getY() + b*p1.getY() + c*p2.getY() + d*p3.getY());
//        return out;
//    }
    public static Point2D bezierInterp(double x1, double y1, double x2, double y2, 
                                       double x3, double y3, double x4, double y4, 
                                       Point2D out, double k) 
    {
        // NOTE: p1 and ps are the control points
        final double EPSILON = 1.0e-5;
        assert k >= -EPSILON && k <= 1.0+EPSILON;
        double invK = (1.0 - k);
        double a = (invK * invK * invK);
        double b = 3 * (invK * invK * k);
        double c = 3 * (invK * k * k);
        double d = (k * k * k);
        
        out.setLocation(a*x1 + b*x2 + c*x3 + d*x4, 
                        a*y1 + b*y2 + c*y3 + d*y4);
        return out;
    }

    public static double distance(Point2D p1, Point2D p2) {
        double dx = p2.getX() - p1.getX();
        double dy = p2.getY() - p1.getY();
        return Math.sqrt(dx*dx + dy*dy);
    }
    public static double clamp(double value, double min, double max) {
        return (value < min) ? min : ((value > max) ? max : value);
    }
    
    public static MovementHandle newPoint2DHandle(final Point2D point, final MeshGridSize mgs) {
        return new DraggableHandle() {
            @Override
            public HandlePosition savePosition() { return new PointHandlePosition(point.getX(), point.getY()); }
            @Override
            public void restorePosition(HandlePosition hp) { 
                PointHandlePosition php = (PointHandlePosition)hp;
                point.setLocation(php.point);
            }
            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                point.setLocation(x, y);
            }
            @Override public boolean canMoveTo() { return true; }
            @Override public BoxShape getBoxShape() { return null; }
            @Override public EditableCell getEditable() {return null; }
            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { return mgs; }
        };
    }
    
    
    private static final int HIT_RECT_SIZE = 3;
    private static final Rectangle2D hitRect = new Rectangle2D.Double();
    public static Rectangle2D makeHitRectangle(Point2D pt, int zoomLevel) {
        double hitSize = NetObject.screenToLogic(HIT_RECT_SIZE, zoomLevel);
        double hitSize2 = NetObject.screenToLogic(HIT_RECT_SIZE*2+1, zoomLevel);
        hitRect.setRect(pt.getX() - hitSize, pt.getY() - hitSize, hitSize2, hitSize2);
        return hitRect;
    }
    public static Rectangle2D makeHitpointRectangle(Point2D pt, int zoomLevel) {
        final int SIZE_IN_PIXELS = 4;
        final double a = screenToLogic(SIZE_IN_PIXELS, zoomLevel);
        hitRect.setRect(pt.getX() - a, pt.getY() - a, 2*a, 2*a);
        return hitRect;
    }
    
    
    public static final double DEG_TO_RAD = Math.PI / 180.0;
    public static final double RAD_TO_DEG = 180.0 / Math.PI;
    
    public static final double DEFAULT_GRID_ANGLE_SNAPPING_DEGREES = 15.0;
    public static final double DEFAULT_MULTISEL_GRID_SNAPPING_DEGREES = 15.0;
    
    // Grid management
    public static class MeshGridSize {
        public Point2D rotationCenter;
        public double rotationShaft;
        public double gridSnapping;

        public MeshGridSize(Point2D rotationCenter, double rotationShaft, double gridSnapping) {
            this.rotationCenter = rotationCenter;
            this.rotationShaft = rotationShaft;
            this.gridSnapping = gridSnapping;
        }
        public boolean isRotation() {
            return this != NODE_GRID && this != POINT_GRID;
        }
    }
//    public enum MeshGridSize {
//        NODE_GRID, POINT_GRID
//    }
    public static final MeshGridSize NODE_GRID = new MeshGridSize(null, 0, 0);
    public static final MeshGridSize POINT_GRID = new MeshGridSize(null, 0, 0);
            
    public static MeshGridSize mcdGridSize(MeshGridSize mgs1, MeshGridSize mgs2) {
        if (mgs1 == POINT_GRID && mgs2 == POINT_GRID)
            return POINT_GRID;
        return NODE_GRID;
    }
    
    
    
    public static boolean almostEqual(double v1, double v2) {
        return nearZero(v1 - v2);
    }
    
    public static boolean isDouble(String str) {
        try {
            Double.parseDouble(str);
            return true;
        }
        catch (NumberFormatException nfe) {
            return false;
        }
    }
    
    public static boolean isInteger(String str) {
        try {
            Integer.parseInt(str);
            return true;
        }
        catch (NumberFormatException nfe) {
            return false;
        }
    }
    
    public static boolean nearZero(double v) {
        final double EPSILON = 1.0e-5;
        return Math.abs(v) < EPSILON;
    }
    
    public static double sanitizeRadiants(double rad) {
        while (rad < 0)
            rad += 2 * Math.PI;
        while (rad > 2 * Math.PI)
            rad -= 2 * Math.PI;
        return rad;
    }
    
    public static Color blend(Color c1, Color c2, double k) {
        return new Color((int)linearInterp(c1.getRed(), c2.getRed(), k),
                         (int)linearInterp(c1.getGreen(), c2.getGreen(), k),
                         (int)linearInterp(c1.getBlue(), c2.getBlue(), k));
    }
    
    public static boolean isAsciiLetter(char ch) {
        return (ch>='a' && ch<='z') || (ch>='A' && ch<='Z');
    }
    public static boolean isAsciiLetterOrDigit(char ch) {
        return (ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9');
    }
    
    // check if the identifier is a well-formed alphanumeric id
    public static boolean isAlphanumericIdentifier(String str) {
        if (str == null || str.length() == 0)
            return false;
        char ch = str.charAt(0);
        if (!isAsciiLetter(ch) && ch != '_')
            return false;
        for (int i=1; i<str.length(); i++) {
            ch = str.charAt(i);
            if (!isAsciiLetterOrDigit(ch) && ch != '_'/* && ch != '\''*/)
                return false; 
        }
        return true;
    }
    
    // check if an alphanumeric identifier is a keyword
    public static boolean isIdentifierKeyword(String id, Class nodeClass) {
        if (keywords == null) {
            // Initialize list of identifiers
            keywords = new HashSet<>();
            for (String kw : ExprLangLexer.tokenNames) {
                if (isAlphanumericIdentifier(kw))
                    keywords.add(kw);
            }
            keywords.addAll(Arrays.asList(extraKeyWords));
        }
        if (keywords.contains(id))
            return true;
        // Special class-dependent identifiers
        if (nodeClass == ColorClass.class ||
            nodeClass == ParsedColorSubclass.class) 
        {
            if (id.equals("S") || id.equals("d"))
                return true;
        }
        return false;
    }
    private static Set<String> keywords = null;
    private static final String[] extraKeyWords = {
        "E", "A", "X", "G", "F", "U", "EX", "EF", "EG", "AX", "AF", "AG", 
        "CN", "If", "when", "ever", "Card", "All", "Subclass",
        "en", "bounds", "deadlock", "ndeadlock", "initial", "possibly", "impossibly",
        "invariantly", "I", "Uniform", "Triangular", "Erlang", "TruncatedExp", "Pareto", 
        "S", /* SWN for <All> */
    };
    
//    // check if the identifier is a well-formed superposition tag (no pipe accepted)
//    public static boolean isSuperpositionTagIdentifier(String str) {
//        if (str == null || str.length() == 0)
//            return false;
//        for (int i=0; i<str.length(); i++) {
//            char ch = str.charAt(i);
//            if (!isAsciiLetterOrDigit(ch) && ch != '_')// && ch != '<' && ch != '>' && ch != ',')
//                return false; 
//        }
//        return true;
//    }
//    
//    // check that the superposition tag list is valid
//    public static boolean isValidTagList(String list) {
//        if (list.length() == 0)
//            return true;
//        String clean = list.replaceAll("\\s", ""); // remove whitespaces
//        String[] tags = clean.split("\\|");
//        if (tags.length == 0)
//            return true; // the empty list is a valid list
//        for (String tag : tags) {
//            if (!isSuperpositionTagIdentifier(tag))
//                return false; // invalid tag
//        }
//        return true;
//    }
    
    // Accepted font styles
    public static final int STYLE_ROMAN = 0;
    public static final int STYLE_ITALIC = 1;
    public static final int STYLE_BOLD = 2;
    public static final int STYLE_BOLD_ITALIC = 3;
    public static final int NUM_STYLES = 4;    
}
