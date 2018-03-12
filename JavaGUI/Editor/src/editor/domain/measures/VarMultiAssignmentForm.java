/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import editor.domain.NetPage;
import editor.domain.grammar.VarMultiAssignment;
import java.awt.Color;
import java.awt.Insets;

/**
 *
 * @author elvio
 */
public abstract class VarMultiAssignmentForm extends javax.swing.JPanel 
            implements ExprField.ExprFieldListener 
{
    
    public static final Color BKGND_DISABLED = new Color(240, 240, 240);
    
    VarMultiAssignment assignment;
    int numVar, totalVars;
    boolean initializing = true;

    @Override
    public void onExprModified() {
        onNewAssignment((VarMultiAssignment) Util.deepCopy(assignment));
    }
    
    @Override
    public void onEditingText() {
        if (!initializing)
            onEditingTextChanged();
    }
    
    public VarMultiAssignmentForm(boolean allowsMultipleValues, boolean allowMovements)
    {
        initComponents();
               
        if (Util.isWindows()) {
            jButtonUp.setMargin(new Insets(2, 4, 2, 4));
            jButtonDown.setMargin(new Insets(2, 4, 2, 4));
        }
        jButtonUp.setVisible(allowMovements);
        jButtonDown.setVisible(allowMovements);
        
        jComboBox_assignModel.setVisible(allowsMultipleValues);
        jLabel_eq.setVisible(!allowsMultipleValues);
        
        exprField_singleVal.setExprListener(this);
        exprField_multiVals.setExprListener(this);
        exprField_rangeFrom.setExprListener(this);
        exprField_rangeTo.setExprListener(this);
        exprField_rangeStep.setExprListener(this);
        
        initializing = false;
    }
    
    public void initializeFor(VarMultiAssignment _assignment, 
                              boolean allowsMultipleValues,
                              int numVar, int totalVars,
                              NetPage page) 
    {
        initializing = true;
        this.assignment = (VarMultiAssignment)Util.deepCopy(_assignment);
        this.numVar = numVar;
        this.totalVars = totalVars;

        jComboBox_assignModel.removeAllItems();
        jComboBox_assignModel.addItem(VarMultiAssignment.BindingModel.SINGLE_VALUE);
        jComboBox_assignModel.addItem(VarMultiAssignment.BindingModel.MULTIPLE_VALUES);
        if (assignment.hasRangeAssignment())
            jComboBox_assignModel.addItem(VarMultiAssignment.BindingModel.RANGE);
        jComboBox_assignModel.setSelectedItem(assignment.bindModel);
        
        exprField_singleVal.initializeFor(assignment.singleVal.getEditableValue(), page);
        exprField_multiVals.initializeFor(assignment.multipleVals.getEditableValue(), page);
        exprField_rangeFrom.initializeFor(assignment.rangeFrom.getEditableValue(), page);
        exprField_rangeTo.initializeFor(assignment.rangeTo.getEditableValue(), page);
        exprField_rangeStep.initializeFor(assignment.rangeStep.getEditableValue(), page);
        
        jButtonUp.setEnabled(isEnabled() && numVar > 0);
        jButtonDown.setEnabled(isEnabled() && numVar < totalVars-1);


        prepareForAssignmentModel();
        initializing = false;
    }
    
    public void deinitialize() {
        exprField_singleVal.deinitialize();
        exprField_multiVals.deinitialize();
        exprField_rangeFrom.deinitialize();
        exprField_rangeTo.deinitialize();
        exprField_rangeStep.deinitialize();
        assignment = null;
    }
    
    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        
        exprField_singleVal.setEnabled(enabled);
        exprField_multiVals.setEnabled(enabled);
        exprField_rangeFrom.setEnabled(enabled);
        exprField_rangeTo.setEnabled(enabled);
        exprField_rangeStep.setEnabled(enabled);
        jComboBox_assignModel.setEnabled(enabled);
        
        jButtonUp.setEnabled(isEnabled() && numVar > 0);
        jButtonDown.setEnabled(isEnabled() && numVar < totalVars-1);
        
        Color bkgnd = (enabled ? Color.WHITE : BKGND_DISABLED);
        setBackground(bkgnd);
        exprField_singleVal.setBackground(bkgnd);
        exprField_multiVals.setBackground(bkgnd);
        exprField_rangeFrom.setBackground(bkgnd);
        exprField_rangeTo.setBackground(bkgnd);
        exprField_rangeStep.setBackground(bkgnd);
    }
    
    public abstract void onNewAssignment(VarMultiAssignment expr);
    public abstract void onEditingTextChanged();
    public abstract void onMoveUpDown(boolean isUp);
    
    private void prepareForAssignmentModel() {
        exprField_singleVal.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.SINGLE_VALUE);
        
        exprField_multiVals.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.MULTIPLE_VALUES);
        
        exprField_rangeFrom.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.RANGE);
        exprField_rangeTo.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.RANGE);
        exprField_rangeStep.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.RANGE);
        
        jLabel_rangeFrom.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.RANGE);
        jLabel_rangeTo.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.RANGE);
        jLabel_rangeStep.setVisible(assignment.bindModel == VarMultiAssignment.BindingModel.RANGE);
    }
    
    public boolean isBindingValid() {
        switch (assignment.bindModel) {
            case SINGLE_VALUE:
                return exprField_singleVal.isValueValid();
            case MULTIPLE_VALUES:
                return exprField_multiVals.isValueValid();
            case RANGE:
                return exprField_rangeFrom.isValueValid() &&
                       exprField_rangeTo.isValueValid() &&
                       exprField_rangeStep.isValueValid();
            default:
                throw new IllegalStateException();
        }
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
        jLabel_eq = new javax.swing.JLabel();
        jComboBox_assignModel = new javax.swing.JComboBox<editor.domain.grammar.VarMultiAssignment.BindingModel>();
        exprField_singleVal = new editor.domain.measures.ExprField();
        exprField_multiVals = new editor.domain.measures.ExprField();
        jLabel_rangeFrom = new javax.swing.JLabel();
        exprField_rangeFrom = new editor.domain.measures.ExprField();
        jLabel_rangeTo = new javax.swing.JLabel();
        exprField_rangeTo = new editor.domain.measures.ExprField();
        jLabel_rangeStep = new javax.swing.JLabel();
        exprField_rangeStep = new editor.domain.measures.ExprField();
        jButtonUp = new javax.swing.JButton();
        jButtonDown = new javax.swing.JButton();

        setBackground(new java.awt.Color(255, 255, 255));
        setLayout(new java.awt.GridBagLayout());

        jLabel_eq.setText(" = ");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        add(jLabel_eq, gridBagConstraints);

        jComboBox_assignModel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jComboBox_assignModelActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 3);
        add(jComboBox_assignModel, gridBagConstraints);

        exprField_singleVal.setBackground(new java.awt.Color(255, 255, 255));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(exprField_singleVal, gridBagConstraints);

        exprField_multiVals.setBackground(new java.awt.Color(255, 255, 255));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(exprField_multiVals, gridBagConstraints);

        jLabel_rangeFrom.setText("from:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        add(jLabel_rangeFrom, gridBagConstraints);

        exprField_rangeFrom.setBackground(new java.awt.Color(255, 255, 255));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(exprField_rangeFrom, gridBagConstraints);

        jLabel_rangeTo.setText("to:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 6, 3, 3);
        add(jLabel_rangeTo, gridBagConstraints);

        exprField_rangeTo.setBackground(new java.awt.Color(255, 255, 255));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(exprField_rangeTo, gridBagConstraints);

        jLabel_rangeStep.setText("step:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 6, 3, 3);
        add(jLabel_rangeStep, gridBagConstraints);

        exprField_rangeStep.setBackground(new java.awt.Color(255, 255, 255));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(exprField_rangeStep, gridBagConstraints);

        jButtonUp.setIcon(resourceFactory.getArrowUp16());
        jButtonUp.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonUpActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        add(jButtonUp, gridBagConstraints);

        jButtonDown.setIcon(resourceFactory.getArrowDown16());
        jButtonDown.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonDownActionPerformed(evt);
            }
        });
        add(jButtonDown, new java.awt.GridBagConstraints());
    }// </editor-fold>//GEN-END:initComponents

    private void jComboBox_assignModelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jComboBox_assignModelActionPerformed
        if (initializing)
            return;
        
        VarMultiAssignment.BindingModel bm = (VarMultiAssignment.BindingModel)jComboBox_assignModel.getSelectedItem();
        if (assignment.bindModel == bm)
            return; // nothing changed
        
        assignment.bindModel = bm;
        onNewAssignment((VarMultiAssignment)Util.deepCopy(assignment));
        onEditingTextChanged();
        prepareForAssignmentModel();
    }//GEN-LAST:event_jComboBox_assignModelActionPerformed

    private void jButtonDownActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonDownActionPerformed
        onMoveUpDown(false);
    }//GEN-LAST:event_jButtonDownActionPerformed

    private void jButtonUpActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonUpActionPerformed
        onMoveUpDown(true);
    }//GEN-LAST:event_jButtonUpActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private editor.domain.measures.ExprField exprField_multiVals;
    private editor.domain.measures.ExprField exprField_rangeFrom;
    private editor.domain.measures.ExprField exprField_rangeStep;
    private editor.domain.measures.ExprField exprField_rangeTo;
    private editor.domain.measures.ExprField exprField_singleVal;
    private javax.swing.JButton jButtonDown;
    private javax.swing.JButton jButtonUp;
    private javax.swing.JComboBox<editor.domain.grammar.VarMultiAssignment.BindingModel> jComboBox_assignModel;
    private javax.swing.JLabel jLabel_eq;
    private javax.swing.JLabel jLabel_rangeFrom;
    private javax.swing.JLabel jLabel_rangeStep;
    private javax.swing.JLabel jLabel_rangeTo;
    private editor.gui.ResourceFactory resourceFactory;
    // End of variables declaration//GEN-END:variables
}
