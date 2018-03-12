/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.gui.net;

import editor.domain.EditableValue;
import javax.swing.JButton;

/**
 *
 * @author elvio
 */
public class ButtonForEditable extends JButton {
    
    private EditableValue editable;
    private Object retrievedValue = null;
    
    public EditableValue getEditableProxy() {
        return editable;
    }
    
    public static final Object INDETERMINATE = new Object();
    
    public void setEditableProxy(boolean condition, EditableValue editable) {
        if (!condition)
            editable = null;
        this.editable = editable;
        if (editable == null || !editable.isEditable()) {
            setEnabled(false);
        }
        else {
            setEnabled(true);
//            Object value = editable.getValue();
//            if (value == INDETERMINATE)
//                setSelected(true);
//            else
//                setSelected(editable.getValue().equals(Boolean.TRUE));
        }
    }

    public Object getRetrievedValue() {
        return retrievedValue;
    }

    public void setRetrievedValue(Object retrievedValue) {
        this.retrievedValue = retrievedValue;
    }
}
