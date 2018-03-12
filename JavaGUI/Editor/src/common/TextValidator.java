/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

/**
 *
 * @author Elvio
 */
public interface TextValidator {
    // Ritorna null se è valido, una stringa con la motivazione se non lo è.
    public String isValidText(String text);
}
