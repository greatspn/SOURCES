/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.io;

import common.UnixPrintWriter;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.Transition;
import editor.domain.elements.Transition.Type;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.EvaluationException;
import static editor.domain.grammar.ExpressionLanguage.GRML;
import editor.domain.grammar.ParserContext;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/**
 *
 * @author benoit
 */
public class GRMLFormat {

    // The file filter
    public static final FileFilter fileFilter 
            = new FileNameExtensionFilter("GRML File (*.grml)", new String[]{"grml"});

    public static String exportGspn(GspnPage gspn, File grmlFile) throws Exception {
        if (!gspn.isPageCorrect()) {
            throw new UnsupportedOperationException("GSPN must be correct before exporting.");
        }

        Properties sysProps = System.getProperties();
        Object oldLineSep = null;
        try {
            ParserContext context = new ParserContext(gspn);
            try {
                gspn.compileParsedInfo(context);
            }
            catch (EvaluationException e) {
                throw new UnsupportedOperationException("GRML exporter does not support "
                        + "parametric color classes. All color classes bounds must be defined.");
            }
            oldLineSep = sysProps.setProperty("line.separator", "\n");
            
            ArrayList<String> log = new ArrayList<>();

            PrintWriter grml = new UnixPrintWriter(new BufferedOutputStream(new FileOutputStream(grmlFile)));
            
            int idCounter =2;
            grml.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            String formalUrl = "http://formalisms.cosyverif.org/sptgd-net.fml";
            if (gspn.gspnHasColors())
                formalUrl = "http://formalisms.cosyverif.org/swn.fml";
            grml.println("<model id=\"1\" formalismUrl=\""+formalUrl+"\" xmlns=\"http://cosyverif.org/ns/model\">");
            grml.println("<attribute name=\"declaration\">");
            // Declared color classes
            for (Node node : gspn.nodes) {
                if (node instanceof ColorClass) {
                    ColorClass cc = (ColorClass)node;
                    if (cc.isSimpleClass()) {
                        grml.println("<attribute name=\"classDeclaration\">");
                        grml.println("\t<attribute name=\"name\">"+cc.getUniqueName()+"</attribute>");
                        // Write the enumerated colors
                        grml.println("\t<attribute name=\"classType\">");
                        grml.println("\t<attribute name=\"classEnum\">");
                        int numColors = cc.numColors();
                        for (int i=0; i<numColors; i++) {
                            String clrName = cc.getColorName(i);
                            grml.println("\t\t<attribute name=\"enumValue\">"+clrName+"</attribute>");
                        }
                        grml.println("\t</attribute>"); // classEnum
                        grml.println("\t</attribute>"); // classType
                        // Write circular property
                        if (cc.isSimpleClass()) 
                            grml.println("\t<attribute name=\"circular\">"+(cc.isCircular() ? "true" : "false")+"</attribute>");
                        grml.println("</attribute>"); // classDeclaration
                    }
                    else { // domain class
                        grml.println("<attribute name=\"domainDeclaration\">");
                        grml.println("\t<attribute name=\"name\">"+cc.getUniqueName()+"</attribute>");
                        // Write the enumerated colors
                        grml.println("\t<attribute name=\"domainType\">");
                        grml.println("\t<attribute name=\"cartesianProduct\">");
                        for (int i=0; i<cc.getNumClassesInDomain(); i++) {
                            String clsName = cc.getColorClassName(i);
                            grml.println("\t\t<attribute name=\"type\">"+clsName+"</attribute>");
                        }
                        grml.println("\t</attribute>"); // cartesianProduct
                        grml.println("\t</attribute>"); // domainType
                        // Write circular property
                        if (cc.isSimpleClass()) 
                            grml.println("\t<attribute name=\"circular\">"+(cc.isCircular() ? "true" : "false")+"</attribute>");
                        grml.println("</attribute>"); // domainDeclaration
                        
                    }
                }
            }
            // Declared Color variables
            for (Node node : gspn.nodes) {
                if (node instanceof ColorVar) {
                    ColorVar cvar = (ColorVar)node;
                    grml.println("<attribute name=\"variableDeclaration\">");
                    grml.println("\t<attribute name=\"name\">"+cvar.getUniqueName()+"</attribute>");
                    grml.println("\t<attribute name=\"type\">"+cvar.getDomainExpr().getExpr()+"</attribute>");
                    grml.println("</attribute>"); // variableDeclaration
                }
            }
            // Declared constants
            grml.println("<attribute name=\"constants\">");
            grml.println("\t<attribute name=\"intConsts\">");
            for (Node node : gspn.nodes) {
                if (node instanceof ConstantID || node instanceof TemplateVariable) {
                    String value ="";
                    boolean isPop = false;
                    if (node instanceof ConstantID){
                      ConstantID con = (ConstantID)node;
                      if(con.isIntConst() && con.isInNeutralDomain()){
                          isPop = true;
                        value = con.getConstantExpr().convertLang(context, EvaluationArguments.NO_ARGS, GRML);
                      }
                    }
                    if (node instanceof TemplateVariable) {
                        TemplateVariable tvar = (TemplateVariable)node;
                        if (tvar.getType() == TemplateVariable.Type.INTEGER){
                            isPop=true;
                            value = "<attribute name=\"intValue\">0</attribute>";
                        }
                    }

                    if (isPop) {
                        grml.println("\t\t<attribute name=\"intConst\">");
                        grml.println("\t\t\t<attribute name=\"name\">"+ node.getUniqueName() +"</attribute>");
                        if(!value.isEmpty()){
                            grml.print("\t\t\t<attribute name=\"expr\">");
                            grml.print(value);
                            grml.println("</attribute>");
                        }
                        grml.println("\t\t</attribute>");
                    }
                   
                }
            }
            grml.println("\t</attribute>"); // end of int constants
            
            grml.println("\t<attribute name=\"realConsts\">");
            for (Node node : gspn.nodes) {
                if (node instanceof ConstantID || node instanceof TemplateVariable) {
                    String value ="";
                    boolean isPop = false;
                    if (node instanceof ConstantID){
                      ConstantID con = (ConstantID)node;
                      if(con.isRealConst() && con.isInNeutralDomain()){
                          isPop = true;
                        value = con.getConstantExpr().convertLang(context, EvaluationArguments.NO_ARGS, GRML);
                      }
                    }
                    if (node instanceof TemplateVariable) {
                        TemplateVariable tvar = (TemplateVariable)node;
                        if (tvar.getType() == TemplateVariable.Type.REAL){
                            isPop=true;
                            value = "<attribute name=\"numValue\">0</attribute>";
                        }
                    }

                    if (isPop) {
                        grml.println("\t\t<attribute name=\"realConst\">");
                        grml.println("\t\t\t<attribute name=\"name\">"+ node.getUniqueName() +"</attribute>");
                        if(!value.isEmpty()){
                            grml.print("\t\t\t<attribute name=\"expr\">");
                            grml.print(value);
                            grml.println("</attribute>");
                        }
                        grml.println("\t\t</attribute>");
                    }
                   
                }
            }
            grml.println("\t</attribute>"); // end of real constants
            grml.println("</attribute>"); // end of all declarations
            
            grml.println("</attribute>");
            
            Map<Node, Integer> node2id = new HashMap<>();
            for (Node node : gspn.nodes) {
                if (node instanceof Place) {
                    Place plc = (Place) node;
                    if (plc.isContinuous())
                        log.add("Continuous place "+plc.getUniqueName()+" is not convertible in GrML format.");
                    String initMarkExpr;
                    String color = "", domain = "";
                    
                    initMarkExpr = plc.getInitMarkingEditable().getValue().toString();
                    
                    grml.println("<node id=\"" + idCounter +"\" nodeType=\"place\" x=\"" + (int)(10 * plc.getCenterX()) + "\" y=\""+ (int)(10*plc.getCenterY()) + "\">");
                    node2id.put(node, idCounter);
                    idCounter= 1+ idCounter;
                    grml.println("\t<attribute name=\"name\">" + plc.getUniqueName() + "</attribute>");
                    grml.print("\t<attribute name=\"marking\">\n");
                    if (plc.isInColorDomain()) {
                        if(!initMarkExpr.isEmpty()) {
                            grml.print("\t\t");
                            grml.print(plc.convertInitMarkingLang(context, GRML));
                        }
                    }
                    else { // Neutral color
                        grml.print("\t<attribute name=\"expr\">\n\t\t");
                        if(!initMarkExpr.isEmpty())
                            grml.print(plc.convertInitMarkingLang(context, GRML));
                        else
                            grml.print("<attribute name=\"intValue\">0</attribute>");
                        grml.println("\n\t</attribute>\n");
                    }
                    grml.println("\n\t</attribute>"); // marking
                    
                    // Color domain
                    if (plc.isInColorDomain()) {
                        grml.println("\t<attribute name=\"domain\"><attribute name=\"type\">"+plc.getColorDomainName()+"</attribute></attribute>");
                    }
                    grml.println("</node>");
                }
                
                if (node instanceof Transition) {
                    Transition tr = (Transition) node;
                    if (tr.isContinuous())
                        log.add("Continuous transition "+tr.getUniqueName()+" is not convertible in GrML format.");
                    String initMarkExpr;
                    String color = "", domain = "";
                    
                    grml.println("<node id=\"" + idCounter +"\" nodeType=\"transition\" x=\"" + (int)(10 * tr.getCenterX()) + "\" y=\""+ (int)(10*tr.getCenterY()) + "\">");
                    node2id.put(node, idCounter);
                    idCounter= 1+ idCounter;
                    grml.println("\t<attribute name=\"name\">" + tr.getUniqueName() + "</attribute>");
                    
                    grml.println("\t<attribute name=\"distribution\">");
                    if (tr.getType() != Type.GEN) {
                        grml.print("\t\t<attribute name=\"type\">");
                        switch (tr.getType()) {
                            case EXP:
                                grml.print("EXPONENTIAL");
                                break;
                            case IMM:
                                grml.print("IMMEDIATE");
                                break;
                            default:
                                throw new UnsupportedOperationException("Continuous delay not supported");
                        }
                        grml.println("</attribute>");
                        if (tr.hasDelay()) {
                            grml.print("\t\t<attribute name=\"param\"><attribute name=\"expr\">");
                            grml.print(tr.convertDelayLang(context, null, GRML));
                            grml.println("</attribute></attribute>");
                        }
                    } else {
                        grml.println(tr.convertDelayLang(context, null, GRML));
                    }
                    grml.println("\t</attribute>");
                    
                    if(tr.hasWeight()){
                        grml.print("\t<attribute name=\"weight\"><attribute name=\"expr\">");
                        grml.print(tr.convertWeightLang(context, null, GRML));
                        grml.println("</attribute></attribute>");
                    }
                    
                    if (tr.hasNumServers()) {
                        grml.print("\t<attribute name=\"service\"><attribute name=\"expr\">");
                        grml.print(tr.convertNumServersLang(context, null, GRML));
                        grml.println("</attribute></attribute>");
                    }
                    
                    if(tr.hasPriority()) {
                        grml.print("\t<attribute name=\"priority\"><attribute name=\"expr\">");
                        grml.print(tr.convertPriorityLang(context, null, GRML));
                        grml.println("</attribute></attribute>");
                    }
                    
                    if (tr.hasGuard() && gspn.gspnHasColors()) {
                        grml.print("\t<attribute name=\"guard\"><attribute name=\"boolExpr\">");
                        grml.print(tr.convertGuardLang(context, null, GRML));
                        grml.println("</attribute></attribute>");
                    }
                    
                    grml.println("</node>");
                }                
            }
            
            // Print arcs
            for(Edge edge : gspn.edges){
                if (edge instanceof GspnEdge) {
                    GspnEdge arc = (GspnEdge) edge;
                    int source,target;
                    target = node2id.get(arc.getHeadNode());
                    source = node2id.get(arc.getTailNode());
                    String kind = "arc";
                    if(arc.getEdgeKind()== GspnEdge.Kind.INHIBITOR)
                        kind="inhibitorarc";
                    
                    grml.println("<arc id=\"" + idCounter +"\" arcType=\""+kind+"\" source=\""+source+"\" target=\""+target+"\">");
                    idCounter= 1+ idCounter;
                    grml.print("\t<attribute name=\"valuation\">");
                    if (arc.getConnectedPlace().isInColorDomain()) {
                        grml.print("\n\t\t");
                        grml.print(arc.convertMultiplicityLang(context, GRML));
                        grml.print("\n");
                    }
                    else {
                        grml.print("<attribute name=\"expr\">");
                        grml.print(arc.convertMultiplicityLang(context, GRML));
                        grml.println("</attribute>");
                    }
                    grml.println("</attribute>"); // valuation
                    grml.println("</arc>");
                }
            }
           

            
            grml.println("</model>");
            
            grml.close();

            if (log.isEmpty()) {
                return null; // Everything went ok
            }
            else {
                String message = "Detected problems in the exported GrML net.\n\n";
                for (String s : log) {
                    message += s + "\n";
                }
                return message;
            }
        }
        finally {
            // Reset the line separator
            if( oldLineSep != null ) {
                sysProps.put("line.separator", oldLineSep);
            }
        }
            
                    
    }
}
