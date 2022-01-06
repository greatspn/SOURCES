/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.composition;

import common.Action;
import common.Condition;
import common.Util;
import editor.Main;
import editor.domain.DrawHelper;
import editor.domain.NetPage;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import editor.domain.elements.GspnPage;
import editor.domain.measures.ExprField;
import editor.domain.measures.SolverParams;
import editor.domain.unfolding.Algebra2;
import editor.gui.AbstractPageEditor;
import editor.gui.CutCopyPasteEngine;
import editor.gui.MainWindowInterface;
import editor.gui.PagePrintExportManager;
import editor.gui.SharedResourceProvider;
import editor.gui.net.IndeterminateListCellRenderer;
import editor.gui.net.NetViewerPanel;
import editor.gui.net.ShowNetMatricesDialog;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Iterator;
import javax.swing.ActionMap;
import javax.swing.DefaultListModel;
import javax.swing.InputMap;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import static javax.swing.JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.ListCellRenderer;
import javax.swing.UIManager;
import javax.swing.border.LineBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 *
 * @author elvio
 */
public class MultiNetEditorPanel extends javax.swing.JPanel implements AbstractPageEditor {

    private boolean initializing = true;
    private ProjectFile currProject = null;
    private MultiNetPage currPage = null;
    private MainWindowInterface mainInterface = null;
    
    // The side-by-side nets viewer
    private final MultiNetViewerPanel viewerPanel;
    
    // List of net instance editor panels
    private final ArrayList<NetInstanceDescriptorPanel> instanceEditors = new ArrayList<>();
    
    // Panel that will contain the multiNet definition
    private final NetInstanceEditorTable editorTable;
    
    private final SolverParams.IntExpr alignDxCopy, alignDyCopy;
    
    private final SolverParams.IntExpr[] dxMultCopy;
    private final SolverParams.IntExpr[] dyMultCopy;
    private final ExprField[] exprFieldDx;
    private final ExprField[] exprFieldDy;

    /**
     * Creates new form MultiNetEditorPanel
     * @param shActProv
     * @param actionCondition
     */
    public MultiNetEditorPanel(SharedResourceProvider shActProv, Condition actionCondition) {
        initializing = true;
        initComponents();
        editorTable = new NetInstanceEditorTable();
        panel_netInstanceEditor.add(editorTable);
        viewerPanel = new MultiNetViewerPanel(scrollPaneCentral);
        scrollPaneCentral.setViewportView(viewerPanel);
        Util.reformatPanelsForPlatformLookAndFeel(this);
//        Util.reformatPanelsForPlatformLookAndFeel(scrollPaneCentral);
        Util.reformatPanelsForPlatformLookAndFeel(propertyPanel);
//        if (Util.useUnifiedToolbar()) {
//            propertyPanel.setOpaque(true);
//            propertyPanel.setBackground(Util.UNIFIED_GRAY_PANEL_BKGND);
//        }
        
        scrollPaneCentral.getViewport().setBackground(getBackground());
        
        scrollPaneCentral.getViewport().addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                if (currPage != null)
                    currPage.viewportPos = scrollPaneCentral.getViewport().getViewPosition();
            }
        });
        
        // Tag-based composition Property Panel
        list_TagsP.setCellRenderer(new CellRenderer());
        list_TagsT.setCellRenderer(new CellRenderer());
        list_TagsP2.setCellRenderer(new CellRenderer());
        list_TagsT2.setCellRenderer(new CellRenderer());
        
        Dimension minSize = scrollPane_TagsP.getPreferredSize();
        int minHeight = jLabel1.getFontMetrics(jLabel1.getFont()).getHeight() * 8;
        minSize.height = Math.max(minSize.height, minHeight);
        minSize.width = 1;
//        scrollPane_TagsP.setMinimumSize(minSize);
        scrollPane_TagsP.setPreferredSize(minSize);
        scrollPane_TagsT.setPreferredSize(minSize);
        
        scrollPane_TagsP2.setPreferredSize(minSize);
        scrollPane_TagsT2.setPreferredSize(minSize);
        
        alignDxCopy = new SolverParams.IntExpr("1");
        alignDyCopy = new SolverParams.IntExpr("1");
        exprField_algebraDx.setExprListener(new ExprField.ExprFieldListener() {
            @Override public void onExprModified() {
                mainInterface.executeUndoableCommand("change dx offset.", (ProjectData proj, ProjectPage elem) -> {
                    TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
                    acp.alignDx.setExpr(alignDxCopy.getExpr());
                });
            }
            @Override public void onEditingText() { }
        });
        exprField_algebraDy.setExprListener(new ExprField.ExprFieldListener() {
            @Override public void onExprModified() {
                mainInterface.executeUndoableCommand("change dy offset.", (ProjectData proj, ProjectPage elem) -> {
                    TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
                    acp.alignDy.setExpr(alignDyCopy.getExpr());
                });
            }
            @Override public void onEditingText() { }
        });
        
        exprField_algebraDx2.setExprListener(new ExprField.ExprFieldListener() {
            @Override public void onExprModified() {
                mainInterface.executeUndoableCommand("change dx offset.", (ProjectData proj, ProjectPage elem) -> {
                    TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
                    acp.alignDx.setExpr(alignDxCopy.getExpr());
                });
            }
            @Override public void onEditingText() { }
        });
        exprField_algebraDy2.setExprListener(new ExprField.ExprFieldListener() {
            @Override public void onExprModified() {
                mainInterface.executeUndoableCommand("change dy offset.", (ProjectData proj, ProjectPage elem) -> {
                    TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
                    acp.alignDy.setExpr(alignDyCopy.getExpr());
                });
            }
            @Override public void onEditingText() { }
        });
        
        
        dxMultCopy = new SolverParams.IntExpr[UnfoldingCompositionPage.NUM_OFFSET_ROWS];
        dyMultCopy = new SolverParams.IntExpr[UnfoldingCompositionPage.NUM_OFFSET_ROWS];
        for (int i=0; i<dxMultCopy.length; i++) {
            dxMultCopy[i] = new SolverParams.IntExpr("1");
            dyMultCopy[i] = new SolverParams.IntExpr("1");
        }
        exprFieldDx = new ExprField[] { exprField_unfDxMult1, exprField_unfDxMult2, exprField_unfDxMult3, exprField_unfDxMult4 };
        exprFieldDy = new ExprField[] { exprField_unfDyMult1, exprField_unfDyMult2, exprField_unfDyMult3, exprField_unfDyMult4 };
        for (int i=0; i<4; i++) {
            final int ii = i;
            exprFieldDx[ii].setExprListener(new ExprField.ExprFieldListener() {
                @Override public void onExprModified() {
                    mainInterface.executeUndoableCommand("change dx multiplier.", (ProjectData proj, ProjectPage elem) -> {
                        UnfoldingCompositionPage ucp = (UnfoldingCompositionPage)currPage;
                        ucp.dxMult[ii].setExpr(dxMultCopy[ii].getExpr());
                    });
                }
                @Override public void onEditingText() { }
            });
            exprFieldDy[ii].setExprListener(new ExprField.ExprFieldListener() {
                @Override public void onExprModified() {
                    mainInterface.executeUndoableCommand("change dy multiplier.", (ProjectData proj, ProjectPage elem) -> {
                        UnfoldingCompositionPage ucp = (UnfoldingCompositionPage)currPage;
                        ucp.dyMult[ii].setExpr(dyMultCopy[ii].getExpr());
                    });
                }
                @Override public void onEditingText() { }
            });
        }
        
        comboBox_semantics.setRenderer(new IndeterminateListCellRenderer());
        for (Algebra2.Semantics sem : Algebra2.Semantics.values())
            comboBox_semantics.addItem(sem);

        
        // Add actions to the input map manually
        InputMap inMap = getInputMap(WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        ActionMap actMap = getActionMap();
        Action.registerAllActions(actMap, inMap, this, getClass(), actionCondition);
        
        initializing = false;
    }

     protected class CellRenderer implements ListCellRenderer<JCheckBox> {
        public Component getListCellRendererComponent(
                JList<? extends JCheckBox> list, JCheckBox value, int index,
                boolean isSelected, boolean cellHasFocus) {
            JCheckBox checkbox = value;

            //Drawing checkbox, change the appearance here
//      checkbox.setBackground(isSelected ? getSelectionBackground()
//          : getBackground());
//      checkbox.setForeground(isSelected ? getSelectionForeground()
//          : getForeground());
            checkbox.setEnabled(isEnabled());
            checkbox.setFont(getFont());
            checkbox.setFocusPainted(false);
            checkbox.setBorderPainted(false);
            checkbox.setBackground(UIManager.getColor("List.background"));
//      checkbox.setBorder(isSelected ? UIManager
//          .getBorder("List.focusCellHighlightBorder") : UIManager);
            return checkbox;
        }
    }

    @Override
    public void setEditorEnabledFor(ProjectFile pf, ProjectPage page, MainWindowInterface mainInterface) {
        assert page != null && page instanceof MultiNetPage;
        currProject = pf;
        currPage = (MultiNetPage)page;
        this.mainInterface = mainInterface;
        
        updateGUI(pf, page);
    }

    @Override
    public void setEditorDisabled() {
        assert currProject != null;
        currProject = null;
        currPage = null;
        mainInterface = null;
        
        for (NetInstanceDescriptorPanel panel : instanceEditors)
            panel.deinitialize();
        editorTable.removeAll();
        instanceEditors.clear();
        
        exprField_algebraDx.deinitialize();
        exprField_algebraDy.deinitialize();
        exprField_algebraDx2.deinitialize();
        exprField_algebraDy2.deinitialize();
        for (ExprField ef : exprFieldDx)
            ef.deinitialize();
        for (ExprField ef : exprFieldDy)
            ef.deinitialize();
    }

    @Override
    public void updateGUI(ProjectFile pf, ProjectPage page) {
        assert pf == currProject;
        currPage = (MultiNetPage)page;
        initializing = true;
        
        toolbarButton_addSubnet.setEnabled(!currPage.hasFixedNumOfOperators());
        label_operator.setIcon(currPage.getOperatorIcon());
        label_operator.setText(currPage.getOperatorName());
        
        //-----------------------------------------
        // Algebra Property Panel
        if (currPage instanceof TagBasedCompositionPage) {
            TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
            
            DefaultListModel<JCheckBox> modelP = new DefaultListModel<JCheckBox>();
            if (acp.commonTagsP != null) {
                for (String tag : acp.commonTagsP) {
                    JCheckBox elem = new JCheckBox(tag, acp.selTagsP.contains(tag));
                    modelP.addElement(elem);
                }
            }
            list_TagsP.setModel(modelP);
            
            DefaultListModel<JCheckBox> modelT = new DefaultListModel<JCheckBox>();
            if (acp.commonTagsT != null) {
                for (String tag : acp.commonTagsT) {
                    JCheckBox elem = new JCheckBox(tag, acp.selTagsT.contains(tag));
                    modelT.addElement(elem);
                }
            }
            list_TagsT.setModel(modelT);
            toggle_horizontal.setSelected(acp.alignment == TagBasedCompositionPage.Alignment.HORIZONTAL);
            toggle_vertical.setSelected(acp.alignment == TagBasedCompositionPage.Alignment.VERTICAL);
            toggle_custom.setSelected(acp.alignment == TagBasedCompositionPage.Alignment.CUSTOM);
            
            alignDxCopy.setExpr(acp.alignDx.getExpr());
            exprField_algebraDx.initializeFor(alignDxCopy.getEditableValue(), page);
            alignDyCopy.setExpr(acp.alignDy.getExpr());
            exprField_algebraDy.initializeFor(alignDyCopy.getEditableValue(), page);
            
            exprField_algebraDx.setEnabled(acp.alignment == TagBasedCompositionPage.Alignment.CUSTOM);
            exprField_algebraDy.setEnabled(acp.alignment == TagBasedCompositionPage.Alignment.CUSTOM);
            
            checkBox_useBrokenEdges.setSelected(acp.useBrokenEdges);
            
            panel_algebra.setVisible(true);
        }
        else {
            list_TagsP.removeAll();
            list_TagsT.removeAll();
            exprField_algebraDx.deinitialize();
            exprField_algebraDy.deinitialize();
            panel_algebra.setVisible(false);
        }
        
        //-----------------------------------------
        // Algebra Property Panel (version 2)
        if (currPage instanceof TagBasedCompositionPage2) {
            TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
            
            DefaultListModel<JCheckBox> modelP = new DefaultListModel<JCheckBox>();
            if (acp.commonTagsP != null) {
                for (String tag : acp.commonTagsP) {
                    JCheckBox elem = new JCheckBox(tag, acp.selTagsP.contains(tag));
                    modelP.addElement(elem);
                }
            }
            list_TagsP2.setModel(modelP);
            
            DefaultListModel<JCheckBox> modelT = new DefaultListModel<JCheckBox>();
            if (acp.commonTagsT != null) {
                for (String tag : acp.commonTagsT) {
                    JCheckBox elem = new JCheckBox(tag, acp.selTagsT.contains(tag));
                    modelT.addElement(elem);
                }
            }
            list_TagsT2.setModel(modelT);
            toggle_horizontal2.setSelected(acp.alignment == TagBasedCompositionPage2.Alignment.HORIZONTAL);
            toggle_vertical2.setSelected(acp.alignment == TagBasedCompositionPage2.Alignment.VERTICAL);
            toggle_custom2.setSelected(acp.alignment == TagBasedCompositionPage2.Alignment.CUSTOM);
            
            alignDxCopy.setExpr(acp.alignDx.getExpr());
            exprField_algebraDx2.initializeFor(alignDxCopy.getEditableValue(), page);
            alignDyCopy.setExpr(acp.alignDy.getExpr());
            exprField_algebraDy2.initializeFor(alignDyCopy.getEditableValue(), page);
            
            exprField_algebraDx2.setEnabled(acp.alignment == TagBasedCompositionPage2.Alignment.CUSTOM);
            exprField_algebraDy2.setEnabled(acp.alignment == TagBasedCompositionPage2.Alignment.CUSTOM);
            
            checkBox_useBrokenEdges2.setSelected(acp.useBrokenEdges);
            checkBox_applyRestrictions.setSelected(acp.applyRestrictions);
            
            comboBox_semantics.setSelectedItem(acp.semantics);
            
            panel_algebra2.setVisible(true);
        }
        else {
            list_TagsP2.removeAll();
            list_TagsT2.removeAll();
            exprField_algebraDx2.deinitialize();
            exprField_algebraDy2.deinitialize();
            panel_algebra2.setVisible(false);
        }
        
        //-----------------------------------------
        // Unfolding Property Panel
        if (currPage instanceof UnfoldingCompositionPage) {
            UnfoldingCompositionPage ucp = (UnfoldingCompositionPage)currPage;
            
            for (int i=0; i<4; i++) {
                dxMultCopy[i].setExpr(ucp.dxMult[i].getExpr());
                exprFieldDx[i].initializeFor(dxMultCopy[i].getEditableValue(), page);
                dyMultCopy[i].setExpr(ucp.dyMult[i].getExpr());
                exprFieldDy[i].initializeFor(dyMultCopy[i].getEditableValue(), page);
            }
            
            panel_unfolding.setVisible(true);
        }
        else {
            for (ExprField ef : exprFieldDx)
            ef.deinitialize();
        for (ExprField ef : exprFieldDy)
            ef.deinitialize();
            panel_unfolding.setVisible(false);
        }
        
        //-----------------------------------------
        // Subnet visualization
        if (currPage.areSubnetsVisualizable()) {
            // Flatten list of netpages
//            ArrayList<NetPage> flatNetList = new ArrayList<>();
//            ArrayList<String> flatNetNames = new ArrayList<>();
//            recursiveFlatten(currPage, flatNetList, flatNetNames, "");
//            String[] flatNetNames = currPage.flattenedSubNetNames.toArray
//                        (new String[currPage.flattenedSubNetNames.size()]);
//            NetPage[] flatNetList = currPage.flattenedSubNets.toArray
//                    (new NetPage[currPage.flattenedSubNets.size()]);
            String[] flatNetNames = currPage.getVisualizedSubnetNames();
            NetPage[] flatNetList = currPage.getVisualizedSubnets();
            
            viewerPanel.removeAllNetPanels();
            viewerPanel.initializePanel(flatNetList, flatNetNames,
                                        flatNetList[0], currPage.viewProfile, false);
            viewerPanel.setVisible(true);
        }
        else {
            viewerPanel.setVisible(false);
        }
        
        // Ensure that we have the same number of panel editors as required.
        while (instanceEditors.size() < currPage.netsDescr.size()) {
            NetInstanceDescriptorPanel panel = new NetInstanceDescriptorPanel(instanceEditors.size());
            GridBagConstraints gbc = new GridBagConstraints(0, instanceEditors.size(), 1, 1, 1.0, 1.0, 
                    GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0,0,0,0), 0, 0);
            instanceEditors.add(panel);
            editorTable.add(panel, gbc);
        }
        while (instanceEditors.size() > currPage.netsDescr.size()) {
            editorTable.remove(instanceEditors.get(instanceEditors.size() - 1));
            instanceEditors.remove(instanceEditors.size() - 1);
        }
        assert instanceEditors.size() == currPage.netsDescr.size();
        assert editorTable.getComponentCount() == instanceEditors.size();
        
        // Setup the net instance descriptor editors
        int numInst = 0;
        for (NetInstanceDescriptor descr : currPage.netsDescr) {
            instanceEditors.get(numInst).initialize(mainInterface, pf.getCurrent(), currPage);
            numInst++;
        }
        
        
        //------------------------------
        // Update the graphic window
        invalidate();
        validate();
        repaint();        
        initializing = false;
    }
    
//    private void recursiveFlatten(ComposableNet net, ArrayList<NetPage> flatNetList,
//                                  ArrayList<String> flatNetNames, String namePrefix) 
//    {
//        if (net instanceof NetPage) {
//            flatNetList.add((NetPage)net);
//            flatNetNames.add(namePrefix + net.getPageName());
//        }
//        else {
//            Iterator<ComposableNet> snIt =  net.subnetsIterator();
//            String newPrefix;
//            if (net == currPage)
//                newPrefix = "";
//            else
//                newPrefix = namePrefix + net.getPageName() + ".";
//            while (snIt.hasNext()) {
//                recursiveFlatten(snIt.next(), flatNetList, flatNetNames, newPrefix);
//            }
//        }
//    }

    @Override
    public void updateEnablingOfSharedActions(SharedResourceProvider shResProv) {
        int numSel = currPage.countSelected();
        int numDescr = currPage.netsDescr.size();

        for (SharedResourceProvider.ActionName actName : SharedResourceProvider.ActionName.values()) {
            common.Action act = shResProv.getSharedAction(actName);
            switch (actName) {
                case EDIT_SELECT_ALL:
                    act.setEnabled(numSel < numDescr);
                    break;
                case EDIT_DESELECT_ALL:
                    act.setEnabled(numSel > 0);
                    break;
                case EDIT_INVERT_SELECTION:
                    act.setEnabled(numDescr > 0);
                    break;
                case EDIT_DELETE_SELECTED:
                    act.setEnabled(numSel > 0);
                    break;
                case MAKE_EDITABLE_NET:
                    act.setEnabled(currPage.isPageCorrect());
                    break;
                case EXPORT_GREATSPN_FORMAT:
                case EXPORT_GRML_FORMAT:
                case EXPORT_APNN_FORMAT:
                case EXPORT_PNML_FORMAT:
                    act.setEnabled(currPage.isPageCorrect() && currPage.getComposedNet() instanceof GspnPage);
                    break;
                case EXPORT_AS_PDF:
                case EXPORT_AS_PNG:
                    act.setEnabled(currPage.isPageCorrect());
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
            case MAKE_EDITABLE_NET:
                makeEditableNet();
                break;
            case SHOW_NET_MATRICES:
                showNetMatrices();
                break;
            case EXPORT_GREATSPN_FORMAT:
                PagePrintExportManager.exportGspnInGreatSPNFormat(mainInterface, (GspnPage)currPage.getComposedNet());
                return;
            case EXPORT_PNML_FORMAT:
                PagePrintExportManager.exportGspnInPNMLFormat(mainInterface, (GspnPage)currPage.getComposedNet());
                return;                
            case EXPORT_GRML_FORMAT:
                PagePrintExportManager.exportGspnInGRMLFormat(mainInterface, (GspnPage)currPage.getComposedNet());
                return;                
            case EXPORT_APNN_FORMAT:
                PagePrintExportManager.exportGspnInAPNNFormat(mainInterface, (GspnPage)currPage.getComposedNet());
                return;                        
            case EXPORT_AS_PDF:
                PagePrintExportManager.printAsPdf(mainInterface, currPage.getComposedNet());
                return;                
            case EXPORT_AS_PNG:
                PagePrintExportManager.printAsPng(mainInterface, currPage.getComposedNet());
                return;                
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
        for (NetInstanceDescriptor descr : currPage.netsDescr)
            descr.setSelected(!descr.isSelected());
        mainInterface.setStatus("selection inverted.", true);
        mainInterface.invalidateGUI();
    }

    private void deleteSelected() {
        mainInterface.executeUndoableCommand("delete selected.", (ProjectData proj, ProjectPage page) -> {
            MultiNetPage mnp = (MultiNetPage)page;
            Iterator<NetInstanceDescriptor> it = mnp.netsDescr.iterator();
            while (it.hasNext())
                if (it.next().isSelected())
                    it.remove();
        });
    }
    
    private void makeEditableNet() {
        mainInterface.executeUndoableCommand("duplicate composable net as editable.", (ProjectData proj, ProjectPage elem) -> {
            NetPage newPage = (NetPage)Util.deepCopy(currPage.getComposedNet());
            newPage.setPageName(proj.generateUniquePageName(newPage.getPageName()));
            newPage.viewProfile = (ViewProfile)Util.deepCopy(currPage.viewProfile);
            proj.addPage(newPage);
            mainInterface.switchToProjectPage(currProject, newPage, null);
        });
    }
    
    private void showNetMatrices() {
        ShowNetMatricesDialog dlg = new ShowNetMatricesDialog(mainInterface.getWindowFrame(), true, 
                                                              (GspnPage)currPage.getComposedNet());
        dlg.setVisible(true);
    }
    
    private class NetInstanceEditorTable extends JPanel 
        implements CutCopyPasteEngine.CutCopyPasteActivation,
                   CutCopyPasteEngine.CutCopyPasteActuator
    {
        public NetInstanceEditorTable() {
            setLayout(new GridBagLayout());
            setBorder(new LineBorder(Color.black, 1));
            setBackground(Color.white);
            CutCopyPasteEngine.setupCutCopyPaste(this, this);
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
        public ProjectPage getCurrPage() { return currPage; }

        @Override
        public MainWindowInterface getMainInterface() { return mainInterface; }
    }

    @Override
    public JComponent getFocusTarget() {
        return editorTable;
    }

    @Override
    public PageErrorWarning[] getAllErrorsAndWarnings() {
        return currPage.getAllErrorsAndWarnings();
    }

    @Override
    public JComponent[] getToolbars() {
        return new JComponent[]{};
    }

    @Override
    public boolean isModalEditor() {
        return false;
    }

    @Override
    public boolean isZoomPanelUsed() {
        return true;
    }

    @Override
    public int getPageZoom() {
        return currPage.viewProfile.zoom;
    }

    @Override
    public void zoomChanged(int newZoomLevel) {
        viewerPanel.zoomChanged(newZoomLevel);
    }
    
    @Override
    public JComponent getPropertyPanel() {
        return propertyPanel;
    }
    
    @Override
    public JComponent getEditorComponent() {
        return this;
    }
    
    private class MultiNetViewerPanel extends NetViewerPanel {

        public MultiNetViewerPanel(JScrollPane scrollPane) {
            super(scrollPane);
        }

        @Override
        public void netClicked(JNetPanel panel, NetPage net, MouseEvent evt) {
        }

        @Override
        public String getOverlayMessage(NetPage net) {
            return null;
        }

        @Override
        public void setupDrawContext(NetPage net, DrawHelper dh) {
            dh.multiNet = currPage;
        }

        @Override
        public void allBindingsOk(boolean allOk) {
        }

        @Override
        public Color getNetBackground(String overlayMsg) {
            return editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR;
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
        propertyPanel = new javax.swing.JPanel();
        panel_operator = new javax.swing.JPanel();
        label_operator = new javax.swing.JLabel();
        panel_algebra = new javax.swing.JPanel();
        panelTags = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        scrollPane_TagsT = new javax.swing.JScrollPane();
        list_TagsT = new javax.swing.JList<>();
        scrollPane_TagsP = new javax.swing.JScrollPane();
        list_TagsP = new javax.swing.JList<>();
        panel_alignment = new javax.swing.JPanel();
        jLabel4 = new javax.swing.JLabel();
        exprField_algebraDx = new editor.domain.measures.ExprField();
        exprField_algebraDy = new editor.domain.measures.ExprField();
        jLabel5 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        panel_toggles = new javax.swing.JPanel();
        toggle_horizontal = new javax.swing.JToggleButton();
        toggle_vertical = new javax.swing.JToggleButton();
        toggle_custom = new javax.swing.JToggleButton();
        checkBox_useBrokenEdges = new javax.swing.JCheckBox();
        panel_bottom = new javax.swing.JPanel();
        panel_unfolding = new javax.swing.JPanel();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();
        exprField_unfDxMult1 = new editor.domain.measures.ExprField();
        exprField_unfDyMult1 = new editor.domain.measures.ExprField();
        jLabel9 = new javax.swing.JLabel();
        jLabel10 = new javax.swing.JLabel();
        exprField_unfDxMult2 = new editor.domain.measures.ExprField();
        exprField_unfDyMult2 = new editor.domain.measures.ExprField();
        jLabel11 = new javax.swing.JLabel();
        exprField_unfDxMult3 = new editor.domain.measures.ExprField();
        exprField_unfDyMult3 = new editor.domain.measures.ExprField();
        jLabel12 = new javax.swing.JLabel();
        exprField_unfDxMult4 = new editor.domain.measures.ExprField();
        exprField_unfDyMult4 = new editor.domain.measures.ExprField();
        button_resetOffsetMatrix = new javax.swing.JButton();
        panel_algebra2 = new javax.swing.JPanel();
        panelTags1 = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        jLabel13 = new javax.swing.JLabel();
        scrollPane_TagsT2 = new javax.swing.JScrollPane();
        list_TagsT2 = new javax.swing.JList<>();
        scrollPane_TagsP2 = new javax.swing.JScrollPane();
        list_TagsP2 = new javax.swing.JList<>();
        panel_alignment1 = new javax.swing.JPanel();
        jLabel14 = new javax.swing.JLabel();
        exprField_algebraDx2 = new editor.domain.measures.ExprField();
        exprField_algebraDy2 = new editor.domain.measures.ExprField();
        jLabel15 = new javax.swing.JLabel();
        jLabel16 = new javax.swing.JLabel();
        panel_toggles1 = new javax.swing.JPanel();
        toggle_horizontal2 = new javax.swing.JToggleButton();
        toggle_vertical2 = new javax.swing.JToggleButton();
        toggle_custom2 = new javax.swing.JToggleButton();
        checkBox_useBrokenEdges2 = new javax.swing.JCheckBox();
        checkBox_applyRestrictions = new javax.swing.JCheckBox();
        panel_semantics2 = new javax.swing.JPanel();
        label_semantics2 = new javax.swing.JLabel();
        comboBox_semantics = new javax.swing.JComboBox<>();
        actionAddSubnet = new common.Action();
        scrollPaneCentral = new javax.swing.JScrollPane();
        toolbar = new javax.swing.JToolBar();
        toolbarButton_addSubnet = new common.JToolbarButton();
        panel_netInstanceEditor = new javax.swing.JPanel();

        propertyPanel.setLayout(new java.awt.GridBagLayout());

        panel_operator.setBorder(javax.swing.BorderFactory.createTitledBorder("Composition operator:"));
        panel_operator.setLayout(new java.awt.GridBagLayout());

        label_operator.setText("OPX");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 8;
        gridBagConstraints.ipady = 8;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 1.0;
        panel_operator.add(label_operator, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        propertyPanel.add(panel_operator, gridBagConstraints);

        panel_algebra.setBorder(javax.swing.BorderFactory.createTitledBorder("Tag Options:"));
        panel_algebra.setLayout(new java.awt.GridBagLayout());

        panelTags.setLayout(new java.awt.GridBagLayout());

        jLabel2.setText("Transitions:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panelTags.add(jLabel2, gridBagConstraints);

        jLabel1.setText("Places:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        panelTags.add(jLabel1, gridBagConstraints);

        list_TagsT.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                list_TagsTMouseClicked(evt);
            }
        });
        scrollPane_TagsT.setViewportView(list_TagsT);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panelTags.add(scrollPane_TagsT, gridBagConstraints);

        list_TagsP.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                list_TagsPMouseClicked(evt);
            }
        });
        scrollPane_TagsP.setViewportView(list_TagsP);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        panelTags.add(scrollPane_TagsP, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        panel_algebra.add(panelTags, gridBagConstraints);

        panel_alignment.setLayout(new java.awt.GridBagLayout());

        jLabel4.setText("Alignment:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        panel_alignment.add(jLabel4, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 2);
        panel_alignment.add(exprField_algebraDx, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 2);
        panel_alignment.add(exprField_algebraDy, gridBagConstraints);

        jLabel5.setText("dx:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 7, 0, 3);
        panel_alignment.add(jLabel5, gridBagConstraints);

        jLabel6.setText("dy:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 7, 0, 3);
        panel_alignment.add(jLabel6, gridBagConstraints);

        panel_toggles.setLayout(new java.awt.GridBagLayout());

        toggle_horizontal.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12H.png"))); // NOI18N
        toggle_horizontal.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_horizontalActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        panel_toggles.add(toggle_horizontal, gridBagConstraints);

        toggle_vertical.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12V.png"))); // NOI18N
        toggle_vertical.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_verticalActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        panel_toggles.add(toggle_vertical, gridBagConstraints);

        toggle_custom.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12HV.png"))); // NOI18N
        toggle_custom.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_customActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        panel_toggles.add(toggle_custom, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        panel_alignment.add(panel_toggles, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_algebra.add(panel_alignment, gridBagConstraints);

        checkBox_useBrokenEdges.setText("Use broken edges.");
        checkBox_useBrokenEdges.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_useBrokenEdgesActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.FIRST_LINE_START;
        panel_algebra.add(checkBox_useBrokenEdges, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        propertyPanel.add(panel_algebra, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 8;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        propertyPanel.add(panel_bottom, gridBagConstraints);

        panel_unfolding.setBorder(javax.swing.BorderFactory.createTitledBorder("Unfolding Options:"));
        panel_unfolding.setLayout(new java.awt.GridBagLayout());

        jLabel7.setText("dx");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        panel_unfolding.add(jLabel7, gridBagConstraints);

        jLabel8.setText("dy");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        panel_unfolding.add(jLabel8, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDxMult1, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDyMult1, gridBagConstraints);

        jLabel9.setText("Class 1:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 7);
        panel_unfolding.add(jLabel9, gridBagConstraints);

        jLabel10.setText("Class 2:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 7);
        panel_unfolding.add(jLabel10, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDxMult2, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDyMult2, gridBagConstraints);

        jLabel11.setText("Class 3:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 7);
        panel_unfolding.add(jLabel11, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDxMult3, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDyMult3, gridBagConstraints);

        jLabel12.setText("Class 4:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 7);
        panel_unfolding.add(jLabel12, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDxMult4, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_unfolding.add(exprField_unfDyMult4, gridBagConstraints);

        button_resetOffsetMatrix.setText("Reset offset matrix");
        button_resetOffsetMatrix.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_resetOffsetMatrixActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(4, 0, 4, 0);
        panel_unfolding.add(button_resetOffsetMatrix, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        propertyPanel.add(panel_unfolding, gridBagConstraints);

        panel_algebra2.setBorder(javax.swing.BorderFactory.createTitledBorder("Tag Options:"));
        panel_algebra2.setLayout(new java.awt.GridBagLayout());

        panelTags1.setLayout(new java.awt.GridBagLayout());

        jLabel3.setText("Transitions:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panelTags1.add(jLabel3, gridBagConstraints);

        jLabel13.setText("Places:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        panelTags1.add(jLabel13, gridBagConstraints);

        list_TagsT2.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                list_TagsT2MouseClicked(evt);
            }
        });
        scrollPane_TagsT2.setViewportView(list_TagsT2);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        panelTags1.add(scrollPane_TagsT2, gridBagConstraints);

        list_TagsP2.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                list_TagsP2MouseClicked(evt);
            }
        });
        scrollPane_TagsP2.setViewportView(list_TagsP2);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        panelTags1.add(scrollPane_TagsP2, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        panel_algebra2.add(panelTags1, gridBagConstraints);

        panel_alignment1.setLayout(new java.awt.GridBagLayout());

        jLabel14.setText("Alignment:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        panel_alignment1.add(jLabel14, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 2);
        panel_alignment1.add(exprField_algebraDx2, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(0, 2, 0, 2);
        panel_alignment1.add(exprField_algebraDy2, gridBagConstraints);

        jLabel15.setText("dx:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 7, 0, 3);
        panel_alignment1.add(jLabel15, gridBagConstraints);

        jLabel16.setText("dy:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.insets = new java.awt.Insets(0, 7, 0, 3);
        panel_alignment1.add(jLabel16, gridBagConstraints);

        panel_toggles1.setLayout(new java.awt.GridBagLayout());

        toggle_horizontal2.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12H.png"))); // NOI18N
        toggle_horizontal2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_horizontal2ActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        panel_toggles1.add(toggle_horizontal2, gridBagConstraints);

        toggle_vertical2.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12V.png"))); // NOI18N
        toggle_vertical2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_vertical2ActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        panel_toggles1.add(toggle_vertical2, gridBagConstraints);

        toggle_custom2.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12HV.png"))); // NOI18N
        toggle_custom2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_custom2ActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        panel_toggles1.add(toggle_custom2, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridheight = 2;
        panel_alignment1.add(panel_toggles1, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        panel_algebra2.add(panel_alignment1, gridBagConstraints);

        checkBox_useBrokenEdges2.setText("Use broken edges.");
        checkBox_useBrokenEdges2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_useBrokenEdges2ActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.FIRST_LINE_START;
        panel_algebra2.add(checkBox_useBrokenEdges2, gridBagConstraints);

        checkBox_applyRestrictions.setText("Restrict selected tags.");
        checkBox_applyRestrictions.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_applyRestrictionsActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        panel_algebra2.add(checkBox_applyRestrictions, gridBagConstraints);

        panel_semantics2.setLayout(new java.awt.GridBagLayout());

        label_semantics2.setText("Semantics:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 12);
        panel_semantics2.add(label_semantics2, gridBagConstraints);

        comboBox_semantics.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_semanticsActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        panel_semantics2.add(comboBox_semantics, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        panel_algebra2.add(panel_semantics2, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        propertyPanel.add(panel_algebra2, gridBagConstraints);

        actionAddSubnet.setActionName("Add subnet");
        actionAddSubnet.setIcon(resourceFactory.getMultiNetAdd24());
        actionAddSubnet.setTooltipDesc("Add a new subnet in this multi page.");
        actionAddSubnet.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddSubnetActionPerformed(evt);
            }
        });

        setLayout(new java.awt.GridBagLayout());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        add(scrollPaneCentral, gridBagConstraints);

        toolbar.setFloatable(false);

        toolbarButton_addSubnet.setAction(actionAddSubnet);
        toolbarButton_addSubnet.setText("Add subnet.");
        toolbarButton_addSubnet.setHorizontalTextPosition(javax.swing.SwingConstants.TRAILING);
        toolbar.add(toolbarButton_addSubnet);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        add(toolbar, gridBagConstraints);

        panel_netInstanceEditor.setBorder(javax.swing.BorderFactory.createTitledBorder("Subnet definitions:"));
        panel_netInstanceEditor.setLayout(new javax.swing.BoxLayout(panel_netInstanceEditor, javax.swing.BoxLayout.LINE_AXIS));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        add(panel_netInstanceEditor, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents

    private void actionAddSubnetActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddSubnetActionPerformed
        JPopupMenu popup = new JPopupMenu();
        ProjectData data = currProject.getCurrent();
        for (int p=0; p<data.getPageCount(); p++) {
            final ProjectPage itemPage = data.getPageAt(p);
            if (itemPage instanceof ComposableNet) {
                JMenuItem item = new JMenuItem(itemPage.getPageName(), itemPage.getPageIcon());
                item.addActionListener(new ActionListener() {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        mainInterface.executeUndoableCommand("add subnet.", (ProjectData proj, ProjectPage page) -> {
                            NetInstanceDescriptor inst = new NetInstanceDescriptor();
                            inst.targetNetName = itemPage.getPageName();
                            ((MultiNetPage)page).netsDescr.add(inst);
                        });
                    }
                });
                popup.add(item);
            }
        }
        popup.show(toolbarButton_addSubnet, 0, toolbarButton_addSubnet.getHeight());
    }//GEN-LAST:event_actionAddSubnetActionPerformed

    private void list_TagsPMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_list_TagsPMouseClicked
        JList list = (JList)evt.getSource();
        if (evt.getClickCount() == 1 && evt.getButton()==MouseEvent.BUTTON1) {
            final int index = list.locationToIndex(evt.getPoint());
            if (index == -1) 
                return; // out of selection
            final JCheckBox box = (JCheckBox)list.getModel().getElementAt(index);
            final String tag = box.getText();
//            System.out.println("list_TagsPMouseClicked "+index+" "+tag);
            
            mainInterface.executeUndoableCommand("change place tag selection.", (ProjectData proj, ProjectPage elem) -> {
                TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
                if (acp.selTagsP.contains(tag))
                    acp.selTagsP.remove(tag);
                else
                    acp.selTagsP.add(tag);
            });
        }
    }//GEN-LAST:event_list_TagsPMouseClicked

    private void list_TagsTMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_list_TagsTMouseClicked
        JList list = (JList)evt.getSource();
        if (evt.getClickCount() == 1 && evt.getButton()==MouseEvent.BUTTON1) {
            final int index = list.locationToIndex(evt.getPoint());
            if (index == -1) 
                return; // out of selection
            final JCheckBox box = (JCheckBox)list.getModel().getElementAt(index);
            final String tag = box.getText();
//            System.out.println("list_TagsPMouseClicked "+index+" "+tag);
            
            mainInterface.executeUndoableCommand("change transition tag selection.", (ProjectData proj, ProjectPage elem) -> {
                TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
                if (acp.selTagsT.contains(tag))
                    acp.selTagsT.remove(tag);
                else
                    acp.selTagsT.add(tag);
            });
        }
    }//GEN-LAST:event_list_TagsTMouseClicked

    private void toggle_horizontalActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_horizontalActionPerformed
        mainInterface.executeUndoableCommand("toggle horizontal alignment.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
            acp.alignment = TagBasedCompositionPage.Alignment.HORIZONTAL;
        });
    }//GEN-LAST:event_toggle_horizontalActionPerformed

    private void toggle_verticalActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_verticalActionPerformed
        mainInterface.executeUndoableCommand("toggle vertical alignment.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
            acp.alignment = TagBasedCompositionPage.Alignment.VERTICAL;
        });
    }//GEN-LAST:event_toggle_verticalActionPerformed

    private void toggle_customActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_customActionPerformed
        mainInterface.executeUndoableCommand("toggle custom alignment.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
            acp.alignment = TagBasedCompositionPage.Alignment.CUSTOM;
        });
    }//GEN-LAST:event_toggle_customActionPerformed

    private void checkBox_useBrokenEdgesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_useBrokenEdgesActionPerformed
        mainInterface.executeUndoableCommand("change broken edge flag.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage acp = (TagBasedCompositionPage)currPage;
            acp.useBrokenEdges = checkBox_useBrokenEdges.isSelected();
        });
    }//GEN-LAST:event_checkBox_useBrokenEdgesActionPerformed

    private void button_resetOffsetMatrixActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_resetOffsetMatrixActionPerformed
        mainInterface.executeUndoableCommand("reset offset matrix.", (ProjectData proj, ProjectPage elem) -> {
            UnfoldingCompositionPage ucp = (UnfoldingCompositionPage)currPage;
            ucp.resetOffsetMatrix();
        });
    }//GEN-LAST:event_button_resetOffsetMatrixActionPerformed

    
    
    
    
    
    private void list_TagsT2MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_list_TagsT2MouseClicked
        JList list = (JList)evt.getSource();
        if (evt.getClickCount() == 1 && evt.getButton()==MouseEvent.BUTTON1) {
            final int index = list.locationToIndex(evt.getPoint());
            if (index == -1) 
                return; // out of selection
            final JCheckBox box = (JCheckBox)list.getModel().getElementAt(index);
            final String tag = box.getText();
//            System.out.println("list_TagsPMouseClicked "+index+" "+tag);
            
            mainInterface.executeUndoableCommand("change transition tag selection.", (ProjectData proj, ProjectPage elem) -> {
                TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
                if (acp.selTagsT.contains(tag))
                    acp.selTagsT.remove(tag);
                else
                    acp.selTagsT.add(tag);
            });
        }
    }//GEN-LAST:event_list_TagsT2MouseClicked

    private void list_TagsP2MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_list_TagsP2MouseClicked
        JList list = (JList)evt.getSource();
        if (evt.getClickCount() == 1 && evt.getButton()==MouseEvent.BUTTON1) {
            final int index = list.locationToIndex(evt.getPoint());
            if (index == -1) 
                return; // out of selection
            final JCheckBox box = (JCheckBox)list.getModel().getElementAt(index);
            final String tag = box.getText();
//            System.out.println("list_TagsPMouseClicked "+index+" "+tag);
            
            mainInterface.executeUndoableCommand("change place tag selection.", (ProjectData proj, ProjectPage elem) -> {
                TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
                if (acp.selTagsP.contains(tag))
                    acp.selTagsP.remove(tag);
                else
                    acp.selTagsP.add(tag);
            });
        }
    }//GEN-LAST:event_list_TagsP2MouseClicked

    private void toggle_horizontal2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_horizontal2ActionPerformed
        mainInterface.executeUndoableCommand("toggle horizontal alignment.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
            acp.alignment = TagBasedCompositionPage2.Alignment.HORIZONTAL;
        });
    }//GEN-LAST:event_toggle_horizontal2ActionPerformed

    private void toggle_vertical2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_vertical2ActionPerformed
        mainInterface.executeUndoableCommand("toggle vertical alignment.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
            acp.alignment = TagBasedCompositionPage2.Alignment.VERTICAL;
        });
    }//GEN-LAST:event_toggle_vertical2ActionPerformed

    private void toggle_custom2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_custom2ActionPerformed
        mainInterface.executeUndoableCommand("toggle custom alignment.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
            acp.alignment = TagBasedCompositionPage2.Alignment.CUSTOM;
        });
    }//GEN-LAST:event_toggle_custom2ActionPerformed

    private void checkBox_useBrokenEdges2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_useBrokenEdges2ActionPerformed
        mainInterface.executeUndoableCommand("change broken edge flag.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
            acp.useBrokenEdges = checkBox_useBrokenEdges2.isSelected();
        });
    }//GEN-LAST:event_checkBox_useBrokenEdges2ActionPerformed

    private void checkBox_applyRestrictionsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_applyRestrictionsActionPerformed
        mainInterface.executeUndoableCommand("change restriction flag.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
            acp.applyRestrictions = checkBox_applyRestrictions.isSelected();
        });
    }//GEN-LAST:event_checkBox_applyRestrictionsActionPerformed

    private void comboBox_semanticsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_semanticsActionPerformed
        if (initializing)
            return;
        mainInterface.executeUndoableCommand("change composition semantics.", (ProjectData proj, ProjectPage elem) -> {
            TagBasedCompositionPage2 acp = (TagBasedCompositionPage2)currPage;
            acp.semantics = (Algebra2.Semantics)comboBox_semantics.getSelectedItem();
        });
    }//GEN-LAST:event_comboBox_semanticsActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private common.Action actionAddSubnet;
    private javax.swing.JButton button_resetOffsetMatrix;
    private javax.swing.JCheckBox checkBox_applyRestrictions;
    private javax.swing.JCheckBox checkBox_useBrokenEdges;
    private javax.swing.JCheckBox checkBox_useBrokenEdges2;
    private javax.swing.JComboBox<Algebra2.Semantics> comboBox_semantics;
    private editor.domain.measures.ExprField exprField_algebraDx;
    private editor.domain.measures.ExprField exprField_algebraDx2;
    private editor.domain.measures.ExprField exprField_algebraDy;
    private editor.domain.measures.ExprField exprField_algebraDy2;
    private editor.domain.measures.ExprField exprField_unfDxMult1;
    private editor.domain.measures.ExprField exprField_unfDxMult2;
    private editor.domain.measures.ExprField exprField_unfDxMult3;
    private editor.domain.measures.ExprField exprField_unfDxMult4;
    private editor.domain.measures.ExprField exprField_unfDyMult1;
    private editor.domain.measures.ExprField exprField_unfDyMult2;
    private editor.domain.measures.ExprField exprField_unfDyMult3;
    private editor.domain.measures.ExprField exprField_unfDyMult4;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel14;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel16;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JLabel label_operator;
    private javax.swing.JLabel label_semantics2;
    private javax.swing.JList<JCheckBox> list_TagsP;
    private javax.swing.JList<JCheckBox> list_TagsP2;
    private javax.swing.JList<JCheckBox> list_TagsT;
    private javax.swing.JList<JCheckBox> list_TagsT2;
    private javax.swing.JPanel panelTags;
    private javax.swing.JPanel panelTags1;
    private javax.swing.JPanel panel_algebra;
    private javax.swing.JPanel panel_algebra2;
    private javax.swing.JPanel panel_alignment;
    private javax.swing.JPanel panel_alignment1;
    private javax.swing.JPanel panel_bottom;
    private javax.swing.JPanel panel_netInstanceEditor;
    private javax.swing.JPanel panel_operator;
    private javax.swing.JPanel panel_semantics2;
    private javax.swing.JPanel panel_toggles;
    private javax.swing.JPanel panel_toggles1;
    private javax.swing.JPanel panel_unfolding;
    private javax.swing.JPanel propertyPanel;
    private editor.gui.ResourceFactory resourceFactory;
    private javax.swing.JScrollPane scrollPaneCentral;
    private javax.swing.JScrollPane scrollPane_TagsP;
    private javax.swing.JScrollPane scrollPane_TagsP2;
    private javax.swing.JScrollPane scrollPane_TagsT;
    private javax.swing.JScrollPane scrollPane_TagsT2;
    private javax.swing.JToggleButton toggle_custom;
    private javax.swing.JToggleButton toggle_custom2;
    private javax.swing.JToggleButton toggle_horizontal;
    private javax.swing.JToggleButton toggle_horizontal2;
    private javax.swing.JToggleButton toggle_vertical;
    private javax.swing.JToggleButton toggle_vertical2;
    private javax.swing.JToolBar toolbar;
    private common.JToolbarButton toolbarButton_addSubnet;
    // End of variables declaration//GEN-END:variables
}
