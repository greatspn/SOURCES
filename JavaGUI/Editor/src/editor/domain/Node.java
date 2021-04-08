/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import static editor.domain.NetObject.ACTIVE_COLOR;
import static editor.domain.NetObject.isDouble;
import editor.domain.grammar.ExprIdReplacer;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.io.XmlExchangeable;
import editor.domain.measures.ComputedScalar;
import editor.domain.play.ActivityState;
import editor.gui.NoOpException;
import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.RoundRectangle2D;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Map;
import java.util.UUID;
import javax.swing.Icon;
import javax.swing.SwingConstants;
import latex.LatexFormula;
import org.w3c.dom.Element;
import editor.domain.semiflows.PTFlows;

/** A Node in a graph (a place, a transition, a location, etc...)
 *
 * @author Elvio
 */
public abstract class Node extends SelectableObject 
        implements Serializable, DecorHolder, PlaceableObject, ResourceHolder, XmlExchangeable//, NodeGroup
{
    // Position
    private Point2D pos = new Point2D.Double();
    
    // Unique name and alternate (LaTeX) name
    private String uniqueName = null;
    private String alternateName = null;
    private AlternateNameFunction altNameFn = AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS;
    
    // Edge attachment model
    private static final EdgeAttachmentModel DEFAULT_ATTACHMENT_MODEL = EdgeAttachmentModel.CENTER_ONLY;
    private EdgeAttachmentModel edgeAttachmentModel = DEFAULT_ATTACHMENT_MODEL;
    
    public Node() { }

    protected final void initializeNode(Point2D pos, String uniqueName) {
        assert isAlphanumericIdentifier(uniqueName);
        this.pos.setLocation(pos);
        setUniqueName(uniqueName);
    }
    
    public abstract double getWidth();
    public abstract double getHeight();
    public abstract double getDefaultWidth();
    public abstract double getDefaultHeight();
    public abstract double getRotation();
    public abstract boolean mayRotate();
    public abstract void setRotation(double rotation);
    public abstract double getRoundX();
    public abstract double getRoundY();
    public abstract LabelDecor getUniqueNameDecor();
    public abstract boolean mayResizeWidth();
    public abstract boolean mayResizeHeight();
    public abstract void setWidth(double newWidth);
    public abstract void setHeight(double newHeight);
    public abstract boolean hasShadow();
    
    @Override public boolean isLocked() { return false; }
    public boolean isBackgroundNode() { return false; }
    
    public String getVisualizedUniqueName() {
        return getUniqueNameDecor().getVisualizedValue();
    }
    
    public final double getX() { return pos.getX(); }
    public final double getY() { return pos.getY(); }
    public final double getCenterX() { return pos.getX() + getWidth()/2.0; }
    public final double getCenterY() { return pos.getY() + getHeight()/2.0; }
    public final void setX(double x) { pos.setLocation(x, pos.getY()); }
    public final void setY(double y) { pos.setLocation(pos.getX(), y); }
    
    // Used in play mode to test for active/firable nodes
    public abstract double getActivityValue(ActivityState activity);
    
    public void setNodePosition(double x, double y) {
        pos.setLocation(x, y);
    }
    
    
    public final String getUniqueName() { return uniqueName; }
    public final void setUniqueName(String newUniqueName) {
        uniqueName = newUniqueName;
        uniqueNameHasChanged();
    }
    public void uniqueNameHasChanged() {}
    
    public abstract EditableValue getUniqueNameEditable();
    public int getUniqueNameFontStyle() { return STYLE_ROMAN; }
    
    public final String getAlternateName() { return alternateName; }
    public final void setAlternateName(String newAltName) {
        alternateName = newAltName;
    }

    public AlternateNameFunction getAlternateNameFunction() {
        return altNameFn;
    }
    public void setAlternateNameFunction(AlternateNameFunction altNameFn) {
        this.altNameFn = altNameFn;
    }

//    public final boolean hasSuperPosTags() { return getGroupClass().hasTagSuperposition(); }
    public abstract boolean hasSuperPosTags();
    public abstract Point2D getSuperPosTagsDefaultPos();
    public String getSuperPosTags() { throw new IllegalStateException(); }
    public void setSuperPosTags(String superPosTags) { throw new IllegalStateException(); }
    public LabelDecor getSuperPosTagsDecor() { throw new IllegalStateException(); }
//    @Override
//    public abstract GroupClass getGroupClass();
//
//    @Override public int numSynchNodes() { return 1; }
//    @Override public NodeGroup getSynchNode(int i) { return this; }
//    @Override public int getSynchNodeNetIndex(int i) { return 0; }

    // Precomputed set of splitted tags
    private transient String[] tagList = null;
    public int numTags() { rebuiltTagList(); return tagList.length; }
    public String getTag(int i) { rebuiltTagList(); return tagList[i]; }
    public boolean hasTag(String tag) { 
        rebuiltTagList();
        for (String s : tagList)
            if (s.equals(tag))
                return true;
        return false;
    }
    protected void invalidateTagList() { tagList = null; }
    private void rebuiltTagList() {
        if (tagList != null)
            return;
        if (!hasSuperPosTags()) {
            tagList = new String[0];
            return;
        }
        tagList = getSuperPosTags().split("\\|"); // | is escaped since it is a regex
        int nonEmpty = 0;
        for (int i=0; i<tagList.length; i++) {
            tagList[i] = tagList[i].trim();
            if (tagList[i].isEmpty())
                tagList[i] = null;
            else
                nonEmpty++;
        }
        if (nonEmpty != tagList.length) {
            String[] nTagList = new String[nonEmpty];
            nonEmpty=0;
            for (String s : tagList)
                if (s != null)
                    nTagList[nonEmpty++] = s;
            tagList = nTagList;
        }
    }
    
    
    
    // Support for reference nodes
//    public Node getReferencedNode() { return this; }
//    public boolean isReference() { return false; }
    
//    public void setTags(String[] tags) { }
//    public String[] getTags() { return null; }
    
    
    
    
    
    
    public boolean canConnectEdges() { return true; }
    
    // Should this node be checked for correctness before the other nodes (used for type nodes, like ColorClass'es)
    public void checkNodeCorrectness(NetPage page, ParserContext context) { 
        // Validate super-position tags (if exists)
        if (hasSuperPosTags()) {
            boolean ok = true;
            for (int t=0; t<numTags() && ok; t++) {
                String tag = getTag(t);
                for (int c=0; c<tag.length() && ok; c++) {
                    if (!Character.isLetterOrDigit(tag.charAt(c))) {
                        page.addErrorWarningObject(PageErrorWarning.newError(
                                "Invalid tag '"+tag+"'. "+
                                "Tags list must be a '|'-separated list of alphanumeric identifiers. ", this));
                        ok = false;
                    }
                }
            }
        }
        /* add PageError's to the page */ 
    }
    
    // Apply a rewriting rule to this node
    public abstract void rewriteNode(ParserContext context, ExprRewriter rewriter);
    
    @Override
    public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable) { /* get the resources */ }
    @Override
    public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable) { /* get the resources */ }
    
    public class UniqueNameLabel extends NodeLabelDecor {

        public UniqueNameLabel(float logicSize, Point2D pos, int horizAlign, int vertAlign) {
            super(logicSize, pos, horizAlign, vertAlign);
        }        
        public UniqueNameLabel(float size, Point2D initPos) {
            super(size, initPos, SwingConstants.CENTER, SwingConstants.CENTER);
        }        
        @Override public Object getValue() { return getUniqueName(); }

        @Override public String getVisualizedValue() {
            return getAlternateNameFunction().prepareLatexText(uniqueName, getAlternateName(),
                                                               getUniqueNameFontStyle());
//            String altName = getAlternateName();
//            if (altName != null)
//                return altName;
//            return prepareIdentifierForVisualization(getUniqueName(), getUniqueNameFontStyle());
        }
        
        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            assert page instanceof NetPage;
            NetPage npage = (NetPage)page;
            if (!isAlphanumericIdentifier((String)value))
                return false;
            Node n = npage.getNodeByUniqueName((String)value);
            return (n==Node.this) || (n==null); // check uniqueness
        }
        @Override
        public void setValue(ProjectData project, ProjectPage page, Object value) { 
            String oldName = getUniqueName(), newName = (String)value;
            if (oldName != newName) {
                // Verify uniqueness of the new name
                if (((NetPage)page).getNodeByUniqueName(newName) != null)
                    throw new NoOpException(); // avoid changing the name
            }
            
            ExprIdReplacer rewriter = new ExprIdReplacer(oldName, newName);
            ParserContext context = new ParserContext(page);
            ((NetPage)page).applyRewriteRule(context, rewriter);
            for (int i=0; i<project.getPageCount(); i++)
                project.getPageAt(i).onAnotherPageNodeRenaming(page, oldName, newName);
            setUniqueName(newName);
        }
        @Override public boolean isEditable() { return true; }
        @Override public boolean isCurrentValueValid() { return true; }
        @Override public boolean editAsMultiline() { return false; }

        @Override 
        public boolean isVisible(ViewProfile vp) {
            return vp.viewNames;
        }
    }
    public class AlternateNameEditable implements EditableValue, Serializable {
        @Override public Object getValue() { return getAlternateName(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            return true;
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
            String newAltName = (String)value;
            if (newAltName != null && newAltName.length() == 0)
                newAltName = null;
            setAlternateName(newAltName);
        }
        @Override public boolean isCurrentValueValid() { return true; }
        @Override public boolean isEditable() { return true; }        
    }
    protected AlternateNameEditable altNameEditable = new AlternateNameEditable();
    public EditableValue getAlternateNameEditable() { return altNameEditable; }
    
    public class AlternateNameFunctionEditable implements EditableValue, Serializable {
        @Override public Object getValue() { return getAlternateNameFunction(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            return true;
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
            AlternateNameFunction newANF = (AlternateNameFunction)value;
            if (newANF.requiresAlternateText && alternateName == null) {
                if (newANF==AlternateNameFunction.LATEX_TEXT)
                    // Take the current LaTeX label
                    alternateName = getAlternateNameFunction().prepareLatexText(uniqueName, null, getUniqueNameFontStyle());
                else throw new IllegalStateException();
            }
            else // Forget the currently set alternate name.
                alternateName = null;
            setAlternateNameFunction(newANF);
        }
        @Override public boolean isCurrentValueValid() { return true; }
        @Override public boolean isEditable() { return true; }        
    }
    protected AlternateNameFunctionEditable altNameFnEditable = new AlternateNameFunctionEditable();
    public EditableValue getAlternateNameFunctionEditable() { return altNameFnEditable; }
    
    public class SuperPosTagsLabel extends NodeLabelDecor {

        public SuperPosTagsLabel(float size, Point2D initPos) {
            super(size, initPos, SwingConstants.CENTER, SwingConstants.CENTER);
        }        
        @Override public Object getValue() { return getSuperPosTags(); }

        @Override public String getVisualizedValue() {
            AlternateNameFunction anf = AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS;
            return anf.prepareLatexText(getSuperPosTags(), null, STYLE_ROMAN);
        }
        
        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            return true;
//            String taglist = (String)value;
//            return isValidTagList(taglist);
        }
        @Override
        public void setValue(ProjectData project, ProjectPage page, Object value) { 
            setSuperPosTags((String)value);
        }
        @Override public boolean isCurrentValueValid() { return true; }
        @Override public boolean isEditable() { return true; }
        @Override public boolean editAsMultiline() { return false; }

        @Override
        public boolean isVisible(ViewProfile vp) { 
            return vp.viewSuperPosTags && getSuperPosTags().length() > 0;
        }
    }
    public EditableValue getSuperPosTagsLabelEditable() { return null; }

    public enum ShapeType implements ListRenderable {
        CIRCLE("Circle"), RECTANGLE("Rectangle"), ROUND_RECTANGLE("Round rectangle");
        
        private final String descr;
        private ShapeType(String descr) {
            this.descr = descr;
        }
        @Override public String getDescription() { return descr; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    public abstract ShapeType getShapeType();

    public EdgeAttachmentModel getEdgeAttachmentModel() {
        return edgeAttachmentModel;
    }

    public void setEdgeAttachmentModel(EdgeAttachmentModel edgeAttachmentModel, NetPage page) {
        this.edgeAttachmentModel = edgeAttachmentModel;
        // Update connected edges
        for (Edge e : page.edges)
            e.invalidateIfConnectedTo(Node.this);
    }
    
    // Return the Editable object when the ser double clicks on the node
    @Override
    public abstract EditableCell getCentralEditable(double px, double py);

    // Where is located the snap point (the center, by default)
    public double getSnapX() { return getWidth() / 2; }
    public double getSnapY() { return getHeight() / 2; }
    
    // Center-point movement handle
    @Override
    public MovementHandle getCenterHandle(final NetPage page) {
        return new ShapeHandle() {
            @Override
            public HandlePosition savePosition() {
                return new PointHandlePosition(getX() + getSnapX(), getY() + getSnapY());
            }

            @Override
            public void restorePosition(HandlePosition hp) {
                PointHandlePosition php = (PointHandlePosition)hp;
                setNodePosition(php.point.getX() - getSnapX(), php.point.getY() - getSnapY());
                // Update connected edges
                for (Edge e : page.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }

            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                setNodePosition(x - getSnapX(), y - getSnapY());
                // Update connected edges
                for (Edge e : page.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }
            
            @Override public boolean canMoveTo() { return true; }

            @Override
            public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                              boolean includeDecors) 
            {
                return Node.this.intersectRectangle(rect, viewProfile, includeDecors);
            }
            @Override public EditableCell getEditable() {
                return getCentralEditable(0, 0);
            }
            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { return NODE_GRID; }
        };
    }
        
    public static final int RSZ_LEFT = 0;
    public static final int RSZ_TOP = 0;
    public static final int RSZ_CENTER = 1;
    public static final int RSZ_RIGHT = 2;
    public static final int RSZ_BOTTOM = 2;

    class ResizeHandlePosition extends PointHandlePosition {
        double x, y, w, h;

        public ResizeHandlePosition(double refX, double refY, double x, double y, double w, double h) {
            super(refX, refY);
            this.x = x;
            this.y = y;
            this.w = w;
            this.h = h;
        }
    }
    
    protected boolean symmetricResize() { return true; }
    
    // Resize handle commands
    private static final Color RESIZE_HANDLE_COLOR = new Color(95, 13, 143);
    public static Point2D.Double resizeHandlePt = new Point2D.Double();
    public MovementHandle getResizeHandle(final NetPage page, final int horizCmd, final int vertCmd) {
        if (horizCmd == RSZ_CENTER && vertCmd == RSZ_CENTER)
            return null;
        if (horizCmd != RSZ_CENTER && !mayResizeWidth())
            return null;
        if (vertCmd != RSZ_CENTER && !mayResizeHeight())
            return null;
        return new DraggableHandle() {
            @Override protected DraggableHandle.BoxShape getBoxShape() { return BoxShape.SQUARE; }

            @Override protected Color getHandleColor() { return RESIZE_HANDLE_COLOR; }
            
            @Override protected int getBoxSize() { return 3; }
            

            @Override public HandlePosition savePosition() {
                ResizeHandlePosition rhp = new ResizeHandlePosition(getValX(), getValY(), 
                        pos.getX(), pos.getY(), getWidth(), getHeight());
                rotatePointAroundCenter(rhp.point, rhp.point, getRotation());
                return rhp;
            }

            @Override public void restorePosition(HandlePosition hp) {
                ResizeHandlePosition rhp = (ResizeHandlePosition)hp;
                setNodePosition(rhp.x, rhp.y);
                if (mayResizeWidth())
                    setWidth(rhp.w);
                if (mayResizeHeight())
                    setHeight(rhp.h);
                // Update connected edges
                for (Edge e : page.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }

            @Override public void moveTo(double x, double y, boolean isMultiSelMove) {
                resizeHandlePt.setLocation(x, y);
                rotatePointAroundCenter(resizeHandlePt, resizeHandlePt, -getRotation());
//                System.out.println("x="+x+"  y="+y+"  x'="+resizeHandlePt.x+"  y'="+resizeHandlePt.y);
                setValX(resizeHandlePt.x);
                setValY(resizeHandlePt.y);
                // Update connected edges
                for (Edge e : page.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }

            @Override public boolean canMoveTo() { return true; }

            @Override public EditableCell getEditable() { return null; }

            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { return POINT_GRID; }
            
            private double getValX() {
                switch (horizCmd) {
                    case RSZ_LEFT:   return pos.getX();
                    case RSZ_CENTER: return pos.getX() + getWidth() / 2;
                    case RSZ_RIGHT:  return pos.getX() + getWidth();
                }
                throw new IllegalStateException();
            }
            private double getValY() {
                switch (vertCmd) {
                    case RSZ_TOP:     return pos.getY();
                    case RSZ_CENTER:  return pos.getY() + getHeight()/ 2;
                    case RSZ_BOTTOM:  return pos.getY() + getHeight();
                }
                throw new IllegalStateException();
            }
            private void setValX(double nx) {
                if (horizCmd == RSZ_CENTER)
                    return;
                double dx = nx - getValX();
                if (horizCmd == RSZ_RIGHT)
                    dx = -dx;
                double p1 = pos.getX(), p2 = pos.getX() + getWidth();
                if (symmetricResize()) {
                    p1 += dx;
                    p2 -= dx;
                }
                else {
                    if (horizCmd == RSZ_RIGHT)
                        p2 -= dx;
                    else
                        p1 += dx;
                }
                setNodePosition(Math.min(p1, p2), getY());
                setWidth(Math.abs(p2 - p1));
            }
            private void setValY(double ny) {
                if (vertCmd == RSZ_CENTER)
                    return;
                double dy = ny - getValY();
                if (vertCmd == RSZ_BOTTOM)
                    dy = -dy;
                double p1 = pos.getY(), p2 = pos.getY() + getHeight();
                if (symmetricResize()) {
                    p1 += dy;
                    p2 -= dy;
                }
                else {
                    if (vertCmd == RSZ_BOTTOM)
                        p2 -= dy;
                    else
                        p1 += dy;
                }
                setNodePosition(getX(), Math.min(p1, p2));
                setHeight(Math.abs(p2 - p1));
            }
        };
    }
    
    // Rotation handle
    private class RotationHandlePosition implements HandlePosition {
        public double rotation;
        public Point2D handlePos;
        public RotationHandlePosition(double rotation, Point2D handlePos) {
            this.rotation = rotation;
            this.handlePos = handlePos;
        }
        @Override public double getRefX() { return handlePos.getX(); }
        @Override public double getRefY() { return handlePos.getY(); }
        @Override public Point2D getRefPoint() { return new Point2D.Double(handlePos.getX(), handlePos.getY()); }
    }
    private static final Color ROTATION_HANDLE_COLOR = new Color(0, 171, 14);
    public MovementHandle getRotationHandle(final NetPage page) {
        assert mayRotate();
        return new DraggableHandle() {
            @Override
            protected DraggableHandle.BoxShape getBoxShape() { return BoxShape.LIGHT_CIRCLE; }

            @Override
            protected Color getHandleColor() { return ROTATION_HANDLE_COLOR; }

            @Override
            protected Point2D getGizmoCenter() {
                return new Point2D.Double(getCenterX(), getCenterY());
            }
            
            @Override
            public HandlePosition savePosition() {
                Point2D p = new Point2D.Double(getCenterX() + getRotationShaftLen(), getCenterY());
                rotatePointAroundCenter(p, p, getRotation());
                return new RotationHandlePosition(getRotation(), p);
            }

            @Override
            public void restorePosition(HandlePosition hp) {
                RotationHandlePosition rhp = (RotationHandlePosition)hp;
                setRotation(rhp.rotation);
                // Update connected edges
                for (Edge e : page.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }

            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                double newTheta = -Math.atan2(y - getCenterY(), x - getCenterX());
                setRotation(sanitizeRadiants(newTheta));
                // Update connected edges
                for (Edge e : page.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }
            
            private double getRotationShaftLen() {
                return Math.ceil(2 + 0.5 * Math.max(getHeight(), getWidth()));
            }

            @Override
            public boolean canMoveTo() { return true; }

            @Override
            public EditableCell getEditable() { return null; }

            @Override
            public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { 
                return new MeshGridSize(new Point2D.Double(getCenterX(), getCenterY()), 
                                        getRotationShaftLen());
            }
        };
    }
    
    
    
    

    private Point2D rotatePointAroundCenter(Point2D in, Point2D out, double theta) {
        double sinTheta = Math.sin(theta), cosTheta = Math.cos(theta);
        double inX = in.getX() - getCenterX();
        double inY = in.getY() - getCenterY();
        double x = inX * cosTheta - inY * sinTheta;
        double y = inX * sinTheta + inY * cosTheta;
        out.setLocation(x + getCenterX(), y + getCenterY());
        return out;
    }
//    private static Point2D rotatePointAround(Point2D in, Point2D out, double theta, int centerX, int centerY) {
//        double sinTheta = Math.sin(theta), cosTheta = Math.cos(theta);
//        double inX = in.getX() - centerX;
//        double inY = in.getY() - centerY;
//        double x = inX * cosTheta - inY * sinTheta;
//        double y = inX * sinTheta + inY * cosTheta;
//        out.setLocation(x + centerX, y + centerY);
//        return out;
//    }
    
    
    // Generate the node shape in screen coordinates
    private static Ellipse2D.Double shapeEllipse = new Ellipse2D.Double();    
    private static Rectangle2D.Double shapeRectangle = new Rectangle2D.Double();
    private static RoundRectangle2D.Double shapeRoundRect = new RoundRectangle2D.Double();    
    protected Shape getShape(Graphics2D g) {
        // The graphics2d object is sent to avoid a printing bug in Java.
        // When printing, rectangle2D are converted to roundRectangle2d for more precision.
        switch (getShapeType()) {
            case CIRCLE:
                shapeEllipse.setFrame(getX(), getY(), getWidth(), getHeight());
                return shapeEllipse;
                
            case RECTANGLE:
                boolean isPrinter = false;
                if (g != null) {
                    GraphicsConfiguration gc = g.getDeviceConfiguration();
                    if (gc != null) {
                        isPrinter = (gc.getDevice().getType() == GraphicsDevice.TYPE_PRINTER);
                    }
                    else isPrinter = true; // for instance, PDF printer has no device
                }
                if (isPrinter) {
//                    System.out.println("isPrinter");
                    shapeRoundRect.setRoundRect(getX(), getY(), getWidth(), getHeight(), 0, 0);
                    return shapeRoundRect;
                }
                else {
                    shapeRectangle.setRect(getX(), getY(), getWidth(), getHeight());
                    return shapeRectangle;
                }
                
            case ROUND_RECTANGLE:
                shapeRoundRect.setRoundRect(getX(), getY(), getWidth(), getHeight(), 
                                            getRoundX(), getRoundY());
                return shapeRoundRect;
            default:
                throw new UnsupportedOperationException();
        }        
    }
    
    @Override
    public MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile) {
        // Should return resize & rotate handles
        ArrayList<MovementHandle> handles = new ArrayList<>();
        for (int i=0; i<getNumDecors(); i++) {
            if (!getDecor(i).isVisible(viewProfile))
                continue;
            handles.add(getDecor(i).getCenterHandle(thisPage));
        }
        for (int mx=0; mx<3; mx++)
            for (int my=0; my<3; my++) {
                MovementHandle rmh = getResizeHandle(thisPage, mx, my);
                if (rmh != null)
                    handles.add(rmh);
            }
        if (mayRotate())
            handles.add(getRotationHandle(thisPage));
        return handles.toArray(new MovementHandle[0]);
    }

    // Hit-test function - determines whether a circle of radius @radius 
    // and center @pt intersects the node shape.
    private static RoundRectangle2D.Double roundRectCircleTest = new RoundRectangle2D.Double();
    private static Rectangle2D.Double rectCircleTest = new Rectangle2D.Double();
    private static Point2D.Double ptCircleTest = new Point2D.Double();
    boolean isCircleInside(Point2D pt, double radius) {
        rotatePointAroundCenter(pt, ptCircleTest, -getRotation());
        switch (getShapeType()) {
            case CIRCLE:
                // For now we assume that the Node circle is not an ellipse.
                // Extend to the general case if needed
                assert getWidth() == getHeight();
                double nodeRadius = getWidth() / 2.0;
                return ptCircleTest.distance(getCenterX(), getCenterY()) < nodeRadius + radius;
                
            case RECTANGLE:
                if (radius == 0.0) {
                    rectCircleTest.setRect(getX(), getY(), getWidth(), getHeight());
                    return rectCircleTest.contains(ptCircleTest);
                }
                else {
                    roundRectCircleTest.setRoundRect(getX()-radius, getY()-radius, 
                                                    getWidth()+2*radius, getHeight()+2*radius, 
                                                    radius, radius);
                    return roundRectCircleTest.contains(ptCircleTest);
                }
                
            case ROUND_RECTANGLE:
                roundRectCircleTest.setRoundRect(getX()-radius, getY()-radius, 
                                                 getWidth()+2*radius, getHeight()+2*radius,
                                                 getRoundX() + radius, getRoundY() + radius);
                return roundRectCircleTest.contains(ptCircleTest);
                
            default:
                throw new UnsupportedOperationException();
        }
    }
    
    
    // Test whether the node shape intersects or is inside @rect
    private static RoundRectangle2D.Double roundRect4IntersectTest = new RoundRectangle2D.Double();
    private static Rectangle2D.Double rect4IntersectTest = new Rectangle2D.Double();
    private static Point2D.Double point4IntersectTest = new Point2D.Double();
    @Override
    public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                      boolean includeDecors) 
    {
        double theta = getRotation();
        switch (getShapeType()) {
            case CIRCLE:
                double RX = getWidth()/2, RY = getHeight()/2;
                roundRect4IntersectTest.setRoundRect(rect.getX() - RX, rect.getY() - RY,
                                                    rect.getWidth() + 2*RX,
                                                    rect.getHeight() + 2*RY,
                                                    2*RX, 2*RY);
                point4IntersectTest.setLocation(getCenterX(), getCenterY());
                if (roundRect4IntersectTest.contains(point4IntersectTest))
                    return true;
                break;
                
            case RECTANGLE:
                if (nearZero(theta) || nearZero(theta - Math.PI)) {
                    // The shape rectangle is rotated 0 or 180 degrees
                    rect4IntersectTest.setRect(getX(), getY(), getWidth(), getHeight());
                    if (rect4IntersectTest.intersects(rect))
                        return true;
                }
                else if (nearZero(theta - Math.PI/2) || nearZero(theta - 3*Math.PI/2)) {
                    // The rotation is of 90 or 270 degrees
                    double w = getHeight(), h = getWidth(), cx = getCenterX(), cy = getCenterY();
                    rect4IntersectTest.setRect(cx - w/2, cy - h/2, w, h);
                    if (rect4IntersectTest.intersects(rect))
                        return true;
                }
                else {
                    // General rotated rectangle - use Polygon area test
                    AffineTransform at = new AffineTransform();
                    at.rotate(getRotation(), getCenterX(), getCenterY());
                    rect4IntersectTest.setRect(getX(), getY(), getWidth(), getHeight());
                    Area area = new Area(rect4IntersectTest).createTransformedArea(at);
                    if (area.intersects(rect))
                        return true;
                }
                break;
                
            case ROUND_RECTANGLE:
                if (nearZero(theta) || nearZero(theta - Math.PI)) {
                    // The shape rectangle is rotated 0 or 180 degrees
                    roundRect4IntersectTest.setRoundRect(getX(), getY(), getWidth(), getHeight(), 
                                                         getRoundX(), getRoundY());
                    if (roundRect4IntersectTest.intersects(rect))
                        return true;
                }
                else if (nearZero(theta - Math.PI/2) || nearZero(theta - 3*Math.PI/2)) {
                    // The rotation is of 90 or 270 degrees
                    double w = getHeight(), h = getWidth(), cx = getCenterX(), cy = getCenterY();
                    roundRect4IntersectTest.setRoundRect(cx - w/2, cy - h/2, w, h, 
                                                         getRoundY(), getRoundX());
                    if (roundRect4IntersectTest.intersects(rect))
                        return true;                    
                }
                else {
                    // General rotated rectangle - use Polygon area test
                    AffineTransform at = new AffineTransform();
                    at.rotate(getRotation(), getCenterX(), getCenterY());
                    roundRect4IntersectTest.setRoundRect(getX(), getY(), getWidth(), getHeight(), 
                                                         getRoundX(), getRoundY());
                    Area area = new Area(roundRect4IntersectTest).createTransformedArea(at);
                    if (area.intersects(rect))
                        return true;
                }
                break;
                
            default:
                throw new UnsupportedOperationException();
        }
        
        if (includeDecors) {
            for (int i=0; i<getNumDecors(); i++) {
                if (!getDecor(i).isVisible(viewProfile))
                    continue;
                if (getDecor(i).intersectRectangle(rect, viewProfile, includeDecors))
                    return true;
            }
        }
        return false;
    }
    
    
    // Get the nearest point on the perimeter of a node, trying to preserve the
    // orthogonality of the ideal edge that goes from @P to that point. 
    // For instance, if P.y is between yMin and ymax (bounds of the node shape), 
    // then the nearest point should connect P with an horizontal line.
    private static final Point2D.Double npoRotatedNearPt = new Point2D.Double();
    private static final Point2D.Double npoNearPt = new Point2D.Double();
    private static final Point2D.Double npoRotatedP = new Point2D.Double();
    public final void getNearsetPointOrtho(Point2D.Double out, Point2D P, boolean doBinarySearch) 
    {
//        if (node.isCircleInside(P, 0.0)) {
//            out.setLocation(P);
//            return;
//        }
        double W = getWidth(), H = getHeight(), X = getX(), Y = getY();
        npoRotatedNearPt.x = X + W/2;
        npoRotatedNearPt.y = Y + H/2;
        rotatePointAroundCenter(P, npoRotatedP, -getRotation());
        double roundX, roundY;
        
        switch (getShapeType()) {
            case CIRCLE:
                final double SQRT2HALF = 0.50;//1 - 0.70710678118;
                roundX = SQRT2HALF * W/2;
                roundY = SQRT2HALF * H/2;
                break;
                
            case ROUND_RECTANGLE:
                roundX = getRoundX();
                roundY = getRoundY();
                break;
            case RECTANGLE:
                roundX = 0.0001;
                roundY = 0.0001;
                break;
                
            default:
                throw new UnsupportedOperationException();
        }
        // Clamp point coordinates
        npoRotatedNearPt.x = Math.min(Math.max(npoRotatedP.x, X+roundX), X+W-roundX);
        npoRotatedNearPt.y = Math.min(Math.max(npoRotatedP.y, Y+roundY), Y+H-roundY);
       
        rotatePointAroundCenter(npoRotatedNearPt, npoNearPt, getRotation());
        assert isCircleInside(npoNearPt, 0.0);
        
//        if (!isCircleInside(npoOut2Pt, 0.0)) {
//            System.out.println("!!! NOT INSIDE  P="+P+" npoPt="+npoOut2Pt+" X="+X+" Y="+Y+
//                    " W="+W+" H="+H+"   "+getShapeType()+" "+getUniqueName());  
//            isCircleInside(npoOut2Pt, 0.0);
//        }

        if (!doBinarySearch) {
            // Return the point as-is. In this case, out will be inside the node
            // shape, and not on the perimeter of the node.
            out.setLocation(npoNearPt);
            return;
        }
        
        // Find a point on the node perimeter that is in-between P and NearPt
        final double MAX_PRECISION = 0.0001;
        double k1 = 0.0, k2 = 1.0;
        while (true) {
            double kMid = (k1 + k2) / 2;
            if (Math.abs(kMid - k1) < MAX_PRECISION)
                break;
            linearInterp(P, npoNearPt, out, kMid);
            if (isCircleInside(out, 0.0))
                k2 = kMid;
            else
                k1 = kMid;
        }
    }
    
    
    // Find the nearest magnet to @pos, and return its index (or -1)
    private static Point2D fnmMagnet = new Point2D.Double();
    public int findNearestMagnet(Point2D pos, int zoomLevel) {
        Shape nodeShape = getShape(null);
        double rot = getRotation();
        double maxRadius = screenToLogic(MAGNET_BOX_SIZE, zoomLevel) / 1.8;
        for (int m=0; m<edgeAttachmentModel.numMagnets(); m++) {
            edgeAttachmentModel.getMagnet(fnmMagnet, m, nodeShape);
            rotatePointAroundCenter(fnmMagnet, fnmMagnet, rot);
            if (pos.distance(fnmMagnet) < maxRadius)
                return m;
        }
        return -1;
    }
    
    
    public int getNumMagnets() { 
        return edgeAttachmentModel.numMagnets();
    }

    public void getMagnetPosition(Point2D out, int magNum) {
        Shape nodeShape = getShape(null);
        edgeAttachmentModel.getMagnet(out, magNum, nodeShape);
        rotatePointAroundCenter(out, out, getRotation());
    }
    
    // Generate a modified screen shape for aura/selection drawings
    private static Ellipse2D.Double  expandedEllipse = new Ellipse2D.Double();
//    private static Rectangle2D.Double expandedRectangle = new Rectangle2D.Double();
    private static RoundRectangle2D.Double expandedRoundRectangle = new RoundRectangle2D.Double();
    protected Shape expandShape(Shape shape, double offsetX, double offsetY,
                                double widthGrowth, double heightGrowth) {
        Shape expanded;
        if (shape instanceof Ellipse2D) {
            Ellipse2D e = (Ellipse2D)shape;
            expandedEllipse.setFrame(e.getX() + offsetX, 
                                     e.getY() + offsetY, 
                                     e.getWidth() + widthGrowth, 
                                     e.getHeight() + heightGrowth);
            expanded = expandedEllipse;
        }
        else if (shape instanceof Rectangle2D) {
            Rectangle2D r = (Rectangle2D)shape;
            expandedRoundRectangle.setRoundRect(r.getX() + offsetX, 
                                                r.getY() + offsetY, 
                                                r.getWidth() + widthGrowth, 
                                                r.getHeight() + heightGrowth,
                                                2 * widthGrowth, 2 * heightGrowth);
            expanded = expandedRoundRectangle;
        }
        else if (shape instanceof RoundRectangle2D) {
            RoundRectangle2D r = (RoundRectangle2D)shape;
            expandedRoundRectangle.setRoundRect(r.getX() + offsetX, 
                                                r.getY() + offsetY, 
                                                r.getWidth() + widthGrowth, 
                                                r.getHeight() + heightGrowth,
                                                r.getArcWidth() + 2 * widthGrowth, 
                                                r.getArcHeight() + 2 * heightGrowth);
            expanded = expandedRoundRectangle;
        }
        else throw new UnsupportedOperationException("Add a new shape in drawShadowPass()");
        
        return expanded;
    }
    
    public abstract Color getFillColor(ActivityState activity);
    public abstract Color getFillColorGrayed();
    public abstract Color getFillColorError();
    
    public abstract Color getBorderColor();
    public abstract Color getBorderColorGrayed();
    public abstract Color getBorderColorError();
    
    public DrawHelper.StrokeWidth getBorderWidth() { return DrawHelper.StrokeWidth.BASIC; }
    
    private static Rectangle2D.Double pnCardRect = new Rectangle2D.Double();
    // Paint the node shape
    public void paintNode(Graphics2D g, DrawHelper dh, boolean errorFlag) {
        Shape nodeShape = getShape(g);
        DrawHelper.StrokeWidth sw = getBorderWidth();
        
        // Rotation
        AffineTransform oldAT = g.getTransform();
        applyRotation(g);
        
        // Selection
        if (isSelected() && dh.shouldPaintSelection())
            paintSelection(g, dh, nodeShape); 

        // Aura around the border
        if (isAuraActive())
            paintNodeAura(g, dh, nodeShape);
        
        // Active/fireable aura
        if (dh.activity != null) {
            double activity = getActivityValue(dh.activity);
            if (activity > 0) 
                paintNodeActiveBorder(g, dh, nodeShape, activity, ACTIVE_COLOR);
        }
        if (dh.multiNet != null) {
            Color clr = dh.multiNet.colorOfMergedNode(this);
            if (clr != null)
                paintNodeActiveBorder(g, dh, nodeShape, 0.2, clr);
        }
        
        // Highlighted in visualization mode
        int semiflowCard = -100;
        if (dh.selectedPTFlow != null && dh.selectedPTFlow.contains(this)) {
            semiflowCard = dh.selectedPTFlow.getNodeCardinality(this);
            paintNodeActiveBorder(g, dh, nodeShape, DEFAULT_ACTIVITY_AURA_SIZE, 
                                  dh.selectedPTFlow.getLineColor(semiflowCard));
        }
        
        // Fill the node interior
        Color fillColor;
        if (errorFlag)
            fillColor = getFillColorError();
        else if (isGrayed())
            fillColor = getFillColorGrayed();
        else
            fillColor = getFillColor(dh.activity);
        if (isAuraActive() && getMagnetWithAura() == -1)
            fillColor = blend(fillColor, AURA_COLOR, 0.2);
        if (fillColor != null) {
            g.setColor(fillColor);
            g.fill(nodeShape);
        }
        
        // Draw the border of the shape
        Color borderColor = null;
        Stroke borderStroke = sw.logicStroke; 
        if (errorFlag)
            borderColor = getBorderColorError();
        if (borderColor==null && isGrayed())
            borderColor = getBorderColorGrayed();
        if (borderColor==null)
            borderColor = getBorderColor();
        if (borderColor != null && dh.selectedPTFlow != null) {
            Color selBorderColor = dh.selectedPTFlow.getBorderColor(this);
            if (selBorderColor != null) {
                borderColor = selBorderColor;
                borderStroke = sw.logicStrokeWider;
            }
        }
        if (borderColor != null) {
            Stroke sk = g.getStroke();
            g.setStroke(borderStroke);
            g.setColor(borderColor);
            g.draw(nodeShape);
            g.setStroke(sk);
        }
        
        // Paint the magnets
        if (isAuraActive())
            paintMagnets(g, dh, nodeShape);
        
        // Custom node interior
        paintNodeInterior(g, dh, borderColor, errorFlag, nodeShape);
        
        // P/T semiflows count
        if (dh.selectedPTFlow != null) {
            String countText = null;
            if (dh.selectedPTFlow.getType() == PTFlows.Type.PLACE_BOUNDS_FROM_PINV) {
                int upper = dh.selectedPTFlow.getNodeBound(this, false);
                int lower = dh.selectedPTFlow.getNodeBound(this, true);
                if (upper >= 0 && lower >= 0) {
                    if (upper == Integer.MAX_VALUE)
                        countText = "\\mathbf{["+lower+","+PTFlows.INFINITY_UNICODE+"]}";
                    else
                        countText = "\\mathbf{["+lower+","+upper+"]}";
                }
            }
            else {
                if (dh.selectedPTFlow.contains(this)) {
//                    int card = dh.semiflows.getNodeCardinality(this);
                    if (semiflowCard != 0 && semiflowCard != Integer.MAX_VALUE)
                        countText = "\\mathbf{"+semiflowCard+"}";
                }
            }
            
            if (countText != null)
                paintPTSemiflowsCardinality(g, dh, dh.selectedPTFlow.getTextColor(semiflowCard), 
                                            oldAT, 0.95f, countText);
        }
        
        // Measure value
        if (dh.measures != null) {
            ComputedScalar value = dh.measures.getNodeMeasure(this);
            if (value != null) {
                paintPTSemiflowsCardinality(g, dh, dh.measures.measureColorFor(this),
                                            oldAT, 0.85f, value.toString());
            }
        }
        
        g.setTransform(oldAT);
        
        // Paint decors
        int numDecors = getNumDecors();
        for (int i=0; i<numDecors; i++) {
            Decor decor = getDecor(i);
            if (!decor.isVisible(dh.viewProfile))
                continue;
            decor.paintDecor(g, dh, isGrayed(), errorFlag);
        }
    }
    
    // Custom paint routine (for additional items like place tokens)
    protected abstract void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor,
                                              boolean errorFlag, Shape nodeShape);
    
    
    // Draw a small aura around the node border
    // There are two kinds of aura - for the node (larger) or for a node magnet (smaller)
    private static final Color AURA_COLOR = new Color(255, 106, 0);
    protected void paintNodeAura(Graphics2D g, DrawHelper dh, Shape nodeShape) {
        DrawHelper.StrokeWidth sw = getBorderWidth();
        // The node is the target, not one of its magnets
        boolean auraForNode = (getMagnetWithAura() == -1);
        // Get the aura offset in screen coordinates
        int b = Math.max(1, dh.logicToScreen(DrawHelper.getBaseStrokeWidth() * sw.widthMult));
        //float a = Math.max(dh.pixelSize, DrawHelper.baseStrokeWidth); 
        float a = (float)dh.screenToLogic(b) * (auraForNode ? 1.5f : 1.0f);
        
        // Expand the node shape with the aura offset
        Shape auraShape = expandShape(nodeShape, -a, -a, 2*a, 2*a);
        // Draw the aura
        Stroke sk = g.getStroke();
        Color clr = g.getColor();
        g.setStroke(auraForNode ? new BasicStroke(2 * DrawHelper.getBaseStrokeWidth() * sw.widthMult) : sw.logicStroke);
        g.setColor(AURA_COLOR);
        g.draw(auraShape);
        g.setStroke(sk); 
        g.setColor(clr);
    }
    
    
    // Draw a small aura around the border that indicates that the node
    // is active/fireable. Used in the play mode
    protected void paintNodeActiveBorder(Graphics2D g, DrawHelper dh, Shape nodeShape, double auraSize, Color color) {
        if (auraSize <= 0)
            return;
        
        // Draw the aura
        Stroke sk = g.getStroke();
        Color clr = g.getColor();
        Composite comp = g.getComposite();
        g.setColor(color);
        g.setComposite(ALPHA_25);
        for (int i=0; i<NUM_ACTIVITY_AURA_STEPS; i++) {
            // Expand the node shape with the aura offset
            double a = auraSize * ((i+1.0)/NUM_ACTIVITY_AURA_STEPS);
            Shape auraShape = expandShape(nodeShape, -a, -a, 2*a, 2*a);        
            g.setStroke(new BasicStroke((float)(2 * a)));
            g.draw(auraShape);
        }        
        g.setStroke(sk); 
        g.setColor(clr);
        g.setComposite(comp);
    }
    
    
    private static final Color MAGNET_COLOR = new Color(220, 20, 220);
    private static final Color MAGNET_COLOR_PALE = new Color(235, 160, 185);
    private static Point2D pnaMagnet = new Point2D.Double();
    private static final int MAGNET_BOX_SIZE = 9;
    protected void paintMagnets(Graphics2D g, DrawHelper dh, Shape nodeShape) {
                
        AffineTransform oldAT = g.getTransform();
        Composite oldComp = g.getComposite();
        Color oldClr = g.getColor();
        
        g.setTransform(dh.baseTransform);
        
        // Draw node magnets
        for (int m=0; m<edgeAttachmentModel.numMagnets(); m++) {
            edgeAttachmentModel.getMagnet(pnaMagnet, m, nodeShape);
            rotatePointAroundCenter(pnaMagnet, pnaMagnet, getRotation());
            //dh.drawLogicPoint(g, pnaMagnet.getX(), pnaMagnet.getY(), 0.08);
            boolean isWithAura = (m == getMagnetWithAura());
            boolean isPale = getMagnetWithAura() != -1 && !isWithAura;
            int magX = dh.logicToScreen(pnaMagnet.getX());
            int magY = dh.logicToScreen(pnaMagnet.getY());
            final int halfBox = MAGNET_BOX_SIZE / 2;
            
            if (isWithAura) {
                g.setColor(AURA_COLOR);
                g.setComposite(NetObject.ALPHA_50);
                int b = MAGNET_BOX_SIZE + 6;
                g.fillOval(magX - b/2, magY - b/2, b, b); 
                b = MAGNET_BOX_SIZE + 4;
                g.fillOval(magX - b/2, magY - b/2, b, b); 
            }
            
            g.setComposite(NetObject.ALPHA_75);
            g.setColor(isWithAura ? MAGNET_COLOR : Color.WHITE);
            g.fillOval(magX - halfBox, magY - halfBox, MAGNET_BOX_SIZE, MAGNET_BOX_SIZE); 

            g.setComposite(oldComp);
            g.setColor(isPale ? MAGNET_COLOR_PALE : MAGNET_COLOR);
            g.drawOval(magX - halfBox, magY - halfBox, MAGNET_BOX_SIZE, MAGNET_BOX_SIZE); 
        }
        
        g.setColor(oldClr);
        g.setComposite(oldComp);
        g.setTransform(oldAT);
    }
    
    // Selection border around the node shape
    protected void paintSelection(Graphics2D g, DrawHelper dh, Shape nodeShape) {
        float a = SELECTION_STROKE_WIDTH / 2.1f;
        Shape selectionShape = expandShape(nodeShape, -a, -a, 2*a, 2*a);
        Stroke oldStroke = g.getStroke();
        Color oldColor = g.getColor();
        Composite oldComp = g.getComposite();
        g.setComposite(ALPHA_50);
        g.setStroke(getBorderWidth().logicSelectStrokeNodes);
        g.setColor(isLocked() ? LOCKED_SELECTION_COLOR : SELECTION_COLOR);
        g.draw(selectionShape);
        g.setColor(oldColor);
        g.setStroke(oldStroke);
        g.setComposite(oldComp);
    }
    
    // Paint the cardinality number of a P/T semiflows
    protected void paintPTSemiflowsCardinality(Graphics2D g, DrawHelper dh, Color textColor,
                                               AffineTransform unrotated, float size, String value) 
    {
        LatexFormula cardText = new LatexFormula(value, size  * (float)getUnitToPixels());
        double scaleFact = 1.0 / (double)getUnitToPixels();
        double textX = getCenterX() - (cardText.getWidth() * scaleFact) / 2.0;
        double textY = getCenterY() - (cardText.getHeight() * scaleFact) / 2.0;
        Color clr = g.getColor();
        Stroke sk = g.getStroke();
        AffineTransform at = g.getTransform();
        Composite cmp = g.getComposite();
        final float EXPAND = 0.0f;
        pnCardRect.setRect(textX-EXPAND, textY-EXPAND, 
                           cardText.getWidth()*scaleFact+2*EXPAND, 
                           cardText.getHeight()*scaleFact+2*EXPAND);
        g.setColor(Color.WHITE);
        g.setTransform(unrotated);
        g.setComposite(ALPHA_90);
        g.fill(pnCardRect);
        g.setComposite(cmp);
        g.setColor(textColor);
        g.setStroke(getBorderWidth().logicStroke);
//        g.draw(pnCardRect);
        cardText.draw(g, textX, textY, scaleFact, false);
        g.setColor(clr);
        g.setStroke(sk);
        g.setTransform(at);
        g.setComposite(cmp);
    }
    
    private void applyRotation(Graphics2D g) {
        double theta = 0.0;
        if (mayRotate())
            theta = getRotation();
        if (theta != 0.0) 
            g.rotate(theta, getCenterX(), getCenterY());
    }
    
    // cast a shadow behind the node shape
    private static final int NUM_SHADOW_PASSES = 3;
    private static Composite SHADOW_ALPHA = AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.05f);
    public void paintNodeShadow(Graphics2D g, DrawHelper dh) {
        if (!hasShadow())
            return;
        AffineTransform oldAT = g.getTransform();
        Shape nodeShape = getShape(null);
        final float shadowUnit = 0.2f;
        Composite oldComp = g.getComposite();
        g.setComposite(SHADOW_ALPHA);
        g.setColor(Color.BLACK);
        for (int i=0; i<NUM_SHADOW_PASSES; i++) {
            float a = shadowUnit * ((i+1) / (float)NUM_SHADOW_PASSES);
            //g.setTransform(dh.logic2screen);
            g.setTransform(oldAT);
            g.translate(0, a);
            applyRotation(g);
            Shape shadow = expandShape(nodeShape, -a, -a, 2*a, 2*a);
            g.fill(shadow);
        }
        g.setComposite(oldComp);
        g.setTransform(oldAT);
    }

    // Red/Write PNPRO data
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "name", "@UniqueName", null, String.class);
        // Compatibility - convert field name
        if (exDir.XmlToFields() && 
            el.hasAttribute("alt-name-fn") && el.getAttribute("alt-name-fn").equals("latex"))
            el.setAttribute("alt-name-fn", AlternateNameFunction.LATEX_TEXT.name());
        
        bindXMLAttrib(this, el, exDir, "alt-name-fn", "@AlternateNameFunction", AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS);
        if ((exDir.XmlToFields() && el.hasAttribute("shown-as")) ||
            (exDir.FieldsToXml() && alternateName != null))
            bindXMLAttrib(this, el, exDir, "shown-as", "@AlternateName", null, String.class);
        bindXMLAttrib(this, el, exDir, "x", "@X", null, double.class);
        bindXMLAttrib(this, el, exDir, "y", "@Y", null, double.class);
        if (mayResizeWidth())
            bindXMLAttrib(this, el, exDir, "width", "@Width", getDefaultWidth());
        if (mayResizeHeight())
            bindXMLAttrib(this, el, exDir, "height", "@Height", getDefaultHeight());
        if (mayRotate())
            bindXMLAttrib(this, el, exDir, "rotation", "@Rotation", 0.0);
        if (canConnectEdges())
            bindXMLAttrib(this, el, exDir, "magnets", "edgeAttachmentModel", DEFAULT_ATTACHMENT_MODEL);
        if (hasSuperPosTags()) {
            bindXMLAttrib(this, el, exDir, "superposition-tags", "@SuperPosTags", "");
            bindXMLAttrib(this, el, exDir, "superpos-x", "@SuperPosTagsDecor.@InternalPosX", getSuperPosTagsDefaultPos().getX());
            bindXMLAttrib(this, el, exDir, "superpos-y", "@SuperPosTagsDecor.@InternalPosY", getSuperPosTagsDefaultPos().getY());
        }
    }
    
    public abstract class NodeLabelDecor extends LabelDecor {

        public NodeLabelDecor(float logicSize, Point2D pos, int horizAlign, int vertAlign) {
            super(logicSize, pos, horizAlign, vertAlign);
        }

        @Override
        public void getAttachPoint(Point2D pt, double K) {
            // The anchor point of a node is its center
            pt.setLocation(getCenterX(), getCenterY());
        }
        
        @Override public boolean drawLineBetweenDecorAndEdge() { 
            throw new UnsupportedOperationException("should not be here"); 
        }        
    }
    
    
    public EditableValue getWidthEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return ""+getWidth(); }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return isDouble((String)value);
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                assert page instanceof NetPage;
                NetPage npage = (NetPage)page;
                setWidth(Double.parseDouble((String)value));
                for (Edge e : npage.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return mayResizeWidth(); }
        };
    }

    public EditableValue getHeightEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return ""+getHeight(); }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return isDouble((String)value);
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                assert page instanceof NetPage;
                NetPage npage = (NetPage)page;
                setHeight(Double.parseDouble((String)value));
                for (Edge e : npage.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return mayResizeHeight(); }
        };
    }
    
    public EditableValue getRotationEditable() {
        return new EditableValue() {
            @Override public Object getValue() { 
                return ""+(Math.round(-getRotation() * NetObject.RAD_TO_DEG + 360) % 360); 
            }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return isDouble((String)value);
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                assert page instanceof NetPage;
                NetPage npage = (NetPage)page;
                setRotation(Math.round(360 -Double.parseDouble((String)value)) * NetObject.DEG_TO_RAD);
                for (Edge e : npage.edges)
                    e.invalidateIfConnectedTo(Node.this);
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return mayRotate(); }
        };
    }
    
    public EditableValue getEdgeAttachmentEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return getEdgeAttachmentModel(); }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                assert page instanceof NetPage;
                setEdgeAttachmentModel((EdgeAttachmentModel)value, (NetPage)page);
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return canConnectEdges(); }
        };
    }
}
