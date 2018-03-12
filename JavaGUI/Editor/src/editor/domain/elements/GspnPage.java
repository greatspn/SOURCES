/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import common.MutablePair;
import common.Triple;
import common.Tuple;
import editor.domain.Edge;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.gui.AbstractPageEditor;
import editor.gui.RapidMeasureCmd;
import editor.gui.ResourceFactory;
import editor.gui.net.NetPlayPanel;
import editor.gui.SharedResourceProvider;
import java.awt.datatransfer.DataFlavor;
import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

/** A project page that contains a GSPN model.
 *
 * @author elvio
 */
public class GspnPage extends NetPage implements Serializable {

    public GspnPage() { }
    
    // True if this page contains colors (and therefore enables actions like the unfolding)
    private transient Boolean hasColors = null;
    // True if the model has general transitions
    private transient Integer numGeneralTransitions = null;
    
    public boolean isUntimed() { return !viewProfile.viewRatesDelays; }
    public boolean isTimed()   { return viewProfile.viewRatesDelays; }
    
    @Override
    protected boolean checkPageCorrectness(boolean isNewOrModified, ProjectData proj, 
                                           Set<ProjectPage> changedPages, 
                                           ProjectPage invokerPage) 
    {
        // Do nodes/edges validation
        if (!super.checkPageCorrectness(isNewOrModified, proj, changedPages, invokerPage))
            return false;
        
        // Extra GSPN rules that has to be checked.

        Set<Triple<Node, Node, GspnEdge.Kind>> knownGspnArcs = new HashSet<>();
        for (Edge edge : edges) {
            // Verify that GSPN arcs do not overlap from/to the same <place,transition> pair
            if (edge instanceof GspnEdge) {
                GspnEdge ge = (GspnEdge)edge;
                if (edge.getTailNode() != null && edge.getHeadNode() != null) {
                    Triple<Node, Node, GspnEdge.Kind> t = new Triple<>
                            (ge.getHeadNode(), ge.getTailNode(), ge.getEdgeKind());
                    if (knownGspnArcs.contains(t)) {
                        addPageError("Multiple GSPN arcs of the same type with the "+
                                     "same <Place,Transition> pair.", edge);
                    }
                    knownGspnArcs.add(t);
                }
            }
        }
        
        // Verify that arcs from discrete places to continuous transitions
        // are test-arcs only, or inhibitors.
        Map<MutablePair<Node, Node>, GspnEdge> dcTestArcs = new HashMap<>();
        MutablePair<Node, Node> nn = new MutablePair<>(null, null);
        for (Edge edge : edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge ge = (GspnEdge)edge;
                if (ge.isDiscrete() && ge.isFiringFlow() && ge.getEdgeKind()!=GspnEdge.Kind.INHIBITOR) {
                    // Search for the opposite arc
                    nn.setPair(ge.getTailNode(), ge.getHeadNode());
                    if (dcTestArcs.containsKey(nn)) {
                        GspnEdge otherEdge = dcTestArcs.get(nn);
                        assert otherEdge != null;
                        if (!otherEdge.getMultiplicity().equals(ge.getMultiplicity())) {
                            addPageError("Arcs from a discrete place to a continuous transition "
                                       + "must have the same multiplicity.", edge);
                        }                        
                        dcTestArcs.remove(nn);
                    }
                    else {
                        // Insert this arc, waiting its opposite
                        dcTestArcs.put(new MutablePair<>(nn.second, nn.first), ge);
                    }
                }
            }
        }
        for (GspnEdge ge : dcTestArcs.values()) {
            addPageError("Arcs from a discrete place to a continuous transition "
                       + "can only be test arcs. Add another arc in the opposite "
                       + "direction to do the test.", ge);
        }
        
        // Verify tha color variables that appear in Transition guards
        // also appear on the transition's edges.
        Set<Tuple<Transition, ColorVar>> allowedColorVars = new HashSet<>();
        for (Edge edge : edges) {
            if (edge instanceof GspnEdge) {
                GspnEdge ge = (GspnEdge)edge;
                Transition connTrn = ge.getConnectedTransition();
                if (connTrn == null)
                    continue;
                Set<ColorVar> cv = ge.getColorVarsInUse();
                if (cv == null || cv.isEmpty())
                    continue;
                
                for (ColorVar var : cv) {
                    allowedColorVars.add(new Tuple<>(connTrn, var));
                }
            }
        }
        hasColors = false;
        boolean hasKroneckerPartitions = false;
        numGeneralTransitions = 0;
        for (Node node : nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition)node;
                Set<ColorVar> cv = trn.getColorVarsInUse();
                if (cv == null || cv.isEmpty())
                    continue;
                for (ColorVar var : cv) {
                    if (!allowedColorVars.contains(new Tuple<>(trn, var))) {
                        addPageError("Color variable '"+var.getUniqueName()+"' used in expressions of transition '"+
                                     trn.getUniqueName()+"' does not appear on any arc of that transition.", trn);
                    }
                }
                if (trn.isGeneral())
                    numGeneralTransitions++;
            }
            // Piggyback: test for presence of Kronecker partition labels
            if (node instanceof Place) {
                Place plc = (Place)node;
                if (plc.getKroneckerPartition().length() > 0)
                    hasKroneckerPartitions = true;
                if (plc.isInColorDomain())
                    hasColors = true;
            }
        }
        
        // If Kronecker partitions are used, all places should have a partition
        if (hasKroneckerPartitions) {
            Set<Selectable> withoutPartition = new HashSet<>();
            for (Node node : nodes) {
                if (node instanceof Place) {
                    Place plc = (Place)node;
                    if (plc.getKroneckerPartition().length() == 0) {
                        withoutPartition.add(plc);
                    }
                }
            }
            if (withoutPartition.size() == 1)
                addPageWarning("Place does not have a partition label that identifies "
                        + "its Kronecker component.", withoutPartition.iterator().next());
            else if (withoutPartition.size() > 0)
                addPageWarningSet("Some places do not have a partition label that identifies "
                        + "their Kronecker components.", withoutPartition);
            if (withoutPartition.size() > 0)
                addPageError("If Kronecker partitions are used, all places should have "
                           + "a non-empty partition label.", null);
        }
        
        return true;
    }
    
    @Override public Icon getPageIcon() {
        return ResourceFactory.getInstance().getPageNet16();
    }
    @Override public String getPageTypeName() {
        return "GSPN";
    }
    
    private static final DataFlavor DATA_FLAVOUR = new DataFlavor(GspnPage.class, "GSPN Objects");
    @Override public DataFlavor getDataFlavour() { return DATA_FLAVOUR; }

    @Override public boolean pageSupportsPlay() { return true; }
    
    @Override public PlayCommand canPlay(ProjectFile project) {
        // Should return only if the GSPN is ok
        return isPageCorrect() ? PlayCommand.IMMEDIATE : PlayCommand.NO;
    }
    @Override public JPopupMenu getPlayDropdownMenu(ProjectFile project, javax.swing.Action action) {
        throw new UnsupportedOperationException();
    }
    private static NetPlayPanel playPanel;
    @Override public AbstractPageEditor getPlayWindow(SharedResourceProvider shActProv, JMenuItem menuItem) {
        if (playPanel == null)
            playPanel = new NetPlayPanel(shActProv);
        assert isPageCorrect();
        playPanel.initializeFor(this, null);
        return playPanel;
    }
    
    @Override
    public boolean hasPlaceTransInv() {
        return (hasColors==null || !hasColors);
    }
    
    @Override public boolean pageSupportsAlgebraTool() { return true; }
    
    @Override
    public boolean pageSupportsUnfolding() {
        return hasColors!=null && hasColors;
    }

    @Override
    public boolean canBeUnfolded() {
        return isPageCorrect() && hasColors!=null && hasColors;
    }
    
    public boolean gspnHasColors() {
        if (isPageCorrect())
            return hasColors!=null && hasColors;
        return false;
    }
    
    @Override
    public boolean pageSupportsRG(RgType rgType) {
        boolean isCPN = (hasColors!=null && hasColors);
        switch (rgType) {
            case SRG:
                return isCPN;
            case RG:
                return true;
            case CTMC:
                return !isCPN && isTimed();
        }
        throw new IllegalStateException();
    }

    @Override
    public boolean canBuildRG(RgType rgType) {
        return isPageCorrect();
    }
    
    @Override
    public boolean hasRapidMesures() { return true; }
    @Override
    public boolean pageSupportsRapidMeasure(RapidMeasureCmd rmc) {
        boolean isCPN = (hasColors!=null && hasColors);
        boolean isGEN = (numGeneralTransitions!=null && numGeneralTransitions>0);
        switch (rmc) {
            case CTL_MODEL_CHECKING:        return !isCPN;
            case BUILD_RG:                  return true;
            case BUILD_SYMRG:               return isCPN;
            case BUILD_RS_MDD:              return !isCPN;
            case STEADY_STATE_SOLUTION:     return isTimed() && !isGEN;
            case TRANSIENT_SOLUTION:        return isTimed() && !isGEN;
            case STEADY_STATE_SIM:          return isTimed();
            case TRANSIENT_SIM:             return isTimed();
            default:
                return false;
        }
    }
    @Override
    public boolean canDoRapidMeasure(RapidMeasureCmd rmc) {
        return isPageCorrect();
    }


}
