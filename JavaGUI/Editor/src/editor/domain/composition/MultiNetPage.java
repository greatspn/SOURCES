/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.composition;

import common.EmptyIterator;
import common.Util;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.ProjectResource;
import editor.domain.ViewProfile;
import static editor.domain.composition.UnfoldingCompositionPage.NUM_OFFSET_ROWS;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ConstantID;
import editor.domain.elements.GspnPage;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.NodeNamespace;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import editor.domain.io.XmlExchangeable;
import editor.gui.AbstractPageEditor;
import editor.gui.RapidMeasureCmd;
import editor.gui.SharedResourceProvider;
import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.datatransfer.DataFlavor;
import java.awt.print.PageFormat;
import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.UUID;
import javax.swing.Action;
import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/** Abstract composition of n subnets.
 *
 * @author elvio
 */
public abstract class MultiNetPage extends ProjectPage implements Serializable, ComposableNet, NodeNamespace, XmlExchangeable {
    
    // Current viewport position
    public Point viewportPos = new Point(0, 0);
    
    // View profile
    public ViewProfile viewProfile = new ViewProfile();
    
    // Descriptor of subnets that will be composed by this multi-net page
    public final ArrayList<NetInstanceDescriptor> netsDescr = new ArrayList<>();
    // Is replication counters fully instantiatable, or are they also parametric?
//    public boolean areReplicaCountParametric;
    
    // Binding values for template variables to be propagated to the composed net
    private Map<String, String> lastBindingExprs = new HashMap<>();
    
    //-------------------------------------------------------------------------
    // The composed net
    private transient NetPage compNet;
    
    // How the composed net is visualized
    private transient String[] visualizedSubNetNames;
    private transient NetPage[] visualizedSubNets;
    
    
    // Visualization
    public NetPage[] getVisualizedSubnets() { 
        return visualizedSubNets;
    }
    public String[] getVisualizedSubnetNames() { 
        return visualizedSubNetNames;
    }
    
    protected void setCompositionTarget(NetPage compNet) {
        this.compNet = compNet;
    }
    
    protected void setCompositionSuccessfull(NetPage compNet, ViewProfile newProfile,
                                             String[] visualizedSubNetNames, NetPage[] visualizedSubNets) 
    {
        newProfile.zoom = this.viewProfile.zoom;
        this.viewProfile = newProfile;
        this.compNet = compNet;
        this.visualizedSubNetNames = visualizedSubNetNames;
        this.visualizedSubNets = visualizedSubNets;
    }
    
    // Return true if the composed net is available, even if it contains some error
    // This method is different from isPageCorrect(), which requires the compNet to be fully correct.
    public boolean areSubnetsVisualizable() {
        return visualizedSubNets != null && visualizedSubNets.length > 0;
    }

    // Default failure target for @compNet (for operators generating GSPN)
    public static final GspnPage UNSUCCESSFULL_GSPN_TARGET;
    static {
        UNSUCCESSFULL_GSPN_TARGET = new GspnPage();
        UNSUCCESSFULL_GSPN_TARGET.setPageName("ERROR");
        UNSUCCESSFULL_GSPN_TARGET.addPageError("Could not compose.", null);
    }
    
    public MultiNetPage() {
    }
    
    
    //==========================================================================

    public static final Color[] RED_PALETTE = {
        new Color(0x800000), // maroon
        new Color(0xFF8C00), // dark orange
        new Color(0xFF0000), // red
        new Color(0xFFD700), // gold
        new Color(0xFF7F50), // coral
        new Color(0xFFFF00), // yellow
        new Color(0xFFB6C1), // light pink
        new Color(0xB8860B), // dark golden rod
        new Color(0xFF1493), // deep pink
        new Color(0xBDB76B), // dark khaki
        new Color(0xC71585), // medium violet red
        new Color(0xFF6347), // tomato
        new Color(0xFF00FF), // magenta / fuchsia
    };

    public static final Color[] BLUE_PALETTE = {
        new Color(0x6495ED), // corn flower blue
        new Color(0x00BFFF), // deep sky blue
        new Color(0xADD8E6), // light blue
        new Color(0x1E90FF), // dodger blue
        new Color(0x191970), // midnight blue
        new Color(0x00FFFF), // aqua
        new Color(0xE0FFFF), // light cyan
        new Color(0x3CB371), // medium sea green
        new Color(0x2F4F4F), // dark slate gray
        new Color(0x00FF7F), // spring green
        new Color(0x7B68EE), // medium slate blue  
    };
    
    //==========================================================================

    @Override
    public Node getNodeByUniqueName(String id) {
        return null;
    }

    @Override
    public Iterator<ColorClass> colorClassIterator() {
        @SuppressWarnings("unchecked") Iterator<ColorClass> it = EmptyIterator.INSTANCE;
        return it;
    }

    //==========================================================================
    // Composition interface
    
    public abstract String getOperatorName();
    public abstract Icon getOperatorIcon();
    
    // Is the number of composed nets fixed or variable?
    public abstract boolean hasFixedNumOfOperators();
    
    // Get the fixed number of composed nets
    public abstract int getFixedNumOfOperators();
    
    // Is this net composable?
    public abstract boolean canComposeWith(ProjectPage page);
    
    // Is net replication count available
    public abstract boolean useReplicaCount();
    
    // Net requires to instantiate color class parameters?
    public abstract boolean requireParamBinding();
    
    // do the net composition
    protected abstract void compose(ParserContext context);
    
    // reset composition targets. This allows to set a default compNet 
    // when composition fails
    protected abstract void resetCompositionTargets();
    
    // color of merged nodes for visualization
    public abstract Color colorOfMergedNode(Node node);

    //==========================================================================

    @Override
    public Set<TemplateVariable> enumerateParamsForNetComposition() {
        assert isPageCorrect() && compNet != null;
        return compNet.enumerateParamsForNetComposition();
    }

    @Override
    public void instantiateParams(TemplateBinding binding) {
        assert isPageCorrect() && compNet != null;
        compNet.instantiateParams(binding);
    }

    @Override
    public NetPage getComposedNet() {
        return compNet;
    }

    @Override
    public boolean hasSubnets() {
        return true;
    }
    
    //==========================================================================

    @Override
    protected final boolean checkPageCorrectness(boolean isNewOrModified, ProjectData proj, 
                                                 Set<ProjectPage> changedPages, ProjectPage invokerPage) 
    {
        clearPageErrorsAndWarnings();
        boolean doCompose = isNewOrModified;
        boolean dependenciesAreOk = true;
        ProjectPage theInvokerPage = (invokerPage == null ? this : invokerPage);

        ParserContext context = new ParserContext(this);
        
        // Save existing binding expressions (if any)
        if (compNet != null) {
            for (Node node : compNet.nodes)
                if (node instanceof TemplateVariable) {
                    TemplateVariable tvar = (TemplateVariable)node;
                    lastBindingExprs.put(node.getUniqueName(), 
                                         tvar.getLastBindingExpr().getExpr());
//                    System.out.println("## "+tvar.getUniqueName()+" = "+lastBindingExprs.get(tvar.getUniqueName()));
                }
        }
        
        
        // Rebuild transient references to nets (by-name lookup)
//        areReplicaCountParametric = false;
        for (NetInstanceDescriptor descr : netsDescr) {
            descr.paramRefs = new TreeMap<>();
            if (descr.targetNetName == null)
                dependenciesAreOk = false; // Possible??
            else {
                ProjectPage namedPage = proj.findPageByName(descr.targetNetName);
                if (canComposeWith(namedPage)) {
                    // Validate the referenced page
                    namedPage.checkPage(proj, changedPages, theInvokerPage, descr);
                    if (!namedPage.isPageCorrect()) {
                        dependenciesAreOk = false;
                        addPageError("Page \""+namedPage.getPageName()+"\" contains errors. "
                                + "Correct these errors before doing any measure computation.", null);
                    }
                    else {
                        descr.net = (ComposableNet)namedPage;
                        if (changedPages != null)
                            doCompose = doCompose || changedPages.contains(namedPage);
                        
                        // Keep in synch the template variables binding list
                        if (requireParamBinding()) {
                            Set<TemplateVariable> pageParams = descr.net.enumerateParamsForNetComposition();
    //                        Set<TemplateVariable> pageParams = descr.net.enumerateParams();
                            for (TemplateVariable var : pageParams) {
    //                            if (!descr.isParamKnown(var.getUniqueName()))
    //                                descr.unbindParam(var.getUniqueName());
                                if (!descr.isParamBound(var.getUniqueName()))
                                    descr.bindParam(var.getUniqueName(), var.getLastBindingExpr());
                            }
                            descr.removeMissingParams(pageParams);

                            // Copy latex string of template variables
//                            descr.paramRefs = new TreeMap<>();
                            for (TemplateVariable tvar : pageParams)
                                descr.paramRefs.put(tvar.getUniqueName(), tvar);

                            descr.checkBindingCorrectness(this, context);
                        }
                        
//                        System.out.println("pageParams="+pageParams.size()+
//                                " bound="+descr.instParams.binding.size());
////                                " unbound="+descr.unboundParams.size());
                    }
                    // Validate the replica count
                    descr.numReplicas.checkExprCorrectness(context, this, descr);
                }
                else {
                    addPageError("\""+descr.targetNetName+"\" is not a valid composable net name.", descr);
                    dependenciesAreOk = false;
                }
            }
        }
        if (netsDescr.isEmpty())
            addPageError("No Petri nets in composition. Add some net.", null);
        if (hasFixedNumOfOperators() && getFixedNumOfOperators() != netsDescr.size())
            addPageError("Wrong number of net operands.", null);
        
        // Check the additional fields of the derived classes
        checkPageFieldsCorrectness(isNewOrModified, dependenciesAreOk, proj);

        if (!dependenciesAreOk) {
            for (NetInstanceDescriptor descr : netsDescr)
                descr.net = null;
            resetCompositionTargets();
        }
        else if (compNet==null || doCompose) {
            resetCompositionTargets();
            if (isPageCorrect()) {
                System.out.println("REBUILDING "+getPageName());
                ParserContext rootContext = new ParserContext(this);
                compose(rootContext);
                // Since we are rebuilding the composed net, mark this page as changed
                if (changedPages != null)
                    changedPages.add(this);
                
                if (compNet == null)
                    addPageError("Composition failed.", null); // should not happen
                else {
                    // Now check that the composed net is actually a valid net
                    compNet.preparePageCheck();
                    compNet.checkPage(null, null, compNet, null);
//                    compNet.addPageError("PROVA", null);
                    if (!compNet.isPageCorrect()) {
                        // Transfer the errors of the generated net into the multinet
                        for (int i=0; i<compNet.getNumErrorsAndWarnings(); i++) {
                            PageErrorWarning pew = compNet.getErrorWarning(i);
                            if (pew.isWarning())
                                addPageWarning("Composed net: "+pew.getDescription(), null);
                            else
                                addPageError("Composed net: "+pew.getDescription(), null);
                        }
                    }
                    else {
                        // Restore expression bindings
                        for (Node node : compNet.nodes)
                            if (node instanceof TemplateVariable)
                                if (lastBindingExprs.containsKey(node.getUniqueName()))
                                    ((TemplateVariable)node).getLastBindingExpr()
                                            .setExpr(lastBindingExprs.get(node.getUniqueName()));
                    }
                }
            }
        }
//        else {
//            System.out.println("Keeping net composition of "+getPageName());
//        }
        
        return true;
    }
    
    abstract protected void checkPageFieldsCorrectness(boolean isNewOrModified, boolean dependenciesAreOk, ProjectData proj);
    
    
    // Apply parameter substitution to a netpage
    public static void substituteParameters(NetPage page, TemplateBinding binding) {
        for (int n=0; n<page.nodes.size(); n++) {
            Node node = page.nodes.get(n);
            if (node instanceof TemplateVariable) {
                TemplateVariable tvar = (TemplateVariable)node;
                // Check if we want to instantiate this parameter
                if (binding.binding.containsKey(node.getUniqueName())) {
                    ConstantID con = new ConstantID(tvar);
                    String value = binding.getSingleValueBoundTo(tvar).getExpr();
                    if (value.isEmpty())
                        value = "???";
                    con.getConstantExpr().setExpr(value);
                    page.nodes.set(n, con);
                }
            }
        }
    }

    @Override
    public void onAnotherPageRenaming(String oldName, String newName) {
        // Keep page names in sync
        for (NetInstanceDescriptor descr : netsDescr)
            if (descr != null && descr.targetNetName.equals(oldName))
                descr.targetNetName = newName;
    }

    @Override
    public void onAnotherPageNodeRenaming(ProjectPage otherPage, String oldId, String newId) {
        // Keep template variable names in synch
        for (NetInstanceDescriptor descr : netsDescr) {
            if (descr != null && descr.targetNetName.equals(otherPage.getPageName())) {
                if (descr.isParamBound(oldId)) {
                    System.out.println("onAnotherPageNodeRenaming "+otherPage.getPageName()+" "+oldId+"->"+newId);
                    descr.renameBoundParam(oldId, newId);
                }
            }
        }
    }
    
    private static final DataFlavor dataFlavour = new DataFlavor(MultiNetPage.class, "MultiNetDef");
    @Override public DataFlavor getDataFlavour() { return dataFlavour; }

    @Override public boolean hasEditableName() { return true; }

    @Override public boolean hasClearMeasureCmd() { return false; }
    @Override public boolean canClearMeasure(File projectFile) {
        throw new UnsupportedOperationException(); 
    }
    @Override public String clearMeasures(File projectFile, Component wnd) {
        throw new UnsupportedOperationException();
    }

    // Only its composed nets can be unfolded.
    @Override public boolean canBeUnfolded() { return false; }
    @Override public boolean pageSupportsUnfolding() { return false; }
    @Override public boolean pageSupportsAlgebraTool() { return false; }

    
    @Override
    public boolean pageSupportsRG(RgType rgType) {
        if (!isPageCorrect())
            return false;
        return getComposedNet().pageSupportsRG(rgType);
    }

    @Override
    public boolean canBuildRG(RgType rgType) {
        if (!isPageCorrect())
            return false;
        return getComposedNet().canBuildRG(rgType);
    }

    @Override
    public boolean hasRapidMesures() { return true; }
    @Override
    public boolean pageSupportsRapidMeasure(RapidMeasureCmd rmc) {
        if (!isPageCorrect())
            return false;
        return getComposedNet().pageSupportsRapidMeasure(rmc);
    }
    @Override
    public boolean canDoRapidMeasure(RapidMeasureCmd rmc) {
        if (!isPageCorrect())
            return false;
        return getComposedNet().canDoRapidMeasure(rmc);
    }
    
    
    

    
    @Override public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable) {
    }
    @Override public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable) {
    }
    
    //============================================
    
    @Override
    public Class getEditorClass() {
        return MultiNetEditorPanel.class;
    }

    @Override
    public boolean hasPlaceTransInv() {
        if (!isPageCorrect())
            return false;
        return getComposedNet().hasPlaceTransInv();
    }

    // ======== Print support ==================================
    @Override
    public boolean canPrint() {
        return isPageCorrect();
    }

    @Override
    public void print(Graphics2D g, PageFormat pf) {
        getComposedNet().print(g, pf);
    }

    // ======== Cut/Copy/Paste support =========================
    @Override
    public boolean canCutOrCopy() {
        if (hasFixedNumOfOperators())
            return false;
        for (NetInstanceDescriptor descr : netsDescr)
            if (descr.isSelected())
                return true;
        return false;
    }
    
    private static class DescriptorsInClipboard implements Serializable {
        public ArrayList<NetInstanceDescriptor> copiedDescr = new ArrayList<>();
    }

    @Override
    public Object copyData() {
        DescriptorsInClipboard dc = new DescriptorsInClipboard();
        for (NetInstanceDescriptor descr : netsDescr)
            if (descr.isSelected())
                dc.copiedDescr.add((NetInstanceDescriptor)Util.deepCopy(descr));
        return dc;
    }

    @Override
    public void eraseCutData(Object data) {
        if (hasFixedNumOfOperators())
            return;
        Iterator<NetInstanceDescriptor> nidIt = netsDescr.iterator();
        while (nidIt.hasNext()) {
            NetInstanceDescriptor descr = nidIt.next();
            if (descr.isSelected())
                nidIt.remove();
        }
    }

    @Override
    public void pasteData(Object data) {
        if (hasFixedNumOfOperators())
            return;
        setSelectionFlag(false);
        // Append pasted data to the end
        DescriptorsInClipboard dc = (DescriptorsInClipboard)data;
        for (NetInstanceDescriptor descr : dc.copiedDescr)
            netsDescr.add(descr);
    }

    @Override
    public void onSelectErrorWarning(PageErrorWarning errWarn) {
        for (NetInstanceDescriptor descr : netsDescr)
            descr.setSelected(false);
        if (errWarn != null)
            errWarn.selectAffectedElements(true);
    }
    
    @Override
    public void setSelectionFlag(boolean isSelected) {
        for (NetInstanceDescriptor descr : netsDescr)
            descr.setSelected(isSelected);
    }
    
    public int countSelected() {
        if (hasFixedNumOfOperators())
            return 0;
        int numSel = 0;
        for (NetInstanceDescriptor descr : netsDescr)
            if (descr.isSelected())
                numSel++;
        return numSel;
    }
    
    @Override public boolean pageSupportsPlay() { return true; }

    @Override
    public PlayCommand canPlay(ProjectFile project) {
//        return PlayCommand.NO;
        if (!isPageCorrect())
            return PlayCommand.NO;
        return getComposedNet().canPlay(project);
    }

    @Override
    public JPopupMenu getPlayDropdownMenu(ProjectFile project, Action action) {
        if (!isPageCorrect())
            return null;
        return getComposedNet().getPlayDropdownMenu(project, action);
    }

    @Override
    public AbstractPageEditor getPlayWindow(SharedResourceProvider shActProv, JMenuItem menuItem) {
        if (!isPageCorrect())
            return null;
        return getComposedNet().getPlayWindow(shActProv, menuItem);
    }
    
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        Document doc = exDir.getDocument();
        if (exDir.FieldsToXml()) {
            Element bindingListElem = doc.createElement("bindings-list");
            el.appendChild(bindingListElem);
            for (Map.Entry<String, String> e : lastBindingExprs.entrySet()) {
                Element bindElem = doc.createElement("binding");
                bindElem.setAttribute("variable", e.getKey());
                bindElem.setAttribute("value", e.getValue());
                bindingListElem.appendChild(bindElem);
            }
        }
        else {
            NodeList bindingListElemL = el.getElementsByTagName("bindings-list");
            if (bindingListElemL.getLength() > 0) {
                org.w3c.dom.Node bindingsListNode = bindingListElemL.item(0);
                if (bindingsListNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                    Element bindingListElem = (Element)bindingsListNode;

                    NodeList bindElemList = bindingListElem.getElementsByTagName("binding");
                    for (int i=0; i<bindElemList.getLength(); i++) {
                        org.w3c.dom.Node bindElemNode = bindElemList.item(i);
                        if (bindElemNode.getNodeType() == org.w3c.dom.Node.ELEMENT_NODE) {
                            Element bindElem = (Element)bindElemNode;
                            String varName = bindElem.getAttribute("variable");
                            String value = bindElem.getAttribute("value");
                            if (varName != null && value != null) {
                                lastBindingExprs.put(varName, value);
                            }
                        }
                    }
                }
            }
        }
    }
}
