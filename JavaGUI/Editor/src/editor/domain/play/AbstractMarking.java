/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.play;

import editor.domain.elements.ClockVar;
import editor.domain.elements.Place;
import editor.domain.values.EvaluatedFormula;

/** Base class of a marking object, that contains the state of a GSPN.
 *
 * @author elvio
 */
public interface AbstractMarking {
    
    public EvaluatedFormula getMarkingOfPlace(Place place);
    
    public double getValueOfClockVariable(ClockVar clockVar);
}
