/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.Tuple;
import editor.gui.SharedResourceProvider;
import common.Util;
import editor.domain.DrawHelper;
import editor.domain.elements.DtaEdge;
import editor.domain.elements.DtaPage;
import editor.domain.Edge;
import editor.domain.elements.GspnPage;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import static editor.domain.ProjectPage.NO_ERRORS;
import editor.gui.AbstractPageEditor;
import editor.gui.MainWindowInterface;
import java.awt.Color;
import java.awt.Component;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.Timer;
import static editor.domain.play.GspnDtaPlayEngine.*;
import editor.domain.elements.Transition;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.play.ActivityState;
import editor.domain.play.Firable;
import editor.domain.play.FirableWithBindings;
import editor.domain.play.JointFiring;
import editor.domain.play.JointState;
import editor.domain.play.TimeElapse;
import editor.domain.values.Bound;
import editor.gui.ResourceFactory;
import java.awt.CardLayout;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Locale;
import java.util.Random;
import javax.swing.DefaultListCellRenderer;
import javax.swing.DefaultListModel;
import javax.swing.Icon;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

/**
 *
 * @author elvio
 */
public class NetPlayPanel extends javax.swing.JPanel implements AbstractPageEditor {

    private MainWindowInterface mainInterface;
    
    // Original GSPN/DTA passed during initialization, not used during the simulation
    private GspnPage origGspn;
    private DtaPage origDta;
    
    // Viewer with the GSPN and the DTA
    private NetViewerPanel viewerPanel;
    
    // State of the simulation
    private ArrayList<JointState> stateHistory = null;
    private int currStateIndex = -1;
    private JointFiring lastFiring = null;
    private Timer firingTimer = null;
    private java.util.Date startAnimTime = null;
    int standAnimPhase = 0;
    Random randomGenerator = new Random();
    
    private boolean isFiring() { return lastFiring != null; }
    
    private JointState currentState() { return stateHistory.get(currStateIndex); }
    
    private static final String CONTROLS_CARD = "controls";
    private static final String BINDING_CARD = "binding";
    
    /**
     * Creates new form NetPlayPanel
     * @param shActProv
     */
    public NetPlayPanel(SharedResourceProvider shActProv) {
        initComponents();
        Util.reformatPanelsForPlatformLookAndFeel(this);
        
        jScrollPaneAllNets.getViewport().setOpaque(true);
        jScrollPaneAllNets.getViewport().setBackground(NetEditorPanel.OUTSIDE_PAGE_COLOR);
        jToolbarButtonSwitchToUntimed.setVisible(false);
        
        // Install actions
        jToolbarButtonNext.setAction(shActProv.getSharedAction(SharedResourceProvider.ActionName.PLAY_NEXT));
        jToolbarButtonPrev.setAction(shActProv.getSharedAction(SharedResourceProvider.ActionName.PLAY_PREVIOUS));
        jToolbarButtonRestart.setAction(shActProv.getSharedAction(SharedResourceProvider.ActionName.PLAY_RESTART));
        jToolbarButtonChangeBindings.setAction(shActProv.getSharedAction(SharedResourceProvider.ActionName.CHANGE_BINDINGS));
        jToolbarButtonChangeBindings.setHideActionText(false);
        
        DefaultListCellRenderer enabTrnRendered = new DefaultListCellRenderer() {
            @Override
            public Component getListCellRendererComponent(JList list, Object value, int index,
                                                          boolean isSelected, boolean cellHasFocus) 
            {
                Component c = super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
                JLabel label = (JLabel) c;
                if (value instanceof FirableWithBindings) {
                    @SuppressWarnings("unchecked")
                    FirableWithBindings<Transition> fwb = (FirableWithBindings<Transition>)value;
                //if (fwb.firable instanceof Transition) {
                    Transition t = (Transition)fwb.firable;
                    String text = t.getUniqueName();
                    if (currentState().timedEventsAreUrgent)
                        text += " [urgent]";
                    else if (currentState().countBindingsWithLimitedTimeSupport(t) > 0)
                        text += " "+currentState().firstTimeOfEvent(t);
                    label.setText(text);
                    Icon ico = IMM_TRN_ICON;
                    if (t.isExponential())
                        ico = EXP_TRN_ICON;
                    else if (t.isGeneral())
                        ico = GEN_TRN_ICON;
                    else if (t.isContinuous())
                        ico = CONT_TRN_ICON;
                    label.setIcon(ico);
                    label.setDisabledIcon(Util.getGrayedIcon(ico));     
                }
                else if (value instanceof DtaEdge) {
                    DtaEdge e = (DtaEdge)value;
                    String text = e.getTailNode().getUniqueName()+" -> "+e.getHeadNode().getUniqueName();
                    if (currentState().boundaryDtaEdgesAreUrgent)
                        text += " [urgent]";
                    else if (currentState().isTangible() && currentState().countBindingsWithLimitedTimeSupport(e) > 0)
                        text += " "+currentState().firstTimeOfEvent(e);
                    label.setText(text);
                    Icon ico = INNER_EDGE_ICON;
                    if (e.isBoundary())
                        ico = BOUNDARY_EDGE_ICON;
                    label.setIcon(ico);
                    label.setDisabledIcon(Util.getGrayedIcon(ico));  
                }
                else if (value instanceof TimeElapse) {
                    label.setText("Let time elapse.");
                    label.setIcon(TIME_ELAPSE_ICON);
                    label.setDisabledIcon(Util.getGrayedIcon(TIME_ELAPSE_ICON));  
                }
                else {
                    label.setText(value.toString());
                    label.setIcon(null);
                    label.setDisabledIcon(null);      
                }
                return c;
            }
        };
        jListEnabledTransitions.setCellRenderer(enabTrnRendered);
        jListEnabledTransitions.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent evt) {
                if (isFiring())
                    return;
                if (evt.getClickCount() == 2) {
                    int index = jListEnabledTransitions.locationToIndex(evt.getPoint());
                    if (index == -1)
                        return;
                    Object obj = (Object)jListEnabledTransitions.getModel().getElementAt(index);
                    if (obj instanceof FirableWithBindings) {
                        @SuppressWarnings("unchecked")
                        FirableWithBindings<Transition> fwb = (FirableWithBindings)obj;
                        assert fwb.bindings.size() >= 1;
                        
                        //startNodeFiring(fwb.firable, (ColorVarsBinding)fwb.bindings.iterator().next());
                        chooseBindingAndFire(fwb, jListEnabledTransitions, evt.getX(), evt.getY());
                    }
                    else if (obj instanceof Firable)
                        startNodeFiring((Firable)obj, null);
                }
            }
        });
        
        normalListFont = jListPathTrace.getFont();
        boldListFont = new Font(normalListFont.getName(), Font.BOLD, normalListFont.getSize());
        DefaultListCellRenderer pathRenderer = new DefaultListCellRenderer() {
            @Override
            public Component getListCellRendererComponent(JList list, Object value, int index,
                                                          boolean isSelected, boolean cellHasFocus) 
            {
                Component c = super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
                JLabel label = (JLabel) c;
                assert value instanceof JointState;
                JointState state = (JointState)value;
                
                if (!isSelected)
                    label.setForeground(state.stateNum < currStateIndex ? Color.BLACK : Color.GRAY);
                label.setText(state.markingText);
                Icon ico = EMPTY_ICON;
                if (state.isTop())
                    ico = TOP_STATE;
                else if (state.isBot())
                    ico = BOT_STATE;
                label.setIcon(ico);
                label.setDisabledIcon(ico);   
                label.setFont(currentState()==state ? boldListFont : normalListFont);
//                label.setForeground(Color.yellow);
                
                return c;
            }
        };
        jListPathTrace.setCellRenderer(pathRenderer);
        jListPathTrace.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseReleased(MouseEvent me) {
                if (isFiring())
                    return;
                int index = jListPathTrace.locationToIndex(me.getPoint());
                if (index == -1)
                    return;
                currStateIndex = index;
                mainInterface.invalidateGUI();
            }
        });
        
        jPanelNextTime.setVisible(isTimedSimulation());
        jPanelUntimed.setVisible(!isTimedSimulation());
        jSliderTimeStep.addChangeListener(timeStepSliderListener);
    }
    
    private Icon EMPTY_ICON = ResourceFactory.getInstance().getEmpty16();
    private Icon TOP_STATE = ResourceFactory.getInstance().getTop16();
    private Icon BOT_STATE = ResourceFactory.getInstance().getBot16();

    private Icon IMM_TRN_ICON = ResourceFactory.getInstance().getToolImmTrn16();
    private Icon EXP_TRN_ICON = ResourceFactory.getInstance().getToolTrn16();
    private Icon GEN_TRN_ICON = ResourceFactory.getInstance().getToolDetTrn16();
    private Icon CONT_TRN_ICON = ResourceFactory.getInstance().getToolFlowtrn16();

    private Icon INNER_EDGE_ICON = ResourceFactory.getInstance().getToolArrowTriangle16();
    private Icon BOUNDARY_EDGE_ICON = ResourceFactory.getInstance().getToolArrowImmed16();
    
    private Icon TIME_ELAPSE_ICON = ResourceFactory.getInstance().getTimeElapse16();
    
    private Font normalListFont, boldListFont;
//    Icon TIME_ICON = Main.loadImageIcon("time.png");
    
    private class NetPlayViewerPanel extends NetViewerPanel {
        Point2D mousePt = new Point2D.Double();
        public NetPlayViewerPanel(JScrollPane scrollPane) {
            super(scrollPane);
        }

        @Override
        public void netClicked(JNetPanel panel, NetPage net, MouseEvent evt) {
            if (isFiring() || !panel.isEnabled()) {
                return;
            }
            int zoom = viewerPanel.getZoomLevel();
            mousePt.setLocation(NetObject.screenToLogic(evt.getX(), zoom) + panel.pageBounds.getX(),
                                NetObject.screenToLogic(evt.getY(), zoom) + panel.pageBounds.getY());
            // Search for an active node that can fire
            Rectangle2D hitRect = NetObject.makeHitRectangle(mousePt, zoom);
            for (Node n : net.nodes) {
                FirableWithBindings<Transition> enabledBind;
                if (n instanceof Transition
                    && null != (enabledBind = currentState().getEnabledFirableBindingsOf((Transition) n))
                    && n.intersectRectangle(hitRect, net.viewProfile, true)) 
                {
                    // Fire this transition node
                    chooseBindingAndFire(enabledBind, panel, evt.getX(), evt.getY());
                    return;
                }
            }
            for (Edge e : net.edges) {
                if (e instanceof DtaEdge
                    && currentState().enabledDtaEdges.contains((DtaEdge) e)
                    && e.intersectRectangle(hitRect, net.viewProfile, true)) {
                    // Fire the active edge
                    startNodeFiring((DtaEdge)e, null);
                    return;
                }

            }
        }

        @Override
        public String getOverlayMessage(NetPage net) {
            if (net instanceof DtaPage && currStateIndex >= 0 && 
                !isFiring() && !viewerPanel.isInBindingPhase()) 
            {
                if (currentState().isTop())
                    return "\\textcolor{0000FF}{\\text{Path is accepted.}}";
                else if (currentState().isBot())
                    return "\\textcolor{AA0000}{\\text{Path is rejected.}}";
            }
            return null;
        }

        @Override
        public void setupDrawContext(NetPage net, DrawHelper dh) {
            // Set the current net state
            if (isFiring()) {
                long time = (new java.util.Date().getTime() - startAnimTime.getTime());
                Tuple<Double,Double> ph = lastFiring.getAnimationPhases((int)time);
                dh.activity = new ActivityState(currentState(), lastFiring, ph.x, ph.y);
//                System.out.println(" firingAnimCoeff = "+dh.activity.phase);
                if (isTimedSimulation()) {
                    double actualTime;
                    if (dh.activity.isFlowPhase())
                        actualTime = NetObject.linearInterp(lastFiring.startTime, 
                                                             lastFiring.reachedState.time,
                                                             dh.activity.getFlowCoeff());
                    else
                        actualTime = lastFiring.reachedState.time;
                    jTextField_Time.setText(String.format(Locale.US, "%.4f", actualTime));
                }
            }
            else if (currStateIndex >= 0) {
                // Pulse animation
                double pulseCoeff = (standAnimPhase % 2) == 0 ? 1.0 : 0.5;
                dh.activity = new ActivityState(currentState(), pulseCoeff);
//                System.out.println(" pulse = "+pulseCoeff);
            }
        }

        @Override
        public void allBindingsOk(boolean allOk) {
            jToolbarButtonStart.setEnabled(allOk);
        }

        @Override
        public Color getNetBackground(String overlayMsg) {
            boolean isWhite = (isEnabled() && currStateIndex >= 0 && 
                               !viewerPanel.isInBindingPhase());
            return isWhite ? Color.WHITE : VERY_LIGHT_GRAY_BKGND;
        }
    }
    
    public void initializeFor(GspnPage inGspn, DtaPage inDta) {
        origGspn = inGspn;
        origDta = inDta;
        
        GspnPage gspn;
        DtaPage dta;
        if (inGspn != null) {
            gspn = (GspnPage)Util.deepCopyRelink(inGspn);
            gspn.preparePageCheck();
            gspn.checkPage(null, null, null, null);
            if (inDta == null)
                dta = null;
            else {
                dta = (DtaPage)Util.deepCopyRelink(inDta);
                dta.preparePageCheck();
                dta.checkPage(null, null, null, null);
            }
        }
        else throw new UnsupportedOperationException();
        
        gspn.setSelectionFlag(false);
        gspn.setGrayedFlag(false);
        gspn.setAuraFlag(false);
        gspn.viewProfile.viewSuperPosTags = false;
        gspn.viewProfile.viewRatesDelays = isTimedSimulation();
        
        if (dta != null) {
            dta.setSelectionFlag(false);
            dta.setGrayedFlag(false);
            dta.setAuraFlag(false);
            dta.viewProfile.viewSuperPosTags = false;
            dta.viewProfile.viewRatesDelays = isTimedSimulation();
        }

        // Setup the net viewer in the scrollpane
        viewerPanel = new NetPlayViewerPanel(jScrollPaneAllNets);
        viewerPanel.initializePanel(gspn, dta, true /* show binding forms */);
        jScrollPaneAllNets.setViewportView(viewerPanel);
    }
    
    
    public void closeBindingForm() {
        viewerPanel.closeBindingForm();
        ((CardLayout)getLayout()).show(this, CONTROLS_CARD);

        // Prepare the initial state
        stateHistory.add(getInitialState(viewerPanel.getGspn(), viewerPanel.getGspnBinding(), 
                                         viewerPanel.getDta(), viewerPanel.getDtaBinding(),
                                         isTimedSimulation()));
        currStateIndex = 0;

        // Start the timers
        startAnimTime = new java.util.Date();
        standAnimPhase = 0;
        boolean isTimerNew = (firingTimer == null);
        if (isTimerNew)
            firingTimer = new Timer(ANIM_PULSE_SPEED, firingAnimationListener);
        firingTimer.setInitialDelay(ANIM_PULSE_SPEED);
        if (isTimerNew)
            firingTimer.start();
        else
            firingTimer.restart();
        
        //setupTimeStepPanel(true);
    }
    
    
    private void setupTimeStepPanel(boolean enabled) {
        boolean isTimedStep = false;
        boolean hasEnabledTimedTransition = false;
        //double nextTime = currentState().time;
        
        if (currStateIndex >= 0) {
            isTimedStep = enabled && isTimedSimulation() && currentState().isTangible();
            // Check if the time step panel is effectively used by some transition.
            if (currStateIndex >= 0 && currentState().isTangible()) {
                for (FirableWithBindings<Transition> fwb : currentState().enabledTransitions) {
                    if (fwb.firable.isTimed()) {
//                        if (currentState().countBindingsWithLimitedTimeSupport(fwb.firable, false) == 0) {
                        // There is some enabled transition that depends on the time panel
                        hasEnabledTimedTransition = true;
                        break;
                    }
                }
                if (!hasEnabledTimedTransition) {
                    isTimedStep = false;
                }
            }
        }
        textFieldTimeStep.setBackground(Color.WHITE);
        textFieldTimeStep.setEditable(isTimedStep);
        jSliderTimeStep.setEnabled(isTimedStep);
        jButtonRandomTime.setEnabled(isTimedStep);
//        jButtonSwitchToTimed.setEnabled(enabled);
//        jButtonSwitchToUntimed.setEnabled(enabled);
        if (isTimedSimulation() && currStateIndex >= 0) {
            if (enabled) {
                if (currentState().isTangible() && hasEnabledTimedTransition) {
                    setRandomTimeStep();
                }
                else if (!hasEnabledTimedTransition) {
                    textFieldTimeStep.setText(" -");
                }
                else if (currentState().isVanishing()) {
                    String reason = "<immediate>";
                    if (currentState().isTop() || currentState().isBot())
                        reason = "<stop>";
                    textFieldTimeStep.setText(reason);
                }
            }
            jTextField_Time.setText(String.format(Locale.US, "%.4f", currentState().time));
        }
    }
    
    
    private void setTimeStepTextFieldFromSlider() {
        assert isTimedSimulation() && currentState().isTangible();
        double time;
        final int max = jSliderTimeStep.getMaximum(), val = jSliderTimeStep.getValue();
        if (currentState().nextTimeBound > 0.0) {
            // We have a time bound
            if (max == val)
                time = currentState().nextTimeBound;
            else {
                time = (currentState().nextTimeBound / max) * val;
                time = Double.valueOf(String.format(Locale.US, "%.4f", time));
            }
        }
        else {
            // We have an average time
            double step = Math.pow(10, Math.round(Math.log10(currentState().avgTimeNextTransition)) - 1);
            time = (step * val);
            time = Double.valueOf(String.format(Locale.US, "%.4f", time));
        }
        textFieldTimeStep.setText("" + time);        
    }
    
    
    private void setRandomTimeStep() {
        assert isTimedSimulation() && currentState().isTangible();
        int min = jSliderTimeStep.getMinimum(), max = jSliderTimeStep.getMaximum();
        int sliderPos = min + randomGenerator.nextInt(max - min);
        if (currentState().nextTimeBound > 0.0) {
            // Measure how probable is that the next time step is the boundary
            double r = currentState().avgTimeNextTransition / currentState().nextTimeBound;
            if (r > 3) { // Already quite probable
                if (randomGenerator.nextDouble() > 1/r)
                    sliderPos = max;
            }
        }
        boolean isChanging = (sliderPos != jSliderTimeStep.getValue());
        jSliderTimeStep.setValue(sliderPos);
        if (!isChanging)
            setTimeStepTextFieldFromSlider(); // call it manually!!
    }
    
        
    private final ChangeListener timeStepSliderListener = new ChangeListener() {
        @Override
        public void stateChanged(ChangeEvent e) {
            setTimeStepTextFieldFromSlider();
        }
    };
    
    
    private void restartFromInitialState(boolean reopenBindings) {
        stateHistory = new ArrayList<>();
        currStateIndex = -1;
        lastFiring = null;
        
        ((CardLayout)getLayout()).show(this, BINDING_CARD);
        viewerPanel.reopenBindingForms();
        if (!viewerPanel.hasBindingForms() || !reopenBindings)
            closeBindingForm();
    }
    
    
    private ActionListener firingAnimationListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent ae) {
            if (isFiring()) {
                long time = (new java.util.Date().getTime() - startAnimTime.getTime());
                if (time > lastFiring.getTotalAnimationTime()) {
                    // Firing animation has terminated, step back to the stand animation
                    lastFiring = null;
//                    mainInterface.setSuspended(false);

                    startAnimTime = new java.util.Date();
                    standAnimPhase = 0;
                    firingTimer.setDelay(ANIM_PULSE_SPEED);
                    firingTimer.setInitialDelay(ANIM_PULSE_SPEED);
                    firingTimer.restart();
                    mainInterface.invalidateGUI();
                    //setupTimeStepPanel(true);
                }
            }
            else {
                standAnimPhase++;
                // Should we fire automatically ?
                boolean semiAutoFiring = checkBoxSemiAutoFiring.isSelected();
                boolean randomFiring = checkBoxRandomFiring.isSelected();
                if (standAnimPhase >= 2 && currStateIndex >= 0 &&
                    (semiAutoFiring || randomFiring) && !popupVisible) 
                {
                    // Make the list of firable transitions
                    ArrayList<Tuple<Firable, ColorVarsBinding>> firables 
                            = new ArrayList<>();
                    for (FirableWithBindings<Transition> fwb : currentState().enabledTransitions)
                        if (fwb.firable.isFiringInstantaneous())
                            for (ColorVarsBinding binding : fwb.bindings)
                                firables.add(new Tuple<Firable, ColorVarsBinding>(fwb.firable, binding));
                    for (DtaEdge edge : currentState().enabledDtaEdges)
                        firables.add(new Tuple<Firable, ColorVarsBinding>(edge, null));
                    if (currentState().allowTimeElapse())
                        firables.add(new Tuple<Firable, ColorVarsBinding>(TimeElapse.SINGLETON, null));
                    if ((randomFiring && firables.size() >= 1) ||
                        (semiAutoFiring && firables.size() == 1))
                    {
                        // Select the firing transition
                        int firedIndex = randomGenerator.nextInt(firables.size());
                        
                        // Start a new event firing automatically
                        startNodeFiring(firables.get(firedIndex).x, firables.get(firedIndex).y);
                        return;
                    }
                }
            }
            // Redraw the window for the new animation frame
            viewerPanel.repaint();
        }
    };
    
    private boolean popupVisible = false;
    
    private void chooseBindingAndFire(final FirableWithBindings<Transition> fwb, 
                                      Component invoker, int x, int y) 
    {
        // Flow transitions do not fire with a mouse click.
        if (fwb.firable.isFiringFlow())
            return;
        assert fwb.bindings.size() >= 1;
        if (fwb.bindings.size() == 1) {
            // Just one single choice, do not open the popup menu.
            startNodeFiring(fwb.firable, fwb.bindings.iterator().next());
            return;
        }
        JPopupMenu popup = new JPopupMenu();
        for (final ColorVarsBinding bind : fwb.bindings) {
            JMenuItem item = new JMenuItem("Fire with binding: "+bind);
            item.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    startNodeFiring(fwb.firable, bind);
                }
            });
            popup.add(item);
        }
        popup.addPopupMenuListener(new PopupMenuListener() {
            @Override
            public void popupMenuWillBecomeVisible(PopupMenuEvent e) {
                popupVisible = true;
            }
            @Override
            public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {
                popupVisible = false;
            }
            @Override
            public void popupMenuCanceled(PopupMenuEvent e) {
                popupVisible = false;
            }
        });
        popup.show(invoker, x, y);
    }

    
    private final Color TEXTFIELD_TIMESTEP_ERROR_BACKGROUND = new Color(255, 240, 240);
    private void startNodeFiring(Firable n, ColorVarsBinding binding) {
        // Do not fire continuous transitions
        if (n.isFiringFlow())
            return;
        // Chech if everything is ok
        double elapsedTime = -1;
        if (isTimedSimulation()) {
            if (currentState().isTangible()) {
                // Check if this event has a specified firing instant
                Tuple<Firable, ColorVarsBinding> trnBind = new Tuple<>(n, binding);
                if (currentState().firingTimeSupport.containsKey(trnBind)) {
                    Bound support = currentState().firingTimeSupport.get(trnBind);
                    if (support.isImpulse())
                        elapsedTime = support.a;
                    else {
                        elapsedTime = Double.parseDouble(textFieldTimeStep.getText());
                        if (elapsedTime <= 0.0)
                            throw new NumberFormatException();
                        elapsedTime = Math.max(elapsedTime, support.a);
                        elapsedTime = Math.min(elapsedTime, support.b);
                    }
                    // Change the time step textfield, for the animation
                    textFieldTimeStep.setText("" + elapsedTime);
                }
                else {
                    // Get the user-provided time step from the textfield
                    try {
                        elapsedTime = Double.parseDouble(textFieldTimeStep.getText());
                        if (elapsedTime <= 0.0)
                            throw new NumberFormatException();
                    }
                    catch (NumberFormatException e) {
                        textFieldTimeStep.requestFocusInWindow();
                        textFieldTimeStep.setBackground(TEXTFIELD_TIMESTEP_ERROR_BACKGROUND);
                        return; // Could not fire
                    }
                }
            }
            else elapsedTime = 0.0; // immediate, no time elapse
        }
        
        // Do the firing!
        lastFiring = fireTransition(viewerPanel.getGspn(), viewerPanel.getGspnBinding(), 
                                    viewerPanel.getDta(), viewerPanel.getDtaBinding(), 
                                    currentState(), n, binding, lastFiring, 
                                    elapsedTime, getSpeedUp());
        for (int i=stateHistory.size()-1; i > currStateIndex; i--)
            stateHistory.remove(i);
        stateHistory.add(lastFiring.reachedState);
        currStateIndex++;
        startAnimTime = new java.util.Date();
        firingTimer.setDelay(ANIM_FIRING_SPEED);
        firingTimer.setInitialDelay(ANIM_FIRING_SPEED);
        firingTimer.restart();

//        mainInterface.setSuspended(true);
        mainInterface.invalidateGUI();
    }

    
    private double getSpeedUp() {
        // Speedup goes from 0.25 to 3.0
        int max = jSliderSpeed.getMaximum(), val = jSliderSpeed.getValue();
        return NetObject.linearInterp(0.25, 3.0, 1 - (val / (double)max));
    }
    
    
    private boolean timedSimulation = false;
    private boolean isTimedSimulation() {
        return timedSimulation;
    }
    public void setTimedSimulation(boolean timedSimulation) {
        this.timedSimulation = timedSimulation;
        jPanelNextTime.setVisible(timedSimulation);
        jPanelUntimed.setVisible(!timedSimulation);
    }
    
    
    @Override
    public boolean isModalEditor() {
        return true;
    }

    @Override
    public void setEditorEnabledFor(ProjectFile pf, ProjectPage page, MainWindowInterface interf) {
//        System.out.println("NetPlayPanel.setEditorEnabledFor");
        assert mainInterface == null;
//        currFile = pf;
//        currPage = (NetPage)page;
        mainInterface = interf;
        
        restartFromInitialState(true);
    }

    @Override
    public void setEditorDisabled() {
//        System.out.println("NetPlayPanel.setEditorDisabled");
        if (firingTimer != null) {
            firingTimer.stop();
            firingTimer.removeActionListener(firingAnimationListener);
            firingTimer = null;
        }
        startAnimTime = null;
        
        jListEnabledTransitions.setModel(new DefaultListModel<>());
        jListPathTrace.setModel(new DefaultListModel<JointState>());
                
        // Save the template bindings
        viewerPanel.saveBindingParameters(mainInterface, origGspn, origDta);
        
        assert mainInterface != null;
        mainInterface = null;
        viewerPanel.disablePanel();
        jScrollPaneAllNets.setViewportView(null);
        viewerPanel = null;        
    }

    @Override
    public JComponent getEditorComponent() {
        return jPanelAllNets;
    }

    @Override
    public JComponent getFocusTarget() {
        return jPanelAllNets;
    }
    
    @Override
    public JComponent getPropertyPanel() { 
        return this;
    }

    @Override
    public JComponent getToolbar() {
        return jToolBarPlayCommands;
    }

    @Override
    public PageErrorWarning[] getAllErrorsAndWarnings() {
        return NO_ERRORS;
    }
    
    @Override
    public void updateEnablingOfSharedActions(SharedResourceProvider shResProv) {
        boolean canBeUsed = !isFiring() && !mainInterface.isSuspended();
        
        for (SharedResourceProvider.ActionName actName : SharedResourceProvider.ActionName.values()) {
            common.Action act = shResProv.getSharedAction(actName);
            
            switch (actName) {
                case PLAY_NEXT:
                    act.setEnabled(canBeUsed && currStateIndex < (stateHistory.size() - 1));
                    break;

                case PLAY_PREVIOUS:
                    act.setEnabled(canBeUsed && currStateIndex > 0);
                    break;

                case PLAY_RESTART:
                    act.setEnabled(canBeUsed && (currStateIndex > 0));
                    break;
                    
                case CHANGE_BINDINGS:
                    act.setEnabled(canBeUsed && viewerPanel.hasBindingForms() &&
                                   !viewerPanel.isInBindingPhase());
                    break;

                default:
                    act.setEnabled(false);
            }
        }
    }
    
    @Override
    public void sharedActionListener(SharedResourceProvider.ActionName shAction, ActionEvent event) {
        switch (shAction) {
            case PLAY_NEXT: {
                assert !isFiring();
                assert currStateIndex < (stateHistory.size() - 1);
                currStateIndex++;
                mainInterface.invalidateGUI();
                break;
            }
                
            case PLAY_PREVIOUS: {
                assert !isFiring();
                assert currStateIndex > 0;
                currStateIndex--;
                mainInterface.invalidateGUI();
                break;
            }
                
            case PLAY_RESTART: {
//                currStateIndex = 0;
//                JointState init = stateHistory.get(0);
//                stateHistory.clear();
//                stateHistory.add(init);
//                lastFiring = null;
                restartFromInitialState(false);
                mainInterface.invalidateGUI();
                break;
            }
                
            case CHANGE_BINDINGS: {
                restartFromInitialState(true);
                mainInterface.invalidateGUI();
                break;
            }
        }
    }
    
    @Override
    public void updateGUI(ProjectFile pf, ProjectPage page) {
        boolean canBeUsed = !isFiring() && !mainInterface.isSuspended();
        boolean isStarted = canBeUsed && !viewerPanel.isInBindingPhase();
        
        boolean allBindingsOk = viewerPanel.areAllBindingsOk();
        jToolbarButtonStart.setEnabled(allBindingsOk && currStateIndex == -1);
        
        jToolbarButtonSwitchToTimed.setVisible(!isTimedSimulation());
        jToolbarButtonSwitchToUntimed.setVisible(isTimedSimulation());

        jListEnabledTransitions.setEnabled(isStarted);
        jListPathTrace.setEnabled(isStarted);
        
        // Update the list of enabled transitions
        DefaultListModel<Object> model = new DefaultListModel<>();
        if (currStateIndex >= 0) {
            for (FirableWithBindings<Transition> fwb : currentState().enabledTransitions)
                model.addElement(fwb);
            for (DtaEdge e : currentState().enabledDtaEdges)
                model.addElement(e);
            if (currentState().allowTimeElapse())
                model.addElement(TimeElapse.SINGLETON);
        }
        jListEnabledTransitions.setModel(model);
        
        // Update the list of states in the path
        DefaultListModel<JointState> pathModel = new DefaultListModel<>();
        for (JointState js : stateHistory)
            pathModel.addElement(js);
        jListPathTrace.setModel(pathModel);
        jListPathTrace.setSelectedIndex(currStateIndex);
        jListPathTrace.ensureIndexIsVisible(jListPathTrace.getSelectedIndex());
        
        setupTimeStepPanel(canBeUsed);
        
        viewerPanel.getGspn().viewProfile.viewRatesDelays = isTimedSimulation();
        if (viewerPanel.getDta() != null) {
            viewerPanel.getDta().viewProfile.viewRatesDelays = isTimedSimulation();
        }
        
        viewerPanel.repaint();
    }

    @Override
    public boolean isZoomPanelUsed() {
        return true;
    }

    @Override
    public int getPageZoom() {
        return viewerPanel.getZoomLevel();
    }

    @Override
    public void zoomChanged(int newZoomLevel) {
        viewerPanel.zoomChanged(newZoomLevel);
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

        jToolBarPlayCommands = new javax.swing.JToolBar();
        jToolbarButtonSwitchToTimed = new common.JToolbarButton();
        jToolbarButtonSwitchToUntimed = new common.JToolbarButton();
        jToolbarButtonStart = new common.JToolbarButton();
        jToolbarButtonChangeBindings = new common.JToolbarButton();
        jToolbarButtonRestart = new common.JToolbarButton();
        jToolbarButtonPrev = new common.JToolbarButton();
        jToolbarButtonNext = new common.JToolbarButton();
        jPanelAllNets = new javax.swing.JPanel();
        jScrollPaneAllNets = new javax.swing.JScrollPane();
        resourceFactory = new editor.gui.ResourceFactory();
        jPanelControls = new javax.swing.JPanel();
        jPanelTransitions = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jListEnabledTransitions = new javax.swing.JList<>();
        jPanelNextTime = new javax.swing.JPanel();
        jLabelTime = new javax.swing.JLabel();
        jTextField_Time = new javax.swing.JTextField();
        jLabelTimeStep = new javax.swing.JLabel();
        jSliderTimeStep = new javax.swing.JSlider();
        textFieldTimeStep = new javax.swing.JTextField();
        jButtonRandomTime = new javax.swing.JButton();
        jPanelStates = new javax.swing.JPanel();
        jScrollPane2 = new javax.swing.JScrollPane();
        jListPathTrace = new javax.swing.JList<>();
        jPanelSpeed = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jSliderSpeed = new javax.swing.JSlider();
        jPanelUntimed = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        jPanelModes = new javax.swing.JPanel();
        checkBoxSemiAutoFiring = new javax.swing.JCheckBox();
        checkBoxRandomFiring = new javax.swing.JCheckBox();
        jPanelBinding = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();

        jToolBarPlayCommands.setFloatable(false);
        jToolBarPlayCommands.setRollover(true);

        jToolbarButtonSwitchToTimed.setIcon(resourceFactory.getTimeGo32());
        jToolbarButtonSwitchToTimed.setText("Switch to Timed");
        jToolbarButtonSwitchToTimed.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jToolbarButtonSwitchToTimedActionPerformed(evt);
            }
        });
        jToolBarPlayCommands.add(jToolbarButtonSwitchToTimed);

        jToolbarButtonSwitchToUntimed.setIcon(resourceFactory.getTimeDelete32());
        jToolbarButtonSwitchToUntimed.setText("Switch to Untimed");
        jToolbarButtonSwitchToUntimed.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jToolbarButtonSwitchToUntimedActionPerformed(evt);
            }
        });
        jToolBarPlayCommands.add(jToolbarButtonSwitchToUntimed);

        jToolbarButtonStart.setIcon(resourceFactory.getPlay32());
        jToolbarButtonStart.setText("Start!");
        jToolbarButtonStart.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jToolbarButtonStartActionPerformed(evt);
            }
        });
        jToolBarPlayCommands.add(jToolbarButtonStart);
        jToolBarPlayCommands.add(jToolbarButtonChangeBindings);
        jToolBarPlayCommands.add(jToolbarButtonRestart);
        jToolBarPlayCommands.add(jToolbarButtonPrev);
        jToolBarPlayCommands.add(jToolbarButtonNext);

        jPanelAllNets.setLayout(new java.awt.GridLayout(1, 0));
        jPanelAllNets.add(jScrollPaneAllNets);

        setLayout(new java.awt.CardLayout());

        jPanelControls.setLayout(new java.awt.GridBagLayout());

        jPanelTransitions.setBorder(javax.swing.BorderFactory.createTitledBorder("Enabled Transitions:"));
        jPanelTransitions.setLayout(new javax.swing.BoxLayout(jPanelTransitions, javax.swing.BoxLayout.LINE_AXIS));

        jListEnabledTransitions.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        jScrollPane1.setViewportView(jListEnabledTransitions);

        jPanelTransitions.add(jScrollPane1);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.3;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelControls.add(jPanelTransitions, gridBagConstraints);

        jPanelNextTime.setBorder(javax.swing.BorderFactory.createTitledBorder("Time elapsed:"));
        jPanelNextTime.setLayout(new java.awt.GridBagLayout());

        jLabelTime.setText("Global time:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelNextTime.add(jLabelTime, gridBagConstraints);

        jTextField_Time.setEditable(false);
        jTextField_Time.setText("0.0");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelNextTime.add(jTextField_Time, gridBagConstraints);

        jLabelTimeStep.setText("Time step:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelNextTime.add(jLabelTimeStep, gridBagConstraints);

        jSliderTimeStep.setMaximum(11);
        jSliderTimeStep.setMinimum(1);
        jSliderTimeStep.setMinorTickSpacing(1);
        jSliderTimeStep.setPaintTicks(true);
        jSliderTimeStep.setSnapToTicks(true);
        jSliderTimeStep.setValue(1);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        jPanelNextTime.add(jSliderTimeStep, gridBagConstraints);

        textFieldTimeStep.setText("0.0");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelNextTime.add(textFieldTimeStep, gridBagConstraints);

        jButtonRandomTime.setText("Random!");
        jButtonRandomTime.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonRandomTimeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        jPanelNextTime.add(jButtonRandomTime, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelControls.add(jPanelNextTime, gridBagConstraints);

        jPanelStates.setBorder(javax.swing.BorderFactory.createTitledBorder("Path trace:"));
        jPanelStates.setLayout(new javax.swing.BoxLayout(jPanelStates, javax.swing.BoxLayout.LINE_AXIS));

        jListPathTrace.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        jScrollPane2.setViewportView(jListPathTrace);

        jPanelStates.add(jScrollPane2);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.5;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelControls.add(jPanelStates, gridBagConstraints);

        jPanelSpeed.setLayout(new java.awt.GridBagLayout());

        jLabel1.setText("Speed:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 8, 0);
        jPanelSpeed.add(jLabel1, gridBagConstraints);

        jSliderSpeed.setMaximum(10);
        jSliderSpeed.setMinorTickSpacing(1);
        jSliderSpeed.setPaintLabels(true);
        jSliderSpeed.setPaintTicks(true);
        jSliderSpeed.setSnapToTicks(true);
        jSliderSpeed.setValue(5);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        jPanelSpeed.add(jSliderSpeed, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 6;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelControls.add(jPanelSpeed, gridBagConstraints);

        jPanelUntimed.setBorder(javax.swing.BorderFactory.createTitledBorder("Time elapsed:"));
        jPanelUntimed.setLayout(new java.awt.GridBagLayout());

        jLabel2.setText("Token game is untimed.");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelUntimed.add(jLabel2, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        jPanelControls.add(jPanelUntimed, gridBagConstraints);

        jPanelModes.setLayout(new java.awt.GridBagLayout());

        checkBoxSemiAutoFiring.setText("Semi-automatic firing.");
        checkBoxSemiAutoFiring.setToolTipText("When this checkbox is selected, stops automatic firing when there is a choice between multiple events.");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        jPanelModes.add(checkBoxSemiAutoFiring, gridBagConstraints);

        checkBoxRandomFiring.setText("Random automatic firing.");
        checkBoxRandomFiring.setToolTipText("Play the token game automatically, choosing randomly the next event.");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        jPanelModes.add(checkBoxRandomFiring, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelControls.add(jPanelModes, gridBagConstraints);

        add(jPanelControls, "controls");

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

        jLabel4.setText("<html>When all parameters have been assigned correctly, press the \"Start!\" button on the application toolbar.");
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
        jPanelBinding.add(jPanel1, gridBagConstraints);

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
    }// </editor-fold>//GEN-END:initComponents

    private void jToolbarButtonStartActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jToolbarButtonStartActionPerformed
        closeBindingForm();
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_jToolbarButtonStartActionPerformed

    private void jButtonRandomTimeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonRandomTimeActionPerformed
        setRandomTimeStep();
    }//GEN-LAST:event_jButtonRandomTimeActionPerformed

    private void jToolbarButtonSwitchToTimedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jToolbarButtonSwitchToTimedActionPerformed
        setTimedSimulation(true);
        
        if (viewerPanel.areAllBindingsOk()) {
            // Restart (without reopening the binding dialog)
            restartFromInitialState(false);
        }
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_jToolbarButtonSwitchToTimedActionPerformed

    private void jToolbarButtonSwitchToUntimedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jToolbarButtonSwitchToUntimedActionPerformed
        setTimedSimulation(false);
        
        if (viewerPanel.areAllBindingsOk()) {
            // Restart (without reopening the binding dialog)
            restartFromInitialState(false);
        }
        mainInterface.invalidateGUI();
    }//GEN-LAST:event_jToolbarButtonSwitchToUntimedActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JCheckBox checkBoxRandomFiring;
    private javax.swing.JCheckBox checkBoxSemiAutoFiring;
    private javax.swing.JButton jButtonRandomTime;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabelTime;
    private javax.swing.JLabel jLabelTimeStep;
    private javax.swing.JList<Object> jListEnabledTransitions;
    private javax.swing.JList<JointState> jListPathTrace;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanelAllNets;
    private javax.swing.JPanel jPanelBinding;
    private javax.swing.JPanel jPanelControls;
    private javax.swing.JPanel jPanelModes;
    private javax.swing.JPanel jPanelNextTime;
    private javax.swing.JPanel jPanelSpeed;
    private javax.swing.JPanel jPanelStates;
    private javax.swing.JPanel jPanelTransitions;
    private javax.swing.JPanel jPanelUntimed;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPaneAllNets;
    private javax.swing.JSlider jSliderSpeed;
    private javax.swing.JSlider jSliderTimeStep;
    private javax.swing.JTextField jTextField_Time;
    private javax.swing.JToolBar jToolBarPlayCommands;
    private common.JToolbarButton jToolbarButtonChangeBindings;
    private common.JToolbarButton jToolbarButtonNext;
    private common.JToolbarButton jToolbarButtonPrev;
    private common.JToolbarButton jToolbarButtonRestart;
    private common.JToolbarButton jToolbarButtonStart;
    private common.JToolbarButton jToolbarButtonSwitchToTimed;
    private common.JToolbarButton jToolbarButtonSwitchToUntimed;
    private editor.gui.ResourceFactory resourceFactory;
    private javax.swing.JTextField textFieldTimeStep;
    // End of variables declaration//GEN-END:variables
}
