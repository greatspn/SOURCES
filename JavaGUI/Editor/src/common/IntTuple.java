/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

/** An immutable pair of elements.
 *
 * @author elvio
 */
public class IntTuple { 
    public final int x; 
    public final int y; 
    
    public IntTuple(int x, int y) { 
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
        if (!(other instanceof IntTuple)){
            return false;
        }
        IntTuple other_ = (IntTuple)other;
        if (other_.x == this.x && other_.y == this.y)
            return true;
        return this.x == other_.x && this.y == other_.y;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + Integer.hashCode(x);
        result = prime * result + Integer.hashCode(y);
        return result;
    }
}
