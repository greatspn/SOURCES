/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.io.Serializable;

/** Implements the Selectable interface
 *
 * @author elvio
 */
public abstract class SelectableObject extends NetObject implements Serializable, Selectable {
    
    private boolean selectionFlag = false;
    private boolean isGrayed = false;
    private boolean isAuraActive = false;
    private int magnetWithAura = -1;
    
    // Cut/Copy flag - used for cut/copy/paste implementation
    private boolean isCopied = false;

    @Override
    public boolean isSelected() {
        return selectionFlag;
    }

    @Override
    public void setSelected(boolean isSelected) {
        this.selectionFlag = isSelected;
    }
    
    // Movement handles when it is single-selected
    @Override
    public abstract MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile);
    
    // Center-point movement handle
    @Override
    public abstract MovementHandle getCenterHandle(NetPage thisPage);

    
    @Override
    public boolean isGrayed() {
        return isGrayed;
    }

    @Override
    public void setGrayed(boolean isGrayed) {
        this.isGrayed = isGrayed;
    }

    @Override
    public boolean isAuraActive() {
        return isAuraActive;
    }

    @Override
    public void setAuraActive(boolean isAuraActive) {
        this.isAuraActive = isAuraActive;
    }

    public int getMagnetWithAura() {
        return magnetWithAura;
    }

    public void setMagnetWithAura(int magnetWithAura) {
        this.magnetWithAura = magnetWithAura;
    }
    
    public boolean isCopied() {
        return isCopied;
    }

    public void setCopied(boolean isCopied) {
        this.isCopied = isCopied;
    }
    
    
}
