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
public class PartialSum {
    private final int sumOfInts;
    private final String restOfSum;

    public static final PartialSum ZERO = new PartialSum(0, "");

    public PartialSum(int sum, String ssum) { 
        this.sumOfInts = sum;
        this.restOfSum = ssum;
    }

    public PartialSum add(int num, String s) {
        if (s.isBlank()) {
            if (num == 0)
                return this;
            else
                return new PartialSum(sumOfInts + num, restOfSum);
        }
        try {
            int n = Integer.parseInt(s);
            return new PartialSum(sumOfInts + num + n, restOfSum);
        }
        catch (NumberFormatException e) {
            if (restOfSum.isEmpty())
                return new PartialSum(sumOfInts + num, s);
            else
                return new PartialSum(sumOfInts + num, restOfSum + s);
        }
    }

    public PartialSum add(PartialSum ps) {
        return add(ps.sumOfInts, ps.restOfSum);
    }

    public PartialSum subtract(int num, String s) {
        if (restOfSum.equals(s))
            return new PartialSum(sumOfInts - num, "");
        else if (s.isBlank())
            return new PartialSum(sumOfInts - num, restOfSum);
        else 
            return new PartialSum(sumOfInts - num, restOfSum+"-("+s+")");
    }

    public PartialSum subtract(PartialSum ps) {
        return subtract(ps.sumOfInts, ps.restOfSum);
    }

    public boolean isEmpty() {
        return (sumOfInts==0 && restOfSum.isEmpty());
    }

    public boolean isZero() {
        return (sumOfInts==0 && restOfSum.isBlank());
    }

    public boolean isRestBlank() { return restOfSum.isBlank(); }

    public int getIntPart() { return sumOfInts; }

    public String toStringExtended(String sign, String termForZero) {
        if (restOfSum.isBlank()) {
            if (sumOfInts == 0)
                return termForZero;
            return sign+sumOfInts;
        }
        else if (sumOfInts == 0) {
            if (sign.isBlank())
                return sign+"("+restOfSum+")";
            else
                return restOfSum;
        }
        else {
            if (sign.isBlank())
                return sumOfInts +"+"+restOfSum;
            else
                return sign+"("+sumOfInts +"+"+restOfSum+")";
        }
    }

    @Override
    public String toString() {
        return toStringExtended("", "0");
    }

    public String toStringEmptyZero(String sign) {
        return toStringExtended(sign, "");
    }
}
