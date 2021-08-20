/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.values.EvaluatedFormula;
import java.util.Set;
import org.antlr.v4.runtime.tree.ParseTree;

/** Drop all Boolean sub-expressions that cannot be fully evaluated
 * given a set of symbols.
 * ex: symbols = {a,b,c}
 *   (a and b) or (d and c) ==> (a and b) or (True)
 *    since (d and c) cannot be evaluated, it is replaced with True
 *
 * @author elvio
 */
public class FormulaDropper extends SemanticParser {
//    FormulaEvaluator eval;
    Set<String> knownSymbols;
    
    public FormulaDropper(ParserContext context, int parseFlags, 
                          ExpressionLanguage lang,
                          Set<String> knownSymbols) 
    {
        super(context, parseFlags);
        this.lang = lang;
        this.knownSymbols = knownSymbols;
//        this.eval = new FormulaEvaluator(context, args);
    }

//    @Override
//    public FormattedFormula visit(ParseTree pt) {
//        if (applyReduce) {
//            try {
//                // Try compute the value of this expression
//                EvaluatedFormula result = eval.visit(pt);
//                // Return the evaluated value;
//                String reduced = result.toStringFormat(lang, null);
//                FormulaPayload payload = null;
//                if (result.isMultiSet())
//                    payload = new MultiSetElemType(result.getDomain());
//                return new FormattedFormula(lang, result.isScalar(), reduced, payload);
//            }
//            catch (EvaluationException e) {
//                // ignore evaluation error, means that it is not reducible
//            }
//        }
//        // Not evaluatable, return the expression as it is
//        return super.visit(pt);        
//    }
    
//    @Override
//    public FormattedFormula visit(ParseTree pt) {
//        if (applyReduce) {
//            try {
//                // Try compute the value of this expression
//                EvaluatedFormula result = eval.visit(pt);
//                // Return the evaluated value;
//                String reduced = result.toStringFormat(lang, null);
//                FormulaPayload payload = null;
//                if (result.isMultiSet())
//                    payload = new MultiSetElemType(result.getDomain());
//                return new FormattedFormula(lang, result.isScalar(), reduced, payload);
//            }
//            catch (EvaluationException e) {
//                // ignore evaluation error, means that it is not reducible
//            }
//        }
//        // Not evaluatable, return the expression as it is
//        return super.visit(pt);        
//    }

    @Override
    public FormattedFormula visitColorVarIdentifier(ExprLangParser.ColorVarIdentifierContext ctx) {
        if (!knownSymbols.contains(ctx.COLORVAR_ID().getText())) {
            throw new EvaluationDropException("");
        }
        return super.visitColorVarIdentifier(ctx);
    }
    
    

    @Override
    public FormattedFormula visitBoolExprNot(ExprLangParser.BoolExprNotContext ctx) {
        FormattedFormula fA = super.visitBoolExprNot(ctx);
        
        if (fA.getFormula().equals("True"))
            return fA.replaceFormula("False");
        else if (fA.getFormula().equals("False"))
            return fA.replaceFormula("True");
        else
            return fA;
    }

    @Override
    public FormattedFormula visitBoolExprOr(ExprLangParser.BoolExprOrContext ctx) {
        FormattedFormula fA = super.visit(ctx.boolExpr(0));
        FormattedFormula fB = super.visit(ctx.boolExpr(1));
        
        if (fA.getFormula().equals("True")) { // True OR B
            return fA;
        }
        else if (fA.getFormula().equals("False")) { // False OR B
            return fB;
        }
        else { // A OR B
            if (fB.getFormula().equals("True")) // A OR True
                return fB;
            else if (fB.getFormula().equals("False")) // A OR False
                return fA;
            else
                return super.visitBoolExprOr(ctx); 
        }
    }

    @Override
    public FormattedFormula visitBoolExprAnd(ExprLangParser.BoolExprAndContext ctx) {
        FormattedFormula fA = super.visit(ctx.boolExpr(0));
        FormattedFormula fB = super.visit(ctx.boolExpr(1));
        
        if (fA.getFormula().equals("True")) { // True AND B
            return fB;
        }
        else if (fA.getFormula().equals("False")) { // False AND B
            return fA;
        }
        else { // A AND B
            if (fB.getFormula().equals("True")) // A AND True
                return fA;
            else if (fB.getFormula().equals("False")) // A AND False
                return fB;
            else
                return super.visitBoolExprAnd(ctx);
        }
    }

    @Override
    public FormattedFormula visitBoolExprColorTermIn(ExprLangParser.BoolExprColorTermInContext ctx) {
        try {
            return super.visitBoolExprColorTermIn(ctx); 
        }
        catch (EvaluationDropException e) {
            return new FormattedFormula(lang, true, "True");
        }
    }

    @Override
    public FormattedFormula visitBoolExprColorTermComp(ExprLangParser.BoolExprColorTermCompContext ctx) {
        try {
            return super.visitBoolExprColorTermComp(ctx); 
        }
        catch (EvaluationDropException e) {
            return new FormattedFormula(lang, true, "True");
        }
    }

    @Override
    public FormattedFormula visitBoolExprIntComp(ExprLangParser.BoolExprIntCompContext ctx) {
        try {
            return super.visitBoolExprIntComp(ctx); 
        }
        catch (EvaluationDropException e) {
            return new FormattedFormula(lang, true, "True");
        }
    }
    
    
}
