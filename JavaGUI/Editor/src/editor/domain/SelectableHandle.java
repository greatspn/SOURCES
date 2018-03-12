/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

/**
 *
 * @author elvio
 */
public abstract class SelectableHandle extends DraggableHandle implements Selectable {

    @Override public abstract boolean isSelected();
    @Override public abstract void setSelected(boolean isSelected);

    boolean isGrayed = false;
    @Override public boolean isGrayed() { return isGrayed; }
    @Override public void setGrayed(boolean isGrayed) { this.isGrayed = isGrayed; }

    boolean isAuraActive = false;
    @Override public boolean isAuraActive() { return isAuraActive; }
    @Override public void setAuraActive(boolean isAuraActive) { this.isAuraActive = isAuraActive; }

//    @Override
//    public boolean intersectRectangle(Rectangle2D rect) {    }

    @Override
    public MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile) 
    { return null; }

    @Override
    public MovementHandle getCenterHandle(NetPage thisPage) { return this; }

    @Override
    public EditableCell getCentralEditable(double px, double py) { return null; }
    
    @Override public boolean isLocked() { return false; }
}
