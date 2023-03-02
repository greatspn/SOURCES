/*
 * Matrix row with [e|l] vectors
 */
package editor.domain.semiflows;

import java.util.Comparator;

/** Row of a horizontally stacked matrix [ E | L ], made
 * by two vectors e and l.
 *
 * @author elvio
 */
public class Row {
    // new empty row
    public Row(int N, int M, boolean initial) {
        e = new int[N];
        l = new int[M];
        this.initial = initial;
    }
    
    // sum of two rows
    public Row(Row r1, Row r2) {
        this(r1.e.length, r1.l.length, false);
        assert r1.e.length == r2.e.length && r1.l.length == r2.l.length;
        add(r1);
        add(r2);
    }

    // sum of two multiplied rows
    public Row(int mult1, Row r1, int mult2, Row r2) {
        this(r1.e.length, r1.l.length, false);
        assert r1.e.length == r2.e.length && r1.l.length == r2.l.length;
        addMult(r1, mult1);
        addMult(r2, mult2);
    }

    int[] e; // elements
    int[] l; // labels
    boolean initial; // part of the initial setup

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int j = 0; j < e.length; j++) {
            sb.append(e[j] < 0 ? "" : " ").append(e[j]).append(" ");
        }
        if (l != null) {
            sb.append("| ");
            for (int j = 0; j < l.length; j++) {
                sb.append(l[j] < 0 ? "" : " ").append(l[j]).append(" ");
            }
        }
        sb.append("]");
        if (is_l_zero())
            sb.append(" *");
        if (initial)
            sb.append(" init");
        return sb.toString();
    }

    public final void add(Row r) {
        assert e.length==r.e.length && l.length==r.l.length;
        for (int j=0; j<e.length; j++)
            e[j] = Math.addExact(e[j], r.e[j]);
//            e[j] += r.e[j];
        for (int j=0; j<l.length; j++)
            l[j] = Math.addExact(l[j], r.l[j]);
//            l[j] += r.l[j];
    }

    public final void addMult(Row r, int mult) {
        assert e.length==r.e.length && l.length==r.l.length;
        for (int j=0; j<e.length; j++)
            e[j] = Math.addExact(e[j], Math.multiplyExact(mult, r.e[j]));
//            e[j] += mult * r.e[j];
        for (int j=0; j<l.length; j++)
            l[j] = Math.addExact(l[j], Math.multiplyExact(mult, r.l[j]));
//            l[j] += mult * r.l[j];            
    }

    public boolean equal(Row r) {
        assert e.length==r.e.length && l.length==r.l.length;
        for (int j=0; j<e.length; j++)
            if (e[j] != r.e[j])
                return false;
        for (int j=0; j<l.length; j++)
            if (l[j] != r.l[j])
                return false;
        return true;
    }
    
    public boolean less_equal_e(Row r) {
        for (int j=0; j<e.length; j++)
            if (e[j] > r.e[j])
                return false;
        return true;
    }
    
    public int degree() {
        int deg = 0;
        for (int val : e)
            deg += val;
        return deg;
    }
    
    public boolean is_e_zero() {
        for (int j=0; j<e.length; j++)
            if (e[j] != 0)
                return false;
        return true;
    }
    
    public boolean is_l_zero() {
        for (int j=0; j<l.length; j++)
            if (l[j] != 0)
                return false;
        return true;
    }
    
    public static int scalarGCD(int a, int b) {
        assert a >= 0 && b >= 0;
        if (a == 0)
            return b;

        while (b != 0) {
            if (a > b)
                a = a - b;
            else
                b = b - a;
        }
        return a;
    }
    
    public int vectorGCD() {
        int gcdVal = Math.abs(e[0]);
        for (int i=1; i<e.length; i++)
            gcdVal = scalarGCD(gcdVal, Math.abs(e[i]));
        for (int i=0; i<l.length; i++)
            gcdVal = scalarGCD(gcdVal, Math.abs(l[i]));
        return gcdVal;
    }
    
    // divide by the common [e|l] gcd
    public void canonicalize() {
        int div = vectorGCD();
        // ensure that the first non-zero in e is positive
        for (int i=0; i<e.length; i++) {
            if (e[i] != 0) {
                if (e[i] * div < 0)
                    div = -div;
                break;
            }
        }
        if (div != 1) {
            for (int i=0; i<e.length; i++)
                e[i] /= div;
            for (int i=0; i<l.length; i++)
                l[i] /= div;
        }
    }
    
    // truncate the e[] vector up to a new size
    public void truncate_e(int newSz) {
        assert newSz < e.length;
        int[] ne = new int[newSz];
        System.arraycopy(e, 0, ne, 0, newSz);
        e = ne;
    }
    
    //-----------------------------------------------------------------------

    // lexicographically compare the e vector
    private static class LexComparator implements Comparator<Row> {
        @Override
        public int compare(Row row1, Row row2) {
            for (int j=0; j<row1.e.length; j++) {
                if (row1.e[j] < row2.e[j])
                    return -1;
                if (row1.e[j] > row2.e[j])
                    return +1;
            }
            return 0;
        }
    }
    public static final Comparator<Row> LEX_COMPARATOR = new LexComparator();

    //-----------------------------------------------------------------------
    
    private static class DegLexComparator implements Comparator<Row> {
        @Override
        public int compare(Row row1, Row row2) {
//            int c = compare2(row1, row2);
//            System.out.println(row1+ " vs "+row2+" = "+c);
//            return c;
//        }
//        public int compare2(Row row1, Row row2){
            int deg1 = row1.degree();
            int deg2 = row2.degree();
            if (deg1 < deg2)
                return -1;
            if (deg1 > deg2)
                return +1;

            for (int j=0; j<row1.e.length; j++) {
//            for (int j=row1.e.length-1; j>=0; j--) {
                if (row1.e[j] < row2.e[j])
                    return -1;
                if (row1.e[j] > row2.e[j])
                    return +1;
            }
            return 0;
        }
    }
    public static final Comparator<Row> DEGLEX_COMPARATOR = new DegLexComparator();
}
