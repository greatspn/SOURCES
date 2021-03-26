/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Action;
import common.Condition;
import common.Util;
import editor.Main;
import editor.domain.NetPage;
import editor.domain.elements.GspnPage;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.grammar.VarListMultiAssignment;
import editor.domain.composition.ComposableNet;
import editor.gui.AbstractPageEditor;
import editor.gui.CutCopyPasteEngine;
import editor.gui.MainWindowInterface;
import editor.gui.NoOpException;
import editor.gui.SharedResourceProvider;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.ActionMap;
import javax.swing.InputMap;
import javax.swing.JButton;
import javax.swing.JComponent;
import static javax.swing.JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.MutableComboBoxModel;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/** The editor of the measure pages
 *
 * @author elvio
 */
public class MeasureEditorPanel extends javax.swing.JPanel 
    implements AbstractPageEditor, 
               FormulaListPanel.FormulaListListener,
               VarMultiAssignmentListPanel.MultiAssignmentListListener,
               SolverPanel.SolverPanelListener
{
    private boolean initializing = true;
    private ProjectFile currProject = null;
    private MeasurePage currPage = null;
    private MainWindowInterface mainInterface = null;
    
    // The list of measures
    private final JMeasureList measList;
    private boolean updatingGspnCombo = false;

    private final VarMultiAssignmentListPanel bindingForms;
    private final FormulaListPanel formulaForm;
    private SolverPanel solverPanel;
    
    /**
     * Creates new form MeasureEditorPanel
     * @param shActProv
     * @param actionCondition
     */
    public MeasureEditorPanel(SharedResourceProvider shActProv, Condition actionCondition) {
        initComponents();
        Util.reformatPanelsForPlatformLookAndFeel(this);
        Util.reformatPanelsForPlatformLookAndFeel(jPanelScrollContainer);
        Util.reformatPanelsForPlatformLookAndFeel(propertyPanel);
        if (Util.useUnifiedToolbar()) {
            jPanelScrollContainer.setOpaque(true);
            jPanelScrollContainer.setBackground(Util.UNIFIED_GRAY_PANEL_BKGND);
        }
        
        initializing = true;
        measList =  new JMeasureList();
        
        jScrollPaneList.setViewportView(measList);
        jScrollPaneList.getViewport().setBackground(getBackground());
        
        jScrollPaneList.getViewport().addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                if (currPage != null)
                    currPage.viewportPos = jScrollPaneList.getViewport().getViewPosition();
            }
        });
        
        // Add actions to the input map manually
        InputMap inMap = getInputMap(WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        ActionMap actMap = getActionMap();
        Action.registerAllActions(actMap, inMap, this, getClass(), actionCondition);
        
        // The template variable (multi)bindings panel
        bindingForms = new VarMultiAssignmentListPanel();
        bindingForms.setMultiAssignmentListListener(this);
        jPanelTemplateParams.add(bindingForms);
        
        // The measure list panel
        formulaForm = new FormulaListPanel();
        formulaForm.setFormulaLisListener(this);
        jPanelMeasures.add(formulaForm);
        
        // The list of solvers
        for (SolverParams.Solvers s : SolverParams.Solvers.values())
            jComboBox_solver.addItem(s);
        
        initializing = false;
    }

    @Override
    public void setEditorEnabledFor(ProjectFile pf, ProjectPage page, MainWindowInterface mainInterface) {
        assert page != null && page instanceof MeasurePage;
        currProject = pf;
        currPage = (MeasurePage)page;
        this.mainInterface = mainInterface;
        
        updateGUI(pf, page);
    }

    @Override
    public void setEditorDisabled() {
        assert currProject != null;
        currProject = null;
        currPage = null;
        mainInterface = null;
        
        bindingForms.deinitialize();
        formulaForm.deinitialize();
        solverPanel.deinitialize();
    }

    private static final String UNSELECTED_GSPN = "<Select a Petri net model>";
    @Override
    public void updateGUI(ProjectFile pf, ProjectPage page) {
        assert pf == currProject;
        currPage = (MeasurePage)page;
        initializing = true;
        
        // Selection information
        int numSel = currPage.countSelectedMeasures();
        
        // Update the list of GSPN models
        updatingGspnCombo = true;
        jComboBox_gspn.removeAllItems();
        boolean hasSelection = false;
        for (int p = 0; p < pf.getCurrent().getPageCount(); p++) {
            ProjectPage targetPage = pf.getCurrent().getPageAt(p);
            if (targetPage instanceof ComposableNet) {
                NetPage comp = ((ComposableNet)targetPage).getComposedNet();
                if (comp != null && comp instanceof GspnPage) {
                    GspnPage gspn = (GspnPage)comp;
                    jComboBox_gspn.addItem(targetPage.getPageName());
                    if (currPage.targetGspnName != null && 
                        currPage.targetGspnName.equals(targetPage.getPageName())) 
                    {
                        hasSelection = true;
                        jComboBox_gspn.setSelectedItem(currPage.targetGspnName);
                    }
                }
            }
        }
        if (!hasSelection) {
            ((MutableComboBoxModel<String>)jComboBox_gspn.getModel()).insertElementAt(UNSELECTED_GSPN, 0);
            jComboBox_gspn.setSelectedItem(UNSELECTED_GSPN);
        }
        updatingGspnCombo = false;
        
        // Update the list of template bindings (if any)
        boolean hasBindings = (currPage.targetGspn != null &&
                               currPage.varListAssignments.assigns.size() > 0) ;
        bindingForms.initializeFor(currPage.varListAssignments, currPage.targetGspn, true);
        jPanelTemplateParamsTitle.setVisible(hasBindings);
        
        // Selected solver
        jComboBox_solver.setSelectedItem(currPage.solverParams.getSolver());
 
        // Simplified UI support
        jComboBox_gspn.setEnabled(!currPage.simplifiedUI);
        jComboBox_solver.setEnabled(!currPage.simplifiedUI);

        // Update the solver parameters section
        if (solverPanel == null ||
            solverPanel.getClass() != currPage.solverParams.getPanelClass()) 
        {
            // Create a new solver panel
            if (solverPanel != null) {
                jPanelSolverParams.remove(solverPanel);
                solverPanel = null;
            }
            try {
                solverPanel = (SolverPanel)currPage.solverParams.getPanelClass().newInstance();
            }
            catch (IllegalAccessException | InstantiationException ie) {
                Main.logException(ie, true);
                System.exit(-1);
            }
            solverPanel.setSolverPanelListener(this);
            jPanelSolverParams.add(solverPanel);
        }
        solverPanel.initializeFor(currPage.solverParams, currPage.targetGspn, 
                                  currPage.targetGspn, currPage.simplifiedUI, currPage);
        
        // Update the list of measures
        formulaForm.initializeFor(currPage.measures, currPage);

        // Keep viewport in sync with the measure page position
        jScrollPaneList.getViewport().setViewPosition(currPage.viewportPos);
        
        // Update actions
        actionCommentMeasure.setEnabled(numSel > 0);
        actionMoveUp.setEnabled(numSel > 0 && !currPage.measures.get(0).isSelected());
        actionMoveDown.setEnabled(numSel > 0 &&
                                  !currPage.measures.get(currPage.measures.size()-1).isSelected());
        actionViewLastLog.setEnabled(!currPage.lastLog.isNull());
        
        boolean mayExport = (numSel > 0);
        for (int m=0; m<currPage.measures.size() && mayExport; m++)
            if (currPage.measures.get(m).isSelected())
                mayExport = mayExport && currPage.measures.get(m).canExportResultsAsWorkSheets();
        actionExportExcel.setEnabled(mayExport);
        
        initializing = false;
        updateEditing();
        
        
        //System.out.println("updateGUI: isPageCorrect="+currPage.isPageCorrect());
    }

    @Override
    public void updateEnablingOfSharedActions(SharedResourceProvider shResProv) {
        
        int numSel = currPage.countSelectedMeasures();
        int numMeas = currPage.measures.size();

        for (SharedResourceProvider.ActionName actName : SharedResourceProvider.ActionName.values()) {
            common.Action act = shResProv.getSharedAction(actName);
            switch (actName) {
                case EDIT_SELECT_ALL:
                    act.setEnabled(numSel < numMeas);
                    break;
                case EDIT_DESELECT_ALL:
                    act.setEnabled(numSel > 0);
                    break;
                case EDIT_INVERT_SELECTION:
                    act.setEnabled(numMeas > 0);
                    break;
                case EDIT_DELETE_SELECTED:
                    act.setEnabled(numSel > 0);
                    break;
                default:
                    act.setEnabled(false);
            }
        }
    }

    @Override
    public void sharedActionListener(SharedResourceProvider.ActionName shAction, ActionEvent event) {
        switch (shAction) {
            case EDIT_SELECT_ALL:
                selectAll();
                break;
            case EDIT_DESELECT_ALL:
                deselectAll();
                break;
            case EDIT_INVERT_SELECTION:
                invertSelected();
                break;
            case EDIT_DELETE_SELECTED:
                deleteSelected();
                break;
            default:
                throw new IllegalStateException();
        }
    }
    
    private void selectAll() {
        currPage.setSelectionFlag(true);
        mainInterface.setStatus("select all.", true);
        mainInterface.invalidateGUI();
    }
    
    private void deselectAll() {       
        currPage.setSelectionFlag(false);
        mainInterface.setStatus("select all.", true);
        mainInterface.invalidateGUI();
    }

    private void invertSelected() {       
        for (AbstractMeasure m : currPage.measures)
            m.setSelected(!m.isSelected());
        mainInterface.setStatus("selection inverted.", true);
        mainInterface.invalidateGUI();
    }

    private void deleteSelected() {
        mainInterface.executeUndoableCommand("delete selected.", (ProjectData proj, ProjectPage page) -> {
            MeasurePage mp = (MeasurePage)page;
            Iterator<AbstractMeasure> it = mp.measures.iterator();
            while (it.hasNext())
                if (it.next().isSelected())
                    it.remove();
        });
    }

    @Override
    public PageErrorWarning[] getAllErrorsAndWarnings() {
        return currPage.getAllErrorsAndWarnings();
    }
    
    @Override
    public void onNewMeasureList(final ArrayList<AbstractMeasure> newList) {
        mainInterface.executeUndoableCommand("measure changed.", (ProjectData proj, ProjectPage page) -> {
            //System.out.println("onNewMeasureList!");
            MeasurePage mpage = (MeasurePage) page;
            mpage.measures = newList;
        });
    }

    @Override
    public void onMeasureListSelectionChanged(int num, boolean isSelected) {
        currPage.measures.get(num).setSelected(isSelected);
        int numSel = currPage.countSelectedMeasures();
        mainInterface.setStatus("selection changed.", true);
        mainInterface.invalidateGUI();
    }

    @Override
    public void onNewListAssignment(final VarListMultiAssignment newList) {
        mainInterface.executeUndoableCommand("variable assignment changed.", (ProjectData proj, ProjectPage page) -> {
            MeasurePage mpage = (MeasurePage)page;
            mpage.varListAssignments = newList;
        });
    }

    @Override
    public void onSolverParamsModified(final SolverParams newParams) {
        mainInterface.executeUndoableCommand("solver parameters changed.", (ProjectData proj, ProjectPage page) -> {
            MeasurePage mpage = (MeasurePage)page;
            mpage.solverParams = newParams;
        });
    }

    // Editing the list of template parameters
    @Override
    public void onEditingListAssignment() {
        updateEditing();
    }

    // Editing the solver parameters
    @Override
    public void onEditingSolverParams() {
        updateEditing();
    }

    // Editing the measured formulas
    @Override
    public void onFormulaListEditing() {
        updateEditing();
    }
    
    void updateEditing() {
//        System.out.println(System.currentTimeMillis()+
//                           " has-gspn: "+(currPage.targetGspn!=null)+
//                           " template-vars: "+bindingForms.areAllBindingsValid()+
//                           "  solver: "+solverPanel.areParamsCorrect()+
//                           "  measures: "+formulaForm.areAllFormulasValid());
        boolean paramsOk = (currPage.targetGspn != null) &&
                           currPage.allDepsAreCorrect &&
                           bindingForms.areAllBindingsValid() &&
                           solverPanel.areParamsCorrect();
        boolean allOk = paramsOk && formulaForm.areAllFormulasValid(currPage.solverParams);
        
        jButtonComputeAll.setEnabled(allOk && currPage.measures.size() > 0);
        actionViewLastLog.setEnabled(!currPage.lastLog.isNull());
        formulaForm.setComputeButtons(paramsOk, currPage.solverParams);
    }

    @Override
    public void onFormulaCompute(int formulaNum) {
        // Compute the selected formula
        computeMeasures(new AbstractMeasure[]{currPage.measures.get(formulaNum)});
    }
    
    private void computeMeasures(final AbstractMeasure[] measures) {
        assert currPage.targetGspn != null;
        mainInterface.executeUndoableCommand("compute results.", (ProjectData proj, ProjectPage page) -> {
            try {
                // Reset the result tables of the measures
                for (AbstractMeasure m : measures)
                    m.setResults(new ResultTable());
                // Prepare the evaluation context
                ParserContext context = new ParserContext(currPage.targetGspn);
                context.templateVarsBinding = new TemplateBinding();
                context.bindingContext = context; // Ask itself for var bindings
                
                // Prepare the tool invokation
                SolverDialog dlg = new SolverDialog(mainInterface.getWindowFrame(), true);
                SolverInvokator solver = currPage.solverParams.makeNewSolver();
                solver.invoke(currPage, measures, context, currProject.getFilename(), dlg);
                dlg.setVisible(true);
                currPage.lastLog.setRef(new DocumentResource(dlg.getComputationLog()));
            }
            catch (MeasureEvaluationException mee) {
                JOptionPane.showMessageDialog(mainInterface.getWindowFrame(),
                        mee, "Error during measure evaluation.",
                        JOptionPane.ERROR_MESSAGE);
                mee.printStackTrace();
                throw new NoOpException();
            }
        });
    }

    private static ResultTableDialog resultTableDialog;
    private static PetriNetMeasuresPanel netAllMeasurePanel;
    @Override
    public void onViewFormulaResults(int formulaNum) {
        AbstractMeasure m = currPage.measures.get(formulaNum);
        if (m instanceof FormulaMeasure) {
            FormulaMeasure fm = (FormulaMeasure)m;
            if (fm.getLanguage() == FormulaLanguage.ALL) {
                if (netAllMeasurePanel == null)
                    netAllMeasurePanel = new PetriNetMeasuresPanel(mainInterface);
                // Activate the modal panel
                MainWindowInterface wnd = mainInterface;
                wnd.switchToProjectPage(currProject, currPage, netAllMeasurePanel);
                wnd.setStatus(null, true);
                wnd.invalidateGUI();
                return;
            }
            else if (fm.getLanguage() == FormulaLanguage.RG || 
                     fm.getLanguage() == FormulaLanguage.TRG ||
                     fm.getLanguage() == FormulaLanguage.DD ||
                     fm.getLanguage() == FormulaLanguage.INC) {
                // If it is a single result, show the PDF of the (T)RG immediately.
                // Otherwise, ask which TRG to show.
                PdfResultEntry pdf = null;
                if (fm.getResults().table.size() == 1) {
                    pdf = (PdfResultEntry)fm.getResults().table.get(0);
                    Main.viewPDF(pdf.getPdfFile());
                }
                else {
                    //Create the popup menu with all the computed (T)RGs.
                    final JPopupMenu popup = new JPopupMenu();
                    for (final ResultEntry re : fm.getResults().table) {
                        JMenuItem item = new JMenuItem(fm.getLanguage().name()+" for  "+re.assign.getBindingRepr());
                        item.addActionListener(new ActionListener() {
                            @Override
                            public void actionPerformed(ActionEvent e) {
                                PdfResultEntry pdf = (PdfResultEntry)re;
                                Main.viewPDF(pdf.getPdfFile());
                            }
                        });
                        item.setEnabled(re.isComputedOk());
                        popup.add(item);
                    }
                    Util.reformatMenuPanels(popup);
                    JButton clicked = formulaForm.getViewResultsButtonOf(formulaNum);
                    popup.show(clicked, 0, clicked.getHeight());
                }
            }
            else if (fm.getLanguage() == FormulaLanguage.STAT) {
                if (fm.getResults().table.size() == 1) {
                    StatResultEntry stat = (StatResultEntry)fm.getResults().table.get(0);
                    StatWindow w = new StatWindow(mainInterface.getWindowFrame());
                    w.showFor(stat);
                }
                else {
                    // Popup for the instance, than open the stat window.
                    final JPopupMenu popup = new JPopupMenu();
                    for (final ResultEntry re : fm.getResults().table) {
                        JMenuItem item = new JMenuItem("Show statistics for  "+re.assign.getBindingRepr());
                        item.addActionListener(new ActionListener() {
                            @Override
                            public void actionPerformed(ActionEvent e) {
                                StatResultEntry stat = (StatResultEntry)re;
                                StatWindow w = new StatWindow(mainInterface.getWindowFrame());
                                w.showFor(stat);
                            }
                        });
                        item.setEnabled(re.isComputedOk());
                        popup.add(item);
                    }
                    Util.reformatMenuPanels(popup);
                    JButton clicked = formulaForm.getViewResultsButtonOf(formulaNum);
                    popup.show(clicked, 0, clicked.getHeight());
                }
            }
            else {
                // Show a table of results
                if (resultTableDialog == null)
                    resultTableDialog = new ResultTableDialog(mainInterface.getWindowFrame(), true);
                resultTableDialog.showModalDialog(currPage, formulaNum);
            }
        }
        else throw new UnsupportedOperationException();
    }
    
    //--------------------------------------------------------------------------
    private class JMeasureList extends JPanel implements Scrollable, 
            CutCopyPasteEngine.CutCopyPasteActivation,
            CutCopyPasteEngine.CutCopyPasteActuator 
    {

        public JMeasureList() {
            super(new GridLayout());
            add(jPanelScrollContainer);
            CutCopyPasteEngine.setupCutCopyPaste(this, this);
        }

        @Override
        public ProjectPage getCurrPage() {
            return currPage;
        }

        @Override
        public MainWindowInterface getMainInterface() {
            return mainInterface;
        }
        
        @Override
        public boolean canCut() {
            return currPage!=null && currPage.canCutOrCopy();
        }

        @Override
        public boolean canCopy() {
            return currPage!=null && currPage.canCutOrCopy();
        }

        @Override
        public boolean canPaste() {
            if (currPage == null)
                    return false;
            Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
            try {
                if (clipboard.isDataFlavorAvailable(currPage.getDataFlavour()))
                    return true;
            }
            catch (Exception e) { Main.logException(e, true); }
            return false;
        }
        
        @Override
        public Dimension getPreferredScrollableViewportSize() { 
            return getPreferredSize(); 
        }
        @Override
        public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) { return 12; }
        @Override
        public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction) {
            if (orientation == SwingConstants.HORIZONTAL)
                return (int)visibleRect.getWidth();
            return (int)visibleRect.getHeight();
        }
        @Override
        public boolean getScrollableTracksViewportWidth() { 
            return true; /* resize the panel to occupy all the width of the extent. */ 
        }
        @Override
        public boolean getScrollableTracksViewportHeight() { return false;}
    }
    
    //--------------------------------------------------------------------------
    @Override
    public JComponent getEditorComponent() {
        return this;
    }

    @Override
    public JComponent getFocusTarget() {
        return measList;
    }

    @Override
    public boolean isModalEditor() {
        return false;
    }

    @Override
    public JComponent[] getToolbars() {
        return new JComponent[]{};
    }

    @Override
    public boolean isZoomPanelUsed() {
        return false;
    }

    private final JPanel propertyPanel = new JPanel();
    @Override
    public JComponent getPropertyPanel() {
        return propertyPanel;
    }
    
    

    @Override
    public int getPageZoom() {
        return 100;
    }

    @Override
    public void zoomChanged(int newZoomLevel) {
        // Do nothing
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

        jPanelScrollContainer = new javax.swing.JPanel();
        jPanelGspn = new javax.swing.JPanel();
        jLabel_gspn = new javax.swing.JLabel();
        jComboBox_gspn = new javax.swing.JComboBox<>();
        jLabel_solver = new javax.swing.JLabel();
        jComboBox_solver = new javax.swing.JComboBox<>();
        jPanel_filler = new javax.swing.JPanel();
        jPanelTemplateParamsTitle = new javax.swing.JPanel();
        jPanelTemplateParams = new javax.swing.JPanel();
        jPanelSolverParamsTitle = new javax.swing.JPanel();
        jPanelSolverParams = new javax.swing.JPanel();
        jPanelMeasuresTitle = new javax.swing.JPanel();
        jPanelMeasures = new javax.swing.JPanel();
        jPanelBottomButtons = new javax.swing.JPanel();
        jButtonComputeAll = new javax.swing.JButton();
        jButtonViewLastLog = new javax.swing.JButton();
        actionAddMeasure = new common.Action();
        actionCommentMeasure = new common.Action();
        actionMoveUp = new common.Action();
        actionMoveDown = new common.Action();
        actionComputeAll = new common.Action();
        actionViewLastLog = new common.Action();
        actionExportExcel = new common.Action();
        resourceFactory = new editor.gui.ResourceFactory();
        jToolBar = new javax.swing.JToolBar();
        jToolbarButton_addMeasure = new common.JToolbarButton();
        jToolbarButton_commentMeasure = new common.JToolbarButton();
        jToolbarButton_moveUp = new common.JToolbarButton();
        jToolbarButton_moveDown = new common.JToolbarButton();
        jToolbarButton_exportExcel = new common.JToolbarButton();
        jScrollPaneList = new javax.swing.JScrollPane();

        jPanelScrollContainer.setLayout(new java.awt.GridBagLayout());

        jPanelGspn.setLayout(new java.awt.GridBagLayout());

        jLabel_gspn.setText("Target model:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 6, 3, 3);
        jPanelGspn.add(jLabel_gspn, gridBagConstraints);

        jComboBox_gspn.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jComboBox_gspnActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelGspn.add(jComboBox_gspn, gridBagConstraints);

        jLabel_solver.setText("Solver:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 30, 3, 3);
        jPanelGspn.add(jLabel_solver, gridBagConstraints);

        jComboBox_solver.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jComboBox_solverActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 6);
        jPanelGspn.add(jComboBox_solver, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        jPanelGspn.add(jPanel_filler, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.insets = new java.awt.Insets(3, 5, 3, 5);
        jPanelScrollContainer.add(jPanelGspn, gridBagConstraints);

        jPanelTemplateParamsTitle.setBorder(javax.swing.BorderFactory.createTitledBorder("Template parameters:"));
        jPanelTemplateParamsTitle.setLayout(new java.awt.GridBagLayout());

        jPanelTemplateParams.setBorder(javax.swing.BorderFactory.createEmptyBorder(2, 3, 2, 3));
        jPanelTemplateParams.setLayout(new javax.swing.BoxLayout(jPanelTemplateParams, javax.swing.BoxLayout.LINE_AXIS));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        jPanelTemplateParamsTitle.add(jPanelTemplateParams, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.3;
        gridBagConstraints.insets = new java.awt.Insets(3, 5, 3, 5);
        jPanelScrollContainer.add(jPanelTemplateParamsTitle, gridBagConstraints);

        jPanelSolverParamsTitle.setBorder(javax.swing.BorderFactory.createTitledBorder("Solver parameters:"));
        jPanelSolverParamsTitle.setLayout(new java.awt.GridBagLayout());

        jPanelSolverParams.setBorder(javax.swing.BorderFactory.createEmptyBorder(2, 3, 2, 3));
        jPanelSolverParams.setLayout(new javax.swing.BoxLayout(jPanelSolverParams, javax.swing.BoxLayout.LINE_AXIS));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        jPanelSolverParamsTitle.add(jPanelSolverParams, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 5, 3, 5);
        jPanelScrollContainer.add(jPanelSolverParamsTitle, gridBagConstraints);

        jPanelMeasuresTitle.setBorder(javax.swing.BorderFactory.createTitledBorder("Measures:"));
        jPanelMeasuresTitle.setLayout(new java.awt.GridBagLayout());

        jPanelMeasures.setBorder(javax.swing.BorderFactory.createEmptyBorder(2, 3, 2, 3));
        jPanelMeasures.setLayout(new javax.swing.BoxLayout(jPanelMeasures, javax.swing.BoxLayout.LINE_AXIS));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        jPanelMeasuresTitle.add(jPanelMeasures, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 5, 3, 5);
        jPanelScrollContainer.add(jPanelMeasuresTitle, gridBagConstraints);

        jPanelBottomButtons.setLayout(new java.awt.GridBagLayout());

        jButtonComputeAll.setAction(actionComputeAll);
        jButtonComputeAll.setText("Compute all");
        jButtonComputeAll.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonComputeAllActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.weightx = 0.1;
        jPanelBottomButtons.add(jButtonComputeAll, gridBagConstraints);

        jButtonViewLastLog.setAction(actionViewLastLog);
        jButtonViewLastLog.setText("View log");
        jButtonViewLastLog.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonViewLastLogActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        jPanelBottomButtons.add(jButtonViewLastLog, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 5, 3, 5);
        jPanelScrollContainer.add(jPanelBottomButtons, gridBagConstraints);

        actionAddMeasure.setActionName("Add measure");
        actionAddMeasure.setIcon(resourceFactory.getMeasureNew24());
        actionAddMeasure.setTooltipDesc("Adds a new measure to the measure list.");
        actionAddMeasure.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddMeasureActionPerformed(evt);
            }
        });

        actionCommentMeasure.setActionName("Comment...");
        actionCommentMeasure.setIcon(resourceFactory.getComment24());
        actionCommentMeasure.setTooltipDesc("Add or remove a comment on the selected measures.");
        actionCommentMeasure.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionCommentMeasureActionPerformed(evt);
            }
        });

        actionMoveUp.setActionName("Move up");
        actionMoveUp.setIcon(resourceFactory.getArrowUp16());
        actionMoveUp.setTooltipDesc("Move up the selected measures.");
        actionMoveUp.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionMoveUpActionPerformed(evt);
            }
        });

        actionMoveDown.setActionName("Move down.");
        actionMoveDown.setIcon(resourceFactory.getArrowDown16());
        actionMoveDown.setTooltipDesc("Move down the selected measures.");
        actionMoveDown.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionMoveDownActionPerformed(evt);
            }
        });

        actionComputeAll.setActionName("Compute All");
        actionComputeAll.setTooltipDesc("Compute all the measures.");

        actionViewLastLog.setActionName("View log...");
        actionViewLastLog.setTooltipDesc("View the log of the last computation.");

        actionExportExcel.setActionName("Export Excel");
        actionExportExcel.setIcon(resourceFactory.getExportExcel24());
        actionExportExcel.setTooltipDesc("Export the results of the selected measures in Excel format.");
        actionExportExcel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionExportExcelActionPerformed(evt);
            }
        });

        setLayout(new java.awt.BorderLayout());

        jToolBar.setFloatable(false);
        jToolBar.setRollover(true);

        jToolbarButton_addMeasure.setAction(actionAddMeasure);
        jToolbarButton_addMeasure.setText("Add measure");
        jToolBar.add(jToolbarButton_addMeasure);

        jToolbarButton_commentMeasure.setAction(actionCommentMeasure);
        jToolbarButton_commentMeasure.setText("Comment");
        jToolBar.add(jToolbarButton_commentMeasure);

        jToolbarButton_moveUp.setAction(actionMoveUp);
        jToolbarButton_moveUp.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButton_moveUp.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToolbarButton_moveUp);

        jToolbarButton_moveDown.setAction(actionMoveDown);
        jToolbarButton_moveDown.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButton_moveDown.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToolbarButton_moveDown);

        jToolbarButton_exportExcel.setAction(actionExportExcel);
        jToolbarButton_exportExcel.setText("Export Excel");
        jToolBar.add(jToolbarButton_exportExcel);

        add(jToolBar, java.awt.BorderLayout.NORTH);
        add(jScrollPaneList, java.awt.BorderLayout.CENTER);
    }// </editor-fold>//GEN-END:initComponents

    private void jComboBox_gspnActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jComboBox_gspnActionPerformed
        if (updatingGspnCombo)
            return;
        final String newGspn = (String)jComboBox_gspn.getSelectedItem();
        if (newGspn.equals(UNSELECTED_GSPN))
            return;
        if (currPage.targetGspnName!=null && newGspn.equals(currPage.targetGspnName))
            return; // nothing changed
        
        mainInterface.executeUndoableCommand("change measured GSPN.", (ProjectData proj, ProjectPage page) -> {
            currPage.targetGspnName = newGspn;
        });
    }//GEN-LAST:event_jComboBox_gspnActionPerformed

    private void actionCommentMeasureActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionCommentMeasureActionPerformed
        mainInterface.executeUndoableCommand("toggle measure comments.", (ProjectData proj, ProjectPage page) -> {
            MeasurePage mpage = (MeasurePage)page;
            boolean allShown = true;
            for (AbstractMeasure m : mpage.measures)
                if (m.isSelected())
                    allShown = allShown && m.isCommentShown();
            // Toggle comment flag
            for (AbstractMeasure m : mpage.measures)
                if (m.isSelected())
                    m.setCommentShown(!allShown);
        });
    }//GEN-LAST:event_actionCommentMeasureActionPerformed

    private void jComboBox_solverActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jComboBox_solverActionPerformed
        if (initializing)
            return;
        Object selObj = jComboBox_solver.getSelectedItem();
        final SolverParams.Solvers sol = (SolverParams.Solvers)selObj;
        if (sol == currPage.solverParams.getSolver())
            return; // nothing changed
        mainInterface.executeUndoableCommand("solver changed.", (ProjectData proj, ProjectPage page) -> {
            MeasurePage mpage = (MeasurePage)page;
            mpage.solverParams = (SolverParams)sol.getParamsClass().newInstance();
        });
    }//GEN-LAST:event_jComboBox_solverActionPerformed

    private void actionMoveUpActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionMoveUpActionPerformed
        mainInterface.executeUndoableCommand("move up selection.", (ProjectData proj, ProjectPage page) -> {
            MeasurePage mpage = (MeasurePage)page;
            for (int i=1; i<mpage.measures.size(); i++)
                if (mpage.measures.get(i).isSelected()) {
                    AbstractMeasure am = mpage.measures.get(i-1);
                    mpage.measures.set(i-1, mpage.measures.get(i));
                    mpage.measures.set(i, am);
                }
        });
    }//GEN-LAST:event_actionMoveUpActionPerformed

    private void actionMoveDownActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionMoveDownActionPerformed
        mainInterface.executeUndoableCommand("move down selection.", (ProjectData proj, ProjectPage page) -> {
            MeasurePage mpage = (MeasurePage)page;
            for (int i=mpage.measures.size()-2; i>=0; i--)
                if (mpage.measures.get(i).isSelected()) {
                    AbstractMeasure am = mpage.measures.get(i+1);
                    mpage.measures.set(i+1, mpage.measures.get(i));
                    mpage.measures.set(i, am);
                }
        });
    }//GEN-LAST:event_actionMoveDownActionPerformed

    private void actionAddMeasureActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddMeasureActionPerformed
        //Create the popup menu.
        final JPopupMenu popup = new JPopupMenu();
        for (final FormulaLanguage fl : FormulaLanguage.values()) {
            JMenuItem item = new JMenuItem(fl.getName(), fl.getIcon24());
            item.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    mainInterface.executeUndoableCommand("new measure", (ProjectData proj, ProjectPage page) -> {
                        ((MeasurePage)page).measures.add(new FormulaMeasure("", false, "", fl));
                    });
                }
            });
            if (null != currPage.solverParams.isLanguageSupported(fl))
                item.setEnabled(false);
            popup.add(item);
        }
        Util.reformatMenuPanels(popup);
        popup.show(jToolbarButton_addMeasure, 0, jToolbarButton_addMeasure.getHeight());
    }//GEN-LAST:event_actionAddMeasureActionPerformed

    private void jButtonComputeAllActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonComputeAllActionPerformed
        // Compute all the formulas
        AbstractMeasure[] meas = new AbstractMeasure[currPage.measures.size()];
        int n = 0;
        for (AbstractMeasure am : currPage.measures)
            meas[n++] = am;
        computeMeasures(meas);
    }//GEN-LAST:event_jButtonComputeAllActionPerformed

    private void jButtonViewLastLogActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonViewLastLogActionPerformed
        if (!currPage.lastLog.isNull()) {
            SolverDialog dlg = new SolverDialog(mainInterface.getWindowFrame(), true);
            dlg.setComputationLog(currPage.lastLog.resRef().docData);
            dlg.setVisible(true);
        }
    }//GEN-LAST:event_jButtonViewLastLogActionPerformed

    private void actionExportExcelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionExportExcelActionPerformed
        ArrayList<ExcelXml.WorkSheet> ws = new ArrayList<>();
        int numSel = currPage.countSelectedMeasures();
        String title = null;
        for (AbstractMeasure m : currPage.measures) {
            if (m.isSelected()) {
                m.getResultsAsWorkSheets(currPage.varListAssignments, ws);
                
                if (numSel == 1) // title for this single selected measure
                    title = m.getSuggestedWorkSheetTitle();
            }
        }
        if (title == null)
            title = "Results of selected measures";
        ExcelXml.exportWorkSheets(ws, title, this);
    }//GEN-LAST:event_actionExportExcelActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private common.Action actionAddMeasure;
    private common.Action actionCommentMeasure;
    private common.Action actionComputeAll;
    private common.Action actionExportExcel;
    private common.Action actionMoveDown;
    private common.Action actionMoveUp;
    private common.Action actionViewLastLog;
    private javax.swing.JButton jButtonComputeAll;
    private javax.swing.JButton jButtonViewLastLog;
    private javax.swing.JComboBox<String> jComboBox_gspn;
    private javax.swing.JComboBox<SolverParams.Solvers> jComboBox_solver;
    private javax.swing.JLabel jLabel_gspn;
    private javax.swing.JLabel jLabel_solver;
    private javax.swing.JPanel jPanelBottomButtons;
    private javax.swing.JPanel jPanelGspn;
    private javax.swing.JPanel jPanelMeasures;
    private javax.swing.JPanel jPanelMeasuresTitle;
    private javax.swing.JPanel jPanelScrollContainer;
    private javax.swing.JPanel jPanelSolverParams;
    private javax.swing.JPanel jPanelSolverParamsTitle;
    private javax.swing.JPanel jPanelTemplateParams;
    private javax.swing.JPanel jPanelTemplateParamsTitle;
    private javax.swing.JPanel jPanel_filler;
    private javax.swing.JScrollPane jScrollPaneList;
    private javax.swing.JToolBar jToolBar;
    private common.JToolbarButton jToolbarButton_addMeasure;
    private common.JToolbarButton jToolbarButton_commentMeasure;
    private common.JToolbarButton jToolbarButton_exportExcel;
    private common.JToolbarButton jToolbarButton_moveDown;
    private common.JToolbarButton jToolbarButton_moveUp;
    private editor.gui.ResourceFactory resourceFactory;
    // End of variables declaration//GEN-END:variables
}
