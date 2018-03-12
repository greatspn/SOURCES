/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.Main;
import editor.gui.AbstractPageEditor;
import editor.gui.RapidMeasureCmd;
import editor.gui.SharedResourceProvider;
import java.awt.Component;
import java.awt.Graphics2D;
import java.awt.datatransfer.DataFlavor;
import java.awt.print.PageFormat;
import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Set;
import java.util.UUID;
import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

/** A page of a project. 
 * Project pages are shown in the page tree, in the main window.
 * 
 * @author Elvio
 */
public abstract class ProjectPage implements Serializable, UuidObject, ResourceHolder {
    
    // The visible name of this page
    private String pageName = null;
    
    // The list of errors and warnings found in this page by the checkPageCorrectness() method
    private final ArrayList<PageErrorWarning> errorsAndWarnings = new ArrayList<>();
    // The number of effective errors in errorsAndWarnings (excluding the warnings)
    private int numRealErrors = 0;

    private enum CheckingFlag {
        CHECKED,      // No checking needed (call preparePageCheck() to ask for check)
        RECHECKING,   // Doing recheck phase (inside a checkPage() call, to avoid circular calls)
        NEED_RECHECK  // Invalid, needs a call to checkPage()
    }
    private CheckingFlag chFlag = CheckingFlag.CHECKED;
    
    // Page version number, changes at every page modification
    private transient UniqueVersionID versionId = new UniqueVersionID();
    public UniqueVersionID getVersionId() {
        return versionId;
    }
    public void setVersionId(UniqueVersionID versionId) {
        this.versionId = versionId;
    }
    
    
    public ProjectPage() { }

    // Page type description
    public abstract Icon getPageIcon();
    public abstract String getPageTypeName();
    public abstract boolean hasEditableName();
    public abstract Class getEditorClass();
    public abstract DataFlavor getDataFlavour();

    public String getPageName() {
        return pageName;
    }

    public void setPageName(String pageName) {
        this.pageName = pageName;
    }
    
    // called when another page in the project has been renamed
    public void onAnotherPageRenaming(String oldName, String newName) { }
    
    // Syntax/semantic analysis of the page content
    public final boolean isPageCorrect() { return numRealErrors == 0; }
    
    // Rebuild the error/warning table of this page
    // returns true if the table has been rebuilt, false otherwise.
    // The invoker page is not null if page correctness checking has been asked by another page
    // instead of being asked by the main ProjectData' checkPage() loop
    protected abstract boolean checkPageCorrectness(boolean isNewOrModified, ProjectData proj, 
                                                    Set<ProjectPage> changedPages, 
                                                    ProjectPage invokerPage);
        
    public final void preparePageCheck() {
        assert chFlag == CheckingFlag.CHECKED;
        chFlag = CheckingFlag.NEED_RECHECK;
    }
    
    public final boolean checkPage(ProjectData proj, Set<ProjectPage> changedPages, 
                                   ProjectPage invokerPage, Selectable invokerSelectable) 
    {
        // pageName must have been specified at page creation using setPageName().
        assert pageName != null;
        
        if (chFlag == CheckingFlag.CHECKED) {
            return false; // Already checked
        }
        if (chFlag == CheckingFlag.RECHECKING) {
            // This method has been called recursively due to 
            // some circular dependency between pages. Add an error to the invoker page.
            if (invokerPage != null)
                invokerPage.addPageError("Circular dependency between pages are not allowed. "+
                                         "Dependency found with page \""+getPageName()+"\".", 
                                         invokerSelectable);
            else
                throw new IllegalStateException("Circular dependency in page checking.");
            return false;
        }
        try {
            chFlag = CheckingFlag.RECHECKING;
            // Call the abstract validation method
            boolean isNewOrModified = (changedPages == null || changedPages.contains(this));
            checkPageCorrectness(isNewOrModified, proj, changedPages, invokerPage);
        }
        catch (Exception e) { 
            Main.logException(e, true);
            addPageError("Internal error (Java exception raised in page validation phase).", null);
        }
        chFlag = CheckingFlag.CHECKED;
        return true;
    }
    
    protected void clearPageErrorsAndWarnings() { numRealErrors=0; errorsAndWarnings.clear(); }
    protected void addErrorWarningObject(PageErrorWarning err) { 
        errorsAndWarnings.add(err);
        numRealErrors += (err.isWarning ? 0 : 1);
    }
    public void addPageError(String descr, Selectable errObj) { 
        addErrorWarningObject(PageErrorWarning.newError(descr, errObj));
    }
    public void addPageErrorSet(String descr, Set<Selectable> errObjectList) { 
        addErrorWarningObject(PageErrorWarning.newErrorSet(descr, errObjectList));
    }
    public void addPageWarning(String descr, Selectable errObj) { 
        addErrorWarningObject(PageErrorWarning.newWarning(descr, errObj));
    }
    public void addPageWarningSet(String descr, Set<Selectable> errObjectList) { 
        addErrorWarningObject(PageErrorWarning.newWarningSet(descr, errObjectList));
    }
    public int getNumErrorsAndWarnings() { return errorsAndWarnings.size(); }
    public PageErrorWarning getErrorWarning(int i) { return errorsAndWarnings.get(i); }
    public abstract void onSelectErrorWarning(PageErrorWarning errWarn);
    
    public static final PageErrorWarning[] NO_ERRORS = new PageErrorWarning[0];
    public PageErrorWarning[] getAllErrorsAndWarnings() { 
        if (errorsAndWarnings.isEmpty())
            return NO_ERRORS;
        PageErrorWarning[] errs = new PageErrorWarning[errorsAndWarnings.size()];
        for (int i=0; i<errorsAndWarnings.size(); i++)
            errs[i] = errorsAndWarnings.get(i);
        return errs; 
    }
    
    protected final void addErrorWarningList(PageErrorWarning[] err) { 
        for (PageErrorWarning p : err) addErrorWarningObject(p);
    }
    
    // Internal UUID: gets the unique version ID
    private UUID objectUUID = UUID.randomUUID();
    @Override
    public UUID getObjectUUID() {
        return objectUUID;
    }
    public void generateNewUUID() {
        // makes this page different by changing its UUID.
        objectUUID = UUID.randomUUID();
    }
    
    // Cut/copy/paste actions
    public abstract boolean canCutOrCopy();
    public abstract Object copyData();
    public abstract void eraseCutData(Object data);
    public abstract void pasteData(Object data);
    public abstract void setSelectionFlag(boolean isSelected);
    
    // Print actions
    public abstract boolean canPrint();
    public abstract void print(Graphics2D g, PageFormat pf);
    
       
    // Play command interface
    public enum PlayCommand { NO, IMMEDIATE, DROPDOWN }
    public abstract boolean pageSupportsPlay();
    public abstract PlayCommand canPlay(ProjectFile project);
    public abstract JPopupMenu getPlayDropdownMenu(ProjectFile project, javax.swing.Action action);
    public abstract AbstractPageEditor getPlayWindow(SharedResourceProvider shActProv, JMenuItem menuItem);
    
    public abstract boolean hasPlaceTransInv();
    
    // This page class has the unfolding
    public abstract boolean pageSupportsUnfolding();
    // Page data can be unfolded
    public abstract boolean canBeUnfolded();
    
    public enum RgType {
        RG, SRG, CTMC
    }

    // This page support RG construction?
    public abstract boolean pageSupportsRG(RgType rgType);
    // Current page can construct the RG?
    public abstract boolean canBuildRG(RgType rgType);
    
    // we can use the 'algebra' tool on this page
    public abstract boolean pageSupportsAlgebraTool();
    
    // Clear the cache directory where intermediate solver files are stored.
    public abstract boolean hasClearMeasureCmd();
    public abstract boolean canClearMeasure(File projectFile);
    public abstract String clearMeasures(File projectFile, Component wnd); // returns error string or null
    
    // Which rapid measures are supported
    public abstract boolean hasRapidMesures();
    public abstract boolean pageSupportsRapidMeasure(RapidMeasureCmd rmc);
    public abstract boolean canDoRapidMeasure(RapidMeasureCmd rmc);
}
