/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

import editor.domain.ListRenderable;
import editor.domain.values.EvaluatedFormula;
import java.io.Serializable;
import javax.swing.Icon;

/** The type of the tokens contained in a place and moved by the arcs in a GSPN.
 *
 * @author elvio
 */
public enum TokenType implements Serializable, ListRenderable {

    DISCRETE("Discrete", EvaluatedFormula.Type.INT),
    CONTINUOUS("Continuous", EvaluatedFormula.Type.REAL);

    private final String descr;
    private final EvaluatedFormula.Type formulaType;

    private TokenType(String descr, EvaluatedFormula.Type formulaType) {
        this.descr = descr;
        this.formulaType = formulaType;
    }
    
    @Override
    public String getDescription() {
        return descr;
    }

    @Override
    public Icon getIcon16() {
        return null;
    }
    
    @Override public int getTextSize() { return -1; /* default*/ }

    public EvaluatedFormula.Type getFormulaType() {
        return formulaType;
    }
}
