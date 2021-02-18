/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.elements.ParsedColorSubclass;
import editor.domain.values.EvaluatedFormula;
import editor.domain.elements.ClockVar;
import editor.domain.elements.ConstantID;
import editor.domain.Expr;
import editor.domain.elements.ColorClass;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.values.BooleanScalarValue;
import editor.domain.values.MultiSet;
import editor.domain.values.IntScalarValue;
import editor.domain.values.ListOfBounds;
import editor.domain.values.ObjectValue;
import editor.domain.values.RealScalarValue;
import editor.domain.values.ValuedMultiSet;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import org.antlr.v4.runtime.tree.TerminalNode;

/** Evaluates the value of a formula in a given context.
 *
 * @author elvio
 */
public class FormulaEvaluator extends ExprLangBaseVisitor<EvaluatedFormula> {
    
    // The context in which formulas are checked and formatted.
    ParserContext context;
    
    // The arguments passed to the evaluated function.
    EvaluationArguments args;

    public FormulaEvaluator(ParserContext context, EvaluationArguments args) {
        this.context = context;
        this.args = args;
    }
    
    private void requireBoundOfPdfMode() {
        if (!context.evaluatePdfBounds)
            throw new IllegalStateException("Internal error: expression only evaluatable in bound mode.");
    }

    //==========================================================================
    //  Entry points:
    //==========================================================================
    
    private void requireCompleteEvaluation(TerminalNode EOF) {
        if (EOF == null)
            throw new IllegalStateException("Internal error: incomplete evaluation of expression.");        
    }
    
    @Override
    public EvaluatedFormula visitMainRealExpr(ExprLangParser.MainRealExprContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.realExpr());
    }

    @Override
    public EvaluatedFormula visitMainBoolExpr(ExprLangParser.MainBoolExprContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.boolExpr());
    }

    @Override
    public EvaluatedFormula visitMainIntExpr(ExprLangParser.MainIntExprContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.intExpr());
    }

    @Override
    public EvaluatedFormula visitMainIntExprList(ExprLangParser.MainIntExprListContext ctx) {
        List<ExprLangParser.IntExprContext> intExprs = ctx.intExpr();
        ArrayList<Integer> array = new ArrayList<>();
        Iterator<ExprLangParser.IntExprContext> it = intExprs.iterator();
        while (it.hasNext()) {
            ExprLangParser.IntExprContext expr = it.next();
            array.add(visit(expr).getScalarInt());
        }
        int[] intList = new int[array.size()];
        for (int i=0; i<intList.length; i++)
            intList[i] = array.get(i);
        return new ObjectValue(intList);
    }

    @Override
    public EvaluatedFormula visitMainRealExprList(ExprLangParser.MainRealExprListContext ctx) {
        List<ExprLangParser.RealExprContext> realExprs = ctx.realExpr();
        ArrayList<Double> array = new ArrayList<>();
        Iterator<ExprLangParser.RealExprContext> it = realExprs.iterator();
        while (it.hasNext()) {
            ExprLangParser.RealExprContext expr = it.next();
            array.add(visit(expr).getScalarRealOrIntAsReal());
        }
        double[] realList = new double[array.size()];
        for (int i=0; i<realList.length; i++)
            realList[i] = array.get(i);
        return new ObjectValue(realList);
    }
    
    @Override
    public EvaluatedFormula visitMainIntOrInfiniteExpr(ExprLangParser.MainIntOrInfiniteExprContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        if (ctx.INFINITY() != null)
            return new ObjectValue(EvaluationSymbol.INFINITY);
        return visit(ctx.intExpr());
    }

    @Override
    public EvaluatedFormula visitMainClockDef(ExprLangParser.MainClockDefContext ctx) {
        throw new IllegalStateException("Should not be here.");
    }

    @Override
    public EvaluatedFormula visitMainActSet(ExprLangParser.MainActSetContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.actSet());
    }

    @Override
    public EvaluatedFormula visitMainActBinding(ExprLangParser.MainActBindingContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.actBinding());
    }

    @Override
    public EvaluatedFormula visitMainStatePropExpr(ExprLangParser.MainStatePropExprContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.boolExpr());
    }

    @Override
    public EvaluatedFormula visitMainClockConstr(ExprLangParser.MainClockConstrContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.clockConstr());
    }

    @Override
    public EvaluatedFormula visitMainCsltaExpr(ExprLangParser.MainCsltaExprContext ctx) {
        throw new UnsupportedOperationException("CSL^TA expression evaluation is not implemented.");
    }

    @Override
    public EvaluatedFormula visitMainMeasure(ExprLangParser.MainMeasureContext ctx) {
        throw new UnsupportedOperationException("Measure expression evaluation is not implemented.");
    }

    @Override
    public EvaluatedFormula visitMainIntMSetExpr(ExprLangParser.MainIntMSetExprContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.intMSetExpr());
    }

    @Override
    public EvaluatedFormula visitMainRealMSetExpr(ExprLangParser.MainRealMSetExprContext ctx) {
        requireCompleteEvaluation(ctx.EOF());
        return visit(ctx.realMSetExpr());
    }

    @Override
    public EvaluatedFormula visitMainColorClassDef(ExprLangParser.MainColorClassDefContext ctx) {
        throw new UnsupportedOperationException("Should not evaluate this kind of formula.");
    }

    @Override
    public EvaluatedFormula visitMainColorVarDef(ExprLangParser.MainColorVarDefContext ctx) {
        throw new UnsupportedOperationException("Should not evaluate this kind of formula.");
    }

    @Override
    public EvaluatedFormula visitMainPlaceColorDomainDef(ExprLangParser.MainPlaceColorDomainDefContext ctx) {
        throw new UnsupportedOperationException("Should not evaluate this kind of formula.");
    }
    
    //==========================================================================
    //  Integer expressions:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitIntConstId(ExprLangParser.IntConstIdContext ctx) {
        ConstantID intConst = (ConstantID)context
                .getNodeByUniqueName(ctx.INT_CONST_ID().getText());
        return intConst.evaluate(context);
//        int intConstIdValue = context.intConstIdValues.get(ctx.INT_CONST_ID().getText());
//        return intFormula(intConstIdValue);
    }

    @Override
    public EvaluatedFormula visitIntTemplateId(ExprLangParser.IntTemplateIdContext ctx) {
        TemplateVariable templIntId = (TemplateVariable)context.getNodeByUniqueName(ctx.getText());
        return templIntId.evaluate(context, args);
    }

    @Override
    public EvaluatedFormula visitIntConstLiteral(ExprLangParser.IntConstLiteralContext ctx) {
        return IntScalarValue.makeNew(Integer.parseInt(ctx.INT().getText()));
    }

    @Override
    public EvaluatedFormula visitIntExprUnaryRealFn(ExprLangParser.IntExprUnaryRealFnContext ctx) {
        return visit(ctx.realExpr()).evalUnaryFn(ctx.unaryIntRealFn().fn.getType());
    }

    @Override
    public EvaluatedFormula visitIntExprParen(ExprLangParser.IntExprParenContext ctx) {
        return visit(ctx.intExpr());
    }

    @Override
    public EvaluatedFormula visitIntExprCond(ExprLangParser.IntExprCondContext ctx) {
        if (visit(ctx.boolExpr()).getScalarBoolean())
            return visit(ctx.intExpr(0));
        else
            return visit(ctx.intExpr(1));
    }
    
    @Override
    public EvaluatedFormula visitIntExprCond2(ExprLangParser.IntExprCond2Context ctx) {
        if (visit(ctx.boolExpr()).getScalarBoolean())
            return visit(ctx.intExpr(0));
        else
            return visit(ctx.intExpr(1));
    }

    @Override
    public EvaluatedFormula visitIntExprBinaryFn(ExprLangParser.IntExprBinaryFnContext ctx) {
        return visit(ctx.intExpr(0)).evalBinaryFn(ctx.binaryIntFn().fn.getType(), visit(ctx.intExpr(1)));
    }

    @Override
    public EvaluatedFormula visitIntExprUnaryFn(ExprLangParser.IntExprUnaryFnContext ctx) {
        return visit(ctx.intExpr()).evalUnaryFn(ctx.unaryIntFn().fn.getType());
    }

    @Override
    public EvaluatedFormula visitIntExprNegate(ExprLangParser.IntExprNegateContext ctx) {
        return visit(ctx.intExpr()).evalUnaryFn(ExprLangParser.SUB);
    }

    @Override
    public EvaluatedFormula visitIntExprAddSub(ExprLangParser.IntExprAddSubContext ctx) {
        return visit(ctx.intExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.intExpr(1)));
    }

    @Override
    public EvaluatedFormula visitIntExprMulDiv(ExprLangParser.IntExprMulDivContext ctx) {
        return visit(ctx.intExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.intExpr(1)));
    }

    @Override
    public EvaluatedFormula visitIntExprConst(ExprLangParser.IntExprConstContext ctx) {
        return visit(ctx.intConst());
    }

    @Override
    public EvaluatedFormula visitIntExprPlaceMarking(ExprLangParser.IntExprPlaceMarkingContext ctx) {
        Place place = (Place)context.getNodeByUniqueName(ctx.INT_PLACE_ID().getText());
        assert place.isDiscrete() && place.isInNeutralDomain();
        return args.jointState.getMarkingOfPlace(place);
    }

    @Override
    public EvaluatedFormula visitIntExprCardMSet(ExprLangParser.IntExprCardMSetContext ctx) {
        ColorClass fixedDomain = context.colorDomainOfExpr;
        try {
            context.colorDomainOfExpr = null;
            return visit(ctx.intMSetExpr()).evalUnaryFn(ExprLangParser.MULTISET_CARD);
        }
        finally {
            context.colorDomainOfExpr = fixedDomain;
        }
    }

    @Override
    public EvaluatedFormula visitIntExprColorNum(ExprLangParser.IntExprColorNumContext ctx) {
        return visit(ctx.colorTerm()).evalUnaryFn(ExprLangParser.COLOR_ORDINAL);
    }    

    @Override
    public EvaluatedFormula visitIntExprUnknownId(ExprLangParser.IntExprUnknownIdContext ctx) {
        throw new IllegalStateException("Should not evaluate a formula with an unknown ID.");
    }

    @Override
    public EvaluatedFormula visitIntExprUnknownPlaceId(ExprLangParser.IntExprUnknownPlaceIdContext ctx) {
        throw new IllegalStateException("Should not evaluate a formula with an unknown place ID.");
    }
    
    //==========================================================================
    //  Real expressions:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitRealConstId(ExprLangParser.RealConstIdContext ctx) {
        if (context.evaluatePdfBounds) 
            return ListOfBounds.UNBOUNDED;
        ConstantID realConst = (ConstantID)context
                .getNodeByUniqueName(ctx.REAL_CONST_ID().getText());
        return realConst.evaluate(context);
//        double realConstIdValue = context.realConstIdValues.get(ctx.REAL_CONST_ID().getText());
//        return realFormula(realConstIdValue);        
    }

    @Override
    public EvaluatedFormula visitRealTemplateId(ExprLangParser.RealTemplateIdContext ctx) {
        TemplateVariable templRealId = (TemplateVariable)context.getNodeByUniqueName(ctx.getText());
        return templRealId.evaluate(context, args);
    }

    @Override
    public EvaluatedFormula visitRealConstLiteral(ExprLangParser.RealConstLiteralContext ctx) {
        if (context.evaluatePdfBounds) 
            return ListOfBounds.UNBOUNDED;
        return RealScalarValue.makeNew(Double.parseDouble(ctx.REAL().getText()));
    }

    @Override
    public EvaluatedFormula visitRealFromIntConst(ExprLangParser.RealFromIntConstContext ctx) {
        if (context.evaluatePdfBounds) 
            return ListOfBounds.UNBOUNDED;
        return visit(ctx.intConst()).evalUnaryFn(ExprLangParser.INT_TO_REAL);
    }

    @Override
    public EvaluatedFormula visitRealExprNegate(ExprLangParser.RealExprNegateContext ctx) {
        return visit(ctx.realExpr()).evalUnaryFn(ExprLangParser.SUB);
    }

    @Override
    public EvaluatedFormula visitRealExprBinaryFn(ExprLangParser.RealExprBinaryFnContext ctx) {
        int fn = (ctx.binaryIntFn()!= null) ? ctx.binaryIntFn().fn.getType() 
                                            : ctx.binaryRealFn().fn.getType();
        if (context.evaluatePdfBounds) {
            switch (fn) {
                case ExprLangParser.TRIANGULAR_FN:
                case ExprLangParser.RECT_FN: 
                case ExprLangParser.UNIFORM_FN: 
                {
                    // The bound is the [expr(0), expr(1)]
                    context.evaluatePdfBounds = false;
                    EvaluatedFormula lower = visit(ctx.realExpr(0));
                    EvaluatedFormula upper = visit(ctx.realExpr(1));
                    context.evaluatePdfBounds = true;
                    return ListOfBounds.makeNew(lower.getScalarRealOrIntAsReal(), 
                                                 upper.getScalarRealOrIntAsReal());
                }

                case ExprLangParser.TRUNCATED_EXP_FN:
                {
                    // bound is [0, expr(1)]
                    context.evaluatePdfBounds = false;
                    EvaluatedFormula upper = visit(ctx.realExpr(1));
                    context.evaluatePdfBounds = true;
                    return ListOfBounds.makeNew(0, upper.getScalarRealOrIntAsReal());
                }

                case ExprLangParser.ERLANG_FN:
                case ExprLangParser.PARETO_FN:
                {
                    // No bound
                    return ListOfBounds.UNBOUNDED;
                }
            }
        }
        return visit(ctx.realExpr(0)).evalBinaryFn(fn, visit(ctx.realExpr(1)));
    }

    @Override
    public EvaluatedFormula visitRealExprAddSub(ExprLangParser.RealExprAddSubContext ctx) {
        if (context.evaluatePdfBounds)
            return ListOfBounds.joinBounds(visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
        return visit(ctx.realExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.realExpr(1)));
    }

    @Override
    public EvaluatedFormula visitRealExprInt(ExprLangParser.RealExprIntContext ctx) {
        if (context.evaluatePdfBounds) 
            return ListOfBounds.UNBOUNDED;
        return visit(ctx.intExpr()).evalUnaryFn(ExprLangParser.INT_TO_REAL);
    }

    @Override
    public EvaluatedFormula visitRealExprUnaryFn(ExprLangParser.RealExprUnaryFnContext ctx) {
        int fn = (ctx.unaryIntFn() != null) ? ctx.unaryIntFn().fn.getType() : ctx.unaryRealFn().fn.getType();
        if (context.evaluatePdfBounds) {
            switch (fn) {
                case ExprLangParser.DIRAC_DELTA_FN: {
                    context.evaluatePdfBounds = false;
                    EvaluatedFormula impulse = visit(ctx.realExpr());
                    context.evaluatePdfBounds = true;
                    return ListOfBounds.makeNew(impulse.getScalarRealOrIntAsReal());
                }
            }
        }
        return visit(ctx.realExpr()).evalUnaryFn(fn);
    }

    @Override
    public EvaluatedFormula visitRealExprMulDiv(ExprLangParser.RealExprMulDivContext ctx) {
        if (context.evaluatePdfBounds)  // in bound mode, */ are sum, and +- are max
            return ListOfBounds.combineBounds(visit(ctx.realExpr(0)), visit(ctx.realExpr(1)));
        return visit(ctx.realExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.realExpr(1)));
    }

    @Override
    public EvaluatedFormula visitRealExprParen(ExprLangParser.RealExprParenContext ctx) {
        return visit(ctx.realExpr());
    }

    @Override
    public EvaluatedFormula visitRealExprPlaceMarking(ExprLangParser.RealExprPlaceMarkingContext ctx) {
        Place place = (Place)context.getNodeByUniqueName(ctx.REAL_PLACE_ID().getText());
        assert place.isContinuous() && place.isInNeutralDomain();
        return args.jointState.getMarkingOfPlace(place);
    }

    @Override
    public EvaluatedFormula visitRealExprConst(ExprLangParser.RealExprConstContext ctx) {
        if (context.evaluatePdfBounds) 
            return ListOfBounds.UNBOUNDED;
        return visit(ctx.realConst());
    }

    @Override
    public EvaluatedFormula visitRealExprCond(ExprLangParser.RealExprCondContext ctx) {
        if (visit(ctx.boolExpr()).getScalarBoolean())
            return visit(ctx.realExpr(0));
        else
            return visit(ctx.realExpr(1));
    }
    
    @Override
    public EvaluatedFormula visitRealExprCond2(ExprLangParser.RealExprCond2Context ctx) {
        if (visit(ctx.boolExpr()).getScalarBoolean())
            return visit(ctx.realExpr(0));
        else
            return visit(ctx.realExpr(1));
    }

    @Override
    public EvaluatedFormula visitRealExprPdfXVar(ExprLangParser.RealExprPdfXVarContext ctx) {
        requireBoundOfPdfMode();
        return RealScalarValue.ZERO;
    }

    @Override
    public EvaluatedFormula visitRealExprUnknownId(ExprLangParser.RealExprUnknownIdContext ctx) {
        throw new IllegalStateException("Should not evaluate a formula with an unknown ID.");
    }
    
    //==========================================================================
    //  Boolean expressions:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitBoolConst(ExprLangParser.BoolConstContext ctx) {
        if (ctx.TRUE() != null)
            return BooleanScalarValue.TRUE;
        else if (ctx.FALSE() != null)
            return BooleanScalarValue.FALSE;
        else throw new UnsupportedOperationException();
    }

    @Override
    public EvaluatedFormula visitBoolExprStatePropId(ExprLangParser.BoolExprStatePropIdContext ctx) {
        TemplateVariable statePropId = (TemplateVariable)context.getNodeByUniqueName(ctx.getText());
        Expr varExpr = context.templateVarsBinding.getSingleValueBoundTo(statePropId);
        EvaluatedFormula value = varExpr.evaluate(context.bindingContext, args);
        if (value.getType() != EvaluatedFormula.Type.BOOLEAN)
            throw new EvaluationException("Binding of state proposition variable "+
                                          statePropId.getUniqueName()+" has "+
                                          " type "+value.getType());
        return value;
    }

    @Override
    public EvaluatedFormula visitBoolExprIntComp(ExprLangParser.BoolExprIntCompContext ctx) {
        return visit(ctx.intExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.intExpr(1)));
    }

    @Override
    public EvaluatedFormula visitBoolExprRealComp(ExprLangParser.BoolExprRealCompContext ctx) {
        return visit(ctx.realExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.realExpr(1)));
    }

    @Override
    public EvaluatedFormula visitBoolExprAnd(ExprLangParser.BoolExprAndContext ctx) {
        // Don't do short-circuiting, since we still need to know the type
        // of the result (scalar or multiset), which depends on both the operands.
        return visit(ctx.boolExpr(0)).evalBinaryFn(ExprLangParser.AND, visit(ctx.boolExpr(1)));
    }

    @Override
    public EvaluatedFormula visitBoolExprParen(ExprLangParser.BoolExprParenContext ctx) {
        return visit(ctx.boolExpr());
    }

    @Override
    public EvaluatedFormula visitBoolExprOr(ExprLangParser.BoolExprOrContext ctx) {
        // Don't do short-circuiting, since we still need to know the type
        // of the result (scalar or multiset), which depends on both the operands.
        return visit(ctx.boolExpr(0)).evalBinaryFn(ExprLangParser.OR, visit(ctx.boolExpr(1)));
    }

    @Override
    public EvaluatedFormula visitBoolExprImply(ExprLangParser.BoolExprImplyContext ctx) {
        // Don't do short-circuiting, since we still need to know the type
        // of the result (scalar or multiset), which depends on both the operands.
        return visit(ctx.boolExpr(0)).evalBinaryFn(ExprLangParser.IMPLY, visit(ctx.boolExpr(1)));
    }

    @Override
    public EvaluatedFormula visitBoolExprBiimply(ExprLangParser.BoolExprBiimplyContext ctx) {
        // Don't do short-circuiting, since we still need to know the type
        // of the result (scalar or multiset), which depends on both the operands.
        return visit(ctx.boolExpr(0)).evalBinaryFn(ExprLangParser.BIIMPLY, visit(ctx.boolExpr(1)));
    }

    @Override
    public EvaluatedFormula visitBoolExprConst(ExprLangParser.BoolExprConstContext ctx) {
        return visit(ctx.boolConst());
    }

    @Override
    public EvaluatedFormula visitBoolExprNot(ExprLangParser.BoolExprNotContext ctx) {
        return visit(ctx.boolExpr()).evalUnaryFn(ExprLangParser.NOT);
    }

    @Override
    public EvaluatedFormula visitBoolExprUnknownId(ExprLangParser.BoolExprUnknownIdContext ctx) {
        throw new IllegalStateException("Should not evaluate a formula with an unknown ID.");
    }

    @Override
    public EvaluatedFormula visitBoolExprColorTermComp(ExprLangParser.BoolExprColorTermCompContext ctx) {
        EvaluatedFormula t0, t1;
        try {
            t0 = visit(ctx.colorTerm(0));
            t1 = visit(ctx.colorTerm(1));
        }
        catch (OutOfBoundColorException e) {
            return BooleanScalarValue.FALSE;
        }
        if (ctx.op.getType() == ExprLangParser.EQUAL ||
            ctx.op.getType() == ExprLangParser.NOT_EQUAL) {
            return t0.evalBinaryFn(ctx.op.getType(), t1);
        }
        
        // Comparison of single terms using <, >, <= or >=
        MultiSet m0 = (MultiSet)t0;
        MultiSet m1 = (MultiSet)t1;
        if (m0.domain != m1.domain)
            throw new EvaluationException("Comparison between different color domains!");
        if (m0.domain.isCrossDomain())
            throw new EvaluationException("Color term comparison with <,<=,>,>= can be done only on simple color domains!");
        if (m0.numElements()!=1 || m1.numElements()!=1)
            throw new EvaluationException("Color term comparison with <,<=,>,>= cannot be done on sets!");
        int r = m0.getElement(0).compareTo(m1.getElement(0));
        
        switch (ctx.op.getType()) {
            case ExprLangParser.GREATER:
                return BooleanScalarValue.makeNew(r > 0);
            case ExprLangParser.GREATER_EQ:
                return BooleanScalarValue.makeNew(r >= 0);
            case ExprLangParser.LESS:
                return BooleanScalarValue.makeNew(r < 0);
            case ExprLangParser.LESS_EQ:
                return BooleanScalarValue.makeNew(r <= 0);
            default:
                throw new UnsupportedOperationException();
        }
    }

    @Override
    public EvaluatedFormula visitBoolExprColorTermIn(ExprLangParser.BoolExprColorTermInContext ctx) {
        EvaluatedFormula term = visit(ctx.colorTerm());
        ColorClass currCC = msetElemColorClass;
        try {
            msetElemColorClass = term.getDomain();
            EvaluatedFormula set = visit(ctx.colorSet());
            EvaluatedFormula e = set.evalBinaryFn(ExprLangParser.CONTAINS, term);
            System.out.println(" evaluate "+set+"  CONTAINS  "+term+"   = "+e);
            if (ctx.DONT_CONTAINS() != null)
                e = e.evalUnaryFn(ExprLangParser.NOT);
            return e;
        } 
        finally {
            msetElemColorClass = currCC;
        }
    }
    
    //==========================================================================
    //  Color variables and multiset terms:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitColorVarIdentifier(ExprLangParser.ColorVarIdentifierContext ctx) {
        assert args.colorVarsBinding != null;
        EvaluatedFormula value = args.colorVarsBinding.getValueBoundTo(ctx.COLORVAR_ID().getText());
        if (value == null)
            throw new EvaluationException("Missing color variable value in binding set.");
        return value;
    }

    @Override
    public EvaluatedFormula visitColorSubclassIdentifier(ExprLangParser.ColorSubclassIdentifierContext ctx) {
        String id = ctx.COLORSUBCLASS_ID().getText();
        Iterator<ColorClass> ccIt = context.colorClassIterator();
        while (ccIt.hasNext()) {
            ColorClass cc = ccIt.next();
            if (!cc.isSimpleClass())
                continue;
            int base = 0;
            for (int s=0; s<cc.numSubClasses(); s++) {
                ParsedColorSubclass pcs = cc.getSubclass(s);
                if (pcs.isNamed() && pcs.name.equals(id)) {
                    // Return all the colors of this subclass
                    Set<DomainElement> allElems = new TreeSet<>();
                    int[] singleColor = new int[1];
                    for (int i=0; i<pcs.getNumColors(); i++) {
                        singleColor[0] = base + i;
                        allElems.add(new DomainElement(cc, singleColor));
                    }
                    return MultiSet.makeNew(cc, allElems);
                }
                base += pcs.getNumColors();
            }
        }
        throw new IllegalStateException("Could not find the subclass.");
    }

//    @Override
//    public EvaluatedFormula visitMSetTermAll(ExprLangParser.MSetTermAllContext ctx) {
//        assert msetElemColorClass != null; // Only in fixedDomain expressions
//        // Return all the colors in msetElemColorClass
//        int numColors = msetElemColorClass.numColors();
//        int[] singleColor = new int[1];
//        assert numColors >= 0;
//        Set<DomainElement> allElems = new TreeSet<>();
//        for (int i=0; i<numColors; i++) {
//            singleColor[0] = i;
//            allElems.add(new DomainElement(msetElemColorClass, singleColor));
//        }
//        return MultiSet.makeNew(msetElemColorClass, allElems);
//    }

//    @Override
//    public EvaluatedFormula visitMSetTermColorVar(ExprLangParser.MSetTermColorVarContext ctx) {
//        EvaluatedFormula value = visit(ctx.colorVar());
//        assert value instanceof MultiSet && ((MultiSet)value).numElements() == 1;
//        return value;
//    }

//    @Override
//    public EvaluatedFormula visitMSetTermColorVarNext(ExprLangParser.MSetTermColorVarNextContext ctx) {
//        EvaluatedFormula value = visit(ctx.colorVar());
//        assert value instanceof MultiSet && ((MultiSet)value).numElements() == 1;
//        Set<DomainElement> nextElem = new TreeSet<>();
//        int[] nextColor = new int[1];
//        int varColor = ((MultiSet)value).getElement(0).getColor(0);
//        nextColor[0] = (varColor + 1) % value.getDomain().numColors();
//        nextElem.add(new DomainElement(value.getDomain(), nextColor));
//        return MultiSet.makeNew(value.getDomain(), nextElem);
//    }
//
//    @Override
//    public EvaluatedFormula visitMSetTermColorVarPrev(ExprLangParser.MSetTermColorVarPrevContext ctx) {
//        EvaluatedFormula value = visit(ctx.colorVar());
//        assert value instanceof MultiSet && ((MultiSet)value).numElements() == 1;
//        Set<DomainElement> nextElem = new TreeSet<>();
//        int[] nextColor = new int[1];
//        int varColor = ((MultiSet)value).getElement(0).getColor(0);
//        nextColor[0] = (varColor == 0 ? value.getDomain().numColors() : varColor) - 1;
//        nextElem.add(new DomainElement(value.getDomain(), nextColor));
//        return MultiSet.makeNew(value.getDomain(), nextElem);
//    }

//    @Override
//    public EvaluatedFormula visitMSetTermColorName(ExprLangParser.MSetTermColorNameContext ctx) {
//        String id = ctx.COLOR_ID().getText();
//        for (Node node : context.page.nodes) {
//            if (node instanceof ColorClass) {
//                ColorClass cc = (ColorClass)node;
//                if (!cc.isSimpleClass())
//                    continue;
//                if (cc.testHasColorNamed(context.page, id)) {
//                    Set<DomainElement> elems = new TreeSet<>();
//                    int[] singleColor = new int[1];
//                    singleColor[0] = cc.getColorIndex(id);
//                    elems.add(new DomainElement(cc, singleColor));
//                    return MultiSet.makeNew(cc, elems);
//                }
//            }
//        }
//        throw new IllegalStateException("Could not find the named color.");
//    }

//    @Override
//    public EvaluatedFormula visitMSetElemTerm(ExprLangParser.MSetElemTermContext ctx) {
//        return visit(ctx.multiSetTerm());
//    }

    @Override
    public EvaluatedFormula visitMSetElemAddSub(ExprLangParser.MSetElemAddSubContext ctx) {
        int op = (ctx.op.getType() == ExprLangParser.ADD) ? ExprLangParser.UNION : ExprLangParser.SETMINUS;
        return visit(ctx.multiSetElem(0)).evalBinaryFn(op, visit(ctx.multiSetElem(1)));
    }

    @Override
    public EvaluatedFormula visitMSetElemColorTerm(ExprLangParser.MSetElemColorTermContext ctx) {
        return visit(ctx.colorSet());
    }
    
    //==========================================================================
    //  Color Sets:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitColorSetAll(ExprLangParser.ColorSetAllContext ctx) {
        assert msetElemColorClass != null; // Only in fixedDomain expressions
        // Return all the colors in msetElemColorClass
        int numColors = msetElemColorClass.numColors();
        int[] singleColor = new int[1];
        assert numColors >= 0;
        Set<DomainElement> allElems = new TreeSet<>();
        for (int i=0; i<numColors; i++) {
            singleColor[0] = i;
            allElems.add(new DomainElement(msetElemColorClass, singleColor));
        }
        return MultiSet.makeNew(msetElemColorClass, allElems);
    }

    @Override
    public EvaluatedFormula visitColorSetTerm(ExprLangParser.ColorSetTermContext ctx) {
        return visit(ctx.colorTerm());
    }

    @Override
    public EvaluatedFormula visitColorSetClass(ExprLangParser.ColorSetClassContext ctx) {
        return colorSetOfClass(ctx.SIMPLECOLORCLASS_ID().getText());
    }

    @Override
    public EvaluatedFormula visitColorSetClass2(ExprLangParser.ColorSetClass2Context ctx) {
        return colorSetOfClass(ctx.SIMPLECOLORCLASS_ID().getText());
    }
    
    private EvaluatedFormula colorSetOfClass(String colorClassName) {
        String ccName = colorClassName;
        Iterator<ColorClass> ccIt = context.colorClassIterator();
        while (ccIt.hasNext()) {
            ColorClass cc = ccIt.next();
            if (!cc.isSimpleClass())
                continue;
            if (!cc.getUniqueName().equals(ccName))
                continue;

            int numColors = cc.numColors();
            int[] singleColor = new int[1];
            assert numColors >= 0;
            Set<DomainElement> allElems = new TreeSet<>();
            for (int i=0; i<numColors; i++) {
                singleColor[0] = i;
                allElems.add(new DomainElement(cc, singleColor));
            }
            return MultiSet.makeNew(cc, allElems);
        }
        throw new IllegalStateException("Could not find color class "+colorClassName);
    }

    @Override
    public EvaluatedFormula visitColorSetSubClass(ExprLangParser.ColorSetSubClassContext ctx) {
        return visit(ctx.colorSubclass());
    }

    @Override
    public EvaluatedFormula visitColorSetSubClass2(ExprLangParser.ColorSetSubClass2Context ctx) {
        return visit(ctx.colorSubclass());
    }

    @Override
    public EvaluatedFormula visitColorSetSubclassOf(ExprLangParser.ColorSetSubclassOfContext ctx) {
        return colorSubclassOfTerm(visit(ctx.colorTerm()));
    }
    
    @Override
    public EvaluatedFormula visitColorSetSubclassOf2(ExprLangParser.ColorSetSubclassOf2Context ctx) {
        return colorSubclassOfTerm(visit(ctx.colorTerm()));
    }
    
    private static EvaluatedFormula colorSubclassOfTerm(EvaluatedFormula term) {
        assert term.isMultiSetBoolean();
        ColorClass dom = term.getDomain();
        assert dom.isSimpleClass();
        MultiSet value = (MultiSet)term;
        if (value.numElements() != 1)
            throw new IllegalStateException("Asking subclass of a set!");
        int colorNum = value.getElement(0).getColor(0);
        
        // Find the subclass that contains colorNum, and return the entire subclass
        int base = 0;
        for (int s=0; s<dom.numSubClasses(); s++) {
            ParsedColorSubclass pcs = dom.getSubclass(s);
            int numColorsInSub = pcs.getNumColors();
            if (colorNum < numColorsInSub) {
                // Return all the colors of this subclass
                Set<DomainElement> allElems = new TreeSet<>();
                int[] singleColor = new int[1];
                for (int i=0; i<pcs.getNumColors(); i++) {
                    singleColor[0] = base + i;
                    allElems.add(new DomainElement(dom, singleColor));
                }
                return MultiSet.makeNew(dom, allElems);
            }
            colorNum -= numColorsInSub;
        }
        throw new IllegalStateException("Invalid color index");
    }

    //==========================================================================
    //  Color Terms:
    //==========================================================================

    @Override
    public EvaluatedFormula visitColorTermColor(ExprLangParser.ColorTermColorContext ctx) {
        String id = ctx.COLOR_ID().getText();
        Iterator<ColorClass> ccIt = context.colorClassIterator();
        while (ccIt.hasNext()) {
            ColorClass cc = ccIt.next();
            if (!cc.isSimpleClass())
                continue;
            if (cc.testHasColorNamed(context, id)) {
                Set<DomainElement> elems = new TreeSet<>();
                int[] singleColor = new int[1];
                singleColor[0] = cc.getColorIndex(id);
                elems.add(new DomainElement(cc, singleColor));
                return MultiSet.makeNew(cc, elems);
            }
        }
        throw new IllegalStateException("Could not find the named color.");
    }

    @Override
    public EvaluatedFormula visitColorTermVar(ExprLangParser.ColorTermVarContext ctx) {
        EvaluatedFormula value = visit(ctx.colorVar());
        assert value instanceof MultiSet && ((MultiSet)value).numElements() == 1;
        return value;
    }

    @Override
    public EvaluatedFormula visitColorTermNextPrev(ExprLangParser.ColorTermNextPrevContext ctx) {
        EvaluatedFormula value = visit(ctx.colorTerm());
        return colorNextPrev(value, ctx.op.getType());
    }

//    @Override
//    public EvaluatedFormula visitColorTermNextPrev2(ExprLangParser.ColorTermNextPrev2Context ctx) {
//        EvaluatedFormula value = visit(ctx.colorTerm());
//        int op = ctx.op.getType() == ExprLangParser.NOT ? ExprLangParser.POSTINCR 
//                                                        : ExprLangParser.POSTDECR;
//        return colorNextPrev(value, op);
//    }
    
    private EvaluatedFormula colorNextPrev(EvaluatedFormula value, int op) {
        assert value instanceof MultiSet && ((MultiSet)value).numElements() == 1;
        Set<DomainElement> nextElem = new TreeSet<>();
        int[] nextColor = new int[1];
        if (((MultiSet)value).numElements() == 0 && !((MultiSet)value).getDomain().isCircular())
            return value;
        if (((MultiSet)value).numElements() != 1)
            throw new EvaluationException("Asking next/prev color on a color set!");
        
        int varColor = ((MultiSet)value).getElement(0).getColor(0);
        nextColor[0] = varColor + ((op == ExprLangParser.POSTINCR) ? +1 : -1);   
        int domainCard = value.getDomain().numColors();
        if (((MultiSet)value).getDomain().isCircular())
            nextColor[0] = (nextColor[0] + domainCard) % domainCard;
                
        if (nextColor[0] < 0 || nextColor[0] >= value.getDomain().numColors())
//            return MultiSet.makeNew(value.getDomain(), nextElem);
            throw new OutOfBoundColorException("Out-of-bound unary ++/-- operator on a enumerative color.");

        nextElem.add(new DomainElement(value.getDomain(), nextColor));
        return MultiSet.makeNew(value.getDomain(), nextElem);
    }

    @Override
    public EvaluatedFormula visitColorTermFilterThis(ExprLangParser.ColorTermFilterThisContext ctx) {
        if (context.filterThisDomain == null)
            throw new EvaluationException("@[i] statement can only be used in filter predicates.");
        DomainElement thisElem = context.filterThisDomain;
        
        int index = Integer.parseInt(ctx.INT().getText());
        if (index < 0 || index >= thisElem.getDomain().getNumClassesInDomain())
            throw new EvaluationException("Domain index "+index+" is outside its bounds.");
        
//        int mset = ((IntScalarValue)context.filterThisValue).value;
        int[] colorIndex = new int[1];
        colorIndex[0] = thisElem.getColor(index);
        DomainElement singleCol = new DomainElement(thisElem.getDomain().getColorClass(index), 
                                                    colorIndex);        
        Set<DomainElement> cset = new TreeSet<>();
        cset.add(singleCol);
        EvaluatedFormula term = MultiSet.makeNew(thisElem.getDomain().getColorClass(index), cset);
        return term;
    }
    
    //==========================================================================
    //  Multiset predicate:
    //==========================================================================

    @Override
    public EvaluatedFormula visitMSetBoolPredicate(ExprLangParser.MSetBoolPredicateContext ctx) {
        return visit(ctx.boolExpr());
    }

    @Override
    public EvaluatedFormula visitMSetElemBoolPredicate(ExprLangParser.MSetElemBoolPredicateContext ctx) {
        return visit(ctx.boolExpr());
    }
    
    //==========================================================================
    //  Int multiset expression:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitIntMSetExprParen(ExprLangParser.IntMSetExprParenContext ctx) {
        return visit(ctx.intMSetExpr());
    }
    
    @Override
    public EvaluatedFormula visitIntMSetExprAddSub(ExprLangParser.IntMSetExprAddSubContext ctx) {
        return visit(ctx.intMSetExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.intMSetExpr(1)));
    }

    @Override
    public EvaluatedFormula visitIntMSetExprElemProduct(ExprLangParser.IntMSetExprElemProductContext ctx) {
        EvaluatedFormula pred = (ctx.mSetPredicate() != null ? visit(ctx.mSetPredicate()) : BooleanScalarValue.TRUE);
        EvaluatedFormula mult = (ctx.intExpr() != null ? visit(ctx.intExpr()) : IntScalarValue.ONE);
        if (!pred.getScalarBoolean())
            mult = IntScalarValue.ZERO; // Predicate evaluates to false.
        EvaluatedFormula resMset = visitMsetExprElemProduct(mult, ctx.multiSetElem());
        
        // Evaluate filter predicate
        if (ctx.mSetElemPredicate() != null) {
            ValuedMultiSet mset = (ValuedMultiSet)resMset;
            Map<DomainElement, EvaluatedFormula> newSet = new TreeMap<>();
            for (int i=0; i<mset.numElements(); i++) {
                DomainElement ed = mset.getElement(i);
                EvaluatedFormula ev = mset.getValue(i);
                
                context.filterThisDomain = ed;
                context.filterThisValue = ev;
                boolean keep = visit(ctx.mSetElemPredicate()).getScalarBoolean();
                context.filterThisDomain = null;
                context.filterThisValue = null;

//                System.out.println("eval ed="+ed+" ev="+ev+"   keep="+keep);
                if (keep)
                    newSet.put(ed, ev);
            }
            resMset = ValuedMultiSet.makeNew(mset.getType(), mset.getDomain(), newSet);
        }
        
        return resMset;
    }

    @Override
    public EvaluatedFormula visitIntMsetExprPlaceMarking(ExprLangParser.IntMsetExprPlaceMarkingContext ctx) {
        Place place = (Place)context.getNodeByUniqueName(ctx.INT_MSET_PLACE_ID().getText());
        assert place.isDiscrete() && place.isInColorDomain();
        return args.jointState.getMarkingOfPlace(place);
    }

    @Override
    public EvaluatedFormula visitIntMSetExprConst(ExprLangParser.IntMSetExprConstContext ctx) {
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        ConstantID intConst = (ConstantID)context
                .getNodeByUniqueName(ctx.INT_MSET_CONST_ID().getText());
        assert fixedDomain == null || intConst.getColorDomain() == fixedDomain;
        return intConst.evaluate(context);
    }

    @Override
    public EvaluatedFormula visitRealMSetExprParen(ExprLangParser.RealMSetExprParenContext ctx) {
        return visit(ctx.realMSetExpr());
    }
    
    @Override
    public EvaluatedFormula visitRealMSetExprAddSub(ExprLangParser.RealMSetExprAddSubContext ctx) {
        return visit(ctx.realMSetExpr(0)).evalBinaryFn(ctx.op.getType(), visit(ctx.realMSetExpr(1)));
    }

    @Override
    public EvaluatedFormula visitRealMSetExprElemProduct(ExprLangParser.RealMSetExprElemProductContext ctx) {
        EvaluatedFormula pred = (ctx.mSetPredicate() != null ? visit(ctx.mSetPredicate()) : BooleanScalarValue.TRUE);
        EvaluatedFormula mult = (ctx.realExpr() != null ? visit(ctx.realExpr()) : RealScalarValue.ONE);
        if (!pred.getScalarBoolean())
            mult = RealScalarValue.ZERO; // Predicate evaluates to false.
        EvaluatedFormula resMset = visitMsetExprElemProduct(mult, ctx.multiSetElem());
        
        // Evaluate filter predicate
        if (ctx.mSetElemPredicate() != null) {
            ValuedMultiSet mset = (ValuedMultiSet)resMset;
            Map<DomainElement, EvaluatedFormula> newSet = new TreeMap<>();
            for (int i=0; i<mset.numElements(); i++) {
                DomainElement ed = mset.getElement(i);
                EvaluatedFormula ev = mset.getValue(i);
                
                context.filterThisDomain = ed;
                context.filterThisValue = ev;
                boolean keep = visit(ctx.mSetElemPredicate()).getScalarBoolean();
                context.filterThisDomain = null;
                context.filterThisValue = null;

                if (keep)
                    newSet.put(ed, ev);
            }
            resMset = ValuedMultiSet.makeNew(mset.getType(), mset.getDomain(), newSet);
        }
        
        return resMset;
    }

    @Override
    public EvaluatedFormula visitRealMsetExprPlaceMarking(ExprLangParser.RealMsetExprPlaceMarkingContext ctx) {
        Place place = (Place)context.getNodeByUniqueName(ctx.REAL_MSET_PLACE_ID().getText());
        assert place.isContinuous() && place.isInColorDomain();
        return args.jointState.getMarkingOfPlace(place);
    }

    @Override
    public EvaluatedFormula visitRealMsetExprConst(ExprLangParser.RealMsetExprConstContext ctx) {
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        ConstantID intConst = (ConstantID)context
                .getNodeByUniqueName(ctx.REAL_MSET_CONST_ID().getText());
        assert fixedDomain == null || intConst.getColorDomain() == fixedDomain;
        return intConst.evaluate(context);
    }
    
    
    
    // Assigned during the visit of multiset elements
    private transient ColorClass msetElemColorClass = null;
    
    private MultiSet visitMsetExprElemProduct(EvaluatedFormula mult, List<ExprLangParser.MultiSetElemContext> msetElems) {
        final ColorClass fixedDomain = context.colorDomainOfExpr;
        
        final int numCl = msetElems.size();
        assert fixedDomain == null || numCl == fixedDomain.getNumClassesInDomain();
        EvaluatedFormula[] elems = new EvaluatedFormula[numCl];
        ListIterator<ExprLangParser.MultiSetElemContext> it = msetElems.listIterator();
        for (int i=0; i<numCl; i++) {
            try {
                if (fixedDomain != null)
                    msetElemColorClass = fixedDomain.getColorClass(i);
                elems[i] = visit(it.next());
                assert elems[i].isMultiSet() && elems[i].getDomain().isSimpleClass() &&
                       (fixedDomain == null || elems[i].getDomain() == msetElemColorClass);
            }
            finally {
                msetElemColorClass = null;
            }
        }
        
        ColorClass resultDomain = null;
        if (fixedDomain != null)
            resultDomain = fixedDomain;
        else {
            // Search for a valid color domain.
            Iterator<ColorClass> ccIt = context.colorClassIterator();
            while (ccIt.hasNext()) {
                ColorClass cc = ccIt.next();
                assert cc.isParseDataOk();
                if (cc.getNumClassesInDomain() == numCl) {
                    boolean matches = true;
                    for (int i=0; i<elems.length; i++) 
                        if (elems[i].getDomain() != cc.getColorClass(i)) {
                            matches = false;
                            break;
                        }
                    if (matches) {
                        resultDomain = cc;
                        break;
                    }
                }
            }
        }
        assert resultDomain != null;
        
        Map<DomainElement, EvaluatedFormula> newTuple = new TreeMap<>();
        int[] colors = new int[numCl];
        // Construct the cross-product of tuples
        generateMultiSetEntriesRecursively(colors, 0, elems, resultDomain, newTuple, mult);
        return MultiSet.makeNew(mult.getType(), resultDomain, newTuple);
    }
    
    private void generateMultiSetEntriesRecursively(int[] colors, int level, 
                                                    EvaluatedFormula[] elemsInCross,
                                                    ColorClass domain,
                                                    Map<DomainElement, EvaluatedFormula> newTuple, 
                                                    EvaluatedFormula mult)
    {
        if (level == elemsInCross.length) {
            // Insert this new element
            newTuple.put(new DomainElement(domain, colors), mult);
            return;
        }
        // Generate all the combinations in the cross product
        assert elemsInCross[level].isMultiSetBoolean();
        MultiSet mset = (MultiSet)elemsInCross[level];
        assert mset.getDomain().isSimpleClass();
        for (int j=0; j<mset.numElements(); j++) {
            colors[level] = mset.getClassEntry(j, 0);
            generateMultiSetEntriesRecursively(colors, level + 1, elemsInCross,
                                               domain, newTuple, mult);
        }
    }
    
    //==========================================================================
    //  Action lists and sets:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitActListEmpty(ExprLangParser.ActListEmptyContext ctx) {
        return BooleanScalarValue.FALSE;
    }

    @Override
    public EvaluatedFormula visitActListList(ExprLangParser.ActListListContext ctx) {
        if (args.firedNode == null)
            throw new EvaluationException("Should not evaluate an ActList without having the firedNode set.");
        List<TerminalNode> actList = ctx.ACTION_ID();
        for (TerminalNode actNode : actList) {
            TemplateVariable actName = (TemplateVariable)context.getNodeByUniqueName(actNode.getText());
            Expr bindTrnExpr = context.templateVarsBinding.getSingleValueBoundTo(actName);
            EvaluatedFormula isInSet = bindTrnExpr.evaluate(context.bindingContext, args);
            if (isInSet.getScalarBoolean())
                return BooleanScalarValue.TRUE;
        }
        return BooleanScalarValue.FALSE;
    }

    @Override
    public EvaluatedFormula visitActSetAllExceptList(ExprLangParser.ActSetAllExceptListContext ctx) {
        if (visit(ctx.actList()).getScalarBoolean())
            return BooleanScalarValue.FALSE;
        return BooleanScalarValue.TRUE;
    }

    @Override
    public EvaluatedFormula visitActSetList(ExprLangParser.ActSetListContext ctx) {
        return visit(ctx.actList());
    }

    @Override
    public EvaluatedFormula visitActSetAll(ExprLangParser.ActSetAllContext ctx) {
        return BooleanScalarValue.TRUE;
    }

    @Override
    public EvaluatedFormula visitActBindingTransition(ExprLangParser.ActBindingTransitionContext ctx) {
        boolean isFired = args.firedNode.isIdentifiedAs(ctx.TRANSITION_ID().getText());
        return BooleanScalarValue.makeNew(isFired);
    }
    
    //==========================================================================
    //  Language of clock constraints for DTA edges:
    //==========================================================================
    
    @Override
    public EvaluatedFormula visitClockVarId(ExprLangParser.ClockVarIdContext ctx) {
        ClockVar clockVar = (ClockVar)context.getNodeByUniqueName(ctx.CLOCK_ID().getText());
        double currClockValue = (Double)args.jointState.getValueOfClockVariable(clockVar);
        return RealScalarValue.makeNew(currClockValue);
    }

    @Override
    public EvaluatedFormula visitClockConstrEquals(ExprLangParser.ClockConstrEqualsContext ctx) {
        double clockValue = visit(ctx.clockVar()).getScalarReal();
        double realConst = visit(ctx.realExpr()).getScalarReal();
        if (args instanceof ClockGuardEvaluationArguments) {
            ((ClockGuardEvaluationArguments)args).setTimeBound(clockValue, realConst);
        }
        return BooleanScalarValue.makeNew(clockValue == realConst);
    }

    @Override
    public EvaluatedFormula visitClockConstrBetween(ExprLangParser.ClockConstrBetweenContext ctx) {
        double clockValue = visit(ctx.clockVar()).getScalarReal();
        double realConstMin = visit(ctx.realExpr(0)).getScalarReal();
        double realConstMax = visit(ctx.realExpr(1)).getScalarReal();
        if (args instanceof ClockGuardEvaluationArguments) {
            ((ClockGuardEvaluationArguments)args).setTimeBound(clockValue, realConstMin);
            ((ClockGuardEvaluationArguments)args).setTimeBound(clockValue, realConstMax);
        }
        return BooleanScalarValue.makeNew(realConstMin < clockValue && clockValue < realConstMax);
    }

    @Override
    public EvaluatedFormula visitClockConstrIdCmpReal(ExprLangParser.ClockConstrIdCmpRealContext ctx) {
        EvaluatedFormula clockValue = visit(ctx.clockVar());
        EvaluatedFormula realConst = visit(ctx.realExpr());
        if (args instanceof ClockGuardEvaluationArguments) {
            ((ClockGuardEvaluationArguments)args).setTimeBound(clockValue.getScalarReal(), realConst.getScalarReal());
        }
        return clockValue.evalBinaryFn(ctx.op.getType(), realConst);
    }

    @Override
    public EvaluatedFormula visitClockConstrRealCmpId(ExprLangParser.ClockConstrRealCmpIdContext ctx) {
        EvaluatedFormula clockValue = visit(ctx.clockVar());
        EvaluatedFormula realConst = visit(ctx.realExpr());
        if (args instanceof ClockGuardEvaluationArguments) {
            ((ClockGuardEvaluationArguments)args).setTimeBound(clockValue.getScalarReal(), realConst.getScalarReal());
        }
        return realConst.evalBinaryFn(ctx.op.getType(), clockValue);
    }
    
    
}
