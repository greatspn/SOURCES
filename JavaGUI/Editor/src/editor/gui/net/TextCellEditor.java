/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.EditableCell;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import javax.swing.JComponent;
import javax.swing.JTextField;
import javax.swing.event.CellEditorListener;
import javax.swing.event.ChangeEvent;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.EventListenerList;

/**
 *
 * @author elvio
 */
public class TextCellEditor extends JTextField implements BaseCellEditor {

    EditableCell editable;
    private final EventListenerList cellListeners = new EventListenerList();
    private Point screenPos;
    private boolean isTextValid = false;
    private final ProjectData proj;
    private final NetPage page;
    private String textAtFocusGain = null;
    
    public TextCellEditor(ProjectData proj, NetPage page, EditableCell editable, Point _screenPos) {
        super((String)editable.getValue());
        this.editable = editable;
        this.screenPos = _screenPos;
        this.proj = proj;
        this.page = page;
        
        selectAll();
        isTextValid = editable.isValueValid(proj, page, getText());
        Dimension dim = getPreferredSize();
        dim.width += dim.height/2;
        setBounds(Math.max(0, screenPos.x - dim.width / 2),
                  screenPos.y - dim.height / 2,
                  dim.width, dim.height);
        
        addFocusListener(new FocusListener() {
            @Override
            public void focusGained(FocusEvent fe) { 
                textAtFocusGain = getText();
                common.Action.addGlobalActionListener(globalChangeAction);
            }

            @Override
            public void focusLost(FocusEvent fe) {
                if (textAtFocusGain != null && !textAtFocusGain.equals(getText())) {
                    fireActionPerformed();
                }
                else // same text
                    fireEditingCanceled(new ChangeEvent(TextCellEditor.this));
                textAtFocusGain = null;
                common.Action.removeGlobalActionListener(globalChangeAction);
                //fireEditingCanceled(new ChangeEvent(TextCellEditor.this));
            }
        });
        addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent ae) {
//                if (isTextValid)
                fireEditingStopped(new ChangeEvent(TextCellEditor.this));
//                else
//                    fireEditingCanceled(new ChangeEvent(TextCellEditor.this));
            }
        });
        addKeyListener(new KeyListener() {
            @Override
            public void keyTyped(KeyEvent ke) {
                Dimension dim = getPreferredSize();
                Dimension dim2 = getSize();
                if (dim.width + dim.height/2 > dim2.width)
                    dim.width += dim.height;
                dim.width = Math.max(dim.width, dim2.width);
                dim.height = Math.max(dim.height, dim2.height);
                setBounds(Math.max(0, screenPos.x - dim.width / 2),
                          screenPos.y - dim.height / 2,
                          dim.width, dim.height);
            }

            @Override
            public void keyPressed(KeyEvent ke) { 
                if (ke.getKeyCode() == KeyEvent.VK_ESCAPE)
                    cancelEditing();
            }

            @Override
            public void keyReleased(KeyEvent ke) {  }
        });
        getDocument().addDocumentListener(new DocumentListener() {
            @Override public void insertUpdate(DocumentEvent de) { onTextChange(); }
            @Override public void removeUpdate(DocumentEvent de) { onTextChange(); }
            @Override public void changedUpdate(DocumentEvent de) { onTextChange(); }
        });
    }
    
    private final ActionListener globalChangeAction = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            if (editable == null || textAtFocusGain == null)
                return;
            if (!getText().equals(textAtFocusGain)) {
                System.out.println("TextCellEditor: global action trigger!");
                textAtFocusGain = getText();
                fireActionPerformed();
            }
        }
    };
    
    public void onTextChange() {
        isTextValid = editable.isValueValid(proj, page, getText());
        if (isTextValid) {
            setForeground(Color.BLACK);
        }
        else {
            setForeground(Color.RED);
        }
    }
    
    @Override
    public JComponent getEditorComponent() {
        return this;
    }

    @Override
    public void cancelEditing() {
        fireEditingCanceled(null);
    }

    @Override
    public String getEditedString() {
        return getText();
    }

    @Override
    public void addCellEditorListener(CellEditorListener cel) {
        cellListeners.add(CellEditorListener.class, cel);
    }

    @Override
    public void removeCellEditorListener(CellEditorListener cel) {
        cellListeners.remove(CellEditorListener.class, cel);
    }
    
    public CellEditorListener[] getCellEditorListener() {
        return cellListeners.getListeners(CellEditorListener.class);
    }
    
    protected void fireEditingStopped(ChangeEvent evt) {
        if (editable == null)
            return;
        CellEditorListener[] list = getCellEditorListener();
        for (CellEditorListener cel : list) {
            cel.editingStopped(evt);
        }
        editable = null;
    }
    protected void fireEditingCanceled(ChangeEvent evt) {
        if (editable == null)
            return;
        CellEditorListener[] list = getCellEditorListener();
        for (CellEditorListener cel : list) {
            cel.editingCanceled(evt);
        }
        editable = null;
    }
    
}
