/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.io;

import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.ParsedColorSubclass;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.Transition;
import editor.domain.elements.Transition.Type;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.grammar.EvaluationArguments;
import static editor.domain.grammar.ExpressionLanguage.GRML;
import editor.domain.grammar.ParserContext;
import static editor.domain.io.GreatSpnFormat.ensureInt;
import static editor.domain.io.GreatSpnFormat.ensureReal;
import static editor.domain.io.GreatSpnFormat.intOrMpar;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
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
            oldLineSep = sysProps.setProperty("line.separator", "\n");
            
            ArrayList<String> log = new ArrayList<>();

            PrintWriter grml = new PrintWriter(new BufferedOutputStream(new FileOutputStream(grmlFile)));
            
            int idCounter =2;
            grml.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            grml.println("<model id=\"1\" formalismUrl=\"http://formalisms.cosyverif.org/sptgd-net.fml\" xmlns=\"http://cosyverif.org/ns/model\">");
            grml.println("<attribute name=\"declaration\">");
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
            grml.println("\t</attribute>");
            
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
            grml.println("\t</attribute>"); 
            grml.println("</attribute>");

            for (Node node : gspn.nodes) {
                if (node instanceof ColorClass) {
                    ColorClass cc = (ColorClass)node;
                    if (!cc.isSimpleClass())
                        continue;
                    
                    grml.println("<attribute name=\"classDeclaration\">");
                    grml.println("\t<attribute name=\"name\">" + cc.getUniqueName() +"</attribute>");
                    grml.println("\t<attribute name=\"circular\">" + cc.isCircular() + "</attribute>");
                    
                    /*String[] subclassNames = new String[cc.numSubClasses()];
                    String allSubclasses = "";
                    for (int i=0; i<cc.numSubClasses(); i++) {
                        subclassNames[i] = (cc.getSubclass(i).isNamed() ? 
                                            cc.getSubclass(i).name : cc.getUniqueName()+"_"+i);
                        allSubclasses += (i==0 ? "" : ",") + subclassNames[i];
                    }
                    String colorDef = "("+cc.getUniqueName()+" c " +
                                      scaleCoord(cc.getX()) + " " +
                                      scaleCoord(cc.getY()) + " " +                            
                                      " (@c\n" + (cc.isCircular() ? "o " : "u ") +
                                      allSubclasses + "\n))";
                    defs.add(colorDef);
                    for (int i=0; i<cc.numSubClasses(); i++) {
                        ParsedColorSubclass pcs = cc.getSubclass(i);
                        String subClassDef;
                        if (pcs.isInterval())
                            subClassDef = (pcs.getIntervalPrefix() == null ? "" : pcs.getIntervalPrefix())+
                                          "{"+pcs.getStartRangeExpr()+"-"+pcs.getEndRangeExpr()+"}";
                        else {
                            subClassDef = "{";
                            for (int j=0; j<pcs.getNumColors(); j++)
                                subClassDef += (j==0 ? "" : ",") + pcs.getColorName(j);
                            subClassDef += "}";
                        }
                        colorDef = "("+subclassNames[i]+" c " +
                                      scaleCoord(cc.getX() + 4*(i+1)) + " " +
                                      scaleCoord(cc.getY()) + " " +                                
                                   " (@c\n"+subClassDef+"\n))";
                        defs.add(colorDef);
                    }*/
                    grml.println("</attribute>");
                }
            }
            
            
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
                    if(!initMarkExpr.isEmpty()){
                        grml.print("\t<attribute name=\"marking\"><attribute name=\"expr\">");
                        grml.print(plc.convertInitMarkingLang(context, GRML));
                        grml.println("</attribute></attribute>");
                    }else{
                        grml.print("\t<attribute name=\"marking\"><attribute name=\"expr\">");
                        grml.print("<attribute name=\"intValue\">0</attribute>");
                        grml.println("</attribute></attribute>");
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
                            grml.print(tr.convertDelayLang(context, null /* you dont need this param, it as a different usage*/, GRML));
                            grml.println("</attribute></attribute>");
                        }
                    } else {
                        grml.println(tr.convertDelayLang(context, null /* you dont need this param, it as a different usage*/, GRML));
                    }
                    grml.println("\t</attribute>");
                    
                    if(tr.hasWeight()){
                        grml.print("\t<attribute name=\"weight\"><attribute name=\"expr\">");
                        grml.print(tr.convertWeightLang(context, null /* you dont need this param, it as a different usage*/, GRML));
                        grml.println("</attribute></attribute>");
                    }
                    
                    if(tr.hasPriority()){
                        grml.print("\t<attribute name=\"priority\"><attribute name=\"expr\">");
                        grml.print(tr.convertPriorityLang(context, null /* you dont need this param, it as a different usage*/, GRML));
                        grml.println("</attribute></attribute>");
                    }
                    
                    
                                               
                    grml.println("</node>");
                }
                
            }
            for(Edge edge : gspn.edges){
                if (edge instanceof GspnEdge) {
                    GspnEdge arc = (GspnEdge) edge;
                    int source,target;
                    target = node2id.get(arc.getHeadNode());
                    source = node2id.get(arc.getTailNode());
                    String kind = "arc";
                    if(arc.getEdgeKind()== GspnEdge.Kind.INHIBITOR)kind="inhibitorarc";
                    
                    grml.println("<arc id=\"" + idCounter +"\" arcType=\""+kind+"\" source=\""+source+"\" target=\""+target+"\">");
                    idCounter= 1+ idCounter;
                    grml.print("\t<attribute name=\"valuation\"><attribute name=\"expr\">");
                    grml.print(arc.convertMultiplicityLang(context, GRML));
                    grml.println("</attribute></attribute>");
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
