/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.BoxLayout;
import javax.swing.JPopupMenu;
import javax.swing.SwingConstants;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

/**
 *
 * @author elvio
 */
public class JDropdownToolbarButton extends JToolbarButton {
    
    private JPopupMenu popup;
    
    private int horizontalPopupDirection = SwingConstants.LEFT; // LEFT or RIGHT
    
    private long lastViewProfilesPopupClosed = -1;
    private final long TIME_BETWEEN_POPUP_REOPEN = 300;
    private final PopupMenuListener menuListener = new PopupMenuListener() {
        @Override public void popupMenuWillBecomeVisible(PopupMenuEvent e) { }
        @Override public void popupMenuWillBecomeInvisible(PopupMenuEvent e) { 
            lastViewProfilesPopupClosed = System.currentTimeMillis();
        }
        @Override public void popupMenuCanceled(PopupMenuEvent e) {
            lastViewProfilesPopupClosed = System.currentTimeMillis();
        }
    };
    
    public JPopupMenu getPopupMenu() {
        return popup;
    }

    public void setPopupMenu(JPopupMenu popup) {
        if (this.popup != null)
            this.popup.removePopupMenuListener(menuListener);
        this.popup = popup;
        popup.addPopupMenuListener(menuListener);
    }

    public JDropdownToolbarButton() {
        popup = new JPopupMenu();
        popup.setLayout(new BoxLayout(popup, BoxLayout.PAGE_AXIS));
        popup.add("Empty menu");
        
        // Drop down action
        addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent ae) {
                if (popup != null) {
                    // Avoid immediate reopening
                    if (System.currentTimeMillis() - lastViewProfilesPopupClosed < TIME_BETWEEN_POPUP_REOPEN)
                        return;
                    
                    Util.reformatMenuPanels(popup);
                    int x;
                    if (horizontalPopupDirection == SwingConstants.LEFT)
                        x = 0;
                    else
                        x = getWidth() - popup.getWidth();
                    popup.show(JDropdownToolbarButton.this, x, getHeight());
                }
            }
        });
    }

    public int getHorizontalPopupDirection() {
        return horizontalPopupDirection;
    }

    public void setHorizontalPopupDirection(int horizontalPopupDirection) {
        this.horizontalPopupDirection = horizontalPopupDirection;
    }
}
