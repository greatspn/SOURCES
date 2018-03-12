/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.awt.geom.Rectangle2D;
import javax.swing.JComponent;

/** Objects in the graph editor that can be selected
 *
 * @author elvio
 */
public interface Selectable extends HitTestable, PlaceableObject {
    
    // Selection status flag
    public boolean isSelected();
    public void setSelected(boolean isSelected);
    
    // grayed status flag
    public boolean isGrayed();
    public void setGrayed(boolean isGrayed);

    // aura status flag
    public boolean isAuraActive();
    public void setAuraActive(boolean isAuraActive);

    // Movement handles when it is single-selected
    public abstract MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile);
    
    // A locked object can only be selected with single selection.
    public abstract boolean isLocked();
    
    public abstract class BaseSelectable implements Selectable {
        @Override public boolean isGrayed() { throw new UnsupportedOperationException(); }
        @Override public void setGrayed(boolean isGrayed) { throw new UnsupportedOperationException(); }
        @Override public boolean isAuraActive() { throw new UnsupportedOperationException(); }
        @Override public void setAuraActive(boolean isAuraActive) { throw new UnsupportedOperationException(); }
        @Override public MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile) { throw new UnsupportedOperationException(); }
        @Override public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile, boolean includeDecors) { throw new UnsupportedOperationException(); }
        @Override public MovementHandle getCenterHandle(NetPage thisPage) { throw new UnsupportedOperationException(); }
        @Override public EditableCell getCentralEditable(double px, double py) { throw new UnsupportedOperationException(); }
    }
    
    public class DummySelectable extends BaseSelectable {
        boolean isSelected = false;

        @Override public boolean isSelected() { return isSelected; }
        @Override public void setSelected(boolean s) { isSelected = s; }
        @Override public String toString() { return "---"; }
        @Override public boolean isLocked() { return false; }
    }
    
    public class SwingSelectable extends BaseSelectable {
        JComponent component;
        String compName;

        public SwingSelectable(JComponent component, String compName) {
            this.component = component;
            this.compName = compName;
        }
        
        @Override
        public boolean isSelected() {
            return component.hasFocus();
        }

        @Override
        public void setSelected(boolean s) {
            if (s)
                component.requestFocusInWindow();
        }
        @Override public String toString() { return compName==null ? "---" : compName; }
        @Override public boolean isLocked() { return false; }
    }
}
