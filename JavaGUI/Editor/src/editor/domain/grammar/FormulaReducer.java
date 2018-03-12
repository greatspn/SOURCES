/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

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
