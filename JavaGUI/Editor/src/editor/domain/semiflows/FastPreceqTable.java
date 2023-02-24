package editor.domain.semiflows;

import java.util.ArrayList;

/** Fast table to perform preceq_A comparisons used in the Buchberger algorithm
 *
 * @author elvio
 */
public class FastPreceqTable {
    final int M; // label vector length
    final Node root; // root node

    public FastPreceqTable(int M) {
        this.M = M;
        this.root = new LabelNode(0);
    }
    
    
    private abstract class Node {
        // Insert one row in the tree
        public abstract void insertRow(Row r); 
        // // There exists at least one row that is preceq @r ?
        public abstract Row preceq(Row r);
    }
    
    private class LabelNode extends Node {
        final int level;
        final ArrayList<Node> positives = new ArrayList<>();
        final ArrayList<Node> negatives = new ArrayList<>();
        Node zeroes;

        public LabelNode(int level) {
            this.level = level;
        }
        
        @Override
        public void insertRow(Row r) {
            int val = r.l[level];
            if (val==0) {
                if (zeroes==null)
                    zeroes = allocNode();
                zeroes.insertRow(r);
            }
            else if (val > 0) {
                while (val >= positives.size())
                    positives.add(null);
                if (positives.get(val) == null)
                    positives.set(val, allocNode());
                positives.get(val).insertRow(r);
            }
            else if (val < 0) {
                while (-val >= negatives.size())
                    negatives.add(null);
                if (negatives.get(-val) == null)
                    negatives.set(-val, allocNode());
                negatives.get(-val).insertRow(r);
            }
        }

        @Override
        public Row preceq(Row r) {
            Row hasPrecEq = null;
            // check zeroes (always)
            if (zeroes!=null)
                hasPrecEq = zeroes.preceq(r);
            // check positives
            if (null==hasPrecEq && r.l[level] > 0) {
                int bound = Math.min(r.l[level]+1, positives.size());
                for (int val=0; null==hasPrecEq && val<bound; val++)
                    if (positives.get(val) != null)
                        hasPrecEq = positives.get(val).preceq(r);
                    
            }
            // check negatives
            if (null==hasPrecEq && r.l[level] < 0) {
                int bound = Math.min(-r.l[level]+1, negatives.size());
                for (int val=0; null==hasPrecEq && val<bound; val++)
                    if (negatives.get(val) != null)
                        hasPrecEq = negatives.get(val).preceq(r);
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
            for (int val=negatives.size()-1; val>=0; val--) {
                if (negatives.get(val) != null) 
                    levelToString(sb, negatives.get(val), -val);
            }
            if (zeroes!=null) 
                levelToString(sb, zeroes, 0);
            for (int val=0; val <positives.size(); val++) {
                if (positives.get(val) != null) 
                    levelToString(sb, positives.get(val), val);
            }
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
        public Row preceq(Row r) {
            for (Row precRow : rows)
                if (precRow.less_equal_e(r))
                    return precRow;
            return null;
//            return rows.get(0);
        }
    }

    //-----------------------------------------------------------------------
    
    public void insertRow(Row row) {
        root.insertRow(row);
    }
    
    public Row preceq(Row r) {
        return root.preceq(r);
    }

    @Override
    public String toString() {
        return root.toString();
    }
    
    //-----------------------------------------------------------------------
    
    public static void main(String[] args) {
        FastPreceqTable le = new FastPreceqTable(3);
        
        int[][] labels = {
            {0,1,-2},
            {1,1,-2},
            {0,3,0}
        };
        int[][] tested = {
            {0,1,-1},
            {1,1,-3},
            {0,3,0},
            {0,2,0},
            {0,8,0},
        };
        
        for (int[] lab : labels) {        
            Row row = new Row(2, lab.length, false);
            for (int i=0; i<lab.length; i++)
                row.l[i] = lab[i];
            le.insertRow(row);
            System.out.println(le);
        }
        
        for (int[] lab : tested) {
            Row row = new Row(2, lab.length, false);
            for (int i=0; i<lab.length; i++)
                row.l[i] = lab[i];
            System.out.println(row + " has preceq: " + le.preceq(row));
        }
    }
}
