package editor.domain.semiflows;

import java.util.ArrayList;

/** Fast table to perform preceq_A comparisons used in the Buchberger algorithm
 *
 * @author elvio
 */
public class FastSupportTestTable {
    final int M; // label vector length
    final Node root; // root node

    public FastSupportTestTable(int M) {
        this.M = M;
        this.root = new LabelNode(0);
    }
    
    
    private abstract class Node {
        // Insert one row in the tree
        public abstract void insertRow(Row r); 
        // // There exists at least one row that has a smaller support?
        public abstract Row smallerSupport(Row r);
    }
    
    private class LabelNode extends Node {
        final int level;
        Node zeroes, nonzeroes;

        public LabelNode(int level) {
            this.level = level;
        }
        
        @Override
        public void insertRow(Row r) {
            int val = r.e[level];
            if (val==0) {
                if (zeroes==null)
                    zeroes = allocNode();
                zeroes.insertRow(r);
            }
            else {
                if (nonzeroes==null)
                    nonzeroes = allocNode();
                nonzeroes.insertRow(r);
            }
        }

        @Override
        public Row smallerSupport(Row r) {
            Row hasPrecEq = null;
            // check zeroes (always)
            if (zeroes!=null)
                hasPrecEq = zeroes.smallerSupport(r);
            
            // check non-zeroes if r[level] is non-zero, too
            if (hasPrecEq==null && r.e[level] != 0) {
                if (nonzeroes!=null)
                    hasPrecEq = nonzeroes.smallerSupport(r);
            }
            return hasPrecEq;
        }
        
        private Node allocNode() {
            if (level + 1 == M) 
                return new ElemNode();
            return new LabelNode(level + 1);
        }

        @Override
        public String toString() {
            StringBuffer sb = new StringBuffer();
            if (zeroes!=null) 
                levelToString(sb, zeroes, 0);
            if (nonzeroes!=null) 
                levelToString(sb, nonzeroes, 1);
            return sb.toString();
        }
        
        private void levelToString(StringBuffer sb, Node node, int val) {
            for (int i=0; i<level; i++) sb.append("  ");
            sb.append("LabelNode(level=").append(level).append(" is ").append(val).append("):\n");
            sb.append(node.toString());            
        }
    }
    
    private class ElemNode extends Node {
        ArrayList<Row> rows = new ArrayList<>();
        
        @Override
        public void insertRow(Row r) {
            rows.add(r);
        }
        
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            for (int i=0; i<M; i++) sb.append("  ");
            sb.append("ElemNode:\n");
            for (Row r : rows) {
                for (int i=0; i<M; i++) sb.append("  ");
                sb.append(r).append("\n");                
            }
            return sb.toString();
        }

        @Override
        public Row smallerSupport(Row r) {
            // return a row that is not r
            for (Row r2 : rows)
                if (r2 != r) // not the same object
                    return r2;
            return null;
        }
    }

    //-----------------------------------------------------------------------
    
    public void insertRow(Row row) {
        root.insertRow(row);
    }
    
    public Row smallerSupport(Row r) {
        return root.smallerSupport(r);
    }

    @Override
    public String toString() {
        return root.toString();
    }
    
    //-----------------------------------------------------------------------
    
    public static void main(String[] args) {
        FastSupportTestTable fstt = new FastSupportTestTable(5);
        
        int[][] labels = {
            {0,1,0,1,0},
            {1,1,0,0,0},
            {0,0,1,1,1}
        };
        int[][] tested = {
            {1,0,1,0,1},
            {1,1,0,0,0},
            {1,1,0,0,1},
            {1,1,1,1,1},
            {0,0,0,0,0},
            {0,0,1,1,0},
        };
        
        for (int[] lab : labels) {        
            Row row = new Row(2, lab.length, false);
            for (int i=0; i<lab.length; i++)
                row.l[i] = lab[i];
            fstt.insertRow(row);
            System.out.println(fstt);
        }
        
        for (int[] lab : tested) {
            Row row = new Row(2, lab.length, false);
            for (int i=0; i<lab.length; i++)
                row.l[i] = lab[i];
            System.out.println(row + " has smaller support? " + fstt.smallerSupport(row));
        }
    }
}
