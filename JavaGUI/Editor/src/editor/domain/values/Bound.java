/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.values;

/** A single time bound in a list of bounds
 *
 * @author elvio
 */
public class Bound {
    public double a;
    public double b;

    public Bound(double a, double b) {
        this.a = a;
        this.b = b;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 11 * hash + (int) (Double.doubleToLongBits(this.a) ^ (Double.doubleToLongBits(this.a) >>> 32));
        hash = 11 * hash + (int) (Double.doubleToLongBits(this.b) ^ (Double.doubleToLongBits(this.b) >>> 32));
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        final Bound other = (Bound) obj;
        if (Double.doubleToLongBits(this.a) != Double.doubleToLongBits(other.a)) {
            return false;
        }
        if (Double.doubleToLongBits(this.b) != Double.doubleToLongBits(other.b)) {
            return false;
        }
        return true;
    }

    public boolean isImpulse() {
        return a == b;
    }
    
    public boolean overlaps(double a1, double b1) {
        return (a < b1 && a < a1);
    }
    
    public boolean overlaps(Bound bnd) {
        return (a < bnd.b && a < bnd.a);
    }
    
    public void enlarge(double a1, double b1) {
        a = Math.min(a, a1);
        b = Math.max(b, b1);
    }
    
}
