/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

/**
 *
 * @author elvio
 */
public class RealScalarValue extends ScalarValue {
    public final double value;

    @Override
    public Type getType() {
        return Type.REAL;
    }

    private RealScalarValue(double value) {
        this.value = value;
    }
    
    @Override
    public boolean equals(Object obj) {
        if (!super.equals(obj))
            return false;
        return value == ((RealScalarValue)this).value;
    }

    @Override
    public boolean equalsZero() {
        return (value == 0.0);
    }
    
    @Override
    public int hashCode() {
        int hash = super.hashCode();
        hash = 11 * hash + Double.valueOf(value).hashCode();
        return hash;
    }
    
    //================== Object factory =====================
    
    public static final RealScalarValue ZERO = new RealScalarValue(0.0);
    public static final RealScalarValue ONE = new RealScalarValue(1.0);
    public static final RealScalarValue MINIMUM = new RealScalarValue(Double.MIN_VALUE);
    public static final RealScalarValue MAXIMUM = new RealScalarValue(Double.MAX_VALUE);
    
    public static RealScalarValue makeNew(double value) {
        if (value == 0.0)
            return ZERO;
        else if (value == 1.0)
            return ONE;
        else return new RealScalarValue(value);
    }
}
