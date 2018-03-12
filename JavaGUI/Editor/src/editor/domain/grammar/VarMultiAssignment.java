/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.elements.TemplateVariable;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.io.XmlExchangeable;
import editor.domain.measures.MeasurePage;
import java.io.Serializable;
import org.w3c.dom.Element;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;

/** An assignment of an expression or multiple expressions.
 *
 * @author elvio
 */
public class VarMultiAssignment implements Serializable, XmlExchangeable {

    public static enum BindingModel {
        SINGLE_VALUE(" = "),
        MULTIPLE_VALUES(" in "),
        RANGE(" ranges ");
        
        private final String text;

        private BindingModel(String text) {
            this.text = text;
        }
        @Override
        public String toString() {
            return text;
        }
    }
    
    public class AssignmentExpr extends TemplateVariable.AbstractBoundedExpr {
        public AssignmentExpr(String expr) { super(expr); }
        @Override public TemplateVariable.Type varType() { 
            return type;
        }
        @Override public String varName() { 
            return varName;
        }
    }
    public class AssignmentListExpr extends TemplateVariable.AbstractBoundedExpr {
        public AssignmentListExpr(String expr) { super(expr); }
        @Override public TemplateVariable.Type varType() { 
            if (type == TemplateVariable.Type.INTEGER)
                return TemplateVariable.Type.INTEGER_LIST;
            if (type == TemplateVariable.Type.REAL)
                return TemplateVariable.Type.REAL_LIST;
            throw new IllegalStateException("Expr type "+type+" has not a list type.");
        }
        @Override public String varName() { 
            return varName;
        }
    }

    // The name of the assigned template variable
    public String varName;
    // The type of the assigned variable
    public TemplateVariable.Type type;
    // Is the type of the assignment modifiable?
    public boolean allowMultipleValues = true;
    // How the variable is assigned in this multi-assignment
    public BindingModel bindModel;
    // Single value assignment
    public AssignmentExpr singleVal = new AssignmentExpr("");
    // Multiple values assignment
    public AssignmentListExpr multipleVals = new AssignmentListExpr("");
    // Range assignment (int/real variables only)
    public AssignmentExpr rangeFrom = new AssignmentExpr("");
    public AssignmentExpr rangeTo = new AssignmentExpr("");
    public AssignmentExpr rangeStep = new AssignmentExpr("");
    
    public static VarMultiAssignment createSingle(String varName, TemplateVariable.Type type, String singleVal, boolean allowMultipleValues) {
        VarMultiAssignment vma = new VarMultiAssignment(varName, type, BindingModel.SINGLE_VALUE);
        vma.singleVal.setExpr(singleVal);
        vma.allowMultipleValues = allowMultipleValues;
        return vma;
    }
    public static VarMultiAssignment createMulti(String varName, TemplateVariable.Type type, String multipleVals) {
        VarMultiAssignment vma = new VarMultiAssignment(varName, type, BindingModel.MULTIPLE_VALUES);
        vma.multipleVals.setExpr(multipleVals);
        return vma;
    }
    public static VarMultiAssignment createRange(String varName, TemplateVariable.Type type, String from, String to, String step) {
        VarMultiAssignment vma = new VarMultiAssignment(varName, type, BindingModel.RANGE);
        vma.rangeFrom.setExpr(from);
        vma.rangeTo.setExpr(to);
        vma.rangeStep.setExpr(step);
        return vma;
    }
    
    public VarMultiAssignment() { }

    private VarMultiAssignment(String varName, TemplateVariable.Type type, BindingModel bindModel) {
        this.varName = varName;
        this.type = type;
        this.bindModel = bindModel;
    }
    
    public boolean hasRangeAssignment() {
        return (type==TemplateVariable.Type.INTEGER || type==TemplateVariable.Type.REAL);
    }
    
    public void checkAssignmentCorrectness(MeasurePage page, ParserContext context) {
        switch (bindModel) {
            case SINGLE_VALUE:
                singleVal.checkExprCorrectness(context, page, null);
                break;
            case MULTIPLE_VALUES:
                multipleVals.checkExprCorrectness(context, page, null);
                break;
            case RANGE:
                rangeFrom.checkExprCorrectness(context, page, null);
                rangeTo.checkExprCorrectness(context, page, null);
                rangeStep.checkExprCorrectness(context, page, null);
                break;
            default: 
                throw new IllegalStateException();
        }
    }

    @Override
    public String toString() {
        String str = type+" "+varName;
        switch (bindModel) {
            case SINGLE_VALUE:
                str += " = "+singleVal;
                break;
            case MULTIPLE_VALUES:
                str += " = ["+multipleVals+"]";
                break;
            case RANGE:
                str += " ranges from "+rangeFrom+" to "+rangeTo+" step "+rangeStep;
                break;
            default: 
                throw new IllegalStateException();
        }
        return str;
    }
    
    public AssignmentExpr createExpr(String expr) {
        return new AssignmentExpr(expr);
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "varname", "varName", null, String.class);
        bindXMLAttrib(this, el, exDir, "type", "type", null, TemplateVariable.Type.class);
        bindXMLAttrib(this, el, exDir, "bind-model", "bindModel", null, BindingModel.class);
        switch (bindModel) {
            case SINGLE_VALUE:
                bindXMLAttrib(this, el, exDir, "single-val", "singleVal.@Expr", null, String.class);
                break;
            case MULTIPLE_VALUES:
                bindXMLAttrib(this, el, exDir, "multiple-vals", "multipleVals.@Expr", null, String.class);
                break;
            case RANGE:
                bindXMLAttrib(this, el, exDir, "range-from", "rangeFrom.@Expr", null, String.class);
                bindXMLAttrib(this, el, exDir, "range-to", "rangeTo.@Expr", null, String.class);
                bindXMLAttrib(this, el, exDir, "range-step", "rangeStep.@Expr", null, String.class);
                break;
            default:
                throw new UnsupportedOperationException();
        }
        
//        String varName = assignElem.getAttribute("varname");
//        TemplateVariable.Type type = TemplateVariable.Type.valueOf(assignElem.getAttribute("type"));
//        VarMultiAssignment.BindingModel bm = VarMultiAssignment.BindingModel.valueOf(assignElem.getAttribute("bind-model"));
//        VarMultiAssignment assign = null;
//        switch (bm) {
//            case SINGLE_VALUE:
//                assign = VarMultiAssignment.createSingle(varName, type, assignElem.getAttribute("single-val"));
//                break;
//            case MULTIPLE_VALUES:
//                assign = VarMultiAssignment.createMulti(varName, type, assignElem.getAttribute("multiple-vals"));
//                break;
//            case RANGE:
//                assign = VarMultiAssignment.createRange(varName, type, assignElem.getAttribute("range-from"),
//                                                        assignElem.getAttribute("range-to"), 
//                                                        assignElem.getAttribute("range-step"));
//                break;
//            default:
//                throw new UnsupportedOperationException();
//        }

//  assignElem.setAttribute("varname", assign.varName);
//    assignElem.setAttribute("bind-model", assign.bindModel.name());
//    assignElem.setAttribute("type", assign.type.toString());
//    switch (assign.bindModel) {
//        case SINGLE_VALUE:
//            assignElem.setAttribute("single-val", assign.singleVal.getExpr());
//            break;
//        case MULTIPLE_VALUES:
//            assignElem.setAttribute("multiple-vals", assign.multipleVals.getExpr());
//            break;
//        case RANGE:
//            assignElem.setAttribute("range-from", assign.rangeFrom.getExpr());
//            assignElem.setAttribute("range-to", assign.rangeTo.getExpr());
//            assignElem.setAttribute("range-step", assign.rangeStep.getExpr());
//            break;
//        default:
//            throw new UnsupportedOperationException();
//    }
    }
}
