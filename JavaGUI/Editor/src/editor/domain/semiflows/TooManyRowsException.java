/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Exception.java to edit this template
 */
package editor.domain.semiflows;

/**
 *
 * @author elvio
 */
public class TooManyRowsException extends Exception {

    /**
     * Creates a new instance of <code>TooManyRowsException</code> without
     * detail message.
     */
    public TooManyRowsException() {
    }

    /**
     * Constructs an instance of <code>TooManyRowsException</code> with the
     * specified detail message.
     *
     * @param msg the detail message.
     */
    public TooManyRowsException(String msg) {
        super(msg);
    }
}
