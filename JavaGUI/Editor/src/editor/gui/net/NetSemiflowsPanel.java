/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.Util;
import editor.domain.DrawHelper;
import editor.domain.Edge;
import editor.domain.NetObject;
import editor.domain.elements.GspnEdge;
import editor.domain.elements.GspnPage;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.elements.Place;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import static editor.domain.ProjectPage.NO_ERRORS;
import editor.domain.Selectable;
import editor.domain.elements.Transition;
import editor.domain.semiflows.FlowsGenerator;
import editor.domain.composition.ComposableNet;
import editor.domain.semiflows.NetIndex;
import editor.gui.AbstractPageEditor;
import editor.gui.MainWindowInterface;
import editor.gui.SharedResourceProvider;
import java.awt.CardLayout;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import javax.swing.DefaultListModel;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.event.ListSelectionEvent;
import editor.domain.semiflows.PTFlows;
import javax.swing.UIManager;

/**
 *
 * @author elvio
 */
public class NetSemiflowsPanel extends javax.swing.JPanel implements AbstractPageEditor {
    
    PTFlows.Type sfType;
    boolean hasSemiflows = false;

    // The GSPN viewer
    NetViewerPanel viewerPanel;
    
    // Original GSPN reference
    GspnPage origGspn;
    
    MainWindowInterface mainInterface;
    
    // Indices for easy P/T objects manipulation
    NetIndex netIndex;
    
    // The list of computed flows
    FlowsGenerator algo;
    PageErrorWarning[] algoErrsWarns;
    
    // Highlighted places, transitions and edges
    Set<Selectable> highlightedElems;
    private Timer showTimer = null;
    private int dashPhase = 0;
    
    // selection management
    private boolean updatingList = false;
    private Node selectedNode = null;
    
    private static final String CONTROLS_CARD = "controls";
    private static final String BINDING_CARD = "binding";
    
    /**
     * Creates new form NetSemiflowsPanel
     * @param shActProv the set of shared actions.
     */
    public NetSemiflowsPanel(SharedResourceProvider shActProv) {
        initComponents();
        Util.reformatPanelsForPlatformLookAndFeel(this);
        
        jScrollPaneNet.getViewport().setOpaque(true);
        jScrollPaneNet.getViewport().setBackground(NetEditorPanel.OUTSIDE_PAGE_COLOR);
        
        jToolbarButtonChangeBindings.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.CHANGE_BINDINGS));
        jToolbarButtonComputePlaceSemiflows.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.PLACE_SEMIFLOWS));
        jToolbarButtonComputePlaceFlows.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.PLACE_FLOWS));
        jToolbarButtonComputeTransitionSemiflows.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.TRANS_SEMIFLOWS));
        jToolbarButtonComputeTransitionFlows.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.TRANS_FLOWS));
        jToolbarButtonComputeBoundsFromPinv.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.PLACE_BOUNDS_FROM_SEMIFLOWS));
        jToolbarButtonComputePlaceBasis.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.PLACE_BASIS));
        jToolbarButtonComputeTransitionBasis.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.TRANS_BASIS));
        jToolbarButtonComputeTraps.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.TRAPS));
        jToolbarButtonComputeSiphons.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.SIPHONS));
        jToolbarButtonShowMatrices.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.SHOW_NET_MATRICES));
        
        jToolbarButtonChangeBindings.setHideActionText(false);
        
        jList_flows.getSelectionModel().addListSelectionListener((ListSelectionEvent lse) -> {
            if (updatingList)
                return;
            updateListPanel(false, jList_flows.getSelectedIndex());
//            highlightFlow(jList_flows.getSelectedIndex());
            viewerPanel.repaint();
        });
        
        showTimer = new Timer(100, (ActionEvent ae) -> {
            //                System.out.println("timer!");
            dashPhase++;
            viewerPanel.repaint();
        });
        showTimer.setDelay(300);
        showTimer.stop();
    }
    
    public void setAnalysisType(PTFlows.Type sfType) {
        this.sfType = sfType;
    }

    @Override
    public void setEditorEnabledFor(ProjectFile pf, ProjectPage page, MainWindowInterface interf) {
        assert mainInterface == null && origGspn == null && viewerPanel == null;
        assert page != null && page instanceof ComposableNet && page.isPageCorrect();
        mainInterface = interf;
        
        origGspn = (GspnPage)((ComposableNet)page).getComposedNet();
        GspnPage gspn = (GspnPage)Util.deepCopyRelink(origGspn);
        gspn.preparePageCheck();
        gspn.checkPage(null, null, null, null);
        
        gspn.setSelectionFlag(false);
        gspn.setGrayedFlag(false);
        gspn.setAuraFlag(false);
        gspn.viewProfile.viewPriorities = false;
        gspn.viewProfile.viewRatesDelays = false;
        gspn.viewProfile.viewSuperPosTags = false;
        
        netIndex = new NetIndex(gspn);
        
        // Setup the net viewer in the scrollpane
        viewerPanel = new SemiflowViewerPanel(jScrollPaneNet);
        viewerPanel.initializePanel(gspn, null, true /* show binding forms */);
        jScrollPaneNet.setViewportView(viewerPanel);
        
        restartFromBeginning(true);
    }

    @Override
    public void setEditorDisabled() {
        // save template bindings
        viewerPanel.saveBindingParameters(mainInterface, origGspn, null);
        
        showTimer.stop();
        assert mainInterface != null;
        mainInterface = null;
        origGspn = null;
        viewerPanel.disablePanel();
        jScrollPaneNet.setViewportView(null);
        viewerPanel = null;
        hasSemiflows = false;
        netIndex = null;
        selectedNode = null;
        algo = null;
    }

    @Override
    public void updateGUI(ProjectFile pf, ProjectPage page) {
    }

    @Override
    public void updateEnablingOfSharedActions(SharedResourceProvider shResProv) {
       // boolean canBeUsed = !isFiring() && !mainInterface.isSuspended();
        
        for (SharedResourceProvider.ActionName actName : SharedResourceProvider.ActionName.values()) {
            common.Action act = shResProv.getSharedAction(actName);
            
            switch (actName) {
                case CHANGE_BINDINGS:
                    act.setEnabled(viewerPanel.hasBindingForms() && !viewerPanel.isInBindingPhase());
                    break;
                    
                case PLACE_SEMIFLOWS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.PLACE_SEMIFLOWS));
                    break;
                    
                case PLACE_FLOWS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.PLACE_FLOWS));
                    break;
                    
                case TRANS_SEMIFLOWS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.TRANSITION_SEMIFLOWS));
                    break;
                    
                case TRANS_FLOWS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.TRANSITION_FLOWS));
                    break;
                    
                case PLACE_BOUNDS_FROM_SEMIFLOWS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.PLACE_BOUNDS_FROM_PINV));
                    break;

                case PLACE_BASIS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.PLACE_BASIS));
                    break;
 
                 case TRANS_BASIS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.TRANSITION_BASIS));
                    break;
 
                case TRAPS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.TRAPS));
                    break;
 
                case SIPHONS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != PTFlows.Type.SIPHONS));
                    break;
 
                case SHOW_NET_MATRICES:
                    act.setEnabled(!viewerPanel.isInBindingPhase() &&
                                   (sfType != PTFlows.Type.PLACE_BOUNDS_FROM_PINV));
                    break;
                
                default:
                    act.setEnabled(false);
            }
        }
    }

    @Override
    public void sharedActionListener(SharedResourceProvider.ActionName shAction, ActionEvent event) {
        switch (shAction) {
            case CHANGE_BINDINGS:
                restartFromBeginning(true);
                break;
                
            case PLACE_SEMIFLOWS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.PLACE_SEMIFLOWS);
                break;
                
            case PLACE_FLOWS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.PLACE_FLOWS);
                break;
                
            case TRANS_SEMIFLOWS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.TRANSITION_SEMIFLOWS);
                break;

            case TRANS_FLOWS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.TRANSITION_FLOWS);
                break;
                
            case PLACE_BOUNDS_FROM_SEMIFLOWS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.PLACE_BOUNDS_FROM_PINV);
                break;
                
            case PLACE_BASIS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.PLACE_BASIS);
                break;
                
            case TRANS_BASIS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.TRANSITION_BASIS);
                break;
                
            case TRAPS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.TRAPS);
                break;
                
            case SIPHONS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeFlowsLater(PTFlows.Type.SIPHONS);
                break;
                
            case SHOW_NET_MATRICES:
                showFlowsMatrix();
                break;
            
            default:
                throw new UnsupportedOperationException();
        }
    }
    
    public void recomputeFlowsLater(final PTFlows.Type type) {
        SwingUtilities.invokeLater(() -> {
            jList_flows.removeAll();
            recomputeFlows(type);
        });
    }
    
    private void updateListPanel(boolean recomputed, int selIndex) {
        if (recomputed) {
            selectedNode = null;
        }
        updatingList = true;
//        int selIndex = jList_flows.getSelectedIndex();
        // Update window
        DefaultListModel<String> model = new DefaultListModel<>();
        if (algo != null) {
            if (sfType.isBound()) {
                for (int p=0; p<netIndex.numPlaces(); p++) {
                    StringBuilder name = new StringBuilder();
                    name.append("Bound of ").append(netIndex.places.get(p).getUniqueName())
                        .append(": [").append(algo.getLowerBoundOf(p)).append(", ");
                    if (algo.getUpperBoundOf(p) == Integer.MAX_VALUE)
                        name.append(PTFlows.INFINITY_UNICODE);
                    else
                        name.append(algo.getUpperBoundOf(p));
                    name.append("]");
                    model.addElement(name.toString());
                }
            }
            else {
                final int numSemiflows = (algo.isComputed() ? algo.numFlows() : 0);
                for (int i=0; i<numSemiflows; i++)
                    model.addElement(algo.flowToString(i, netIndex, true, 
                            UIManager.getColor("List.foreground"), 
                            UIManager.getColor("List.background"),
                            selectedNode, selIndex));
            }
        }
        jList_flows.setModel(model);
        if (selIndex >= 0)
            selIndex = Math.min(selIndex, model.size());
        else
            selIndex = -1;
        if (model.size() > 0)
            jList_flows.setSelectedIndex(selIndex);
        highlightFlow(jList_flows.getSelectedIndex());
        updatingList = false;
    }
    
    private void recomputeFlows(PTFlows.Type type) {
        this.sfType = type;
        highlightedElems = null;
        selectedNode = null;
        
        // Both P-flows and Place bounds are initialized in the same way
        boolean initializeForBounds = (sfType == PTFlows.Type.PLACE_BOUNDS_FROM_PINV);
        
        algo = FlowsGenerator.makeFor(sfType, netIndex);
//        if (sfType.isPlace())
//            algo = new FlowsGenerator(netIndex.numPlaces(), netIndex.numTransition());
//        else
//            algo = new FlowsGenerator(netIndex.numTransition(), netIndex.numPlaces());
//        algo.onlySemiflows = sfType.isSemiflow();
        algo.initialize(type, viewerPanel.getGspnBinding(), netIndex);
        final int MAX_ROWS = 100000;
        algo.setMaxRows(MAX_ROWS);
        
        // Compute (semi)flows (in a separate thread)
        SemiflowComputationDialog dlg = new SemiflowComputationDialog(mainInterface.getWindowFrame(), algo);
        dlg.showDialogAndStart();
//        algo.compute(false /*log*/);
        
//        // Update window
//        DefaultListModel<String> model = new DefaultListModel<>();
//        if (initializeForBounds) {
//            for (int p=0; p<netIndex.numPlaces(); p++) {
//                StringBuilder name = new StringBuilder();
//                name.append("Bound of ").append(netIndex.places.get(p).getUniqueName())
//                    .append(": [").append(algo.getLowerBoundOf(p)).append(", ");
//                if (algo.getUpperBoundOf(p) == Integer.MAX_VALUE)
//                    name.append(PTFlows.INFINITY_UNICODE);
//                else
//                    name.append(algo.getUpperBoundOf(p));
//                name.append("]");
//                model.addElement(name.toString());
//            }
//        }
//        else {
//            final int numSemiflows = (algo.isComputed() ? algo.numFlows() : 0);
//            for (int i=0; i<numSemiflows; i++)
//                model.addElement(algo.flowToString(i, netIndex, true, UIManager.getColor("List.foreground")));
//        }
//        jList_flows.setModel(model);
//        if (model.size() > 0)
//            jList_flows.setSelectedIndex(0);
//        highlightFlow(jList_flows.getSelectedIndex());
        updateListPanel(true, 0);

        // Update titles and warnings
        jLabel_computedFlows.setVisible(true);
        jLabel_computedFlows.setText(sfType.printableName());
        jLabel_computedFlows.setIcon(sfType.getIcon32());
        
        // Make the list of errors
        ArrayList<PageErrorWarning> errsWarns = new ArrayList<>();
        if (!algo.isComputed()) {
            errsWarns.add(PageErrorWarning.newError(algo.getFailureReason(), 
                          new Selectable.DummySelectable()));
        }
        else {
            if (algo.hasInhibitorEdges)
                errsWarns.add(PageErrorWarning.newWarning("Inhibitor edges have no impact on this "
                              + "analysis, and are ignored.", new Selectable.DummySelectable()));
            if (algo.hasMarkDepEdges)
                errsWarns.add(PageErrorWarning.newWarning("Marking-dependent edges are not supported. "
                                + "Using cardinality 1 for them.", new Selectable.DummySelectable()));
            if (algo.hasEdgeWithZeroCard)
                errsWarns.add(PageErrorWarning.newWarning("The net has edges with cardinality 0.", 
                                                          new Selectable.DummySelectable()));
            if (algo.hasContinuousEdges)
                errsWarns.add(PageErrorWarning.newWarning("Continuous transition/places/edges are ignored.", 
                              new Selectable.DummySelectable()));
            if (algo.hasColoredPlaces)
                errsWarns.add(PageErrorWarning.newWarning("Colored places are not supported and are ignored.",
                              new Selectable.DummySelectable()));
            
            if (algo.hasNonIntegerInitMarks)
                errsWarns.add(PageErrorWarning.newWarning("Place bounds can only be computed if all places "
                        + "have a scalar integer initial marking.",
                              new Selectable.DummySelectable()));
                
            // Determine if all places/transitions are covered by P/T invariants
            int numPT = (sfType.isPlace()) ? 
                         netIndex.numPlaces() : netIndex.numTransition();
            boolean[] covered = new boolean[numPT];
            Arrays.fill(covered, false);
            for (int sf=0; sf<algo.numFlows(); sf++) {
                int[] flow = algo.getFlowVector(sf);
                for (int i=0; i<flow.length; i++)
                    if (flow[i] != 0)
                        covered[i] = true;
            }
            int numUncover = 0;
            for (int i=0; i<numPT; i++)
                if (!covered[i])
                    numUncover++;
            if (numUncover > 0) {
                String err = ""+numUncover;
                if (sfType.isPlace()) {
                    err += ((numUncover==1) ? " place is " : " places are ")
                            +"not covered by P-"+(sfType.isSemiflow()?"semi":"")+"flows: ";
                    int j = numUncover;
                    for (int i=0; i<numPT; i++)
                        if (!covered[i])
                            err += netIndex.places.get(i).getUniqueName() + (--j>0?", ":"");
                    if (initializeForBounds) 
                        err += (numUncover==1) ? ". No finite bound is available for this place" : 
                                    ". No finite bounds are available for these places.";
                }
                else { // Transitions
                    err += ((numUncover==1) ? " transition is " : " transitions are ")
                            +"not covered by T-"+(sfType.isSemiflow()?"semi":"")+"flows: ";
                    for (int i=0; i<numPT; i++)
                        if (!covered[i])
                            err += netIndex.transitions.get(i).getUniqueName() + (--numUncover>0?", ":"");                    
                }
                errsWarns.add(PageErrorWarning.newWarning(err, new Selectable.DummySelectable()));
            }
        }
        
        // Show the error/warning list
        if (errsWarns.isEmpty())
            algoErrsWarns = NO_ERRORS;
        else {
            algoErrsWarns = new PageErrorWarning[errsWarns.size()];
            for (int i=0; i<errsWarns.size(); i++)
                algoErrsWarns[i] = errsWarns.get(i);
        }
        
        hasSemiflows = algo.isComputed();
        showTimer.restart();
        mainInterface.invalidateGUI();
        viewerPanel.repaint();
    }
    
    public void closeBindingForm() {
        jLabel_Binding.setVisible(false);
        viewerPanel.closeBindingForm();
        ((CardLayout)getLayout()).show(this, CONTROLS_CARD);
    }
    
    private void restartFromBeginning(boolean reopenBindings) {
        algo = null;
        highlightedElems = null;
        selectedNode = null;
        viewerPanel.reopenBindingForms();
        ((CardLayout)getLayout()).show(this, BINDING_CARD);
        jLabel_Binding.setVisible(true);
        jLabel_computedFlows.setVisible(false);
        jList_flows.setModel(new DefaultListModel<>());
        algoErrsWarns = NO_ERRORS;
        showTimer.stop();

        if (!viewerPanel.hasBindingForms() || !reopenBindings) {
            closeBindingForm();
            recomputeFlowsLater(sfType);
        }
        else {
            hasSemiflows = false;
            mainInterface.invalidateGUI();
            viewerPanel.repaint();
        }
    }
    
    private void highlightFlow(int sfNum) {
        if (sfNum < 0 || sfNum >= jList_flows.getModel().getSize()) {
            highlightedElems = null;
            return;
        }
        highlightedElems = new HashSet<>();
        GspnPage gspn = viewerPanel.getGspn();
        
        if (sfType == PTFlows.Type.PLACE_BOUNDS_FROM_PINV) {
            // highlight place number sfNum
            highlightedElems.add(netIndex.places.get(sfNum));
        }
        else if (sfType.isPlace()) {
            int[] placeSF = algo.getFlowVector(sfNum);
            // [1] Add all places of the flow into the list
            for (int k=0; k<placeSF.length; k++)
                if (placeSF[k] != 0)
                    highlightedElems.add(netIndex.places.get(k));
            // [2] Add all transitions that have both an input and an output edge
            //     between highlighted places
            Set<Transition> inputTrns = new HashSet<>();
            for (Edge edge : gspn.edges)
                if (edge instanceof GspnEdge && ((GspnEdge)edge).getEdgeKind() == GspnEdge.Kind.INPUT &&
                    highlightedElems.contains((Place)edge.getTailNode()))
                    inputTrns.add((Transition)edge.getHeadNode());
            for (Edge edge : gspn.edges)
                if (edge instanceof GspnEdge && ((GspnEdge)edge).getEdgeKind() == GspnEdge.Kind.OUTPUT &&
                    inputTrns.contains((Transition)edge.getTailNode()))
                    highlightedElems.add((Transition)edge.getTailNode());
        }
        else if (sfType.isTransition()) {                
            int[] trnSF = algo.getFlowVector(sfNum);
            // [1] Add all transitions of the flow into the list
            for (int k=0; k<trnSF.length; k++)
                if (trnSF[k] != 0)
                    highlightedElems.add(netIndex.transitions.get(k));
            // [2] Add all places that have both an input and an output edge
            //     between highlighted transitions
            Set<Place> inputPlaces = new HashSet<>();
            for (Edge edge : gspn.edges)
                if (edge instanceof GspnEdge && ((GspnEdge)edge).getEdgeKind() == GspnEdge.Kind.INPUT &&
                    highlightedElems.contains((Transition)edge.getHeadNode()))
                    inputPlaces.add((Place)edge.getTailNode());
            for (Edge edge : gspn.edges)
                if (edge instanceof GspnEdge && ((GspnEdge)edge).getEdgeKind() == GspnEdge.Kind.OUTPUT &&
                    inputPlaces.contains((Place)edge.getHeadNode()))
                    highlightedElems.add((Place)edge.getHeadNode());
        }
        
        // Highlight edges between place/transitions, when visualizing flows
        if (sfType != PTFlows.Type.PLACE_BOUNDS_FROM_PINV) {
            // [3] Add all input/output edges between selected places and transitions
            for (Edge edge : gspn.edges)
                if (edge instanceof GspnEdge && 
                    ((GspnEdge)edge).getEdgeKind() != GspnEdge.Kind.INHIBITOR &&
                    highlightedElems.contains(edge.getHeadNode()) && 
                    highlightedElems.contains(edge.getTailNode()))
                    highlightedElems.add(edge);
        }
    }
    
    private void showFlowsMatrix() {
        ShowFlowsMatrixDialog dlg;
        dlg = new ShowFlowsMatrixDialog(mainInterface.getWindowFrame(), true, 
                                        algo, origGspn.getPageName(), netIndex);
        dlg.setVisible(true);
    }
    
    @Override
    public JComponent getEditorComponent() {
        return jPanelNet;
    }

    @Override
    public JComponent getFocusTarget() {
        return jPanelNet;
    }

    @Override
    public JComponent getPropertyPanel() {
        return this;
    }
    
    @Override
    public JComponent[] getToolbars() {
        return new JComponent[]{toolbar_basic, toolbar_advanced};
    }

    @Override
    public boolean isModalEditor() {
        return true;
    }

    @Override
    public boolean isZoomPanelUsed() {
        return true;
    }
    
    @Override
    public PageErrorWarning[] getAllErrorsAndWarnings() {
        return algoErrsWarns;
    }

    @Override
    public int getPageZoom() {
        return viewerPanel.getZoomLevel();
    }

    @Override
    public void zoomChanged(int newZoomLevel) {
        viewerPanel.zoomChanged(newZoomLevel);
    }
    
    private static final Color POSITIVE_PLACE_LINE_CLR = new Color(0x5ab4f0);
    private static final Color NEGATIVE_PLACE_LINE_CLR = new Color(0x8975FF);
    private static final Color POSITIVE_TRANSITION_LINE_CLR = new Color(0xf08246);
    private static final Color NEGATIVE_TRANSITION_LINE_CLR = new Color(0xC06CE5);
    
    private static final Color POSITIVE_PLACE_TEXT_CLR = new Color(0x2832b4);
    private static final Color NEGATIVE_PLACE_TEXT_CLR = new Color(0x5f32b4);
    private static final Color POSITIVE_TRANSITION_TEXT_CLR = new Color(0xbe2814);
    private static final Color NEGATIVE_TRANSITION_TEXT_CLR = new Color(0x5F008A);
    
    private static final Color TRAP_PLACE_TEXT_CLR = new Color(0x604800);
    private static final Color TRAP_PLACE_LINE_CLR = new Color(0xF0B400);
    private static final Color SIPHON_PLACE_TEXT_CLR = new Color(0x283F3A);
    private static final Color SIPHON_PLACE_LINE_CLR = new Color(0x70c2ae);


    private static final Color PLACE_BOUND_LINE_CLR = new Color(0x00d200);
    private static final Color PLACE_BOUND_TEXT_CLR = new Color(0x00a000);

    
    private class SelectedFlow implements PTFlows {

        @Override
        public boolean contains(Selectable elem) {
            if (algo == null || highlightedElems == null)
                return false;
            return highlightedElems.contains(elem);
        }

        @Override
        public int getNodeCardinality(Selectable elem) {
            if (algo == null)
                return Integer.MAX_VALUE;
            int selSF = jList_flows.getSelectedIndex();
            boolean hasSelSF = true;
            if (selSF < 0 || selSF >= jList_flows.getModel().getSize())
                hasSelSF = false;
            
            switch (sfType) {
                case PLACE_SEMIFLOWS:
                case PLACE_FLOWS:
                case PLACE_BASIS:
                case TRAPS: 
                case SIPHONS:
                    if (!hasSelSF)
                        return Integer.MAX_VALUE;
                    if (elem instanceof Place)
                        return algo.getFlowVector(selSF)[netIndex.place2index.get((Place)elem)];
                    break;
                    
                case TRANSITION_SEMIFLOWS:
                case TRANSITION_FLOWS:
                case TRANSITION_BASIS:
                    if (!hasSelSF)
                        return Integer.MAX_VALUE;
                    if (elem instanceof Transition)
                        return algo.getFlowVector(selSF)[netIndex.trn2index.get((Transition)elem)];
                    break;
                    
                case PLACE_BOUNDS_FROM_PINV:
                    return Integer.MAX_VALUE; // do not throw
            }
            return Integer.MAX_VALUE;
        }

        @Override
        public int getNodeBound(Selectable elem, boolean lower) {
            if (algo == null)
                return -1;
            if (sfType != Type.PLACE_BOUNDS_FROM_PINV)
                return -1;
            if (!(elem instanceof Place))
                return -1;
            
            if (lower)
                return algo.getLowerBoundOf(netIndex.place2index.get((Place)elem));
            else
                return algo.getUpperBoundOf(netIndex.place2index.get((Place)elem));
        }

        @Override
        public Type getType() {
            return sfType;
        }

        @Override
        public Color getLineColor(int card) {
            switch (sfType) {
                case PLACE_SEMIFLOWS:
                case PLACE_FLOWS:
                case PLACE_BASIS:
                    return (card > 0 ? POSITIVE_PLACE_LINE_CLR : NEGATIVE_PLACE_LINE_CLR);
                case TRANSITION_SEMIFLOWS:
                case TRANSITION_FLOWS:
                case TRANSITION_BASIS:
                    return (card > 0 ? POSITIVE_TRANSITION_LINE_CLR : NEGATIVE_TRANSITION_LINE_CLR);
                case TRAPS:
                    return TRAP_PLACE_LINE_CLR;
                case SIPHONS:
                    return SIPHON_PLACE_LINE_CLR;
                case PLACE_BOUNDS_FROM_PINV:
                    return PLACE_BOUND_LINE_CLR;
            }
            return null;
        }

        @Override
        public Color getTextColor(int card) {
            switch (sfType) {
                case PLACE_SEMIFLOWS:
                case PLACE_FLOWS:
                case PLACE_BASIS:
                    return (card > 0 ? POSITIVE_PLACE_TEXT_CLR : NEGATIVE_PLACE_TEXT_CLR);
                case TRANSITION_SEMIFLOWS:
                case TRANSITION_FLOWS:
                case TRANSITION_BASIS:
                    return (card > 0 ? POSITIVE_TRANSITION_TEXT_CLR : NEGATIVE_TRANSITION_TEXT_CLR);
                case TRAPS:
                    return TRAP_PLACE_TEXT_CLR;
                case SIPHONS:
                    return SIPHON_PLACE_TEXT_CLR;
                case PLACE_BOUNDS_FROM_PINV:
                    return PLACE_BOUND_TEXT_CLR;
            }
            return null;
        }

        @Override
        public Color getBorderColor(Node node) {
            if (selectedNode != node)
                return null;
            
            switch (sfType) {
                case PLACE_SEMIFLOWS:
                case PLACE_FLOWS:
                case PLACE_BASIS:
                case PLACE_BOUNDS_FROM_PINV:
                case TRAPS:
                case SIPHONS:
                    return Color.RED;
                case TRANSITION_SEMIFLOWS:
                case TRANSITION_FLOWS:
                case TRANSITION_BASIS:
                    return Color.BLUE;
            }
            return null;
        }
        
        @Override
        public int getDashPhase() {
            return dashPhase;
        }
    }
    private final SelectedFlow selectedFlow = new SelectedFlow();

    private class SemiflowViewerPanel extends NetViewerPanel {
        private Point2D mousePt = new Point2D.Double();

        public SemiflowViewerPanel(JScrollPane scrollPane) {
            super(scrollPane);
        }

        @Override
        public void netClicked(JNetPanel panel, NetPage net, MouseEvent evt) {
            int zoom = viewerPanel.getZoomLevel();
            mousePt.setLocation(NetObject.screenToLogic(evt.getX(), zoom) + panel.pageBounds.getX(),
                                 NetObject.screenToLogic(evt.getY(), zoom) + panel.pageBounds.getY());
            // Search for a place that has been clicked, and activate its line
            // in the flows panel (which shows the bound of the clicked place)
            Rectangle2D hitRect = NetObject.makeHitRectangle(mousePt, zoom);
            for (Node n : net.nodes) {
                if (sfType.isPlace() && n instanceof Place && 
                        n.intersectRectangle(hitRect, net.viewProfile, true))
                {
                    if (sfType == PTFlows.Type.PLACE_BOUNDS_FROM_PINV) {
                        jList_flows.setSelectedIndex(netIndex.place2index.get((Place)n));
                    }
                    else {
                        selectedNode = n;
                        updateListPanel(false, jList_flows.getSelectedIndex());
                        repaint();    
                    }
                    return;
                }
                else if (sfType.isTransition()&& n instanceof Transition && 
                        n.intersectRectangle(hitRect, net.viewProfile, true))
                {
                    selectedNode = n;
                    updateListPanel(false, jList_flows.getSelectedIndex());
                    repaint();
                    return;
                }
            }
            
            if (selectedNode != null) {
                selectedNode = null;
                updateListPanel(false, jList_flows.getSelectedIndex());
                repaint();
            }
        }

        @Override
        public String getOverlayMessage(NetPage net) {
            return null;
        }

        @Override
        public void setupDrawContext(NetPage net, DrawHelper dh) {
//            if (algo == null || !algo.isComputed())
//                return;
//            if (jList_flows.getModel().getSize() == 0)
//                return;

            dh.selectedPTFlow = selectedFlow;
        }

        @Override
        public void allBindingsOk(boolean allOk) {
            mainInterface.invalidateGUI();
        }

        @Override
        public Color getNetBackground(String overlayMsg) {
            if (isEnabled() && !isInBindingPhase() && hasSemiflows)
                return editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR;
            return editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_DISABLED_COLOR;
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

        jPanelNet = new javax.swing.JPanel();
        jScrollPaneNet = new javax.swing.JScrollPane();
        resourceFactory = new editor.gui.ResourceFactory();
        toolbar_basic = new javax.swing.JToolBar();
        jToolbarButtonChangeBindings = new common.JToolbarButton();
        jToolbarButtonComputePlaceSemiflows = new common.JToolbarButton();
        jToolbarButtonComputeTransitionSemiflows = new common.JToolbarButton();
        jToolbarButtonComputeBoundsFromPinv = new common.JToolbarButton();
        jToolbarButtonShowMatrices = new common.JToolbarButton();
        toolbar_advanced = new javax.swing.JToolBar();
        jToolbarButtonComputePlaceFlows = new common.JToolbarButton();
        jToolbarButtonComputeTransitionFlows = new common.JToolbarButton();
        jToolbarButtonComputePlaceBasis = new common.JToolbarButton();
        jToolbarButtonComputeTransitionBasis = new common.JToolbarButton();
        jToolbarButtonComputeTraps = new common.JToolbarButton();
        jToolbarButtonComputeSiphons = new common.JToolbarButton();
        jPanelBinding = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();
        jPanelControls = new javax.swing.JPanel();
        jLabel_computedFlows = new javax.swing.JLabel();
        jLabel_Binding = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jList_flows = new javax.swing.JList<>();

        jPanelNet.setLayout(new javax.swing.BoxLayout(jPanelNet, javax.swing.BoxLayout.LINE_AXIS));
        jPanelNet.add(jScrollPaneNet);

        toolbar_basic.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(41, 43, 45)), "Basic", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.BOTTOM));
        toolbar_basic.setFloatable(false);
        toolbar_basic.setRollover(true);
        toolbar_basic.add(jToolbarButtonChangeBindings);
        toolbar_basic.add(jToolbarButtonComputePlaceSemiflows);
        toolbar_basic.add(jToolbarButtonComputeTransitionSemiflows);
        toolbar_basic.add(jToolbarButtonComputeBoundsFromPinv);
        toolbar_basic.add(jToolbarButtonShowMatrices);

        toolbar_advanced.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(41, 43, 45)), "Advanced", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.BOTTOM));
        toolbar_advanced.setFloatable(false);
        toolbar_advanced.setRollover(true);

        jToolbarButtonComputePlaceFlows.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonComputePlaceFlows.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar_advanced.add(jToolbarButtonComputePlaceFlows);

        jToolbarButtonComputeTransitionFlows.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonComputeTransitionFlows.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar_advanced.add(jToolbarButtonComputeTransitionFlows);

        jToolbarButtonComputePlaceBasis.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonComputePlaceBasis.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar_advanced.add(jToolbarButtonComputePlaceBasis);

        jToolbarButtonComputeTransitionBasis.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonComputeTransitionBasis.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar_advanced.add(jToolbarButtonComputeTransitionBasis);

        jToolbarButtonComputeTraps.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonComputeTraps.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar_advanced.add(jToolbarButtonComputeTraps);

        jToolbarButtonComputeSiphons.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonComputeSiphons.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        toolbar_advanced.add(jToolbarButtonComputeSiphons);

        setLayout(new java.awt.CardLayout());

        jPanelBinding.setLayout(new java.awt.GridBagLayout());

        jLabel3.setText("<html>\n<b>Bind model parameters:</b><br/>\nAssign a value to all the model parameters before starting the token game.");
        jLabel3.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        jLabel3.setVerticalTextPosition(javax.swing.SwingConstants.TOP);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.PAGE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(10, 10, 10, 10);
        jPanelBinding.add(jLabel3, gridBagConstraints);

        jLabel5.setText("<html>Parameters assigned correctly have this icon.");
        jLabel5.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        jLabel5.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        jLabel5.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(10, 10, 10, 10);
        jPanelBinding.add(jLabel5, gridBagConstraints);

        jLabel6.setText("<html>Parameters that are still unassigned or with an incorrectly assigned value have this icon.");
        jLabel6.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        jLabel6.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        jLabel6.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(10, 10, 10, 10);
        jPanelBinding.add(jLabel6, gridBagConstraints);

        jLabel4.setText("<html>When all parameters have been assigned correctly, start the computation of the structural properties by pressing the corresponding button on the application toolbar.");
        jLabel4.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        jLabel4.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        jLabel4.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(10, 10, 10, 10);
        jPanelBinding.add(jLabel4, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        jPanelBinding.add(jPanel2, gridBagConstraints);

        jLabel7.setIcon(resourceFactory.getOk16());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.FIRST_LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(14, 10, 0, 0);
        jPanelBinding.add(jLabel7, gridBagConstraints);

        jLabel8.setIcon(resourceFactory.getExclamation16());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.FIRST_LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(14, 10, 0, 0);
        jPanelBinding.add(jLabel8, gridBagConstraints);

        add(jPanelBinding, "binding");

        jPanelControls.setLayout(new java.awt.GridBagLayout());

        jLabel_computedFlows.setText("Computed flows:");
        jLabel_computedFlows.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 1, 1);
        jPanelControls.add(jLabel_computedFlows, gridBagConstraints);

        jLabel_Binding.setIcon(resourceFactory.getChangeBindings32());
        jLabel_Binding.setText("Binding template parameters...");
        jLabel_Binding.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 1, 1);
        jPanelControls.add(jLabel_Binding, gridBagConstraints);

        jPanel1.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        jPanel1.setLayout(new java.awt.GridLayout(1, 0));

        jScrollPane1.setViewportView(jList_flows);

        jPanel1.add(jScrollPane1);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 9;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        jPanelControls.add(jPanel1, gridBagConstraints);

        add(jPanelControls, "controls");
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel_Binding;
    private javax.swing.JLabel jLabel_computedFlows;
    private javax.swing.JList<String> jList_flows;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanelBinding;
    private javax.swing.JPanel jPanelControls;
    private javax.swing.JPanel jPanelNet;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPaneNet;
    private common.JToolbarButton jToolbarButtonChangeBindings;
    private common.JToolbarButton jToolbarButtonComputeBoundsFromPinv;
    private common.JToolbarButton jToolbarButtonComputePlaceBasis;
    private common.JToolbarButton jToolbarButtonComputePlaceFlows;
    private common.JToolbarButton jToolbarButtonComputePlaceSemiflows;
    private common.JToolbarButton jToolbarButtonComputeSiphons;
    private common.JToolbarButton jToolbarButtonComputeTransitionBasis;
    private common.JToolbarButton jToolbarButtonComputeTransitionFlows;
    private common.JToolbarButton jToolbarButtonComputeTransitionSemiflows;
    private common.JToolbarButton jToolbarButtonComputeTraps;
    private common.JToolbarButton jToolbarButtonShowMatrices;
    private editor.gui.ResourceFactory resourceFactory;
    private javax.swing.JToolBar toolbar_advanced;
    private javax.swing.JToolBar toolbar_basic;
    // End of variables declaration//GEN-END:variables
}
