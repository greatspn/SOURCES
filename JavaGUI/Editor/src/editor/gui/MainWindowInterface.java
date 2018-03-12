/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import javax.swing.JFrame;

/** This is the common interface exposed by the main window to the editor subwindows.
 *
 * @author Elvio
 */
public interface MainWindowInterface extends SharedResourceProvider {
    
    public void invalidateGUI();
    public boolean isGUIvalid();
    
    public JFrame getWindowFrame();
    
    public void executeUndoableCommand(String descr, UndoableCommand cmd);
    
    public void setStatus(String str, boolean isOk);
    
    public void setSuspended(boolean isSuspended);
    public boolean isSuspended();
    
    public void switchToProjectPage(ProjectFile pf, ProjectPage page, 
                                    AbstractPageEditor modalPanel);
}
