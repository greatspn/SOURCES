/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import editor.domain.elements.DtaEdge;
import editor.domain.elements.DtaLocation;
import editor.domain.elements.DtaPage;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.TemplateVariable;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/**
 *
 * @author elvio
 */
public class DtaFormat {
    
    public static final FileFilter fileFilter = new FileNameExtensionFilter("MC4CSL^TA .dta file (*.dta)", new String[]{"dta"});

    
    public static String export(DtaPage dta, File file) throws Exception {
        if (!dta.isPageCorrect()) {
            throw new UnsupportedOperationException("DTA must be correct before exporting.");
        }
        
        ArrayList<String> log = new ArrayList<>();

        PrintWriter pw = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));
        
        StringBuilder clockSet = new StringBuilder();
        StringBuilder actSet = new StringBuilder();
        StringBuilder spSet = new StringBuilder();
        
        for (Node node : dta.nodes) {
            if (node instanceof TemplateVariable) {
                TemplateVariable tvar = (TemplateVariable)node;
                StringBuilder sb = null;
                switch (tvar.getType()) {
                    case REAL:       
                    case INTEGER:
                        sb = clockSet; break;
                    case ACTION:     sb = actSet; break;
                    case STATEPROP:  sb = spSet; break;
                    default:
                        log.add("Cannot export template variable "+tvar.getUniqueName());
                }
                if (sb != null) {
                    if (sb.length() > 0)
                        sb.append(", ");
                    sb.append(tvar.getUniqueName());
                }
            }
        }
        
        pw.println("\nDTA "+dta.getPageName()+" = {");
        pw.println("\tCLOCKVALUESET = { "+clockSet+" }");
        pw.println("\tACTIONSET = { "+actSet+" }");
        pw.println("\tATOMICPROPOSITIONSET = { "+spSet+" }\n\t");
        
        pw.println("\tLOCATIONS = {");
        for (Node node : dta.nodes) {
            if (node instanceof DtaLocation) {
                DtaLocation loc = (DtaLocation)node;
                pw.print("\t\t"+(loc.isInitial() ? "INITIAL " : ""));
                if (loc.isFinalAccepting())
                    pw.print("FINAL ");
                else if (loc.isFinalRejecting())
                    pw.print("REJECT ");
                pw.println(loc.getUniqueName()+" : "+loc.getStatePropositionDecor().getValue()+";");
            }
        }
        pw.println("\t}\n\n\tEDGES = {");
        for (Edge netEdge : dta.edges) {
            if (netEdge instanceof DtaEdge) {
                DtaEdge edge = (DtaEdge)netEdge;
                pw.print("\t\t"+edge.getTailNode().getUniqueName()+" -> "+edge.getHeadNode().getUniqueName()+" (");
                pw.print(edge.getClockGuardEditable().getValue()+", ");
                pw.print(edge.isInner() ? edge.getActSetEditable().getValue() : "#");
                if ((Boolean)edge.getResetClockEditable().getValue())
                    pw.print(", RESET");
                pw.println(");");
            }
        }
        pw.println("\t}\n}\n\n");
        
        pw.close();
        
        if (log.isEmpty())
            return null; // Everything went ok
        else {
            String message = "Detected problems in the exported MC4CSL^TA DTA.\n\n";
            for (String s : log)
                message += s + "\n";
            return message;
        }
    }
}
