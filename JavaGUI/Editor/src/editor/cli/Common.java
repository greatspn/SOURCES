/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.cli;

import editor.domain.Node;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.Transition;
import editor.domain.io.GreatSpnFormat;
import editor.domain.io.PNMLFormat;
import editor.domain.io.PnProFormat;
import java.io.File;
import java.util.Map;
import java.util.TreeMap;

/**
 *
 * @author elvio
 */
public class Common {
    
    // show stats of the GSPN
    public static void printGspnStat(GspnPage gspn, PNMLFormat.NuPNUnit rootUnit) {
        int numPlaces = 0;
        int numColorClasses = 0;
        int numColorVars = 0;
        int numTransitions = 0;
        int numConst = 0;
        int numTemplateVars = 0;
        int numOtherNodes = 0;
//        int numNuPNUnits = -1;
        
        for (Node node : gspn.nodes) {
            if (node instanceof Place)
                numPlaces++;
            else if (node instanceof Transition) 
                numTransitions++;
            else if (node instanceof ColorClass)
                numColorClasses++;
            else if (node instanceof ColorVar)
                numColorVars++;
            else if (node instanceof ConstantID)
                numConst++;
            else if (node instanceof TemplateVariable)
                numTemplateVars++;
            else
                numOtherNodes++;
        }
        
        boolean hasColors = (numColorClasses + numColorVars) > 0;        
        System.out.println("  MODEL CLASS:   "+(hasColors ? "SYMMETRIC NET" : "P/T NET")+
                (rootUnit==null ? "" : " WITH NUPN EXTENSION"));
        System.out.println("  PLACES:        "+numPlaces);
        System.out.println("  TRANSITIONS:   "+numTransitions);
        System.out.println("  CONSTANTS:     "+numConst);
        System.out.println("  TEMPLATE VARS: "+numTemplateVars);
        if (hasColors) {
            System.out.println("  COLOR CLASSES: "+numColorClasses);
            System.out.println("  COLOR VARS:    "+numColorVars);
        }
        if (numOtherNodes > 0)
            System.out.println("  OTHER NODES:   "+numOtherNodes);
        System.out.println("  ARCS:          "+gspn.edges.size());
        if (rootUnit != null) {
            System.out.println("  NUPN UNITS:    "+rootUnit.numUnits);
        }
    }
    
    
    public static GspnPage loadPage(String baseName) throws Exception {
        GspnPage gspn = null;
        PNMLFormat.NuPNUnit rootUnit[] = new PNMLFormat.NuPNUnit[1];
        // Input files
        String netName = new File(baseName).getName();
        File inputPnpro = new File(baseName+".PNPRO");
        File inputPnml = new File(baseName+".pnml");
        File inputNet = new File(baseName+".net");
        File inputDef = new File(baseName+".def");
        
        if (inputPnpro.exists()) {
            ProjectFile pf = PnProFormat.readXML(inputPnpro);
            ProjectPage page0 = pf.getCurrent().getPageAt(0);
            if (!(page0 instanceof GspnPage)) {
                System.out.println("The first page of the PNPRO file "+inputPnpro.getAbsolutePath()+" must be a GSPN.");
                System.exit(1);
            }
            gspn = (GspnPage)page0;
            gspn.setPageName(netName);
        }
        else if (inputPnml.canRead()) {
            // Read input PNML file
            System.out.println("LOADING "+inputPnml.getName()+" ...");
            Map<String, String> pnmlId2name = new TreeMap<>();
            gspn = new GspnPage();
            String log = PNMLFormat.importPNML(gspn, inputPnml, pnmlId2name, rootUnit);
            gspn.setPageName(netName);
            if (log != null) {
                System.out.println("Error loading PNML file "+inputPnml.getAbsolutePath()+".\n"+log);
                System.exit(1);
            }
        }
        else if (inputNet.canRead() && inputDef.canRead()) {
            // Read net/def file
            System.out.println("LOADING "+inputNet.getName()+"/def ...");
            gspn = new GspnPage();
            String log = GreatSpnFormat.importGspn(gspn, inputNet, inputDef);
            gspn.setPageName(netName);
            if (log != null) {
                System.out.println("Error loading net/def file "+baseName+".(net/def).\n"+log);
                System.exit(1);
            }
        }
        else {
            System.out.println("Cannot read file: "+baseName+".{PNPRO,pnml,net/def}");
            System.exit(1);
        }
        
        return gspn;
    }
    
    
    public static void savePage(GspnPage gspn, String baseName, boolean saveAsPnml) throws Exception {
        boolean savePnmlGfx = true;
        if (saveAsPnml) {
            File pnmlNet = new File(baseName+".pnml");
            System.out.println("SAVING AS "+baseName+".pnml ...");
            String ret = PNMLFormat.exportGspn(gspn, pnmlNet, savePnmlGfx);
            if (ret != null) {
                System.out.println("Problems exporting the PNML file:\n"+ret);
                System.exit(1);
            }
        }
        else { // Save in GreatSPN format
            // Save in GreatSPN net/def format
            File outNet = new File(baseName+".net");
            File outDef = new File(baseName+".def");
            System.out.println("SAVING AS "+baseName+".(net/def) ...");
            String ret = GreatSpnFormat.exportGspn(gspn, outNet, outDef, true, true);
            if (ret != null) {
                System.out.println("Problems exporting the net/def files:\n"+ret);
                System.exit(1);
            }
        }        
    }
}
