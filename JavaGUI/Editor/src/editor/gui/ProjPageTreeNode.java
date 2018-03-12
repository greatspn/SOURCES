/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import editor.domain.ProjectPage;
import editor.domain.ProjectFile;
import editor.domain.UuidObject;
import editor.gui.ProjPageTreeCellRenderer.UuidString;
import java.util.UUID;
import javax.swing.Icon;
import javax.swing.tree.DefaultMutableTreeNode;

/** Pages of a project, as shown in the project tree.
 * 
 * @author elvio
 */
public final class ProjPageTreeNode extends DefaultMutableTreeNode {
    // Appartiene ad un progetto attivo?
    public boolean isActiveProject;

    public ProjPageTreeNode(boolean isActiveProject, UuidObject o, boolean allowsChildren) {
        super(o, allowsChildren);
        this.isActiveProject = isActiveProject;
    }

    public ProjPageTreeNode(ProjPageTreeNode node) {
        super(node.getEffectiveUserObject(), node.allowsChildren);
        this.isActiveProject = node.isActiveProject;
    }
    
    @Override
    public Object getUserObject() {
        Object nodeObj = super.getUserObject();
        
        if (nodeObj instanceof ProjectFile)
            return ((ProjectFile) nodeObj).getShortFileDescr();
        
        if (nodeObj instanceof ProjectPage) {
            ProjectPage elem = (ProjectPage) nodeObj;
            if (elem.hasEditableName())
                return elem.getPageName();
            return elem.getPageName();
        } 
        if (nodeObj instanceof UuidString)
            return ((UuidString)nodeObj).desc;
        
        return nodeObj;
    }
    
    public Object getEffectiveUserObject() {
        return super.getUserObject();
    }
    
    public Icon getIcon() {
        Object nodeObj = super.getUserObject();
        if (nodeObj instanceof ProjectFile)
            return ((ProjectFile) nodeObj).getProjectIcon();
            
        if (nodeObj instanceof ProjectPage)
            return ((ProjectPage)nodeObj).getPageIcon();
            
        return null;
    }
    
    public boolean isEditable() {
        Object nodeObj = super.getUserObject();
        if (nodeObj instanceof ProjectFile)
            return false;
        if (nodeObj instanceof ProjectPage)
            return ((ProjectPage)nodeObj).hasEditableName();
        return false;
    }

    public UUID getObjectUUID() {
        return ((UuidObject)super.getUserObject()).getObjectUUID();
    }
}
