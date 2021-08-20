/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import common.Tuple;
import editor.domain.Edge;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.elements.ParsedColorSubclass;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 *
 * @author elvio
 */
public class NetLogoFormat {
 
    public static String export(GspnPage gspn, File file, 
                                String[] agentsColorList, 
                                ParserContext context,
                                boolean verbose) 
            throws Exception 
    {
        ArrayList<String> log = new ArrayList<>();
        PrintWriter pw = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));
        
        // Get all mark/rate parameters as global variables
        String varNames = "";
        String varSetup = "";
        for (Node node : gspn.nodes) {
            if (node instanceof ConstantID) {
                ConstantID cid = (ConstantID)node;
                varNames += cid.getUniqueName()+" ";
                varSetup += "  set "+cid.getUniqueName()+" "+cid.getConstantExpr().getExpr()+"\n";
            }
        }
//        System.out.println("varNames="+varNames);
//        System.out.println("varSetup="+varSetup);
        
        // Preamble
        pw.println(";; preamble");
        pw.println("extensions [rnd]");
        pw.println("globals ["+varNames+"time gammatot]");
        pw.println();
        
        // Declare all agents
        pw.println(";; declare agent classes");
        for (String agentClass : agentsColorList) {
            pw.println("breed ["+agentClass+" a_"+agentClass+"]");
        }
        pw.println();
        
        // set of agent classes
        Set<String> agentClasses = new HashSet<>();
        for (String ac : agentsColorList) {
            agentClasses.add(ac);
            Node ncc = gspn.getNodeByUniqueName(ac);
            if (ncc!=null && ncc instanceof ColorClass) {
                ColorClass cc = (ColorClass)ncc;
                if (!cc.isSimpleClass())
                    log.add("Agent class "+ac+" is not a simple class.");
            }
            else
                log.add("Agent class "+ac+" does not exist as a color class.");
        }
        
        // agent class -> attributes
        Map<String, Set<Tuple<String, Integer>>> agentAttrs = new HashMap<>();
        // domain color classes -> attribute names
        Map<ColorClass, String[]> domain2attrs = new HashMap<>();
        
        // Derive agent attributes from the color domains
        for (Node node : gspn.nodes) {
            if (node instanceof ColorClass) {
                ColorClass cc = (ColorClass)node;
                if (cc.isCrossDomain()) {
                    String agentClass = cc.getColorClassName(0);
                    String[] attrNames = new String[cc.getNumClassesInDomain()];
                    boolean haveAttr = agentAttrs.containsKey(agentClass);
                    Set<Tuple<String, Integer>> attributes;
                    if (haveAttr)
                        attributes = agentAttrs.get(agentClass);
                    else
                        attributes = new HashSet<>();
                    if (!agentClasses.contains(agentClass)) {
                        log.add("Color "+agentClass+
                                " that appears as first color in domain "+cc.getUniqueName()+
                                " is not an agent class.");
                    }
                    Map<String, Integer> colorOccurrences = new HashMap<>();
                    for (int i=1; i<cc.getNumClassesInDomain(); i++) {
                        String colorAttr = cc.getColorClassName(i);
                        int rep = 0;                        
                        if (colorOccurrences.containsKey(colorAttr)) {
                            rep = colorOccurrences.get(colorAttr) + 1;
                            colorOccurrences.replace(colorAttr, rep);
                        }
                        else {
                            colorOccurrences.put(colorAttr, rep);
                        }
                        attributes.add(new Tuple<>(colorAttr, rep));
                        attrNames[i] = colorAttr+"_"+rep;
                    }
                    
                    if (!haveAttr)
                        agentAttrs.put(agentClass, attributes);
                    domain2attrs.put(cc, attrNames);
                }
            }
        }
        
        // Sanity check for places
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place pl = (Place)node;
                ColorClass plDom = pl.getColorDomain();
                
                if (plDom==null || plDom.isNeutralDomain()) {
                    log.add("Place "+pl.getUniqueName()+
                            " does not have a color domain.");
                }
                String agentClass = plDom.getColorClassName(0);
                if (!agentClasses.contains(agentClass)) {
                    log.add("Place "+pl.getUniqueName()+
                            " does not belong to an agent class.");
                }
            }
        }
        // Do not continue if problems were detected so far.
        if (!log.isEmpty())
            return reportLog(log, pw);

        // Pre-process all input arcs. 
        //  edge -> tuples of color variables
        Map<Edge, ArrayList<String[]>> edge2Agents = new HashMap<>(); 
        //  transition -> agents involved
        Map<Transition, Set<String>> trn2agents = new HashMap<>();
        // transition -> input edges
//        Map<Transition, Set<GspnEdge>> trn2inputTmp = new HashMap<>();
//        Map<Transition, GspnEdge[]> trn2input = new HashMap<>();
        Map<Transition, ArrayList<Tuple<GspnEdge, String[]>>> trn2inAgents = new HashMap<>();
        // Each arc expresses a sum of agents involved in the transition
        for (Edge edge : gspn.edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge e = (GspnEdge)edge;
                if (e.getConnectedPlace().isInNeutralDomain())
                    continue;
                ColorClass dom = e.getConnectedPlace().getColorDomain();
                assert domain2attrs.containsKey(dom);
                
                String nlMult = e.convertMultiplicityLang(context, ExpressionLanguage.NETLOGO);
//                System.out.println(e.getMultiplicity()+" ==> "+nlMult);
                String[] allTupleTerms = splitTupleSum(nlMult, log);
                // Separate sum terms
                ArrayList<String[]> tuples = new ArrayList<>();
                for (String tupleTerm : allTupleTerms) {
//                    if (!tupleTerm.startsWith("<") || !tupleTerm.endsWith(">")) {
//                        log.add("Could not separate terms of \""+e.getMultiplicity()+"\": error in: "+tupleTerm);
//                        break;
//                    }
//                    tupleTerm = tupleTerm.substring(1, tupleTerm.length()-1);
                    String[] colorVars = tupleTerm.split(",");
                    for (int i=0; i<colorVars.length; i++)
                        colorVars[i] = stripVarTerm(colorVars[i], log);
                    tuples.add(colorVars);
                    
                    Transition trn = e.getConnectedTransition();
                    if (!trn2agents.containsKey(trn)) {
                        trn2agents.put(trn, new HashSet<>());
//                        trn2inputTmp.put(trn, new HashSet<>());
                        trn2inAgents.put(trn, new ArrayList<>());
                    }
                    trn2agents.get(trn).add(colorVars[0]);
//                    trn2inputTmp.get(trn).add(e);
                    trn2inAgents.get(trn).add(new Tuple<>(e, colorVars));
                }
                edge2Agents.put(edge, tuples);
            }
        }
        /*for (Map.Entry<Edge, ArrayList<String[]>> ee : edge2Agents.entrySet()) {
            GspnEdge e2 = (GspnEdge)ee.getKey();
            System.out.println("edge "+e2.getEdgeKind()+
                    " "+e2.getConnectedPlace().getUniqueName()+
                    " "+e2.getConnectedTransition().getUniqueName());
            ColorClass dom = e2.getConnectedPlace().getColorDomain();
            String[] attrs = domain2attrs.get(dom);
            for (String[] tuple : ee.getValue()) {
                assert attrs.length == tuple.length;
                System.out.print("   agent: "+tuple[0]+"  attributes: ");
                for (int i=1; i<tuple.length; i++)
                    System.out.print(tuple[i]+"("+attrs[i]+") ");
                System.out.println("");
            }
        }//*/
        
//        for (Map.Entry<Transition, Set<GspnEdge>> ee : trn2inputTmp.entrySet()) 
//            trn2input.put(ee.getKey(), ee.getValue().toArray(new GspnEdge[ee.getValue().size()]));
//        trn2inputTmp = null;
        
        // agent -> transitions for which it is the leader
        Map<String, Set<Transition>> leadersOfTrn = new HashMap<>();
        Map<Tuple<String, Transition>, Integer> leaderTrnPos = new HashMap<>();
        for (String agentClass : agentClasses) 
            leadersOfTrn.put(agentClass, new HashSet<>());
        
        for (Map.Entry<Transition, ArrayList<Tuple<GspnEdge, String[]>>> ee : trn2inAgents.entrySet()) {
            String agentClass = ee.getValue().get(0).x.getConnectedPlace().getColorDomain().getColorClassName(0);
            leaderTrnPos.put(new Tuple<>(agentClass, ee.getKey()), leadersOfTrn.get(agentClass).size());
            leadersOfTrn.get(agentClass).add(ee.getKey());
            System.out.println(agentClass+" is leader of transition "+ee.getKey().getUniqueName());
        }


        ///////////////////////////////////////////
        // Write agent attributes
        pw.println(";; Agent attributes");
        for (String agentClass : agentClasses) {
            pw.print(agentClass+"-own [");
            Set<Tuple<String, Integer>> attributes = agentAttrs.get(agentClass);
            if (attributes != null) {
                for (Tuple<String, Integer> attr : attributes)
                    pw.print(attr.x+"_"+attr.y+" ");
            }
            pw.println("place myrate]");
        }
        pw.println();
        
        
        // Model Setup procedure
        pw.println(";; model setup procedure");
        pw.println("to setup");
        pw.print(varSetup);
        // write initial markings as sprout directives
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place pl = (Place)node;
                ColorClass plDom = pl.getColorDomain();
                if (plDom==null || plDom.isNeutralDomain()) 
                    continue;
                String agentClassName = plDom.getColorClassName(0);
                String initMark = pl.getInitMarkingExpr();
                if (initMark.isEmpty())
                    continue;
                
                String[] allSumTerms = splitTupleSum(initMark, log);
                for (String sumTerm : allSumTerms) { 
                    String[] sumTupleEl = sumTerm.split(",");
                    int numSproutedAgents = countColorNum(plDom.getColorClass(0), sumTupleEl[0], log);


                    pw.println("  sprout-"+agentClassName+" "+numSproutedAgents+" [");
                    pw.println("    set place "+pl.getUniqueName());
                    String[] attrs = domain2attrs.get(plDom);
                    // set the attributes
                    for (int i=1; i<plDom.getNumClassesInDomain(); i++) {
                        String value = sumTupleEl[i];
                        pw.println("    set "+attrs[i]+" "+value);
                    }
                    pw.println("  ]");
                }
            }
        }
        pw.println("end\n");
        
        
        // Ask all agents to initialize their myrate list
        pw.println(";; ask agents to initialize myrate");
        for (String agentClass : agentClasses) {
            Set<Transition> leadersOf = leadersOfTrn.get(agentClass);
            if (leadersOf.size() > 0) {
                pw.print("ask "+agentClass+" [set myrate list");
                for (int i=0; i<leadersOf.size(); i++)
                    pw.print(" 0");
                pw.println("]");
            }
        }
//        pw.println("ask turtles [set myrate [] ]");
        pw.println();
        
        // Convert transitions
        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition)node;
                Map<String, String> varConv = new HashMap<>();
                Set<String> knownVars = new HashSet<>();
                int ind=0;
                int agentNum = 1;
                
                ArrayList<Tuple<GspnEdge, String[]>> allInAgents = trn2inAgents.get(trn);
                String leaderAgentClass = allInAgents.get(0).x.getConnectedPlace().getColorDomain().getColorClassName(0);
                pw.println(";; transition "+trn.getUniqueName());
                
                // cycle through all input agents
                for (Tuple<GspnEdge, String[]> agent : allInAgents) {
                    Place plc = agent.x.getConnectedPlace();
                    ColorClass dom = plc.getColorDomain();
                    String[] attrs = domain2attrs.get(dom);
                    String guard = "";
//                    if (agentNum == allInAgents.size()) {
                    varConv.put("($"+agent.y[0]+"$)", "[who] of self");
                    for (int i=1; i<agent.y.length; i++)
                        varConv.put("($"+agent.y[i]+"$)", "["+attrs[i]+"] of self");
//                    }
                    
                    for (int i=0; i<agent.y.length; i++)
                        knownVars.add(agent.y[i]);
                    String nlGuard = trn.dropGuardSubTerms(context, knownVars, ExpressionLanguage.NETLOGO);
//                    String nlGuard = trn.convertGuardLang(context, null, ExpressionLanguage.NETLOGO);
//                        System.out.println(trn.getGuard()+" ==> "+nlGuard);
                    if (!nlGuard.isEmpty()) {
                        for (Map.Entry<String, String> ee : varConv.entrySet()) {
                            nlGuard = nlGuard.replace(ee.getKey(), ee.getValue());
                        }
                        guard = " AND " + nlGuard; 
                    }
                    
//                    {
//                        for (int i=0; i<agent.y.length; i++)
//                            knownVars.add(agent.y[i]);
//                        String drGuard = trn.dropGuardSubTerms(context, knownVars, ExpressionLanguage.NETLOGO);
//                        System.out.println("drGuard = "+drGuard);
//                    }
                    

                    indent(pw, ind); 
                    pw.println("let A"+agentNum+" "+dom.getColorClassName(0)+
                             " with [place = "+plc.getUniqueName()+guard+"]"); 
                    indent(pw, ind); pw.println("if-else A"+agentNum+" = nobody ["+
                            (agentNum==1 ? " set myrate  lput 0 myrate " : " ")+"]");
                    indent(pw, ind); pw.println("["); ind++;

                    if (agentNum == allInAgents.size()) { // last agent set
                        indent(pw, ind); pw.println("set countInstances  countInstances + (count A"+agentNum+")");
                    }
                    else {
                        // ask the selected agent
                        indent(pw, ind);  pw.println("ask A"+agentNum+" ["); ind++;
                        if (agentNum == 1) {
                             indent(pw, ind); pw.println("set countInstances 0");
                        }
                        // name self
                        indent(pw, ind); 
                        pw.println("let "+agent.y[0]+" [who] of self");
                        varConv.put("($"+agent.y[0]+"$)", agent.y[0]);
                        // name all attributes
                        for (int i=1; i<agent.y.length; i++) {
                            indent(pw, ind); 
                            pw.println("let "+agent.y[i]+" ["+attrs[i]+"] of self");
                            varConv.put("($"+agent.y[i]+"$)", agent.y[i]);
                        }
                    }
                    agentNum++;
                }
                
                while (ind > 0) {
                    if (ind == 2) {
                        // determine the transition index inside leadersOfTrn
                        Tuple<String, Transition> pp = new Tuple<>(leaderAgentClass, trn);
                        int myratePos = leaderTrnPos.get(pp);
                        indent(pw, ind); pw.println("replace-item "+myratePos+" myrate countInstances");
                    }
                    ind--;
                    indent(pw, ind); pw.println("]");
                }
            }
        }
        
        
        return reportLog(log, pw);
    }
    
    
    private static void indent(PrintWriter pw, int indentLevel) {
        for (int i=0; i<indentLevel; i++)
            pw.print("  ");
    }
    
    
    private static String[] splitTupleSum(String tupleSum, ArrayList<String> log) {
        ArrayList<String> tuples = new ArrayList<>();
        
        int start=-1;
        for (int i=0; i<tupleSum.length(); i++) {
            if (tupleSum.charAt(i) == '<') {
                if (start != -1) {
                    log.add("Cannot nest tuples: "+tupleSum);
                }
                start = i;
            }
            else if (tupleSum.charAt(i) == '>') {
                if (start == -1) {
                    log.add("Closed tuple: "+tupleSum);
                }
                else {
                    String term = tupleSum.substring(start+1, i).trim();
//                    System.out.println("term = "+term);
                    tuples.add(term);
                    start = -1;
                }
            }
        }
        return tuples.toArray(new String[tuples.size()]);
    }
    
    // Given a term in the form " ($var$)  ", returns "var"
    private static String stripVarTerm(String term, ArrayList<String> log) {
        term = term.strip();
        if (term.startsWith("($") && term.endsWith("$)")) {
            term = term.substring(2, term.length()-2);
            if (term.contains("$")) {
                log.add("The term "+term+" was not parsed correctly.");
            }
        }
        else {
            log.add("Could not identify the variable in "+term);
        }
        return term;
    }
    
    
    private static int countColorNum(ColorClass cc, String clrExpr, ArrayList<String> log) {
        int num = 0;
        String[] clrTerms = clrExpr.split("\\+");
        for (String clrTerm : clrTerms) {
            boolean found = false;
            int numSubClasses = cc.numSubClasses();
//            System.out.println("numSubClasses="+numSubClasses);
            // Try if clrTerm is a single color name
            for (int sc=0; sc<numSubClasses && !found; sc++) {
                ParsedColorSubclass pcsc = cc.getSubclass(sc);
//                System.out.println("confronto 1: "+pcsc.name);
                if (clrTerm.equals(pcsc.name)) {
                    // clrTerm is a static subclass
                    System.out.println("color term "+clrTerm+" is a static subclass");
                    num += pcsc.getNumColors();
                    found = true;
                }
//                System.out.println("Parsed static subclass "+pcsc.name+" has "+pcsc.getNumColors()+" colors.");
                for (int c=0; c<pcsc.getNumColors() && !found; c++) {
//                    System.out.println("confronto 2: "+pcsc.getColorName(c));
                    if (pcsc.getColorName(c).equals(clrTerm)) {
                        // clrTerm is a single named color
                        System.out.println("color term "+clrTerm+" is a named single color");
                        num += 1;
                        found = true;
                    }
                }
            }
            if (!found) {
                log.add("Color expression "+clrExpr+" is not valid.");
            }
        }
        return num;
    }
    
    // close the print writer stream and prepare the report to be returned
    private static String reportLog(ArrayList<String> log, PrintWriter pw) {
        pw.close();
        if (log.isEmpty()) {
            return null; // Everything went ok
        }
        else {
            String message = "Detected problems while exporting the NetLogo model.\n\n";
            for (String s : log) {
                message += s + "\n";
            }
            return message;
        }       
    }
}
