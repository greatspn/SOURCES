/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

/**
 *
 * @author elvio
 */
public class EvaluationSymbol {
    
    private final String symbolName;

    public EvaluationSymbol(String symbolName) {
        this.symbolName = symbolName;
    }
    
    public static EvaluationSymbol INFINITY = new EvaluationSymbol("Infinite");
}
