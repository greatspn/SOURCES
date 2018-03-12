/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.EditableValue;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import editor.gui.ResourceFactory;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JLabel;
import javax.swing.event.EventListenerList;

/**
 *
 * @author elvio
 */
public class CheckBoxForEditable extends JLabel {
    
    private EditableValue editable;
    private NetPage page;
    private ProjectData project;
    private boolean selected = false, indeterminate = false;

    public static final Object INDETERMINATE = new Object();
    
    public CheckBoxForEditable() {        
        setIcon(ResourceFactory.getInstance().getCheckBoxDeselected16());
        addMouseListener(new MouseAdapter() {

            @Override
            public void mouseClicked(MouseEvent e) {
                setSelected(!isSelected());
                fireAction(new ActionEvent(CheckBoxForEditable.this, 0, ""));
            }
            
        });
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
            setSelected(false);
        }
        else {
            setEnabled(true);
            Object value = editable.getValue();
            if (value == INDETERMINATE)
                setIndeterminate();
            else
                setSelected(editable.getValue().equals(Boolean.TRUE));
        }
    }

    public boolean isSelected() {
        return selected;
    }

    public void setSelected(boolean selected) {
        this.selected = selected;
        this.indeterminate = false;
        setIcon(selected ? ResourceFactory.getInstance().getCheckBoxSelected16() : 
                           ResourceFactory.getInstance().getCheckBoxDeselected16());
    }

    

    public boolean isIndeterminate() {
        return indeterminate;
    }

    public void setIndeterminate() {
        this.selected = false;
        this.indeterminate = true;
        setIcon(ResourceFactory.getInstance().getCheckBoxIndeterminate16());
    }
    
    
    private final EventListenerList myListenerList = new EventListenerList();
    
    public void actionPerformed(ActionEvent e) {
        // reinoltra l'evento ai Listeners registrati
        fireAction(e);        
    }
    
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
}
