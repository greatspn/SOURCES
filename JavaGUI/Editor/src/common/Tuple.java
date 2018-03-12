/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

/** An immutable pair of elements.
 *
 * @author elvio
 */
public class Tuple<X, Y> { 
    public final X x; 
    public final Y y; 
    
    public Tuple(X x, Y y) { 
        this.x = x; 
        this.y = y; 
    }

    @Override
    public String toString() {
        return "(" + x + "," + y + ")";
    }

    @Override
    public boolean equals(Object other) {
        if (other == null) {
            return false;
        }
        if (other == this) {
            return true;
        }
        if (!(other instanceof Tuple)){
            return false;
        }
        Tuple other_ = (Tuple)other;
        if (other_.x == this.x && other_.y == this.y)
            return true;
        return this.x.equals(other_.x) && this.y.equals(other_.y);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((x == null) ? 0 : x.hashCode());
        result = prime * result + ((y == null) ? 0 : y.hashCode());
        return result;
    }
}
