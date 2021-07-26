/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import common.Tuple;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnPage;
import editor.domain.elements.ParsedColorSubclass;
import editor.domain.elements.Place;
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
                                String[] agentsColorList, boolean verbose) 
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
//                    sumTerm = sumTerm.trim();
//                    System.out.println("sumTerm = "+sumTerm);
//                    if (sumTerm.charAt(0)!='<' || sumTerm.charAt(sumTerm.length()-1)!='>') {
//                        log.add("Invalid element "+sumTerm+" in initial marking of "+pl.getUniqueName());
//                        continue;
//                    }
//                    String sumTuple = sumTerm.substring(1, sumTerm.length() - 1);
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
        
        // Convert transitions
        
        
        return reportLog(log, pw);
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
