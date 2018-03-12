/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

/**
 *
 * @author elvio
 */
public class Condition {
    
    private boolean satisfied = true;

    public Condition() {
    }

    public boolean isSatisfied() {
        return satisfied;
    }

    public void setSatisfied(boolean satisfied) {
        this.satisfied = satisfied;
    }
    
}
