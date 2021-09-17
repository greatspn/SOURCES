/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.cli;

import editor.Main;
import static editor.cli.Common.loadPage;
import static editor.cli.Common.printGspnStat;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.ParserContext;
import editor.domain.io.NetLogoFormat;
import java.io.File;
import java.util.ArrayList;
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

            boolean status = toolMain(args);

            System.out.println("TOTAL TIME: "+(System.currentTimeMillis() - totalStart)/1000.0);
            System.out.println(status ? "OK." : "FAILED.");
        }
        catch (Throwable e) {
            System.out.println("EXCEPTION RAISED.");
            e.printStackTrace();
            System.out.println("FAILED.");
            System.exit(1);            
        }
    }
    // GREATSPN_BINDIR=~/GreatSPN/SOURCES/JavaGUI/Editor/dist/
    // java -ea -cp ${GREATSPN_BINDIR}/Editor.jar:${GREATSPN_BINDIR}/lib/antlr-runtime-4.2.1.jar 
    //   editor.cli.NetLogoCommand -ac Sheep,Wolf SheepWolfModif2 out.netlogo && cat out.netlogo
    
    public static boolean toolMain(String[] args) throws Exception {
        // Read command line arguments
        if (args.length < 1) {
            System.out.println("Not enough arguments.");
            System.exit(1);
        }
        String agentColors = "";
        boolean verbose = false;
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
                    
                case "-v":
                    verbose = true;
                    break;

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
        String outBaseName = args[c++];
        
//        System.out.println("inBaseName = " + inBaseName);
//        System.out.println("outBaseName = " + outBaseName);
        
        // Load input nets
        long loadStart = System.currentTimeMillis();
        GspnPage gspn = loadPage(inBaseName);
        printGspnStat(gspn, null);
        System.out.println("");
        System.out.println("LOADING TIME: "+(System.currentTimeMillis() - loadStart)/1000.0);
        System.out.println("");
        
        
        // GSPN semantic check
        ArrayList<ProjectPage> pages = new ArrayList<>();
        pages.add(gspn);
        ProjectData proj = new ProjectData("project", pages);
        gspn.preparePageCheck();
        gspn.checkPage(proj, null, gspn, null);
        
        ParserContext context = new ParserContext(gspn);
        gspn.compileParsedInfo(context);
        if (!gspn.isPageCorrect()) {
            for (int err=0; err < gspn.getNumErrorsAndWarnings(); err++) {
                PageErrorWarning pew = gspn.getErrorWarning(err);
                System.out.println(" "+pew.getDescription());
            }
            System.out.println("Found errors in loaded file. Stop.");
            System.exit(1);
        }
        
        String[] agentsColorList = agentColors.split(",");
        
        
        // Export in NetLogo format
        long saveStart = System.currentTimeMillis();
        System.out.println("");
        File netLogoFile = new File(outBaseName);
        String log = NetLogoFormat.export(gspn, netLogoFile, agentsColorList, context, verbose);
        
        if (log != null) {        
            System.out.println(log);
            netLogoFile.delete();
            return false;
        }
        else {
            System.out.println("SAVING TIME: "+(System.currentTimeMillis() - saveStart)/1000.0);
            return true;
        }
    }
}
