/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import editor.domain.elements.ColorVar;
import editor.domain.values.MultiSet;
import java.util.HashMap;
import java.util.Map;

/** Defines the binding of the color variables in a transition firing.
 *
 * @author elvio
 */
public class ColorVarsBinding {
    
    private final Map<String, MultiSet> binding = new HashMap<>();
    
    public void bind(ColorVar var, MultiSet value) {
        assert this != EMPTY_BINDING;
        binding.put(var.getUniqueName(), value);
    }
    
    public boolean isVarBound(ColorVar var) {
        return binding.containsKey(var.getUniqueName());
    }
    
    public MultiSet getValueBoundTo(ColorVar var) {
        return binding.get(var.getUniqueName());
    }
    
    public MultiSet getValueBoundTo(String varName) {
        return binding.get(varName);
    }
    
    public static final ColorVarsBinding EMPTY_BINDING = new ColorVarsBinding();

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof ColorVarsBinding) {
            boolean result = binding.equals(((ColorVarsBinding)obj).binding);
            return result;
        }
        return false;
    }

    @Override
    public int hashCode() {
        int hash = 5;
        hash = 11 * hash + (this.binding != null ? this.binding.hashCode() : 0);
        return hash;
    }

    @Override
    public String toString() {
        if (this == EMPTY_BINDING)
            return "[]";
        StringBuilder sb = new StringBuilder("[");
        int i = 0;
        for (Map.Entry<String, MultiSet> e : binding.entrySet()) {
            sb.append(i++ == 0 ? "" : ", ").append(e.getKey()).append("=").append(e.getValue());
        }
        return sb.append("]").toString();
    }
}
