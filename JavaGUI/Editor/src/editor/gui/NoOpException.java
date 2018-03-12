/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

/**
 *
 * @author Elvio
 */
public class NoOpException extends RuntimeException {

    /**
     * Creates a new instance of
     * <code>NoOpException</code> without detail message.
     */
    public NoOpException() {
    }

    /**
     * Constructs an instance of
     * <code>NoOpException</code> with the specified detail message.
     *
     * @param msg the detail message.
     */
    public NoOpException(String msg) {
        super(msg);
    }
}
