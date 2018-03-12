/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import editor.domain.NetPage;
import javax.swing.JPanel;

/**
 *
 * @author elvio
 */
public abstract class SolverPanel extends JPanel {
    
    public interface SolverPanelListener {
        public void onSolverParamsModified(SolverParams newParams);
        public void onEditingSolverParams();
    }
    public SolverPanelListener listener;

    public void setSolverPanelListener(SolverPanelListener listener) {
        this.listener = listener;
    }
    
    
    public abstract void initializeFor(SolverParams params, NetPage page, 
                                       NetPage evalPage, boolean simplifiedUI,
                                       MeasurePage measPage);
    public abstract SolverParams currParams();
    public abstract void deinitialize();
    public abstract boolean areParamsCorrect();

    public class SolverExprListener implements ExprField.ExprFieldListener {
        @Override
        public void onExprModified() {
            fireSolverParamsModified();
        }
        @Override
        public void onEditingText() {
            listener.onEditingSolverParams();
        }
    }
    protected SolverExprListener solverExprListener = new SolverExprListener();
    
    protected void fireSolverParamsModified() {
        assert listener != null && currParams() != null;
        listener.onSolverParamsModified((SolverParams)Util.deepCopy(currParams()));
    }
}
