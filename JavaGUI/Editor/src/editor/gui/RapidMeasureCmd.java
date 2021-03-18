/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import editor.domain.ProjectPage;
import editor.domain.elements.GspnPage;
import editor.domain.measures.FormulaLanguage;
import editor.domain.measures.FormulaMeasure;
import editor.domain.measures.GreatSPNSolverParams;
import editor.domain.measures.MeasurePage;
import editor.domain.measures.RGMEDD2SolverParams;
import editor.domain.superposition.ComposableNet;

/** Common page commands
 *
 * @author elvio
 */
public enum RapidMeasureCmd {
    CTL_MODEL_CHECKING,
    BUILD_RG,
    BUILD_SYMRG,
    BUILD_RS_MDD,
    STEADY_STATE_SOLUTION,
    TRANSIENT_SOLUTION,
    STEADY_STATE_SIM,
    TRANSIENT_SIM;
    
    public MeasurePage createRapidMeasureFor(ProjectPage page) {
        final GspnPage compGspn = (GspnPage)((ComposableNet)page).getComposedNet();
        MeasurePage mp = new MeasurePage();
        mp.rapidMeasureType = this.name();
        mp.simplifiedUI = true;
        mp.targetGspnName = page.getPageName();
        
        switch (this) {
            case CTL_MODEL_CHECKING: {
                mp.setPageName("CTL model checking of "+page.getPageName());
                RGMEDD2SolverParams sp = new RGMEDD2SolverParams();
                sp.genCounterExamples = true;
                sp.varOrder = RGMEDD2SolverParams.VariableOrder.META_HEURISTIC;
                mp.solverParams = sp;
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.STAT));
                mp.measures.add(new FormulaMeasure("CTL formula", false, 
                        "E( #P0==0 U #P1!=0 )", FormulaLanguage.CTL));
            }
            break;
            
            case BUILD_RG:
            case BUILD_SYMRG: {
                boolean sym = (this == BUILD_SYMRG);
                boolean isCPN = compGspn.gspnHasColors();
                mp.setPageName((sym ? "SRG" : "RG")+" of "+page.getPageName());
                GreatSPNSolverParams sp = new GreatSPNSolverParams();
                sp.mode = sym ? GreatSPNSolverParams.SolverMode.SWN_SYM
                              : GreatSPNSolverParams.SolverMode.SWN_ORD;
                mp.solverParams = sp;
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.STAT));
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.RG));
            }
            break;

            case BUILD_RS_MDD: {
                mp.setPageName("RS using DD of "+page.getPageName());
                RGMEDD2SolverParams sp = new RGMEDD2SolverParams();
                sp.genCounterExamples = false;
                sp.varOrder = RGMEDD2SolverParams.VariableOrder.META_HEURISTIC;
                mp.solverParams = sp;
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.STAT));
            }
            break;
            
            case STEADY_STATE_SOLUTION: 
            case TRANSIENT_SOLUTION: {
                boolean steadyState = (this == STEADY_STATE_SOLUTION);
                mp.setPageName((steadyState ? "Steady State" : "Transient")+" analysis of "+page.getPageName());
                GreatSPNSolverParams sp = new GreatSPNSolverParams();
                sp.mode = GreatSPNSolverParams.SolverMode.SWN_ORD;
                sp.solTime = steadyState ? GreatSPNSolverParams.SolutionTime.STEADY_STATE
                                         : GreatSPNSolverParams.SolutionTime.TRANSIENT;
                mp.solverParams = sp;
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.STAT));
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.RG));
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.ALL));
            }
            break;

            case STEADY_STATE_SIM: 
            case TRANSIENT_SIM: {
                boolean steadyState = (this == STEADY_STATE_SIM);
                mp.setPageName((steadyState ? "Steady State" : "Transient")+" simulation of "+page.getPageName());
                GreatSPNSolverParams sp = new GreatSPNSolverParams();
                sp.mode = GreatSPNSolverParams.SolverMode.SWN_ORD_SIMUL;
                sp.solTime = steadyState ? GreatSPNSolverParams.SolutionTime.STEADY_STATE
                                         : GreatSPNSolverParams.SolutionTime.TRANSIENT;
                mp.solverParams = sp;
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.STAT));
                mp.measures.add(new FormulaMeasure("", false, "", FormulaLanguage.ALL));
            }
            break;
        }
        if (mp.getPageName() == null)
            mp = null; // missing case in the switch
        return mp;
    }
}
