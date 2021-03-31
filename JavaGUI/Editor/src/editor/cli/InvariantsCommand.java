/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.cli;

import editor.Main;
import editor.domain.elements.GspnPage;
import latex.DummyLatexProvider;
import static editor.cli.Common.*;
import editor.domain.grammar.TemplateBinding;
import editor.domain.semiflows.FlowsGenerator;
import editor.domain.semiflows.NetIndex;
import editor.domain.semiflows.StructuralAlgorithm;
import editor.domain.semiflows.PTFlows;


/**
 *
 * @author elvio
 */
public class InvariantsCommand {

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
        PTFlows.Type type = PTFlows.Type.PLACE_SEMIFLOWS;
        TemplateBinding bindings = new TemplateBinding();
        boolean verbose = false;
        int c;
        for (c=0; c<args.length; c++) {
//            System.out.println("args["+c+"]="+args[c]);
            if (!args[c].startsWith("-"))
                break; // end of options
            switch (args[c]) {
                case "-ps":
                    type = PTFlows.Type.PLACE_SEMIFLOWS;
                    break;
                
                case "-ts":
                    type = PTFlows.Type.TRANSITION_SEMIFLOWS;
                    break;
                    
                case "-pf":
                    type = PTFlows.Type.PLACE_FLOWS;
                    break;
                
                case "-tf":
                    type = PTFlows.Type.TRANSITION_FLOWS;
                    break;

                case "-pb":
                    type = PTFlows.Type.PLACE_BASIS;
                    break;
                
                case "-tb":
                    type = PTFlows.Type.TRANSITION_BASIS;
                    break;

                case "-traps":
                    type = PTFlows.Type.TRAPS;
                    break;

                case "-siphons":
                    type = PTFlows.Type.SIPHONS;
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
        // Input net
        if (c+1 != args.length) {
            throw new IllegalArgumentException("input net name is not correct.");
        }
        String inBaseName = args[c++];
        
        // Load input nets
        long loadStart = System.currentTimeMillis();
        GspnPage net = loadPage(inBaseName);
        printGspnStat(net, null);
        System.out.println("");
        System.out.println("LOADING TIME: "+(System.currentTimeMillis() - loadStart)/1000.0);
        System.out.println("");
        
        // Initialize object maps
        NetIndex netIndex = new NetIndex(net);
        
        // Initialize Farkas methos
        FlowsGenerator algo = FlowsGenerator.makeFor(type, netIndex);
        algo.initialize(type, bindings, netIndex);
        
        // Compute invariants
        StructuralAlgorithm.ProgressObserver obs = (int step, int total, int s, int t) -> { };
        algo.compute(verbose, obs);
        
        for (int i=0; i<algo.numFlows(); i++) {
            System.out.print("FLOW "+(i+1)+": ");
            System.out.println(algo.flowToString(i, netIndex, true, null));
        }
        System.out.println();
    }
}
