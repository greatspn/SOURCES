/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

import editor.domain.EditableCell;
import editor.domain.Expr;
import editor.domain.MovementHandle;
import editor.domain.NetPage;
import editor.domain.SelectableObject;
import editor.domain.ViewProfile;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.measures.SolverParams;
import editor.domain.values.EvaluatedFormula;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

/** Describes a net that will be part of the multinet. 
 *  The net may appear in more than one instance, 
 *  and may have a partial parameter instantiation.
 *
 * @author elvio
 */
public class NetInstanceDescriptor extends SelectableObject implements Serializable {
    
    // Name of the net page
    public String targetNetName;
    
    // Number of instances
    public final SolverParams.IntExpr numReplicas = new SolverParams.IntExpr("1");
    
    // Partial parameter instantiations
    // Only parameters that will be assigned/replaced appears in this table
    // For replicated instances, instParams is applied uniformly to all the nets.
    public final TemplateBinding instParams = new TemplateBinding();
    // Variables that are kept unbound
//    public final Set<String> unboundParams = new HashSet<>();
    
    //----------------------------------------------------------
    // Transient reference to the target net
    public transient ComposableNet net;
    // Transient set of actual template parameters
    public transient Map<String, TemplateVariable> paramRefs;
    
    public Integer evalNumReplicas(ParserContext context) {
        try {
            EvaluatedFormula num = numReplicas.evaluate(context, EvaluationArguments.NO_ARGS);
            return num.getScalarInt();
        }
        catch (EvaluationException e) {
            // Expression depends on non-instantiated parameters
            return null;
        }
    }
    
//    public void copyBindingsInto(TemplateBinding tb, int repNum) {
//        for (Map.Entry<String, Expr> e : instParams.binding.entrySet())
//            tb.bindSingleValue(e.getKey(), e.getValue());
//    }
    
    public TemplateBinding getListOfBoundVars() {
        return instParams;
    }
//    public Set<String> getListOfUnboundVars() {
//        return unboundParams;
//    }
    
    public TemplateBinding getBindingOfReplica(int rep) {
        // Here we have the opportunity of modifying bound values for the specific replica
        return instParams;
    }
    
    // Add a new parameter to the set of instantiated parameters, to all the replicas
    public void bindParam(String param, Expr startValue) {
        instParams.bindSingleValue(param, startValue);
//        unboundParams.remove(param);
    }
    
    // Remove a parameter from the list of instantiated params
    public void unbindParam(String param) {
        instParams.unbind(param);
//        unboundParams.add(param);
    }
    
//    // Is this parameter in the list of known parameters?
//    public boolean isParamKnown(String param) {
//        return instParams.binding.containsKey(param) || unboundParams.contains(param);
//    }
    
    // Is this parameter already bound?
    public boolean isParamBound(String param) {
        return instParams.binding.containsKey(param);
    }
    
    // rename a binding
    public void renameBoundParam(String oldName, String newName) {
        instParams.bindSingleValue(newName, instParams.binding.get(oldName));
        instParams.binding.remove(oldName);
    }
    
    // Remove parameters that are not in the list of @params
    public void removeMissingParams(Set<TemplateVariable> params) {
//        Iterator<String> it1 = unboundParams.iterator();
//        while (it1.hasNext()) {
//            String par = it1.next();
//            
//            boolean found = false;
//            Iterator<TemplateVariable> itVar = params.iterator();
//            while (itVar.hasNext()) {
//                TemplateVariable tvar = itVar.next();
//                if (tvar.getUniqueName().equals(par)) {
//                    found = true;
//                    break;
//                }
//            }
//            if (!found) {
//                it1.remove();
//                System.out.println("Removing unbound param "+par+" from NetInstance of "+targetNetName);
//            }
//        }
        
        Iterator<Entry<String, Expr>> it2 = instParams.binding.entrySet().iterator();
        while (it2.hasNext()) {
            String par = it2.next().getKey();
            
            boolean found = false;
            Iterator<TemplateVariable> itVar = params.iterator();
            while (itVar.hasNext()) {
                TemplateVariable tvar = itVar.next();
                if (tvar.getUniqueName().equals(par)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                it2.remove();
                System.out.println("Removing bound param "+par+" from NetInstance of "+targetNetName);
            }
        }
    }

    @Override public MovementHandle getCenterHandle(NetPage thisPage) 
    {   throw new UnsupportedOperationException("Not supported."); }

    @Override public MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile) 
    {   throw new UnsupportedOperationException("Not supported."); }

    @Override public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile, boolean includeDecors) 
    {   throw new UnsupportedOperationException("Not supported."); }

    @Override public EditableCell getCentralEditable(double px, double py) 
    {   throw new UnsupportedOperationException("Not supported."); }
    
    @Override public boolean isLocked() { return false; }
}
