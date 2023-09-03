/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.unfolding;

import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author elvio
 */
public class IncidenceMatrixFormatter {
    private final GspnPage gspn;
    
    ArrayList<Place> places = new ArrayList<>();
    ArrayList<Transition> trans = new ArrayList<>();
    Map<Place, Integer> pl2pos = new HashMap<>();
    Map<Transition, Integer> tr2pos = new HashMap<>();

    
    public IncidenceMatrixFormatter(GspnPage gspn) {
        this.gspn = gspn;
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
    }

    public GspnPage getGspn() {
        return gspn;
    }
    
    public String latexFor(MatrixMode mode, boolean separateSums, boolean writeZeros, boolean stacked) {
        PartialSum[][] matAdd=null, matSub=null;
        switch (mode) {
            case INPUT_MATRIX:
                matAdd = extractMatrix(gspn, GspnEdge.Kind.INPUT, pl2pos, tr2pos);
                break;
                
            case OUTPUT_MATRIX:
                matAdd = extractMatrix(gspn, GspnEdge.Kind.OUTPUT, pl2pos, tr2pos);
                break;
                
            case INHIBITOR_MATRIX:
                matAdd = extractMatrix(gspn, GspnEdge.Kind.INHIBITOR, pl2pos, tr2pos);
                break;
                
            case INCIDENCE_MATRIX:
                matAdd = extractMatrix(gspn, GspnEdge.Kind.OUTPUT, pl2pos, tr2pos);
                matSub = extractMatrix(gspn, GspnEdge.Kind.INPUT, pl2pos, tr2pos);
                break;
        }
        boolean twoMatrices = (matSub != null);
        
        StringBuilder sb = new StringBuilder();
        
        // header
        sb.append("$");
//        sb.append("\\mathbf{I}-\\mathbf{O} = ");
        sb.append("\\begin{array}{r|");
        for (int f=0; f<trans.size(); f++)
            sb.append("c|");
        sb.append("r}\n& ");
        for (int f=0; f<trans.size(); f++)
            sb.append("\\rotatebox{90}{").append(trans.get(f).getUniqueNameDecor().getVisualizedValue()).append("}& ");
        sb.append("\\mathbf{m}_0\\\\ \n\\hline\n");

        final GspnEdge.Kind[] kinds = new GspnEdge.Kind[]{ GspnEdge.Kind.OUTPUT, GspnEdge.Kind.INPUT };
        for (int pl=0; pl<places.size(); pl++) {
            sb.append(places.get(pl).getUniqueNameDecor().getLatexFormula().getLatex()).append(" & ");

            for (int tr=0; tr<trans.size(); tr++) {
                PartialSum inVal = matAdd[pl][tr];
                PartialSum outVal = matSub != null ? matSub[pl][tr] : null;
                
                if (twoMatrices && separateSums) {
                    String color;
                    if (inVal.isEmpty() && outVal.isEmpty()) {
                        if (writeZeros)
                            sb.append("\\textcolor{Gray}{0}");
                    }
                    else {
                        String sIn = inVal.toStringEmptyZero("");
                        if (!sIn.isBlank())
                            sIn = "\\textcolor{Blue}{"+sIn+"}";
                        String sOut = outVal.toStringEmptyZero("-");
                        if (!sOut.isBlank())
                            sOut = "\\textcolor{Mahogany}{"+sOut+"}";
                        
                        if (stacked) {
                            sb.append("\\genfrac{}{}{0pt}{0}{").append(sIn).append("}{").append(sOut).append("}");
                        }
                        else {
                            sb.append(sIn).append(sOut);
                        }
                    }
                }
                else {
                    PartialSum inOutSum;
                    if (twoMatrices)
                        inOutSum = inVal.subtract(outVal);
                    else
                        inOutSum = inVal;
//                    System.out.println("inOutSum="+inOutSum+" inVal="+inVal+" outVal="+outVal);
                    String color;
                    if (inOutSum.isZero())
                        color = "Gray";
                    else {
                        if (!inOutSum.isRestBlank())
                            color = "ForestGreen";
                        else if (inOutSum.getIntPart() > 0)
                            color = "Blue";
                        else
                            color = "Mahogany";
                    }
                    sb.append("\\textcolor{").append(color).append("}{")
                            .append(inOutSum.toStringExtended("", writeZeros ? "0" : "")).append("}");
                }
                sb.append(" & ");
            }
            
            String m0p = places.get(pl).getInitMarkingExpr();
            if (m0p.isBlank())
                m0p = "0";
            sb.append("\\mathbf{"+m0p+"}\\\\ \n\\hline\n");
        }
        
        sb.append("\\end{array}$");
        return sb.toString();
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
//                    System.out.println("matrix[pl][tr]="+matrix[pl][tr]);
                }
            }
        }
        return matrix;
    }

    
}
