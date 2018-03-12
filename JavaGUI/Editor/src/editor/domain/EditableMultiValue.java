/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import java.util.ArrayList;

/**
 *
 * @author elvio
 */
public class EditableMultiValue implements EditableValue {
    private final ArrayList<EditableValue> editables = new ArrayList<>();
    private boolean isEditable = true, isCurrentlyValid = true;
    private final Object indeterminateObj;

    public EditableMultiValue(Object indeterminateObj) {
        this.indeterminateObj = indeterminateObj;
    }
    
    public void add(EditableValue ev) {
        editables.add(ev);
        isEditable = isEditable && (ev != null && ev.isEditable());
    }
    
    public int getCount() { return editables.size(); }

    @Override
    public Object getValue() {
        if (!isEditable())
            return null;
        Object value = editables.get(0).getValue();
        for (int i=1; i<editables.size(); i++) {
            Object v2 = editables.get(i).getValue();
            isCurrentlyValid = isCurrentlyValid && editables.get(i).isCurrentValueValid();
            if (v2 == null && value == null)
                value = null;
            else if (value != null && v2 != null && value.equals(v2))
                value = v2;
            else
                value = indeterminateObj;
        }
        return value;
    }

    public Object getIndeterminateObj() {
        return indeterminateObj;
    }

    @Override
    public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
        if (value == null)
            return false;
        for (int i=0; i<editables.size(); i++) {
            if (editables.get(i) == null)
                return false;
            if (!editables.get(i).isValueValid(proj, page, value))
                return false;
        }
        return true;
    }

    @Override
    public void setValue(ProjectData project, ProjectPage page, Object value) {
        for (int i=0; i<editables.size(); i++)
            editables.get(i).setValue(project, page, value);
    }
    
    @Override public boolean isCurrentValueValid() { return isCurrentlyValid; }

    @Override
    public boolean isEditable() {
        return getCount() > 0 && isEditable;
    }
}
