/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.LabelDecor;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import editor.domain.ListRenderable;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import static editor.domain.elements.TemplateVariable.fromConstType;
import static editor.domain.elements.TemplateVariable.toConstType;
import editor.domain.grammar.EvaluationArguments;
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
import org.w3c.dom.Element;

/** A graphical constant ID object in the form:  ID=value
 *
 * @author elvio
 */
public class ConstantID extends BaseID implements Serializable, ColorClass.DomainHolder {
    
    public enum ConstType implements Serializable, ListRenderable {
        INTEGER("Int"), REAL("Real");
        
        private final String descr;
        private ConstType(String descr) {
            this.descr = descr;
        }
        @Override public String getDescription() { return descr; }
        @Override public Icon getIcon16() { return null; } 
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    class ValueExpr extends Expr {
        public ValueExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Constant expression of " + getUniqueName() + ".";
        }
        @Override protected int getParseFlags() { return ParserContext.PF_CONST_EXPR; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            if (isInNeutralDomain()) {
                return type == ConstType.INTEGER ? ParserContext.ParserEntryPoint.INT_EXPR 
                                                 : ParserContext.ParserEntryPoint.REAL_EXPR; 
            }
            else { // colored domain
                return type == ConstType.INTEGER ? ParserContext.ParserEntryPoint.INT_MULTISET_EXPR 
                                                 : ParserContext.ParserEntryPoint.REAL_MULTISET_EXPR; 
            }
        }
        @Override
        public boolean prepareContext(ParserContext context) {
            if (isInColorDomain()) {
                context.colorDomainOfExpr = getColorDomain();
                return (context.colorDomainOfExpr != null);
            }
            return true;
        }
    }
    
    class ColorDomainExpr extends ColorClass.ColorDomainExprBase {
        public ColorDomainExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Color domain of constant " + getUniqueName() + ".";
        }
    }
    
    private ConstType type = ConstType.INTEGER;
    private final ValueExpr valueExpr = new ValueExpr("1");;
    private final ColorDomainExpr colorDomainExpr = new ColorDomainExpr("");;

    public Expr getConstantExpr() { return valueExpr; }
    
    public ConstantID(ConstType type, String name, String value, String colorDom, Point2D pos) {
        this();
        initializeNode(pos, name);
        this.type = type;
        valueExpr.setExpr(value);
        colorDomainExpr.setExpr(colorDom);
    }
    
    public ConstantID() {
        setLabelSequence(new LabelDecor[]{
            new ConstNameLabel(DEFAULT_TEXT_SIZE),
            new NonEditableTextLabel(" = "),
            new EditableExpressionLabel() {
                @Override protected Expr getValueExpr() { return valueExpr; }
            },
            new EditableExpressionLabel() {
                @Override Expr getValueExpr() { return colorDomainExpr; }
                @Override public String getVisualizedValue() {
                    if (isInNeutralDomain())
                        return "";
                    return " : " + colorDomainExpr.getVisualizedExpr();
                }
            }
        });
    }
    
    public ConstantID(TemplateVariable tv) {
        this();
        initializeNode(new Point2D.Double(tv.getX(), tv.getY()), tv.getUniqueName());
        this.type = toConstType(tv.getType());
        String expr = tv.getLastBindingExpr().getExpr();
        this.valueExpr.setExpr(expr.length() == 0 ? "0" : expr);
        this.setGfxTextSize(tv.getGfxTextSize());
    }

    @Override
    public void checkNodeCorrectness(NetPage page, ParserContext context) {
        super.checkNodeCorrectness(page, context);
        
        if (isInColorDomain())
            colorDomainExpr.checkExprCorrectness(context, page, this);
        
        valueExpr.checkExprCorrectness(context, page, this);
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        if (isInColorDomain())
            colorDomainExpr.rewrite(context, rewriter);
        
        valueExpr.rewrite(context, rewriter);
    }

    
    public EditableValue getTypeEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return type; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object nvalue) {
                ParserContext context = new ParserContext(page);
                ConstType currType = type;
                type = (ConstType)nvalue;
                boolean isValid = valueExpr.isValidExpr(context, valueExpr.getExpr());
                type = currType; //  Set back the actual type.
                return isValid;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object nvalue) { 
                type = (ConstType)nvalue;
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { 
                return true;
//                // The type is editable if it can be parsed as both an INT or a REAL.
//                if (isInteger((String)valueLabel.getValue()))
//                    return true; 
//                return false;
            }
        };
    }

    public ConstType getConstType() {
        return type;
    }
    
    public boolean isCovertibleToTemplate() {
        return fromConstType(type) != null && isInNeutralDomain();
    }
    
    // Return the domain name, or null if it is neutral
    @Override public String getColorDomainName() {
        return colorDomainExpr.getColorDomainName();
    }
    
    // this method return the parsed color domain of this place, which could
    // be the neutral color if the place is uncolored. It returns null if there
    // are syntactic/semantic errors in the place definition.
    @Override public ColorClass getColorDomain() { 
        return colorDomainExpr.getColorDomain(); 
    }
    @Override public boolean isInNeutralDomain() {
        return colorDomainExpr.isInNeutralDomain();
    }
    @Override public boolean isInColorDomain() {
        return colorDomainExpr.isInColorDomain();
    }
    @Override public EditableValue getColorDomainEditable() {
        return colorDomainExpr.getColorDomainEditable();
    }
    
//    @Override public GroupClass getGroupClass() { 
//        return GroupClass.CONSTANT;
//    }
    @Override public boolean hasSuperPosTags() { return false; }
    
    public boolean isIntConst() {
        return type == ConstType.INTEGER;
    }
    public boolean isRealConst() {
        return type == ConstType.REAL;
    }
    
    public EvaluatedFormula.Type getEvaluationType() {
        switch (type) {
            case INTEGER:   return EvaluatedFormula.Type.INT;
            case REAL:      return EvaluatedFormula.Type.REAL;
        }
        throw new UnsupportedOperationException();
    }
        
    public EvaluatedFormula evaluate(ParserContext context) {
        if (context.partialEvaluation && isInNeutralDomain())
            return new UnevaluatedFormula(getEvaluationType(), getColorDomain(), getUniqueName());
        return valueExpr.evaluate(context, EvaluationArguments.NO_ARGS);
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        bindXMLAttrib(this, el, exDir, "consttype", "type", null, ConstType.class);
        bindXMLAttrib(this, el, exDir, "value", "valueExpr.@Expr", null, String.class);
        if (exDir.XmlToFields() || isInColorDomain())
            bindXMLAttrib(this, el, exDir, "domain", "colorDomainExpr.@Expr", "");
    }
}
