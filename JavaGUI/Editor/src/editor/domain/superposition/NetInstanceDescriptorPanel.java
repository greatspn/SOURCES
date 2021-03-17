/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

import editor.domain.Expr;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.measures.ExprField;
import editor.domain.measures.SolverParams;
import editor.gui.MainWindowInterface;
import java.awt.Color;
import java.awt.Dimension;
import java.util.ArrayList;
import java.util.Map;

/**
 *
 * @author elvio
 */
public final class NetInstanceDescriptorPanel extends javax.swing.JPanel implements ExprField.ExprFieldListener {

    private MainWindowInterface mainInterface;
    private ProjectData currProject;
    private MultiNetPage mnPage;
    private final int numInst;
    
    private NetInstanceDescriptor descr;
    private final SolverParams.IntExpr numReplicaExprCopy;
    
    private final ArrayList<VarBindingPanel> bindPanels = new ArrayList<>();
    
    boolean initializing = true;
    
    public NetInstanceDescriptorPanel(int number) {
        initComponents();
        this.numInst = number;
        numReplicaExprCopy = new SolverParams.IntExpr("1");
        label_Num.setText((numInst+1)+"º");
        exprField_replicas.setExprListener(this);
        initializing = false;

        button_up.setEnabled(numInst != 0);
        label_netName.setVisible(numInst == 0);
        label_numReplicas.setVisible(numInst == 0);
        panel_horizBar.setVisible(numInst == 0);

        // Fix combo box size
        Dimension comboSize = comboBox_model.getPreferredSize();
        comboSize.width = 12 * comboSize.height;
        comboBox_model.setPreferredSize(comboSize);
        comboBox_model.setMinimumSize(comboSize);
        comboBox_model.setMaximumSize(comboSize);        
    }
    
    public void initialize(MainWindowInterface mainInterface, ProjectData currProject, 
                           MultiNetPage mnPage) 
    {
        this.mainInterface = mainInterface;
        this.currProject = currProject;
        this.mnPage = mnPage;
        initializing = true;
        
        descr = mnPage.netsDescr.get(numInst);
        
        button_down.setEnabled(numInst != (mnPage.netsDescr.size() - 1));
        
        int numBoundVars = descr.getListOfBoundVars().binding.size();
        int numVars = numBoundVars;// + descr.getListOfUnboundVars().size();
        panel_binding.setVisible(numBoundVars > 0);
//        button_bindParam.setEnabled(numVars > 0);
        
        // Net selection combo box
        comboBox_model.removeAllItems();
        boolean found = mnPage.canComposeWith(currProject.findPageByName(descr.targetNetName));
        comboBox_model.setForeground(found ? Color.BLACK : Color.red);
        if (!found) {
            comboBox_model.addItem(descr.targetNetName+" <missing>");
            comboBox_model.setSelectedIndex(0);
        }
        for (int p=0; p<currProject.getPageCount(); p++) {
            ProjectPage page = currProject.getPageAt(p);
            if (mnPage.canComposeWith(page)) {
                comboBox_model.addItem(page.getPageName());
            }
        }
        if (found)
            comboBox_model.setSelectedItem(descr.targetNetName);
        
        // Selection
        checkBox_selected.setSelected(descr.isSelected());
        
        // Replica count
        numReplicaExprCopy.setExpr(descr.numReplicas.getExpr());
        exprField_replicas.initializeFor(numReplicaExprCopy.getEditableValue(), mnPage);
        exprField_replicas.setEnabled(mnPage.useReplicaCount());
        
        // Bound variables
        for (VarBindingPanel vbp : bindPanels) {
            vbp.deinitialize();
        }
        bindPanels.clear();
        panel_bindVars.removeAll();
        
        for (Map.Entry<String, Expr> bind : descr.instParams.binding.entrySet()) {
            if (descr.paramRefs.containsKey(bind.getKey())) {
//        for (String varName : descr.paramRefs.keySet()) {
//            if (descr.instParams.binding.containsKey(varName)) {
                VarBindingPanel vbp = new VarBindingPanel(descr.paramRefs.get(bind.getKey()), 
                                                          descr, numInst, mnPage, mainInterface);
                bindPanels.add(vbp);
                panel_bindVars.add(vbp);
            }
        }
        
        initializing = false;
    }
    
    public void deinitialize() {
        exprField_replicas.deinitialize();
        this.mainInterface = null;
        this.currProject = null;
        this.mnPage = null;
        this.descr = null;
        
        for (VarBindingPanel vbp : bindPanels)
            vbp.deinitialize();
        bindPanels.clear();
        panel_bindVars.removeAll();
    }

    @Override
    public void onExprModified() {
        mainInterface.executeUndoableCommand("change replica count.", (ProjectData proj, ProjectPage page) -> {
            descr.numReplicas.setExpr(numReplicaExprCopy.getExpr());
        });
    }

    @Override
    public void onEditingText() {
        // Editing text in the replica's expression box
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
        label_Num = new javax.swing.JLabel();
        checkBox_selected = new javax.swing.JCheckBox();
        comboBox_model = new javax.swing.JComboBox<>();
        exprField_replicas = new editor.domain.measures.ExprField();
        button_up = new javax.swing.JButton();
        button_down = new javax.swing.JButton();
        panel_binding = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        panel_bindVars = new javax.swing.JPanel();
        panel_horizBar = new javax.swing.JPanel();
        label_netName = new javax.swing.JLabel();
        label_numReplicas = new javax.swing.JLabel();

        setBackground(new java.awt.Color(255, 255, 255));
        setLayout(new java.awt.GridBagLayout());

        label_Num.setText("N");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 5, 0, 0);
        add(label_Num, gridBagConstraints);

        checkBox_selected.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_selectedActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridy = 2;
        add(checkBox_selected, gridBagConstraints);

        comboBox_model.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_modelActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        add(comboBox_model, gridBagConstraints);

        exprField_replicas.setBackground(new java.awt.Color(255, 255, 255));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.5;
        gridBagConstraints.weighty = 0.3;
        gridBagConstraints.insets = new java.awt.Insets(0, 8, 0, 0);
        add(exprField_replicas, gridBagConstraints);

        button_up.setIcon(resourceFactory.getArrowUp16());
        button_up.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_upActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridy = 2;
        add(button_up, gridBagConstraints);

        button_down.setIcon(resourceFactory.getArrowDown16());
        button_down.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_downActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 5);
        add(button_down, gridBagConstraints);

        panel_binding.setBackground(new java.awt.Color(255, 255, 255));
        panel_binding.setLayout(new java.awt.GridBagLayout());

        jLabel1.setText("Bind parameter values:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        panel_binding.add(jLabel1, gridBagConstraints);

        panel_bindVars.setBackground(new java.awt.Color(255, 255, 255));
        panel_bindVars.setLayout(new javax.swing.BoxLayout(panel_bindVars, javax.swing.BoxLayout.LINE_AXIS));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 0.1;
        panel_binding.add(panel_bindVars, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        add(panel_binding, gridBagConstraints);

        panel_horizBar.setBorder(javax.swing.BorderFactory.createMatteBorder(1, 0, 0, 0, new java.awt.Color(0, 0, 0)));

        javax.swing.GroupLayout panel_horizBarLayout = new javax.swing.GroupLayout(panel_horizBar);
        panel_horizBar.setLayout(panel_horizBarLayout);
        panel_horizBarLayout.setHorizontalGroup(
            panel_horizBarLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 434, Short.MAX_VALUE)
        );
        panel_horizBarLayout.setVerticalGroup(
            panel_horizBarLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 0, Short.MAX_VALUE)
        );

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 7;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(2, 0, 2, 0);
        add(panel_horizBar, gridBagConstraints);

        label_netName.setText("Net:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        add(label_netName, gridBagConstraints);

        label_numReplicas.setText("Number of Replicas:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weighty = 0.3;
        gridBagConstraints.insets = new java.awt.Insets(0, 8, 0, 0);
        add(label_numReplicas, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents

    private void comboBox_modelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_modelActionPerformed
        if (initializing)
            return;
        final String newNetName = (String)comboBox_model.getSelectedItem();
        if (newNetName.equals(descr.targetNetName))
            return;
        mainInterface.executeUndoableCommand("change net reference.", (ProjectData proj, ProjectPage page) -> {
            descr.targetNetName = newNetName;
        });
    }//GEN-LAST:event_comboBox_modelActionPerformed

    private void checkBox_selectedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_selectedActionPerformed
        descr.setSelected(checkBox_selected.isSelected());
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_checkBox_selectedActionPerformed

    private void button_upActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_upActionPerformed
        mainInterface.executeUndoableCommand("move descriptor up.", (ProjectData proj, ProjectPage page) -> {
            MultiNetPage mnp = (MultiNetPage)page;
            NetInstanceDescriptor nd = mnp.netsDescr.get(numInst);
            mnp.netsDescr.set(numInst, mnp.netsDescr.get(numInst-1));
            mnp.netsDescr.set(numInst-1, nd);
        });
    }//GEN-LAST:event_button_upActionPerformed

    private void button_downActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_downActionPerformed
        mainInterface.executeUndoableCommand("move descriptor down.", (ProjectData proj, ProjectPage page) -> {
            MultiNetPage mnp = (MultiNetPage)page;
            NetInstanceDescriptor nd = mnp.netsDescr.get(numInst);
            mnp.netsDescr.set(numInst, mnp.netsDescr.get(numInst+1));
            mnp.netsDescr.set(numInst+1, nd);
        });
    }//GEN-LAST:event_button_downActionPerformed

//    private void button_bindParamActionPerformed(java.awt.event.ActionEvent evt) {                                                 
//        JPopupMenu popup = new JPopupMenu();
//        // Add binding commands
//        for (final String var : descr.unboundParams) {
//            final TemplateVariable tvar = descr.paramRefs.get(var);
//            JMenuItem bindCmd = new JMenuItem("Bind parameter "+var);
//            bindCmd.addActionListener(new ActionListener() {
//                @Override
//                public void actionPerformed(ActionEvent e) {
//                    mainInterface.executeUndoableCommand("bind parameter "+var, (ProjectData proj, ProjectPage page) -> {
//                        NetInstanceDescriptor d = ((MultiNetPage)page).netsDescr.get(numInst);
//                        d.unboundParams.remove(var);
//                        d.instParams.bindSingleValue(var, tvar.getLastBindingExpr());
//                    });
//                }
//            });
//            popup.add(bindCmd);
//        }
//        // Add unbinding commands
//        for (final String var : descr.instParams.binding.keySet()) {
//            final TemplateVariable tvar = descr.paramRefs.get(var);
//            JMenuItem unbindCmd = new JMenuItem("Unbind parameter "+var);
//            unbindCmd.addActionListener(new ActionListener() {
//                @Override
//                public void actionPerformed(ActionEvent e) {
//                    mainInterface.executeUndoableCommand("unbind parameter "+var, (ProjectData proj, ProjectPage page) -> {
//                        NetInstanceDescriptor d = ((MultiNetPage)page).netsDescr.get(numInst);
//                        d.instParams.unbind(var);
//                        d.unboundParams.add(var);
//                    });
//                }
//            });
//            popup.add(unbindCmd);
//        }
//        // Show the popup
//        popup.show(button_bindParam, 0, button_bindParam.getHeight());
//    }                                                

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton button_down;
    private javax.swing.JButton button_up;
    private javax.swing.JCheckBox checkBox_selected;
    private javax.swing.JComboBox<String> comboBox_model;
    private editor.domain.measures.ExprField exprField_replicas;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel label_Num;
    private javax.swing.JLabel label_netName;
    private javax.swing.JLabel label_numReplicas;
    private javax.swing.JPanel panel_bindVars;
    private javax.swing.JPanel panel_binding;
    private javax.swing.JPanel panel_horizBar;
    private editor.gui.ResourceFactory resourceFactory;
    // End of variables declaration//GEN-END:variables
}
