/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.Expr;
import editor.domain.elements.TemplateVariable;
import java.io.Serializable;
import java.util.Map;
import java.util.TreeMap;

/** Defines the binding of the template variables.
 *  The binding of a template variable is its expression.
 *
 * @author elvio
 */
public class TemplateBinding implements Serializable {
    
    // TemplateVariable unique name -> assigned Expr value
    public final Map<String, Expr> binding = new TreeMap<>();
    
    public Expr getSingleValueBoundTo(TemplateVariable tvar) {
        return binding.get(tvar.getUniqueName());
    }
    
    public void bindSingleValue(String varName, Expr singleVal) {
        binding.put(varName, singleVal);
    }
    
    public void unbind(String varName) {
        binding.remove(varName);
    }
    
    public TemplateBinding duplicate() {
        TemplateBinding tb = new TemplateBinding();
        tb.binding.putAll(binding);
        return tb;
    }
    
    public EvaluatedBinding createEvaluated(ParserContext context) {
        EvaluatedBinding eb = new EvaluatedBinding();
        for (Map.Entry<String, Expr> bind : binding.entrySet()) {
            String eval = bind.getValue().evaluate(context, EvaluationArguments.NO_ARGS)
                    .toStringFormat(ExpressionLanguage.PNPRO, "%f");
            eb.bindEvaluatedValue(bind.getKey(), eval);
        }
        return eb;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (Map.Entry<String, Expr> e : binding.entrySet()) {
            sb.append(count++ == 0 ? "" : ", ");
            sb.append(e.getKey()).append(" = ").append(e.getValue().getExpr());
        }
        return sb.toString();
    }
}
