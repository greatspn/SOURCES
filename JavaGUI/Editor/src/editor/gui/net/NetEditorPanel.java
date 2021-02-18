/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.gui.SharedResourceProvider;
import common.Action;
import common.Condition;
import common.JToggleTriState;
import common.ModalLogDialog;
import common.Util;
import editor.Main;
import editor.domain.Decor;
import editor.domain.DrawHelper;
import editor.domain.elements.DtaPage;
import editor.domain.Edge;
import editor.domain.elements.GspnPage;
import editor.domain.NetPage;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.gui.AbstractPageEditor;
import editor.gui.MainWindowInterface;
import editor.domain.NetObject;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.Selectable;
import editor.domain.ViewProfile;
import editor.domain.io.ApnnFormat;
import editor.domain.io.DtaFormat;
import editor.domain.io.GRMLFormat;
import editor.domain.io.GreatSpnFormat;
import editor.domain.io.PNMLFormat;
import editor.domain.struct.StructInfo;
import editor.gui.CutCopyPasteEngine;
import editor.gui.ResourceFactory;
import editor.gui.UndoableCommand;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.UUID;
import javax.swing.AbstractButton;
import javax.swing.ActionMap;
import javax.swing.InputMap;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.KeyStroke;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;
import javax.swing.UIManager;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

/**
 *
 * @author elvio
 */
public class NetEditorPanel extends javax.swing.JPanel implements AbstractPageEditor {

    // Currently edited data
    ProjectFile currProject = null;
    NetPage currPage = null;
    private UUID currPageUUID = null;
    MainWindowInterface mainInterface = null;
    private NetToolBase currTool = null;
    BaseCellEditor activeEditor = null;
    NetPropertyPanel propertyPanel;

    
    // Additional swing components - the editor pane and the rulers
    JNetPanel netPanel;
    private final Ruler horizRuler, vertRuler;
    
    @Override
    public int getPageZoom() {
        assert currPage != null;
        return currPage.viewProfile.zoom;
    }

    @Override
    public void zoomChanged(int newZoomLevel) {
        //System.out.println("zoomChanged "+newZoomLevel+" "+currPage.zoomLevel);
        if (newZoomLevel != currPage.viewProfile.zoom || newZoomLevel != horizRuler.getZoomLevel()) {
            currPage.viewProfile.zoom = newZoomLevel;
            netPanel.resizeToAccomodate();
            horizRuler.setZoomLevel(newZoomLevel);
            vertRuler.setZoomLevel(newZoomLevel);
            netPanel.repaint();
        }
        assert getPageZoom() == horizRuler.getZoomLevel();
        assert getPageZoom() == vertRuler.getZoomLevel();
    }
    
    // Editor tools
    public enum Tool implements Serializable {
        SELECT,
        NEW_INT_CONST, NEW_REAL_CONST,
        NEW_PLACE, NEW_CONT_PLACE, NEW_EXP_TRN, NEW_IMM_TRN, NEW_GEN_TRN, NEW_CONT_TRN,
        NEW_DTA_LOC, NEW_DTA_INIT_LOC, NEW_DTA_ACCEPT_LOC, NEW_DTA_REJECT_LOC, 
        NEW_GSPN_ARC, NEW_GSPN_INHIB_ARC,
        NEW_DTA_INNER_EDGE, NEW_DTA_BOUNDARY_EDGE,
        NEW_COLOR_CLASS, NEW_COLOR_VAR,
        NEW_INT_TEMPLATE, NEW_REAL_TEMPLATE,
        NEW_STATEPROP, NEW_ACTION, NEW_COLOR_CLASS_TEMPLATE, 
        NEW_CLOCKVAR, NEW_TEXT_BOX_NODE
    }
    private AbstractButton getToolButton(Tool tool) {
        switch (tool) {
            case SELECT:                return jToggleButtonSelection;
            case NEW_INT_CONST:         return jToggleTriStateAddIntConst;
            case NEW_REAL_CONST:        return jToggleTriStateAddRealConst;
            case NEW_PLACE:             return jToggleTriStateAddPlace;
            case NEW_CONT_PLACE:        return jToggleTriStateAddContPlace;
            case NEW_EXP_TRN:           return jToggleTriStateAddExpTransition;
            case NEW_IMM_TRN:           return jToggleTriStateAddImmTransition;
            case NEW_GEN_TRN:           return jToggleTriStateAddGenTransition;
            case NEW_CONT_TRN:          return jToggleTriStateAddContTransition;
            case NEW_DTA_LOC:           return jToggleTriStateAddDtaLoc;
            case NEW_DTA_INIT_LOC:      return jToggleTriStateAddDtaInitLoc;
            case NEW_DTA_ACCEPT_LOC:    return jToggleTriStateAddDtaAcceptLoc;
            case NEW_DTA_REJECT_LOC:    return jToggleTriStateAddDtaRejectLoc;
            case NEW_GSPN_ARC:          return jToggleTriStateAddGspnArc;
            case NEW_GSPN_INHIB_ARC:    return jToggleTriStateAddGspnInhibArc;
            case NEW_DTA_INNER_EDGE:    return jToggleTriStateAddDtaInnerEdge;
            case NEW_DTA_BOUNDARY_EDGE: return jToggleTriStateAddDtaBoundaryEdge;
            case NEW_COLOR_CLASS:       return jToggleTriStateAddColorClass;
            case NEW_COLOR_VAR:         return jToggleTriStateAddColorVar;
            case NEW_COLOR_CLASS_TEMPLATE:  return jToggleTriStateAddColorClassTemplate;
            case NEW_INT_TEMPLATE:      return jToggleTriStateAddIntTemplate;
            case NEW_REAL_TEMPLATE:     return jToggleTriStateAddRealTemplate;
            case NEW_STATEPROP:         return jToggleTriStateAddStatePropTemplate;
            case NEW_ACTION:            return jToggleTriStateAddActionTemplate;
            case NEW_CLOCKVAR:          return jToggleTriStateAddClockVar;
            case NEW_TEXT_BOX_NODE:     return jToggleTriStateAddTextBox;
            default:
                throw new AssertionError(tool.name());            
        }
    }
    private NetToolBase newToolInstance(Tool tool) {
        switch (tool) {
            case NEW_INT_CONST:
            case NEW_REAL_CONST:
            case NEW_PLACE:
            case NEW_CONT_PLACE:
            case NEW_EXP_TRN:
            case NEW_IMM_TRN:
            case NEW_GEN_TRN:
            case NEW_CONT_TRN:
            case NEW_DTA_LOC:
            case NEW_DTA_INIT_LOC:
            case NEW_DTA_ACCEPT_LOC:
            case NEW_DTA_REJECT_LOC:
            case NEW_COLOR_CLASS:
            case NEW_COLOR_VAR:
            case NEW_COLOR_CLASS_TEMPLATE:
            case NEW_INT_TEMPLATE:
            case NEW_REAL_TEMPLATE:
            case NEW_STATEPROP:
            case NEW_ACTION:
            case NEW_CLOCKVAR:
            case NEW_TEXT_BOX_NODE:
                return new AddNodeTool(this, tool);
            case NEW_GSPN_ARC:
            case NEW_GSPN_INHIB_ARC:
            case NEW_DTA_INNER_EDGE:
            case NEW_DTA_BOUNDARY_EDGE:
                return new AddNewEdgeTool(this, tool);
            default:
                return new SelectMoveTool(this, tool);
        }
    }
    
    private int getToolRepetition(Tool tool) {
        AbstractButton ab = getToolButton(tool);
        assert ab.isSelected();
        if (!(ab instanceof JToggleTriState))
            return 1;
        JToggleTriState triState = (JToggleTriState)ab;
        return triState.isFirstToggleState() ? 1 : 2;
    }
    private void setToolRepetition(Tool tool, int rep) {
        AbstractButton ab = getToolButton(tool);
        if (!ab.isSelected())
            ab.setSelected(true);
        assert ab.isSelected() && (rep == 1 || rep == 2);
        if (!(ab instanceof JToggleTriState))
            return; // Affects only tri-state buttons
        JToggleTriState triState = (JToggleTriState)ab;
        triState.setSelected(true, rep==1);
    }
    
    void toolEnds(boolean moveToSelect) {
        if (currTool.activeTool != Tool.SELECT &&
            (moveToSelect || getToolRepetition(currTool.activeTool)==1))
        {
            // The current non-selection tool has finished. change back to selection.
            changeTool(Tool.SELECT);
        }
        else {
            // Reload a new instance of the active tool
            mainInterface.invalidateGUI();
//            currTool.toolDisabled();
//            NetToolBase t = newToolInstance(currTool.activeTool);
//            currTool = t;
        }
    }
    
    private void changeTool(Tool selTool) {
        boolean isNewTool = (currPage.editorTool != selTool);        
        if (isNewTool) {
            currPage.editorTool = selTool;
            currPage.editorToolRepetition = 1;
            //mainInterface.invalidateGUI();
            //updateGUI(currProject, currPage);
        }
        else {
            currPage.editorToolRepetition = (currPage.editorToolRepetition==1 ? 2 : 1);
            if (selTool == Tool.SELECT)
                currPage.editorToolRepetition = 1;
            setToolRepetition(selTool, currPage.editorToolRepetition);
        }
        currPage.selectionEnds();
        mainInterface.invalidateGUI();
    }
    
    boolean isGridSnappingActive() {
        return jToggleButtonSnapToGrid.isSelected();
    }
    
    
    /**
     * Creates new form NetEditorPanel
     * @param shActProv
     * @param actionCondition
     */
    public NetEditorPanel(SharedResourceProvider shActProv, Condition actionCondition) {
        initComponents();
        Util.reformatPanelsForPlatformLookAndFeel(this);
        netPanel = new JNetPanel();
        jScrollPane.setViewportView(netPanel);
        jScrollPane.getViewport().addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                if (currPage != null)
                    currPage.viewportPos = jScrollPane.getViewport().getViewPosition();
            }
        });
        
        // Add the rulers
        horizRuler = new Ruler(Ruler.HORIZONTAL, netPanel);
        vertRuler = new Ruler(Ruler.VERTICAL, netPanel);
        jScrollPane.setColumnHeaderView(horizRuler);
        jScrollPane.setRowHeaderView(vertRuler);
        JPanel corner = new JPanel();
        corner.setBackground(Color.WHITE);
        jScrollPane.setCorner(JScrollPane.UPPER_LEFT_CORNER, corner);
        
        // Add actions to the input map manually
        InputMap inMap = getInputMap(WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        ActionMap actMap = getActionMap();
        Action.registerAllActions(actMap, inMap, this, getClass(), actionCondition);
        //actionAddExpTransition.mapInto(actMap, inMap, KeyStroke.getKeyStroke(KeyEvent.VK_T, 0));
        actionAddGenTransition.mapInto(actMap, inMap, KeyStroke.getKeyStroke(KeyEvent.VK_D, 0));
        
        // Prepare the property panel components
        propertyPanel = new NetPropertyPanel(this);
    }
    
    private final WindowListener windowListener = new WindowAdapter() {

        @Override
        public void windowActivated(WindowEvent e) {
            if (currTool != null)
                currTool.topWindowActivated(true);
        }

        @Override
        public void windowDeactivated(WindowEvent e) {
            if (currTool != null)
                currTool.topWindowActivated(false);
        }
    };
    
    protected boolean isTopLevelActive() {
        return mainInterface.getWindowFrame().isActive();
    }
    
    @Override
    public void setEditorEnabledFor(ProjectFile pf, ProjectPage page, 
                                    MainWindowInterface newMainInterface)
    {
        assert (pf != null && page != null);
        currProject = pf;
        currPage = (NetPage)page;
        currPageUUID = page.getObjectUUID();
        mainInterface = newMainInterface;
        
        mainInterface.getWindowFrame().addWindowListener(windowListener);
        propertyPanel.setPanelEnabledFor(pf, page, newMainInterface);

        updateGUI(currProject, currPage);
    }

    @Override
    public void setEditorDisabled() {
        if (activeEditor != null)
            activeEditor.cancelEditing();
        activeEditor = null;
        mainInterface.getWindowFrame().removeWindowListener(windowListener);
        currProject = null;
        currPage = null;
        currPageUUID = null;
        mainInterface = null;
        horizRuler.setZoomLevel(-1);
        vertRuler.setZoomLevel(-1);
        currTool.toolDisabled();
        currTool = null;
        propertyPanel.setPanelDisabled();
    }

    @Override
    public boolean isModalEditor() {
        return false;
    }

    @Override
    public JComponent getEditorComponent() {
        return this;
    }

    @Override
    public JComponent getFocusTarget() {
        return netPanel;
    }

    @Override
    public JComponent getPropertyPanel() {
        return propertyPanel;
    }

    @Override
    public JComponent getToolbar() {
        return jToolBarEmpty;
    }

    @Override
    public PageErrorWarning[] getAllErrorsAndWarnings() {
        return currPage.getAllErrorsAndWarnings();
    }
    
    @Override
    public void updateGUI(ProjectFile pf, ProjectPage page) {
//        System.out.print("updateGUI("+pf.getCurrent().projName+", "+page.getPageName()+")");
//        System.out.println("   updateGUI "+page.getObjectUUID()+"  "+currPageUUID);
        assert pf == currProject;
//        assert page.getObjectUUID().equals(currPageUUID);
        currPage = (NetPage)page;
        
        // Setup zoom and position in the editor panel
        zoomChanged(getPageZoom());
        jScrollPane.getViewport().setViewPosition(currPage.viewportPos);
                
        // Get selection information
        int selNodesCount = currPage.countSelectedNodes();
        int selEdgesCount = currPage.countSelectedEdges();
        int totalSelCount = selNodesCount + selEdgesCount;
//        boolean inSelectMode = (currTool.activeTool == Tool.SELECT);
        boolean isGspn = (page instanceof GspnPage);
        boolean isDta = (page instanceof DtaPage);
        ViewProfile vp = currPage.viewProfile;
        vp.singleSelObject = currPage.getSingleSelectedObject();
        
        // Select and activate the current tool
        updateToolbarToggles(getToolButton(currPage.editorTool));
        setToolRepetition(currPage.editorTool, currPage.editorToolRepetition);
        if (currTool != null)
            currTool.toolDisabled();
        currTool = newToolInstance(currPage.editorTool);

        //------------------------------
        // Update actions and controls
        jToggleButtonSnapToGrid.setSelected(currPage.gridSnappingActive);
        jToolbarButtonViewProfiles.setVisible(isGspn);
        if (selEdgesCount > 0) {
            jToolBarEdgeCmd.setVisible(true);
            if (totalSelCount == 1 && selEdgesCount == 1) {
                // A single edge is selected
                Edge selEdge = (Edge)currPage.getSingleSelectedObject();
                boolean hasSelectedPoints = false, hasSuccessiveSelectedPoints = false;
                if (selEdge != null) {
                    for (int i=0; i<selEdge.numPoints(); i++) {
                        if (selEdge.isSubObjectSelected(i))
                            hasSelectedPoints = true;
                        if (i>=1 && selEdge.isSubObjectSelected(i) && selEdge.isSubObjectSelected(i-1))
                            hasSuccessiveSelectedPoints = true;
                    }
                }
                actionNewEdgePoint.setEnabled(hasSuccessiveSelectedPoints);
                actionDeleteEdgePoint.setEnabled(hasSelectedPoints);
                actionClearAllEdgePoints.setEnabled(selEdge!=null && selEdge.mayClearEdgePoints());
                jToolbarButtonNewPoint.setVisible(true);
                jToolbarButtonDeletePoint.setVisible(true);
            }
            else {
                boolean mayClearEdgePoints = false;
                for (Edge selEdge : currPage.edges) {
                    if (selEdge.isSelected()) {
                        if (selEdge.mayClearEdgePoints()) {
                            mayClearEdgePoints = true;
                            break;
                        }
                    }
                }
                actionNewEdgePoint.setEnabled(false);
                actionDeleteEdgePoint.setEnabled(false);
                actionClearAllEdgePoints.setEnabled(mayClearEdgePoints);
                jToolbarButtonNewPoint.setVisible(false);
                jToolbarButtonDeletePoint.setVisible(false);
                jToolbarButtonClearPoints.setVisible(true);
            }
            int numBrokenEdges = 0;
            boolean allBreakable = true;
            for (Edge e : currPage.edges)
                if (e.isSelected()) { 
                    if (!e.canBeBroken()) {
                        allBreakable = false;
                        break;
                    }
                    if (e.isBroken)
                        numBrokenEdges++;
                }
            actionSetBrokenEdge.setEnabled(allBreakable);
            jToggleButtonBrokenEdge.setVisible(allBreakable);
            jToggleButtonBrokenEdge.setSelected(numBrokenEdges == selEdgesCount);
        }
        else {
            jToolBarEdgeCmd.setVisible(false);
            actionNewEdgePoint.setEnabled(false);
            actionDeleteEdgePoint.setEnabled(false);
            actionClearAllEdgePoints.setEnabled(false);
            actionSetBrokenEdge.setEnabled(false);
        }
        // Actions that are always active - reenable because they could be made
        // inactive by an interface suspension (by editing nodes with a double click)
        boolean canBeUsed = (!mainInterface.isSuspended());

        // Common commands
        jToggleTriStateAddRealConst.setVisible(vp.showTimedCommands || vp.showFluidCommands);
        jToggleTriStateAddRealTemplate.setVisible(vp.showTimedCommands || vp.showFluidCommands);
        
        // GSPN EDITOR COMMANDS
        actionAddPlace.setEnabled(isGspn && canBeUsed);
        actionAddContPlace.setEnabled(isGspn && canBeUsed);
        actionAddExpTransition.setEnabled(isGspn && canBeUsed);
        actionAddGenTransition.setEnabled(isGspn && canBeUsed);
        actionAddImmTransition.setEnabled(isGspn && canBeUsed);
        actionAddContTransition.setEnabled(isGspn && canBeUsed);
        actionAddGspnArc.setEnabled(isGspn && canBeUsed);
        actionAddGspnInhibArc.setEnabled(isGspn && canBeUsed);
        actionAddColorClass.setEnabled(isGspn && canBeUsed);
        actionAddColorVar.setEnabled(isGspn && canBeUsed);
        //actionAddColorClassTemplate.setEnabled(isGspn && canBeUsed);
        actionAddColorClassTemplate.setEnabled(false);

        jToggleTriStateAddPlace.setVisible(isGspn);
        jToggleTriStateAddContPlace.setVisible(isGspn && vp.showFluidCommands);
        jToggleTriStateAddExpTransition.setVisible(isGspn);
        jToggleTriStateAddGenTransition.setVisible(isGspn && vp.showTimedCommands);
        jToggleTriStateAddImmTransition.setVisible(isGspn && vp.showTimedCommands);
        jToggleTriStateAddContTransition.setVisible(isGspn && vp.showFluidCommands);
        jToggleTriStateAddGspnArc.setVisible(isGspn);
        jToggleTriStateAddGspnInhibArc.setVisible(isGspn);
        jToggleTriStateAddColorClass.setVisible(isGspn && vp.showColorCommands);
        jToggleTriStateAddColorVar.setVisible(isGspn && vp.showColorCommands);
        //jToggleTriStateAddColorClassTemplate.setVisible(isGspn && vp.showColorCommands);
        jToggleTriStateAddColorClassTemplate.setVisible(false);
        
        // DTA EDITOR COMMANDS
        actionAddDtaLoc.setEnabled(isDta && canBeUsed);
        actionAddDtaInitLoc.setEnabled(isDta && canBeUsed);
        actionAddDtaAcceptLoc.setEnabled(isDta && canBeUsed);
        actionAddDtaRejectLoc.setEnabled(isDta && canBeUsed);
        actionAddDtaBoundaryEdge.setEnabled(isDta && canBeUsed);
        actionAddDtaInnerEdge.setEnabled(isDta && canBeUsed);
        actionAddStatePropTemplate.setEnabled(isDta && canBeUsed);
        actionAddActionTemplate.setEnabled(isDta && canBeUsed);
        actionAddClockVar.setEnabled(isDta && canBeUsed);
        jToggleTriStateAddDtaInitLoc.setVisible(isDta);
        jToggleTriStateAddDtaLoc.setVisible(isDta);
        jToggleTriStateAddDtaAcceptLoc.setVisible(isDta);
        jToggleTriStateAddDtaRejectLoc.setVisible(isDta);
        jToggleTriStateAddDtaBoundaryEdge.setVisible(isDta);
        jToggleTriStateAddDtaInnerEdge.setVisible(isDta);
        jToggleTriStateAddStatePropTemplate.setVisible(isDta);
        jToggleTriStateAddActionTemplate.setVisible(isDta);
        jToggleTriStateAddClockVar.setVisible(isDta);
        
        // COMMON EDITOR COMMANDS
        actionSelection.setEnabled(canBeUsed);
        actionAddIntConst.setEnabled(canBeUsed);         
        actionAddRealConst.setEnabled(canBeUsed);
        actionAddIntTemplate.setEnabled(canBeUsed);
        actionAddRealTemplate.setEnabled(canBeUsed);
        actionToggleRuler.setEnabled(canBeUsed);         
        //currPage.printNet();
        
                
        propertyPanel.updatePropertyPanel(pf.getCurrent(), currPage, canBeUsed);
        
        //------------------------------
        // Update the graph window
        netPanel.resizeToAccomodate();
        netPanel.repaint();
    }
    
    @Override
    public void updateEnablingOfSharedActions(SharedResourceProvider shResProv) {
        int selNodesCount = currPage.countSelectedNodes();
        int selEdgesCount = currPage.countSelectedEdges();
        int totalSelCount = selNodesCount + selEdgesCount;
        boolean isGspn = (currPage instanceof GspnPage);
        boolean isDta = (currPage instanceof DtaPage);
        boolean inSelectMode = (currTool.activeTool == Tool.SELECT);
        
        for (SharedResourceProvider.ActionName actName : SharedResourceProvider.ActionName.values()) {
            common.Action act = shResProv.getSharedAction(actName);
            switch (actName) {
                case EDIT_SELECT_ALL:
                    act.setEnabled(inSelectMode && totalSelCount != currPage.nodes.size() + currPage.edges.size());
                    break;

                case EDIT_DESELECT_ALL:
                    act.setEnabled(inSelectMode && totalSelCount > 0);
                    break;

                case EDIT_INVERT_SELECTION:
                    act.setEnabled(inSelectMode && totalSelCount > 0);
                    break;

                case EDIT_DELETE_SELECTED:
                    act.setEnabled(inSelectMode && totalSelCount > 0);
                    break;
                    
                case EXPORT_GREATSPN_FORMAT:
                case EXPORT_GRML_FORMAT:
                case EXPORT_APNN_FORMAT:
                case EXPORT_PNML_FORMAT:
                    act.setEnabled(isGspn && currPage.isPageCorrect());
                    break;

                case EXPORT_DTA_FORMAT:
                    act.setEnabled(isDta && currPage.isPageCorrect());
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
                return;
                
            case EDIT_DESELECT_ALL:
                deselectAll();
                return;
                
            case EDIT_INVERT_SELECTION:
                invertSelection();
                return;
                
            case EDIT_DELETE_SELECTED:
                deleteSelected();
                return;
                
            case EXPORT_GREATSPN_FORMAT:
                exportGspnInGreatSPNFormat();
                return;
                
            case EXPORT_PNML_FORMAT:
                exportGspnInPNMLFormat();
                return;
                
            case EXPORT_GRML_FORMAT:
                exportGspnInGRMLFormat();
                return;
                
            case EXPORT_APNN_FORMAT:
                exportGspnInAPNNFormat();
                return;
                        
            case EXPORT_DTA_FORMAT:
                exportInDtaFormat();
                return;
                        
            default:
                throw new IllegalStateException();
        }
    }

    
    private Tool updateToolbarToggles(AbstractButton toolButton) {
        Tool theTool = null;
        for (Tool tool : Tool.values()) {
            boolean isToolButton = (getToolButton(tool) == toolButton);
            getToolButton(tool).setSelected(isToolButton);
            if (isToolButton)
                theTool = tool;
        }
        assert theTool != null;
        return theTool;
    }

    @Override
    public boolean isZoomPanelUsed() { return true; }
    
    public static final Color OUTSIDE_PAGE_COLOR = Util.mix(UIManager.getColor("Panel.background"), Color.gray, 0.90f);
    public static final Color PAGE_SHADOW_COLOR = new Color(140, 140, 140);
    public static final Color[] PAGE_BORDER = new Color[] {
        PAGE_SHADOW_COLOR,
        // fade progressively to OUTSIDE_PAGE_COLOR
//        new Color(190, 190, 190),
//        new Color(200, 200, 200),
//        new Color(210, 210, 210),
//        new Color(220, 220, 220),
        Util.mix(PAGE_SHADOW_COLOR, OUTSIDE_PAGE_COLOR, 0.80f),
        Util.mix(PAGE_SHADOW_COLOR, OUTSIDE_PAGE_COLOR, 0.60f),
        Util.mix(PAGE_SHADOW_COLOR, OUTSIDE_PAGE_COLOR, 0.40f),
        Util.mix(PAGE_SHADOW_COLOR, OUTSIDE_PAGE_COLOR, 0.20f),
    };
    
    public static final Color PAGE_BACKGROUND_COLOR = Util.mix(UIManager.getColor("TextField.background"), Color.WHITE, 0.20f);
    

    class JNetPanel extends JPanel implements Scrollable, 
            CutCopyPasteEngine.CutCopyPasteActivation,
            CutCopyPasteEngine.CutCopyPasteActuator
    {
        public JNetPanel() {
            setBackground(PAGE_BACKGROUND_COLOR);
            setFocusable(true);
            setAutoscrolls(true); // Enable syntetic scroll events
            setOpaque(false);
            setLayout(null);
            addMouseMotionListener(new MouseMotionListener() {
                @Override public void mouseDragged(MouseEvent e) { if (canForwardEvents()) currTool.myMouseDragged(e); }
                @Override public void mouseMoved(MouseEvent e) { if (canForwardEvents()) currTool.myMouseMoved(e); }
            });
            addMouseListener(new MouseListener() {
                @Override public void mouseClicked(MouseEvent e) { if (canForwardEvents()) currTool.myMouseClicked(e); }
                @Override public void mousePressed(MouseEvent e) { 
                    JComponent source = (JComponent)e.getSource();
                    if (!source.isFocusOwner())
                        source.requestFocus();
                    if (currTool!=null) currTool.myMousePressed(e); 
                }
                @Override public void mouseReleased(MouseEvent e) { if (canForwardEvents()) currTool.myMouseReleased(e); }
                @Override public void mouseEntered(MouseEvent e) { if (canForwardEvents()) currTool.myMouseEntered(e); }
                @Override public void mouseExited(MouseEvent e) { if (canForwardEvents()) currTool.myMouseExited(e); }
            });
            addKeyListener(new KeyListener() {
                @Override public void keyTyped(KeyEvent e) { if (canForwardEvents()) currTool.keyTyped(e); }
                @Override public void keyPressed(KeyEvent e) { if (canForwardEvents()) currTool.keyPressed(e); }
                @Override public void keyReleased(KeyEvent e) { if (canForwardEvents()) currTool.keyReleased(e); }
            });
            jScrollPane.getViewport().addChangeListener(new ChangeListener() {
                @Override
                public void stateChanged(ChangeEvent ce) {
                    Dimension d1 = jScrollPane.getViewport().getExtentSize();
                    Dimension d2 = getPreferredSize();
                    if (d1.width > d2.width || d1.height > d2.height)
                        resizeToAccomodate();
                }
            });
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
            catch (Exception e) { Main.logException(e, false); }
            return false;
        }
        
        
        
        // When should tools be active and usable by the user?
        private boolean canForwardEvents() {
            return currTool!=null && mainInterface.isGUIvalid();
        }

        // Resize to accomodate the document
//        private final Dimension paPageDim = new Dimension();
        public void resizeToAccomodate() {
            assert currPage != null;
            Dimension viewportDim = jScrollPane.getViewport().getExtentSize();
//            Dimension pageDim = NetObject.logicToScreen(currPage.getPageDimension(paPageDim), currPage.zoomLevel);
            int pageWidth = NetObject.logicToScreen(currPage.getPageWidthForEditor(), currPage.viewProfile.zoom);
            int pageHeight = NetObject.logicToScreen(currPage.getPageHeightForEditor(), currPage.viewProfile.zoom);
            setPreferredSize(new Dimension(Math.max(viewportDim.width, pageWidth), 
                                           Math.max(viewportDim.height, pageHeight)));
            revalidate();
        }

        @Override
        public Dimension getPreferredScrollableViewportSize() {
            return getPreferredSize();
        }

        @Override
        public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) {
            return (int)(12 * Math.max(1.0, currPage.viewProfile.zoom / 100));
        }

        @Override
        public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction) {
            if (orientation == SwingConstants.HORIZONTAL)
                return (int)visibleRect.getWidth();
            return (int)visibleRect.getHeight();
        }

        @Override
        public boolean getScrollableTracksViewportWidth() {
            return false;
        }

        @Override
        public boolean getScrollableTracksViewportHeight() {
            return false;
        }

//        private int paintCount = 0;
        @Override
        protected void paintComponent(Graphics g) {
//            System.out.println("NetEditor.paintComponent "+paintCount++);
            assert currPage != null;
            super.paintComponent(g); 
            
            // Render the page content
            Graphics2D g2 = (Graphics2D)g;
            g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                                RenderingHints.VALUE_ANTIALIAS_ON);
            g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
                                RenderingHints.VALUE_INTERPOLATION_BICUBIC);
            g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
                                RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
            g2.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
                                RenderingHints.VALUE_FRACTIONALMETRICS_ON);
//            g2.setRenderingHint(RenderingHints.KEY_RENDERING,
//                                RenderingHints.VALUE_RENDER_QUALITY);
            DrawHelper dh = new DrawHelper(g2, currPage.viewProfile,
                                           currPage.viewProfile.zoom, 0, 0);
            

            // Draw the background
            int pX = dh.logicToScreen(currPage.getPageWidthForEditor());
            int pY = dh.logicToScreen(currPage.getPageHeightForEditor());
            int rX = (getWidth() - pX), rY = (getHeight() - pY);
            g.setColor(PAGE_BACKGROUND_COLOR);
            g.fillRect(0, 0, pX, pY);
            g.setColor(OUTSIDE_PAGE_COLOR);
            g.fillRect(pX, 0, rX, pY);
            g.fillRect(0, pY, rX+pX, rY);
            for (int i=0; i<PAGE_BORDER.length; i++) {
                g.setColor(PAGE_BORDER[i]);
                g.drawLine(pX+i, i, pX+i, pY);
                g.drawLine(pX+i, pY, pX, pY+i);
                g.drawLine(i, pY+i, pX, pY+i);        
            }
            g.setColor(Color.BLACK);

            // Setup the logic->screen coordinate system
            AffineTransform oldAT = g2.getTransform();
            g2.setTransform(dh.logic2screen);
            
            // Paint the net and the tool overlays
            currTool.paintBeforeNet(g2, dh);
            currPage.paintNet(g2, dh);
            currTool.paintAfterNet(g2, dh);
            g2.setTransform(oldAT);
        }
    }
    

    
    private void exportGspnInGreatSPNFormat() {
        assert currPage instanceof GspnPage && currPage.isPageCorrect();
        
        File netFile, defFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+currPage.getPageName()+"\" in GreatSPN format...");
            String curDir = Util.getPreferences().get("greatspn-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+currPage.getPageName()+".net") : null);
            fileChooser.addChoosableFileFilter(GreatSpnFormat.fileFilter);
            fileChooser.setFileFilter(GreatSpnFormat.fileFilter);
            if (fileChooser.showSaveDialog(this) != JFileChooser.APPROVE_OPTION)
                return;
            netFile = fileChooser.getSelectedFile();
            
            // Generate the .net and .def filenames
            String path = netFile.getPath();
            int lastDot = path.lastIndexOf(".");
            //System.out.println("path.substring(lastDot) = "+path.substring(lastDot));
            if (lastDot != -1 && path.substring(lastDot).equalsIgnoreCase(".net"))
                path = path.substring(0, lastDot);
            netFile = new File(path + ".net");
            defFile = new File(path + ".def");
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("greatspn-export-dir", curDir);
            if (netFile.exists()) {
                int r = JOptionPane.showConfirmDialog(this, 
                         "The file \""+netFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        System.out.println("netFile = "+netFile);
        System.out.println("defFile = "+defFile);
        
        try {
            String log = GreatSpnFormat.exportGspn((GspnPage)currPage, netFile, defFile, 
                                                    Main.isGreatSPNExtAllowed());
            if (log != null)
                new ModalLogDialog(this, log).setVisible(true);
            mainInterface.setStatus("GSPN exported.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(this, 
                                          "An error happened while exporting the page in GreatSPN format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+currPage.getPageName()+"\" in GreatSPN format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN.", true);
        }
    }
    
    private void exportGspnInGRMLFormat() {
        assert currPage instanceof GspnPage && currPage.isPageCorrect();
        
        File grmlFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+currPage.getPageName()+"\" in GrML format...");
            String curDir = Util.getPreferences().get("grml-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+currPage.getPageName()+".grml") : null);
            fileChooser.addChoosableFileFilter(GRMLFormat.fileFilter);
            fileChooser.setFileFilter(GRMLFormat.fileFilter);
            if (fileChooser.showSaveDialog(this) != JFileChooser.APPROVE_OPTION)
                return;
            grmlFile = fileChooser.getSelectedFile();
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("grml-export-dir", curDir);
            if (grmlFile.exists()) {
                int r = JOptionPane.showConfirmDialog(this, 
                         "The file \""+grmlFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            //StructInfo struct = StructInfo.computeStructInfo(mainInterface.getWindowFrame(), 
            //                                                 (GspnPage)currPage, null, null);
            String log = GRMLFormat.exportGspn((GspnPage)currPage, grmlFile);
            if (log != null)
                new ModalLogDialog(this, log).setVisible(true);
            mainInterface.setStatus("GSPN exported in GrML format.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(this, 
                                          "An error happened while exporting the page in GrML format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+currPage.getPageName()+"\" in GrML format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN in GrML format.", true);
        }        
    }
    
    private void exportGspnInPNMLFormat() {
        assert currPage instanceof GspnPage && currPage.isPageCorrect();
        
        File pnmlFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+currPage.getPageName()+"\" in PNML format...");
            String curDir = Util.getPreferences().get("pnml-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+currPage.getPageName()+".pnml") : null);
            fileChooser.addChoosableFileFilter(PNMLFormat.fileFilter);
            fileChooser.setFileFilter(PNMLFormat.fileFilter);
            if (fileChooser.showSaveDialog(this) != JFileChooser.APPROVE_OPTION)
                return;
            pnmlFile = fileChooser.getSelectedFile();
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("pnml-export-dir", curDir);
            if (pnmlFile.exists()) {
                int r = JOptionPane.showConfirmDialog(this, 
                         "The file \""+pnmlFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            //StructInfo struct = StructInfo.computeStructInfo(mainInterface.getWindowFrame(), 
            //                                                 (GspnPage)currPage, null, null);
            String log = PNMLFormat.exportGspn((GspnPage)currPage, pnmlFile, true);
            if (log != null)
                new ModalLogDialog(this, log).setVisible(true);
            mainInterface.setStatus("GSPN exported in PNML format.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(this, 
                                          "An error happened while exporting the page in PNML format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+currPage.getPageName()+"\" in PNML format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN in PNML format.", true);
        }       
    }
    
    private void exportGspnInAPNNFormat() {
        assert currPage instanceof GspnPage && currPage.isPageCorrect();
        
        File apnnFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+currPage.getPageName()+"\" in APNN format...");
            String curDir = Util.getPreferences().get("apnn-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+currPage.getPageName()+".apnn") : null);
            fileChooser.addChoosableFileFilter(ApnnFormat.fileFilter);
            fileChooser.setFileFilter(ApnnFormat.fileFilter);
            if (fileChooser.showSaveDialog(this) != JFileChooser.APPROVE_OPTION)
                return;
            apnnFile = fileChooser.getSelectedFile();
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("apnn-export-dir", curDir);
            if (apnnFile.exists()) {
                int r = JOptionPane.showConfirmDialog(this, 
                         "The file \""+apnnFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            StructInfo struct = StructInfo.computeStructInfo(mainInterface.getWindowFrame(), 
                                                             (GspnPage)currPage, null, null);
            String log = ApnnFormat.exportGspn((GspnPage)currPage, apnnFile, struct, null);
            if (log != null)
                new ModalLogDialog(this, log).setVisible(true);
            mainInterface.setStatus("GSPN exported in APNN format.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(this, 
                                          "An error happened while exporting the page in APNN format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+currPage.getPageName()+"\" in APNN format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export GSPN in APNN format.", true);
        }
    }
    
    
    private void exportInDtaFormat() {
        assert currPage instanceof DtaPage && currPage.isPageCorrect();
        
        File dtaFile;
        boolean repeatChooser;
        do {
            repeatChooser = false;
            final JFileChooser fileChooser = new JFileChooser();
            fileChooser.setDialogTitle("Export \""+currPage.getPageName()+"\" in MC4CSLTA format...");
            String curDir = Util.getPreferences().get("dta-export-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setSelectedFile(curDir!=null ? new File(curDir+File.separator+currPage.getPageName()+".dta") : null);
            fileChooser.addChoosableFileFilter(DtaFormat.fileFilter);
            fileChooser.setFileFilter(DtaFormat.fileFilter);
            if (fileChooser.showSaveDialog(this) != JFileChooser.APPROVE_OPTION)
                return;
            dtaFile = fileChooser.getSelectedFile();
            
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("dta-export-dir", curDir);
            if (dtaFile.exists()) {
                int r = JOptionPane.showConfirmDialog(this, 
                         "The file \""+dtaFile+"\" already exists! Overwrite it?", 
                                                       "Overwrite file", 
                                                       JOptionPane.YES_NO_CANCEL_OPTION, 
                                                       JOptionPane.WARNING_MESSAGE);
                if (r == JOptionPane.NO_OPTION)
                    repeatChooser = true;
                else if (r == JOptionPane.CANCEL_OPTION)
                    return;
            }
        } while (repeatChooser);
        
        try {
            String log = DtaFormat.export((DtaPage)currPage, dtaFile);
            if (log != null)
                new ModalLogDialog(this, log).setVisible(true);
            mainInterface.setStatus("DTA exported.", true);
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(this, 
                                          "An error happened while exporting the DTA in MC4CSLTA format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Export \""+currPage.getPageName()+"\" in MC4CSLTA format...", 
                                          JOptionPane.ERROR_MESSAGE);            
            mainInterface.setStatus("could not export DTA.", true);
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

        actionSelection = new common.Action();
        actionToggleRuler = new common.Action();
        actionAddPlace = new common.Action();
        actionAddExpTransition = new common.Action();
        actionAddImmTransition = new common.Action();
        actionAddGenTransition = new common.Action();
        actionAddGspnArc = new common.Action();
        actionAddGspnInhibArc = new common.Action();
        actionNewEdgePoint = new common.Action();
        actionDeleteEdgePoint = new common.Action();
        actionClearAllEdgePoints = new common.Action();
        actionSetBrokenEdge = new common.Action();
        actionAddDtaInitLoc = new common.Action();
        actionAddDtaLoc = new common.Action();
        actionAddDtaAcceptLoc = new common.Action();
        actionAddDtaRejectLoc = new common.Action();
        actionAddDtaInnerEdge = new common.Action();
        actionAddDtaBoundaryEdge = new common.Action();
        actionAddIntConst = new common.Action();
        actionAddRealConst = new common.Action();
        actionAddIntTemplate = new common.Action();
        actionAddRealTemplate = new common.Action();
        actionAddStatePropTemplate = new common.Action();
        actionAddActionTemplate = new common.Action();
        actionAddClockVar = new common.Action();
        actionAddTextBox = new common.Action();
        jToolBarEmpty = new javax.swing.JToolBar();
        popupMenuViewProfile = new javax.swing.JPopupMenu();
        viewRatesMenuItem = new javax.swing.JCheckBoxMenuItem();
        viewGuardsMenuItem = new javax.swing.JCheckBoxMenuItem();
        viewPrioritiesMenuItem = new javax.swing.JCheckBoxMenuItem();
        viewNamesMenuItem = new javax.swing.JCheckBoxMenuItem();
        viewSuperPosTagsMenuItem = new javax.swing.JCheckBoxMenuItem();
        viewProfileSeparator = new javax.swing.JPopupMenu.Separator();
        showTimedCmdMenuItem = new javax.swing.JCheckBoxMenuItem();
        showColorCmdMenuItem = new javax.swing.JCheckBoxMenuItem();
        showFluidCmdMenuItem = new javax.swing.JCheckBoxMenuItem();
        actionAddContPlace = new common.Action();
        actionAddContTransition = new common.Action();
        actionAddColorClass = new common.Action();
        actionAddColorVar = new common.Action();
        actionAddColorClassTemplate = new common.Action();
        resourceFactory = new editor.gui.ResourceFactory();
        jScrollPane = new javax.swing.JScrollPane();
        jPanelToolbars = new javax.swing.JPanel();
        jToolBar = new javax.swing.JToolBar();
        jToggleButtonSelection = new javax.swing.JToggleButton();
        jToggleTriStateAddIntConst = new common.JToggleTriState();
        jToggleTriStateAddRealConst = new common.JToggleTriState();
        jToggleTriStateAddPlace = new common.JToggleTriState();
        jToggleTriStateAddContPlace = new common.JToggleTriState();
        jToggleTriStateAddExpTransition = new common.JToggleTriState();
        jToggleTriStateAddImmTransition = new common.JToggleTriState();
        jToggleTriStateAddGenTransition = new common.JToggleTriState();
        jToggleTriStateAddContTransition = new common.JToggleTriState();
        jToggleTriStateAddDtaInitLoc = new common.JToggleTriState();
        jToggleTriStateAddDtaLoc = new common.JToggleTriState();
        jToggleTriStateAddDtaAcceptLoc = new common.JToggleTriState();
        jToggleTriStateAddDtaRejectLoc = new common.JToggleTriState();
        jToggleTriStateAddClockVar = new common.JToggleTriState();
        jToggleTriStateAddGspnArc = new common.JToggleTriState();
        jToggleTriStateAddGspnInhibArc = new common.JToggleTriState();
        jToggleTriStateAddDtaInnerEdge = new common.JToggleTriState();
        jToggleTriStateAddDtaBoundaryEdge = new common.JToggleTriState();
        jToggleTriStateAddColorClass = new common.JToggleTriState();
        jToggleTriStateAddColorVar = new common.JToggleTriState();
        jToggleTriStateAddIntTemplate = new common.JToggleTriState();
        jToggleTriStateAddRealTemplate = new common.JToggleTriState();
        jToggleTriStateAddStatePropTemplate = new common.JToggleTriState();
        jToggleTriStateAddActionTemplate = new common.JToggleTriState();
        jToggleTriStateAddColorClassTemplate = new common.JToggleTriState();
        jToggleTriStateAddTextBox = new common.JToggleTriState();
        jToolBarEdgeCmd = new javax.swing.JToolBar();
        jSeparator1 = new javax.swing.JToolBar.Separator();
        jToolbarButtonNewPoint = new common.JToolbarButton();
        jToolbarButtonDeletePoint = new common.JToolbarButton();
        jToolbarButtonClearPoints = new common.JToolbarButton();
        jToggleButtonBrokenEdge = new javax.swing.JToggleButton();
        jPanel2 = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jToolBar1 = new javax.swing.JToolBar();
        jToolbarButtonViewProfiles = new common.JToolbarButton();
        jToggleButtonSnapToGrid = new javax.swing.JToggleButton();

        actionSelection.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_ESCAPE, 0));
        actionSelection.setActionName("Selection tool");
        actionSelection.setIcon(resourceFactory.getToolSelect24());
        actionSelection.setTooltipDesc("(Esc) Selection and movement tool.");
        actionSelection.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionSelectionActionPerformed(evt);
            }
        });

        actionToggleRuler.setActionName("Toggle ruler");
        actionToggleRuler.setIcon(resourceFactory.getRuler24());
        actionToggleRuler.setTooltipDesc("Toggles the editor ruler");

        actionAddPlace.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_P, 0));
        actionAddPlace.setActionName("New place");
        actionAddPlace.setIcon(resourceFactory.getToolPlace24());
        actionAddPlace.setTooltipDesc("(P) Add a new (discrete) place  to the GSPN model.");
        actionAddPlace.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddPlaceActionPerformed(evt);
            }
        });

        actionAddExpTransition.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_T, 0));
        actionAddExpTransition.setActionName("New exponential transition");
        actionAddExpTransition.setIcon(resourceFactory.getToolTrn24());
        actionAddExpTransition.setTooltipDesc("(T) Add a new exponential transition to the GSPN model.");
        actionAddExpTransition.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddExpTransitionActionPerformed(evt);
            }
        });

        actionAddImmTransition.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_I, 0));
        actionAddImmTransition.setActionName("New immediate transition");
        actionAddImmTransition.setIcon(resourceFactory.getToolImmTrn24());
        actionAddImmTransition.setTooltipDesc("(I) Add a new immediate transition to the GSPN model.");
        actionAddImmTransition.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddImmTransitionActionPerformed(evt);
            }
        });

        actionAddGenTransition.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_G, 0));
        actionAddGenTransition.setActionName("New general transition");
        actionAddGenTransition.setIcon(resourceFactory.getToolDetTrn24());
        actionAddGenTransition.setTooltipDesc("(G) Add a new general transition to the GSPN model.");
        actionAddGenTransition.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddGenTransitionActionPerformed(evt);
            }
        });

        actionAddGspnArc.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_A, 0));
        actionAddGspnArc.setActionName("New input/output arc");
        actionAddGspnArc.setIcon(resourceFactory.getToolArrow24());
        actionAddGspnArc.setTooltipDesc("(A) Add a new input or output arc between a place and a transition.");
        actionAddGspnArc.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddGspnArcActionPerformed(evt);
            }
        });

        actionAddGspnInhibArc.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_H, 0));
        actionAddGspnInhibArc.setActionName("New inhibitor arc");
        actionAddGspnInhibArc.setIcon(resourceFactory.getToolInhib24());
        actionAddGspnInhibArc.setTooltipDesc("(H) Add a new inhibitor arc.");
        actionAddGspnInhibArc.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddGspnInhibArcActionPerformed(evt);
            }
        });

        actionNewEdgePoint.setActionName("New edge point.");
        actionNewEdgePoint.setIcon(resourceFactory.getEdgePointAdd24());
        actionNewEdgePoint.setTooltipDesc("Add a new point in each selected segments.");
        actionNewEdgePoint.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewEdgePointActionPerformed(evt);
            }
        });

        actionDeleteEdgePoint.setActionName("Delete edge points");
        actionDeleteEdgePoint.setIcon(resourceFactory.getEdgePointDelete24());
        actionDeleteEdgePoint.setTooltipDesc("Delete the selected edge points.");
        actionDeleteEdgePoint.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionDeleteEdgePointActionPerformed(evt);
            }
        });

        actionClearAllEdgePoints.setActionName("Delete all edge points");
        actionClearAllEdgePoints.setIcon(resourceFactory.getEdgePointClear24());
        actionClearAllEdgePoints.setTooltipDesc("Delete all the intermediate edge points of the selected edges.");
        actionClearAllEdgePoints.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionClearAllEdgePointsActionPerformed(evt);
            }
        });

        actionSetBrokenEdge.setActionName("Broken edge");
        actionSetBrokenEdge.setIcon(resourceFactory.getEdgeBroken24());
        actionSetBrokenEdge.setTooltipDesc("Set or disable the selected edge as broken.");
        actionSetBrokenEdge.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionSetBrokenEdgeActionPerformed(evt);
            }
        });

        actionAddDtaInitLoc.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_O, 0));
        actionAddDtaInitLoc.setActionName("New initial location");
        actionAddDtaInitLoc.setIcon(resourceFactory.getToolInitLoc24());
        actionAddDtaInitLoc.setTooltipDesc("(O) Add a new initial location to the current DTA.");
        actionAddDtaInitLoc.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddDtaInitLocActionPerformed(evt);
            }
        });

        actionAddDtaLoc.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_L, 0));
        actionAddDtaLoc.setActionName("New Location");
        actionAddDtaLoc.setIcon(resourceFactory.getToolAddLoc24());
        actionAddDtaLoc.setTooltipDesc("(L) Add a new location to the current DTA.");
        actionAddDtaLoc.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddDtaLocActionPerformed(evt);
            }
        });

        actionAddDtaAcceptLoc.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_C, 0));
        actionAddDtaAcceptLoc.setActionName("New accepting location");
        actionAddDtaAcceptLoc.setIcon(resourceFactory.getToolLocTop24());
        actionAddDtaAcceptLoc.setTooltipDesc("(C) Add a new final accepting location to the current DTA.");
        actionAddDtaAcceptLoc.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddDtaAcceptLocActionPerformed(evt);
            }
        });

        actionAddDtaRejectLoc.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_R, 0));
        actionAddDtaRejectLoc.setActionName("New rejecting location");
        actionAddDtaRejectLoc.setIcon(resourceFactory.getToolLocBot24());
        actionAddDtaRejectLoc.setTooltipDesc("(R) Add a new final rejecting location to the current DTA.");
        actionAddDtaRejectLoc.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddDtaRejectLocActionPerformed(evt);
            }
        });

        actionAddDtaInnerEdge.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_E, 0));
        actionAddDtaInnerEdge.setActionName("New Inner edge");
        actionAddDtaInnerEdge.setIcon(resourceFactory.getToolArrowTriangle24());
        actionAddDtaInnerEdge.setTooltipDesc("(E) Add a new inner edge to the current DTA.");
        actionAddDtaInnerEdge.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddDtaInnerEdgeActionPerformed(evt);
            }
        });

        actionAddDtaBoundaryEdge.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_B, 0));
        actionAddDtaBoundaryEdge.setActionName("New boundary edge");
        actionAddDtaBoundaryEdge.setIcon(resourceFactory.getToolArrowImmed24());
        actionAddDtaBoundaryEdge.setTooltipDesc("(B) Add a new boundary edge to the current DTA.");
        actionAddDtaBoundaryEdge.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddDtaBoundaryEdgeActionPerformed(evt);
            }
        });

        actionAddIntConst.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_N, 0));
        actionAddIntConst.setActionName("New integer constant");
        actionAddIntConst.setIcon(resourceFactory.getConstantN24());
        actionAddIntConst.setTooltipDesc("(N) Add a new integer constant to the current page.");
        actionAddIntConst.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddIntConstActionPerformed(evt);
            }
        });

        actionAddRealConst.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_R, 0));
        actionAddRealConst.setActionName("New real constant");
        actionAddRealConst.setIcon(resourceFactory.getConstantR24());
        actionAddRealConst.setTooltipDesc("(R) Add a new real constant to the current page.");
        actionAddRealConst.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddRealConstActionPerformed(evt);
            }
        });

        actionAddIntTemplate.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_N, java.awt.event.InputEvent.SHIFT_MASK));
        actionAddIntTemplate.setActionName("New template integer Id.");
        actionAddIntTemplate.setIcon(resourceFactory.getToolTemplateN24());
        actionAddIntTemplate.setTooltipDesc("(Shift+N) Add a new integer template Id to the current page.");
        actionAddIntTemplate.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddIntTemplateActionPerformed(evt);
            }
        });

        actionAddRealTemplate.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_R, java.awt.event.InputEvent.SHIFT_MASK));
        actionAddRealTemplate.setActionName("New template real Id.");
        actionAddRealTemplate.setIcon(resourceFactory.getToolTemplateR24());
        actionAddRealTemplate.setTooltipDesc("(Shift+R) Add a new real template Id to the current page.");
        actionAddRealTemplate.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddRealTemplateActionPerformed(evt);
            }
        });

        actionAddStatePropTemplate.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_S, 0));
        actionAddStatePropTemplate.setActionName("New state proposition Id.");
        actionAddStatePropTemplate.setIcon(resourceFactory.getToolTemplateSP24());
        actionAddStatePropTemplate.setTooltipDesc("(S) Add a new state proposition Id to the page.");
        actionAddStatePropTemplate.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddStatePropTemplateActionPerformed(evt);
            }
        });

        actionAddActionTemplate.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_A, java.awt.event.InputEvent.SHIFT_MASK));
        actionAddActionTemplate.setActionName("New action Id.");
        actionAddActionTemplate.setIcon(resourceFactory.getToolTemplateAct24());
        actionAddActionTemplate.setTooltipDesc("(Shift+A) Add a new action Id to the current page.");
        actionAddActionTemplate.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddActionTemplateActionPerformed(evt);
            }
        });

        actionAddClockVar.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_Q, 0));
        actionAddClockVar.setActionName("New clock variable.");
        actionAddClockVar.setIcon(resourceFactory.getToolAddClockvar24());
        actionAddClockVar.setTooltipDesc("(Q) Add a new clock variable to the current page.");
        actionAddClockVar.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddClockVarActionPerformed(evt);
            }
        });

        actionAddTextBox.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_X, 0));
        actionAddTextBox.setActionName("New Text box");
        actionAddTextBox.setIcon(resourceFactory.getLatexNode24());
        actionAddTextBox.setTooltipDesc("(X) Add a new Text Box.");
        actionAddTextBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddTextBoxActionPerformed(evt);
            }
        });

        jToolBarEmpty.setFloatable(false);
        jToolBarEmpty.setRollover(true);

        viewRatesMenuItem.setSelected(true);
        viewRatesMenuItem.setText("Show rates/delays");
        viewRatesMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                viewRatesMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(viewRatesMenuItem);

        viewGuardsMenuItem.setSelected(true);
        viewGuardsMenuItem.setText("Show guards");
        viewGuardsMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                viewGuardsMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(viewGuardsMenuItem);

        viewPrioritiesMenuItem.setSelected(true);
        viewPrioritiesMenuItem.setText("Show priorities");
        viewPrioritiesMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                viewPrioritiesMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(viewPrioritiesMenuItem);

        viewNamesMenuItem.setSelected(true);
        viewNamesMenuItem.setText("Show name labels");
        viewNamesMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                viewNamesMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(viewNamesMenuItem);

        viewSuperPosTagsMenuItem.setSelected(true);
        viewSuperPosTagsMenuItem.setText("Show superposition tags");
        viewSuperPosTagsMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                viewSuperPosTagsMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(viewSuperPosTagsMenuItem);
        popupMenuViewProfile.add(viewProfileSeparator);

        showTimedCmdMenuItem.setSelected(true);
        showTimedCmdMenuItem.setText("Show timed/stochastic elements");
        showTimedCmdMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                showTimedCmdMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(showTimedCmdMenuItem);

        showColorCmdMenuItem.setSelected(true);
        showColorCmdMenuItem.setText("Show color/SWN elements");
        showColorCmdMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                showColorCmdMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(showColorCmdMenuItem);

        showFluidCmdMenuItem.setSelected(true);
        showFluidCmdMenuItem.setText("Show hybrid/fluid elements");
        showFluidCmdMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                showFluidCmdMenuItemActionPerformed(evt);
            }
        });
        popupMenuViewProfile.add(showFluidCmdMenuItem);

        actionAddContPlace.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_K, 0));
        actionAddContPlace.setActionName("New continuous place");
        actionAddContPlace.setIcon(resourceFactory.getToolCplace24());
        actionAddContPlace.setTooltipDesc("(K) Add a new continuous place  to the GSPN model.");
        actionAddContPlace.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddContPlaceActionPerformed(evt);
            }
        });

        actionAddContTransition.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_F, 0));
        actionAddContTransition.setActionName("New continuous transition");
        actionAddContTransition.setIcon(resourceFactory.getToolFlowtrn24());
        actionAddContTransition.setTooltipDesc("(F) Add a new continuous (flow) transition to the GSPN model.");
        actionAddContTransition.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddContTransitionActionPerformed(evt);
            }
        });

        actionAddColorClass.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_D, 0));
        actionAddColorClass.setActionName("New color class/domain");
        actionAddColorClass.setIcon(resourceFactory.getToolColor24());
        actionAddColorClass.setTooltipDesc("(D) Add a new color class/domain to the model.");
        actionAddColorClass.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddColorClassActionPerformed(evt);
            }
        });

        actionAddColorVar.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_V, 0));
        actionAddColorVar.setActionName("New color variable");
        actionAddColorVar.setIcon(resourceFactory.getToolColorvar24());
        actionAddColorVar.setTooltipDesc("(V) Add a new color variable to the model.");
        actionAddColorVar.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddColorVarActionPerformed(evt);
            }
        });

        actionAddColorClassTemplate.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_U, 0));
        actionAddColorClassTemplate.setActionName("New template color class/domain Id");
        actionAddColorClassTemplate.setIcon(resourceFactory.getToolTemplateClr24());
        actionAddColorClassTemplate.setTooltipDesc("(U) Add a new template color class/domain Id to the model.");
        actionAddColorClassTemplate.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAddColorClassTemplateActionPerformed(evt);
            }
        });

        setLayout(new java.awt.BorderLayout());
        add(jScrollPane, java.awt.BorderLayout.CENTER);

        jPanelToolbars.setLayout(new java.awt.GridBagLayout());

        jToolBar.setFloatable(false);
        jToolBar.setRollover(true);

        jToggleButtonSelection.setAction(actionSelection);
        jToggleButtonSelection.setSelected(true);
        jToggleButtonSelection.setFocusable(false);
        jToggleButtonSelection.setHideActionText(true);
        jToggleButtonSelection.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleButtonSelection.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleButtonSelection);

        jToggleTriStateAddIntConst.setAction(actionAddIntConst);
        jToggleTriStateAddIntConst.setFocusable(false);
        jToggleTriStateAddIntConst.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddIntConst.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddIntConst);

        jToggleTriStateAddRealConst.setAction(actionAddRealConst);
        jToggleTriStateAddRealConst.setFocusable(false);
        jToggleTriStateAddRealConst.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddRealConst.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddRealConst);

        jToggleTriStateAddPlace.setAction(actionAddPlace);
        jToggleTriStateAddPlace.setFocusable(false);
        jToggleTriStateAddPlace.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddPlace.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddPlace);

        jToggleTriStateAddContPlace.setAction(actionAddContPlace);
        jToggleTriStateAddContPlace.setFocusable(false);
        jToggleTriStateAddContPlace.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddContPlace.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddContPlace);

        jToggleTriStateAddExpTransition.setAction(actionAddExpTransition);
        jToggleTriStateAddExpTransition.setText("");
        jToggleTriStateAddExpTransition.setFocusable(false);
        jToggleTriStateAddExpTransition.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddExpTransition.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddExpTransition);

        jToggleTriStateAddImmTransition.setAction(actionAddImmTransition);
        jToggleTriStateAddImmTransition.setFocusable(false);
        jToggleTriStateAddImmTransition.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddImmTransition.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddImmTransition);

        jToggleTriStateAddGenTransition.setAction(actionAddGenTransition);
        jToggleTriStateAddGenTransition.setFocusable(false);
        jToggleTriStateAddGenTransition.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddGenTransition.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddGenTransition);

        jToggleTriStateAddContTransition.setAction(actionAddContTransition);
        jToggleTriStateAddContTransition.setFocusable(false);
        jToggleTriStateAddContTransition.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddContTransition.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddContTransition);

        jToggleTriStateAddDtaInitLoc.setAction(actionAddDtaInitLoc);
        jToggleTriStateAddDtaInitLoc.setFocusable(false);
        jToggleTriStateAddDtaInitLoc.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddDtaInitLoc.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddDtaInitLoc);

        jToggleTriStateAddDtaLoc.setAction(actionAddDtaLoc);
        jToggleTriStateAddDtaLoc.setFocusable(false);
        jToggleTriStateAddDtaLoc.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddDtaLoc.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddDtaLoc);

        jToggleTriStateAddDtaAcceptLoc.setAction(actionAddDtaAcceptLoc);
        jToggleTriStateAddDtaAcceptLoc.setFocusable(false);
        jToggleTriStateAddDtaAcceptLoc.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddDtaAcceptLoc.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddDtaAcceptLoc);

        jToggleTriStateAddDtaRejectLoc.setAction(actionAddDtaRejectLoc);
        jToggleTriStateAddDtaRejectLoc.setFocusable(false);
        jToggleTriStateAddDtaRejectLoc.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddDtaRejectLoc.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddDtaRejectLoc);

        jToggleTriStateAddClockVar.setAction(actionAddClockVar);
        jToggleTriStateAddClockVar.setFocusable(false);
        jToggleTriStateAddClockVar.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddClockVar.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddClockVar);

        jToggleTriStateAddGspnArc.setAction(actionAddGspnArc);
        jToggleTriStateAddGspnArc.setFocusable(false);
        jToggleTriStateAddGspnArc.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddGspnArc.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddGspnArc);

        jToggleTriStateAddGspnInhibArc.setAction(actionAddGspnInhibArc);
        jToggleTriStateAddGspnInhibArc.setFocusable(false);
        jToggleTriStateAddGspnInhibArc.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddGspnInhibArc.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddGspnInhibArc);

        jToggleTriStateAddDtaInnerEdge.setAction(actionAddDtaInnerEdge);
        jToggleTriStateAddDtaInnerEdge.setFocusable(false);
        jToggleTriStateAddDtaInnerEdge.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddDtaInnerEdge.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddDtaInnerEdge);

        jToggleTriStateAddDtaBoundaryEdge.setAction(actionAddDtaBoundaryEdge);
        jToggleTriStateAddDtaBoundaryEdge.setFocusable(false);
        jToggleTriStateAddDtaBoundaryEdge.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddDtaBoundaryEdge.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddDtaBoundaryEdge);

        jToggleTriStateAddColorClass.setAction(actionAddColorClass);
        jToggleTriStateAddColorClass.setFocusable(false);
        jToggleTriStateAddColorClass.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddColorClass.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddColorClass);

        jToggleTriStateAddColorVar.setAction(actionAddColorVar);
        jToggleTriStateAddColorVar.setFocusable(false);
        jToggleTriStateAddColorVar.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddColorVar.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddColorVar);

        jToggleTriStateAddIntTemplate.setAction(actionAddIntTemplate);
        jToggleTriStateAddIntTemplate.setFocusable(false);
        jToggleTriStateAddIntTemplate.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddIntTemplate.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddIntTemplate);

        jToggleTriStateAddRealTemplate.setAction(actionAddRealTemplate);
        jToggleTriStateAddRealTemplate.setFocusable(false);
        jToggleTriStateAddRealTemplate.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddRealTemplate.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddRealTemplate);

        jToggleTriStateAddStatePropTemplate.setAction(actionAddStatePropTemplate);
        jToggleTriStateAddStatePropTemplate.setFocusable(false);
        jToggleTriStateAddStatePropTemplate.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddStatePropTemplate.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddStatePropTemplate);

        jToggleTriStateAddActionTemplate.setAction(actionAddActionTemplate);
        jToggleTriStateAddActionTemplate.setFocusable(false);
        jToggleTriStateAddActionTemplate.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddActionTemplate.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddActionTemplate);

        jToggleTriStateAddColorClassTemplate.setAction(actionAddColorClassTemplate);
        jToggleTriStateAddColorClassTemplate.setFocusable(false);
        jToggleTriStateAddColorClassTemplate.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddColorClassTemplate.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddColorClassTemplate);

        jToggleTriStateAddTextBox.setAction(actionAddTextBox);
        jToggleTriStateAddTextBox.setFocusable(false);
        jToggleTriStateAddTextBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleTriStateAddTextBox.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToggleTriStateAddTextBox);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        jPanelToolbars.add(jToolBar, gridBagConstraints);

        jToolBarEdgeCmd.setFloatable(false);
        jToolBarEdgeCmd.setRollover(true);
        jToolBarEdgeCmd.add(jSeparator1);

        jToolbarButtonNewPoint.setAction(actionNewEdgePoint);
        jToolbarButtonNewPoint.setText("");
        jToolBarEdgeCmd.add(jToolbarButtonNewPoint);

        jToolbarButtonDeletePoint.setAction(actionDeleteEdgePoint);
        jToolbarButtonDeletePoint.setText("");
        jToolBarEdgeCmd.add(jToolbarButtonDeletePoint);

        jToolbarButtonClearPoints.setAction(actionClearAllEdgePoints);
        jToolbarButtonClearPoints.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonClearPoints.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBarEdgeCmd.add(jToolbarButtonClearPoints);

        jToggleButtonBrokenEdge.setAction(actionSetBrokenEdge);
        jToggleButtonBrokenEdge.setFocusable(false);
        jToggleButtonBrokenEdge.setHideActionText(true);
        jToggleButtonBrokenEdge.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleButtonBrokenEdge.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBarEdgeCmd.add(jToggleButtonBrokenEdge);

        jPanelToolbars.add(jToolBarEdgeCmd, new java.awt.GridBagConstraints());

        jPanel2.setOpaque(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        jPanelToolbars.add(jPanel2, gridBagConstraints);

        jPanel1.setOpaque(false);
        jPanel1.setLayout(new java.awt.GridLayout(1, 0));

        jToolBar1.setFloatable(false);
        jToolBar1.setRollover(true);

        jToolbarButtonViewProfiles.setIcon(resourceFactory.getThreeTags24());
        jToolbarButtonViewProfiles.setToolTipText("Change the view options.");
        jToolbarButtonViewProfiles.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonViewProfiles.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolbarButtonViewProfiles.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jToolbarButtonViewProfilesActionPerformed(evt);
            }
        });
        jToolBar1.add(jToolbarButtonViewProfiles);

        jToggleButtonSnapToGrid.setIcon(resourceFactory.getSnapToGrid24());
        jToggleButtonSnapToGrid.setToolTipText("Enable/disable grid snapping.\nYou can also toggle grid snapping dinamically\nwhile dragging objects by pressing the ALT key.");
        jToggleButtonSnapToGrid.setFocusable(false);
        jToggleButtonSnapToGrid.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToggleButtonSnapToGrid.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToggleButtonSnapToGrid.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jToggleButtonSnapToGridActionPerformed(evt);
            }
        });
        jToolBar1.add(jToggleButtonSnapToGrid);

        jPanel1.add(jToolBar1);

        jPanelToolbars.add(jPanel1, new java.awt.GridBagConstraints());

        add(jPanelToolbars, java.awt.BorderLayout.NORTH);
    }// </editor-fold>//GEN-END:initComponents

    private void jToggleButtonSnapToGridActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jToggleButtonSnapToGridActionPerformed
        if (currPage != null) {
            currPage.gridSnappingActive = jToggleButtonSnapToGrid.isSelected();
            //mainInterface.updateGUI();
            netPanel.repaint();
        }
        jToggleButtonSnapToGrid.setIcon(jToggleButtonSnapToGrid.isSelected() ? 
                                        ResourceFactory.getInstance().getSnapToGrid24() : 
                                        ResourceFactory.getInstance().getRuler24());
    }//GEN-LAST:event_jToggleButtonSnapToGridActionPerformed
    
    private void actionSelectionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionSelectionActionPerformed
        changeTool(Tool.SELECT);
    }//GEN-LAST:event_actionSelectionActionPerformed

    private void actionAddPlaceActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddPlaceActionPerformed
        changeTool(Tool.NEW_PLACE);
    }//GEN-LAST:event_actionAddPlaceActionPerformed

    private void actionAddExpTransitionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddExpTransitionActionPerformed
        changeTool(Tool.NEW_EXP_TRN);
    }//GEN-LAST:event_actionAddExpTransitionActionPerformed

    private void actionAddImmTransitionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddImmTransitionActionPerformed
        changeTool(Tool.NEW_IMM_TRN);
    }//GEN-LAST:event_actionAddImmTransitionActionPerformed

    private void actionAddGenTransitionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddGenTransitionActionPerformed
        changeTool(Tool.NEW_GEN_TRN);
    }//GEN-LAST:event_actionAddGenTransitionActionPerformed

    private void actionAddGspnArcActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddGspnArcActionPerformed
        changeTool(Tool.NEW_GSPN_ARC);
    }//GEN-LAST:event_actionAddGspnArcActionPerformed

    private void actionAddGspnInhibArcActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddGspnInhibArcActionPerformed
        changeTool(Tool.NEW_GSPN_INHIB_ARC);
    }//GEN-LAST:event_actionAddGspnInhibArcActionPerformed

    private void deleteSelected() {
        mainInterface.executeUndoableCommand("delete selected.", (ProjectData proj, ProjectPage page) -> {
            Set<Node> deletedNodes = new HashSet<>();
            Iterator<Node> nodeIt = currPage.nodes.iterator();
            while (nodeIt.hasNext()) {
                Node n = nodeIt.next();
                if (n.isSelected()) {
                    // Remove the node
                    deletedNodes.add(n);
                    nodeIt.remove();
                }
            }
            Iterator<Edge> edgeIt = currPage.edges.iterator();
            while (edgeIt.hasNext()) {
                Edge e  = edgeIt.next();
                if (e.isSelected() || 
                        deletedNodes.contains(e.getHeadNode()) ||
                        deletedNodes.contains(e.getTailNode()))
                    edgeIt.remove();
            }
        });
    }
    
    private void selectAll() {
        currPage.setSelectionFlag(true);
        mainInterface.setStatus("select all.", true);
        mainInterface.invalidateGUI();
    }
    
    private void deselectAll() {
        currPage.setSelectionFlag(false);
        mainInterface.setStatus("deselect all.", true);
        mainInterface.invalidateGUI();
    }
    
    private void invertSelection() {
        for (Node n : currPage.nodes)
            n.setSelected(!n.isSelected());
        for (Edge e : currPage.edges)
            e.setSelected(!e.isSelected());
        mainInterface.invalidateGUI();
        mainInterface.setStatus("selection inverted.", true);
    }
    
    private void actionNewEdgePointActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewEdgePointActionPerformed
        Selectable selEdge = currPage.getSingleSelectedObject();
        assert selEdge != null && selEdge instanceof Edge;
        final Edge edge = (Edge)selEdge;
        mainInterface.executeUndoableCommand("add intermediate edge points.", (ProjectData proj, ProjectPage page) -> {
            // Save decor positions
            Point2D[] decorAnchors = new Point2D[edge.getNumDecors()];
            for (int d=0; d<edge.getNumDecors(); d++)
                decorAnchors[d] = edge.getPointAlongTheLine(new Point2D.Double(),
                        edge.getDecor(d).getEdgeK());
            // create a new set of points
            ArrayList<Point2D> npoints = new ArrayList<>();
            npoints.add((Point2D)edge.points.get(0).clone());
            Point2D.Double p1 = new Point2D.Double(), p2 = new Point2D.Double();
            for (int i=1; i<edge.numPoints(); i++) {
                if (edge.isSubObjectSelected(i-1) && 
                        edge.isSubObjectSelected(i))
                {
                    // Add a new point
                    edge.getPoint(p1, i-1);
                    edge.getPoint(p2, i);
                    npoints.add(NetObject.linearInterp(p1, p2, new Point2D.Double(), 0.5));
                }
                npoints.add((Point2D)edge.points.get(i).clone());
            }
            edge.points = npoints;
            edge.setSubObjectSelection(false);
            edge.invalidateEffectiveEdgePath();
            // Compute new anchor points (near the old anchors)
            for (int d=0; d<edge.getNumDecors(); d++)
                edge.getDecor(d).setEdgeK(edge.getNearestK(decorAnchors[d]));
        });
    }//GEN-LAST:event_actionNewEdgePointActionPerformed

    private void actionDeleteEdgePointActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionDeleteEdgePointActionPerformed
        Selectable selEdge = currPage.getSingleSelectedObject();
        assert selEdge != null && selEdge instanceof Edge;
        final Edge edge = (Edge)selEdge;
        mainInterface.executeUndoableCommand("delete edge points.", (ProjectData proj, ProjectPage page) -> {
            // Save decor positions
            Point2D[] decorAnchors = new Point2D[edge.getNumDecors()];
            for (int d=0; d<edge.getNumDecors(); d++)
                decorAnchors[d] = edge.getPointAlongTheLine(new Point2D.Double(),
                        edge.getDecor(d).getEdgeK());
            // make a new point by deleting the selected points
            ArrayList<Point2D> npoints = new ArrayList<>();
            for (int i=0; i<edge.numPoints(); i++) {
                if (i == 0 || i == edge.numPoints()-1 || !edge.isSubObjectSelected(i))
                    npoints.add(edge.points.get(i));
            }
            edge.points = npoints;
            edge.setSubObjectSelection(false);
            edge.invalidateEffectiveEdgePath();
            // Compute new anchor points (near the old anchors)
            for (int d=0; d<edge.getNumDecors(); d++)
                edge.getDecor(d).setEdgeK(edge.getNearestK(decorAnchors[d]));
        });
    }//GEN-LAST:event_actionDeleteEdgePointActionPerformed

    private void actionAddDtaInitLocActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddDtaInitLocActionPerformed
        changeTool(Tool.NEW_DTA_INIT_LOC);
    }//GEN-LAST:event_actionAddDtaInitLocActionPerformed

    private void actionAddDtaLocActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddDtaLocActionPerformed
        changeTool(Tool.NEW_DTA_LOC);
    }//GEN-LAST:event_actionAddDtaLocActionPerformed

    private void actionAddDtaAcceptLocActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddDtaAcceptLocActionPerformed
        changeTool(Tool.NEW_DTA_ACCEPT_LOC);
    }//GEN-LAST:event_actionAddDtaAcceptLocActionPerformed

    private void actionAddDtaRejectLocActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddDtaRejectLocActionPerformed
        changeTool(Tool.NEW_DTA_REJECT_LOC);
    }//GEN-LAST:event_actionAddDtaRejectLocActionPerformed

    private void actionAddDtaInnerEdgeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddDtaInnerEdgeActionPerformed
        changeTool(Tool.NEW_DTA_INNER_EDGE);
    }//GEN-LAST:event_actionAddDtaInnerEdgeActionPerformed

    private void actionAddDtaBoundaryEdgeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddDtaBoundaryEdgeActionPerformed
        changeTool(Tool.NEW_DTA_BOUNDARY_EDGE);
    }//GEN-LAST:event_actionAddDtaBoundaryEdgeActionPerformed

    private void actionSetBrokenEdgeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionSetBrokenEdgeActionPerformed
//        Selectable selEdge = currPage.getSingleSelectedObject();
//        assert selEdge != null && selEdge instanceof Edge;
//        final Edge edge = (Edge)selEdge;
        mainInterface.executeUndoableCommand("set edge broken state.", (ProjectData proj, ProjectPage page) -> {
            for (Edge edge : currPage.edges) {
                if (!edge.isSelected())
                    continue;
                assert edge.canBeBroken();
                edge.isBroken = jToggleButtonBrokenEdge.isSelected();
                edge.invalidateEffectiveEdgePath();
                if (edge.isBroken) {
                    for (int d=0; d<edge.getNumDecors(); d++) {
                        Decor decor = edge.getDecor(d);
                        decor.setEdgeK(edge.sanitizeK_ForBrokenEdges(decor.getEdgeK()));
                    }
                }
            }
        });
    }//GEN-LAST:event_actionSetBrokenEdgeActionPerformed
            
    private void actionAddIntConstActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddIntConstActionPerformed
        changeTool(Tool.NEW_INT_CONST);
    }//GEN-LAST:event_actionAddIntConstActionPerformed

    private void actionAddRealConstActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddRealConstActionPerformed
        changeTool(Tool.NEW_REAL_CONST);
    }//GEN-LAST:event_actionAddRealConstActionPerformed

    private void actionAddIntTemplateActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddIntTemplateActionPerformed
        changeTool(Tool.NEW_INT_TEMPLATE);
    }//GEN-LAST:event_actionAddIntTemplateActionPerformed

    private void actionAddRealTemplateActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddRealTemplateActionPerformed
        changeTool(Tool.NEW_REAL_TEMPLATE);
    }//GEN-LAST:event_actionAddRealTemplateActionPerformed

    private void actionAddStatePropTemplateActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddStatePropTemplateActionPerformed
        changeTool(Tool.NEW_STATEPROP);
    }//GEN-LAST:event_actionAddStatePropTemplateActionPerformed

    private void actionAddActionTemplateActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddActionTemplateActionPerformed
        changeTool(Tool.NEW_ACTION);
    }//GEN-LAST:event_actionAddActionTemplateActionPerformed

    private void actionAddClockVarActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddClockVarActionPerformed
        changeTool(Tool.NEW_CLOCKVAR);
    }//GEN-LAST:event_actionAddClockVarActionPerformed

    private long lastViewProfilesPopupClosed = -1;
    private final long TIME_BETWEEN_POPUP_REOPEN = 300;
    
    private void jToolbarButtonViewProfilesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jToolbarButtonViewProfilesActionPerformed
        if (lastViewProfilesPopupClosed == -1) {
            lastViewProfilesPopupClosed = 0;
            popupMenuViewProfile.addPopupMenuListener(new PopupMenuListener() {
                @Override public void popupMenuWillBecomeVisible(PopupMenuEvent e) { }
                @Override public void popupMenuWillBecomeInvisible(PopupMenuEvent e) { 
                    lastViewProfilesPopupClosed = System.currentTimeMillis();
                }
                @Override public void popupMenuCanceled(PopupMenuEvent e) {
                    lastViewProfilesPopupClosed = System.currentTimeMillis();
                }
            });
        }
        else {
            if (System.currentTimeMillis() - lastViewProfilesPopupClosed < TIME_BETWEEN_POPUP_REOPEN)
                return;
        }
        // Update the status of the menu items of the popup
        viewRatesMenuItem.setSelected(currPage.viewProfile.viewRatesDelays);
        viewGuardsMenuItem.setSelected(currPage.viewProfile.viewGuards);
        viewPrioritiesMenuItem.setSelected(currPage.viewProfile.viewPriorities);
        viewNamesMenuItem.setSelected(currPage.viewProfile.viewNames);
        viewSuperPosTagsMenuItem.setSelected(currPage.viewProfile.viewSuperPosTags);
        showTimedCmdMenuItem.setSelected(currPage.viewProfile.showTimedCommands);
        showColorCmdMenuItem.setSelected(currPage.viewProfile.showColorCommands);
        showFluidCmdMenuItem.setSelected(currPage.viewProfile.showFluidCommands);
        // Open the view profile menu as a popup
        Util.reformatMenuPanels(popupMenuViewProfile);
        popupMenuViewProfile.pack();
        Dimension popupDim = popupMenuViewProfile.getPreferredSize();
        popupMenuViewProfile.show(jToolbarButtonViewProfiles, 
                                  jToolbarButtonViewProfiles.getWidth() - popupDim.width, 
                                  jToolbarButtonViewProfiles.getHeight());
    }//GEN-LAST:event_jToolbarButtonViewProfilesActionPerformed

    private void viewRatesMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_viewRatesMenuItemActionPerformed
        currPage.viewProfile.viewRatesDelays = viewRatesMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_viewRatesMenuItemActionPerformed

    private void viewPrioritiesMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_viewPrioritiesMenuItemActionPerformed
        currPage.viewProfile.viewPriorities = viewPrioritiesMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_viewPrioritiesMenuItemActionPerformed

    private void viewNamesMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_viewNamesMenuItemActionPerformed
        currPage.viewProfile.viewNames = viewNamesMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_viewNamesMenuItemActionPerformed

    private void viewSuperPosTagsMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_viewSuperPosTagsMenuItemActionPerformed
        currPage.viewProfile.viewSuperPosTags = viewSuperPosTagsMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_viewSuperPosTagsMenuItemActionPerformed

    private void actionAddTextBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddTextBoxActionPerformed
        changeTool(Tool.NEW_TEXT_BOX_NODE);
    }//GEN-LAST:event_actionAddTextBoxActionPerformed

    private void actionAddContPlaceActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddContPlaceActionPerformed
        changeTool(Tool.NEW_CONT_PLACE);
    }//GEN-LAST:event_actionAddContPlaceActionPerformed

    private void actionAddContTransitionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddContTransitionActionPerformed
        changeTool(Tool.NEW_CONT_TRN);
    }//GEN-LAST:event_actionAddContTransitionActionPerformed

    private void actionAddColorClassActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddColorClassActionPerformed
        changeTool(Tool.NEW_COLOR_CLASS);
    }//GEN-LAST:event_actionAddColorClassActionPerformed

    private void actionAddColorVarActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddColorVarActionPerformed
        changeTool(Tool.NEW_COLOR_VAR);
    }//GEN-LAST:event_actionAddColorVarActionPerformed

    private void actionAddColorClassTemplateActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAddColorClassTemplateActionPerformed
        changeTool(Tool.NEW_COLOR_CLASS_TEMPLATE);
    }//GEN-LAST:event_actionAddColorClassTemplateActionPerformed

    private void viewGuardsMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_viewGuardsMenuItemActionPerformed
        currPage.viewProfile.viewGuards = viewGuardsMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_viewGuardsMenuItemActionPerformed

    private void showTimedCmdMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_showTimedCmdMenuItemActionPerformed
        currPage.viewProfile.showTimedCommands = showTimedCmdMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_showTimedCmdMenuItemActionPerformed

    private void showColorCmdMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_showColorCmdMenuItemActionPerformed
        currPage.viewProfile.showColorCommands = showColorCmdMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_showColorCmdMenuItemActionPerformed

    private void showFluidCmdMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_showFluidCmdMenuItemActionPerformed
        currPage.viewProfile.showFluidCommands = showFluidCmdMenuItem.isSelected();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_showFluidCmdMenuItemActionPerformed

    private void actionClearAllEdgePointsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionClearAllEdgePointsActionPerformed
        mainInterface.executeUndoableCommand("delete intermediate edge points.", (ProjectData proj, ProjectPage page) -> {
            assert page == currPage;
            for (Edge edge : currPage.edges) {
                if (!edge.isSelected())
                    continue; // not selected
                if (edge.numPoints() <= 2)
                    continue; // nothing to delete
                boolean isSelfLoop = (edge.getHeadNode() == edge.getTailNode());
                // Save decor positions
                Point2D[] decorAnchors = new Point2D[edge.getNumDecors()];
                for (int d=0; d<edge.getNumDecors(); d++)
                    decorAnchors[d] = edge.getPointAlongTheLine(new Point2D.Double(),
                            edge.getDecor(d).getEdgeK());
                // make a new pointList by deleting the selected points
                ArrayList<Point2D> npoints = new ArrayList<>(isSelfLoop ? 3 : 2);
                npoints.add(edge.points.get(0));
                if (isSelfLoop)
                    npoints.add(edge.points.get(1));
                npoints.add(edge.points.get(edge.numPoints() - 1));
                edge.points = npoints;
                edge.setSubObjectSelection(false);
                edge.invalidateEffectiveEdgePath();
                // Compute new anchor points (near the old anchors)
                for (int d=0; d<edge.getNumDecors(); d++)
                    edge.getDecor(d).setEdgeK(edge.getNearestK(decorAnchors[d]));
            }
        });
    }//GEN-LAST:event_actionClearAllEdgePointsActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private common.Action actionAddActionTemplate;
    private common.Action actionAddClockVar;
    private common.Action actionAddColorClass;
    private common.Action actionAddColorClassTemplate;
    private common.Action actionAddColorVar;
    private common.Action actionAddContPlace;
    private common.Action actionAddContTransition;
    private common.Action actionAddDtaAcceptLoc;
    private common.Action actionAddDtaBoundaryEdge;
    private common.Action actionAddDtaInitLoc;
    private common.Action actionAddDtaInnerEdge;
    private common.Action actionAddDtaLoc;
    private common.Action actionAddDtaRejectLoc;
    private common.Action actionAddExpTransition;
    private common.Action actionAddGenTransition;
    private common.Action actionAddGspnArc;
    private common.Action actionAddGspnInhibArc;
    private common.Action actionAddImmTransition;
    private common.Action actionAddIntConst;
    private common.Action actionAddIntTemplate;
    private common.Action actionAddPlace;
    private common.Action actionAddRealConst;
    private common.Action actionAddRealTemplate;
    private common.Action actionAddStatePropTemplate;
    private common.Action actionAddTextBox;
    private common.Action actionClearAllEdgePoints;
    private common.Action actionDeleteEdgePoint;
    private common.Action actionNewEdgePoint;
    private common.Action actionSelection;
    private common.Action actionSetBrokenEdge;
    private common.Action actionToggleRuler;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanelToolbars;
    private javax.swing.JScrollPane jScrollPane;
    private javax.swing.JToolBar.Separator jSeparator1;
    private javax.swing.JToggleButton jToggleButtonBrokenEdge;
    private javax.swing.JToggleButton jToggleButtonSelection;
    private javax.swing.JToggleButton jToggleButtonSnapToGrid;
    private common.JToggleTriState jToggleTriStateAddActionTemplate;
    private common.JToggleTriState jToggleTriStateAddClockVar;
    private common.JToggleTriState jToggleTriStateAddColorClass;
    private common.JToggleTriState jToggleTriStateAddColorClassTemplate;
    private common.JToggleTriState jToggleTriStateAddColorVar;
    private common.JToggleTriState jToggleTriStateAddContPlace;
    private common.JToggleTriState jToggleTriStateAddContTransition;
    private common.JToggleTriState jToggleTriStateAddDtaAcceptLoc;
    private common.JToggleTriState jToggleTriStateAddDtaBoundaryEdge;
    private common.JToggleTriState jToggleTriStateAddDtaInitLoc;
    private common.JToggleTriState jToggleTriStateAddDtaInnerEdge;
    private common.JToggleTriState jToggleTriStateAddDtaLoc;
    private common.JToggleTriState jToggleTriStateAddDtaRejectLoc;
    private common.JToggleTriState jToggleTriStateAddExpTransition;
    private common.JToggleTriState jToggleTriStateAddGenTransition;
    private common.JToggleTriState jToggleTriStateAddGspnArc;
    private common.JToggleTriState jToggleTriStateAddGspnInhibArc;
    private common.JToggleTriState jToggleTriStateAddImmTransition;
    private common.JToggleTriState jToggleTriStateAddIntConst;
    private common.JToggleTriState jToggleTriStateAddIntTemplate;
    private common.JToggleTriState jToggleTriStateAddPlace;
    private common.JToggleTriState jToggleTriStateAddRealConst;
    private common.JToggleTriState jToggleTriStateAddRealTemplate;
    private common.JToggleTriState jToggleTriStateAddStatePropTemplate;
    private common.JToggleTriState jToggleTriStateAddTextBox;
    private javax.swing.JToolBar jToolBar;
    private javax.swing.JToolBar jToolBar1;
    private javax.swing.JToolBar jToolBarEdgeCmd;
    private javax.swing.JToolBar jToolBarEmpty;
    private common.JToolbarButton jToolbarButtonClearPoints;
    private common.JToolbarButton jToolbarButtonDeletePoint;
    private common.JToolbarButton jToolbarButtonNewPoint;
    private common.JToolbarButton jToolbarButtonViewProfiles;
    private javax.swing.JPopupMenu popupMenuViewProfile;
    private editor.gui.ResourceFactory resourceFactory;
    private javax.swing.JCheckBoxMenuItem showColorCmdMenuItem;
    private javax.swing.JCheckBoxMenuItem showFluidCmdMenuItem;
    private javax.swing.JCheckBoxMenuItem showTimedCmdMenuItem;
    private javax.swing.JCheckBoxMenuItem viewGuardsMenuItem;
    private javax.swing.JCheckBoxMenuItem viewNamesMenuItem;
    private javax.swing.JCheckBoxMenuItem viewPrioritiesMenuItem;
    private javax.swing.JPopupMenu.Separator viewProfileSeparator;
    private javax.swing.JCheckBoxMenuItem viewRatesMenuItem;
    private javax.swing.JCheckBoxMenuItem viewSuperPosTagsMenuItem;
    // End of variables declaration//GEN-END:variables

}
