/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.io;

import editor.domain.Edge;
import editor.domain.NetObject;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.Transition;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.grammar.DomainElement;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import editor.domain.struct.StructInfo;
import editor.domain.values.EvaluatedFormula;
import editor.domain.values.MultiSet;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/** APNN (Abstract Petri Net Notation) file format, designed at TU-Dortmund.
 *
 * @author elvio
 */
public class ApnnFormat {
    
    // The file filter
    public static final FileFilter fileFilter 
            = new FileNameExtensionFilter("APNN File (*.apnn)", new String[]{"apnn"});
    
    // Transform an expression into an integer, acceptable in the APNN format.
    public static String getIntExpr(String expr, String what, Node node, GspnPage gspn, ArrayList<String> log) {
        if (expr.isEmpty())
            return "0";
        if (NetObject.isInteger(expr)) {
            return expr;
        }

        for (Node mpar : gspn.nodes) {
            if (mpar instanceof ConstantID) {
                ConstantID con = (ConstantID) mpar;
                if (con.isInNeutralDomain() && con.isIntConst()) {
                    return getIntExpr(con.getConstantExpr().getExpr(), what, node, gspn, log);
                }
            }
            else if (mpar instanceof TemplateVariable) {
                TemplateVariable tvar = (TemplateVariable)mpar;
                if (tvar.getUniqueName().equals(expr)) {
                    log.add("Could not export correctly " + what + " of " +node.getUniqueName()+
                            " with template value '" + expr + "'");
                    return "0";
                }
            }
        }
        log.add("Could not export correctly " + what + " of " +node.getUniqueName()+
                " with value '" + expr + "'");
        return "0";
    }
    
    public static int exportCoord(double x) {
        return (int)(10 * x);
    }
    
    // Generate all the colors in a color domain
    private static class ColorGenerator {
        public ColorGenerator(ColorClass domain, PrintWriter apnn) {
            this.domain = domain;
            this.apnn = apnn;
        }
        int count = 0;
        ColorClass domain;
        PrintWriter apnn;
        public void generateColors(int comp, String prefix) {
            if (comp == domain.getNumClassesInDomain())
                apnn.print((count++ == 0 ? "" : "|") + prefix);
            else {
                for (int c = 0; c < domain.getColorClass(comp).numColors(); c++) {
                    String newPrefix = prefix + (prefix.length()==0 ? "" : "_") +
                            domain.getColorClass(comp).getColorName(c);
                    generateColors(comp+1, newPrefix);
                }
            }
        }
    }
    
    // generate all the bindings for a given colred transition
    private static class TransitionFiringsGenerator {

        public TransitionFiringsGenerator(Transition trn, ParserContext context, 
                                          GspnPage gspn, ArrayList<String> log, 
                                          Map<GspnEdge, String> arcMult, Set<ColorVar> colorVars) {
            this.trn = trn;
            this.context = context;
            this.gspn = gspn;
            this.log = log;
            this.arcMult = arcMult;
            this.colorVars = colorVars;
        }

        Transition trn;
        ParserContext context;
        GspnPage gspn;
        ArrayList<String> log;
        Map<GspnEdge, String> arcMult;
        Set<ColorVar> colorVars;
        
        boolean guardErr = false;
        ColorVarsBinding colorVarsBinding = new ColorVarsBinding();
        
        public StringBuilder guard = new StringBuilder(); // mode = <name_1> orelse mode = <name_2> orelse ...
        public StringBuilder weight = new StringBuilder(); // case mode of <name_1> => value | <name_2> => value | ...

        public void generateFirings(int comp) {
            if (comp == colorVars.size()) { // All color variables are bound
                // Evaluate transition guard
                if (trn.hasGuard()) {
                    try {
                        EvaluatedFormula enab = trn.evaluateGuard(context, null, colorVarsBinding);
                        if (!enab.getScalarBoolean())
                            return; // Not enabled in this binding
                    }
                    catch (Exception e) {
                        if (!guardErr)
                            log.add("Transition guard of "+trn+" is not unfoldable, so APNN guard cannot be generated correctly.");
                        guardErr = true;
                    }
                }
                // Generate the mode variable
                StringBuilder mode = new StringBuilder();
                Iterator<ColorVar> it = colorVars.iterator();
                while (it.hasNext()) {
                    ColorVar cvar = it.next();
                    int color = colorVarsBinding.getValueBoundTo(cvar).getElement(0).getColor(0);
                    String colorN = colorVarsBinding.getValueBoundTo(cvar).getDomain().getColorName(color);
                    mode.append(mode.length()==0 ? "" : "_").append(cvar.getUniqueName())
                        .append("_").append(colorN);
                }
                // Add the mode in the transition guard
                guard.append(guard.length() == 0 ? "" : " orelse\n          ") 
                     .append("mode = ").append(mode);
                // Fill the mode cases for transition weight
                weight.append(weight.length() == 0 ? "" : " |\n                        ") 
                      .append(mode).append(" => ");
                if (trn.isImmediate())
                    weight.append(trn.reduceWeight(context, null, ExpressionLanguage.APNN));
                else if (trn.isGeneral())
                    weight.append("\\dist{const}{").append(trn.reduceDelay(context, null, colorVarsBinding, ExpressionLanguage.APNN)).append("}");
                else // exponential
                    weight.append(trn.reduceDelay(context, null, colorVarsBinding, ExpressionLanguage.APNN));
                
                // Add multiplicity function to transition arcs
                for (GspnEdge arc : arcMult.keySet()) {
                    String mult = arcMult.get(arc);
                    if (mult == null)
                        mult = "";
                    else mult += " |\n                          ";
                    mult += mode;
                    mult += " => ";
                    mult += arc.reduceMultiplicity(context, null, colorVarsBinding, ExpressionLanguage.APNN);
                    arcMult.put(arc, mult);
                }
            }
            else {
                // Enumerate colors
                Iterator<ColorVar> it = colorVars.iterator();
                ColorVar cvar = it.next();
                for (int i=0; i<comp; i++)
                    cvar = it.next();
                ColorClass dom = cvar.findColorClass(gspn);            
                for (int col = 0; col < dom.numColors(); col++) {
                    DomainElement elem = new DomainElement(dom, new int[]{col});
                    Set<DomainElement> elemSet = new HashSet<>();
                    elemSet.add(elem);
                    colorVarsBinding.bind(cvar, MultiSet.makeNew(dom, elemSet));
                    generateFirings(comp + 1);
                }
            }
        }
    }

    /**
     *
     * @param gspn The GSPN to be saved
     * @param apnnFile Target APNN file
     * @param structInfo Structural info - APNN needs place bounds.
     * @param optionalContext Optional GSPN context - APNN does not 
     *             support marking/rate parameters, so we need to expand them.
     * @return The error log.
     * @throws Exception
     */
    public static String exportGspn(GspnPage gspn, File apnnFile, StructInfo structInfo, ParserContext optionalContext) throws Exception {
        if (!gspn.isPageCorrect()) {
            throw new UnsupportedOperationException("GSPN must be correct before exporting.");
        }
        int entityCnt = 0;
        ArrayList<String> log = new ArrayList<>();
        Map<Node, Integer> node2entity = new HashMap<>();
        PrintWriter apnn = new PrintWriter(new BufferedOutputStream(new FileOutputStream(apnnFile)));
        ParserContext context = optionalContext;
        if (context == null)
            context = new ParserContext(gspn);
        try {
            gspn.compileParsedInfo(context);
            
            // Color classes
            Map<ColorClass, String> placesLike = new HashMap<>();

            // Write APNN header
            apnn.println("\\beginnet{"+gspn.getPageName()+"}");
            apnn.println("\\name{export}");

            // Write places
            for (Node node : gspn.nodes) {
                if (node instanceof Place) {
                    Place plc = (Place) node;
                    if (plc.isContinuous())
                        log.add("Continuous place "+plc.getUniqueName()+" is not convertible in APNN format.");
                    
                    String initMark = plc.reduceInitMarking(context, ExpressionLanguage.APNN);
                    
                    apnn.println("\\place{a"+(++entityCnt)+"}{");
                    node2entity.put(node, entityCnt);
                    apnn.println("  \\name{"+plc.getUniqueName()+"}");
                    if (plc.getKroneckerPartition().length() > 0)
                        apnn.println("  \\partition{"+plc.getKroneckerPartition()+"}");
                    apnn.println("  \\point{"+exportCoord(plc.getCenterX())+" "+exportCoord(plc.getCenterY())+"}");

                    if (plc.isInColorDomain()) {
                        ColorClass dom = plc.getColorDomain();
                        if (placesLike.containsKey(dom))
                            apnn.println("  \\colour{\\like{" + placesLike.get(dom) + "}}");
                        else {
                            // List the colors in this domain, one by one
                            apnn.print("  \\colour{\\with ");
                            ColorGenerator gen = new ColorGenerator(dom, apnn);
                            gen.generateColors(0, "");
                            apnn.println("} ");
                            placesLike.put(dom, plc.getUniqueName());
                        }
                        apnn.println("  \\init{"+initMark+"}");
                    }
                    else { // uncolored
                        apnn.println("  \\colour{with unnamed}");
                        apnn.println("  \\screen_colours{0'unnamed}");
                        apnn.println("  \\init{"+initMark+"'unnamed}");
                        if (structInfo != null)
                            apnn.println("  \\capacity{"+structInfo.boundsOfPlace.get(plc.getUniqueName()).y+"'unnamed}");
                    }

                    //System.out.println(plc.getInitMarkingEditable().getValue() + " --> "+initMark);
//                    if (!initMark.equals("0"))
//                        apnn.print("\\init{"+initMark+"} ");
                    apnn.println("}");
                }
            }

            StringBuilder arcs = new StringBuilder();
            // Write transitions
            for (Node node : gspn.nodes) {
                if (node instanceof Transition) {
                    Transition trn = (Transition) node;
                    if (trn.isContinuous())
                        log.add("Continuous transition "+trn.getUniqueName()+" is not convertible in APNN format.");

                    apnn.println("\\transition{a"+(++entityCnt)+"}{");
                    node2entity.put(node, entityCnt);
                    apnn.println("  \\name{"+trn.getUniqueName()+"}");

                    // Priority
                    if (trn.isImmediate())
                        apnn.println("  \\prio{"+trn.reducePriority(context, null, null/*no color binding*/, ExpressionLanguage.APNN)+"}");
                    else // timed
                        apnn.println("  \\prio{0} ");
                    apnn.println("  \\point{"+exportCoord(trn.getCenterX())+" "+exportCoord(trn.getCenterY())+"}");
                    
                    // # of servers
                    if (trn.isExponential()&& !trn.getNumServers().equals("1"))
                        log.add("Transition "+trn.getUniqueName()+" is not single server. "
                                + "This is currently not supported for APNN export.");                    
                    
                    Set<ColorVar> colorVars = new HashSet<>();
                    colorVars.addAll(trn.getColorVarsInUse());
                    
                    // Arc multiplicity function for this transition
                    Map<GspnEdge, String> arcMult = new HashMap<>();
                    for (Edge e : gspn.edges) {
                        if (e instanceof GspnEdge && ((GspnEdge)e).getConnectedTransition() == trn) {
                            arcMult.put((GspnEdge)e, null);
                            colorVars.addAll(((GspnEdge)e).getColorVarsInUse());
                        }
                    }
                    
                    // Non colored transition
                    if (colorVars.isEmpty()) {
                        String weight;
                        if (trn.isImmediate())
                            weight = trn.reduceWeight(context, null, ExpressionLanguage.APNN);
                        else
                            weight = trn.reduceDelay(context, null, null/*no color binding*/, ExpressionLanguage.APNN);
                        apnn.println("  \\weight{case mode of unnamed => "+weight+"}");
                        apnn.println("  \\guard{mode = unnamed}");
                        apnn.println("  \\screen_colours{case mode of unnamed => 0}");
                    }
                    else { // Unfolding of the guard/weight/arcs
                        
                        // Generate the unfolding of the color variables
                        TransitionFiringsGenerator tfg = new TransitionFiringsGenerator(trn, context, gspn, log, arcMult, colorVars);
                        tfg.generateFirings(0);
                        
                        apnn.println("  \\guard{ " + tfg.guard + " }");
			apnn.println("  \\weight{ case mode of " + tfg.weight + " }");
                    }
                    
                    apnn.println("}");
                    
                    // Write arcs of this transition
                    for (GspnEdge arc : arcMult.keySet()) {
                        arcs.append("\\arc{a").append(++entityCnt).append("}{\n");
                        arcs.append("  \\from{a").append(node2entity.get(arc.getTailNode())).append("}\n");
                        arcs.append("  \\to{a").append(node2entity.get(arc.getHeadNode())).append("}\n");
//                        arcs.append("\\from{").append(arc.getTailNode().getUniqueName()).append("} ");
//                        arcs.append("\\to{").append(arc.getHeadNode().getUniqueName()).append("} ");
                        if (arc.getEdgeKind() == GspnEdge.Kind.INHIBITOR)
                            arcs.append("  \\type{inhibitor}\n");
                        String mult;
                        if (arcMult.get(arc) == null) { // no unfolded arc function
                            mult = arc.reduceMultiplicity(context, null, ColorVarsBinding.EMPTY_BINDING, ExpressionLanguage.APNN);
                            mult = "case mode of unnamed => "+mult+"'unnamed";
                        }
                        else {
                            arcs.append("\n    ");
                            mult = "case mode of " + arcMult.get(arc);
                        }
//                        if (!arc.getMultiplicity().equals("1"))
                        arcs.append("  \\weight{").append(mult).append("}\n");

                        if (arc.numPoints() > 2) {
                            arcs.append("  \\spline{1}{ ");
                            for (int i=1; i<arc.numPoints()-1; i++)
                                arcs.append("\\point{").append(exportCoord(arc.points.get(i).getX())).append(" ")
                                        .append(exportCoord(arc.points.get(i).getY())).append("} ");
                            arcs.append("}\n");
                        }
                        arcs.append("}\n");
                    }
                }
            }
            apnn.println(arcs.toString());

            // footer
            apnn.println("\\endnet");
            apnn.close();

            if (log.isEmpty()) {
                return null; // Everything went ok
            }
            else {
                String message = "Detected problems in the exported APNN net.\n\n";
                for (String s : log) {
                    message += s + "\n";
                }
                return message;
            }
        }
        finally {
            gspn.compileParsedInfo(null);
        }
    }
}
