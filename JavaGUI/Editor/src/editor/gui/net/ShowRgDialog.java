/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.Main;
import editor.domain.Expr;
import editor.domain.ProjectPage.RgType;
import static editor.domain.ProjectPage.RgType.CTMC;
import static editor.domain.ProjectPage.RgType.RG;
import static editor.domain.ProjectPage.RgType.SRG;
import editor.domain.elements.GspnPage;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.GreatSpnFormat;
import editor.domain.measures.SolverInvokator;
import static editor.domain.measures.SolverInvokator.cmdToString;
import static editor.domain.measures.SolverInvokator.startOfCommand;
import static editor.domain.measures.SolverInvokator.useGreatSPN_binary;
import java.awt.Cursor;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;
import javax.swing.SwingUtilities;
import static editor.domain.measures.SolverInvokator.makeFilenameForCmd;
import static editor.domain.measures.SolverInvokator.makeFilenameForCmd;
import static editor.domain.measures.SolverInvokator.makeFilenameForCmd;

/**
 *
 * @author elvio
 */
public class ShowRgDialog extends javax.swing.JDialog {
    
    String result = null;
    GspnPage gspn;
    TemplateBinding binding;
    RgType rgType;
    
    /**
     * Creates new form ShowRgDialog
     */
    public ShowRgDialog(java.awt.Frame parent, GspnPage gspn, TemplateBinding binding, RgType rgType) {
        super(parent, true);
        this.gspn = gspn;
        this.binding = binding;
        this.rgType = rgType;
        initComponents();
        
        switch (rgType) {
            case SRG:
                label.setIcon(resourceFactory.getBuildSymRG32());
                label.setText("Building the Symbolic Reachability Graph...");
                break;
                
            case RG:
                label.setIcon(resourceFactory.getBuildRG32());
                label.setText("Building the Reachability Graph...");
                break;
                
            case CTMC:
                label.setIcon(resourceFactory.getBuildCTMC32());
                label.setText("Building the Continuous Time Markov Chain...");
                break;
        }
        
        pack();
        setLocationRelativeTo(parent);
    }
    
    public String showRG() {
        new Thread() {
            @Override
            public void run() {
                result = execRG();
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        setVisible(false);
                    }
                });
            }
        }.start();
        this.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
        setVisible(true);
        this.setCursor(Cursor.getDefaultCursor());
        return result;
    }
    
    private String execRG() {
        try {        
            File tmpRoot = File.createTempFile("petrinet", "");
            File tmpNet = new File(tmpRoot.getAbsolutePath() + ".net");
            File tmpDef = new File(tmpRoot.getAbsolutePath() + ".def");
            File tmpPdf = new File(tmpRoot.getAbsolutePath() + ".pdf");
            
            GreatSpnFormat.exportGspn(gspn, tmpNet, tmpDef, true);

            ArrayList<String> cmd = startOfCommand();
            cmd.add(useGreatSPN_binary(rgType==RgType.SRG ? "WNSRG" : "WNRG"));
            cmd.add(makeFilenameForCmd(tmpRoot));
            cmd.add("-max-markings");
            cmd.add("10000");
            cmd.add("-max-dot-markings");
            cmd.add("80");
            cmd.add("-dot-F");
            cmd.add(makeFilenameForCmd(tmpRoot));
            if (rgType == RgType.CTMC)
                cmd.add("-m");
            
            for (Map.Entry<String, Expr> e : binding.binding.entrySet()) {
                TemplateVariable tvar = (TemplateVariable)gspn.getNodeByUniqueName(e.getKey());
                if (tvar.getType() == TemplateVariable.Type.INTEGER) {
                    cmd.add("-mpar");
                    cmd.add(tvar.getUniqueName());
                    cmd.add(e.getValue().getExpr());
                }
                else if (tvar.getType() == TemplateVariable.Type.REAL) {
                    cmd.add("-rpar");
                    cmd.add(tvar.getUniqueName());
                    cmd.add(e.getValue().getExpr());
                }
            }
            
            System.out.println("cmd = " + cmdToString(cmd));
            String[] envp = SolverInvokator.prepareRuntimeEnvironmentVars();
            Runtime.getRuntime().exec(cmd.toArray(new String[cmd.size()]), envp).waitFor();
            
            if (tmpPdf.exists()) {
                Main.viewPDF(tmpPdf);
            }
            else {
                return "Could not generate the RG file.";
            }
        }
        catch (IOException e) {
            return "Could not allocate temporary files.\n"+e.getMessage();
        }
        catch (Exception e) {
            return "Could not save GSPN file.\n"+e.getMessage();
        }
        return null;
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        resourceFactory = new editor.gui.ResourceFactory();
        label = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        getContentPane().setLayout(new java.awt.GridBagLayout());

        label.setText("Building the XXX Graph...");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(30, 60, 30, 60);
        getContentPane().add(label, gridBagConstraints);

        pack();
    }// </editor-fold>//GEN-END:initComponents



    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel label;
    private editor.gui.ResourceFactory resourceFactory;
    // End of variables declaration//GEN-END:variables
}
