/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

/** A triple of elements.
 *
 * @author elvio
 */
public class Triple<X, Y, Z> {
    public final X x; 
    public final Y y; 
    public final Z z; 

    public Triple(X x, Y y, Z z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    

    @Override
    public String toString() {
        return "(" + x + "," + y + "," + z + ")";
    }

    @Override
    public boolean equals(Object other) {
        if (other == null) {
            return false;
        }
        if (other == this) {
            return true;
        }
        if (!(other instanceof Triple)){
            return false;
        }
        Triple other_ = (Triple) other;
        if (other_.x == this.x && other_.y == this.y && other_.z == this.z)
            return true;
        return this.x.equals(other_.x) && this.y.equals(other_.y) && this.z.equals(other_.z);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((x == null) ? 0 : x.hashCode());
        result = prime * result + ((y == null) ? 0 : y.hashCode());
        result = prime * result + ((z == null) ? 0 : z.hashCode());
        return result;
    }    
}
