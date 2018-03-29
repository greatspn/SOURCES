/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import static editor.domain.NetObject.GRAYED_BLACK;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import javax.swing.SwingConstants;
import latex.LatexFormula;

/** A decoration that is a text string (like a place/transition/location name)
 *
 * @author Elvio
 */
public abstract class LabelDecor extends Decor implements Serializable, EditableCell {
    
    //private String text;
    private float logicSize;
    private Point2D pos;     // position relative to the parent center    
    private int horizAlign;   // SwingConstants .LEFT .RIGHT .CENTER
    private int vertAlign;
    private double edgeK;    // position along the edge

    // precomputed string sizes, in logic units
    // Is kept transient because otherwise it may interfere with page serialization
    // (detects as page changes what is only a cached object allocation).
    private transient LatexFormula latexText;
    //private double width, height;
    //private String lastText;
    
//    public static final float DEFAULT_TEXT_SIZE = 0.9f * 1.25f;
//    public static final float DEFAULT_SMALL_TEXT_SIZE = 0.78f  * 1.25f;
//    public static final float DEFAULT_VERY_SMALL_TEXT_SIZE = 0.70f * 1.25f;
    public static final float DEFAULT_TEXT_SIZE = 0.9f;
    public static final float DEFAULT_SMALL_TEXT_SIZE = 0.78f;
    public static final float DEFAULT_VERY_SMALL_TEXT_SIZE = 0.70f;
    
    public abstract void getAttachPoint(Point2D pt, double K);

    public LabelDecor(float logicSize, Point2D pos, int horizAlign, int vertAlign) {
        this.pos = new Point2D.Double(pos.getX(), pos.getY());
        this.horizAlign = horizAlign;
        this.vertAlign = vertAlign;
        this.logicSize = logicSize;
    }
    
    // prepare the text string as it will be shown by the label, using the getValue() item
    public String getVisualizedValue() { return (String)getValue(); }

//    @Override
//    public void setText(String text) {
//        latexText = new LatexFormula(text, logicSize);
//    }
//    
//    public String getText() {
//        return text;
//    }
    
//    private void computeSize() {
//        final double PT_MULT = 12;
//        if (lastText == null || !lastText.equals(getText())) {
//            Font font = new Font(DrawHelper.FONT_NAME, Font.PLAIN, (int)Math.round(PT_MULT * logicSize));
//            TextLayout layout = new TextLayout(getText(), font, NET_FRC);
//            width = layout.getBounds().getWidth() / PT_MULT;
//            height = layout.getBounds().getHeight() / PT_MULT;
//            lastText = getText();
//        }
//    }
    
    private void prepare() {
        String text = getVisualizedValue();
        if (latexText == null || !text.equals(latexText.getLatex()))
            latexText = new LatexFormula(text, logicSize * (float)getUnitToPixels());
    }
    
    public LatexFormula getLatexFormula() {
        prepare();
        return latexText;
    }

    @Override
    public double getWidth() { 
        prepare(); 
        return latexText.getWidth() / (double)getUnitToPixels(); 
    }

    @Override
    public double getHeight() { 
        prepare(); 
        return Math.max(latexText.getHeight(), latexText.getLineHeight()) / (double)getUnitToPixels();
    }
    
    public int getHorizontalAlignment() { return horizAlign; }
    public int getVerticalAlignment()   { return vertAlign; }

    @Override
    public double getRelativeX() {
        switch (getHorizontalAlignment()) {
            case SwingConstants.RIGHT:
                return getInternalPosX() - getWidth();
            case SwingConstants.CENTER:
                return getInternalPosX() - getWidth()/2;
            case SwingConstants.LEFT:
                return getInternalPosX();
            default:
                throw new IllegalStateException("Unknown horizontal alignment. "+getClass().getName());
        }
    }
    
    @Override
    public double getRelativeY() { 
        switch (getVerticalAlignment()) {
            case SwingConstants.BOTTOM:
                return getInternalPosY() - getHeight();
            case SwingConstants.CENTER:
                return getInternalPosY() - getHeight()/2;
            case SwingConstants.TOP:
                return getInternalPosY();
            default:
                throw new IllegalStateException("Unknown vertical alignment. "+getClass().getName());
        }
    }

    @Override
    public double getEdgeK() { return edgeK; }
    @Override
    public void setEdgeK(double k) { edgeK = k; }
    
    public double getInternalPosX() { return pos.getX(); }
    public double getInternalPosY() { return pos.getY(); }
    public void setInternalPos(double x, double y) { pos.setLocation(x, y); }
    
    public final void setInternalPosX(double x) { setInternalPos(x, getInternalPosY()); }
    public final void setInternalPosY(double y) { setInternalPos(getInternalPosX(), y); }
    
    @Override public void getEffectivePos(Point2D pt) {
        getAttachPoint(attachPoint, getEdgeK());
        pt.setLocation(attachPoint.x + getRelativeX(), attachPoint.y + getRelativeY());
    }

    @Override
    public Point2D getEditorCenter() {
        getAttachPoint(attachPoint, getEdgeK());
        return new Point2D.Double(attachPoint.x + getRelativeX() + getWidth() / 2, 
                                  attachPoint.y + getRelativeY() + getHeight() / 2);
    }

    private static final Point2D.Double attachPoint = new Point2D.Double();
    @Override
    public void paintDecor(Graphics2D g, DrawHelper dh, 
                           boolean isParentGrayed, boolean isParentError) 
    {
        getAttachPoint(attachPoint, getEdgeK());
        paintDecorAt(g, dh, 
                     getRelativeX() + attachPoint.getX(), 
                     getRelativeY() + attachPoint.getY(),
                     isParentGrayed, isParentError);
    }
    
    public Color getTextColorNormal(DrawHelper dh) { return Color.BLACK; }
    public Color getTextColorError(DrawHelper dh) { return Color.RED; }
    public Color getTextColorGrayed(DrawHelper dh) { return GRAYED_BLACK; }
    
    public void paintDecorAt(Graphics2D g, DrawHelper dh, double textX, double textY,
                             boolean isParentGrayed, boolean isParentError)
    {
        prepare(); 
        if (latexText == null)
            return;
        double scaleFact = 1.0 / (double)getUnitToPixels();
        
        Color textColor = getTextColorNormal(dh), oldColor = g.getColor();
        if (isParentError || !isCurrentValueValid())
            textColor = getTextColorError(dh);
        else if (isParentGrayed)
            textColor = getTextColorGrayed(dh);
        g.setColor(textColor);
        double eY = textY;
        if (latexText.getHeight() < latexText.getLineHeight()) {
            double incr = (latexText.getLineHeight() - latexText.getLineDepth()) -
                          (latexText.getHeight() - latexText.getDepth());
            eY += incr / (double)getUnitToPixels();
        }
        //double eY = textY + logicSize * 0.9 - (latexText.getHeight() - latexText.getDepth()) / (double)UNIT_TO_PIXELS;
        latexText.draw(g, textX, eY, scaleFact, false);
        g.setColor(oldColor);
    }

    @Override
    public MovementHandle getCenterHandle(final NetPage thisPage) {
        return new ShapeHandle() {
            @Override
            public HandlePosition savePosition() {
                getAttachPoint(attachPoint, getEdgeK());
                Point2D p = new Point2D.Double(getInternalPosX() + attachPoint.x, 
                                               getInternalPosY() + attachPoint.y);
                return new PointHandlePosition(p);
            }

            @Override
            public void restorePosition(HandlePosition hp) {
                getAttachPoint(attachPoint, getEdgeK());
                PointHandlePosition php = (PointHandlePosition)hp;
                setInternalPos(php.point.getX() - attachPoint.x, 
                               php.point.getY() - attachPoint.y);
            }

            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                getAttachPoint(attachPoint, getEdgeK());
                setInternalPos(x - attachPoint.x, y - attachPoint.y);
            }

            @Override
            public boolean canMoveTo() { return true; }
            
            @Override
            public void moveBy(double Dx, double Dy, boolean isMultiSelMove) {
                setInternalPos(getInternalPosX() + Dx, getInternalPosY() + Dy);
            }

            @Override
            public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                              boolean includeDecors) 
            {
                return LabelDecor.this.intersectRectangle(rect, viewProfile, true);
            }
            
            @Override public EditableCell getEditable() {
                return LabelDecor.this;
            }
            
            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { return POINT_GRID; }
            
            @Override
            public void paintHandle(Graphics2D g, DrawHelper dh, boolean isClicked) {
                super.paintHandle(g, dh, isClicked); 
                
                // draw a rectangle around the decor.
                getAttachPoint(attachPoint, getEdgeK());
                final double b = 0.1;
                int boxX = dh.logicToScreen(getRelativeX() + attachPoint.x - b);
                int boxY = dh.logicToScreen(getRelativeY() + attachPoint.y - b);
                int boxW = dh.logicToScreen(getWidth() + 2*b);
                int boxH = dh.logicToScreen(getHeight() + 2*b);
                Color oldClr = g.getColor();
                Composite oldComp = g.getComposite();
                Stroke oldStroke = g.getStroke();
                AffineTransform oldAT = g.getTransform();
                
                g.setColor(LABEL_BOX_COLOR);
                g.setComposite(isClicked ? ALPHA_50 : ALPHA_25);
                g.setStroke(LABEL_BOX_STROKE);
                g.setTransform(dh.baseTransform);

                g.drawRect(boxX, boxY, boxW, boxH);

                g.setTransform(oldAT);
                g.setStroke(oldStroke);
                g.setComposite(oldComp);
                
//                g.setColor(Color.RED);
//                dh.drawLogicPoint(g, pos.getX() + attachPoint.x, pos.getY() + attachPoint.y, 0.1);
                
                g.setColor(oldClr);                
            }
        };
    }

    @Override
    public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                      boolean includeDecors) 
    {
        getAttachPoint(attachPoint, getEdgeK());
        return intersectRectangleAt(rect, getRelativeX() + attachPoint.x, 
                                    getRelativeY() + attachPoint.y, includeDecors);
    }

    public boolean intersectRectangleAt(Rectangle2D rect, double centerX, double centerY, boolean includeDecors) {
        return rect.intersects(centerX, centerY, getWidth(), getHeight());
    }

    @Override
    public EditableCell getCentralEditable(double px, double py) { return this; }
}
