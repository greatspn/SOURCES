/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.unfolding;

/**
 *
 * @author elvio
 */
public class CouldNotUnfoldException extends Exception {

    /**
     * Creates a new instance of <code>CouldNotUnfoldException</code> without
     * detail message.
     */
    public CouldNotUnfoldException() {
    }

    /**
     * Constructs an instance of <code>CouldNotUnfoldException</code> with the
     * specified detail message.
     *
     * @param msg the detail message.
     */
    public CouldNotUnfoldException(String msg) {
        super(msg);
    }
}
