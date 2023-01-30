/*
 * Matrix row with [d|c] vectors
 */
package editor.domain.semiflows;

/** Row of a horizontally stacked matrix [ D | C ], made
 * by two vectors d and c.
 *
 * @author elvio
 */
public class Row {
    public Row(int N, int M) {
        e = new int[N];
        l = new int[M];
    }
        
    int[] e; // elements
    int[] l; // labels

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
        return sb.toString();
    }

    public void add(Row r) {
        assert e.length==r.e.length && l.length==r.l.length;
        for (int j=0; j<e.length; j++)
            e[j] += r.e[j];
        for (int j=0; j<l.length; j++)
            l[j] += r.l[j];
    }

    public void addMult(Row r, int mult) {
        assert e.length==r.e.length && l.length==r.l.length;
        for (int j=0; j<e.length; j++)
            e[j] += mult * r.e[j];
        for (int j=0; j<l.length; j++)
            l[j] += mult * r.l[j];            
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
}
