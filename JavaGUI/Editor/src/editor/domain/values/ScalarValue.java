/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

import editor.domain.elements.ColorClass;
import editor.domain.elements.NeutralColorClass;

// Base class of scalar values

public abstract class ScalarValue extends EvaluatedFormula {

    @Override
    public ColorClass getDomain() {
        return NeutralColorClass.INSTANCE;
    }

    @Override
    public boolean isScalar() {
        return true;
    }

    @Override
    public boolean isMultiSet() {
        return false;
    }

    @Override
    public boolean isObject() {
        return false;
    }
    
}
