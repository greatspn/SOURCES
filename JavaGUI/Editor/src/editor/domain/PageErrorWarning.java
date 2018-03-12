/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import common.Util;
import editor.domain.measures.FormulaMeasure;
import editor.domain.superposition.NetInstanceDescriptor;
import editor.gui.ResourceFactory;
import java.awt.Color;
import java.awt.Component;
import java.io.Serializable;
import java.util.HashSet;
import java.util.Set;
import javax.swing.Icon;
import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;

/** An error/warning in a project page, shown in the bottom table of the GUI.
 *
 * @author elvio
 */
public class PageErrorWarning implements Serializable {
    
    boolean isWarning;
    String description;
    Set<Selectable> errObjectList = null;
    
    public static PageErrorWarning newError(String description, Selectable errObject) {
        Set<Selectable> s = null;
        if (errObject != null) {
            s = new HashSet<>();
            s.add(errObject);
        }
        return newErrorSet(description, s);
    }
    public static PageErrorWarning newErrorSet(String description, Set<Selectable> errObjectList) {
        return new PageErrorWarning(description, errObjectList, false);
    }
    public static PageErrorWarning newWarning(String description, Selectable errObject) {
        Set<Selectable> s = null;
        if (errObject != null) {
            s = new HashSet<>();
            s.add(errObject);
        }
        return newWarningSet(description, s);
    }
    public static PageErrorWarning newWarningSet(String description, Set<Selectable> errObjectList) {
        return new PageErrorWarning(description, errObjectList, true);
    }

    private PageErrorWarning(String description, Set<Selectable> errObjectList, boolean isWarning) {
        this.description = description;
        this.errObjectList = errObjectList;
        this.isWarning = isWarning;
    }

    public String getDescription() {
        return description;
    }

    public boolean isWarning() {
        return isWarning;
    }
    
    public boolean areAffectedElementsSelected() {
        if (errObjectList == null)
            return false;
        
        // Error selected if at least one object is selected
        for (Selectable s : errObjectList)
            if (s.isSelected())
                return true;
        return false;
    }
    
    public void selectAffectedElements(boolean sel) {
        if (errObjectList != null) {
            for (Selectable s : errObjectList)
                s.setSelected(sel);
        }
    }
    
    public static Icon ERROR16_ICON = ResourceFactory.getInstance().getExclamation16();
    public static Icon WARN16_ICON = ResourceFactory.getInstance().getError16();
    
    public static void setupPageErrorTable(JTable table) {
        PageErrorWarning.TableModel pageErrModel = new PageErrorWarning.TableModel();
        table.setModel(pageErrModel);
        table.setDefaultRenderer(String.class, pageErrModel.theCellRenderer);
//        ((PageError.TableModel)jTablePageErrors.getModel()).setErrorList(null);
        table.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
        table.getColumnModel().getColumn(0).setPreferredWidth(20);
        table.getColumnModel().getColumn(0).setMaxWidth(20);
        table.getColumnModel().getColumn(0).setMinWidth(20);
        table.getColumnModel().getColumn(1).setPreferredWidth(40);
        table.getColumnModel().getColumn(2).setPreferredWidth(600);
        table.setRowHeight((int)Util.origDefaultUiFontSize+2);
        
    }
    
    
    public static class TableModel extends AbstractTableModel {
        static final String[] colNames = { "", "Element", "Error description" };
        PageErrorWarning[] errors = null;
        
        public void setErrorList(PageErrorWarning[] errors) {
            this.errors = errors;
            fireTableDataChanged();
        }

        public PageErrorWarning getErrorAt(int row) { return errors[row]; }
        
        @Override public String getColumnName(int i) { return colNames[i]; }        
        @Override public int getRowCount() { return (errors != null) ? errors.length : 0; }
        @Override public int getColumnCount() { return colNames.length; }
        @Override public Object getValueAt(int row, int column) {
             switch (column) {
                case 0: return errors[row].isWarning ? WARN16_ICON : ERROR16_ICON; // Icon
                case 1: {
                    Selectable singleErrObj = null;
                    boolean multipleErrObjs = false;
                    if (errors[row].errObjectList != null && errors[row].errObjectList.size() >= 1) {
                        multipleErrObjs = errors[row].errObjectList.size() > 1;
                        if (!multipleErrObjs) {
                            singleErrObj = errors[row].errObjectList.iterator().next();
                        }
                    }
                    if (multipleErrObjs)
                        return "<Multiple objects>";
                    else if (singleErrObj == null) // a page error
                        return "<global>";
                    else if (singleErrObj instanceof Node)
                        return ((Node)singleErrObj).getUniqueName();
                    else if (singleErrObj instanceof Edge) {
                        Node head = ((Edge)singleErrObj).getHeadNode();
                        Node tail = ((Edge)singleErrObj).getTailNode();
                        return (tail==null ? "???" : tail.getUniqueName()) + " -> " +
                               (head==null ? "???" : head.getUniqueName());
                    }
                    else if (singleErrObj instanceof Selectable.BaseSelectable)
                        return singleErrObj.toString();
                    else if (singleErrObj instanceof FormulaMeasure)
                        return "<measure expr>";
                    else if (singleErrObj instanceof NetInstanceDescriptor)
                        return "<subnet>";
                    else {
                        System.out.println("singleErrObj instanceof "+singleErrObj.getClass().getName());
                        return "<unknown object>";
                    }
                }
                case 2: return errors[row].getDescription();
                default: throw new IllegalStateException();
            }
        }
        @Override public Class<?> getColumnClass(int column) {
            switch (column) {
                case 0: return Icon.class;
                case 1: return String.class;
                case 2: return String.class;
                default: throw new IllegalStateException();
            }        
        }
        
        static final Color LIGHT_RED = new Color(255, 240, 240);
        static final Color LIGHT_YELLOW = new Color(255, 255, 240);
        static final Color WARN_YELLOW = new Color(110, 100, 0);

        public class CellRenderer extends DefaultTableCellRenderer {
            @Override
            public Component getTableCellRendererComponent(JTable table, Object element,
                                                           boolean isSelected, boolean hasFocus,
                                                           int row, int column) 
            {
                Component comp = super.getTableCellRendererComponent(table, element, isSelected,
                                                                     hasFocus, row, column); 
                JLabel label = (JLabel)comp;
                if (isSelected)
                    label.setForeground(errors[row].isWarning ? LIGHT_YELLOW : LIGHT_RED);
                else
                    label.setForeground(errors[row].isWarning ? WARN_YELLOW : Color.RED);
                
                return label;
            }            
        }
        public CellRenderer theCellRenderer = new CellRenderer();
    }
}
