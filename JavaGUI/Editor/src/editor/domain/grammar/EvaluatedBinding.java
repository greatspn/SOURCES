/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import editor.domain.elements.TemplateVariable;
import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

/** An evaluated binding between a template variable and an evaluated expression
 *
 * @author elvio
 */
public class EvaluatedBinding implements Serializable {
    
    // From temlate variable to evaluated expression value
    public Map<String, String> binding = new HashMap<>();

    public EvaluatedBinding() { }
        
    public String getSingleValueBoundTo(TemplateVariable tvar) {
        return binding.get(tvar.getUniqueName());
    }
    
    public void bindEvaluatedValue(String varName, String eval) {
        binding.put(varName, eval);
    }
    
    public String getBindingRepr() {
        StringBuilder sb = new StringBuilder();
        for (Map.Entry<String, String> e : binding.entrySet()) {
            if (sb.length() > 0)
                sb.append(", ");
            sb.append(e.getKey()).append("=").append(e.getValue());
        }
        return sb.toString();
    }
}
