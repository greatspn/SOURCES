/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor;

import common.Tuple;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.io.GreatSpnFormat;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;
import latex.DummyLatexProvider;

/**
 *
 * @author elvio
 */
public class TexMatrixTest {
    public static ArrayList<Tuple<int[], int[]>> loadPinFile(File pinFile) throws FileNotFoundException {
        ArrayList<Tuple<int[], int[]>> flows = new ArrayList<>();
        Scanner sc = new Scanner(pinFile);
        
        int numFlows = sc.nextInt();
        for (int f=0; f<numFlows; f++) {
            int[] cards = new int[sc.nextInt()];
            int[] pinds = new int[cards.length];
            for (int n=0; n<cards.length; n++) {
                cards[n] = sc.nextInt();
                pinds[n] = sc.nextInt();
            }
            flows.add(new Tuple<>(cards, pinds));
        }
        
        sc.close();
        return flows;
    }
    
    private static class PartialSum {
        private final int sumOfInts;
        private final String restOfSum;
        
        private static final PartialSum ZERO = new PartialSum(0, "");
        
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
    
    public static PartialSum[][] extractMatrix(GspnPage gspn, GspnEdge.Kind kind,
                                               Map<Place, Integer> pl2pos,
                                               Map<Transition, Integer> tr2pos) 
    {
        PartialSum[][] matrix = new PartialSum[pl2pos.size()][tr2pos.size()];
        for (int pl=0; pl<pl2pos.size(); pl++)
            for (int tr=0; tr<tr2pos.size(); tr++)
                matrix[pl][tr] = PartialSum.ZERO;
        
        for (Edge ee : gspn.edges) {
            if (ee instanceof GspnEdge) {
                GspnEdge ge = (GspnEdge)ee;
                if (ge.getEdgeKind() == kind) {
                    int pl = pl2pos.get(ge.getConnectedPlace());
                    int tr = tr2pos.get(ge.getConnectedTransition());
                    matrix[pl][tr] = matrix[pl][tr].add(0, ge.getMultiplicity());
                    System.out.println("matrix[pl][tr]="+matrix[pl][tr]);
                }
            }
        }
        return matrix;
    }
    
    
    public static void testInc(String baseName) throws Exception {        
        GspnPage gspn = new GspnPage();
        File inputNet = new File(baseName+".net");
        File inputDef = new File(baseName+".def");
        GreatSpnFormat.importGspn(gspn, inputNet, inputDef);
        
        ArrayList<Place> places = new ArrayList<>();
        ArrayList<Transition> trans = new ArrayList<>();
        Map<Place, Integer> pl2pos = new HashMap<>();
        Map<Transition, Integer> tr2pos = new HashMap<>();
        for (Node n : gspn.nodes) {
            if (n instanceof Place) {
                pl2pos.put((Place)n, places.size());
                places.add((Place)n);
            }
            else if (n instanceof Transition) {
                tr2pos.put((Transition)n, trans.size());
                trans.add((Transition)n);
            }
        }
        StringBuffer sb = new StringBuffer();
        
        PartialSum[][] matI = extractMatrix(gspn, GspnEdge.Kind.INPUT, pl2pos, tr2pos);
        PartialSum[][] matO = extractMatrix(gspn, GspnEdge.Kind.OUTPUT, pl2pos, tr2pos);
        
        // header
        sb.append("$");
//        sb.append("\\mathbf{I}-\\mathbf{O} = ");
        sb.append("\\begin{array}{r|");
        for (int f=0; f<trans.size(); f++)
            sb.append("c|");
        sb.append("r}\n& ");
        for (int f=0; f<trans.size(); f++)
            sb.append("\\rotatebox{90}{"+trans.get(f).getUniqueNameDecor().getVisualizedValue()+"}& ");
        sb.append("\\mathbf{m}_0\\\\ \n\\hline\n");

        boolean singleSum = false;
        boolean writeZeros = true;
        boolean stacked = true;
//        PartialSum inSum = new PartialSum();
//        PartialSum outSum = new PartialSum();
        final GspnEdge.Kind[] kinds = new GspnEdge.Kind[]{ GspnEdge.Kind.OUTPUT, GspnEdge.Kind.INPUT };
        for (int pl=0; pl<places.size(); pl++) {
            sb.append(places.get(pl).getUniqueNameDecor().getLatexFormula().getLatex()+" & ");

            for (int tr=0; tr<trans.size(); tr++) {
                PartialSum inVal = matI[pl][tr];
                PartialSum outVal = matO[pl][tr];
                if (singleSum) {
                    PartialSum inOutSum = inVal.subtract(outVal);
                    System.out.println("inOutSum="+inOutSum+" inVal="+inVal+" outVal="+outVal);
                    String color;
                    if (inOutSum.isZero())
                        color = "Gray";
                    else {
                        if (!inOutSum.isRestBlank())
                            color = "ForestGreen";
                        else if (inOutSum.getIntPart() > 0)
                            color = "Blue";
                        else
                            color = "Red";
                    }
                    sb.append("\\textcolor{").append(color).append("}{")
                            .append(inOutSum.toStringExtended("", writeZeros ? "0" : "")).append("}");
                }
                else {
                    String color;
                    if (inVal.isEmpty() && outVal.isEmpty()) {
                        if (writeZeros)
                            sb.append("\\textcolor{Gray}{0}");
                    }
                    else {
                        String sIn = "\\textcolor{Blue}{"+inVal.toStringEmptyZero("")+"}";
                        String sOut = "\\textcolor{Mahogany}{"+outVal.toStringEmptyZero("-")+"}";
                        if (stacked) {
                            sb.append("\\genfrac{}{}{0pt}{0}{").append(sIn).append("}{").append(sOut).append("}");
                        }
                        else {
                            sb.append(sIn).append(sOut);
                        }
                    }
                }
                sb.append(" & ");
            }
            
            String m0p = places.get(pl).getInitMarkingExpr();
            if (m0p.isBlank())
                m0p = "0";
            sb.append("\\mathbf{"+m0p+"}\\\\ \n\\hline\n");
        }
        
        sb.append("\\end{array}$");
        
        latex.TestJLaTeXMath.writeLatexFormulaAsImage(sb.toString(), baseName+"-inc.png", 36, false);
        System.out.println("Ok.");
    }
    
    public static void main(String[] args) throws Exception {
        DummyLatexProvider.initializeProvider();
        testInc("/home/elvio/nets/Eratosthenes-PT-010");
        testInc("/home/elvio/nets/ATPN2020");
        testInc("/home/elvio/nets/2PhaseLockViolation");
        System.exit(0);
        String baseName = "/home/elvio/nets/2PhaseLockViolation";
        
        GspnPage gspn = new GspnPage();
        File inputNet = new File(baseName+".net");
        File inputDef = new File(baseName+".def");
        GreatSpnFormat.importGspn(gspn, inputNet, inputDef);
        
        ArrayList<Place> places = new ArrayList<>();
        ArrayList<Transition> trans = new ArrayList<>();
        for (Node n : gspn.nodes) {
            if (n instanceof Place)
                places.add((Place)n);
            else if (n instanceof Transition)
                trans.add((Transition)n);
        }
        
        File inputPin = new File(baseName+".pin");
        ArrayList<Tuple<int[], int[]>> pinMat = loadPinFile(inputPin);
        int[] sum_pm0 = new int[pinMat.size()];
        String[] extra_pm0 = new String[pinMat.size()];
        for (int i=0; i<extra_pm0.length; i++)
            extra_pm0[i] = "";
        
        StringBuffer sb = new StringBuffer();
        
        // header
        sb.append("$\\begin{array}{r|");
        for (int f=0; f<pinMat.size(); f++)
            sb.append("c|");
        sb.append("r}\n& ");
        for (int f=0; f<pinMat.size(); f++)
            sb.append("i_{"+f+"}& ");
        sb.append("\\mathbf{m}_0\\\\ \n\\hline\n");
                
        // row for a place  
        for (int pl=0; pl<places.size(); pl++) {
            String m0p = places.get(pl).getInitMarkingExpr();
            if (m0p.isBlank())
                m0p = "0";
            int m0val = -1000;
            try {
                m0val = Integer.parseInt(m0p);
            }
            catch (NumberFormatException e) {}
            
            sb.append(places.get(pl).getUniqueNameDecor().getLatexFormula().getLatex()+"&");
            for (int f=0; f<pinMat.size(); f++) {
                int card = 0;
                for (int k=0; k<pinMat.get(f).x.length; k++) {
                    if (pinMat.get(f).y[k] == pl+1) {
                        card = pinMat.get(f).x[k];
                        break;
                    }
                }
                sb.append(card + "&");
                if (m0val >= 0)
                    sum_pm0[f] += m0val * card;
                else if (card > 0)
                    extra_pm0[f] += (extra_pm0[f].isEmpty() ? "" : "+") +"("+m0p+")*"+card;
            } 
            sb.append("\\mathbf{"+m0p+"}\\\\ \n\\hline\n");
            
        }
        
        // final row
        sb.append("\n& ");
        for (int f=0; f<pinMat.size(); f++) {
            if (!extra_pm0[f].isEmpty()) {
                if (sum_pm0[f] == 0)
                    sb.append(extra_pm0[f]);
                else
                    sb.append(sum_pm0[f]+"+"+extra_pm0[f]);
            }
            else sb.append(sum_pm0[f]);
            sb.append("& ");
        }
        sb.append("\\\\ \n");
        
        
        sb.append("\\end{array}$");

        
        latex.TestJLaTeXMath.writeLatexFormulaAsImage(sb.toString(), baseName+"-pin.png", 36, false);
        System.out.println("Ok.");
    }
}
