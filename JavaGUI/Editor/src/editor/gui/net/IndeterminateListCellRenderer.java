/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.Util;
import editor.domain.ListRenderable;
import java.awt.Component;
import java.awt.Font;
import javax.swing.DefaultListCellRenderer;
import javax.swing.Icon;
import javax.swing.JLabel;
import javax.swing.JList;

/**
 * A list cell renderer with icons and indeterminate state.
 *
 * @author elvio
 */
public class IndeterminateListCellRenderer extends DefaultListCellRenderer {
    
    private final Font defaultFont;
    
    public IndeterminateListCellRenderer() {
        defaultFont = getFont();
    }
    
    @Override
    public Component getListCellRendererComponent(JList list, Object value, int index,
                                                  boolean isSelected, boolean cellHasFocus) {
        Component c = super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
        JLabel label = (JLabel) c;
        final Icon emptyIcon = null; // Main.EMPTY16_ICON
        
        if (value instanceof ListRenderable) {
            ListRenderable lr = (ListRenderable) value;
            label.setText(lr.getDescription());
            Icon icon = lr.getIcon16();
            label.setIcon(icon);
            label.setDisabledIcon(Util.getGrayedIcon(icon));
            int size = lr.getTextSize();
            if (size > 0) {
                label.setFont(defaultFont.deriveFont(Font.PLAIN, size));
            }
            else label.setFont(defaultFont);
        } 
        else if (value != null && value instanceof Object) {
            label.setText(value.toString());
            label.setIcon(emptyIcon);
            label.setDisabledIcon(emptyIcon);
            label.setFont(defaultFont);
        }
        else {
            label.setText("---");
            label.setIcon(emptyIcon);
            label.setDisabledIcon(emptyIcon);
            label.setFont(defaultFont);
        }
        return label;
    }
}
