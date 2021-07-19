/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.io;

import common.Tuple;
import editor.domain.Node;
import editor.domain.elements.ColorClass;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
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
                                String[] agentColorList, boolean verbose) 
            throws Exception 
    {
        
        PrintWriter pw = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));
        
        // Preamble
        pw.println(";; preamble");
        pw.println("extensions [rnd]");
        pw.println("globals [time gammatot]");
        pw.println();
        
        // Declare all agents
        pw.println(";; declare agent classes");
        for (String agentClass : agentColorList) {
            pw.println("breed ["+agentClass+" a_"+agentClass+"]");
        }
        pw.println();
        
        // set of agent classes
        Set<String> agentClasses = new HashSet<>();
        for (String ac : agentColorList)
            agentClasses.add(ac);
        
        // agent class -> attributes
        Map<String, Set<Tuple<String, Integer>>> agentAttrs = new HashMap<>();
        
        // Derive agent attributes from the color domains
        for (Node node : gspn.nodes) {
            if (node instanceof ColorClass) {
                ColorClass cc = (ColorClass)node;
                if (cc.isCrossDomain()) {
                    String agentClass = cc.getColorClassName(0);
                    boolean haveAttr = agentAttrs.containsKey(agentClass);
                    Set<Tuple<String, Integer>> attributes;
                    if (haveAttr)
                        attributes = agentAttrs.get(agentClass);
                    else
                        attributes = new HashSet<>();
                    if (!agentClasses.contains(agentClass)) {
                        throw new IllegalStateException("Color "+agentClass+
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
                    }
                    
                    if (!haveAttr)
                        agentAttrs.put(agentClass, attributes);
                }
            }
        }
        
        // Write agent attributes
        pw.println(";; Agent attributes");
        for (String agentClass : agentClasses) {
            pw.print(agentClass+"-own [");
            Set<Tuple<String, Integer>> attributes = agentAttrs.get(agentClass);
            for (Tuple<String, Integer> attr : attributes)
                pw.print(attr.x+"_"+attr.y+" ");
            pw.println("myrate]");
        }
        pw.println();
        
        // Check place sanity
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place pl = (Place)node;
                ColorClass plDom = pl.getColorDomain();
                
                if (plDom==null || plDom.isNeutralDomain()) {
                    throw new IllegalStateException("Place "+pl.getUniqueName()+
                            " does not have a color domain.");
                }
                String agentClass = plDom.getColorClassName(0);
                if (!agentClasses.contains(agentClass)) {
                    throw new IllegalStateException("Place "+pl.getUniqueName()+
                            " does not belong to an agent class.");
                }
            }
        }
        
        
        pw.close();
        return "";
    }
}
