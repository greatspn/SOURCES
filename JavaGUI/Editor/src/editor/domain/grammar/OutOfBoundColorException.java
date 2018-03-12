/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

/**
 *
 * @author elvio
 */
public class OutOfBoundColorException extends EvaluationException {

    /**
     * Constructs an instance of <code>OutOfBoundColorException</code> with the
     * specified detail message.
     *
     * @param msg the detail message.
     */
    public OutOfBoundColorException(String msg) {
        super(msg);
    }
}
