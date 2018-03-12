/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import java.util.Arrays;

/** Type parsed by the semantic parser, used to check static typing correctness.
 *
 * @author elvio
 */
public class ParsedType {
    // The type of the parsed expression
    public final EvaluatedFormula.Type type;
    
    // The domain, stored as a list of color class names.
    public final String[] domain;

    protected ParsedType(EvaluatedFormula.Type type) {
        this.type = type;
        this.domain = null;
    }

    protected ParsedType(EvaluatedFormula.Type type, String[] domain) {
        this.type = type;
        this.domain = domain;
        assert isScalar() || type != EvaluatedFormula.Type.BOOLEAN;
    }
    
    public final boolean isScalar()   { return domain == null; }
    public final boolean isMultiSet() { return domain != null; }
    
    public static ParsedType makeNew(EvaluatedFormula.Type type, String[] domain) {
        if (domain == null) {
            switch (type) {
                case INT:        return SCALAR_INT;
                case REAL:       return SCALAR_REAl;
                case BOOLEAN:    return SCALAR_BOOLEAN;
                case OBJECT:     return SCALAR_OBJECT;
            }
        }
        return new ParsedType(type, domain);
    }
    
    public static ParsedType makeNew(EvaluatedFormula.Type type) {
        return makeNew(type, null);
    }
    
    public static final ParsedType SCALAR_INT = new ParsedType(EvaluatedFormula.Type.INT);
    public static final ParsedType SCALAR_REAl = new ParsedType(EvaluatedFormula.Type.REAL);
    public static final ParsedType SCALAR_BOOLEAN = new ParsedType(EvaluatedFormula.Type.BOOLEAN);
    public static final ParsedType SCALAR_OBJECT = new ParsedType(EvaluatedFormula.Type.OBJECT);
//   
//    public static final ParsedType DEFINITION = new ParsedType(EvaluatedFormula.Type.DEFINITION);
//    public static final ParsedType ACT_SET = new ParsedType(EvaluatedFormula.Type.ACT_SET);
//    public static final ParsedType ERROR = new ParsedType(EvaluatedFormula.Type.ERROR);
    
    public final boolean sameDomainOf(ParsedType pt) {
        if (isScalar() != pt.isScalar())
            return false; // one is scalar, the other is multiset
        
        if (isMultiSet()) {
            if (domain == pt.domain)
                return true; // same domain array...
            if (domain.length != pt.domain.length)
                return false;
            for (int i=0; i<domain.length; i++)
                if (!domain[i].equals(pt.domain[i]))
                    return false;
        }
        return true;
    }

    // Test if two ParseTypes represent the same type.
    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (!(obj instanceof ParsedType))
            return false;
        
        ParsedType pt = (ParsedType)obj;
        if (type != pt.type)
            return false;
        
        return sameDomainOf(pt);
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 41 * hash + (this.type != null ? this.type.hashCode() : 0);
        hash = 41 * hash + Arrays.deepHashCode(this.domain);
        return hash;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(isScalar() ? "Scalar(" : "MultiSet(");
        switch (type) {
            case INT:       sb.append("INT");       break;
            case REAL:      sb.append("REAL");      break;
            case BOOLEAN:   sb.append("BOOLEAN");   break;
            case OBJECT:    sb.append("OBJECT");    break;
        }
        if (isMultiSet()) {
            sb.append(" : <");
            for (int i=0; i<domain.length; i++)
                sb.append(i==0 ? "" : ",").append(domain[i]);
            sb.append(">");
        }
        sb.append(")");
        return sb.toString();
    }
}
