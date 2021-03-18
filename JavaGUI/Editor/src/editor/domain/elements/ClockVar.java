/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.LabelDecor;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import java.awt.geom.Point2D;
import java.io.Serializable;
import org.w3c.dom.Element;

/** A clock variable with form:   ID : clock
 *
 * @author elvio
 */
public class ClockVar extends BaseID implements Serializable {
    
//    class ClockDefExpr extends Expr {
//        public ClockDefExpr(String expr) { super(expr); }
//        @Override protected String getExprDescr() { 
//            return "Clock description of " + getUniqueName() + ".";
//        }
//        @Override protected int getParseFlags() { return ParserContext.PF_CONST_EXPR; }
//        @Override protected ParserContext.ParserEntryPoint getParseRule() { 
//            return ParserContext.ParserEntryPoint.CLOCK_DEF; 
//        }
//    }
    
//    private final ClockDefExpr clockDefExpr;
    
    NonEditableTextLabel clockVarDescr;
        
    public ClockVar(String name, Point2D pos) {
        this();
        initializeNode(pos, name);
    }
    public ClockVar() {
        setLabelSequence(new LabelDecor[] {
            new ConstNameLabel(DEFAULT_TEXT_SIZE),
            clockVarDescr = new NonEditableTextLabel(" : \\text{unknown}")
        });
    }

    @Override
    public void uniqueNameHasChanged() {
        clockVarDescr.setValue(null, null, 
                getUniqueName().equals("x") ? " : \\text{clock}" : " : \\text{variable}");
    }

    @Override
    public int getUniqueNameFontStyle() {
        return STYLE_ITALIC;
    }
    
    
//    @Override public GroupClass getGroupClass() {
//        return GroupClass.CLOCK;
//    }
    @Override public boolean hasSuperPosTags() { return false; }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        // No expressions to rewrite
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
        
        // No other fields to exchange...
    }
}
