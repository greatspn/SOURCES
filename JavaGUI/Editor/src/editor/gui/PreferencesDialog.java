/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import common.Util;
import editor.Main;
import editor.domain.measures.CosmosSolver;
import editor.domain.measures.NSolve;
import editor.domain.measures.RGMEDD2Solver;
import editor.domain.measures.SolverInvokator;
import editor.gui.net.IndeterminateListCellRenderer;
import javax.swing.JFrame;
import javax.swing.SpinnerModel;
import javax.swing.SpinnerNumberModel;
import latex.LatexProvider;
import latex.LatexProviderImpl;

/**
 *
 * @author elvio
 */
public class PreferencesDialog extends javax.swing.JDialog {

    private boolean initializing = true;
    
    /**
     * Creates new form PreferencesDialog
     */
    public PreferencesDialog(JFrame frame, boolean isModal) {
        super(frame, isModal);
        initComponents();
        Util.reformatPanelsForPlatformLookAndFeel(this);
        setLocationRelativeTo(frame);
        getRootPane().setDefaultButton(jButtonCloseDialog);
        jPanelAutoUpdate.setVisible(false);

        // General
        jCheckBoxCheckUpdates.setSelected(Main.isCheckForUpdatesAutomatic());
        jCheckBoxReopenFiles.setSelected(Main.isReopenPrevOpenFilesAtStartup());
        SpinnerModel sm = new SpinnerNumberModel(Main.getMaxUndo(), 3, 100, 1);
        jSpinnerMaxUndo.setModel(sm);
        sm = new SpinnerNumberModel(Main.getMaxLatexCacheEntries(), 50, 10000, 10);
        jSpinnerMaxLatexCache.setModel(sm);
        labelUiNote.setVisible(false);
        comboBoxUiSizes.setRenderer(new IndeterminateListCellRenderer());
        for (Main.UiSize sz : Main.UiSize.values())
            comboBoxUiSizes.addItem(sz);
        comboBoxUiSizes.setSelectedItem(Main.getStartupUiSize());
        sm = new SpinnerNumberModel(Main.getZoomMultiplier(), 10, 1000, 1);
        jSpinnerZoomMultiplier.setModel(sm);
        
        // Solvers
        filenameGreatSPN.setText(Main.getPathToGreatSPN());
        filenameGreatSPN.setDirPrefKey("greatspn-dir-pref");
        
        filenameCosmos.setText(RGMEDD2Solver.getPathToCosmos());
        filenameCosmos.setDirPrefKey("cosmos-dir-pref");
        
        filenameNSolve.setText(NSolve.getPathToNSolve());
        filenameNSolve.setDirPrefKey("nsolve-dir-pref");
        
        textField_rgmeddVer.setText(RGMEDD2Solver.getRGMEDDName());
        
        textFieldAdditionalPaths.setText(SolverInvokator.getAdditionalPathDir());
        textFieldLibraryPath.setText(SolverInvokator.getAdditionalLibraryPathDir());
        
        checkBox_useAppImageDistrib.setSelected(Main.getUseAppImageGreatSPN_Distrib());
        checkBox_useAppImageDistrib.setText("Use the portable App-Image GreatSPN distribution, when available (currently "+
                (Main.isAppImageDistribution() ? "AVAILABLE" : "NOT AVAILABLE")+").");
        checkBox_allowGreatSPNExt.setSelected(Main.isGreatSPNExtAllowed());
        checkBox_allowGreatSPNMdepArcs.setSelected(Main.areGreatSPNMdepArcsAllowed());
        
        updateLatexCacheCount();
        
        initializing = false;
    }
    
    private void updateLatexCacheCount() {
        LatexProviderImpl lp = (LatexProviderImpl)LatexProvider.getProvider();
        int numEntries = lp.getNumCacheEntries();
        jLabelNumLatexCacheEntries.setText("The LaTeX cache has "+numEntries+" entries.");
        jButtonClearLaTeXCache.setEnabled(numEntries > 0);        
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

        jButtonCloseDialog = new javax.swing.JButton();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        tabGeneral = new javax.swing.JPanel();
        jPanel4 = new javax.swing.JPanel();
        jSpinnerMaxUndo = new javax.swing.JSpinner();
        jLabel1 = new javax.swing.JLabel();
        jSpinnerMaxLatexCache = new javax.swing.JSpinner();
        jLabel2 = new javax.swing.JLabel();
        jLabelNumLatexCacheEntries = new javax.swing.JLabel();
        jButtonClearLaTeXCache = new javax.swing.JButton();
        jPanelAutoUpdate = new javax.swing.JPanel();
        jCheckBoxCheckUpdates = new javax.swing.JCheckBox();
        jPanel2 = new javax.swing.JPanel();
        jButtonCheckUpdates = new javax.swing.JButton();
        jPanel3 = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        comboBoxUiSizes = new javax.swing.JComboBox<>();
        labelUiNote = new javax.swing.JLabel();
        jCheckBoxReopenFiles = new javax.swing.JCheckBox();
        jSpinnerZoomMultiplier = new javax.swing.JSpinner();
        jLabel5 = new javax.swing.JLabel();
        tabSolverPath = new javax.swing.JPanel();
        jLabelGreatSPN = new javax.swing.JLabel();
        filenameGreatSPN = new common.JFilenameField();
        checkBox_allowGreatSPNExt = new javax.swing.JCheckBox();
        jLabel4 = new javax.swing.JLabel();
        textFieldAdditionalPaths = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        jLabelNSolve = new javax.swing.JLabel();
        filenameNSolve = new common.JFilenameField();
        jLabelCosmos = new javax.swing.JLabel();
        filenameCosmos = new common.JFilenameField();
        textField_rgmeddVer = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        jLabel9 = new javax.swing.JLabel();
        textFieldLibraryPath = new javax.swing.JTextField();
        checkBox_allowGreatSPNMdepArcs = new javax.swing.JCheckBox();
        checkBox_useAppImageDistrib = new javax.swing.JCheckBox();

        setTitle("Preferences");
        setResizable(false);
        getContentPane().setLayout(new java.awt.GridBagLayout());

        jButtonCloseDialog.setText("Close");
        jButtonCloseDialog.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonCloseDialogActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 6, 6);
        getContentPane().add(jButtonCloseDialog, gridBagConstraints);

        tabGeneral.setOpaque(false);

        jPanel4.setBorder(javax.swing.BorderFactory.createTitledBorder("Application buffers"));
        jPanel4.setOpaque(false);

        jLabel1.setText("Maximum number of undos for each open project.");

        jLabel2.setText("Maximum number of entries in the LaTeX cache.");

        jLabelNumLatexCacheEntries.setText("The LaTeX cache has X entries.");

        jButtonClearLaTeXCache.setText("Clear LaTeX cache");
        jButtonClearLaTeXCache.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonClearLaTeXCacheActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel4Layout = new org.jdesktop.layout.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                    .add(jSpinnerMaxLatexCache)
                    .add(jSpinnerMaxUndo))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jLabel2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jPanel4Layout.createSequentialGroup()
                        .add(jLabelNumLatexCacheEntries)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jButtonClearLaTeXCache)
                        .add(0, 333, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jLabel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jSpinnerMaxUndo))
                .add(11, 11, 11)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jSpinnerMaxLatexCache)
                    .add(jLabel2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabelNumLatexCacheEntries)
                    .add(jButtonClearLaTeXCache))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanelAutoUpdate.setBorder(javax.swing.BorderFactory.createTitledBorder("Automatic check for updates..."));
        jPanelAutoUpdate.setOpaque(false);

        jCheckBoxCheckUpdates.setText("Automatically check for updates at startup.");

        jPanel2.setOpaque(false);
        jPanel2.setLayout(new java.awt.GridBagLayout());

        jButtonCheckUpdates.setText("Check for updates now...");
        jButtonCheckUpdates.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonCheckUpdatesActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(6, 6, 6, 6);
        jPanel2.add(jButtonCheckUpdates, gridBagConstraints);

        org.jdesktop.layout.GroupLayout jPanelAutoUpdateLayout = new org.jdesktop.layout.GroupLayout(jPanelAutoUpdate);
        jPanelAutoUpdate.setLayout(jPanelAutoUpdateLayout);
        jPanelAutoUpdateLayout.setHorizontalGroup(
            jPanelAutoUpdateLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanelAutoUpdateLayout.createSequentialGroup()
                .addContainerGap()
                .add(jPanelAutoUpdateLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jCheckBoxCheckUpdates, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 806, Short.MAX_VALUE)
                    .add(jPanel2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanelAutoUpdateLayout.setVerticalGroup(
            jPanelAutoUpdateLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanelAutoUpdateLayout.createSequentialGroup()
                .addContainerGap()
                .add(jCheckBoxCheckUpdates)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder("General Options"));
        jPanel3.setOpaque(false);

        jLabel3.setText("SIze of the UI fonts and graphics:");

        comboBoxUiSizes.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBoxUiSizesActionPerformed(evt);
            }
        });

        labelUiNote.setText("<html><b>Note:</b> Changes to UI size will take effect the next time you start the application.");

        jCheckBoxReopenFiles.setText("Reopen previously open projects at startup.");

        jLabel5.setText("Zoom multiplier percentage.");

        org.jdesktop.layout.GroupLayout jPanel3Layout = new org.jdesktop.layout.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(jLabel3)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(comboBoxUiSizes, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .add(jCheckBoxReopenFiles, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(labelUiNote, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(jPanel3Layout.createSequentialGroup()
                                .add(jSpinnerZoomMultiplier, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(jLabel5)))
                        .add(0, 343, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(comboBoxUiSizes)
                    .add(jLabel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 27, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(labelUiNote, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jSpinnerZoomMultiplier, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel5))
                .add(9, 9, 9)
                .add(jCheckBoxReopenFiles)
                .add(0, 165, Short.MAX_VALUE))
        );

        org.jdesktop.layout.GroupLayout tabGeneralLayout = new org.jdesktop.layout.GroupLayout(tabGeneral);
        tabGeneral.setLayout(tabGeneralLayout);
        tabGeneralLayout.setHorizontalGroup(
            tabGeneralLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, tabGeneralLayout.createSequentialGroup()
                .addContainerGap()
                .add(tabGeneralLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jPanel3, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(org.jdesktop.layout.GroupLayout.LEADING, jPanelAutoUpdate, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(org.jdesktop.layout.GroupLayout.LEADING, jPanel4, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        tabGeneralLayout.setVerticalGroup(
            tabGeneralLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(tabGeneralLayout.createSequentialGroup()
                .add(jPanelAutoUpdate, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel4, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        jTabbedPane1.addTab("General", tabGeneral);

        tabSolverPath.setOpaque(false);

        jLabelGreatSPN.setText("GreatSPN directory:");

        checkBox_allowGreatSPNExt.setText("Export in extended GreatSPN format (marking parameters on arcs).");

        jLabel4.setText("Additional PATHs:");

        jLabel6.setText("<html><i> When the editor calls the external commands, it adds the 'Additional PATH' to the PATH environment variable, and the 'Additional Library Paths' to the LD_LIBRARY_PATH variable. </i></html>");

        jLabelNSolve.setText("NSolve directory:");

        filenameNSolve.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                filenameNSolveActionPerformed(evt);
            }
        });

        jLabelCosmos.setText("Cosmos directory:");

        filenameCosmos.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                filenameCosmosActionPerformed(evt);
            }
        });

        jLabel8.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        jLabel8.setText("RGMEDD version:");

        jLabel9.setText("Additional Library Paths:");

        checkBox_allowGreatSPNMdepArcs.setText("Export marking-dependent arcs in GreatSPN format (extension only support by DSPN-Tool)");

        checkBox_useAppImageDistrib.setText("Use the portable App-Image GreatSPN distribution, when available.");

        org.jdesktop.layout.GroupLayout tabSolverPathLayout = new org.jdesktop.layout.GroupLayout(tabSolverPath);
        tabSolverPath.setLayout(tabSolverPathLayout);
        tabSolverPathLayout.setHorizontalGroup(
            tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(tabSolverPathLayout.createSequentialGroup()
                .addContainerGap()
                .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(checkBox_allowGreatSPNExt, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(tabSolverPathLayout.createSequentialGroup()
                        .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(jLabel9)
                            .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                .add(jLabelGreatSPN, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .add(jLabelNSolve)
                                .add(jLabel4)
                                .add(jLabelCosmos)
                                .add(jLabel8, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(filenameGreatSPN, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 681, Short.MAX_VALUE)
                            .add(org.jdesktop.layout.GroupLayout.TRAILING, jLabel6, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                            .add(textFieldAdditionalPaths)
                            .add(filenameNSolve, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(org.jdesktop.layout.GroupLayout.TRAILING, filenameCosmos, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(textField_rgmeddVer)
                            .add(textFieldLibraryPath)))
                    .add(checkBox_allowGreatSPNMdepArcs, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 830, Short.MAX_VALUE)
                    .add(checkBox_useAppImageDistrib, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        tabSolverPathLayout.setVerticalGroup(
            tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(tabSolverPathLayout.createSequentialGroup()
                .addContainerGap()
                .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(filenameGreatSPN, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jLabelGreatSPN, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(filenameNSolve, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jLabelNSolve, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jLabelCosmos, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(filenameCosmos, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 32, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(textFieldAdditionalPaths, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel4, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 28, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel9, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 27, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(textFieldLibraryPath, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jLabel6, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 69, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tabSolverPathLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(textField_rgmeddVer, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel8))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 130, Short.MAX_VALUE)
                .add(checkBox_useAppImageDistrib)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(checkBox_allowGreatSPNExt)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(checkBox_allowGreatSPNMdepArcs)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Solvers", tabSolverPath);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(6, 6, 0, 6);
        getContentPane().add(jTabbedPane1, gridBagConstraints);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void jButtonCloseDialogActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonCloseDialogActionPerformed
        // General
        Main.setAutomaticCheckForUpdates(jCheckBoxCheckUpdates.isSelected());
        Main.setReopenPrevOpenFilesAtStartup(jCheckBoxReopenFiles.isSelected());
        Main.setMaxUndo((Integer)jSpinnerMaxUndo.getValue());
        Main.setMaxLatexCacheEntries((Integer)jSpinnerMaxLatexCache.getValue());
        Main.setStartupUiSize((Main.UiSize)comboBoxUiSizes.getSelectedItem());
        Main.setZoomMultiplier((Integer)jSpinnerZoomMultiplier.getValue());
        
        // Solvers
        Main.setPathToGreatSPN(filenameGreatSPN.getText());
        CosmosSolver.setPathToCosmos(filenameCosmos.getText());
        NSolve.setPathToNSolve(filenameNSolve.getText());
        Main.setGreatSPNExtAllowed(checkBox_allowGreatSPNExt.isSelected());
        Main.setGreatSPNMdepArcsAllowed(checkBox_allowGreatSPNMdepArcs.isSelected());
        RGMEDD2Solver.setRGMEDDName(textField_rgmeddVer.getText());
        Main.setUseAppImageGreatSPN_Distrib(checkBox_useAppImageDistrib.isSelected());
        
        SolverInvokator.setAdditionalPathDir(textFieldAdditionalPaths.getText());
        SolverInvokator.setAdditionalLibraryPathDir(textFieldLibraryPath.getText());
        
        setVisible(false);
    }//GEN-LAST:event_jButtonCloseDialogActionPerformed

    private void jButtonCheckUpdatesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonCheckUpdatesActionPerformed
        Main.verifyNewVersion(this, true);
    }//GEN-LAST:event_jButtonCheckUpdatesActionPerformed

    private void jButtonClearLaTeXCacheActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonClearLaTeXCacheActionPerformed
        ((LatexProviderImpl)LatexProvider.getProvider()).clearCache();
        updateLatexCacheCount();
    }//GEN-LAST:event_jButtonClearLaTeXCacheActionPerformed

    private void comboBoxUiSizesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBoxUiSizesActionPerformed
        boolean isCurrent = ((Main.UiSize)comboBoxUiSizes.getSelectedItem()) == Main.getStartupUiSize();
        labelUiNote.setVisible(!isCurrent);
    }//GEN-LAST:event_comboBoxUiSizesActionPerformed

    private void filenameCosmosActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_filenameCosmosActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_filenameCosmosActionPerformed

    private void filenameNSolveActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_filenameNSolveActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_filenameNSolveActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JCheckBox checkBox_allowGreatSPNExt;
    private javax.swing.JCheckBox checkBox_allowGreatSPNMdepArcs;
    private javax.swing.JCheckBox checkBox_useAppImageDistrib;
    private javax.swing.JComboBox<Main.UiSize> comboBoxUiSizes;
    private common.JFilenameField filenameCosmos;
    private common.JFilenameField filenameGreatSPN;
    private common.JFilenameField filenameNSolve;
    private javax.swing.JButton jButtonCheckUpdates;
    private javax.swing.JButton jButtonClearLaTeXCache;
    private javax.swing.JButton jButtonCloseDialog;
    private javax.swing.JCheckBox jCheckBoxCheckUpdates;
    private javax.swing.JCheckBox jCheckBoxReopenFiles;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JLabel jLabelCosmos;
    private javax.swing.JLabel jLabelGreatSPN;
    private javax.swing.JLabel jLabelNSolve;
    private javax.swing.JLabel jLabelNumLatexCacheEntries;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanelAutoUpdate;
    private javax.swing.JSpinner jSpinnerMaxLatexCache;
    private javax.swing.JSpinner jSpinnerMaxUndo;
    private javax.swing.JSpinner jSpinnerZoomMultiplier;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JLabel labelUiNote;
    private javax.swing.JPanel tabGeneral;
    private javax.swing.JPanel tabSolverPath;
    private javax.swing.JTextField textFieldAdditionalPaths;
    private javax.swing.JTextField textFieldLibraryPath;
    private javax.swing.JTextField textField_rgmeddVer;
    // End of variables declaration//GEN-END:variables
}
