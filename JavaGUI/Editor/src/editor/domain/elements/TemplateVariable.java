/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.LabelDecor;
import editor.domain.ListRenderable;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.values.EvaluatedFormula;
import editor.domain.values.UnevaluatedFormula;
import java.awt.geom.Point2D;
import java.io.Serializable;
import javax.swing.Icon;
import latex.LatexFormula;
import org.w3c.dom.Element;

/** A template ID.
 *
 * @author elvio
 */
public class TemplateVariable extends BaseID implements Serializable {
    
    public static enum Type implements Serializable, ListRenderable {
        INTEGER("Int"), REAL("Real"), ACTION("Action"), 
        STATEPROP("State proposition"), COLOR_CLASS("Color class/domain."),
        INTEGER_LIST("Int list"), REAL_LIST("Real list");
        
        private final String descr;
        private Type(String descr) {
            this.descr = descr;
        }
        @Override public String getDescription() { return descr; }
        @Override public Icon getIcon16() { return null; } 
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    public static final Type[] GSPN_TEMPLATE_VAR_TYPES = { Type.INTEGER, Type.REAL/*, Type.COLOR_CLASS*/ };
    public static final Type[] DTA_TEMPLATE_VAR_TYPES = { Type.INTEGER, Type.REAL, Type.ACTION, Type.STATEPROP };
    
    private Type type = Type.INTEGER;
    private static final float TEMPLATE_VAR_LATEX_SIZE = 0.9f;
    
//    private final TemplateVarLabel nameLabel;  // Not shown directly as a Decor
    
    // Last expression that has been bound to this object (used by the simulator)
    private final BoundedExpr lastBinding = new BoundedExpr("");
    
    
    
//    class TemplateVarLabel extends UniqueNameLabel {
//        public TemplateVarLabel(float logicSize, Point2D pos) {
//            super(logicSize, pos, SwingUtilities.LEFT, SwingUtilities.TOP);
//        }
//        
//        @Override
//        public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile, 
//                                          boolean includeDecors) 
//        {
//            return rect.intersects(getX(), getY(), TemplateVariable.this.getWidth(),
//                                   TemplateVariable.this.getHeight());
//        }
//        
//        @Override
//        public Point2D getEditorCenter() {
//            return new Point2D.Double(getX() + TemplateVariable.this.getWidth() / 2,
//                                      getY() + TemplateVariable.this.getHeight() / 2);
//        }
//        
//        public String getVisualizedVarNameWithoutBrackets() {
//            return super.getVisualizedValue(); 
//        }
//
//        @Override
//        public String getVisualizedValue() {
//            String varName = getVisualizedVarNameWithoutBrackets();
//            return "\\left\\langle "+varName+" \\right\\rangle";
//        }
//    }
    
    // Last bounded expression 
    // NOTE: this expression is not checked during editing, it is used only to 
    // suggest the user a possible binding when starting a simulation.
    public static abstract class AbstractBoundedExpr extends Expr {
        public AbstractBoundedExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Expression bounded to template variable " + varName() + ".";
        }
        @Override protected int getParseFlags() { 
            if (varType() == Type.INTEGER || varType() == Type.REAL)
                return ParserContext.PF_CONST_EXPR;
            return 0; 
        }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            switch (varType()) {
                case INTEGER:
                    return ParserContext.ParserEntryPoint.INT_EXPR;
                case REAL:
                    return ParserContext.ParserEntryPoint.REAL_EXPR;
                case ACTION:
                    return ParserContext.ParserEntryPoint.ACTION_BINDING;
                case STATEPROP:
                    return ParserContext.ParserEntryPoint.BOOLEAN_EXPR;
                case INTEGER_LIST:
                    return ParserContext.ParserEntryPoint.INT_LIST_EXPR;
                case REAL_LIST:
                    return ParserContext.ParserEntryPoint.REAL_LIST_EXPR;
                case COLOR_CLASS:
                    return ParserContext.ParserEntryPoint.COLOR_CLASS_DEF;
                default:
                    throw new IllegalStateException();
            }
        }
        public abstract Type varType();
        public abstract String varName();
    }
    public class BoundedExpr extends AbstractBoundedExpr {
        public BoundedExpr(String expr) { super(expr); }
        @Override public Type varType() { return type; }
        @Override public String varName() { return getUniqueName().toString(); }
    }

    public TemplateVariable() {
        setLabelSequence(new LabelDecor[]{
            new NonEditableTextLabel("\\langle"),
            new ConstNameLabel(TEMPLATE_VAR_LATEX_SIZE),
            new NonEditableTextLabel("\\rangle")
        });
    }
    
    public TemplateVariable(Type type, String uniqueName, String lastBinding, Point2D pos) {
        this();
        initializeNode(pos, uniqueName);
        this.type = type;
        this.lastBinding.setExpr(lastBinding);
    }

        
    public TemplateVariable(ConstantID c) {
        this();
        initializeNode(new Point2D.Double(c.getX(), c.getY()), c.getUniqueName());
        this.type = fromConstType(c.getConstType());
        this.lastBinding.setExpr(c.getConstantExpr().getExpr());
        this.copyGfxPropertiesFrom(c);
    }

    public static ConstantID.ConstType toConstType(Type type) {
        switch (type) {
            case INTEGER:
                return ConstantID.ConstType.INTEGER;
            case REAL:
                return ConstantID.ConstType.REAL;
            default:
                return null; // not convertible
        }
    }
    
    public static Type fromConstType(ConstantID.ConstType ctype) {
        switch (ctype) {
            case INTEGER:
                return Type.INTEGER;
            case REAL:
                return Type.REAL;
            default:
                return null; // Not convertible
        }
    }
    
    public boolean isCovertibleToConst() {
        return toConstType(type) != null;
    }
    
    @Override public int getUniqueNameFontStyle() { 
        switch (type) {
            case INTEGER:
            case REAL:
                return STYLE_ITALIC; 
            case ACTION:
                return STYLE_ITALIC;
            case STATEPROP:
                return STYLE_BOLD;
            case COLOR_CLASS:
                return STYLE_ROMAN;
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        // no expressions to rewrite
    }
    
    public ColorClass getColorDomain() {
        return NeutralColorClass.INSTANCE;
    }
    
    public EvaluatedFormula.Type getEvaluationType() {
        switch (type) {
            case INTEGER:   return EvaluatedFormula.Type.INT;
            case REAL:      return EvaluatedFormula.Type.REAL;
        }
        throw new UnsupportedOperationException();
    }
    
    public EvaluatedFormula evaluate(ParserContext context, EvaluationArguments args) {
        if (context.partialEvaluation)
            return new UnevaluatedFormula(getEvaluationType(), getColorDomain(), getUniqueName());
        if (context.templateVarsBinding == null)
            throw new EvaluationException("Net parameters are not instantated.");
        
        Expr varExpr = context.templateVarsBinding.getSingleValueBoundTo(this);
        if (varExpr == null) {
            throw new EvaluationException("Parameter "+getUniqueName()+" is not instantiated.");
        }
        EvaluatedFormula value = varExpr.evaluate(context.bindingContext, args);
        
        // Verify thhat the bound value has the same type of the template parameter.
        if (value.getType() != getEvaluationType())
            throw new EvaluationException("Binding of "+getEvaluationType().name()+
                    " variable "+getUniqueName()+" has "+ " type "+value.getType());
        return value;
    }
    
//    @Override
//    public String getVisualizedUniqueName() {
//        return nameLabel.getVisualizedValue();
//    }
    
//    public String getTemplateVariableUniqueName() {
//        return getNameLabel().getVisualizedValue();
//    }
    
    public LatexFormula getTemplateVariableLatexFormula() {
//        return nameLabel.getLatexFormula();
        return getVisualizedLatex();
    }

//    @Override public double getWidth()  { return nameLabel.getWidth(); }
//    @Override public double getHeight() { return nameLabel.getHeight(); }
//    
//    @Override public double getRotation() { return 0.0; }    
//    @Override
//    public void setRotation(double rotation) { throw new UnsupportedOperationException(); }
//    @Override public boolean mayRotate() { return false; }
//    
//    @Override public double getRoundX() { throw new UnsupportedOperationException(); }
//    @Override public double getRoundY() { throw new UnsupportedOperationException(); }
    
//    @Override public double getActivityValue(ActivityState activity) { return 0.0; }

//    @Override  public EditableCell getUniqueNameEditable() { return nameLabel; }
//    @Override public LabelDecor getUniqueNameDecor() { return null; }
//
//    @Override public ShapeType getShapeType() { return ShapeType.RECTANGLE; }
//    
//    @Override public boolean mayResizeWidth() { return false; }
//    @Override public boolean mayResizeHeight() { return false; }
//    @Override public void setWidth(double newWidth) { throw new UnsupportedOperationException(); }
//    @Override public void setHeight(double newHeight) { throw new UnsupportedOperationException(); }
//
//    @Override public boolean hasShadow() { return false; }
//    @Override public boolean hasSuperPosTags() { return false; }
//    
//    @Override public boolean canConnectEdges() { return false; }
//    
//    @Override public Color getFillColor(ActivityState activity)        { return null; }
//    @Override public Color getFillColorGrayed()  { return null; }
//    @Override public Color getFillColorError()   { return null; }
//    
//    @Override public Color getBorderColor()        { return null; }
//    @Override public Color getBorderColorGrayed()  { return null; }
//    @Override public Color getBorderColorError()   { return null; }
    
    // Set a snap point different from the center point
    @Override public double getSnapX() { return 0; }
    @Override public double getSnapY() { return 0; }

//    @Override
//    public EditableCell getCentralEditable(double px, double py) {
//        return nameLabel;
//    }
//
//    @Override
//    protected void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor,
//                                     boolean errorFlag, Shape nodeShape) 
//    {
//        nameLabel.paintDecorAt(g, dh, getX(), getY(), isGrayed(), errorFlag);
//    }

//    @Override public int getNumDecors() { return 0; }
//    @Override public Decor getDecor(int i) { throw new UnsupportedOperationException(); }

    public Type getType() { return type; }
    
    public EditableValue getTypeEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return type; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object nvalue) {
                return true;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object nvalue) { 
                type = (Type)nvalue;
            }
            @Override public boolean isEditable() { 
                return true;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }

    public Expr getLastBindingExpr() { return lastBinding; }  
    
//    @Override public GroupClass getGroupClass() { 
//        switch (type) {
//            case INTEGER:
//            case REAL:
//                return GroupClass.CONSTANT;
//                
//            case COLOR_CLASS:
//                return GroupClass.COLOR_CLASS;
//        }
//        return GroupClass.NON_GROUPABLE;
//    }
    @Override public boolean hasSuperPosTags() { return false; }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        bindXMLAttrib(this, el, exDir, "type", "type", null, Type.class);
        bindXMLAttrib(this, el, exDir, "last-binding", "lastBinding.@Expr", "");
    }
}
