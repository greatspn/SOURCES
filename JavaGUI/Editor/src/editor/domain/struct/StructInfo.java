/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.struct;

import common.Tuple;
import editor.Main;
import editor.domain.Expr;
import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.GreatSpnFormat;
import editor.domain.measures.SolverInvokator;
import static editor.domain.measures.SolverInvokator.cmdToString;
import static editor.domain.measures.SolverInvokator.startOfCommand;
import editor.domain.values.EvaluatedFormula;
import java.awt.Window;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;
import javax.swing.JOptionPane;
import static editor.domain.measures.SolverInvokator.makeFilenameForCmd;
import static editor.domain.measures.SolverInvokator.makeFilenameForCmd;
import static editor.domain.measures.SolverInvokator.makeFilenameForCmd;

/** Structural informations of a PN.
 * Actually an interface that calls the 'struct' tool of GreatSPN 
 * and reads back the results
 *
 * @author elvio
 */
public class StructInfo {
    
    // Structural bounds (min, max value) of places
    public Map<String, Tuple<Integer, Integer>> boundsOfPlace;
    
    
    private void readPlaceBounds(GspnPage gspn, File bndFile) throws IOException {
        Scanner bnd = new Scanner(bndFile);
        boundsOfPlace = new HashMap<>();
        
        // read bounds of places
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                int min = bnd.nextInt();
                int max = bnd.nextInt();
                boundsOfPlace.put(node.getUniqueName(), new Tuple<>(min, max));
//                System.out.println("Bound of "+node.getUniqueName()+": "+min+" "+max);
            }
        }
        bnd.close();
    }
    
    
    public static StructInfo computeStructInfo(Window wnd, GspnPage gspn, 
                                                TemplateBinding binding,
                                                ParserContext context) 
    {
        StructInfo info = null;
        File tmpName = null;
        
        try {
            // Additional -mpar switches, which are in common to both pinvar and struct
            ArrayList<String> mpars = new ArrayList<>();
            if (binding != null) {
                for (Map.Entry<String, Expr> entry : binding.binding.entrySet()) {
                    String name = entry.getKey();
                    Node node = gspn.getNodeByUniqueName(name);
                    if (node != null && node instanceof TemplateVariable) {
                        TemplateVariable tvar = (TemplateVariable)node;
                        if (tvar.getEvaluationType() == EvaluatedFormula.Type.INT) {
                            int value = entry.getValue().evaluate(context, EvaluationArguments.NO_ARGS).getScalarInt();

                            mpars.add("-mpar");
                            mpars.add(name);
                            mpars.add(""+value);
                        }
                    }
                }
            }
            
            ArrayList<String> cmd = startOfCommand();
            cmd.add(SolverInvokator.useGreatSPN_binary("pinvar"));
            
            // Save the net to a temporary file
            tmpName = File.createTempFile("net", "");
            File tmpNet = new File(tmpName.getAbsolutePath()+".net");
            File tmpDef = new File(tmpName.getAbsolutePath()+".def");
            GreatSpnFormat.exportGspn(gspn, tmpNet, tmpDef, true);
            cmd.add(makeFilenameForCmd(tmpName));
            cmd.addAll(mpars);
            
            // Run the pinvar command
            String[] envp = SolverInvokator.prepareRuntimeEnvironmentVars();
            System.out.println("cmd = " + cmdToString(cmd));
            Process pr = Runtime.getRuntime().exec(cmd.toArray(new String[cmd.size()]), envp);
            int retVal = pr.waitFor();
            if (retVal != 0)
                throw new IllegalStateException("pinvar returned an exit code of "+retVal);
            
            // Run the struct tool
            cmd = startOfCommand();
            cmd.add(SolverInvokator.useGreatSPN_binary("struct"));
            cmd.add(makeFilenameForCmd(tmpName));
            cmd.addAll(mpars);
            System.out.println("cmd = " + cmdToString(cmd));
            pr = Runtime.getRuntime().exec(cmd.toArray(new String[cmd.size()]), envp);
            retVal = pr.waitFor();
            if (retVal != 0)
                throw new IllegalStateException("struct returned an exit code of "+retVal);
            
            // Now we may read back the files we need, before destroying the
            // temporary files.
            info = new StructInfo();
            info.readPlaceBounds(gspn, new File(tmpName.getAbsolutePath()+".bnd"));
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(wnd, 
                    "Could not complete the operation:\n"+e.getMessage(), 
                    "Struct error", JOptionPane.ERROR_MESSAGE);
            Main.logException(e, true);
        }
        finally {
            if (tmpName != null) {
                // Clear intermediate temporary files
                new File(tmpName.getAbsolutePath()+".bnd").delete();
                new File(tmpName.getAbsolutePath()+".cc").delete();
                new File(tmpName.getAbsolutePath()+".def").delete();
                new File(tmpName.getAbsolutePath()+".ecs").delete();
                new File(tmpName.getAbsolutePath()+".me").delete();
                new File(tmpName.getAbsolutePath()+".net").delete();
                new File(tmpName.getAbsolutePath()+".pin").delete();
                new File(tmpName.getAbsolutePath()+".sc").delete();
                new File(tmpName.getAbsolutePath()+".sub").delete();
                new File(tmpName.getAbsolutePath()+".tin").delete();
                tmpName.delete();
            }
        }
                
        return info;
    }
}
