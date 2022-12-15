/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package editor.domain.semiflows;

/** Row of a horizontally stacked matrix [ D | C ], made
 * by two vectors d and c.
 *
 * @author elvio
 */
public class Row {
    public Row(int N, int M) {
        d = new int[N];
        c = new int[M];
    }
        
    int[] d, c;

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int j = 0; j < d.length; j++) {
            sb.append(d[j] < 0 ? "" : " ").append(d[j]).append(" ");
        }
        if (c != null) {
            sb.append("| ");
            for (int j = 0; j < c.length; j++) {
                sb.append(c[j] < 0 ? "" : " ").append(c[j]).append(" ");
            }
        }
        sb.append("]");
        return sb.toString();
    }

    public void add(Row r) {
        assert d.length==r.d.length && c.length==r.c.length;
        for (int j=0; j<d.length; j++)
            d[j] += r.d[j];
        for (int j=0; j<c.length; j++)
            c[j] += r.c[j];
    }

    public void addMult(Row r, int mult) {
        assert d.length==r.d.length && c.length==r.c.length;
        for (int j=0; j<d.length; j++)
            d[j] += mult * r.d[j];
        for (int j=0; j<c.length; j++)
            c[j] += mult * r.c[j];            
    }

    public boolean equal(Row r) {
        assert d.length==r.d.length && c.length==r.c.length;
        for (int j=0; j<d.length; j++)
            if (d[j] != r.d[j])
                return false;
        for (int j=0; j<c.length; j++)
            if (c[j] != r.c[j])
                return false;
        return true;
    }
    
    public boolean is_d_zero() {
        for (int j=0; j<d.length; j++)
            if (d[j] != 0)
                return false;
        return true;
    }
    
    public boolean is_c_zero() {
        for (int j=0; j<c.length; j++)
            if (c[j] != 0)
                return false;
        return true;
    }
}
