/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

/** An immutable pair of elements.
 *
 * @author elvio
 */
public class MutablePair<X, Y> { 
    public X first; 
    public Y second; 
    public MutablePair(X first, Y second) { 
        this.first = first; 
        this.second = second; 
    }
    public MutablePair() { 
        this.first = null; 
        this.second = null; 
    }

    @Override
    public String toString() {
        return "(" + first + "," + second + ")";
    }
    
    public void setPair(X first, Y second) { 
        this.first = first; 
        this.second = second; 
    }

    @Override
    public boolean equals(Object other) {
        if (other == null) {
            return false;
        }
        if (other == this) {
            return true;
        }
        if (!(other instanceof MutablePair)){
            return false;
        }
        MutablePair other_ = (MutablePair)other;
        if (other_.first == this.first && other_.second == this.second)
            return true;
        return this.first.equals(other_.first) && this.second.equals(other_.second);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((first == null) ? 0 : first.hashCode());
        result = prime * result + ((second == null) ? 0 : second.hashCode());
        return result;
    }
}
