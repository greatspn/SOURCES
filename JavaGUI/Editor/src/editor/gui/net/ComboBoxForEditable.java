/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.EditableValue;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import java.awt.Color;
import java.awt.Component;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.text.JTextComponent;

/**
 *
 * @author elvio
 */
public class ComboBoxForEditable<E> extends JComboBox<E> {
    private EditableValue editable;
    private Object lastItemSet;
    private NetPage page;
    private ProjectData project;
    private boolean isValueValid = false;
    private JLabel synchLabel;
    
    private boolean updating = false;
    
    public ComboBoxForEditable() {
        Component editorComp = getEditor().getEditorComponent();
        if (editorComp instanceof JTextField) {
            ((JTextField)editorComp).getDocument().addDocumentListener(new DocumentListener() {
                @Override public void insertUpdate(DocumentEvent de) { onTextChange(); }
                @Override public void removeUpdate(DocumentEvent de) { onTextChange(); }
                @Override public void changedUpdate(DocumentEvent de) { onTextChange(); }
            });
        }
        addFocusListener(new FocusListener() {
            @Override
            public void focusGained(FocusEvent fe) { 
                if (isEditable()) {
                    JTextComponent tc = (JTextComponent)getEditor();
                    tc.select(0, tc.getText().length());
                }
                    
            }

            @Override
            public void focusLost(FocusEvent fe) {
                if (lastItemSet != null)
                    setSelectedItem(lastItemSet);
                else
                    setSelectedIndex(-1);
                onTextChange();
            }
        });
    }

    public boolean isUpdating() {
        return updating;
    }

    public void setUpdating(boolean updating) {
        this.updating = updating;
    }
    
    
    public boolean isValueValid() {
        assert editable != null;
        // BUG: need to update isValueValid here and not in the onTextChange() method,
        // since somethims onTextChange is called after this method (by the editable proxy)
        isValueValid = editable.isValueValid(project, page, getSelectedItem());
        return isValueValid;
    }

    public EditableValue getEditableProxy() {
        return editable;
    }

    public void setEditableProxy(ProjectData proj, NetPage page, boolean condition, EditableValue editable) {
        if (!condition)
            editable = null;
        setUpdating(true);
        this.project = proj;
        this.page = page;
        this.editable = editable;
        if (editable == null || !editable.isEditable()) {
            setEnabled(false);
//            if (editable != null)
//                setSelectedItem()
//            getEditor().setItem(editable == null ? "" : editable.getValue());
            lastItemSet = null;
            setSelectedIndex(-1);
            if (synchLabel != null)
                synchLabel.setEnabled(false);
        }
        else {
            setEnabled(true);
            lastItemSet = editable.getValue();
            if (lastItemSet == null)
                setSelectedIndex(-1);
            else {
                setSelectedItem(lastItemSet);
                if (getSelectedItem()==null || !getSelectedItem().equals(lastItemSet))
                    setSelectedIndex(-1);
            }
            onTextChange();
            if (synchLabel != null)
                synchLabel.setEnabled(true);
        }
        setUpdating(false);
    }
    
    private void onTextChange() {
        if (editable != null) {
            Object obj = getSelectedItem();
            isValueValid = editable.isValueValid(project, page, obj);
            if (isValueValid || (obj == null || obj.equals("---"))) {
                setForeground(Color.BLACK);
            }
            else {
                setForeground(Color.RED);
            }
        }
    }
    
    public JLabel getSynchLabel() {
        return synchLabel;
    }

    public void setSynchLabel(JLabel synchLabel) {
        this.synchLabel = synchLabel;
    }
}
