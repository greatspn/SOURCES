/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.cli;

import editor.Main;
import static editor.cli.Common.loadPage;
import static editor.cli.Common.printGspnStat;
import editor.domain.elements.GspnPage;
import editor.domain.io.NetLogoFormat;
import java.io.File;
import latex.DummyLatexProvider;

/**
 *
 * @author elvio
 */
public class NetLogoCommand {
    
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
        String agentColors = "";
//        String restSetTr = "";
//        int propPlacement = 1;
//        boolean saveAsPnml = false;
        boolean verbose = false;
//        boolean useBrokenEdges = true;
        int c;
        for (c=0; c<args.length; c++) {
//            System.out.println("args["+c+"]="+args[c]);
            if (!args[c].startsWith("-"))
                break; // end of options
            switch (args[c]) {
                case "-ac":
                    if (c+1 == args.length) 
                        throw new IllegalArgumentException("Missing comma-separated set of agent colors.");
                    agentColors = args[++c];
                    break;
                    
//                case "-t":
//                    if (c+1 == args.length) 
//                        throw new IllegalArgumentException("Missing transition restrictions set.");
//                    restSetTr = args[++c];
//                    break;
//                    
//                case "-out-pnml":
//                    saveAsPnml = true;
//                    break;
//                    
//                case "-horiz":
//                    propPlacement = 1;
//                    break;
//
//                case "-vert":
//                    propPlacement = 2;
//                    break;

                case "-v":
                    verbose = true;
                    break;

//                case "-no_ba":
//                    useBrokenEdges = false;
//                    break;

                case "--": // end of arguments
                    break;
                   
                default:
                    System.out.println("Unknown option: "+args[c]);
                    System.exit(1);
            }
        }
        
        if (c+2 != args.length) {
            throw new IllegalArgumentException("input/output net name is not correct.");
        }
        String inBaseName = args[c++];
//        String inBaseName2 = args[c++];
        String outBaseName = args[c++];
        
//        System.out.println("inBaseName1 = " + inBaseName1);
//        System.out.println("inBaseName2 = " + inBaseName2);
//        System.out.println("outBaseName = " + outBaseName);
        
        // Load input nets
        long loadStart = System.currentTimeMillis();
        GspnPage net1 = loadPage(inBaseName);
        printGspnStat(net1, null);
//        System.out.println("");
//        GspnPage net2 = loadPage(inBaseName2);
//        printGspnStat(net2, null);
        System.out.println("");
        System.out.println("LOADING TIME: "+(System.currentTimeMillis() - loadStart)/1000.0);
        System.out.println("");
        
        String[] agentColorList = agentColors.split(",");
        
        
        // Export in NetLogo format
//        long composeStart = System.currentTimeMillis();
//        System.out.println("COMPOSITION STARTS...\n");
//        int dx2shift = 0, dy2shift = 0;
//        Rectangle2D pageBounds1 = net1.getPageBounds();
//        switch (propPlacement) {
//            case 1: // horizontal
//                dx2shift = (int)pageBounds1.getWidth() + 5;
//                break;
//            case 2: // vertical
//                dy2shift = (int)pageBounds1.getHeight() + 5;
//                break;
////            case 3: // user-specified
////                dx2shift = Integer.parseInt(textFieldDxShift.getText());
////                dy2shift = Integer.parseInt(textFieldDyShift.getText());
////                break;
//        }
//
//        Algebra a = new Algebra(MergePolicy.BY_TAG, net1, net2, 
//                        restSetTr.replace(" ", "").split(","), 
//                        restSetPl.replace(" ", "").split(","), 
//                        dx2shift, dy2shift, useBrokenEdges, verbose);
//        a.compose();
//        GspnPage netComp = a.result;
//        netComp.setPageName(net1.getPageName()+"_"+net2.getPageName());
//        netComp.viewProfile = (ViewProfile)Util.deepCopy(net1.viewProfile);
//
//        if (!a.warnings.isEmpty()) {
//            for (String w : a.warnings)
//                System.out.println(w);
//            System.out.println("");
//        }
//        System.out.println("COMPOSITION TIME: "+(System.currentTimeMillis() - composeStart)/1000.0);

        
        // Save the composed net
        long saveStart = System.currentTimeMillis();
        System.out.println("");
        File netLogoFile = new File(outBaseName);
        String log = NetLogoFormat.export(net1, netLogoFile, agentColorList, verbose);
        
        System.out.println(log);
//        savePage(netComp, outBaseName, saveAsPnml);
        System.out.println("SAVING TIME: "+(System.currentTimeMillis() - saveStart)/1000.0);
    }
}
