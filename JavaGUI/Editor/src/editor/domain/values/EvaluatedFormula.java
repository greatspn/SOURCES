/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import common.Tuple;
import editor.domain.elements.ColorClass;
import editor.domain.elements.NeutralColorClass;
import editor.domain.grammar.DomainElement;
import editor.domain.grammar.EvaluationException;
import editor.domain.grammar.ExprLangParser;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.play.GspnDtaPlayEngine;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;

/** The result of an evaluation of a int/real/boolean formula, 
 * encapsulated in an immutable class.
 *
 * @author elvio
 */
public abstract class EvaluatedFormula {

    // The type of this evaluated formula
    public static enum Type { 
        INT, REAL, BOOLEAN, OBJECT,
        
        // Special types, used for some specialized task
        //INT_LIST, REAL_LIST
        LIST_OF_BOUNDS // computation of bounds of general event pdfs
    };
    public abstract Type getType();
    
    // The domain of the result (null means scalar/neutral)
    public abstract ColorClass getDomain();
    public abstract boolean isScalar();
    public abstract boolean isMultiSet();
    public abstract boolean isObject();
    public boolean isBlocked() { return false; }
    
    public int getScalarInt() {
        assert this instanceof IntScalarValue;
        return ((IntScalarValue)this).value;
    }
    
    public double getScalarReal() {
        assert this instanceof RealScalarValue;
        return ((RealScalarValue)this).value;
    }
    
    public double getScalarRealOrIntAsReal() {
        if (this instanceof IntScalarValue)
            return ((IntScalarValue)this).value;
        else if (this instanceof RealScalarValue)
            return ((RealScalarValue)this).value;
        else throw new IllegalStateException("Not int or real.");
    }
    
    public boolean getScalarBoolean() {
        assert this instanceof BooleanScalarValue;
        return ((BooleanScalarValue)this).value;
    }
    
    
    // Returns true if this value can be a multiplicity value in a multiset
    public boolean isValidMultiSetValue() {
        if (this instanceof IntScalarValue)
            return ((IntScalarValue)this).value != 0;
        else if (this instanceof RealScalarValue)
            return ((RealScalarValue)this).value != 0.0;
        else if (this instanceof BooleanScalarValue)
            return true;//((BooleanScalarValue)this).value; // Always consider all True & False
        else if (isBlocked())
            return (!toString().equals("0") || !toString().equals("0.0"));
        else throw new IllegalStateException("Not int or real.");
    }
    
    // Cardinality of a given color in this color set, or zero
    public EvaluatedFormula getMultiSetColorCard(DomainElement elem) {
        if (!(isMultiSetInt() && getDomain().equals(elem.getDomain()))) {
            System.out.println("");
        }
        assert isMultiSetInt() && getDomain().equals(elem.getDomain());
        ValuedMultiSet mset = (ValuedMultiSet)this;
        for (int i=0; i<mset.numElements(); i++) {
            if (mset.entries[i].compareTo(elem) == 0) 
                return mset.values[i];
        }
        return makeZero(getType(), getDomain());
    }
    
    public boolean isScalarInt()      { return (this instanceof IntScalarValue); }
    public boolean isScalarReal()     { return (this instanceof RealScalarValue); }
    public boolean isScalarBoolean()  { return (this instanceof BooleanScalarValue); }

    public boolean isMultiSetInt()      { return (this instanceof MultiSet) && getType() == Type.INT; }
    public boolean isMultiSetReal()     { return (this instanceof MultiSet) && getType() == Type.REAL; }
    public boolean isMultiSetBoolean()  { return (this instanceof MultiSet) && getType() == Type.BOOLEAN; }
    
//    public int getInt() { return getScalarInt(); }
    public double getReal() { return getScalarReal(); }
    
//    public boolean getBoolean() { return getScalarBoolean(); }
    @Override
    public String toString() {
        return toStringFormat(ExpressionLanguage.PNPRO, "%.2f");
    }
    
    public String toStringFormat(ExpressionLanguage lang, String realFormat) {
        assert !isBlocked();
        if (isMultiSet()) {
            String dot;
            boolean maySkipOne = true;
            switch (lang) {
                case LATEX:     dot = "\\cdot";    break;
                case PNPRO:     dot = "*";         break;
                case APNN:      dot = "'"; maySkipOne = false;  break;
                default:
                    throw new UnsupportedOperationException();
            }
            StringBuilder sb = new StringBuilder();
            MultiSet ms = (MultiSet)this;
            for (int j=0; j<ms.numElements(); j++) {
                sb.append(j == 0 ? "" : " + ");
                if (isMultiSetReal() || !maySkipOne ||
                    (isMultiSetInt() && ms.getValue(j).getScalarInt() != 1))
                    sb.append(ms.getValue(j).toStringFormat(lang, realFormat)).append(dot);
                sb.append(ms.getElement(j).toStringFormat(lang));
            }
            return sb.toString();
        }
        switch (getType()) {
            case INT:
                return "" + getScalarInt();
            case REAL:
                if (realFormat != null)
                    return String.format(Locale.US, realFormat, getScalarReal());
                return "" + getScalarReal();
            case BOOLEAN:
                switch (lang) {
                    case LATEX:  
                        return getScalarBoolean() ? "\\mathrm{true}" : "\\mathrm{false}";
                    case APNN:
                        return getScalarBoolean() ? "true" : "false";
                    case PNPRO:  
                        return getScalarBoolean() ? "True" : "False";
                    default:
                            throw new UnsupportedOperationException();
                }                
            case OBJECT:
                return ((ObjectValue)this).toStringFormat(lang);
            default:
                throw new IllegalStateException("Type mismatch in toStringShort()");            
        }
    }
    
    public abstract boolean equalsZero();
    
    public static EvaluatedFormula makeZero(Type type, ColorClass domain) {
        if (domain.isNeutralDomain()) {
            switch (type) {
            case INT:
                return IntScalarValue.ZERO;
            case REAL:
                return RealScalarValue.ZERO;
            default:
                throw new IllegalStateException("Type mismatch in makeZero()");
            }
        }
        else 
            return MultiSet.makeNew(type, domain, EMPTY_MAP);
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof EvaluatedFormula))
            return false;
        EvaluatedFormula e = (EvaluatedFormula)obj;
        
        return getType() == e.getType() && getDomain() == e.getDomain();
    }

    @Override
    public int hashCode() {
        int hash = 5;
        hash = 11 * hash + getType().hashCode();
        hash = 11 * hash + getDomain().hashCode();
        return hash;
    }

    //--------------------------------------------------------------------------
    private static final Map<Integer, Integer> factorialMem = new HashMap<>();
    private static int factorialMemoized(int i) {
        if (i <= 1)
            return 1;
        if (factorialMem.containsKey(i))
            return factorialMem.get(i);
        int val = factorialMemoized(i-1) * i;
        factorialMem.put(i, val);
        return val;
    }
    
    private static final Map<Tuple<Integer,Integer>, Integer> binomialMem = new HashMap<>();
    private static int binomialMemoized(int n, int k) {
	if (k <= 0 || k >= n)
		return 0;
	if (k == 1 || k == n-1)
		return n;
	k = Math.min(k, n-k);
	
	Tuple<Integer, Integer> binom = new Tuple<>(n, k);
        if (binomialMem.containsKey(binom))
            return binomialMem.get(binom);
	
	int val = binomialMemoized(n-1, k) + binomialMemoized(n-1, k-1);
	binomialMem.put(binom, val);
	return val;        
    }
    
    //--------------------------------------------------------------------------
    
    private static EvaluatedFormula THR() {
        throw new UnsupportedOperationException();
    }
    
    private EvaluatedFormula evalUnaryFnScalar(int unaryFn) {
        assert isScalar();
        Type computeType = getType();
        
        if (computeType == Type.INT) {
            int value = getScalarInt(), result;
            switch (unaryFn) {
                case ExprLangParser.SUB:           result = -value;                    break;
                case ExprLangParser.ABS_FN:        result = Math.abs(value);           break;
                case ExprLangParser.FACTORIAL_FN:  result = factorialMemoized(value);  break;
                case ExprLangParser.INT_TO_REAL:   return RealScalarValue.makeNew(value);
                case ExprLangParser.MULTISET_MIN:
                case ExprLangParser.MULTISET_MAX:
                case ExprLangParser.MULTISET_CARD: 
                case ExprLangParser.MULTISET_FILTER_NEGATIVES: 
                    return this;
                default:
                    throw new UnsupportedOperationException("Unsupported unary operator "+unaryFn);
            }
            return IntScalarValue.makeNew(result);
        }
        else if (computeType == Type.REAL) {
            double value = getScalarRealOrIntAsReal(), result;
            switch (unaryFn) {
                case ExprLangParser.SUB:          result = -value;              break;
                case ExprLangParser.ABS_FN:       result = Math.abs(value);     break;                    
                case ExprLangParser.EXP_FN:       result = Math.exp(value);     break;                    
                case ExprLangParser.SQRT_FN:      result = Math.sqrt(value);    break;                    
                case ExprLangParser.SIN_FN:       result = Math.sin(value);     break;                    
                case ExprLangParser.COS_FN:       result = Math.cos(value);     break;                    
                case ExprLangParser.TAN_FN:       result = Math.tan(value);     break;                    
                case ExprLangParser.ARCSIN_FN:    result = Math.asin(value);    break;                    
                case ExprLangParser.ARCCOS_FN:    result = Math.acos(value);    break;                    
                case ExprLangParser.ARCTAN_FN:    result = Math.atan(value);    break;
                case ExprLangParser.MULTISET_MIN:
                case ExprLangParser.MULTISET_MAX:
                case ExprLangParser.MULTISET_CARD: 
                case ExprLangParser.MULTISET_FILTER_NEGATIVES: 
                    result = value;               break;
                case ExprLangParser.CEIL_FN:      return IntScalarValue.makeNew((int)Math.ceil(value));                 
                case ExprLangParser.FLOOR_FN:     return IntScalarValue.makeNew((int)Math.floor(value));               
                case ExprLangParser.ROUND_FN:     return IntScalarValue.makeNew((int)Math.round(value));
                    
                case ExprLangParser.DIRAC_DELTA_FN:
                    throw new EvaluationException("Cannot evaluate a General Distribution descriptor.");
                default:
                    throw new UnsupportedOperationException("Unsupported unary operator "+unaryFn);
            }
            return RealScalarValue.makeNew(result);
        }
        else if (computeType == Type.BOOLEAN) {
            boolean value = getScalarBoolean(), result;
            switch (unaryFn) {
                case ExprLangParser.NOT:          
                    result = !value;
                    break;
                    
                case ExprLangParser.COLOR_ORDINAL:
                    result = value;
                    break;
                
                default:
                    throw new UnsupportedOperationException("Unsupported unary operator "+unaryFn);
            }
            return BooleanScalarValue.makeNew(result);
        }
        else throw new UnsupportedOperationException();
    }
    
    private static Type getUnaryResultType(Type computeType, int unaryFn) {
        if (unaryFn == ExprLangParser.MULTISET_CARD || unaryFn == ExprLangParser.COLOR_ORDINAL)
            return Type.INT;
        if (computeType == Type.INT) {
            if (unaryFn == ExprLangParser.INT_TO_REAL)
                return Type.REAL;
        }
        else if (computeType == Type.REAL) {
            switch (unaryFn) {
                case ExprLangParser.FLOOR_FN:
                case ExprLangParser.CEIL_FN:
                case ExprLangParser.ROUND_FN:
                    return Type.INT;
            }
        }
        return computeType;
    }
    
    public EvaluatedFormula evalUnaryFn(int unaryFn) 
    {
        // The result type of the unary operators is always the input type
        Type computeType = getType();
        // The result type could be different from the computeType, depending on the operator.
        Type resultType = getUnaryResultType(computeType, unaryFn);
        // The neutral term:
        EvaluatedFormula neutralTerm = neutralTermForOperationFn(resultType, unaryFn, UNARY_TERM);
        
        // Scalar unary operation
        if (isScalar())
            return evalUnaryFnScalar(unaryFn);
        
        // Multiset unary operation
        MultiSet msv = (MultiSet)this;
        Map<DomainElement, EvaluatedFormula> newSet = new TreeMap<>();
        for (int i=0; i<msv.numElements(); i++) {
            EvaluatedFormula newVal = msv.getValue(i).evalUnaryFnScalar(unaryFn);
            if (newVal.isValidMultiSetValue())
                newSet.put(msv.getElement(i), newVal);
        }
        return aggregateResult(newSet, unaryFn, resultType, getDomain(), neutralTerm);
    }
    
    //--------------------------------------------------------------------------
    
    private static final int UNARY_TERM = 0;
    private static final int LEFT_TERM = 1;
    private static final int RIGHT_TERM = 2;

    // return the neutral term of the given unary/binary operation
    private static EvaluatedFormula neutralTermForOperationFn(Type outType, int opFn, int leftRight) {
        if (outType == null)
            return null; // blocked expressions
        boolean outInt = (outType == Type.INT);
        boolean outReal = (outType == Type.REAL);
        boolean outBool = (outType == Type.BOOLEAN);
        assert outInt || outReal || outBool;

        switch (opFn) {
            case ExprLangParser.ADD:
            case ExprLangParser.SUB:
            case ExprLangParser.EQUAL:
            case ExprLangParser.NOT_EQUAL:
            case ExprLangParser.LESS:
            case ExprLangParser.LESS_EQ:
            case ExprLangParser.GREATER:
            case ExprLangParser.GREATER_EQ:
            case ExprLangParser.SAFE_DIV:
            case ExprLangParser.CONTAINS:
            case ExprLangParser.MULTISET_CARD:
            case ExprLangParser.COLOR_ORDINAL:
                // Return the zero term
                return outInt ? IntScalarValue.ZERO :
                       outReal ? RealScalarValue.ZERO :
                       outBool ? BooleanScalarValue.FALSE :
                       THR();
                
            case ExprLangParser.LESS_ZEROINF:  // right evaluates to inf, left to zero
                if (leftRight == RIGHT_TERM) // Return infinity
                    return outInt ? IntScalarValue.MAXIMUM :
                           outReal ? RealScalarValue.MAXIMUM :
                           THR();
                else
                    return outInt ? IntScalarValue.ZERO :
                           outReal ? RealScalarValue.ZERO :
                           THR();
                
            case ExprLangParser.MUL:
                // Return the unit term
                return outInt ? IntScalarValue.ONE :
                       outReal ? RealScalarValue.ONE :
                       THR();
                
            case ExprLangParser.MAX_FN:
            case ExprLangParser.MULTISET_MAX:
                // Return the minimum value
                return outInt ? IntScalarValue.MINIMUM :
                       outReal ? RealScalarValue.MINIMUM :
                       THR();
                
            case ExprLangParser.MIN_FN:
            case ExprLangParser.MULTISET_MIN:
                // Return the maximum value
                return outInt ? IntScalarValue.MAXIMUM :
                       outReal ? RealScalarValue.MAXIMUM :
                       THR();
                
            case ExprLangParser.AND:
            case ExprLangParser.SETMINUS:
                // Return the FALSE value
                return outBool ? BooleanScalarValue.FALSE : 
                       THR();
                
            case ExprLangParser.OR:
            case ExprLangParser.UNION:
                // Return the TRUE value
                return outBool ? BooleanScalarValue.TRUE : 
                       THR();
                
            case ExprLangParser.DIRAC_DELTA_FN:
            case ExprLangParser.RECT_FN:
            case ExprLangParser.UNIFORM_FN:
            case ExprLangParser.TRIANGULAR_FN:
            case ExprLangParser.ERLANG_FN:
            case ExprLangParser.TRUNCATED_EXP_FN:
            case ExprLangParser.PARETO_FN:
                
            case ExprLangParser.NOT:
            case ExprLangParser.FRACT_FN:
            case ExprLangParser.DIV:
            case ExprLangParser.MOD_FN:
            case ExprLangParser.BINOMIAL_FN:
            case ExprLangParser.POW_FN:
            case ExprLangParser.MULTISET_FILTER_NEGATIVES:
            case ExprLangParser.INT_TO_REAL:
                
            case ExprLangParser.ABS_FN:
            case ExprLangParser.FACTORIAL_FN:
            case ExprLangParser.EXP_FN:
            case ExprLangParser.SQRT_FN:
            case ExprLangParser.SIN_FN:
            case ExprLangParser.COS_FN:
            case ExprLangParser.TAN_FN:
            case ExprLangParser.ARCSIN_FN:
            case ExprLangParser.ARCCOS_FN:
            case ExprLangParser.ARCTAN_FN:
            case ExprLangParser.CEIL_FN:
            case ExprLangParser.FLOOR_FN:
            case ExprLangParser.ROUND_FN: 
                // Return an invalid value, that cannot be used.
                return null;
                
            default:
                throw new UnsupportedOperationException("Missing neutral term case for "+opFn+" in neutralTermForOperationFn().");
        }
    }
    
    private Type getBinaryComputeType(EvaluatedFormula op) {
        // Both input are ints, return int
        if (getType() == Type.INT && op.getType() == Type.INT)
            return Type.INT;
        // Both are int or real, return real
        if ((getType() == Type.INT || getType() == Type.REAL) &&
            (op.getType() == Type.INT || op.getType() == Type.REAL))
            return Type.REAL;
        // Both are boolean, return boolean
        if (getType() == Type.BOOLEAN && op.getType() == Type.BOOLEAN)
            return Type.BOOLEAN;
        
//        System.out.println("getType()="+getType()+" op.getType()="+op.getType());
        // Don't know
        return null;
    }
    
    private static Type getBinaryResultType(Type computeType, int binaryFn) {
        switch (binaryFn) {
            case ExprLangParser.EQUAL:
            case ExprLangParser.NOT_EQUAL:
            case ExprLangParser.LESS:
            case ExprLangParser.LESS_EQ:
            case ExprLangParser.GREATER:
            case ExprLangParser.GREATER_EQ:
            case ExprLangParser.LESS_ZEROINF:
            case ExprLangParser.CONTAINS:
                return Type.BOOLEAN;
                
            case ExprLangParser.MULTISET_CARD:
                return Type.INT;
                
            default:
                return computeType;
        }
    }
    
    // Normal functions return a scalar given a scalar input, and a multiset
    // given a multiset input. An aggregate function always return a scalar
    // result, for inputs that are both scalar or multiset.
    private static EvaluatedFormula aggregateResult(Map<DomainElement, EvaluatedFormula> newSet,
                                                    int unaryOrBinaryFn, Type resultType, 
                                                    ColorClass domain, EvaluatedFormula neutralTerm) 
    {
        switch (unaryOrBinaryFn) {
            case ExprLangParser.EQUAL:
            case ExprLangParser.NOT_EQUAL:
            case ExprLangParser.LESS:
            case ExprLangParser.LESS_EQ:
            case ExprLangParser.GREATER:
            case ExprLangParser.GREATER_EQ:
            case ExprLangParser.LESS_ZEROINF:
            {
                // Aggregate the (boolean) result - true only if every term is true
                assert resultType == Type.BOOLEAN;
                boolean isTrue = !newSet.isEmpty();
                for (EvaluatedFormula value : newSet.values())
                    isTrue = isTrue && value.getScalarBoolean();
                
                return BooleanScalarValue.makeNew(isTrue);
            }
            
            case ExprLangParser.CONTAINS:
            {
                // Aggregate the (boolean) result - true only if at least one term is true
                assert resultType == Type.BOOLEAN;
                boolean isTrue = false;
                for (EvaluatedFormula value : newSet.values())
                    isTrue = isTrue || value.getScalarBoolean();
                
                return BooleanScalarValue.makeNew(isTrue);
            }
            
            case ExprLangParser.MULTISET_CARD: {
                EvaluatedFormula result = neutralTerm;
                for (EvaluatedFormula value : newSet.values())
                    result = result.add(value);
                assert result.isScalar() && result.getType() == resultType;
                return result;
            }

            case ExprLangParser.MULTISET_MIN:
            case ExprLangParser.MULTISET_MAX: {
                if (newSet.isEmpty())
                    break;
                EvaluatedFormula result = neutralTerm;
                for (EvaluatedFormula value : newSet.values()) {
                    if (unaryOrBinaryFn == ExprLangParser.MULTISET_MIN)
                        result = result.min(value);
                    else
                        result = result.max(value);
                } 
                return result;
            }
                
            case ExprLangParser.MULTISET_FILTER_NEGATIVES: {
                Iterator<Map.Entry<DomainElement, EvaluatedFormula>> it;
                it = newSet.entrySet().iterator();
                while (it.hasNext()) {
                    if (it.next().getValue().getScalarRealOrIntAsReal() <= 0)
                        it.remove();
                }
                break;
            }
                
            case ExprLangParser.COLOR_ORDINAL: {
                EvaluatedFormula result = neutralTerm;
                Iterator<Map.Entry<DomainElement, EvaluatedFormula>> it;
                it = newSet.entrySet().iterator();
                while (it.hasNext()) {
                    Map.Entry<DomainElement, EvaluatedFormula> e = it.next();
                    if (e.getValue().getScalarBoolean()) {
//                        System.out.println("value = "+e.getKey().getColor(0));
                        result = result.add(IntScalarValue.makeNew(e.getKey().getColor(0) + 1));
                    }
                }
                assert result.isScalar() && result.getType() == resultType;
                return result;
            }
        }
        // Remove zero values
        Iterator<Map.Entry<DomainElement, EvaluatedFormula>> it = newSet.entrySet().iterator();
        while (it.hasNext()) {
            EvaluatedFormula ef = it.next().getValue();
            if (ef.isBlocked()) {
                if (ef.toString().equals("0") || ef.toString().equals("0.0"))
                    it.remove();
            }
            else {
                switch (ef.getType()) {
                case INT:
                    if (ef.getScalarInt() == 0)
                        it.remove();
                    break;
                case REAL:
                    if (Math.abs(ef.getScalarReal()) < GspnDtaPlayEngine.EPSILON_VALUE)
                        it.remove();
                    break;
                case BOOLEAN:
                    if (!ef.getScalarBoolean())
                        it.remove();
                    break;
                }
            }
        }
        
        // Do not aggregate the result - return a multiset
        return MultiSet.makeNew(resultType, domain, newSet);
    }
    
    private EvaluatedFormula evalBinaryFnScalar(int binaryFn, EvaluatedFormula op) {
        assert isScalar() && op.isScalar();
        Type computeType = getBinaryComputeType(op);
        
        if (isBlocked() || op.isBlocked()) {
            switch (binaryFn) {
                case ExprLangParser.ADD:  
                    if (equalsZero())
                        return op;
                    if (op.equalsZero())
                        return this;
                    return new UnevaluatedFormula(computeType, NeutralColorClass.INSTANCE, 
                                                  toString()+" + "+op.toString());
                case ExprLangParser.SUB:  
                    if (equalsZero())
                        return op;
                    if (op.equalsZero())
                        return this;
                    return new UnevaluatedFormula(computeType, NeutralColorClass.INSTANCE, 
                                                  toString()+" - "+op.toString());
                case ExprLangParser.MUL:  
                    if (equalsZero())
                        return op;
                    if (op.equalsZero())
                        return this;
                    return new UnevaluatedFormula(computeType, NeutralColorClass.INSTANCE, 
                                                  toString()+" * "+op.toString());
                case ExprLangParser.DIV:  
                    if (equalsZero())
                        return op;
                    if (op.equalsZero())
                        return this;
                    return new UnevaluatedFormula(computeType, NeutralColorClass.INSTANCE, 
                                                  toString()+" / "+op.toString());
                case ExprLangParser.AND:
                    if (equalsZero()) // false
                        return BooleanScalarValue.FALSE;
                    if (op.equalsZero()) // false
                        return BooleanScalarValue.FALSE;
                    return new UnevaluatedFormula(computeType, NeutralColorClass.INSTANCE, 
                                                  toString()+" && "+op.toString());
                   
                case ExprLangParser.EQUAL:  
                    // Both terms must be unblocked to perform the test
                    return new UnevaluatedFormula(computeType, NeutralColorClass.INSTANCE, 
                                                  toString()+"  "+op.toString());
                default:
                    throw new UnsupportedOperationException("Unsupported: blocked binaryFn = "+binaryFn);
            }
        }
        
        if (computeType == Type.INT) {
            final int value1 = getScalarInt(), value2 = op.getScalarInt(), result;
            switch (binaryFn) {
                case ExprLangParser.ADD:        result = value1 + value2;           break;
                case ExprLangParser.SUB:        result = value1 - value2;           break;
                case ExprLangParser.MUL:        result = value1 * value2;           break;
                case ExprLangParser.FRACT_FN:
                case ExprLangParser.DIV:        result = value1 / value2;           break;
                case ExprLangParser.SAFE_DIV:   
                    result = (value2 != 0) ? (value1 / value2) : 0;
                    break;
                case ExprLangParser.MOD_FN:     result = value1 % value2;           break;
                case ExprLangParser.MAX_FN:     result = Math.max(value1, value2);  break;
                case ExprLangParser.MIN_FN:     result = Math.min(value1, value2);  break;
                case ExprLangParser.BINOMIAL_FN: 
                    result = binomialMemoized(value1, value2);  
                    break;
                case ExprLangParser.POW_FN:     result = (int)Math.pow(value1, value2);  break;
                case ExprLangParser.EQUAL:      return BooleanScalarValue.makeNew(value1 == value2);
                case ExprLangParser.NOT_EQUAL:  return BooleanScalarValue.makeNew(value1 != value2);
                case ExprLangParser.LESS:       return BooleanScalarValue.makeNew(value1 < value2);
                case ExprLangParser.LESS_EQ:    return BooleanScalarValue.makeNew(value1 <= value2);
                case ExprLangParser.GREATER:    return BooleanScalarValue.makeNew(value1 > value2);
                case ExprLangParser.GREATER_EQ: return BooleanScalarValue.makeNew(value1 >= value2);
                case ExprLangParser.LESS_ZEROINF:return BooleanScalarValue.makeNew(value1 < value2);
                case ExprLangParser.CONTAINS:   return BooleanScalarValue.makeNew(value1 != 0 && value2 != 0);

                default:
                    throw new UnsupportedOperationException("Unsupported: binaryFn = "+binaryFn);
            }
            return IntScalarValue.makeNew(result);
        }
        else if (computeType == Type.REAL) {
            final double value1 = getScalarRealOrIntAsReal();
            final double value2 = op.getScalarRealOrIntAsReal(), result;
            switch (binaryFn) {
                case ExprLangParser.ADD:        result = value1 + value2;           break;
                case ExprLangParser.SUB:        result = value1 - value2;           break;
                case ExprLangParser.MUL:        result = value1 * value2;           break;
                case ExprLangParser.FRACT_FN:
                case ExprLangParser.DIV:        result = value1 / value2;           break;
                case ExprLangParser.MOD_FN:     result = value1 % value2;           break;
                case ExprLangParser.SAFE_DIV:   
                    result = (Math.abs(value2) > GspnDtaPlayEngine.EPSILON_VALUE) ? (value1 / value2) : 0.0;
                    break;
                case ExprLangParser.MAX_FN:     result = Math.max(value1, value2);  break;
                case ExprLangParser.MIN_FN:     result = Math.min(value1, value2);  break;
                case ExprLangParser.POW_FN:     result = Math.pow(value1, value2);  break;
                case ExprLangParser.EQUAL:      return BooleanScalarValue.makeNew(value1 == value2);
                case ExprLangParser.NOT_EQUAL:  return BooleanScalarValue.makeNew(value1 != value2);
                case ExprLangParser.LESS:       return BooleanScalarValue.makeNew(value1 < value2);
                case ExprLangParser.LESS_EQ:    return BooleanScalarValue.makeNew(value1 <= value2);
                case ExprLangParser.GREATER:    return BooleanScalarValue.makeNew(value1 > value2);
                case ExprLangParser.GREATER_EQ: return BooleanScalarValue.makeNew(value1 >= value2);
                case ExprLangParser.LESS_ZEROINF: return BooleanScalarValue.makeNew(value1 < value2);
                case ExprLangParser.CONTAINS:   return BooleanScalarValue.makeNew(value1 != 0 && value2 != 0);
                    
                case ExprLangParser.RECT_FN:
                case ExprLangParser.UNIFORM_FN:
                case ExprLangParser.TRIANGULAR_FN:
                case ExprLangParser.ERLANG_FN:
                case ExprLangParser.TRUNCATED_EXP_FN:
                case ExprLangParser.PARETO_FN:
                    throw new EvaluationException("Cannot evaluate a General Distribution descriptor.");
                    
                default:
                    throw new UnsupportedOperationException("Unsupported: binaryFn = "+binaryFn);
            }
            return RealScalarValue.makeNew(result);
        }
        else if (computeType == Type.BOOLEAN) {
            final boolean value1 = getScalarBoolean(), value2 = op.getScalarBoolean(), result;
            switch (binaryFn) {
                case ExprLangParser.AND:        result = value1 && value2;           break;
                case ExprLangParser.OR:         result = value1 || value2;           break;
                case ExprLangParser.IMPLY:      result = !value1 || value2;          break;
                case ExprLangParser.BIIMPLY:    result = (value1 == value2);         break;
                case ExprLangParser.UNION:      result = value1 || value2;           break;
                case ExprLangParser.SETMINUS:   result = value1 && !value2;          break;
                case ExprLangParser.EQUAL:      result = value1 == value2;           break;
                case ExprLangParser.NOT_EQUAL:  result = value1 != value2;           break;
                case ExprLangParser.CONTAINS:   result = value1 && value2;           break;
                default:
                    throw new UnsupportedOperationException("Unsupported: binaryFn = "+binaryFn);
            }
            return BooleanScalarValue.makeNew(result);
        }
        else throw new UnsupportedOperationException();
    }
    
    public EvaluatedFormula evalBinaryFn(int binaryFn, EvaluatedFormula op) {
        // Neutral term of the binary operation
        Type computeType = getBinaryComputeType(op);
        Type resultType = getBinaryResultType(computeType, binaryFn);
        EvaluatedFormula neutralTermUnary = neutralTermForOperationFn(computeType, binaryFn, UNARY_TERM);
        EvaluatedFormula neutralTermLeft = neutralTermForOperationFn(computeType, binaryFn, LEFT_TERM);
        EvaluatedFormula neutralTermRight = neutralTermForOperationFn(computeType, binaryFn, RIGHT_TERM);
        
        if (isScalar() && op.isScalar()) {
            // Both operands are scalar.
            return evalBinaryFnScalar(binaryFn, op);
        }
        else if (isScalar() && op.isMultiSet()) {
            // First operand scalar, second operand multiset
            MultiSet msvOp = (MultiSet)op;
            Map<DomainElement, EvaluatedFormula> newSet = new TreeMap<>();
            for (int i=0; i<msvOp.numElements(); i++) {
                EvaluatedFormula newVal = evalBinaryFnScalar(binaryFn, msvOp.getValue(i));
                if (newVal.isValidMultiSetValue())
                    newSet.put(msvOp.getElement(i), newVal);
            }
            return aggregateResult(newSet, binaryFn, resultType, getDomain(), neutralTermUnary);
        }
        else if (isMultiSet() && op.isScalar()) {
            // First operand multiset, second operand scalar
            MultiSet msvThis = (MultiSet)this;
            Map<DomainElement, EvaluatedFormula> newSet = new TreeMap<>();
            for (int i=0; i<msvThis.numElements(); i++) {
                EvaluatedFormula newVal = msvThis.getValue(i).evalBinaryFnScalar(binaryFn, op);
                if (newVal.isValidMultiSetValue())
                    newSet.put(msvThis.getElement(i), newVal);
            }
            return aggregateResult(newSet, binaryFn, resultType, getDomain(), neutralTermUnary);
        }
        else if (isMultiSet() && op.isMultiSet()) {
            // Both operands are multisets
            if (neutralTermLeft == null || neutralTermRight == null) {
                throw new UnsupportedOperationException("Cannot compute this operation on two multisets.");
            }
            MultiSet msv1 = (MultiSet)this;
            MultiSet msv2 = (MultiSet)op;
            Map<DomainElement, EvaluatedFormula> newSet = new TreeMap<>();
            int i1 = 0, i2 = 0;
            
            while (i1 < msv1.numElements() && i2 < msv2.numElements()) {
                EvaluatedFormula newVal;
                DomainElement elem;
                int cmp = msv1.getElement(i1).compareTo(msv2.getElement(i2));
                if (cmp < 0) {
                    newVal = msv1.getValue(i1).evalBinaryFnScalar(binaryFn, neutralTermRight);
                    elem = msv1.getElement(i1);
                    i1++;
                }
                else if (cmp == 0) {
                    newVal = msv1.getValue(i1).evalBinaryFnScalar(binaryFn, msv2.getValue(i2));
                    elem = msv1.getElement(i1);
                    assert elem.equals(msv2.getElement(i2));
                    i1++;
                    i2++;
                }
                else { // cmp > 0
                    newVal = neutralTermLeft.evalBinaryFnScalar(binaryFn, msv2.getValue(i2));
                    elem = msv2.getElement(i2);
                    i2++;
                }
                if (newVal.isValidMultiSetValue())
                    newSet.put(elem, newVal);
            }
            
            while (i1 < msv1.numElements()) {
                EvaluatedFormula newVal = msv1.getValue(i1).evalBinaryFnScalar(binaryFn, neutralTermRight);
                DomainElement elem = msv1.getElement(i1);
                i1++;
                if (newVal.isValidMultiSetValue())
                    newSet.put(elem, newVal);
            }
            
            while (i2 < msv2.numElements()) {
                EvaluatedFormula newVal = neutralTermLeft.evalBinaryFnScalar(binaryFn, msv2.getValue(i2));
                DomainElement elem = msv2.getElement(i2);
                i2++;
                if (newVal.isValidMultiSetValue())
                    newSet.put(elem, newVal);
            }
            
            return aggregateResult(newSet, binaryFn, resultType, getDomain(), neutralTermUnary);
        }
        else throw new UnsupportedOperationException();
    }
    
    //--------------------------------------------------------------------------
    // Shortcut methods
    //--------------------------------------------------------------------------
    
    public EvaluatedFormula abs() {
        return evalUnaryFn(ExprLangParser.ABS_FN);
    }
    public EvaluatedFormula opposite() {
        return evalUnaryFn(ExprLangParser.SUB);
    }

    
    public EvaluatedFormula add(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.ADD, e);
    }
    public EvaluatedFormula subtract(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.SUB, e);
    }
    public EvaluatedFormula multiply(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.MUL, e);
    }
    public EvaluatedFormula divide(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.DIV, e);
    }
    public EvaluatedFormula safeDivide(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.SAFE_DIV, e);
    }
    public EvaluatedFormula min(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.MIN_FN, e);
    }
    public EvaluatedFormula max(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.MAX_FN, e);
    }
    
    public EvaluatedFormula multisetMin() {
        return evalUnaryFn(ExprLangParser.MULTISET_MIN);
    }
    public EvaluatedFormula multisetMax() {
        return evalUnaryFn(ExprLangParser.MULTISET_MAX);
    }
    public EvaluatedFormula multisetCard() {
        return evalUnaryFn(ExprLangParser.MULTISET_CARD);
    }
    public EvaluatedFormula multisetFilterNegatives() {
        return evalUnaryFn(ExprLangParser.MULTISET_FILTER_NEGATIVES);
    }
    
    public EvaluatedFormula equal(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.EQUAL, e);
    }
    public EvaluatedFormula notEqual(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.NOT_EQUAL, e);
    }
    public EvaluatedFormula less(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.LESS, e);
    }
    public EvaluatedFormula less_ZeroAsInf(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.LESS_ZEROINF, e);
    }
    public EvaluatedFormula lessEqual(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.LESS_EQ, e);
    }
    public EvaluatedFormula greater(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.GREATER, e);
    }
    public EvaluatedFormula greaterEqual(EvaluatedFormula e) {
        return evalBinaryFn(ExprLangParser.GREATER_EQ, e);
    }
    
    
    public static final Map<DomainElement, EvaluatedFormula> EMPTY_MAP = new HashMap<>();
    public EvaluatedFormula makeZero() {
        if (getDomain() == null || getDomain() == NeutralColorClass.INSTANCE) { // scalar
            switch (getType()) {
                case INT:       return IntScalarValue.ZERO;
                case REAL:      return RealScalarValue.ZERO;
                default:        return THR();
            }
        }
        else {
            return MultiSet.makeNew(getType(), getDomain(), EMPTY_MAP);
        }
    }

    //--------------------------------------------------------------------------
}
