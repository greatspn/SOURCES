/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import editor.domain.values.BooleanScalarValue;
import editor.domain.values.EvaluatedFormula;
import org.antlr.v4.runtime.tree.ParseTree;

/** Reduces all the parts of an expression that can be computed with the given
 *  evaluation arguments.
 *
 * @author elvio
 */
public class FormulaReducer extends SemanticParser {
    
    FormulaEvaluator eval;
    boolean applyReduce;
    
    public FormulaReducer(ParserContext context, int parseFlags, 
                          EvaluationArguments args, ExpressionLanguage lang,
                          boolean applyReduce) 
    {
        super(context, parseFlags);
        this.lang = lang;
        this.applyReduce = applyReduce;
        if (applyReduce)
            eval = new FormulaEvaluator(context, args);
        else
            eval = null;
    }

    @Override
    public FormattedFormula visit(ParseTree pt) {
        if (applyReduce) {
            
            // Preprocess if-then-else conditions
            // Try to evaluate first the condition of the if-then-else statement: 
            // if they can be evaluated as constants, replace the if statement with the correct branch
            try {
                if (pt instanceof ExprLangParser.RealExprCondContext) {
                    ExprLangParser.RealExprCondContext ptIfThenElse = (ExprLangParser.RealExprCondContext)pt;
                    FormattedFormula reducedCond = visit(ptIfThenElse.boolExpr());
                    EvaluatedFormula result = context.evaluate(reducedCond.getFormula(), 
                                                               ParserContext.ParserEntryPoint.BOOLEAN_EXPR, 
                                                               EvaluationArguments.NO_ARGS, 
                                                               "Reduced condition expression", 0);
                    return visit(ptIfThenElse.realExpr(result.getScalarBoolean() ? 0 : 1));
                }
                else if (pt instanceof ExprLangParser.RealExprCond2Context) {
                    ExprLangParser.RealExprCond2Context ptIfThenElse = (ExprLangParser.RealExprCond2Context)pt;
                    FormattedFormula reducedCond = visit(ptIfThenElse.boolExpr());
                    EvaluatedFormula result = context.evaluate(reducedCond.getFormula(), 
                                                               ParserContext.ParserEntryPoint.BOOLEAN_EXPR, 
                                                               EvaluationArguments.NO_ARGS, 
                                                               "Reduced condition expression", 0);
                    return visit(ptIfThenElse.realExpr(result.getScalarBoolean() ? 0 : 1));
                }
                else if (pt instanceof ExprLangParser.IntExprCondContext) {
                    ExprLangParser.IntExprCondContext ptIfThenElse = (ExprLangParser.IntExprCondContext)pt;
                    FormattedFormula reducedCond = visit(ptIfThenElse.boolExpr());
                    EvaluatedFormula result = context.evaluate(reducedCond.getFormula(), 
                                                               ParserContext.ParserEntryPoint.BOOLEAN_EXPR, 
                                                               EvaluationArguments.NO_ARGS, 
                                                               "Reduced condition expression", 0);
                    return visit(ptIfThenElse.intExpr(result.getScalarBoolean() ? 0 : 1));
                }
                else if (pt instanceof ExprLangParser.IntExprCond2Context) {
                    ExprLangParser.IntExprCond2Context ptIfThenElse = (ExprLangParser.IntExprCond2Context)pt;
                    FormattedFormula reducedCond = visit(ptIfThenElse.boolExpr());
                    EvaluatedFormula result = context.evaluate(reducedCond.getFormula(), 
                                                               ParserContext.ParserEntryPoint.BOOLEAN_EXPR, 
                                                               EvaluationArguments.NO_ARGS, 
                                                               "Reduced condition expression", 0);
                    return visit(ptIfThenElse.intExpr(result.getScalarBoolean() ? 0 : 1));
                }
            }
            catch (EvaluationException e) { /* do nothing, cannot evaluate the if condition */}
            
            
            
            // Preprocess boolean AND conditions
            if (pt instanceof ExprLangParser.BoolExprAndContext) {
                ExprLangParser.BoolExprAndContext ptAnd = (ExprLangParser.BoolExprAndContext)pt;
                for (int i=0; i<2; i++) {
                    try {
                        EvaluatedFormula result = eval.visit(ptAnd.boolExpr(i));
                        if (result.getScalarBoolean())
                            return visit(ptAnd.boolExpr(i==0 ? 1 : 0));
                        else
                            return new FormattedFormula(lang, result.isScalar(), 
                                                        result.toStringFormat(lang, null)); // False
                    }
                    catch (EvaluationException e) { /* do nothing, cannot preprocess logic condition */}
                }
            }
            


            // Preprocess boolean OR conditions
            if (pt instanceof ExprLangParser.BoolExprOrContext) {
                ExprLangParser.BoolExprOrContext ptOr = (ExprLangParser.BoolExprOrContext)pt;
                for (int i=0; i<2; i++) {
                    try {
                        EvaluatedFormula result = eval.visit(ptOr.boolExpr(i));
                        if (result.getScalarBoolean())
                            return new FormattedFormula(lang, result.isScalar(), 
                                                        result.toStringFormat(lang, null)); // True
                        else
                            return visit(ptOr.boolExpr(i==0 ? 1 : 0));
                    }
                    catch (EvaluationException e) { /* do nothing, cannot preprocess logic condition */}
                }
            }
            
            
            
            // General case
            try {
                // Try compute the value of this expression
                EvaluatedFormula result = eval.visit(pt);
                // Return the evaluated value;
                String reduced = result.toStringFormat(lang, null);
                FormulaPayload payload = null;
                if (result.isMultiSet())
                    payload = new MultiSetElemType(result.getDomain());
                return new FormattedFormula(lang, result.isScalar(), reduced, payload);
            }
            catch (EvaluationException e) {
                // ignore evaluation error, means that it is not reducible
            }
        }
        // Not evaluatable, return the expression as it is
        return super.visit(pt);        
    }

    
}
