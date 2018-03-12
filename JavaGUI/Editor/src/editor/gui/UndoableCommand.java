/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import editor.domain.ProjectData;
import editor.domain.ProjectPage;

/** Base interface of all the command records that form the Undo/redo chain.
 *
 * @author elvio
 */
public abstract class UndoableCommand {
    
    // Execute the command on the actual project/page pair
    public abstract void Execute(ProjectData proj, ProjectPage page) throws Exception;
}
