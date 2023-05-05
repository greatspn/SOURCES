/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.cli;

import common.UnixPrintWriter;
import editor.Main;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.grammar.DomainElement;
import editor.domain.grammar.SemanticParser;
import editor.domain.io.GreatSpnFormat;
import editor.domain.io.PNMLFormat;
import editor.domain.io.PnProFormat;
import editor.domain.unfolding.Unfolding;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.TreeMap;
import latex.DummyLatexProvider;
import static editor.cli.Common.*;

/**
 *
 * @author elvio
 */
public class UnfoldingCommand {
    
    public static void main(String[] args) {
        try {
            toolMain(args);
        }
        catch (Throwable e) {
            System.out.println("EXCEPTION RAISED.");
            e.printStackTrace();
            System.out.println("FAILED.");
            System.exit(1);            
        }
    }
    
    public static void toolMain(String[] args) throws Exception {
        Main.useGuiLogWindowForExceptions = false;
        Main.fixedUiSize = Main.UiSize.NORMAL;
        DummyLatexProvider.initializeProvider();
        long totalStart = System.currentTimeMillis();
        boolean doShuffle = false;
        boolean shuffleNames = false;
        boolean saveNameMap = false, savePnmlGfx = true;
        boolean saveAsPnml = false;
        boolean saveAsNetDef = true;
        boolean saveAsPNPRO = false;
        boolean anonimize = false;
        boolean useLongNames = false;
        boolean doUnfolding = true;
        boolean useEvaluationCache = false;
        boolean outSuffix = true;
        SemanticParser.strictColorExpressionChecks = false;
        
        // Read command line arguments
        if (args.length < 1) {
            System.out.println("Missing PNML filename argument.");
            System.exit(1);
        }
        String baseName = args[0];
        for (int c=1; c<args.length; c++) {
            switch (args[c]) {
                case "-shuffle":
                    doShuffle = true;
                    break;
                    
                case "-shuffle-names":
                    shuffleNames = true;
                    break;

                case "-name-map":
                    saveNameMap = true;
                    break;
                    
                case "-out-pnml":
                    saveAsPnml = true;
                    break;
                    
                case "-out-pnpro":
                    saveAsPNPRO = true;
                    break;
                    
                case "-no-out-netdef":
                    saveAsNetDef = false;
                    break;
                    
                case "-no-pnml-gfx":
                    saveAsPnml = true;
                    savePnmlGfx = false;
                    break;
                    
                case "-anonimize":
                    anonimize = true;
                    break;
                    
                case "-long-names":
                    useLongNames = true;
                    break;

                case "-short-names":
                    useLongNames = false;
                    break;
                    
                case "-no-unfolding":
                    doUnfolding = false;
                    break;
                    
                case "-no-suffix":
                    outSuffix = false;
                    break;

                case "-eval-cache":
                    System.out.println("Evaluation cache is bugged and does not work reliably!");
                    useEvaluationCache = true;
                    break;
                    
                default:
                    System.out.println("Unknown option: "+args[c]);
                    System.exit(1);
            }
        }
//            String baseName = "/Users/elvio/Desktop/MCC/Philosophers/COLORED/philosophers-1000";
//            String baseName = "/Users/elvio/Desktop/MCC/Philosophers/PT/philosophers-10";
//            String baseName = "/Users/elvio/Desktop/MCC/SimpleLoadBal/COLORED/simple_lbs-2";

//        Util.initApplication(Main.PREF_ROOT_KEY, "/org/unito/mainprefs");
        long start = System.currentTimeMillis();
        GspnPage gspn = new GspnPage();
        PNMLFormat.NuPNUnit rootUnit[] = new PNMLFormat.NuPNUnit[1];
        Map<String, String> pnmlId2name = null;
        // Input files
        File inputPnpro = new File(baseName+".PNPRO");
        File inputPnml = new File(baseName+".pnml");
        File inputNet = new File(baseName+".net");
        File inputDef = new File(baseName+".def");
        if (inputPnpro.exists()) {
            ProjectFile pf = PnProFormat.readXML(inputPnpro);
            ProjectPage page0 = pf.getCurrent().getPageAt(0);
            if (!(page0 instanceof GspnPage)) {
                System.out.println("The first page of the PNPRO file must be the unfolded GSPN.");
                System.exit(1);
            }
            gspn = (GspnPage)page0;
        }
        else if (inputPnml.canRead()) {
            // Read input PNML file
            System.out.println("LOADING "+inputPnml.getName()+" ...");
            pnmlId2name = new TreeMap<>();
            String log = PNMLFormat.importPNML(gspn, inputPnml, pnmlId2name, rootUnit);
            if (log != null) {
                System.out.println("Error loading PNML file.\n"+log);
                System.exit(1);
            }
        }
        else if (inputNet.canRead() && inputDef.canRead()) {
            // Read net/def file
            System.out.println("LOADING "+inputNet.getName()+"/def ...");
            String log = GreatSpnFormat.importGspn(gspn, inputNet, inputDef);
            if (log != null) {
                System.out.println("Error loading net/def file.\n"+log);
                System.exit(1);
            }
            outSuffix = true;
        }
        else {
            System.out.println("Cannot read file: "+baseName+".{PNPRO,pnml,net/def}");
            System.exit(1);
        }
        printGspnStat(gspn, rootUnit[0]);
        System.out.println("LOADING TIME: "+(System.currentTimeMillis() - start)/1000.0);
        System.out.println("");

        
        if (outSuffix) 
            baseName += doUnfolding ? "_unf" : "_new";

        // GSPN semantic check
        ArrayList<ProjectPage> pages = new ArrayList<>();
        pages.add(gspn);
        ProjectData proj = new ProjectData("project", pages);
        gspn.preparePageCheck();
        gspn.checkPage(proj, null, gspn, null);
        if (!gspn.isPageCorrect()) {
            for (int err=0; err < gspn.getNumErrorsAndWarnings(); err++) {
                PageErrorWarning pew = gspn.getErrorWarning(err);
                System.out.println(" "+pew.getDescription());
            }
            System.out.println("Found errors in loaded file. Stop.");
            System.exit(1);
        }

        // Unfold if the net has colors
        boolean unfolded = false;
        Unfolding unf = null;
        if (gspn.gspnHasColors() && doUnfolding) {
//            if (saveNameMap) {
//                System.out.println("Warning: Cannot save name map when unfolding...");
//                saveNameMap = false;
//            }
            start = System.currentTimeMillis();
            System.out.println("UNFOLDING COLORS...");
            unf = new Unfolding(gspn);
            unf.useLongUnfoldedNames = useLongNames;
            unf.useEvaluationCache = useEvaluationCache;
            unf.unfold();
            unf.unfolded.setPageName(gspn.getPageName()+"_unfolded");
            gspn = unf.unfolded;
            unfolded = true;
            rootUnit[0] = null; // Cannot unfold NuPN units...
            printGspnStat(gspn, rootUnit[0]);
            System.out.println("UNFOLDING TIME: "+(System.currentTimeMillis() - start)/1000.0);
            System.out.println("");
        }

        // Optinally shuffle place/transitions in the unfolded net
        if (doShuffle) {
            System.out.println("SHUFFLING NET ELEMENTS...");
            Random randomGenerator = new Random();
            for (Node node : gspn.nodes) {
                int k1 = randomGenerator.nextInt(gspn.nodes.size());
                int k2 = randomGenerator.nextInt(gspn.nodes.size());
                Node n1 = gspn.nodes.get(k1);
                gspn.nodes.set(k1, gspn.nodes.get(k2));
                gspn.nodes.set(k2, n1);
            }
        }
        // Shuffle names (swap names of similar objects)
        // The name map will not have any meaning after this shuffling
        if (shuffleNames) {
            if (saveNameMap)
                System.out.println("WARNING: After shuffling the name map will be meaningless.");
            System.out.println("SHUFFLING NAMES OF NET ELEMENTS...");
            Random randomGenerator = new Random();
            for (Node node : gspn.nodes) {
                for (int tentatives = 0; tentatives < 100; tentatives++) {
                    int k = randomGenerator.nextInt(gspn.nodes.size());
                    if (gspn.nodes.get(k).getClass() == node.getClass()) {
                        System.out.print("*");
                        String n = node.getUniqueName();
                        node.setUniqueName(gspn.nodes.get(k).getUniqueName());
                        gspn.nodes.get(k).setUniqueName(n);
                        break;
                    }
                }
            }
            System.out.println("");
        }
        // Anonimize names
        if (anonimize) {
            if (saveNameMap)
                System.out.println("WARNING: After anonimization the name map will be meaningless.");
            int P=0, T=0;
            for (Node node : gspn.nodes) {
                if (node instanceof Place)
                    node.setUniqueName("P"+(P++));
                else if (node instanceof Transition)
                    node.setUniqueName("T"+(T++));
            }
        }

        boolean saveUnfoldingRelation = false;
        if (saveAsPnml) {
            File pnmlNet = new File(baseName+".pnml");
            start = System.currentTimeMillis();
            System.out.println("SAVING AS "+baseName+".pnml ...");
            String ret = PNMLFormat.exportGspn(gspn, pnmlNet, savePnmlGfx);
            System.out.println("SAVING TIME: "+(System.currentTimeMillis() - start)/1000.0);
            if (ret != null) {
                System.out.println("Problems exporting the PNML file:\n"+ret);
                System.exit(1);
            }
            saveUnfoldingRelation = true;
        }
        if (saveAsPNPRO) { // Save in PNPRO format
            File pnproNet = new File(baseName+".PNPRO");
            start = System.currentTimeMillis();
            System.out.println("SAVING AS "+baseName+".PNPRO ...");
            ArrayList<ProjectPage> outPages = new ArrayList<>();
            outPages.add(gspn);
            ProjectData outProj = new ProjectData("project", outPages);
            PnProFormat.saveXML(outProj, pnproNet);
            System.out.println("SAVING TIME: "+(System.currentTimeMillis() - start)/1000.0);
//            if (ret != null) {
//                System.out.println("Problems exporting the PNPRO file:\n"+ret);
//                System.exit(1);
//            }
            saveUnfoldingRelation = true;
        }
        if (saveAsNetDef) { // Save in GreatSPN format
            // Save in GreatSPN net/def format
            File outNet = new File(baseName+".net");
            File outDef = new File(baseName+".def");
            start = System.currentTimeMillis();
            System.out.println("SAVING AS "+baseName+".(net/def) ...");
            String ret = GreatSpnFormat.exportGspn(gspn, outNet, outDef, true, true);
            System.out.println("SAVING TIME: "+(System.currentTimeMillis() - start)/1000.0);
            if (ret != null) {
                System.out.println("Problems exporting the net/def files:\n"+ret);
                System.exit(1);
            }

            // Save the id -> name map
            if (saveNameMap) {
                if (pnmlId2name!=null) {
                    System.out.println("SAVING NAME MAP FILE "+baseName+".id2name ...");
                    File outNameMap = new File(baseName+".id2name");
                    PrintWriter map = new UnixPrintWriter(new BufferedOutputStream(new FileOutputStream(outNameMap)));
                    map.println(pnmlId2name.size());
                    for (Map.Entry<String, String> e : pnmlId2name.entrySet()) {
                        map.println(e.getKey());                
                        map.println(e.getValue());                
                    }
                    map.close();
                }

                saveUnfoldingRelation = true;
            }


            // Save NuPN units
            if (rootUnit[0] != null) {
                Map<String, Integer> plc2Index = new HashMap<>();
                for (Node node : gspn.nodes) {
                    if (node instanceof Place)
                        plc2Index.put(node.getUniqueName(), plc2Index.size());
                }

                File outNameMap = new File(baseName+".nu");
                PrintWriter nu = new UnixPrintWriter(new BufferedOutputStream(new FileOutputStream(outNameMap)));
                System.out.println("SAVING NUPN UNITS AS FILE "+baseName+".nu ...");
                nu.println("NUPN units "+rootUnit[0].numUnits);
                saveNuPNUnit(nu, rootUnit[0], 0, pnmlId2name, plc2Index);
                nu.close();
            }
        }
        
        if (unfolded && saveUnfoldingRelation) {
            System.out.println("SAVING UNFOLDING MAP FILE "+baseName+".unfmap ...");
            File outUnfMapName = new File(baseName+".unfmap");
            saveUnfMap(outUnfMapName, unf);
        }


        System.out.println("TOTAL TIME: "+(System.currentTimeMillis() - totalStart)/1000.0);
        System.out.println("OK.");
    }
    
    
    private static void saveUnfMap(File f, Unfolding unf) throws IOException {
        PrintWriter unfmap = new UnixPrintWriter(new BufferedOutputStream(new FileOutputStream(f)));

        // Save unfolded places
        unfmap.println(unf.placeUnfolding.size());
        for (Map.Entry<Place, Map<DomainElement, Place>> e : unf.placeUnfolding.entrySet()) {
            unfmap.println(e.getKey().getUniqueName()+" "+e.getValue().size());
            for (Place p : e.getValue().values())
                unfmap.print(" "+p.getUniqueName());
            unfmap.println();
        }
        // Save unfolded transitions
        unfmap.println(unf.trnUnfolding.size());
        for (Map.Entry<Transition, List<Transition>> e : unf.trnUnfolding.entrySet()) {
            unfmap.println(e.getKey().getUniqueName()+" "+e.getValue().size());
            for (Transition p : e.getValue())
                unfmap.print(" "+p.getUniqueName());
            unfmap.println();
        }

        unfmap.close();
    }
    
//    private static void savePlaceUnfoldingAsNestedUnits(PrintWriter out, Unfolding unf, Map<String, Integer> plc2Index) {
//        Map<Place, List<Integer>> unfPlaces = new HashMap<>();
//        for (Map.Entry<Tuple<Place, DomainElement>, Place> e : unf.placeUnfolding.entrySet()) {
//            List<Integer> l = unfPlaces.get(e.getKey().x);
////            System.out.println("  "+e.getKey().x.getUniqueName()+"    "+e.getValue().getUniqueName());
//            if (l == null)
//                unfPlaces.put(e.getKey().x, l = new LinkedList<>());
//            l.add(plc2Index.get(e.getValue().getUniqueName()));
//        }
//        List<Integer> non_col = new LinkedList<>();
//        for (Node node : unf.gspn.nodes) {
//            if (node instanceof Place) {
//                Place plc = (Place)node;
//                if (plc.isInNeutralDomain())
//                    non_col.add(plc2Index.get(plc.getUniqueName()));
//            }
//        }
//        int count = 0;
//        out.println("NUPN units "+(unfPlaces.size() + 1));
//        // Root unit with non-colored places
//        out.print("  unit u0 subunits "+unfPlaces.size()+" places "+non_col.size()+"  ");
//        for (int p : non_col) {
//            out.print(p);
//            out.print(" ");
//        }
//        out.println();
//        // Nested units with colored places
//        for (Node node : unf.gspn.nodes) {
//            if (node instanceof Place) {
//                List<Integer> places = unfPlaces.get((Place)node);
//                out.print("  unit "+(count++)+" subunits 0 places "+places.size()+"  ");
//                Collections.sort(places);
//                for (int p : places) {
//                    out.print(p);
//                    out.print(" ");
//                }
//                out.println();
//            }
//        }
//    }
    
    private static void saveNuPNUnit(PrintWriter out, PNMLFormat.NuPNUnit unit, 
                                     int level, Map<String, String> pnmlId2name,
                                     Map<String, Integer> plc2Index) 
    {
        for (int l=0; l<level; l++) { out.print("  "); }
        out.print("unit "+unit.unitName+" subunits "+unit.subunits.size()+" places "+unit.places.size()+"  ");
        // Save places
        for (String place : unit.places)
            out.print(" "+plc2Index.get(pnmlId2name.get(place)));
        out.println();
        // Save Nested Units
        for (PNMLFormat.NuPNUnit subunit : unit.subunits)
            saveNuPNUnit(out, subunit, level+1, pnmlId2name, plc2Index);
    }
    

}
