/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import static common.Util.UIscaleFactor;
import editor.domain.NetPage;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.VarMultiAssignment;
import editor.domain.grammar.VarListMultiAssignment;
import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.SwingConstants;
import latex.LatexFormula;

/**
 *
 * @author elvio
 */
public class VarMultiAssignmentListPanel extends javax.swing.JPanel {

    private VarListMultiAssignment varList;
    private VarMultiAssignmentForm[] allForms;
    private JLabel[] allLabels;
    
    public interface MultiAssignmentListListener {
        public void onNewListAssignment(VarListMultiAssignment vlist);
        public void onEditingListAssignment();
    }
    private MultiAssignmentListListener listener;
    
    /**
     * Creates new form TemplateBindingForm2
     */
    public VarMultiAssignmentListPanel() {
        initComponents();
        setBorder(BorderFactory.createMatteBorder(1, 1, 1, 1, Color.GRAY));
        setBackground(editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR);
        
        label_name.setForeground(editor.gui.net.NetEditorPanel.PAGE_TITLE_FOREGROUND);
        label_name.setBackground(editor.gui.net.NetEditorPanel.PAGE_TITLE_BACKGROUND);
        label_assignedValue.setForeground(editor.gui.net.NetEditorPanel.PAGE_TITLE_FOREGROUND);
        label_assignedValue.setBackground(editor.gui.net.NetEditorPanel.PAGE_TITLE_BACKGROUND);
        panel_horizBar.setBackground(editor.gui.net.NetEditorPanel.PAGE_TITLE_BACKGROUND);
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        
        for (VarMultiAssignmentForm aef : allForms)
            aef.setEnabled(enabled);
        setBackground(enabled ? editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR : 
                       VarMultiAssignmentForm.BKGND_DISABLED);
    }

    public void setMultiAssignmentListListener(MultiAssignmentListListener listener) {
        this.listener = listener;
    }
    
    static final int XDIST = 3, YDIST = 2;
    
    private GridBagConstraints getGBCNormal(int x, int y, int yDist) {
        return new GridBagConstraints(x, y, 1, 1, 0.0, 0.0, GridBagConstraints.EAST, 
                                         0, new Insets(yDist, XDIST, YDIST, XDIST), 0, 0);
    }
    private GridBagConstraints getGBCHorixFill(int x, int y, int yDist) {
        return new GridBagConstraints(x, y, 1, 1, 1.0, 0.0, GridBagConstraints.CENTER, 
                                      GridBagConstraints.HORIZONTAL, 
                                      new Insets(yDist, XDIST, YDIST, XDIST), 0, 0);
    }
    
    
    public void initializeFor(VarListMultiAssignment _varList, NetPage page, boolean allowMovements) {
        this.varList = (VarListMultiAssignment)Util.deepCopy(_varList);
        
        assert varList.assigns.isEmpty() || page != null;
        
        // Was initialized, but the number of variables changed. Reinitialize from scratch
        if (allForms != null && allForms.length != varList.assigns.size()) {
            clearForms();
        }
        
        if (allForms == null) {
            // Construct a new table
            int count = 0;
            allForms = new VarMultiAssignmentForm[varList.assigns.size()];
            allLabels = new JLabel[varList.assigns.size()];
            for (VarMultiAssignment vma : varList.assigns) {
                final int pos = count;
                TemplateVariable tvar = (TemplateVariable)page.getNodeByUniqueName(vma.varName);
                LatexFormula lf = tvar.getTemplateVariableLatexFormula();
                allLabels[pos] = new JLabel("", SwingConstants.RIGHT);
                allLabels[pos].setForeground(editor.gui.net.NetEditorPanel.PAGE_FOREGROUND_COLOR);
                allForms[pos] = new VarMultiAssignmentForm(vma.allowMultipleValues, allowMovements) {
                    @Override
                    public void onNewAssignment(VarMultiAssignment expr) {
                        VarListMultiAssignment newList = (VarListMultiAssignment)Util.deepCopy(varList);
                        for (int i=0; i<newList.assigns.size(); i++) {
                            if (newList.assigns.get(i).varName.equals(expr.varName)) {
                                newList.assigns.set(i, expr);
                                listener.onNewListAssignment(newList);
                                break;
                            }                            
                        }
                    }

                    @Override
                    public void onMoveUpDown(boolean isUp) {
                        VarListMultiAssignment newList = (VarListMultiAssignment)Util.deepCopy(varList);
                        VarMultiAssignment v = newList.assigns.get(pos);
                        newList.assigns.remove(pos);
                        newList.assigns.add(isUp ? pos-1 : pos+1, v);
                        listener.onNewListAssignment(newList);
                    }
                    
                    @Override
                    public void onEditingTextChanged() {
                        listener.onEditingListAssignment();
                    }
                };
                add(allLabels[count], getGBCNormal(0, count+3, 0));
                add(allForms[count], getGBCHorixFill(1, count+3, 0));
                count++;
            }
            setVisible(varList.assigns.size() > 0);
        }
        
        // (Re)Initialize the existing table
        assert allForms.length == varList.assigns.size();
        boolean allOk = true;
        for (int i=0; i<allForms.length; i++) {
            VarMultiAssignment vma = varList.assigns.get(i);
            TemplateVariable tvar = (TemplateVariable)page.getNodeByUniqueName(vma.varName);
            LatexFormula lf = tvar.getTemplateVariableLatexFormula();
            allLabels[i].setIcon(lf.getAsIcon(UIscaleFactor));
            allForms[i].initializeFor(vma, true, i, allForms.length, page);
            
            allOk = allOk && allForms[i].isBindingValid();
        }
    }
    
    public VarListMultiAssignment getCurrentAssignment() {
        VarListMultiAssignment currAssign = new VarListMultiAssignment();
        for (VarMultiAssignmentForm aef : allForms) {
            currAssign.assigns.add(aef.assignment);
        }
        return currAssign;
    }
    
    public void deinitialize() {
        clearForms();
        varList = null;
    }
            
    public void clearForms() {
        for (VarMultiAssignmentForm aef : allForms) {
            aef.deinitialize();
            remove(aef);
        }
        for (JLabel lab : allLabels)
            remove(lab);
        allForms = null;
        allLabels = null;
    }
    
//    private final EventListenerList myListenerList = new EventListenerList();
//    
//    public void addActionListener(ActionListener al) {
//        myListenerList.add(ActionListener.class, al);
//    }
//    
//    public void removeActionListener(ActionListener al) {
//        myListenerList.remove(ActionListener.class, al);
//    }
//    
//    public ActionListener[] getActionListeners() {
//        return myListenerList.getListeners(ActionListener.class);
//    }
//    
//    protected void fireAction(ActionEvent evt) {
//        ActionListener[] list = getActionListeners();
//        for (ActionListener al : list) {
//            al.actionPerformed(evt);
//        }
//    }
    
    public boolean areAllBindingsValid() {
        for (VarMultiAssignmentForm aef : allForms)
            if (!aef.isBindingValid())
                return false;
        return true;
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

        label_name = new javax.swing.JLabel();
        label_assignedValue = new javax.swing.JLabel();
        panel_horizBar = new javax.swing.JPanel();

        setBackground(new java.awt.Color(255, 255, 255));
        setLayout(new java.awt.GridBagLayout());

        label_name.setText("Name:");
        label_name.setOpaque(true);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 3;
        gridBagConstraints.ipady = 3;
        add(label_name, gridBagConstraints);

        label_assignedValue.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        label_assignedValue.setText("Assigned Value:");
        label_assignedValue.setOpaque(true);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 3;
        gridBagConstraints.ipady = 3;
        gridBagConstraints.weightx = 0.1;
        add(label_assignedValue, gridBagConstraints);

        panel_horizBar.setBorder(javax.swing.BorderFactory.createMatteBorder(1, 0, 0, 0, new java.awt.Color(0, 0, 0)));

        javax.swing.GroupLayout panel_horizBarLayout = new javax.swing.GroupLayout(panel_horizBar);
        panel_horizBar.setLayout(panel_horizBarLayout);
        panel_horizBarLayout.setHorizontalGroup(
            panel_horizBarLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 357, Short.MAX_VALUE)
        );
        panel_horizBarLayout.setVerticalGroup(
            panel_horizBarLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 0, Short.MAX_VALUE)
        );

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 2, 0);
        add(panel_horizBar, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel label_assignedValue;
    private javax.swing.JLabel label_name;
    private javax.swing.JPanel panel_horizBar;
    // End of variables declaration//GEN-END:variables
}
