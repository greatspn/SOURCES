/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import editor.domain.UuidObject;
import java.awt.Color;
import java.awt.Component;
import java.util.UUID;
import javax.swing.Icon;
import javax.swing.JLabel;
import javax.swing.JTree;
import javax.swing.border.EmptyBorder;
import javax.swing.tree.DefaultTreeCellRenderer;

/**
 *
 * @author Elvio
 */
public class ProjPageTreeCellRenderer extends DefaultTreeCellRenderer {
    
    public static class UuidString extends Object implements UuidObject {
        public UuidString(String desc) {
            this.desc = desc;
        }        
        public final String desc;
        private final UUID objectUUID = UUID.randomUUID();

        @Override public UUID getObjectUUID() {
            return objectUUID;
        }
    }

    public static final UuidObject TREE_HEADER = new UuidString("Progetti aperti:");
    //protected boolean selectedFlag;
    
    //Font normalFont, boldFont;
    
    public ProjPageTreeCellRenderer() {
        //normalFont = new JLabel().getFont();
        //boldFont = new Font(normalFont.getName(), Font.BOLD, normalFont.getSize());
        setBorder(new EmptyBorder(1, 1, 1, 1));
    }
    
    @Override
    public Component getTreeCellRendererComponent(JTree tree, Object value,
                                                  boolean selected, boolean expanded,
                                                  boolean leaf, int row, boolean hasFocus) 
    {
        //setFont(boldFont);
        JLabel comp = (JLabel)super.getTreeCellRendererComponent(tree, value, selected, expanded, 
                                                                 leaf, row, hasFocus);
        // Find out which node we are rendering and get its text
        ProjPageTreeNode node = (ProjPageTreeNode) value;
        Object nodeObj = node.getEffectiveUserObject();
        boolean isActive = node.isActiveProject;

        //selectedFlag = selected;
        //comp.setForeground(selected Color.white);
        if (isActive && selected)
            comp.setForeground(Color.white);
        else if (isActive || nodeObj instanceof UuidString)
            comp.setForeground(Color.black);
        else
            comp.setForeground(Color.gray);
        //comp.setFont(selected ? boldFont : normalFont);
        
        Icon icon = node.getIcon();
        setIcon(icon);
        setDisabledIcon(icon);
        setText(node.getUserObject().toString());

        return comp;
    }
}
