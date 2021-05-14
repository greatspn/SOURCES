/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import static common.Util.UIscaleFactor;
import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.NetPage;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.TemplateBinding;
import editor.gui.ResourceFactory;
import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Map.Entry;
import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.EventListenerList;
import latex.LatexFormula;

/**
 *
 * @author elvio
 */
public class TemplateBindingForm extends javax.swing.JPanel {

    private TemplateBinding tbind;
    private ExprTextField[] allTextFields;
        
//    private static final Color BKGND_DISABLED = new Color(240, 240, 240);
    
    /**
     * Creates new form TemplateBindingForm
     */
    public TemplateBindingForm() {
        initComponents();
        setBorder(BorderFactory.createMatteBorder(0, 0, 1, 0, Color.GRAY));
        setBackground(editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR);
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        
        for (ExprTextField etf : allTextFields) {
            etf.setEnabled(enabled);
        }
        setBackground(enabled ? editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR : 
                    editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_DISABLED_COLOR);
    }
    
    static final int XDIST = 3, YDIST = 2;
    
    private GridBagConstraints getGBCNormal(int x, int y, int yDist) {
        return new GridBagConstraints(x, y, 1, 1, 0.0, 0.0, GridBagConstraints.EAST, 
                                         0, new Insets(yDist, XDIST, YDIST, XDIST), 0, 0);
    }
    private GridBagConstraints getGBCHorixFill(int x, int y, int yDist) {
        return new GridBagConstraints(x, y, 1, 1, 1.0, 0.0, GridBagConstraints.CENTER, 
                                      GridBagConstraints.HORIZONTAL, 
                                      new Insets(yDist, XDIST, YDIST, XDIST), 0, 0);
    }
    
    public void initializeFor(TemplateBinding tbind, NetPage page, NetPage evalPage) {
        this.tbind = tbind;
        
        // Clear any previously set table of variables
        removeAll();
        
        // Make the table header
        JLabel nameLabel = new JLabel("Name", SwingConstants.CENTER);
        JLabel assignedValueLabel = new JLabel("Assigned value:", SwingConstants.LEFT);
        nameLabel.setForeground(editor.gui.net.NetEditorPanel.PAGE_FOREGROUND_COLOR);
        assignedValueLabel.setForeground(editor.gui.net.NetEditorPanel.PAGE_FOREGROUND_COLOR);
        add(nameLabel, getGBCNormal(0, 0, YDIST));
        add(assignedValueLabel, getGBCHorixFill(2, 0, YDIST));
        
        // Construct the editing table
        boolean allOk = true;
        int row = 1;
        allTextFields = new ExprTextField[tbind.binding.size()];
        for (Entry<String, Expr> entry : tbind.binding.entrySet()) {
            TemplateVariable tvar = (TemplateVariable)page.getNodeByUniqueName(entry.getKey());
            LatexFormula lf = tvar.getTemplateVariableLatexFormula();
            JLabel varLabel = new JLabel(lf.getAsIcon(UIscaleFactor), SwingConstants.RIGHT);
            varLabel.setForeground(editor.gui.net.NetEditorPanel.PAGE_FOREGROUND_COLOR);
            JLabel okLabel = new JLabel(ResourceFactory.getInstance().getExclamation16());
            okLabel.setForeground(editor.gui.net.NetEditorPanel.PAGE_FOREGROUND_COLOR);
            ExprTextField textField = new ExprTextField(entry.getValue().getEditableValue(), evalPage, okLabel);
            boolean startValid = textField.isValueValid();
            allTextFields[row-1] = textField;
            JLabel equalLabel = new JLabel(" = ");
            equalLabel.setForeground(editor.gui.net.NetEditorPanel.PAGE_FOREGROUND_COLOR);
            
            add(varLabel, getGBCNormal(0, row, 0));
            add(equalLabel, getGBCNormal(1, row, 0));
            add(textField, getGBCHorixFill(2, row, 0));
            add(okLabel, getGBCNormal(3, row, 0));
            
            allOk = allOk && startValid;
            row++;
        }
    }
    
    private final EventListenerList myListenerList = new EventListenerList();
    
    public void addActionListener(ActionListener al) {
        myListenerList.add(ActionListener.class, al);
    }
    
    public void removeActionListener(ActionListener al) {
        myListenerList.remove(ActionListener.class, al);
    }
    
    public ActionListener[] getActionListeners() {
        return myListenerList.getListeners(ActionListener.class);
    }
    
    protected void fireAction(ActionEvent evt) {
        ActionListener[] list = getActionListeners();
        for (ActionListener al : list) {
            al.actionPerformed(evt);
        }
    }
    
    public boolean areAllBindingsValid() {
        for (ExprTextField etf : allTextFields)
            if (!etf.isValueValid())
                return false;
        return true;
    }
    
    private class ExprTextField extends JTextField {
        private final EditableValue editable;
        private final NetPage page;
        private boolean lastIsValid = false;
        private final JLabel okNoLabel;

        public ExprTextField(EditableValue editable, NetPage page, JLabel okNoLabel) {
            super(editable.getValue().toString());
            this.editable = editable;
            this.page = page;
            this.okNoLabel = okNoLabel;
            
            getDocument().addDocumentListener(new DocumentListener() {
                @Override public void insertUpdate(DocumentEvent de) { onTextChange(); }
                @Override public void removeUpdate(DocumentEvent de) { onTextChange(); }
                @Override public void changedUpdate(DocumentEvent de) { onTextChange(); }
            });
            onTextChange();
        }
        
        public boolean isValueValid() {
            return lastIsValid;
        }
        
        private void onTextChange() {
            editable.setValue(null, page, getText());
            lastIsValid = editable.isValueValid(null, page, editable.getValue());
            
            setForeground(lastIsValid ? UIManager.getColor("TextField.foreground") : Color.RED);
            okNoLabel.setIcon(lastIsValid ? ResourceFactory.getInstance().getOk16() : 
                                            ResourceFactory.getInstance().getExclamation16());
            fireAction(null);
        }
    }


    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        setBorder(javax.swing.BorderFactory.createEmptyBorder(8, 8, 8, 8));
        setLayout(new java.awt.GridBagLayout());
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    // End of variables declaration//GEN-END:variables
}
