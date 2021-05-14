/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.Main;
import editor.domain.io.PnProFormat;
import editor.gui.ResourceFactory;
import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Observable;
import java.util.Set;
import java.util.UUID;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/** Base interface of a project. 
 * A project is described as a set of pages, stored in a ProjectData object.
 * This class contains the set of pages, the filename (if any), and the undo/redo history.
 *
 * @author Elvio
 */
public class ProjectFile extends Observable implements Serializable, UuidObject {
    
    // version structure:
    //   undo[n] ... undo[0] lastSnapshot redo[0] ... redo[m]

    // Associated filename (or null)
    private File filename;    
    // Undo/Redo chain with all the history of changes
    private final ArrayList<ProjectSnapshot> undoChain = new ArrayList<>();
    private final ArrayList<ProjectSnapshot> redoChain = new ArrayList<>();
    // version of the last saved data (a project starts in saved state)
    private UniqueVersionID savedVersionID = null;
//    private UniqueVersionID versionID = new UniqueVersionID();
    // Current document instance
    private ProjectData current;
    // Current snapshot
    private ProjectSnapshot lastSnapshot;

    // Snapshot before modifications
    private ProjectSnapshot beforeChanges = null;
    
    public ProjectFile(ProjectData startData) {
        current = startData;
        lastSnapshot = startData.createSnapshot(null);
        savedVersionID = lastSnapshot.getDataVersionID();
    }
    
    // Current version in the undo/redo chain
    public final ProjectData getCurrent() {
        return current;
    }
    
    public Icon getProjectIcon() {
        return ResourceFactory.getInstance().getPnproj16();
    }
    
    // Short description that appears on the project tree in the application window
    public String getShortFileDescr() {
        if (filename == null) // Not saved on disk
            return "<html>" + getCurrent().projName + " [<i>new</i>]";
        String fname = filename.getName();
        if (fname.endsWith(PNPRO_EXT) && fname.length() > PNPRO_EXT.length() + 1)
            fname = fname.substring(0, fname.length() - PNPRO_EXT.length() - 1);
        if (!isSaved())
            return "<html>" + fname + " [<i>unsaved</i>]";
        return fname;
    }
    // Description that appears on the application window header
    public String getLongFileDescr() {
        if (filename == null) // Not saved on disk
            return getCurrent().projName + " [new]";
        String fname = filename.getAbsolutePath();
        if (!isSaved())
            return fname + " [unsaved]";
        return fname;
    }
    public String getProjectName() {
        if (filename == null) // Not saved on disk - use string name
            return getCurrent().projName;
        String fname = filename.getName(); // Use filename as project name
        if (fname.endsWith(PNPRO_EXT) && fname.length() > PNPRO_EXT.length() + 1)
            fname = fname.substring(0, fname.length() - PNPRO_EXT.length() - 1);
        return fname;
    }
    
    public static final String PNPRO_EXT = "PNPRO";
    private static final FileFilter fileFilter 
            = new FileNameExtensionFilter("Project (*.PNPRO)", new String[]{ PNPRO_EXT });
    public static FileFilter getFileFilter() {
        return fileFilter;
    }
    
    private static final Set<ProjectPage> NO_PAGE_CHANGED = new HashSet<>();
    public void startChanges() {
        if (beforeChanges != null) {
            System.out.println("Warning: beforeChanges != null.");
        }
        beforeChanges = current.createSnapshot(undoChain.size() > 0 ? undoChain.get(0) : lastSnapshot);
    }
    
    public void rollbackChanges() {
        assert beforeChanges != null;
        current = new ProjectData(beforeChanges);
        beforeChanges = null;
        
        getCurrent().checkProjectCorrectness(NO_PAGE_CHANGED);
        setChanged();
        notifyObservers(this);
    }
    
    public void commitChanges(String actionDescr) {
        assert beforeChanges != null;
        
        // delete redo history
        redoChain.clear();

        // delete the first entry in the history chain if it is too long
        final int MAX_UNDO = Main.getMaxUndo();
        while (undoChain.size() > MAX_UNDO) {
            undoChain.remove(undoChain.size() - 1);
        }
        
        current.changeVersionId();
        lastSnapshot = beforeChanges;
        lastSnapshot.setActionName(actionDescr);
        undoChain.add(0, lastSnapshot);
        lastSnapshot = current.createSnapshot(lastSnapshot);
        beforeChanges = null;
        
        // Update page-dependent data on the basis of page changes.
        Set<ProjectPage> changedPages = new HashSet<>();
//        System.out.println("");
        for (int p=0; p<current.getPageCount(); p++) {
            ProjectPage page = current.getPageAt(p);
            if (lastSnapshot.isPageChanged(p))
                changedPages.add(page);
//            System.out.println("Page "+page.getPageName()+" is "+
//                    (lastSnapshot.isPageChanged(p) ? "CHANGED." : "unchanged."));
        }
        
        getCurrent().checkProjectCorrectness(changedPages);
        setChanged();
        notifyObservers(this);
    }
    
    public void undo() {
        assert canUndo() && beforeChanges == null;
        redoChain.add(0, lastSnapshot);
        lastSnapshot = undoChain.get(0);
        undoChain.remove(0);
        current = new ProjectData(lastSnapshot);
//        assert lastSnapshot.getDataVersionID() == versionID - 1;
//        versionID--;
        
        getCurrent().checkProjectCorrectness(null /*recheck everything*/);
        setChanged();
        notifyObservers(this);        
    }
    
    public void redo() {
        assert canRedo() && beforeChanges == null;
        undoChain.add(0, lastSnapshot);
        lastSnapshot = redoChain.get(0);
        redoChain.remove(0);
        current = new ProjectData(lastSnapshot);
//        assert lastSnapshot.getDataVersionID() == versionID + 1;
//        versionID++;
        
        getCurrent().checkProjectCorrectness(null /*recheck everything*/);
        setChanged();
        notifyObservers(this);
    }
    
    public boolean canUndo() {
        return undoChain.size() > 0;
    }
    
    public boolean canRedo() {
        return redoChain.size() > 0;
    }
    
    public String getUndoDesc() {
        assert canUndo();
        return undoChain.get(0).getActionName();
    }
    public String getRedoDesc() {
        assert canRedo();
        return lastSnapshot.getActionName();
    }

    public void save() throws Exception {
        // Implement save
        assert getFilename() != null;

        PnProFormat.saveXML(getCurrent(), getFilename());
        System.out.println("saved! "+getFilename());
      
        setSaved();
    }
    
    public boolean isSaved() {
        return filename != null && savedVersionID == lastSnapshot.getDataVersionID();
    }
    
    private final UUID objectUUID = UUID.randomUUID();
    @Override
    public UUID getObjectUUID() {
        return objectUUID;
    }

    public File getFilename() {
        return filename;
    }

    public void setFilename(File filename) {
        this.filename = filename;
    }

    protected void setSaved() {
        savedVersionID = lastSnapshot.getDataVersionID();
    }
    
    public String generateUniquePageName(String prefix) {
        return getCurrent().generateUniquePageName(prefix);
    }
}
