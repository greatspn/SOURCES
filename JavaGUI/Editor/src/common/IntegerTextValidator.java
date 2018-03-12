/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

/**
 *
 * @author Elvio
 */
public class IntegerTextValidator implements TextValidator {

    private int minimum = Integer.MIN_VALUE;
    private int maximum = Integer.MAX_VALUE;
    private String reason;

    public IntegerTextValidator() {
        makeReason();
    }

    public IntegerTextValidator(int minimum, int maximum) {
        this.minimum = minimum;
        this.maximum = maximum;
        makeReason();
    }
    
    private void makeReason() {
        if (minimum == Integer.MIN_VALUE) {
            if (maximum == Integer.MAX_VALUE) 
                reason = "Richiesto un numero intero.";
            else
                reason = "Richiesto un numero intero minore di "+maximum+".";
        }
        else {
            if (maximum == Integer.MAX_VALUE)
                reason = "Richiesto un numero intero maggiore di "+minimum+".";
            else 
                reason = "Richiesto un numero intero compreso tra "+minimum+" e "+maximum+".";
        }
    }

    public int getMinimum() {
        return minimum;
    }

    public void setMinimum(int minimum) {
        this.minimum = minimum;
        makeReason();
    }

    public int getMaximum() {
        return maximum;
    }

    public void setMaximum(int maximum) {
        this.maximum = maximum;
        makeReason();
    }
    
    @Override
    public String isValidText(String text) {
        try {
            int num = Integer.parseInt(text);
            if (num < minimum || num > maximum)
                return reason;
        }
        catch (NumberFormatException nfe) {
            return "Richiesto un numero intero.";
        }
        return null;
    }
    
}
