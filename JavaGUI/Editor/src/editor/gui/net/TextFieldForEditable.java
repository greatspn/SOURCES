/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.EditableValue;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

/**
 *
 * @author elvio
 */
public class TextFieldForEditable extends JTextField {
    
    private EditableValue editable;
    private String lastTextSet;
    private NetPage page;
    private ProjectData project;
    private boolean isValueValid;
    private JLabel synchLabel;
    private String textAtFocusGain = null;
    

    public TextFieldForEditable() {
        getDocument().addDocumentListener(new DocumentListener() {
            @Override public void insertUpdate(DocumentEvent de) { onTextChange(); }
            @Override public void removeUpdate(DocumentEvent de) { onTextChange(); }
            @Override public void changedUpdate(DocumentEvent de) { onTextChange(); }
        });
        addFocusListener(new FocusListener() {
            @Override
            public void focusGained(FocusEvent fe) { 
                select(0, getText().length());
                textAtFocusGain = getText();
                common.Action.addGlobalActionListener(globalChangeAction);
            }

            @Override
            public void focusLost(FocusEvent fe) {
//                setText(lastTextSet != null ? lastTextSet : "");
//                onTextChange();
                if (textAtFocusGain != null && !textAtFocusGain.equals(getText())) {
                    fireActionPerformed();
                }
                else {
                    setText(lastTextSet != null ? lastTextSet : "");
                    onTextChange();
                }
                textAtFocusGain = null;
                common.Action.removeGlobalActionListener(globalChangeAction);
            }
        });
        addKeyListener(new KeyAdapter() {
            @Override
            public void keyPressed(KeyEvent e) {
                if (e.getKeyCode() == KeyEvent.VK_ENTER) {
                    // Avoid the action() event performed by the focusLost method
                    textAtFocusGain = null;
                }
            }
            
        });
    }
    
    private final ActionListener globalChangeAction = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            if (editable == null || textAtFocusGain == null)
                return;
            if (!getText().equals(textAtFocusGain)) {
                System.out.println("TextFieldForEditable: global action trigger!");
                textAtFocusGain = getText();
                fireActionPerformed();
            }
        }
    };
    
    public boolean isValueValid() {
        assert editable != null;
        return isValueValid;
    }

    public EditableValue getEditableProxy() {
        return editable;
    }

    public void setEditableProxy(ProjectData proj, NetPage page, boolean condition, EditableValue editable) {
        if (!condition)
            editable = null;
        this.project = proj;
        this.page = page;
        this.editable = editable;
        if (editable == null || !editable.isEditable()) {
            setEnabled(false);
            setText(editable == null ? "" : (String)editable.getValue());
            lastTextSet = null;
            if (synchLabel != null)
                synchLabel.setEnabled(false);
        }
        else {
            setEnabled(true);
            lastTextSet = (String)editable.getValue();
            setText(lastTextSet);
            onTextChange();
            if (synchLabel != null)
                synchLabel.setEnabled(true);
        }
    }
    
    private void onTextChange() {
        if (editable != null) {
            String t = getText();
            isValueValid = editable.isValueValid(project, page, t);
            if (isValueValid || t.equals("---")) {
                setForeground(UIManager.getColor("ComboBox.foreground"));
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
