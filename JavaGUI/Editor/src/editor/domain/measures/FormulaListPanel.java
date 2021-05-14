/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.util.ArrayList;
import javax.swing.BorderFactory;
import javax.swing.JButton;

/**
 *
 * @author elvio
 */
public class FormulaListPanel extends javax.swing.JPanel {

    public ArrayList<AbstractMeasure>  measList = new ArrayList<>();
    private FormulaPanel[] allForms;
    
    public interface FormulaListListener {
        public void onNewMeasureList(ArrayList<AbstractMeasure> newList);
        public void onMeasureListSelectionChanged(int num, boolean isSelected);
        public void onFormulaListEditing();
        public void onFormulaCompute(int formulaNum);
        public void onViewFormulaResults(int formulaNum);
    }
    private FormulaListListener listener;
    
    /**
     * Creates new form TemplateBindingForm2
     */
    public FormulaListPanel() {
        initComponents();
        setBorder(BorderFactory.createMatteBorder(1, 1, 1, 1, Color.GRAY));
        setBackground(editor.gui.net.NetEditorPanel.PAGE_TITLE_BACKGROUND);
        jLabel1.setForeground(editor.gui.net.NetEditorPanel.PAGE_TITLE_FOREGROUND);
        jLabel2.setForeground(editor.gui.net.NetEditorPanel.PAGE_TITLE_FOREGROUND);
    }
    
    public void setFormulaLisListener(FormulaListListener listener) {
        this.listener = listener;
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        
        for (FormulaPanel fp : allForms)
            fp.setEnabled(enabled);
        setBackground(enabled ? Color.WHITE : VarMultiAssignmentForm.BKGND_DISABLED);
    }
    
    private GridBagConstraints getGBCHorizWidth2Fill(int y) {
        return new GridBagConstraints(0, y, 2, 1, 1.0, 0.0, GridBagConstraints.CENTER, 
                                      GridBagConstraints.HORIZONTAL, 
                                      new Insets(0,0,0,0), 0, 0);
    }
    
    
    public void initializeFor(ArrayList<AbstractMeasure> _measList, MeasurePage page) {
        @SuppressWarnings("unchecked")
        ArrayList<AbstractMeasure> copy = (ArrayList<AbstractMeasure>)Util.deepCopy(_measList);
        this.measList = copy;
        
        // Was initialized, but the number of formulas changed. Reinitialize from scratch
        if (allForms != null && allForms.length != measList.size()) {
            clearForms();
        }
        
        if (allForms == null) {
            // Construct a new table
            int count = 0;
            //if (measList == null) {
                //System.out.println("measList = "+measList);
                //System.out.println("measList.size() = "+measList.size());
            //}
            allForms = new FormulaPanel[measList.size()];
            for (AbstractMeasure measure : measList) {
                final int pos = count;
                
                allForms[pos] = new FormulaPanel() {
                    @Override
                    public void onNewFormula(FormulaMeasure newMeasure) {
                        Object copy = Util.deepCopy(measList);
                        if (!(copy instanceof ArrayList))
                            throw new IllegalStateException();
                        @SuppressWarnings("unchecked")
                        ArrayList<AbstractMeasure> newMeas = (ArrayList<AbstractMeasure>)copy;
                        newMeas.set(pos, newMeasure);
                        listener.onNewMeasureList(newMeas);
                    }

                    @Override
                    public void onFormulaSelectionChanged(int num, boolean isSelected) {
//                        measList.get(pos).setSelected(isSelected);
                        listener.onMeasureListSelectionChanged(num, isSelected);
                    }

                    @Override
                    public void onFormulaEditing() {
                        listener.onFormulaListEditing();
                    }

                    @Override
                    public void onFormulaCompute(int formulaNum) {
                        listener.onFormulaCompute(formulaNum);
                    }

                    @Override
                    public void onViewResults(int formulaNum) {
                        listener.onViewFormulaResults(formulaNum);
                    }
                };
                add(allForms[count], getGBCHorizWidth2Fill(count+2));
                
                count++;
            }
        }
        
        // (Re)Initialize the existing table
        assert allForms.length == measList.size();
        boolean allOk = true;
        for (int i=0; i<allForms.length; i++) {
            AbstractMeasure abMeas = measList.get(i);
            if (!(abMeas instanceof FormulaMeasure))
                throw new UnsupportedOperationException();
            FormulaMeasure measure = (FormulaMeasure)abMeas;
            allForms[i].initializeFor(measure, i, page);
            
            allOk = allOk && allForms[i].isFormulaValid(page.solverParams);
        }
    }
    
    public void deinitialize() {
        clearForms();
        measList = null;
    }
    
    private void clearForms() {
        for (FormulaPanel fp : allForms) {
            fp.deinitialize();
            remove(fp);
        }
        allForms = null;
    }
    
    public boolean areAllFormulasValid(SolverParams solverParams) {
        for (FormulaPanel fp : allForms)
            if (!fp.isFormulaValid(solverParams))
                return false;
        return true;
    }
    
    public void setComputeButtons (boolean allParamsOk, SolverParams solverParams) {
        for (FormulaPanel fp : allForms)
            fp.setComputeButton(allParamsOk, solverParams);
    }
    
    public JButton getViewResultsButtonOf(int formulaNum) { 
        return allForms[formulaNum].getViewResultsButton();
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

        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();

        setLayout(new java.awt.GridBagLayout());

        jLabel1.setText("Pos:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        add(jLabel1, gridBagConstraints);

        jLabel2.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel2.setText("Measure:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        add(jLabel2, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    // End of variables declaration//GEN-END:variables
}
