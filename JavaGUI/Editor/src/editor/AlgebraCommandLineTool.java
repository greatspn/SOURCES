/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor;

import common.Util;
import static editor.UnfoldPNML2NetDefCommandLineTool.printGspnStat;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import editor.domain.elements.GspnPage;
import editor.domain.io.GreatSpnFormat;
import editor.domain.io.PNMLFormat;
import editor.domain.io.PnProFormat;
import editor.domain.unfolding.Algebra;
import java.awt.geom.Rectangle2D;
import java.io.File;
import java.util.Map;
import java.util.TreeMap;
import latex.DummyLatexProvider;

/**
 *
 * @author elvio
 */
public class AlgebraCommandLineTool {
    
    public static void main(String[] args) {
        try {
            Main.useGuiLogWindowForExceptions = false;
            Main.fixedUiSize = Main.UiSize.NORMAL;
            DummyLatexProvider.initializeProvider();
            long totalStart = System.currentTimeMillis();

            toolMain(args);

            System.out.println("TOTAL TIME: "+(System.currentTimeMillis() - totalStart)/1000.0);
            System.out.println("OK.");
        }
        catch (Throwable e) {
            System.out.println("EXCEPTION RAISED.");
            e.printStackTrace();
            System.out.println("FAILED.");
            System.exit(1);            
        }
    }
    
    public static void toolMain(String[] args) throws Exception {
        // Read command line arguments
        if (args.length < 1) {
            System.out.println("Not enough arguments.");
            System.exit(1);
        }
        String restSetPl = "";
        String restSetTr = "";
        int propPlacement = 1;
        boolean saveAsPnml = false;
        boolean verbose = false;
        boolean useBrokenEdges = true;
        int c;
        for (c=0; c<args.length; c++) {
//            System.out.println("args["+c+"]="+args[c]);
            if (!args[c].startsWith("-"))
                break; // end of options
            switch (args[c]) {
                case "-p":
                    if (c+1 == args.length) 
                        throw new IllegalArgumentException("Missing place restrictions set.");
                    restSetPl = args[++c];
                    break;
                    
                case "-t":
                    if (c+1 == args.length) 
                        throw new IllegalArgumentException("Missing transition restrictions set.");
                    restSetTr = args[++c];
                    break;
                    
                case "-out-pnml":
                    saveAsPnml = true;
                    break;
                    
                case "-horiz":
                    propPlacement = 1;
                    break;

                case "-vert":
                    propPlacement = 2;
                    break;

                case "-v":
                    verbose = true;
                    break;

                case "-no_ba":
                    useBrokenEdges = false;
                    break;

                case "--": // end of arguments
                    break;
                   
                default:
                    System.out.println("Unknown option: "+args[c]);
                    System.exit(1);
            }
        }
        
        if (c+3 != args.length) {
            throw new IllegalArgumentException("input1/input2/output net names are not correct.");
        }
        String inBaseName1 = args[c++];
        String inBaseName2 = args[c++];
        String outBaseName = args[c++];
        
//        System.out.println("inBaseName1 = " + inBaseName1);
//        System.out.println("inBaseName2 = " + inBaseName2);
//        System.out.println("outBaseName = " + outBaseName);
        
        // Load input nets
        long loadStart = System.currentTimeMillis();
        GspnPage net1 = loadPage(inBaseName1);
        printGspnStat(net1, null);
        System.out.println("");
        GspnPage net2 = loadPage(inBaseName2);
        printGspnStat(net2, null);
        System.out.println("");
        System.out.println("LOADING TIME: "+(System.currentTimeMillis() - loadStart)/1000.0);
        System.out.println("");
        
        
        // Do the algebra operation
        long composeStart = System.currentTimeMillis();
        System.out.println("COMPOSITION STARTS...\n");
        int dx2shift = 0, dy2shift = 0;
        Rectangle2D pageBounds1 = net1.getPageBounds();
        switch (propPlacement) {
            case 1: // horizontal
                dx2shift = (int)pageBounds1.getWidth() + 5;
                break;
            case 2: // vertical
                dy2shift = (int)pageBounds1.getHeight() + 5;
                break;
//            case 3: // user-specified
//                dx2shift = Integer.parseInt(textFieldDxShift.getText());
//                dy2shift = Integer.parseInt(textFieldDyShift.getText());
//                break;
        }

        Algebra a = new Algebra(net1, net2, 
                        restSetTr.replace(" ", "").split(","), 
                        restSetPl.replace(" ", "").split(","), 
                        dx2shift, dy2shift, useBrokenEdges, verbose);
        a.compose();
        GspnPage netComp = a.result;
        netComp.setPageName(net1.getPageName()+"_"+net2.getPageName());
        netComp.viewProfile = (ViewProfile)Util.deepCopy(net1.viewProfile);

        if (!a.warnings.isEmpty()) {
            for (String w : a.warnings)
                System.out.println(w);
            System.out.println("");
        }
        System.out.println("COMPOSITION TIME: "+(System.currentTimeMillis() - composeStart)/1000.0);

        
        // Save the composed net
        long saveStart = System.currentTimeMillis();
        System.out.println("");
        savePage(netComp, outBaseName, saveAsPnml);
        System.out.println("SAVING TIME: "+(System.currentTimeMillis() - saveStart)/1000.0);
    }
 
    
    private static GspnPage loadPage(String baseName) throws Exception {
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
    
    
    
    private static void savePage(GspnPage gspn, String baseName, boolean saveAsPnml) throws Exception {
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
            String ret = GreatSpnFormat.exportGspn(gspn, outNet, outDef, true);
            if (ret != null) {
                System.out.println("Problems exporting the net/def files:\n"+ret);
                System.exit(1);
            }
        }        
    }
}
