/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.UnixPrintWriter;
import editor.domain.Edge;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.measures.SolverInvokator;
import static editor.domain.measures.SolverInvokator.cmdToString;
import static editor.domain.measures.SolverInvokator.startOfCommand;
import editor.gui.MainWindowInterface;
import java.awt.Dimension;
import java.awt.geom.Point2D;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

/**
 *
 * @author elvio
 */
public class NetRelayoutWindow extends javax.swing.JDialog {
    
    MainWindowInterface mainInterface;
    NetPage net;
    Thread thread;
    Process process = null;

    ArrayList<Point2D> nodePos = null;
    ArrayList<ArrayList<Point2D>> edgeLines = null;
    String errorMsg = null;

    /**
     * Creates new form NetRelayoutWindow
     */
    public NetRelayoutWindow(JFrame parent, MainWindowInterface mainInterface, NetPage net) {
        super(parent, true);
        this.mainInterface = mainInterface;
        this.net = net;
        initComponents();
        
        DefaultComboBoxModel<String> model = (DefaultComboBoxModel<String>)comboBox_layout.getModel();
        model.addElement("Basic Planarization");
        model.addElement("Grid Planarization");
        model.addElement("Straight Planarization");
//        model.addElement("Draw Planarization");
        model.addElement("FMMM");
        model.addElement("FMME");
//        model.addElement("FPP");
//        model.addElement("GEM");
//        model.addElement("Spring Layout");
//        model.addElement("SpringKK Layout");
//        model.addElement("Sugiyama");
        model.addElement("Visibility");
        model.addElement("Dominance");
        model.addElement("Balloon");
        model.addElement("Circular");
        model.addElement("Davidson-Harel");

//        model.addElement("Schnyder");
//        model.addElement("Tree Layout");
//        model.addElement("Orthogonal Tree Layout");
//        model.addElement("Upward Planarization");

        model.setSelectedItem(model.getElementAt(0));
        
        button_stop.setEnabled(false);
        
        Dimension dim = comboBox_layout.getPreferredSize();
        dim.width = Math.max(dim.width, NetObject.getUnitToPixels()*40);
        comboBox_layout.setMinimumSize(dim);
        
        pack();
        setLocationRelativeTo(parent);
        getRootPane().setDefaultButton(button_start);
    }
    
    public boolean startRelayout(String layoutAlgo) {
        errorMsg = null;
        File outFile=null, inFile=null;
        try {
            outFile = File.createTempFile("gml", "");
            inFile = File.createTempFile("coords", "");
            PrintWriter outWriter = new UnixPrintWriter(new BufferedWriter(new FileWriter(outFile)));
            outWriter.append("graph [\n");
            Map<Node, Integer> node2id = new HashMap<>();
            for (Node node : net.nodes) {
                int id = node2id.size();
                node2id.put(node, id);
                outWriter.append(" node [ id "+id+" w 2 h 2 ]\n");
            }
            for (Edge edge : net.edges) {
                outWriter.append(" edge [ source "+node2id.get(edge.getHeadNode())+
                                 " target "+node2id.get(edge.getTailNode())+" ]\n");
            }
            outWriter.append("]\n");
            outWriter.close();
            System.out.println("outFile = "+outFile.getAbsolutePath());
            System.out.println("inFile  = "+inFile.getAbsolutePath());

            ArrayList<String> cmd = startOfCommand();
            cmd.add(SolverInvokator.useGreatSPN_binary("ogdf"));
            cmd.add(outFile.getAbsolutePath());
            cmd.add(inFile.getAbsolutePath());
            cmd.add(layoutAlgo);
            String[] envp = SolverInvokator.prepareRuntimeEnvironmentVars();
            System.out.println("cmd = " + cmdToString(cmd));
            process = Runtime.getRuntime().exec(cmd.toArray(new String[cmd.size()]), envp);
            int retVal = process.waitFor();
            process = null;
            if (retVal != 0) {
                errorMsg = "ogdf returned an exit code of "+retVal;
                return false;
            }

            // Read back the coordinates
            Scanner sc = new Scanner(inFile);
            sc.useDelimiter(",|\\s+");
//            final double SCALE = 10.0;
            nodePos = new ArrayList<>();
            edgeLines = new ArrayList<>();
            
            for (Node node : net.nodes) {
                nodePos.add(new Point2D.Double(scanDouble(sc), scanDouble(sc)));
                System.out.println(nodePos.get(nodePos.size()-1).getX()+" "+
                                   nodePos.get(nodePos.size()-1).getY());
            }
            for (Edge edge : net.edges) {
                int src = sc.nextInt();
                int tgt = sc.nextInt();
                int numPoints = sc.nextInt();

                ArrayList<Point2D> points = new ArrayList<>();
                for (int i=0; i<numPoints; i++)
                    points.add(new Point2D.Double(scanDouble(sc), scanDouble(sc)));
                
                edgeLines.add(points);
            }
            sc.close();
            return true;
        }
        catch (Exception e) {
            e.printStackTrace();
            errorMsg = e.getMessage();
        }
        finally {
//            if (outFile != null)
//                outFile.delete();
            if (inFile != null)
                inFile.delete();
        }
        return false;
    }
    
    public void completeRelayout() {
        try { thread.join(); }
        catch (InterruptedException e) { }
        thread = null;
        
        if (errorMsg != null) {
            JOptionPane.showMessageDialog(this, 
                              "An error happened during layout operation.\n"
                              + "Reason: "+errorMsg,
                              "Export \""+net.getPageName()+"\" in PNG format...", 
                              JOptionPane.ERROR_MESSAGE);            
//            mainInterface.setStatus("could not re-layout the net.", true);
            
            button_start.setEnabled(true);
            button_stop.setEnabled(false);
            button_close.setEnabled(true);
            progressBar.setIndeterminate(false);
        }
        else {
            // apply the layout
            mainInterface.executeUndoableCommand("net re-layout", (ProjectData proj, ProjectPage page) -> {
                for (int i=0; i<net.nodes.size(); i++)
                    net.nodes.get(i).setNodeCenterPosition(nodePos.get(i).getX(), nodePos.get(i).getY());

                for (int i=0; i<net.edges.size(); i++) {
                    Edge edge = net.edges.get(i);
                    ArrayList<Point2D> line = edgeLines.get(i);

                    edge.points.clear();
                    edge.points.add(new Point2D.Double(edge.getHeadNode().getCenterX(),
                                                       edge.getHeadNode().getCenterY()));

                    for (int k=0; k<line.size(); k++)
                        edge.points.add(line.get(line.size() - k - 1));

                    edge.points.add(new Point2D.Double(edge.getTailNode().getCenterX(),
                                                       edge.getTailNode().getCenterY()));
                    // Reconnect the edge to invalidate the precomputed path and clear the magnets
                    edge.isBroken = false;
                    edge.connectToNode(edge.getHeadNode(), -1, Edge.EndPoint.HEAD);
                    edge.connectToNode(edge.getTailNode(), -1, Edge.EndPoint.TAIL);
                }
            });

            setVisible(false);
        }
    }
    
    private static double scanDouble(Scanner sc) {
        String next = sc.next(); // needed to read negative signs
//        System.out.println("scanDouble "+next);
        return Double.parseDouble(next);
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

        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        comboBox_layout = new javax.swing.JComboBox<>();
        progressBar = new javax.swing.JProgressBar();
        jPanel2 = new javax.swing.JPanel();
        button_start = new javax.swing.JButton();
        button_stop = new javax.swing.JButton();
        button_close = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Layout Algorithm Selection"));
        jPanel1.setLayout(new java.awt.GridBagLayout());

        jLabel1.setText("Layout: ");
        jPanel1.add(jLabel1, new java.awt.GridBagConstraints());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        jPanel1.add(comboBox_layout, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(11, 0, 11, 0);
        jPanel1.add(progressBar, gridBagConstraints);

        getContentPane().add(jPanel1, java.awt.BorderLayout.CENTER);

        jPanel2.setLayout(new java.awt.GridBagLayout());

        button_start.setText("Start!");
        button_start.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_startActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.ipadx = 10;
        gridBagConstraints.insets = new java.awt.Insets(8, 8, 8, 8);
        jPanel2.add(button_start, gridBagConstraints);

        button_stop.setText("Stop");
        button_stop.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_stopActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.ipadx = 10;
        gridBagConstraints.insets = new java.awt.Insets(8, 8, 8, 8);
        jPanel2.add(button_stop, gridBagConstraints);

        button_close.setText("Close");
        button_close.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_closeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.ipadx = 10;
        gridBagConstraints.insets = new java.awt.Insets(8, 8, 8, 8);
        jPanel2.add(button_close, gridBagConstraints);

        getContentPane().add(jPanel2, java.awt.BorderLayout.SOUTH);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void button_startActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_startActionPerformed
        final String layoutAlgo = (String)comboBox_layout.getSelectedItem();
        thread = new Thread(() -> {
            if (startRelayout(layoutAlgo)) {
                SwingUtilities.invokeLater(() -> {
                    completeRelayout();
                });
            }
        });
        thread.start();
        
        button_start.setEnabled(false);
        button_stop.setEnabled(true);
        button_close.setEnabled(false);
        progressBar.setIndeterminate(true);
    }//GEN-LAST:event_button_startActionPerformed

    private void button_stopActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_stopActionPerformed
        if (process != null) {
            process.destroy();
            process = null;
        }
        thread.interrupt();
        errorMsg = "Interrupted.";
        
        button_start.setEnabled(true);
        button_stop.setEnabled(false);
        button_close.setEnabled(true);
        progressBar.setIndeterminate(false);

        //setVisible(false);
    }//GEN-LAST:event_button_stopActionPerformed

    private void button_closeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_closeActionPerformed
        setVisible(false);
    }//GEN-LAST:event_button_closeActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton button_close;
    private javax.swing.JButton button_start;
    private javax.swing.JButton button_stop;
    private javax.swing.JComboBox<String> comboBox_layout;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JProgressBar progressBar;
    // End of variables declaration//GEN-END:variables
}
