/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import editor.domain.NetPage;
import editor.domain.measures.GreatSPNSolverParams.Confidence;
import editor.gui.net.IndeterminateListCellRenderer;

/**
 *
 * @author elvio
 */
public class GreatSPNSolverPanel extends SolverPanel {

    private boolean initializing = false;
    private GreatSPNSolverParams params;
    /**
     * Creates new form GreatSPNSolverPanel
     */
    public GreatSPNSolverPanel() {
        initComponents();
        Util.reformatPanelsForPlatformLookAndFeel(this);
        initializing = true;
        
        exprField_epsilon.setExprListener(solverExprListener);
        exprField_timeT.setExprListener(solverExprListener);
        exprField_maxIters.setExprListener(solverExprListener);
        exprField_maxDotMarkings.setExprListener(solverExprListener);
        exprField_approx.setExprListener(solverExprListener);
        exprField_firstTrLength.setExprListener(solverExprListener);
        exprField_minBatch.setExprListener(solverExprListener);
        exprField_maxBatch.setExprListener(solverExprListener);
        exprField_seed.setExprListener(solverExprListener);

        jComboBox_solverMode.setRenderer(new IndeterminateListCellRenderer());
        for (GreatSPNSolverParams.SolverMode sm : GreatSPNSolverParams.SolverMode.values())
            jComboBox_solverMode.addItem(sm);
        
        comboBox_confidence.setRenderer(new IndeterminateListCellRenderer());
        for (GreatSPNSolverParams.Confidence conf : GreatSPNSolverParams.Confidence.values())
            comboBox_confidence.addItem(conf);
        
        comboBox_batchMode.setRenderer(new IndeterminateListCellRenderer());
        for (GreatSPNSolverParams.BatchLengthMode blm : GreatSPNSolverParams.BatchLengthMode.values())
            comboBox_batchMode.addItem(blm);
        
        if (Util.isOSX()) {
            toggle_steadyState.putClientProperty("JButton.buttonType", "segmented");
            toggle_steadyState.putClientProperty("JButton.segmentPosition", "first");
//            toggle_stateSpace.putClientProperty("JButton.buttonType", "segmented");
//            toggle_stateSpace.putClientProperty("JButton.segmentPosition", "middle");
            toggle_transient.putClientProperty("JButton.buttonType", "segmented");
            toggle_transient.putClientProperty("JButton.segmentPosition", "last");
        }
        
        initializing = false;
    }
    
    @Override
    public void initializeFor(SolverParams _params, NetPage page, NetPage evalPage, 
                              boolean simplifiedUI, MeasurePage measPage) 
    {
        initializing = true;
        
        params = (GreatSPNSolverParams)Util.deepCopy(_params);
        
        exprField_epsilon.initializeFor(params.epsilon.getEditableValue(), page);
        exprField_maxIters.initializeFor(params.maxIters.getEditableValue(), page);
        jComboBox_solverMode.setSelectedItem(params.mode);
        toggle_steadyState.setSelected(params.solTime == GreatSPNSolverParams.SolutionTime.STEADY_STATE);
//        toggle_stateSpace.setSelected(params.solTime == GreatSPNSolverParams.SolutionTime.SIMULATED);
        toggle_transient.setSelected(params.solTime == GreatSPNSolverParams.SolutionTime.TRANSIENT);
        exprField_timeT.setEnabled(toggle_transient.isSelected());
        jLabel_at.setEnabled(toggle_transient.isSelected());
        exprField_timeT.initializeFor(params.timeT.getEditableValue(), page);
        exprField_maxDotMarkings.initializeFor(params.maxDotMarkings.getEditableValue(), page);
        exprField_approx.initializeFor(params.approxPercentage.getEditableValue(), page);
        exprField_firstTrLength.initializeFor(params.firstTrLength.getEditableValue(), page);
        exprField_minBatch.initializeFor(params.minBatch.getEditableValue(), page);
        exprField_maxBatch.initializeFor(params.maxBatch.getEditableValue(), page);
        exprField_seed.initializeFor(params.seed.getEditableValue(), page);
        checkBox_batchConstr.setSelected(params.useBatchConstraints);
        checkBox_firstTrLength.setSelected(params.useFirstTrLength);
        checkBox_useSeed.setSelected(params.useSeed);
        comboBox_confidence.setSelectedItem(params.confidence);
        comboBox_batchMode.setSelectedItem(params.batchLengthMode);
        
        exprField_firstTrLength.setEnabled(checkBox_firstTrLength.isSelected());
        exprField_minBatch.setEnabled(checkBox_batchConstr.isSelected());
        exprField_maxBatch.setEnabled(checkBox_batchConstr.isSelected());
        comboBox_batchMode.setEnabled(checkBox_batchConstr.isSelected());
        exprField_seed.setEnabled(checkBox_useSeed.isSelected());
        
        // Dependency on the requested formulas
        boolean hasTimedAnalysis = false;
        boolean showRG = false;
        for (AbstractMeasure am : measPage.measures) {
            if (am instanceof FormulaMeasure) {
                FormulaMeasure fm = (FormulaMeasure)am;
                switch (fm.getLanguage()) {
                    case ALL:
                    case PERFORMANCE_INDEX:
                        hasTimedAnalysis = true;
                        break;
                        
                    case RG:
                    case TRG:
                        showRG = true;
                        break;
                }
            }
        }

        // always hide transient/steady state for current GreatSPN simulator
        if (params.mode == GreatSPNSolverParams.SolverMode.EGSPN_SIMUL ||
            params.mode == GreatSPNSolverParams.SolverMode.SWN_ORD_SIMUL ||
            params.mode == GreatSPNSolverParams.SolverMode.SWN_SYM_SIMUL)
            hasTimedAnalysis = false; 
        
        panel_Timed.setVisible(hasTimedAnalysis);
        panel_showRG.setVisible(showRG && !params.mode.isSimulation());
        panel_WNRG_Timed.setVisible(hasTimedAnalysis && !params.mode.isSimulation());
        panel_SIMUL.setVisible(params.mode.isSimulation());
        
        // Simplified UI support
        jComboBox_solverMode.setEnabled(!simplifiedUI);
        toggle_transient.setEnabled(!simplifiedUI);
        toggle_steadyState.setEnabled(!simplifiedUI);
        
 
        initializing = false;
    }

    @Override
    public void deinitialize() {
        exprField_epsilon.deinitialize();
        exprField_maxIters.deinitialize();
        exprField_timeT.deinitialize();
        exprField_maxDotMarkings.deinitialize();
        exprField_approx.deinitialize();
        exprField_firstTrLength.deinitialize();
        exprField_minBatch.deinitialize();
        exprField_maxBatch.deinitialize();
        exprField_seed.deinitialize();
        params = null;
    }

    @Override
    public SolverParams currParams() {
        return params;
    }

    @Override
    public boolean areParamsCorrect() {
        return exprField_epsilon.isValueValid() &&
               exprField_maxIters.isValueValid() &&
               exprField_timeT.isValueValid() &&
               exprField_maxDotMarkings.isValueValid() &&
               exprField_approx.isValueValid() &&
               exprField_firstTrLength.isValueValid() &&
               exprField_minBatch.isValueValid() &&
               exprField_maxBatch.isValueValid() &&
               exprField_seed.isValueValid();
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

        jLabel2 = new javax.swing.JLabel();
        jComboBox_solverMode = new javax.swing.JComboBox<>();
        panel_WNRG_Timed = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        exprField_epsilon = new editor.domain.measures.ExprField();
        jLabel3 = new javax.swing.JLabel();
        exprField_maxIters = new editor.domain.measures.ExprField();
        panel_Timed = new javax.swing.JPanel();
        jLabel5 = new javax.swing.JLabel();
        jLabel_at = new javax.swing.JLabel();
        exprField_timeT = new editor.domain.measures.ExprField();
        jPanel1 = new javax.swing.JPanel();
        toggle_transient = new javax.swing.JToggleButton();
        toggle_steadyState = new javax.swing.JToggleButton();
        panel_showRG = new javax.swing.JPanel();
        jLabel4 = new javax.swing.JLabel();
        exprField_maxDotMarkings = new editor.domain.measures.ExprField();
        jPanel2 = new javax.swing.JPanel();
        panel_SIMUL = new javax.swing.JPanel();
        label_confidence = new javax.swing.JLabel();
        comboBox_confidence = new javax.swing.JComboBox<>();
        label_approx = new javax.swing.JLabel();
        exprField_approx = new editor.domain.measures.ExprField();
        checkBox_firstTrLength = new javax.swing.JCheckBox();
        exprField_firstTrLength = new editor.domain.measures.ExprField();
        checkBox_batchConstr = new javax.swing.JCheckBox();
        exprField_minBatch = new editor.domain.measures.ExprField();
        label_maxBatch = new javax.swing.JLabel();
        exprField_maxBatch = new editor.domain.measures.ExprField();
        checkBox_useSeed = new javax.swing.JCheckBox();
        exprField_seed = new editor.domain.measures.ExprField();
        jLabel6 = new javax.swing.JLabel();
        comboBox_batchMode = new javax.swing.JComboBox<>();

        setLayout(new java.awt.GridBagLayout());

        jLabel2.setText("Solver mode:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(6, 6, 6, 0);
        add(jLabel2, gridBagConstraints);

        jComboBox_solverMode.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jComboBox_solverModeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(6, 0, 6, 0);
        add(jComboBox_solverMode, gridBagConstraints);

        panel_WNRG_Timed.setLayout(new java.awt.GridBagLayout());

        jLabel1.setText("Epsilon:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panel_WNRG_Timed.add(jLabel1, gridBagConstraints);

        exprField_epsilon.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        panel_WNRG_Timed.add(exprField_epsilon, gridBagConstraints);

        jLabel3.setText("Max iterations:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 10, 0, 0);
        panel_WNRG_Timed.add(jLabel3, gridBagConstraints);

        exprField_maxIters.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 4;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        panel_WNRG_Timed.add(exprField_maxIters, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        add(panel_WNRG_Timed, gridBagConstraints);

        panel_Timed.setLayout(new java.awt.GridBagLayout());

        jLabel5.setText("CTMC solution is computed in:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panel_Timed.add(jLabel5, gridBagConstraints);

        jLabel_at.setText("at:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panel_Timed.add(jLabel_at, gridBagConstraints);

        exprField_timeT.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 4;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.2;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panel_Timed.add(exprField_timeT, gridBagConstraints);

        jPanel1.setLayout(new java.awt.GridBagLayout());

        toggle_transient.setText("Transient");
        toggle_transient.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_transientActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        jPanel1.add(toggle_transient, gridBagConstraints);

        toggle_steadyState.setText("Steady state");
        toggle_steadyState.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_steadyStateActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        jPanel1.add(toggle_steadyState, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panel_Timed.add(jPanel1, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(8, 0, 8, 0);
        add(panel_Timed, gridBagConstraints);

        panel_showRG.setLayout(new java.awt.GridBagLayout());

        jLabel4.setText("Max. markings in RG plot:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 4;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 10, 0, 0);
        panel_showRG.add(jLabel4, gridBagConstraints);

        exprField_maxDotMarkings.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 5;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        panel_showRG.add(exprField_maxDotMarkings, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.3;
        panel_showRG.add(jPanel2, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(8, 0, 8, 0);
        add(panel_showRG, gridBagConstraints);

        panel_SIMUL.setLayout(new java.awt.GridBagLayout());

        label_confidence.setText("Confidence:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(6, 6, 6, 0);
        panel_SIMUL.add(label_confidence, gridBagConstraints);

        comboBox_confidence.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_confidenceActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        panel_SIMUL.add(comboBox_confidence, gridBagConstraints);

        label_approx.setText("Approx:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panel_SIMUL.add(label_approx, gridBagConstraints);

        exprField_approx.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 4;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        panel_SIMUL.add(exprField_approx, gridBagConstraints);

        checkBox_firstTrLength.setText("Skip first transient of length:");
        checkBox_firstTrLength.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_firstTrLengthActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 7;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 10, 0, 0);
        panel_SIMUL.add(checkBox_firstTrLength, gridBagConstraints);

        exprField_firstTrLength.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 9;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.3;
        panel_SIMUL.add(exprField_firstTrLength, gridBagConstraints);

        checkBox_batchConstr.setText("Set batch");
        checkBox_batchConstr.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_batchConstrActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        panel_SIMUL.add(checkBox_batchConstr, gridBagConstraints);

        exprField_minBatch.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        panel_SIMUL.add(exprField_minBatch, gridBagConstraints);

        label_maxBatch.setText("max:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 5;
        gridBagConstraints.gridy = 1;
        panel_SIMUL.add(label_maxBatch, gridBagConstraints);

        exprField_maxBatch.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 6;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        panel_SIMUL.add(exprField_maxBatch, gridBagConstraints);

        checkBox_useSeed.setText("Specify random seed:");
        checkBox_useSeed.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_useSeedActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 7;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(0, 10, 0, 0);
        panel_SIMUL.add(checkBox_useSeed, gridBagConstraints);

        exprField_seed.setOkNoLabelVisible(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 8;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.3;
        panel_SIMUL.add(exprField_seed, gridBagConstraints);

        jLabel6.setText("to min:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        panel_SIMUL.add(jLabel6, gridBagConstraints);

        comboBox_batchMode.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_batchModeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        panel_SIMUL.add(comboBox_batchMode, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(8, 0, 8, 0);
        add(panel_SIMUL, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents

    private void jComboBox_solverModeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jComboBox_solverModeActionPerformed
        if (initializing)
            return;
        
        Object selObj = jComboBox_solverMode.getSelectedItem();
        GreatSPNSolverParams.SolverMode sm = (GreatSPNSolverParams.SolverMode)selObj;
        if (sm == params.mode)
            return;
        params.mode = sm;
        fireSolverParamsModified();
    }//GEN-LAST:event_jComboBox_solverModeActionPerformed

    private void toggle_steadyStateActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_steadyStateActionPerformed
        if (initializing)
            return;
        if (params.solTime == GreatSPNSolverParams.SolutionTime.STEADY_STATE)
            return;
        params.solTime = GreatSPNSolverParams.SolutionTime.STEADY_STATE;
        fireSolverParamsModified();
    }//GEN-LAST:event_toggle_steadyStateActionPerformed

    private void toggle_transientActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_transientActionPerformed
        if (initializing)
            return;
        if (params.solTime == GreatSPNSolverParams.SolutionTime.TRANSIENT)
            return;
        params.solTime = GreatSPNSolverParams.SolutionTime.TRANSIENT;
        fireSolverParamsModified();
    }//GEN-LAST:event_toggle_transientActionPerformed

    private void comboBox_confidenceActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_confidenceActionPerformed
        if (initializing)
            return;
        
        Confidence conf = (Confidence)comboBox_confidence.getSelectedItem();
        if (conf == params.confidence)
            return;
        params.confidence = conf;
        fireSolverParamsModified();
    }//GEN-LAST:event_comboBox_confidenceActionPerformed

    private void checkBox_firstTrLengthActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_firstTrLengthActionPerformed
        params.useFirstTrLength = checkBox_firstTrLength.isSelected();
        fireSolverParamsModified();
    }//GEN-LAST:event_checkBox_firstTrLengthActionPerformed

    private void checkBox_batchConstrActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_batchConstrActionPerformed
        params.useBatchConstraints = checkBox_batchConstr.isSelected();
        fireSolverParamsModified();
    }//GEN-LAST:event_checkBox_batchConstrActionPerformed

    private void checkBox_useSeedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_useSeedActionPerformed
        params.useSeed = checkBox_useSeed.isSelected();
        fireSolverParamsModified();
    }//GEN-LAST:event_checkBox_useSeedActionPerformed

    private void comboBox_batchModeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_batchModeActionPerformed
        if (initializing)
            return;
        
        GreatSPNSolverParams.BatchLengthMode blm;
        blm = (GreatSPNSolverParams.BatchLengthMode)comboBox_batchMode.getSelectedItem();
        if (blm == params.batchLengthMode)
            return;
        params.batchLengthMode = blm;
        fireSolverParamsModified();
    }//GEN-LAST:event_comboBox_batchModeActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JCheckBox checkBox_batchConstr;
    private javax.swing.JCheckBox checkBox_firstTrLength;
    private javax.swing.JCheckBox checkBox_useSeed;
    private javax.swing.JComboBox<GreatSPNSolverParams.BatchLengthMode> comboBox_batchMode;
    private javax.swing.JComboBox<GreatSPNSolverParams.Confidence> comboBox_confidence;
    private editor.domain.measures.ExprField exprField_approx;
    private editor.domain.measures.ExprField exprField_epsilon;
    private editor.domain.measures.ExprField exprField_firstTrLength;
    private editor.domain.measures.ExprField exprField_maxBatch;
    private editor.domain.measures.ExprField exprField_maxDotMarkings;
    private editor.domain.measures.ExprField exprField_maxIters;
    private editor.domain.measures.ExprField exprField_minBatch;
    private editor.domain.measures.ExprField exprField_seed;
    private editor.domain.measures.ExprField exprField_timeT;
    private javax.swing.JComboBox<GreatSPNSolverParams.SolverMode> jComboBox_solverMode;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel_at;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JLabel label_approx;
    private javax.swing.JLabel label_confidence;
    private javax.swing.JLabel label_maxBatch;
    private javax.swing.JPanel panel_SIMUL;
    private javax.swing.JPanel panel_Timed;
    private javax.swing.JPanel panel_WNRG_Timed;
    private javax.swing.JPanel panel_showRG;
    private javax.swing.JToggleButton toggle_steadyState;
    private javax.swing.JToggleButton toggle_transient;
    // End of variables declaration//GEN-END:variables
}
