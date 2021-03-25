/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.elements.DtaSignature;
import common.Util;
import editor.domain.NetPage;
import editor.domain.elements.DtaPage;
import editor.domain.elements.GspnPage;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.ProjectResource;
import editor.domain.ProjectResourceRef;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.VarMultiAssignment;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.VarListMultiAssignment;
import editor.domain.composition.ComposableNet;
import editor.gui.AbstractPageEditor;
import editor.gui.RapidMeasureCmd;
import editor.gui.ResourceFactory;
import editor.gui.SharedResourceProvider;
import java.awt.Component;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.datatransfer.DataFlavor;
import java.awt.print.PageFormat;
import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.UUID;
import javax.swing.Action;
import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;

/** A project page containing a list of measures
 *
 * @author elvio
 */
public class MeasurePage extends ProjectPage implements Serializable {
    
    // The list of measures
    public ArrayList<AbstractMeasure> measures = new ArrayList<>();
    
    // Current viewport position
    public Point viewportPos = new Point(0, 0);
    
    // Target GSPN page
    public String targetGspnName;
    public transient GspnPage targetGspn = null;
    
    // Target DTAs that can be used in CSL^TA expressions
    public transient DtaSignature[] knownDtas;
    
    // All dependency pages (target GSPN, DTAs) are correct
    public boolean allDepsAreCorrect = true;
    
    // Current template bindings
    public VarListMultiAssignment varListAssignments = new VarListMultiAssignment();
    
    // Solver used
    public SolverParams solverParams;
    
    // Is a rapid measure page?
    public String rapidMeasureType = "";
    
    // Has a simplified UI?
    public boolean simplifiedUI = false;
    
    // Last computation log.
    public ProjectResourceRef<DocumentResource> lastLog = new ProjectResourceRef<>();
    
    public MeasurePage() {
        solverParams = new GreatSPNSolverParams();
    }

    @Override public Icon getPageIcon() {
        return ResourceFactory.getInstance().getMeasure16();
    }
    @Override public String getPageTypeName() {
        return "MEASURES";
    }

    @Override public Class getEditorClass() { return MeasureEditorPanel.class; }
    

    @Override
    public boolean hasEditableName() { return true; }
        
    public int countSelectedMeasures() {
        int count = 0;
        for (AbstractMeasure m : measures)
            if (m.isSelected())
                count++;
        return count;
    }

    @Override
    public boolean pageSupportsUnfolding() {
        return false;
    }

    @Override
    public boolean canBeUnfolded() {
        throw new UnsupportedOperationException(); 
    }
    
    @Override
    public boolean pageSupportsRG(RgType rgType) {
        return false;
    }

    @Override
    public boolean canBuildRG(RgType rgType) {
        throw new UnsupportedOperationException(); 
    }

    @Override
    public boolean hasRapidMesures() { return false; }
    @Override
    public boolean pageSupportsRapidMeasure(RapidMeasureCmd rmc) {
        throw new UnsupportedOperationException(""); 
    }
    @Override
    public boolean canDoRapidMeasure(RapidMeasureCmd rmc) {
        throw new UnsupportedOperationException(""); 
    }

    // Check if this measure page has been generated from a rapid measure command
    public boolean isRapid() {
        return !rapidMeasureType.isEmpty();
    }
    public RapidMeasureCmd getRapidCmd() {
        for (RapidMeasureCmd rmc : RapidMeasureCmd.values()) {
            if (rmc.name().equals(rapidMeasureType))
                return rmc;
        }
        return null;
    }
    
    @Override
    protected boolean checkPageCorrectness(boolean isNewOrModified, ProjectData proj, 
                                           Set<ProjectPage> changedPages, 
                                           ProjectPage invokerPage)
    {
        clearPageErrorsAndWarnings();
        assert invokerPage == this;
        targetGspn = null;
        knownDtas = null;
        allDepsAreCorrect = true;
        
        // Netpages have already been checked.
        // Find the target GSPN page and verify that is ok
        if (targetGspnName != null) {
            ProjectPage p = proj.findPageByName(targetGspnName);
            if (p != null && p instanceof ComposableNet) {
                NetPage comp = ((ComposableNet)p).getComposedNet();
                if (comp != null && comp instanceof GspnPage)
                    targetGspn = (GspnPage)comp;
            }

//            // Non-existent name - delete it.
//            if (targetGspn == null) 
//                targetGspnName = null;
        }
        
        if (targetGspn == null) {
            addPageError("The list of measures is not associated to a valid GSPN.", null);
            varListAssignments.assigns.clear();
            allDepsAreCorrect = false;
        }
        else {
            // Validate the GSPN page
            targetGspn.checkPage(proj, changedPages, this, null);
            if (!targetGspn.isPageCorrect()) {
                addPageError("Page \""+targetGspnName+"\" contains errors. "
                        + "Correct these errors before doing any measure computation.", null);
                allDepsAreCorrect = false;
            }
            
            // Is the GSPN changed from the last page modification?
            boolean gspnChanged = true, dtaChanged = true;
            if (changedPages != null) {
                gspnChanged = changedPages.contains(targetGspn);
                dtaChanged = false;
            }
            
            // Prepare the template bindings
            Map<String, TemplateVariable> gspnVars = new TreeMap<>();
            for (Node node : targetGspn.nodes) {
                if (node instanceof TemplateVariable) {
                    TemplateVariable tvar = (TemplateVariable)node;
                    gspnVars.put(node.getUniqueName(), tvar);
                    if (null == varListAssignments.getAssignmentFor(node.getUniqueName())) {
                        varListAssignments.assigns.add(VarMultiAssignment.createSingle(tvar.getUniqueName(), tvar.getType(), "", true));
                    }
                }
            }
            // Remove unused bindings
            Iterator<VarMultiAssignment> it;
            it = varListAssignments.assigns.iterator();
            while (it.hasNext()) {
                if (!gspnVars.containsKey(it.next().varName))
                    it.remove();
            }

            // Create the parser context of this page
            ParserContext pageContext = new ParserContext(targetGspn);
            //TODO: pageContext.templateVarsBinding = templateVarsBinding;
            
            // List the known DTA signatures of this project
            ArrayList<DtaSignature> sigList = new ArrayList<>();
            for (int i=0; i<proj.getPageCount(); i++) {
                if (proj.getPageAt(i) instanceof DtaPage) {
                    DtaPage dta = (DtaPage)proj.getPageAt(i);
                    DtaSignature sig = dta.createSignature();                    
                    sigList.add(sig);
                }
            }
            knownDtas = sigList.toArray(new DtaSignature[sigList.size()]);

            // Check bindings
            pageContext.knownDtas = knownDtas;
            it = varListAssignments.assigns.iterator();
            while (it.hasNext()) {
                it.next().checkAssignmentCorrectness(this, pageContext);
            }
            // Check measure correctness
            for (AbstractMeasure m : measures) {
                m.checkMeasureCorrectness(this, pageContext);
            }
            // Check measure uniqueness
            for (FormulaLanguage fl : FormulaLanguage.values()) {
                if (fl.isUniqueInPage()) {
                    int num = 0;
                    for (AbstractMeasure m : measures) {
                        if (m instanceof FormulaMeasure && ((FormulaMeasure)m).getLanguage() == fl) {
                            if (num++ == 1) {
                                addPageError("At most one "+fl.name()+" entry can be present in the table of measures.", m);
                            }
                        }
                    }
                }
            }
            
            // Check for changes in used DTAs
            if (changedPages != null) {
                for (DtaSignature ds : sigList) {
                    if (ds.isUsed && changedPages.contains(ds.dta)) {
                        dtaChanged = true;
                        
                        // Validate the DTA page before continuing
                        ds.dta.checkPage(proj, changedPages, this, null);
                        if (!ds.dta.isPageCorrect()) {
                            addPageError("DTA \""+ds.dta.getPageName()+"\" contains errors. "
                                    + "Correct these errors before doing any measure computation.", null);
                            allDepsAreCorrect = false;
                        }
                    }
                }
            }
            
            // Remove computed results if the target GSPN page or some dependent DTAs have changed
            if (changedPages != null) { // real page changes
                if (gspnChanged || dtaChanged) {
                    System.out.println(getPageName()+": Cleaning computed results. ChangedPages="
                                       +changedPages+", gspnChanged="+gspnChanged+", dtaChanged="+dtaChanged); 
                    for (AbstractMeasure m : measures)
                        m.setResults(null);
                    lastLog.setRef(null);
                }
            }
            
            // Ask the selected solver if the selected GSPN is good for it.
            solverParams.isGspnSupported(targetGspn, this);
        }
        
        return true;
    }

    @Override
    public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable) {
        lastLog.relinkTransientResources(resourceTable);
    }

    @Override
    public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable) {
        lastLog.retrieveLinkedResources(resourceTable);
    }

    @Override
    public void onSelectErrorWarning(PageErrorWarning errWarn) {
        setSelectionFlag(false);
        if (errWarn != null)
            errWarn.selectAffectedElements(true);
    }
    
    @Override
    public void setSelectionFlag(boolean isSelected) {
        for (AbstractMeasure m : measures)
            m.setSelected(isSelected);
    }

    @Override
    public void onAnotherPageRenaming(String oldName, String newName) {
        // Here we have the opportunity to keep in sync with the GSPN name renaming
        if (targetGspnName != null && targetGspnName.equals(oldName)) {
            targetGspnName = newName;
            
            if (isRapid() && getPageName().endsWith(oldName)) {
                String n = getPageName();
                setPageName(n.substring(0, n.length() - oldName.length()) + newName);
            }
        }
    }

    //------------ Cut/Copy/Paste ------------------------
    private static final DataFlavor dataFlavour = new DataFlavor(GspnPage.class, "Measures");
    @Override public DataFlavor getDataFlavour() { return dataFlavour; }

    @Override
    public boolean canCutOrCopy() {
        return countSelectedMeasures() > 0;
    }
    
    private static class MeasuresInClipboard implements Serializable {
        public ArrayList<AbstractMeasure> measures = new ArrayList<>();
    }

    @Override
    public Object copyData() {
        MeasuresInClipboard mic = new MeasuresInClipboard();
        for (AbstractMeasure m : measures)
            if (m.isSelected())
                mic.measures.add((AbstractMeasure)Util.deepCopy(m));
        return mic;
    }

    @Override
    public void eraseCutData(Object data) {
        Iterator<AbstractMeasure> mit = measures.iterator();
        while (mit.hasNext()) {
            AbstractMeasure m = mit.next();
            if (m.isSelected())
                mit.remove();
        }
    }

    @Override
    public void pasteData(Object data) {
        setSelectionFlag(false);
        // Append pasted data to the end
        MeasuresInClipboard mic = (MeasuresInClipboard)data;
        for (AbstractMeasure m : mic.measures)
            measures.add(m);
    }
    
    //-------------Clear measure cache interface--------------------
    @Override
    public boolean hasClearMeasureCmd() {
        return true;
    }
    @Override
    public boolean canClearMeasure(File projectFile) {
        return projectFile!=null && 
               getSolutionDirectoryName(projectFile).isDirectory();
    }
    @Override
    public String clearMeasures(File projectFile, Component wnd) {
        assert projectFile!=null;
        File dir = getSolutionDirectoryName(projectFile);
        String err = null;
        if (dir.isDirectory()) {
            // Delete directory content
            File[] fList = dir.listFiles();
            for (File f : fList)
                f.delete();
        }
        else err = dir.getAbsolutePath()+" is not a directory.";
        
        if (!dir.delete())
            err = "Could not delete directory "+dir;
        
        if (err == null)
            JOptionPane.showMessageDialog(wnd, "Cache directory successfully cleared.\n"+dir, "Clear cache", 
                                          JOptionPane.INFORMATION_MESSAGE, 
                                          ResourceFactory.getInstance().getBroom32());
        else
            JOptionPane.showMessageDialog(wnd, "Could not clear cache directory:\n"+err, "Clear cache",
                                          JOptionPane.ERROR_MESSAGE);
        return err;
    }
    
    // Name of the directory that will store all the intermediate files for the invoked solvers
    protected File getSolutionDirectoryName(File projectFile) {
        String pnproFile = projectFile.getAbsolutePath();
        if (pnproFile.endsWith(ProjectFile.PNPRO_EXT))
            pnproFile = pnproFile.substring(0, pnproFile.length() - ProjectFile.PNPRO_EXT.length() - 1);
        // directory specific to this measure page
        return new File(pnproFile + "-" + getPageName() + ".solution");
    }

    //---------------------Print support----------------------------    
    @Override
    public boolean canPrint() {
        return false;
    }

    @Override
    public void print(Graphics2D g, PageFormat pf) {
        throw new UnsupportedOperationException("Not supported yet."); 
    }

    //------------------Unsupported features------------------------
    @Override public boolean pageSupportsPlay() { return false; }

    @Override
    public PlayCommand canPlay(ProjectFile project) { return PlayCommand.NO; }

    @Override
    public JPopupMenu getPlayDropdownMenu(ProjectFile project, Action action) {
        throw new UnsupportedOperationException("Not supported."); 
    }

    @Override
    public AbstractPageEditor getPlayWindow(SharedResourceProvider shActProv, JMenuItem menuItem) {
        throw new UnsupportedOperationException("Not supported.");
    }

    @Override
    public boolean hasPlaceTransInv() { return false; }

    @Override
    public boolean hasNetMatrices() { return false; }
    
    @Override public boolean pageSupportsAlgebraTool() { return false; }
}
