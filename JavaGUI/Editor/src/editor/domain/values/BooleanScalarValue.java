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
public class BooleanScalarValue extends ScalarValue {
    public final boolean value;

    @Override
    public Type getType() {
        return Type.BOOLEAN;
    }

    private BooleanScalarValue(boolean value) {
        this.value = value;
    }
    
    @Override
    public boolean equals(Object obj) {
        if (!super.equals(obj))
            return false;
        return value == ((BooleanScalarValue)this).value;
    }

    @Override
    public boolean equalsZero() {
        return value == false;
    }

    @Override
    public int hashCode() {
        int hash = super.hashCode();
        hash = 2 * hash + (value ? 1 : 0);
        return hash;
    }
    
    //================== Object factory =====================
    
    public static final BooleanScalarValue TRUE = new BooleanScalarValue(true);
    public static final BooleanScalarValue FALSE = new BooleanScalarValue(false);
    
    public static BooleanScalarValue makeNew(boolean value) {
        if (value)
            return TRUE;
        return FALSE;
    }
}
