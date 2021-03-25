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
import editor.domain.grammar.ParserContext;
import editor.domain.semiflows.SemiFlows;
import editor.domain.semiflows.MartinezSilvaAlgorithm;
import editor.domain.composition.ComposableNet;
import editor.gui.AbstractPageEditor;
import editor.gui.MainWindowInterface;
import editor.gui.SharedResourceProvider;
import static editor.gui.SharedResourceProvider.ActionName.COMPUTE_PLACE_SEMIFLOWS;
import java.awt.CardLayout;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import javax.swing.DefaultListModel;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

/**
 *
 * @author elvio
 */
public class NetSemiflowsPanel extends javax.swing.JPanel implements AbstractPageEditor {
    
    SemiFlows.Type sfType;
    boolean hasSemiflows = false;

    // The GSPN viewer
    NetViewerPanel viewerPanel;
    
    // Original GSPN reference
    GspnPage origGspn;
    
    MainWindowInterface mainInterface;
    
    // Place/Transition -> 0-based index
    Map<Place, Integer> place2index;
    Map<Transition, Integer> trn2index;
    ArrayList<Place> places;
    ArrayList<Transition> transitions;
    
    // The list of computed semiflows
    MartinezSilvaAlgorithm algo;
    PageErrorWarning[] algoErrsWarns;
    
    // Highlighted places, transitions and edges
    Set<Selectable> highlightedElems;
    private Timer showTimer = null;
    private int dashPhase = 0;
    
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
                (SharedResourceProvider.ActionName.COMPUTE_PLACE_SEMIFLOWS));
        jToolbarButtonComputeTransitionSemiflows.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.COMPUTE_TRANS_SEMIFLOWS));
        jToolbarButtonComputeBoundsFromPinv.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.COMPUTE_PLACE_BOUNDS_FROM_PINV));
        jToolbarButtonShowMatrices.setAction(shActProv.getSharedAction
                (SharedResourceProvider.ActionName.SHOW_NET_MATRICES));
        
        jToolbarButtonChangeBindings.setHideActionText(false);
        
        jListSemiflows.getSelectionModel().addListSelectionListener(new ListSelectionListener() {
            @Override
            public void valueChanged(ListSelectionEvent lse) {
                highlightSemiflow(jListSemiflows.getSelectedIndex());
                viewerPanel.repaint();
            }
        });
        
        showTimer = new Timer(100, new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent ae) {
//                System.out.println("timer!");
                dashPhase++;
                viewerPanel.repaint();
            }
        });
        showTimer.setDelay(300);
        showTimer.stop();
    }
    
    public void setSemiflowType(SemiFlows.Type sfType) {
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
        
        place2index = new HashMap<>();
        trn2index = new HashMap<>();
        places = new ArrayList<>();
        transitions = new ArrayList<>();
        
        // Prepare place/transition indexes for semiflow construction.
        for (Node node : gspn.nodes) {
            if (node instanceof Place) {
                Place place = (Place)node;
                place2index.put(place, place2index.size());
                places.add(place);
            }
            else if (node instanceof Transition) {
                Transition trn = (Transition)node;
                trn2index.put(trn, trn2index.size());
                transitions.add(trn);
            }
        }
        
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
        place2index = null;
        trn2index = null;
        places = null;
        transitions = null;
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
                    
                case COMPUTE_PLACE_SEMIFLOWS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != SemiFlows.Type.PLACE_SEMIFLOW));
                    break;
                    
                case COMPUTE_TRANS_SEMIFLOWS:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != SemiFlows.Type.TRANSITION_SEMIFLOWS));
                    break;
                    
                case COMPUTE_PLACE_BOUNDS_FROM_PINV:
                    act.setEnabled((viewerPanel.isInBindingPhase() && viewerPanel.areAllBindingsOk()) ||
                                   (!viewerPanel.isInBindingPhase() && sfType != SemiFlows.Type.PLACE_BOUNDS_FROM_PINV));
                    break;
                    
                case SHOW_NET_MATRICES:
                    act.setEnabled(!viewerPanel.isInBindingPhase() &&
                                   (sfType != SemiFlows.Type.PLACE_BOUNDS_FROM_PINV));
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
                
            case COMPUTE_PLACE_SEMIFLOWS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeSemiflowsLater(SemiFlows.Type.PLACE_SEMIFLOW);
                break;
                
            case COMPUTE_TRANS_SEMIFLOWS:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeSemiflowsLater(SemiFlows.Type.TRANSITION_SEMIFLOWS);
                break;
                
            case COMPUTE_PLACE_BOUNDS_FROM_PINV:
                if (viewerPanel.isInBindingPhase())
                    closeBindingForm();
                recomputeSemiflowsLater(SemiFlows.Type.PLACE_BOUNDS_FROM_PINV);
                break;
                
            case SHOW_NET_MATRICES:
                showSemiflowsMatrix();
                break;
            
            default:
                throw new UnsupportedOperationException();
        }
    }
    
    public void recomputeSemiflowsLater(final SemiFlows.Type type) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                jListSemiflows.removeAll();
                recomputeSemiflows(type);
            }
        });
    }
    
    private void recomputeSemiflows(SemiFlows.Type type) {
        sfType = type;
        
        // Both P-semiflows and Place bounds are initialized in the same way
        boolean initializeForPsemiflows = (sfType == SemiFlows.Type.PLACE_SEMIFLOW ||
                                           sfType == SemiFlows.Type.PLACE_BOUNDS_FROM_PINV);
        boolean initializeForBounds = (sfType == SemiFlows.Type.PLACE_BOUNDS_FROM_PINV);
        
        if (initializeForPsemiflows)
            algo = new MartinezSilvaAlgorithm(places.size(), transitions.size());
        else
            algo = new MartinezSilvaAlgorithm(transitions.size(), places.size());
        
        // Compute the semiflows
        ParserContext context = new ParserContext(viewerPanel.getGspn());
        context.templateVarsBinding = viewerPanel.getGspnBinding();
        context.bindingContext = context;
        boolean hasInhibitorEdges = false;
        boolean hasMarkDepEdges = false;
        boolean hasEdgeWithZeroCard = false;
        boolean hasContinuousEdges = false;
        boolean hasColoredPlaces = false;
        boolean hasNonIntegerInitMarks = false;
        for (Edge edge : viewerPanel.getGspn().edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge ge = (GspnEdge)edge;
                int p, t, card;
                
                // Ignore inhibitor edges, but signal their presence.
                if (ge.getEdgeKind() == GspnEdge.Kind.INHIBITOR) {
                    hasInhibitorEdges = true;
                    continue;
                }
                // Ignore flow edges, but signal their presence.
                if (ge.isContinuous() || ge.isFiringFlow()) {
                    hasContinuousEdges = true;
                    continue;
                }
                // Ignore colored edges
                if (!ge.getColorDomainOfConnectedPlace().isNeutralDomain()) {
                    hasColoredPlaces = true;
                    continue;
                }
                switch (ge.getEdgeKind()) {
                    case INPUT:
//                        System.out.println("head="+ge.getHeadNode().getUniqueName()+" tail="+ge.getTailNode().getUniqueName());
                        p = place2index.get((Place)ge.getTailNode());
                        t = trn2index.get((Transition)ge.getHeadNode());
                        try {
                            card = -ge.evaluateMultiplicity(context, null, null).getScalarInt();
                        }
                        catch (Exception e) {
                            hasMarkDepEdges = true;
                            card = 1;
                        }
                        break;
                        
                    case OUTPUT:
                        p = place2index.get((Place)ge.getHeadNode());
                        t = trn2index.get((Transition)ge.getTailNode());
                        try {
                            card = ge.evaluateMultiplicity(context, null, null).getScalarInt();
                        }
                        catch (Exception e) {
                            hasMarkDepEdges = true;
                            card = 1;
                        }
                        break;
                        
                    default:
                        throw new IllegalStateException();
                }
                if (card == 0)
                    hasEdgeWithZeroCard = true;
                if (initializeForPsemiflows)
                    algo.addFlow(p, t, card);
                else
                    algo.addFlow(t, p, card);
            }            
        }
        
        if (sfType == SemiFlows.Type.PLACE_BOUNDS_FROM_PINV || sfType == SemiFlows.Type.PLACE_SEMIFLOW) {
            // Prepare also bound computation
            for (Node node : viewerPanel.getGspn().nodes) {
                if (node instanceof Place) {
                    Place plc = (Place)node;
                    int m0;
                    try {
                        m0 = plc.evaluateInitMarking(context).getScalarInt();
                    }
                    catch (Exception e) {
                        hasNonIntegerInitMarks = true;
                        m0 = 0;
                    }
                    algo.setInitQuantity(place2index.get(plc), m0);
                }
            }
        }
        
        // Compute semiflows
        SemiflowComputationDialog dlg = new SemiflowComputationDialog(mainInterface.getWindowFrame(), algo);
        dlg.showDialogAndStart();
//        algo.compute(false /*log*/);
        
        // Update window
        DefaultListModel<String> model = new DefaultListModel<>();
        if (initializeForBounds) {
            for (int p=0; p<places.size(); p++) {
                StringBuilder name = new StringBuilder();
                name.append("Bound of ").append(places.get(p).getUniqueName())
                    .append(": [").append(algo.getLowerBoundOf(p)).append(", ");
                if (algo.getUpperBoundOf(p) == Integer.MAX_VALUE)
                    name.append(SemiFlows.INFINITY_UNICODE);
                else
                    name.append(algo.getUpperBoundOf(p));
                name.append("]");
                model.addElement(name.toString());
            }
        }
        else {
            final int numSemiflows = (algo.isComputed() ? algo.numSemiflows() : 0);
            for (int i=0; i<numSemiflows; i++) {
                int[] allSF = algo.getSemiflow(i);
                StringBuilder name = new StringBuilder();
                for (int k=0; k<allSF.length; k++) {
                    if (allSF[k] == 0)
                        continue;
                    if (name.length() > 0)
                        name.append(" + ");
                    if (allSF[k] != 1)
                        name.append(allSF[k]).append("*");
                    if (initializeForPsemiflows)
                        name.append(places.get(k).getUniqueName());
                    else
                        name.append(transitions.get(k).getUniqueName());
                }
                model.addElement(name.toString());
            }
        }
        jListSemiflows.setModel(model);
        if (model.size() > 0)
            jListSemiflows.setSelectedIndex(0);
        highlightSemiflow(jListSemiflows.getSelectedIndex());

        // Update titles and warnings
        jLabel_PSemiflow.setVisible(sfType == SemiFlows.Type.PLACE_SEMIFLOW);
        jLabel_TSemiflow.setVisible(sfType == SemiFlows.Type.TRANSITION_SEMIFLOWS);
        jLabel_PlaceBoundsFromPinv.setVisible(sfType == SemiFlows.Type.PLACE_BOUNDS_FROM_PINV);
        
        // Make the list of errors
        ArrayList<PageErrorWarning> errsWarns = new ArrayList<>();
        if (!algo.isComputed())
            errsWarns.add(PageErrorWarning.newWarning("Computation interrupted.", 
                          new Selectable.DummySelectable()));
        else {
            if (hasInhibitorEdges)
                errsWarns.add(PageErrorWarning.newWarning("Inhibitor edges have no impact on this "
                              + "analysis, and are ignored.", new Selectable.DummySelectable()));
            if (hasMarkDepEdges)
                errsWarns.add(PageErrorWarning.newWarning("Marking-dependent edges are not supported. "
                                + "Using cardinality 1 for them.", new Selectable.DummySelectable()));
            if (hasEdgeWithZeroCard)
                errsWarns.add(PageErrorWarning.newWarning("The net has edges with cardinality 0.", 
                                                          new Selectable.DummySelectable()));
            if (hasContinuousEdges)
                errsWarns.add(PageErrorWarning.newWarning("Continuous transition/places/edges are ignored.", 
                              new Selectable.DummySelectable()));
            if (hasColoredPlaces)
                errsWarns.add(PageErrorWarning.newWarning("Colored places are not supported and are ignored.",
                              new Selectable.DummySelectable()));
            
            if (hasNonIntegerInitMarks)
                errsWarns.add(PageErrorWarning.newWarning("Place bounds can only be computed if all places "
                        + "have a scalar integer initial marking.",
                              new Selectable.DummySelectable()));
                
            // Determine if all places/transitions are covered by P/T invariants
            int numPT = (initializeForPsemiflows) ? 
                         places.size() : transitions.size();
            boolean[] covered = new boolean[numPT];
            Arrays.fill(covered, false);
            for (int sf=0; sf<algo.numSemiflows(); sf++) {
                int[] flow = algo.getSemiflow(sf);
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
                if (initializeForPsemiflows) {
                    err += ((numUncover==1) ? " place is " : " places are ")
                            +"not covered by P-semiflows: ";
                    int j = numUncover;
                    for (int i=0; i<numPT; i++)
                        if (!covered[i])
                            err += places.get(i).getUniqueName() + (--j>0?", ":"");
                    if (initializeForBounds) 
                        err += (numUncover==1) ? ". No finite bound is available for this place" : 
                                    ". No finite bounds are available for these places.";
                }
                else { // Transitions
                    err += ((numUncover==1) ? " transition is " : " transitions are ")
                            +"not covered by T-semiflows: ";
                    for (int i=0; i<numPT; i++)
                        if (!covered[i])
                            err += transitions.get(i).getUniqueName() + (--numUncover>0?", ":"");                    
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
        viewerPanel.reopenBindingForms();
        ((CardLayout)getLayout()).show(this, BINDING_CARD);
        jLabel_Binding.setVisible(true);
        jLabel_PSemiflow.setVisible(false);
        jLabel_TSemiflow.setVisible(false);
        jLabel_PlaceBoundsFromPinv.setVisible(false);
        jListSemiflows.setModel(new DefaultListModel<String>());
        algo = null;
        algoErrsWarns = NO_ERRORS;
        highlightedElems = null;
        showTimer.stop();

        if (!viewerPanel.hasBindingForms() || !reopenBindings) {
            closeBindingForm();
            recomputeSemiflowsLater(sfType);
        }
        else {
            hasSemiflows = false;
            mainInterface.invalidateGUI();
            viewerPanel.repaint();
        }
    }
    
    private void highlightSemiflow(int sfNum) {
        if (sfNum < 0 || sfNum >= jListSemiflows.getModel().getSize()) {
            highlightedElems = null;
            return;
        }
        highlightedElems = new HashSet<>();
        GspnPage gspn = viewerPanel.getGspn();
        
        switch (sfType) {
            case PLACE_SEMIFLOW: {
                int[] placeSF = algo.getSemiflow(sfNum);
                // [1] Add all places of the semiflow into the list
                for (int k=0; k<placeSF.length; k++)
                    if (placeSF[k] != 0)
                        highlightedElems.add(places.get(k));
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
            break;
                
            case TRANSITION_SEMIFLOWS: {
                int[] trnSF = algo.getSemiflow(sfNum);
                // [1] Add all transitions of the semiflow into the list
                for (int k=0; k<trnSF.length; k++)
                    if (trnSF[k] != 0)
                        highlightedElems.add(transitions.get(k));
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
            break;
                
            case PLACE_BOUNDS_FROM_PINV: {
                // highlight place number sfNum
                highlightedElems.add(places.get(sfNum));
            }
            break;
        }
        
        // Highlight edges between place/transitions, when visualizing semiflows
        if (sfType == SemiFlows.Type.PLACE_SEMIFLOW || sfType == SemiFlows.Type.TRANSITION_SEMIFLOWS) {
            // [3] Add all input/output edges between selected places and transitions
            for (Edge edge : gspn.edges)
                if (edge instanceof GspnEdge && 
                    ((GspnEdge)edge).getEdgeKind() != GspnEdge.Kind.INHIBITOR &&
                    highlightedElems.contains(edge.getHeadNode()) && 
                    highlightedElems.contains(edge.getTailNode()))
                    highlightedElems.add(edge);
        }
    }
    
    private void showSemiflowsMatrix() {
        ShowSemiflowsMatrixDialog dlg = new ShowSemiflowsMatrixDialog(mainInterface.getWindowFrame(), true, 
                                                                      algo, sfType, places, transitions);
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
    public JComponent getToolbar() {
        return jToolBar;
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
    
    private static final Color PLACE_SEMIFLOW_LINE_CLR = new Color(90, 180, 240);
    private static final Color TRANSITION_SEMIFLOW_LINE_CLR = new Color(240, 130, 70);
    private static final Color PLACE_BOUND_LINE_CLR = new Color(0, 210, 0);
    private static final Color PLACE_SEMIFLOW_TEXT_CLR = new Color(40, 50, 180);
    private static final Color TRANSITION_SEMIFLOW_TEXT_CLR = new Color(190, 40, 20);
    private static final Color PLACE_BOUND_TEXT_CLR = new Color(0, 160, 0);

    
    private class SelectedSemiflow implements SemiFlows {

        @Override
        public boolean contains(Selectable elem) {
            if (algo == null || highlightedElems == null)
                return false;
            return highlightedElems.contains(elem);
        }

        @Override
        public int getNodeCardinality(Selectable elem) {
            if (algo == null)
                return -1;
            int selSF = jListSemiflows.getSelectedIndex();
            boolean hasSelSF = true;
            if (selSF < 0 || selSF >= jListSemiflows.getModel().getSize())
                hasSelSF = false;
            
            switch (sfType) {
                case PLACE_SEMIFLOW:
                    if (!hasSelSF)
                        return -1;
                    if (elem instanceof Place)
                        return algo.getSemiflow(selSF)[place2index.get((Place)elem)];
                    break;
                    
                case TRANSITION_SEMIFLOWS:
                    if (!hasSelSF)
                        return -1;
                    if (elem instanceof Transition)
                        return algo.getSemiflow(selSF)[trn2index.get((Transition)elem)];
                    break;
                    
                case PLACE_BOUNDS_FROM_PINV:
                    if (elem instanceof Place)
                        return algo.getUpperBoundOf(place2index.get((Place)elem));
                    break;
            }
            return -1;
        }

        @Override
        public int getNodeLowerBound(Selectable elem) {
            if (algo == null)
                return -1;
            
            if (sfType == Type.PLACE_BOUNDS_FROM_PINV) {
                if (elem instanceof Place)
                    return algo.getLowerBoundOf(place2index.get((Place)elem));
            }
            return -1;
        }

        @Override
        public Type getType() {
            return sfType;
        }

        @Override
        public Color getLineColor() {
            switch (sfType) {
                case PLACE_SEMIFLOW:
                    return PLACE_SEMIFLOW_LINE_CLR;
                case TRANSITION_SEMIFLOWS:
                    return TRANSITION_SEMIFLOW_LINE_CLR;
                case PLACE_BOUNDS_FROM_PINV:
                    return PLACE_BOUND_LINE_CLR;
            }
            return null;
        }

        @Override
        public Color getTextColor() {
            switch (sfType) {
                case PLACE_SEMIFLOW:
                    return PLACE_SEMIFLOW_TEXT_CLR;
                case TRANSITION_SEMIFLOWS:
                    return TRANSITION_SEMIFLOW_TEXT_CLR;
                case PLACE_BOUNDS_FROM_PINV:
                    return PLACE_BOUND_TEXT_CLR;
            }
            return null;
        }
        
        @Override
        public int getDashPhase() {
            return dashPhase;
        }
    }
    private final SelectedSemiflow selectedSemiflow = new SelectedSemiflow();

    private class SemiflowViewerPanel extends NetViewerPanel {
        private Point2D mousePt = new Point2D.Double();

        public SemiflowViewerPanel(JScrollPane scrollPane) {
            super(scrollPane);
        }

        @Override
        public void netClicked(JNetPanel panel, NetPage net, MouseEvent evt) {
            if (sfType == SemiFlows.Type.PLACE_BOUNDS_FROM_PINV) {
                int zoom = viewerPanel.getZoomLevel();
                mousePt.setLocation(NetObject.screenToLogic(evt.getX(), zoom) + panel.pageBounds.getX(),
                                     NetObject.screenToLogic(evt.getY(), zoom) + panel.pageBounds.getY());
                // Search for a place that has been clicked, and activate its line
                // in the semiflows panel (which shows the bound of the clicked place)
                Rectangle2D hitRect = NetObject.makeHitRectangle(mousePt, zoom);
                for (Node n : net.nodes) {
                    if (n instanceof Place && n.intersectRectangle(hitRect, net.viewProfile, true))
                    {
                        jListSemiflows.setSelectedIndex(place2index.get((Place)n));
                        return;
                    }
                }
            }
        }

        @Override
        public String getOverlayMessage(NetPage net) {
            return null;
        }

        @Override
        public void setupDrawContext(NetPage net, DrawHelper dh) {
            if (algo == null || !algo.isComputed())
                return;
            if (jListSemiflows.getModel().getSize() == 0)
                return;

            dh.semiflows = selectedSemiflow;
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

        jToolBar = new javax.swing.JToolBar();
        jToolbarButtonChangeBindings = new common.JToolbarButton();
        jToolbarButtonComputePlaceSemiflows = new common.JToolbarButton();
        jToolbarButtonComputeTransitionSemiflows = new common.JToolbarButton();
        jToolbarButtonComputeBoundsFromPinv = new common.JToolbarButton();
        jToolbarButtonShowMatrices = new common.JToolbarButton();
        jPanelNet = new javax.swing.JPanel();
        jScrollPaneNet = new javax.swing.JScrollPane();
        resourceFactory = new editor.gui.ResourceFactory();
        jPanelBinding = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();
        jPanelControls = new javax.swing.JPanel();
        jLabel_PSemiflow = new javax.swing.JLabel();
        jLabel_TSemiflow = new javax.swing.JLabel();
        jLabel_PlaceBoundsFromPinv = new javax.swing.JLabel();
        jLabel_Binding = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jListSemiflows = new javax.swing.JList<>();

        jToolBar.setRollover(true);

        jToolbarButtonChangeBindings.setText("jToolbarButton3");
        jToolBar.add(jToolbarButtonChangeBindings);

        jToolbarButtonComputePlaceSemiflows.setText("jToolbarButton1");
        jToolBar.add(jToolbarButtonComputePlaceSemiflows);

        jToolbarButtonComputeTransitionSemiflows.setText("jToolbarButton2");
        jToolBar.add(jToolbarButtonComputeTransitionSemiflows);

        jToolbarButtonComputeBoundsFromPinv.setText("jToolbarButton1");
        jToolbarButtonComputeBoundsFromPinv.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonComputeBoundsFromPinv.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToolbarButtonComputeBoundsFromPinv);

        jToolbarButtonShowMatrices.setText("jToolbarButton1");
        jToolbarButtonShowMatrices.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonShowMatrices.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBar.add(jToolbarButtonShowMatrices);

        jPanelNet.setLayout(new javax.swing.BoxLayout(jPanelNet, javax.swing.BoxLayout.LINE_AXIS));
        jPanelNet.add(jScrollPaneNet);

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

        jLabel4.setText("<html>When all parameters have been assigned correctly, start the computation of the minimal P or T semiflows by pressing the corresponding button on the application toolbar.");
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

        jLabel_PSemiflow.setIcon(resourceFactory.getPinv32());
        jLabel_PSemiflow.setText("P-semiflows:");
        jLabel_PSemiflow.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 1, 1);
        jPanelControls.add(jLabel_PSemiflow, gridBagConstraints);

        jLabel_TSemiflow.setIcon(resourceFactory.getTinv32());
        jLabel_TSemiflow.setText("T-semiflows:");
        jLabel_TSemiflow.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 1, 1);
        jPanelControls.add(jLabel_TSemiflow, gridBagConstraints);

        jLabel_PlaceBoundsFromPinv.setIcon(resourceFactory.getBound32());
        jLabel_PlaceBoundsFromPinv.setText("Place bounds from P-invariants:");
        jLabel_PlaceBoundsFromPinv.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 1, 1);
        jPanelControls.add(jLabel_PlaceBoundsFromPinv, gridBagConstraints);

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

        jScrollPane1.setViewportView(jListSemiflows);

        jPanel1.add(jScrollPane1);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 6;
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
    private javax.swing.JLabel jLabel_PSemiflow;
    private javax.swing.JLabel jLabel_PlaceBoundsFromPinv;
    private javax.swing.JLabel jLabel_TSemiflow;
    private javax.swing.JList<String> jListSemiflows;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanelBinding;
    private javax.swing.JPanel jPanelControls;
    private javax.swing.JPanel jPanelNet;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPaneNet;
    private javax.swing.JToolBar jToolBar;
    private common.JToolbarButton jToolbarButtonChangeBindings;
    private common.JToolbarButton jToolbarButtonComputeBoundsFromPinv;
    private common.JToolbarButton jToolbarButtonComputePlaceSemiflows;
    private common.JToolbarButton jToolbarButtonComputeTransitionSemiflows;
    private common.JToolbarButton jToolbarButtonShowMatrices;
    private editor.gui.ResourceFactory resourceFactory;
    // End of variables declaration//GEN-END:variables
}
