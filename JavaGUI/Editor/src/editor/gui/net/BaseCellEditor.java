/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import javax.swing.JComponent;
import javax.swing.event.CellEditorListener;

/** Base class of the currently active cell editor (if any).
 *
 * @author elvio
 */
public interface BaseCellEditor {
    
    public JComponent getEditorComponent();
    
    // Called externally to close immediately the cell editor and cancel any changes
    public void cancelEditing();
    
    // Editing events
    public void addCellEditorListener(CellEditorListener cel);
    public void removeCellEditorListener(CellEditorListener cel);
    
    public String getEditedString();
}
