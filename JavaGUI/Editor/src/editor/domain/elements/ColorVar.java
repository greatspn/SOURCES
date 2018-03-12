/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

import editor.domain.Expr;
import editor.domain.LabelDecor;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import static editor.domain.NetObject.STYLE_ITALIC;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.NodeNamespace;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.superposition.GroupClass;
import java.awt.geom.Point2D;
import java.io.Serializable;
import org.w3c.dom.Element;

/** A named color variable defined in a color domain, used for transition bindings.
 * It has form:   var  ID  :  domain
 *
 * @author elvio
 */
public class ColorVar extends BaseID implements Serializable {
    
    class ColorDomainExpr extends Expr {
        public ColorDomainExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Color domain of binding variable " + getUniqueName() + ".";
        }
        @Override protected int getParseFlags() { return ParserContext.PF_CONST_EXPR; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.COLOR_VAR_DEF; 
        }
    }
    
    private final ColorDomainExpr colorDomain = new ColorDomainExpr("");
    
    public Expr getDomainExpr() { return colorDomain; }
//    @Override protected boolean isValueEditable() { return true; }
    
    public ColorVar() {
        setLabelSequence(new LabelDecor[] {
            new NonEditableTextLabel("\\text{\\textbf{var }}"),
            new ConstNameLabel(DEFAULT_TEXT_SIZE),
            new NonEditableTextLabel(" : "),
            new EditableExpressionLabel() {
                @Override protected Expr getValueExpr() { return colorDomain; }
            }
        });
    }
    
    public ColorVar(String name, Point2D pos, String colDomain) {
        this();
        initializeNode(pos, name);
        colorDomain.setExpr(colDomain);
        setLabelSequence(new LabelDecor[] {
            new NonEditableTextLabel("\\text{\\textbf{var }}"),
            new ConstNameLabel(DEFAULT_TEXT_SIZE),
            new NonEditableTextLabel(" : "),
            new EditableExpressionLabel() {
                @Override protected Expr getValueExpr() { return colorDomain; }
            }
        });
    }

    @Override
    public int getUniqueNameFontStyle() {
        return STYLE_ITALIC;
    }
    
    @Override
    public void checkNodeCorrectness(NetPage page, ParserContext context) {
        super.checkNodeCorrectness(page, context);
        colorDomain.checkExprCorrectness(context, page, this);
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        colorDomain.rewrite(context, rewriter);
    }


//    @Override
//    protected String getFormulaHeader() {
//        return "\\text{\\textbf{var }}";
//    }

//    @Override
//    public String getAssignSymbol() {
//        return " : ";
//    }
    
    public ColorClass findColorClass(NodeNamespace nspace) {
        Node nodeClass = nspace.getNodeByUniqueName(colorDomain.getExpr());
        if (nodeClass != null && nodeClass instanceof ColorClass)
             return (ColorClass)nodeClass;
        return null;
    }
    
    @Override public GroupClass getGroupClass() { return GroupClass.COLOR_VAR; }
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        bindXMLAttrib(this, el, exDir, "domain", "colorDomain.@Expr", null, String.class);
    }
}

