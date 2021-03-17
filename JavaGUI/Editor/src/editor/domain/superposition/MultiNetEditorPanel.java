/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

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
import editor.gui.AbstractPageEditor;
import editor.gui.CutCopyPasteEngine;
import editor.gui.MainWindowInterface;
import editor.gui.SharedResourceProvider;
import editor.gui.net.NetViewerPanel;
import java.awt.Color;
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
import javax.swing.InputMap;
import javax.swing.JComponent;
import static javax.swing.JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
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
        
        // Add actions to the input map manually
        InputMap inMap = getInputMap(WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        ActionMap actMap = getActionMap();
        Action.registerAllActions(actMap, inMap, this, getClass(), actionCondition);
        
        initializing = false;
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
    }

    @Override
    public void updateGUI(ProjectFile pf, ProjectPage page) {
        assert pf == currProject;
        currPage = (MultiNetPage)page;
        initializing = true;
        
        toolbarButton_addSubnet.setEnabled(!currPage.operator.hasFixedNumOfOperators());
        label_operator.setIcon(currPage.operator.getOperatorIcon());
        label_operator.setText(currPage.operator.getOperatorName());
        
        if (currPage.isPageCorrect()) {
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
    public JComponent getToolbar() {
        return toolBarEmpty;
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
            return Color.WHITE;
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
        toolBarEmpty = new javax.swing.JToolBar();
        propertyPanel = new javax.swing.JPanel();
        panel_operator = new javax.swing.JPanel();
        label_operator = new javax.swing.JLabel();
        actionAddSubnet = new common.Action();
        scrollPaneCentral = new javax.swing.JScrollPane();
        toolbar = new javax.swing.JToolBar();
        toolbarButton_addSubnet = new common.JToolbarButton();
        panel_netInstanceEditor = new javax.swing.JPanel();

        toolBarEmpty.setRollover(true);

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
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        propertyPanel.add(panel_operator, gridBagConstraints);

        actionAddSubnet.setActionName("Add subnet");
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
        toolbarButton_addSubnet.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        toolbarButton_addSubnet.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
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


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private common.Action actionAddSubnet;
    private javax.swing.JLabel label_operator;
    private javax.swing.JPanel panel_netInstanceEditor;
    private javax.swing.JPanel panel_operator;
    private javax.swing.JPanel propertyPanel;
    private editor.gui.ResourceFactory resourceFactory;
    private javax.swing.JScrollPane scrollPaneCentral;
    private javax.swing.JToolBar toolBarEmpty;
    private javax.swing.JToolBar toolbar;
    private common.JToolbarButton toolbarButton_addSubnet;
    // End of variables declaration//GEN-END:variables
}
