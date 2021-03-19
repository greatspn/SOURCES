/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

import editor.domain.Decor;
import editor.domain.DrawHelper;
import editor.domain.EditableCell;
import editor.domain.EditableValue;
import editor.domain.LabelDecor;
import editor.domain.ListRenderable;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectImageResource;
import editor.domain.ProjectPage;
import editor.domain.ProjectResource;
import editor.domain.ProjectResourceRef;
import editor.domain.ViewProfile;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import static editor.domain.io.XmlExchangeUtils.bindXMLContent;
import editor.domain.play.ActivityState;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.Map;
import java.util.UUID;
import javax.swing.Icon;
import javax.swing.SwingConstants;
import latex.LatexFormula;
import org.w3c.dom.Element;

/**
 *
 * @author elvio
 */
public class TextBox extends Node implements Serializable {
    
    // The text shows inside the box
    private String text;
    
    // The image shown aside the text (the effective image data is a project resource)
    ProjectResourceRef<ProjectImageResource> image = new ProjectResourceRef<>();
    private static final double imageScale = 1.0;
    
    // Various formatting/styling flags and parameters
    private TextSize textSize = TextSize.NORMAL;
    private Color fillClr = null;
    private Color borderClr = Color.BLACK;
    private Color textClr = Color.BLACK;
    private double width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT, rotation = 0.0;
    private ShapeType shapeType = ShapeType.ROUND_RECTANGLE;
    private static final double roundRectXY = 1.0;
    private boolean shadow = true;
    private boolean boldText = true;
    private boolean italicText = false;
    private int horizPos = SwingConstants.CENTER;
    private int vertPos = SwingConstants.TOP;
    private boolean locked = false;
    
    private static final double DEFAULT_WIDTH = 12, DEFAULT_HEIGHT = 8;
    
    public static enum TextSize implements ListRenderable {
        VERY_SMALL("Script size", 0.67f),
        SMALL("Small", 0.78f),
        NORMAL("Normal", 0.9f),
        LARGE("Large", 1.0f),
        VERY_LARGE("Very large", 1.2f),
        HEADER1("Header 1", 1.5f),
        HEADER2("Header 2", 1.75f),
        HEADER3("Header 3", 2.0f);

        private TextSize(String descr, float size) {
            this.descr = descr;
            this.size = size;
        }        
        private final String descr;
        private final float size;

        @Override public String getDescription() { return descr; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
        public float getSize() { return size; }
    }

    public TextBox(String text, Point2D pos, String uniqueName) {
        initializeNode(pos, uniqueName);
        this.text = text;
    }
    
    public TextBox() { }
    
    
    private LatexFormula latexText = null;

    @Override public double getWidth() { return width; }
    @Override public boolean mayResizeWidth() { return true; }
    @Override public void setWidth(double newWidth) { width = newWidth; }
    @Override public double getDefaultWidth() { return DEFAULT_WIDTH; }

    @Override public double getHeight() { return height; }
    @Override public boolean mayResizeHeight() { return true; }
    @Override public void setHeight(double newHeight) { height = newHeight; }
    @Override public double getDefaultHeight() { return DEFAULT_HEIGHT; }

    @Override public double getRotation() { return rotation; }
    @Override public boolean mayRotate() { return true; }
    @Override public void setRotation(double newRotation) { rotation = newRotation; }
    
    @Override public double getActivityValue(ActivityState activity) { return 0.0; }

//    @Override public GroupClass getGroupClass() { return GroupClass.NON_GROUPABLE; }
    @Override public boolean hasSuperPosTags() { return false; }
    @Override public Point2D getSuperPosTagsDefaultPos() { throw new UnsupportedOperationException(""); }
    

    @Override public double getRoundX() { return roundRectXY; }
    @Override public double getRoundY() { return roundRectXY; }
    @Override public ShapeType getShapeType() { return shapeType; }

    @Override public boolean hasShadow() { return fillClr!=null && shadow;}
    @Override public boolean isLocked() { return locked; }
    @Override public boolean isBackgroundNode() { return true; }
    @Override protected boolean symmetricResize() { return false; }

    @Override
    public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable) {
        super.retrieveLinkedResources(resourceTable);
        image.retrieveLinkedResources(resourceTable);
    }

    @Override
    public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable) {
        super.relinkTransientResources(resourceTable); 
        image.relinkTransientResources(resourceTable);
    }
    
    public void setImage(ProjectImageResource imageRes) {
        image.setRef(imageRes);
    }

    public ProjectResourceRef<ProjectImageResource> getImageRef() {
        return image;
    }
    
    @Override
    public LabelDecor getUniqueNameDecor() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public EditableValue getUniqueNameEditable() {
        return null; 
    }
    
    @Override
    public EditableCell getCentralEditable(double px, double py) {
        return new EditableCell() {
            @Override
            public Point2D getEditorCenter() {
                return new Point2D.Double(getCenterX(), getCenterY());
            }
            @Override
            public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile, boolean includeDecors) {
                return TextBox.this.intersectRectangle(rect, viewProfile, includeDecors);
            }
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return text; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                setText(value.toString());
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean editAsMultiline() { return true; }
        };
    }

    @Override
    protected void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor,
                                     boolean errorFlag, Shape nodeShape) 
    {
        LatexFormula lf = getLatexFormula();
        double scaleFact = 1.0 / (double)getUnitToPixels();
        Color oldClr = g.getColor();
        g.setColor(textClr);
        double relTextX = getRelativeTextX();
        lf.draw(g, getX() + relTextX, getY() + getRelativeTextY(), scaleFact, false);
        g.setColor(oldClr);
        if (!image.isNull()) {
            final double SCALE = 128;
            AffineTransform trn = g.getTransform();
            g.scale(1./SCALE, 1./SCALE);
            Object interp = g.getRenderingHint(RenderingHints.KEY_INTERPOLATION);
            g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
            g.drawImage(image.resRef().getImage(), 
                        (int)(SCALE * (getX() + relTextX - getLogicImageWidth())), 
                        (int)(SCALE * (getY() + getRelativeImageY())), 
                        (int)(SCALE * (getLogicImageWidth())), 
                        (int)(SCALE * (getLogicImageHeight())), null);
            g.setTransform(trn);
            g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, interp);
        }
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        // nothing to rewrite.
    }
    
    private LatexFormula getLatexFormula() {
        if (latexText == null) {
            String fmt = "\\text{";
            if (boldText)
                fmt += "\\textbf{";
            if (italicText)
                fmt += "\\textit{";
            fmt += text.replace("\n", "\\\\");
            fmt += "}";
            if (boldText)
                fmt += "}";
            if (italicText)
                fmt += "}";
            latexText = new LatexFormula(fmt, textSize.getSize() * (float)getUnitToPixels());
        }
        return latexText;
    }
    
    public void setText(String newText) {
        text = newText;
        latexText = null;
    }
    public String getText() {
        return text;
    }
    private double getTextWidth() { 
        return getLatexFormula().getWidth() / (double)getUnitToPixels();
    }
    private double getTextHeight() { 
        return getLatexFormula().getHeight() / (double)getUnitToPixels();
    }
    
    private double getRelativeTextX() {
        LatexFormula lf = getLatexFormula();
        double textWidth = getTextWidth();
        if (horizPos == SwingConstants.LEFT)
            return 0 + getLogicImageWidth();
        else if (horizPos == SwingConstants.CENTER)
            return (getWidth() - textWidth + getLogicImageWidth()) / 2.0f;
        else
            return getWidth() - textWidth;
    }
    
    private double getRelativeTextY() {
        LatexFormula lf = getLatexFormula();
        if (vertPos == SwingConstants.TOP)
            return 0;
        else if (vertPos == SwingConstants.CENTER)
            return (getHeight() - getTextHeight()) / 2.0f;
        else
            return getHeight() - getTextHeight();
    }

    public void setTextSize(TextSize textSize) {
        this.textSize = textSize;
        latexText = null;
    }
    
    public double getLogicImageWidth() {
        if (image.isNull())
            return 0;
        return (image.resRef().getImage().getWidth(null) * imageScale) / ((double)getUnitToPixels());
    }
    public double getLogicImageHeight() {
        if (image.isNull())
            return 0;
        return (image.resRef().getImage().getHeight(null) * imageScale) / ((double)getUnitToPixels());
    }
    private double getRelativeImageY() {
        LatexFormula lf = getLatexFormula();
        if (vertPos == SwingConstants.TOP)
            return 0;
        else if (vertPos == SwingConstants.CENTER)
            return (getHeight() - getLogicImageHeight()) / 2.0f;
        else
            return getHeight() - getLogicImageHeight();
    }

    @Override public Color getFillColor(ActivityState activity) { return fillClr; }
    @Override public Color getFillColorGrayed() { return fillClr; }
    @Override public Color getFillColorError() { return Color.RED; }

    @Override public Color getBorderColor() { return borderClr; }
    @Override public Color getBorderColorGrayed() { return borderClr; }
    @Override public Color getBorderColorError() { return Color.RED; }

    
    
    @Override public int getNumDecors() { return 0; }
    @Override public Decor getDecor(int i) {
        throw new UnsupportedOperationException("Should not be called.");
    }

    public int getHorizPos() { return horizPos; }
    public void setHorizPos(int horizPos) { this.horizPos = horizPos; }
    public int getVertPos() { return vertPos; }
    public void setVertPos(int vertPos) { this.vertPos = vertPos; }
    
    
    public EditableValue getHorizPosEditable(final int comparison) {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return horizPos == comparison; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return (value instanceof Boolean);
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                if (value == Boolean.TRUE)
                    horizPos = comparison;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getVertPosEditable(final int comparison) {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return vertPos == comparison; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return (value instanceof Boolean);
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                if (value == Boolean.TRUE)
                    vertPos = comparison;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getTextColorEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return textClr; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                textClr = (Color)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getTextSizeEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return textSize; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                setTextSize((TextSize)value);
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getHasShadowEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return shadow; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                shadow = (Boolean)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getShapeTypeEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return shapeType; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                shapeType = (ShapeType)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getFillColorEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return fillClr; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                fillClr = (Color)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getBorderColorEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return borderClr; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                borderClr = (Color)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getLockEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return locked; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                locked = (Boolean)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getTextBoldEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return boldText; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                boldText = (Boolean)value;
                latexText = null;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getTextItalicEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return italicText; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                italicText = (Boolean)value;
                latexText = null;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    
    public EditableValue getSelectImageEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return true; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                ProjectImageResource img = (ProjectImageResource)value;
                ProjectResource dupImg = project.findDuplicateResource(img);
                if (dupImg != null)
                    img = (ProjectImageResource)dupImg;
//                else
//                    project.addResource(img);
                image.setRef(img);
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getClearImageEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return !image.isNull(); }
            @Override public Object getValue() { return true; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                image.setRef(null);
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); //To change body of generated methods, choose Tools | Templates.
        
        bindXMLContent(this, el, exDir, "text", "");
        bindXMLAttrib(this, el, exDir, "text-size", "textSize", TextSize.NORMAL);
        bindXMLAttrib(this, el, exDir, "fill-color", "fillClr", null, Color.class);
        bindXMLAttrib(this, el, exDir, "border-color", "borderClr", null, Color.class);
        bindXMLAttrib(this, el, exDir, "text-color", "textClr", null, Color.class);
        bindXMLAttrib(this, el, exDir, "shape", "shapeType", ShapeType.RECTANGLE);
        bindXMLAttrib(this, el, exDir, "shadow", "shadow", false);
        bindXMLAttrib(this, el, exDir, "bold", "boldText", false);
        bindXMLAttrib(this, el, exDir, "italic", "italicText", false);
        bindXMLAttrib(this, el, exDir, "horiz-pos", "horizPos", SwingConstants.CENTER);
        bindXMLAttrib(this, el, exDir, "vert-pos", "vertPos", SwingConstants.TOP);
        bindXMLAttrib(this, el, exDir, "locked", "locked", false);
        bindXMLAttrib(this, el, exDir, "italic", "italicText", false);
        // Image ID: read only if XML attribute exists, and write only if image exists.
        if ((exDir.XmlToFields() && el.hasAttribute("image-uuid")) ||
            (exDir.FieldsToXml() && !image.isNull()))
            bindXMLAttrib(this, el, exDir, "image-uuid", "image.@ID", null, UUID.class);
    }
}
