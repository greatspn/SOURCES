/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.measures;

import common.Util;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JButton;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

/**
 *
 * @author Elvio
 */
public abstract class FormulaPanel extends javax.swing.JPanel implements ExprField.ExprFieldListener {

    private FormulaMeasure measure;
    private SolverParams currSolverParams;
    private int formulaNum;
    private boolean initializing = false;
    
    /**
     * Creates new form FormulaPanel
     */
    public FormulaPanel() {
        initComponents();
        
        jLabel_dropdown.addMouseListener(openPopupListener);
        jLabel_icon.addMouseListener(openPopupListener);
        jLabel_icon.setText("");
        jTextField_result.setText("---");
        exprField_comment.setExprListener(this);
        exprField_formula.setExprListener(this);
        
        Dimension dim = jButtonViewResults.getPreferredSize();
        dim.width = (int)(dim.width * 1.5);
        dim.height = jTextField_result.getPreferredSize().height;
        jTextField_result.setMinimumSize(dim);
        jTextField_result.setMaximumSize(dim);
        jTextField_result.setPreferredSize(dim);
    }
    
    public abstract void onNewFormula(FormulaMeasure newMeasure);
    public abstract void onFormulaSelectionChanged(int num, boolean isSelected);
    public abstract void onFormulaEditing();
    public abstract void onFormulaCompute(int formulaNum);
    public abstract void onViewResults(int formulaNum);
    
    private static final Color BACKGROUND_SELECTED = new Color(240, 240, 255);
    
    public void initializeFor(FormulaMeasure _measure,
                              int num, MeasurePage page) 
    {
        initializing = true;
        measure = _measure;
        currSolverParams = page.solverParams;
        formulaNum = num;
        jCheckBox_selected.setSelected(measure.isSelected());
        jLabel_icon.setIcon(measure.getLanguage().getIcon24());
        jLabel_num.setText((num+1<10 ? " " : "")+(num+1) + "°");
        
        boolean isComputableBySolver = (null==page.solverParams.isLanguageSupported(measure.getLanguage()));
        Color bkgnd = Color.WHITE;
        if (measure.isSelected())
            bkgnd = BACKGROUND_SELECTED;
        setBackground(bkgnd);
        jCheckBox_selected.setBackground(bkgnd);
        exprField_comment.setBackground(bkgnd);
        exprField_formula.setBackground(bkgnd);
        
        exprField_comment.initializeFor(measure.getCommentEditable(), page);
        exprField_formula.initializeFor(measure.getFormulaEditable(), page);
        
        jLabelComment.setVisible(measure.isCommentShown());
        exprField_comment.setVisible(measure.isCommentShown());
                
        boolean hasFormula = measure.getLanguage().hasFormulaText();
        if (!hasFormula) // show an alternate text box
            jLabelAltText.setText(measure.getLanguage().alternateToFormulaText());
        jLabelAltText.setVisible(!hasFormula);
        jLabelEqual.setVisible(hasFormula);
        exprField_formula.setVisible(hasFormula);
        jTextField_result.setVisible(hasFormula);
        
        if (hasFormula && measure.hasResults()) {
            if (measure.getResults().table.size() == 1) {
                // Single scalar result
                jButtonViewResults.setVisible(false);
                jTextField_result.setVisible(true);
                jTextField_result.setText(measure.getResults().table.get(0).toString());
            }
            else {
                // Table with many results                
                jButtonViewResults.setVisible(true);
                jButtonViewResults.setEnabled(true);
                jTextField_result.setVisible(false);
            }
        }
        else {
            jButtonViewResults.setText(measure.getLanguage().getViewResultButtonText());
            jButtonViewResults.setVisible(measure.hasResults());
            jButtonViewResults.setEnabled(measure.hasResultsMarkedOk());
            jTextField_result.setVisible(false /* no formula to show */);
            jTextField_result.setText("---");
        }
        
        jButtonCompute.setEnabled(isComputableBySolver);
        jLabelAltText.setEnabled(isComputableBySolver);

        
        initializing = false;
    }
    
    public void deinitialize() {
        exprField_comment.deinitialize();
        exprField_formula.deinitialize();
        measure = null;
        currSolverParams = null;
    }
    
    public boolean isFormulaValid(SolverParams solverParams) {
        return exprField_comment.isValueValid() &&
               (!measure.getLanguage().hasFormulaText() || exprField_formula.isValueValid()) &&
                null==solverParams.isLanguageSupported(measure.getLanguage());
    }

    @Override
    public void onExprModified() {
        onNewFormula((FormulaMeasure)Util.deepCopy(measure));
    }

    @Override
    public void onEditingText() {
        if (!initializing)
            onFormulaEditing();
    }
    
    class OpenPupupListener extends MouseAdapter {
        @Override
        public void mouseClicked(MouseEvent e) {
            if (!jLabel_icon.isEnabled() ||
                e.getButton() != 1)
                return;
            //Create the popup menu.
            final JPopupMenu popup = new JPopupMenu();
            for (final FormulaLanguage fl : FormulaLanguage.values()) {
                JMenuItem item = new JMenuItem(fl.getName(), fl.getIcon24());
                item.addActionListener(new ActionListener() {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        if (measure.getLanguage() != fl) {
                            measure.setLanguage(fl);
                            onNewFormula((FormulaMeasure)Util.deepCopy(measure));
                        }
                    }
                });
                if (null != currSolverParams.isLanguageSupported(fl))
                    item.setEnabled(false);
                popup.add(item);
            };
            Util.reformatMenuPanels(popup);
            popup.show(jLabel_icon, 0, jLabel_icon.getHeight());
        }
    }
    private final OpenPupupListener openPopupListener = new OpenPupupListener();

    public void setComputeButton(boolean allParamsOk, SolverParams solverParams) {
        jButtonCompute.setEnabled(allParamsOk && isFormulaValid(solverParams));
    }
    
    public JButton getViewResultsButton() { return jButtonViewResults; }
    
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
        exprField_formula = new editor.domain.measures.ExprField();
        jCheckBox_selected = new javax.swing.JCheckBox();
        jLabel_num = new javax.swing.JLabel();
        jLabel_icon = new javax.swing.JLabel();
        jLabel_dropdown = new javax.swing.JLabel();
        jLabelComment = new javax.swing.JLabel();
        jLabelEqual = new javax.swing.JLabel();
        jTextField_result = new javax.swing.JTextField();
        jButtonCompute = new javax.swing.JButton();
        jPanelVertBar = new javax.swing.JPanel();
        exprField_comment = new editor.domain.measures.ExprField();
        jButtonViewResults = new javax.swing.JButton();
        jLabelAltText = new javax.swing.JLabel();

        setBackground(new java.awt.Color(255, 255, 255));
        setBorder(javax.swing.BorderFactory.createMatteBorder(1, 0, 0, 0, new java.awt.Color(102, 102, 102)));
        setLayout(new java.awt.GridBagLayout());

        exprField_formula.setBackground(new java.awt.Color(255, 255, 255));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 5;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.3;
        gridBagConstraints.insets = new java.awt.Insets(1, 3, 3, 3);
        add(exprField_formula, gridBagConstraints);

        jCheckBox_selected.setBackground(new java.awt.Color(255, 255, 255));
        jCheckBox_selected.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jCheckBox_selectedActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 3);
        add(jCheckBox_selected, gridBagConstraints);

        jLabel_num.setFont(new java.awt.Font("Monospaced", 0, 14)); // NOI18N
        jLabel_num.setText("N°");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 6);
        add(jLabel_num, gridBagConstraints);

        jLabel_icon.setText("ICON");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 0);
        add(jLabel_icon, gridBagConstraints);

        jLabel_dropdown.setIcon(resourceFactory.getDropdown16());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 3);
        add(jLabel_dropdown, gridBagConstraints);

        jLabelComment.setText("Comment:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 5;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 3);
        add(jLabelComment, gridBagConstraints);

        jLabelEqual.setText(" = ");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 8;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(1, 6, 3, 6);
        add(jLabelEqual, gridBagConstraints);

        jTextField_result.setEditable(false);
        jTextField_result.setText("result");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 9;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(1, 3, 3, 3);
        add(jTextField_result, gridBagConstraints);

        jButtonCompute.setText("Compute");
        jButtonCompute.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonComputeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 11;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 3;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.PAGE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 3, 0);
        add(jButtonCompute, gridBagConstraints);

        jPanelVertBar.setBorder(javax.swing.BorderFactory.createMatteBorder(0, 1, 0, 0, new java.awt.Color(102, 102, 102)));
        jPanelVertBar.setLayout(new javax.swing.BoxLayout(jPanelVertBar, javax.swing.BoxLayout.LINE_AXIS));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 4;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 3);
        add(jPanelVertBar, gridBagConstraints);

        exprField_comment.setBackground(new java.awt.Color(255, 255, 255));
        exprField_comment.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 6;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 5;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 3);
        add(exprField_comment, gridBagConstraints);

        jButtonViewResults.setText("Results...");
        jButtonViewResults.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonViewResultsActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 10;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.insets = new java.awt.Insets(1, 3, 3, 3);
        add(jButtonViewResults, gridBagConstraints);

        jLabelAltText.setText("Text alternative to the formula textfield.");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 5;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.gridwidth = 5;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 3, 3, 3);
        add(jLabelAltText, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents

    private void jCheckBox_selectedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jCheckBox_selectedActionPerformed
        if (initializing)
            return;

        // Selection does not create a new undo/redo record
        measure.setSelected(jCheckBox_selected.isSelected());
        onFormulaSelectionChanged(formulaNum, measure.isSelected());
    }//GEN-LAST:event_jCheckBox_selectedActionPerformed

    private void jButtonComputeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonComputeActionPerformed
        onFormulaCompute(formulaNum);
    }//GEN-LAST:event_jButtonComputeActionPerformed

    private void jButtonViewResultsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonViewResultsActionPerformed
        onViewResults(formulaNum);
    }//GEN-LAST:event_jButtonViewResultsActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private editor.domain.measures.ExprField exprField_comment;
    private editor.domain.measures.ExprField exprField_formula;
    private javax.swing.JButton jButtonCompute;
    private javax.swing.JButton jButtonViewResults;
    private javax.swing.JCheckBox jCheckBox_selected;
    private javax.swing.JLabel jLabelAltText;
    private javax.swing.JLabel jLabelComment;
    private javax.swing.JLabel jLabelEqual;
    private javax.swing.JLabel jLabel_dropdown;
    private javax.swing.JLabel jLabel_icon;
    private javax.swing.JLabel jLabel_num;
    private javax.swing.JPanel jPanelVertBar;
    private javax.swing.JTextField jTextField_result;
    private editor.gui.ResourceFactory resourceFactory;
    // End of variables declaration//GEN-END:variables
}
