/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package editor.cli;

import editor.Main;
import static editor.cli.Common.loadPage;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.ParserContext;
import java.io.File;
import editor.domain.io.CppFormat;
import java.util.ArrayList;
import latex.DummyLatexProvider;

/**
 *
 * @author Irene
 */
public class CppCommand {

    public static void main(String[] args) throws Exception {
        
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
    
    
    public static boolean toolMain(String[] args) throws Exception{
        
        // Read command line arguments
        if (args.length < 1) {
            System.out.println("Not enough arguments.");
            System.exit(1);
        }
        
        
        // GREATSPN_BINDIR=~/tesiMagistrale/SOURCESC-/JavaGUI/Editor/dist
        // java -ea -cp ${GREATSPN_BINDIR}/Editor.jar:${GREATSPN_BINDIR}/lib/antlr-runtime-4.2.1.jar editor.cli.CppCommand EsempiExpMTDep out.cpp
        
        String inBaseName = args[0]; //immagino il nome del progetto greatspn
        String outBaseName = args[1]; //immagino il nome con cui sarà salvato
        
                
        GspnPage gspn = loadPage(inBaseName);
        ArrayList<ProjectPage> pages = new ArrayList<>();
        pages.add(gspn);
        ProjectData proj = new ProjectData("project", pages);
        gspn.preparePageCheck();
        gspn.checkPage(proj, null, gspn, null);
        ParserContext context = new ParserContext(gspn);
        gspn.compileParsedInfo(context);
           
       
        File cppFile = new File(outBaseName);

        
        CppFormat.export(cppFile, gspn, context);
        
        return true;
        
    }

}
