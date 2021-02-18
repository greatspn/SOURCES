/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.Decor;
import editor.domain.DrawHelper;
import editor.domain.EditableCell;
import editor.domain.Expr;
import editor.domain.LabelDecor;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.grammar.ParserContext;
import editor.domain.play.ActivityState;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.geom.Point2D;
import java.io.Serializable;
import javax.swing.SwingConstants;
import latex.LatexFormula;

/** A graphical object that represents a sequence of textual labels.
 * Used to represent graphically constants, variables, color definitions, and other.
 * The actual shape of this graphic object is composed by the label sequence.
 *
 * @author elvio
 */
public abstract class BaseID extends Node implements Serializable {
    
    protected static final float VARIABLE_LATEX_SIZE = 0.9f;
        
    // The effectively shown formula (a single LaTeX formula made by the label's formulas).
    // Do not serialize in page snapshots.
    private transient LatexFormula shownFormula;
        
    // The sequence of labels. One of the labels in the sequence must be a ConstNameLabel.
    private LabelDecor[] labelSequence;
        
    class NonEditableTextLabel extends LabelDecor {
        private String labelText;

        public NonEditableTextLabel(String labelText) {
            super(DEFAULT_TEXT_SIZE, new Point2D.Double(0, 0), SwingConstants.CENTER, SwingConstants.CENTER);
            this.labelText = labelText;
        }

        @Override
        public void getAttachPoint(Point2D pt, double K) {
            pt.setLocation(getX() + getXOffsetFor(this), getY());
        }

        @Override
        public boolean drawLineBetweenDecorAndEdge() { return false; }

        @Override
        public boolean isEditable() { return false; }
        @Override
        public Object getValue() { return labelText; }
        @Override
        public boolean isCurrentValueValid() { return true; }
        @Override public boolean editAsMultiline() { return false; }
        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            throw new UnsupportedOperationException("Not supported yet."); 
        }
        @Override
        public void setValue(ProjectData project, ProjectPage page, Object value) {
            if (labelText.equals(value.toString()))
                return;
            labelText = value.toString();
            shownFormula = null;
        }
        
        @Override
        public double getHeight() {
            return BaseID.this.getHeight();
        }
    }
    
    class ConstNameLabel extends Node.UniqueNameLabel {
        public ConstNameLabel(float textSize) {
            super(textSize, new Point2D.Double(0, 0), SwingConstants.LEFT, SwingConstants.TOP);
        }
        @Override
        public void getAttachPoint(Point2D pt, double K) {
            pt.setLocation(getX() + getXOffsetFor(this), getY());
        }
        
        @Override
        public void setValue(ProjectData project, ProjectPage page, Object value) {
            super.setValue(project, page, value); 
            shownFormula = null;
        }

        @Override
        public double getHeight() {
            return BaseID.this.getHeight();
        }
    }
    
    private double getXOffsetFor(LabelDecor label) {
    double offsetX = 0;
        for (LabelDecor l : labelSequence) {
            if (l == label)
                break;
            else
                offsetX += l.getWidth();
        }
        return offsetX;
    }

    
    abstract class EditableExpressionLabel extends LabelDecor {
          abstract Expr getValueExpr();

        public EditableExpressionLabel() {
            super(DEFAULT_TEXT_SIZE, new Point2D.Double(0, 0), SwingConstants.LEFT, SwingConstants.TOP);
        }

        @Override public void getAttachPoint(Point2D pt, double K) {
            pt.setLocation(getX() + getXOffsetFor(this), getY());
        }

        @Override public boolean drawLineBetweenDecorAndEdge() { return false; }
        @Override public Object getValue() { return getValueExpr().getExpr(); }
        @Override public String getVisualizedValue() { return getValueExpr().getVisualizedExpr(); }
        
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object nvalue) {
            ParserContext context = new ParserContext(page);
            return getValueExpr().isValidExpr(context, (String)nvalue);
        }
        @Override public boolean isCurrentValueValid() {
            return getValueExpr().isFormattedAndCorrect();
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object nvalue) { 
            getValueExpr().setExpr((String)nvalue);
            shownFormula = null;
        }
        @Override public final boolean isEditable() { return true; }
        @Override public boolean editAsMultiline() { return false; }
      }
    
    
    public BaseID() { }

    public void setLabelSequence(LabelDecor[] labelSequence) {
        this.labelSequence = labelSequence;
        shownFormula = null;
    }

    @Override
    public void checkNodeCorrectness(NetPage page, ParserContext context) {
        super.checkNodeCorrectness(page, context); 
        shownFormula = null; // Could be different, due to syntactic errors
    }
    
    protected LatexFormula getVisualizedLatex() {
        if (shownFormula == null) {
            // Rebuild the shown latex formula
            StringBuilder sb = new StringBuilder();
            for (LabelDecor l : labelSequence)
                sb.append(l.getVisualizedValue());
            shownFormula = new LatexFormula(sb.toString(), VARIABLE_LATEX_SIZE * getUnitToPixels());
        }
        return shownFormula;
    }
    
    ConstNameLabel getNameLabel() {
        for (LabelDecor l : labelSequence)
            if (l instanceof ConstNameLabel)
                return (ConstNameLabel)l;
        throw new IllegalStateException();
    }
    
    @Override
    public String getVisualizedUniqueName() {
        return getNameLabel().getVisualizedValue();
    }
        
    @Override public double getWidth() { 
        return getVisualizedLatex().getWidth() / (double)getUnitToPixels();
    }

    @Override
    public double getHeight() { 
        return getVisualizedLatex().getHeight()/ (double)getUnitToPixels();
    }

    @Override public double getRotation() { return 0.0; }    
    @Override
    public void setRotation(double rotation) { throw new UnsupportedOperationException(); }
    @Override public boolean mayRotate() { return false; }
    
    @Override public double getRoundX() { throw new UnsupportedOperationException(); }
    @Override public double getRoundY() { throw new UnsupportedOperationException(); }
    
    @Override public double getActivityValue(ActivityState activity) { return 0.0; }

    @Override  public EditableCell getUniqueNameEditable() { return getNameLabel(); }

    @Override public Node.ShapeType getShapeType() { return Node.ShapeType.RECTANGLE; }

    @Override public LabelDecor getUniqueNameDecor() { return null; }
    
    @Override public boolean mayResizeWidth() { return false; }
    @Override public boolean mayResizeHeight() { return false; }
    @Override public double getDefaultWidth() { throw new UnsupportedOperationException("Not supported."); }
    @Override public double getDefaultHeight() { throw new UnsupportedOperationException("Not supported."); }
    @Override public void setWidth(double newWidth) { throw new UnsupportedOperationException(); }
    @Override public void setHeight(double newHeight) { throw new UnsupportedOperationException(); }

    @Override public boolean hasShadow() { return false; }
    
    @Override public Point2D getSuperPosTagsDefaultPos() { throw new UnsupportedOperationException(); }
    
    @Override public boolean canConnectEdges() { return false; }
    
    @Override public Color getFillColor(ActivityState activity)        { return null; }
    @Override public Color getFillColorGrayed()  { return null; }
    @Override public Color getFillColorError()   { return null; }
    
    @Override public Color getBorderColor()        { return null; }
    @Override public Color getBorderColorGrayed()  { return null; }
    @Override public Color getBorderColorError()   { return null; }
    
    // Set a snap point different from the center point
    @Override public double getSnapX() { 
        ConstNameLabel nameLabel = getNameLabel();
        return nameLabel.getWidth() + getXOffsetFor(nameLabel);
    }
    @Override public double getSnapY() { return 0; }

    @Override
    protected void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor, 
                                     boolean errorFlag, Shape nodeShape) 
    {
        double scaleFact = 1.0 / (double)getUnitToPixels();
        Color textColor = Color.BLACK;
        if (isGrayed())
            textColor = GRAYED_BLACK;
        if (errorFlag)
            textColor = Color.RED;
        g.setColor(textColor);
        getVisualizedLatex().draw(g, getX(), getY(), scaleFact, false);
    }
    
    @Override public int getNumDecors() { return 0; }
    @Override public Decor getDecor(int i) { throw new UnsupportedOperationException(); }

    @Override
    public EditableCell getCentralEditable(double px, double py) {
        double scanX = getX();
        for (LabelDecor l : labelSequence) {
            scanX += l.getWidth();
            if (px < scanX)
                return (l.isEditable() ? l : null);
        }
        return null;
    }
}
