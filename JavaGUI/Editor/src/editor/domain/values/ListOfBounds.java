/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import editor.domain.elements.ColorClass;
import editor.domain.elements.NeutralColorClass;
import editor.domain.grammar.EvaluationException;
import java.util.ArrayList;
import java.util.Objects;

/** A list of bounds [a, b) of a general event PDF
 *
 * @author elvio
 */
public class ListOfBounds extends EvaluatedFormula {
    
    public ArrayList<Bound> bounds = new ArrayList<>();
    
    @Override
    public Type getType() {
        return Type.LIST_OF_BOUNDS;
    }

    @Override
    public ColorClass getDomain() {
        return NeutralColorClass.INSTANCE;
    }

    @Override
    public boolean isScalar() { return true; }

    @Override
    public boolean isMultiSet() { return false; }

    @Override
    public boolean isObject() { return false; }

    
    @Override
    public boolean equalsZero() {
        return bounds.isEmpty();
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 29 * hash + Objects.hashCode(this.bounds);
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        final ListOfBounds other = (ListOfBounds) obj;
        if (!Objects.equals(this.bounds, other.bounds)) {
            return false;
        }
        return true;
    }
    
    // derives the bound list of a sum of polynomial terms
    public static EvaluatedFormula joinBounds(EvaluatedFormula l1, EvaluatedFormula l2) {
        boolean isBound1 = (l1.getType() == Type.LIST_OF_BOUNDS);
        boolean isBound2 = (l2.getType() == Type.LIST_OF_BOUNDS);
        
        if (isBound1 && isBound2) {
            ListOfBounds nlb = new ListOfBounds();
            nlb.bounds.addAll(((ListOfBounds)l1).bounds);
            nlb.bounds.addAll(((ListOfBounds)l2).bounds);
            return nlb;
        }
        if (isBound1)
            return l1;
        if (isBound2)
            return l2;
        return UNBOUNDED;
    }
    
    // derives the bound of a polynomial term (product of subterms)
    public static EvaluatedFormula combineBounds(EvaluatedFormula l1, EvaluatedFormula l2) {
        boolean isBound1 = (l1.getType() == Type.LIST_OF_BOUNDS);
        boolean isBound2 = (l2.getType() == Type.LIST_OF_BOUNDS);
        
        if (isBound1 && isBound2) {
            if (l1 == UNBOUNDED)
                return l2;
            if (l2 == UNBOUNDED)
                return l1;
            
            // A combinable expression may only have a single non-trivial bound
            throw new EvaluationException("A polynomial term in a PDF may have at most "
                    + "one support specifier (I[a] or R[a,b])");
        }
        if (isBound1)
            return l1;
        if (isBound2)
            return l2;
        return UNBOUNDED;
    }
    
    public double getUpperBound() {
        if (bounds.isEmpty())
            return Double.MAX_VALUE;
        double upper = 0;
        for (Bound b : bounds)
            upper = Math.max(upper, b.b);
        return upper;
    }
    
    public double getLowerBound() {
        if (bounds.isEmpty())
            return 0;
        double lower = Double.MAX_VALUE;
        for (Bound b : bounds)
            lower = Math.min(lower, b.a);
        return lower;
    }
    
    
    // Add a new interval
    public void addBound(double a, double b) {
        for (Bound bnd : bounds) {
            if (bnd.overlaps(bnd)) {
                bnd.enlarge(a, b);
                return;
            }
        }
        bounds.add(new Bound(a, b));
    }
    
    
    //================== Object factory =====================
    
    public static ListOfBounds UNBOUNDED = new ListOfBounds();
    
    public static ListOfBounds makeNew(double a) {
        return makeNew(a, a);
    }
    
    public static ListOfBounds makeNew(double a, double b) {
        ListOfBounds rbv = new ListOfBounds();
        rbv.bounds.add(new Bound(a, b));
        return rbv;
    }
}
