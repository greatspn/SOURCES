/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.gui.net;

import editor.domain.EditableValue;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import javax.swing.JToggleButton;

/**
 *
 * @author elvio
 */
public class ToggleButtonForEditable extends JToggleButton {
    
    private EditableValue editable;
//    private NetPage page;
//    private ProjectData project;
    
    // A true toggle button may either set or unset its property.
    // A radio toggle button may only set the property, it cannot unset it.
    private boolean radioToggle = false;
    
    public EditableValue getEditableProxy() {
        return editable;
    }
    
    public static final Object INDETERMINATE = new Object();
    
    public void setEditableProxy(ProjectData proj, NetPage page, boolean condition, EditableValue editable) {
        if (!condition)
            editable = null;
//        this.project = proj;
//        this.page = page;
        this.editable = editable;
        if (editable == null || !editable.isEditable()) {
            setEnabled(false);
            setSelected(false);
        }
        else {
            setEnabled(true);
            Object value = editable.getValue();
            if (value == INDETERMINATE)
                setSelected(true);
            else
                setSelected(editable.getValue().equals(Boolean.TRUE));
        }
    }

    public boolean isRadioToggle() {
        return radioToggle;
    }

    public void setRadioToggle(boolean radioToggle) {
        this.radioToggle = radioToggle;
    }
}
