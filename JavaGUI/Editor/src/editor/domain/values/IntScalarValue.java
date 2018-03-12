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
public class IntScalarValue extends ScalarValue {
    public final int value;

    @Override
    public Type getType() {
        return Type.INT;
    }

    private IntScalarValue(int value) {
        this.value = value;
    }
    
    @Override
    public boolean equals(Object obj) {
        if (!super.equals(obj))
            return false;
        return value == ((IntScalarValue)this).value;
    }

    @Override
    public boolean equalsZero() {
        return (value == 0);
    }
    
    @Override
    public int hashCode() {
        int hash = super.hashCode();
        hash = 11 * hash + Integer.valueOf(value).hashCode();
        return hash;
    }
    
    //================== Object factory =====================
    
    private static final IntScalarValue[] precompIntScalars = new IntScalarValue[10];
    public static final IntScalarValue ZERO;
    public static final IntScalarValue ONE;
    public static final IntScalarValue MINIMUM = new IntScalarValue(Integer.MIN_VALUE);
    public static final IntScalarValue MAXIMUM = new IntScalarValue(Integer.MAX_VALUE);
    static {
        for (int i=0; i<precompIntScalars.length; i++)
            precompIntScalars[i] = new IntScalarValue(i);
        ZERO = precompIntScalars[0];
        ONE = precompIntScalars[1];
    }
    
    public static IntScalarValue makeNew(int value) {
        if (value >= 0 && value < precompIntScalars.length)
            return precompIntScalars[value];
        return new IntScalarValue(value);
    }
}
