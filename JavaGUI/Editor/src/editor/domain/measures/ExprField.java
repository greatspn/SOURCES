/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.EditableValue;
import editor.domain.ProjectPage;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

/**
 *
 * @author elvio
 */
public class ExprField extends javax.swing.JPanel {

    public final Color VERY_LIGHT_GRAY_BKGND = new Color(240, 240, 240);

    public static interface ExprFieldListener {
        // New modification confirmed, create an undo/redo block.
        public void onExprModified();
        // Called during modifications
        public void onEditingText();
    }

    private EditableValue editable;
    private ExprFieldListener listener;
    private ProjectPage page;
    private boolean lastIsValid = false;
    private boolean okNoLabelVisible = true;
    private boolean initializing = false;
    private String strWhenFocusGained = null;

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled); //To change body of generated methods, choose Tools | Templates.
        textField.setEnabled(enabled);
        label_okNo.setEnabled(enabled);
    }

    public ExprField() {
        initComponents();

        textField.getDocument().addDocumentListener(new DocumentListener() {
            @Override
            public void insertUpdate(DocumentEvent de) {
                onTextChange();
            }

            @Override
            public void removeUpdate(DocumentEvent de) {
                onTextChange();
            }

            @Override
            public void changedUpdate(DocumentEvent de) {
                onTextChange();
            }
        });
        textField.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                listener.onExprModified();
            }
        });
        textField.addFocusListener(new FocusListener() {
            @Override
            public void focusGained(FocusEvent e) {
                strWhenFocusGained = textField.getText();
                // register the global action trigger. This ensures that 
                // buttons that are non-focusable will still cause an 
                // activatedAction() call, to push the modified expr event.
                common.Action.addGlobalActionListener(activatedAction);
            }

            @Override
            public void focusLost(FocusEvent e) {
                common.Action.removeGlobalActionListener(activatedAction);
                if (editable == null || strWhenFocusGained == null) {
                    return;
                }
                if (!textField.getText().equals(strWhenFocusGained)) {
                    listener.onExprModified();
                }
                strWhenFocusGained = null;
            }
        });
    }
    
    private ActionListener activatedAction = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            if (editable == null || strWhenFocusGained == null)
                return;
            if (!textField.getText().equals(strWhenFocusGained)) {
                System.out.println("global action trigger!");
                strWhenFocusGained = textField.getText();
                listener.onExprModified();
            }
        }
    };
    
    public void setExprListener(ExprFieldListener _listener) {
        this.listener = _listener;
    }

    public void initializeFor(EditableValue editable, ProjectPage page) {
        assert this.listener != null; // Should be already set.
        initializing = true;
        this.editable = editable;
        this.page = page;
        textField.setText(editable.getValue().toString());
        if (strWhenFocusGained != null) {
            strWhenFocusGained = editable.getValue().toString();
        }
        onTextChange();
        initializing = false;
    }

    public void deinitialize() {
        initializing = true;
        editable = null;
        page = null;
        strWhenFocusGained = null;
        textField.setText("");
        initializing = false;
    }

    public boolean isValueValid() {
        return lastIsValid;
    }

    private void onTextChange() {
        if (editable == null) {
            return;
        }
        editable.setValue(null, page, textField.getText());
        lastIsValid = editable.isValueValid(null, page, editable.getValue());

        textField.setForeground(lastIsValid ? UIManager.getColor("TextField.foreground") : Color.RED);
        label_okNo.setIcon(lastIsValid ? resourceFactory.getOk16() : resourceFactory.getExclamation16());
        //textChangedListener.actionPerformed(null);
        if (!initializing)
            listener.onEditingText();
    }

    public boolean isOkNoLabelVisible() {
        return okNoLabelVisible;
    }

    public void setOkNoLabelVisible(boolean okNoLabelVisible) {
        this.okNoLabelVisible = okNoLabelVisible;
        label_okNo.setVisible(okNoLabelVisible);
    }

    @Override
    public void setBackground(Color bg) {
        super.setBackground(bg); 
        // This coomented code below makes the textfield gray on macosx.
//        if (textField != null)
//            textField.setBackground(bg);
        if (label_okNo != null)
            label_okNo.setBackground(bg);
    }
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        resourceFactory = new editor.gui.ResourceFactory();
        textField = new javax.swing.JTextField();
        label_okNo = new javax.swing.JLabel();

        setLayout(new java.awt.GridBagLayout());

        textField.setText("expr");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(textField, gridBagConstraints);

        label_okNo.setIcon(resourceFactory.getOk16());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(1, 2, 1, 4);
        add(label_okNo, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel label_okNo;
    private editor.gui.ResourceFactory resourceFactory;
    private javax.swing.JTextField textField;
    // End of variables declaration//GEN-END:variables
}
