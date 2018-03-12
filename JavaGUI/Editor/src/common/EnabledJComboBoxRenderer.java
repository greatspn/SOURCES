
package common;

import java.awt.Color;
import java.awt.Component;
import javax.swing.JList;
import javax.swing.ListSelectionModel;
import javax.swing.UIManager;
import javax.swing.plaf.basic.BasicComboBoxRenderer;

/**
 *
 * Class that can be used as a renderer for a JComboBox which enables/disables
 *
 * items based upon a ListSelectionModel that specifies which items are enabled.  *
 * @author Greg Cope
 *
 *
 *
 */

public class EnabledJComboBoxRenderer<T> extends BasicComboBoxRenderer {
    static final long serialVersionUID = -984932432414L;
    private final ListSelectionModel enabledItems;
    private Color disabledColor = Color.lightGray;

    /**
     *
     * Constructs a new renderer for a JComboBox which enables/disables items
     *
     * based upon the parameter model.
     *
     * @param enabled
     *
     */
    public EnabledJComboBoxRenderer(ListSelectionModel enabled) {
        super();
        this.enabledItems = enabled;

    }

    /**
     *
     * Sets the color to render disabled items.      *
     * @param disabledColor
     *
     */
    public void setDisabledColor(Color disabledColor) {
        this.disabledColor = disabledColor;
    }

    /**
     *
     * Custom implementation to color items as enabled or disabled.      *
     */
    @Override

    public Component getListCellRendererComponent(JList list,
            Object value,
            int index,
            boolean isSelected,
            boolean cellHasFocus) {

        Component c = super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
        if (!enabledItems.isSelectedIndex(index)) {//not enabled
            if (isSelected) {
                c.setBackground(UIManager.getColor("ComboBox.background"));
            } else {
                c.setBackground(super.getBackground());
            }
            c.setForeground(disabledColor);
        } else {
            c.setBackground(super.getBackground());
            c.setForeground(super.getForeground());
        }
        return c;
    }
}
