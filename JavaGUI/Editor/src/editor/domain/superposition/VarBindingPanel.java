/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

import common.Util;
import static common.Util.UIscaleFactor;
import editor.domain.Expr;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.elements.TemplateVariable;
import editor.domain.measures.ExprField;
import static editor.domain.superposition.NetInstanceDescriptorPanel.LABEL_FOREGROUND_COLOR;
import editor.gui.MainWindowInterface;

/**
 *
 * @author elvio
 */
public class VarBindingPanel extends javax.swing.JPanel implements ExprField.ExprFieldListener {

    private final TemplateVariable tvar;
    private final NetInstanceDescriptor descr;
    private final Expr exprCopy;
    private final MainWindowInterface mainInterface;
    private final int numInst;
    
    /**
     * Creates new form VarBindingPanel
     * @param tvar
     * @param descr
     * @param numInst
     * @param mnPage
     * @param mainInterface
     */
    public VarBindingPanel(TemplateVariable tvar, NetInstanceDescriptor descr, int numInst,
                           MultiNetPage mnPage, MainWindowInterface mainInterface) 
    {
        this.tvar = tvar;
        this.descr = descr;
        this.numInst = numInst;
        this.mainInterface = mainInterface;
        initComponents();
        
        label_varEq.setIcon(tvar.getTemplateVariableLatexFormula().getAsIcon(UIscaleFactor));
        exprCopy = (Expr)Util.deepCopy(descr.instParams.getSingleValueBoundTo(tvar));
        exprField.setExprListener(this);
        exprField.initializeFor(exprCopy.getEditableValue(), mnPage);
        
        label_varEq.setForeground(LABEL_FOREGROUND_COLOR);
    }
    
    public void deinitialize() {
        exprField.deinitialize();
    }

    @Override
    public void onExprModified() {
        mainInterface.executeUndoableCommand("change bound value", (ProjectData proj, ProjectPage page) -> {
            NetInstanceDescriptor d = ((MultiNetPage)page).netsDescr.get(numInst);
            d.instParams.bindSingleValue(tvar.getUniqueName(), exprCopy);
        });
    }

    @Override
    public void onEditingText() {
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

        label_varEq = new javax.swing.JLabel();
        exprField = new editor.domain.measures.ExprField();

        setOpaque(false);
        setLayout(new java.awt.GridBagLayout());

        label_varEq.setText(" = ");
        add(label_varEq, new java.awt.GridBagConstraints());

        exprField.setBackground(new java.awt.Color(255, 255, 255));
        exprField.setOpaque(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(exprField, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private editor.domain.measures.ExprField exprField;
    private javax.swing.JLabel label_varEq;
    // End of variables declaration//GEN-END:variables
}
