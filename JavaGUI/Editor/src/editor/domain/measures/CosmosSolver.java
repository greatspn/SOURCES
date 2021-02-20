/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.Main;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.EvaluatedBinding;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.TemplateBinding;
import editor.domain.io.GRMLFormat;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;

/** Cosmos toolchain invokator for:  [gspn|swn]_[ord|sym]_[rg|sym|tr]
 *
 * @author benoit
 */
public class CosmosSolver extends SolverInvokator {
        
    @Override
    void prepareStepForBinding(SolutionStep step, TemplateBinding currBind, 
                               AbstractMeasure[] measures) throws IOException 
    {
        CosmosSolverParams params = (CosmosSolverParams)getPage().solverParams;
        EvaluatedBinding evalBind = currBind.createEvaluated(getContext());
        
        String baseName = getSolutionDir()+File.separator+"step"+step.stepNum+"net";
        File grmlFile = new File(baseName+".grml");
        GspnPage gspn = getPage().targetGspn;
        
        asyncLogStdout("\033[0mPREPARE GrML FOR STEP "+step.stepNum+"\n");
        
        try {
            String log = GRMLFormat.exportGspn(gspn, grmlFile);
            if (log != null)
                throw new UnsupportedOperationException("Error saving the APNN file for step: "+step.stepNum+
                        "\nReason:\n"+log);
        }
        catch (Exception e) {
            Main.logException(e, true);
            throw new IOException("Cannot save GrML file. "+e.getMessage());
        }
        
        // Check if we need to invoke the v_graph command in the tool chain of this step
        String vGraphCmd = null;
       
        int measureNum = 0;
        String measCmd = "";
        boolean hasAll = false, hasMeasures = false, hasStat = false;
        for (AbstractMeasure meas : measures) {
            String measName = "MEASURE" + (measureNum++);
            ResultEntry entry;
            if (meas instanceof FormulaMeasure) {
                FormulaMeasure fm = (FormulaMeasure)meas;
                switch (fm.getLanguage()) {
                    case PERFORMANCE_INDEX:
                        hasMeasures = true;
                        entry = new ScalarResultEntry(measName, evalBind);
                        String spnMeas = fm.getFormula().convertLang(getContext(), EvaluationArguments.NO_ARGS, 
                                        ExpressionLanguage.GRML);
                        measCmd += " -measure "+measName+" \""+spnMeas+"\"";
                        step.entries.add(entry);
                        break;
                    case ALL:
                        hasMeasures = true;
                        if (hasAll)
                            throw new UnsupportedOperationException("Only one ALL measure can be computed at once.");
                        hasAll = true;
                        entry = new AllMeasuresResult("ALL", evalBind);
                        step.entries.add(entry);
                        break;
                    case STAT:
                        if (hasStat)
                            throw new UnsupportedOperationException("Only one STAT measure can be computed at once.");
                        hasStat = true;
                        entry = new StatResultEntry("STAT", evalBind);
                        step.entries.add(entry);
                        break;
                    default:
                        throw new UnsupportedOperationException("Language "+fm.getLanguage()+" is not supported by the tool.");
                }
            }
            else throw new UnsupportedOperationException("missing measure impl.");
            meas.getResults().table.add(entry);
        }
  
        ArrayList<String> cmd = startOfCommand();
        cmd.add(useCosmos_binary("Cosmos"));
        cmd.add(makeFilenameForCmd(grmlFile));
        cmd.add("--loop");
        cmd.add("100");
        cmd.add("--max-run");
        cmd.add("20000");
        step.addCmd(cmd);
//        step.addCmd(useCosmos_binary("Cosmos") + " \"" + grmlFile.getAbsolutePath() +
//                    "\" --loop 100 --max-run 20000");
    }

    @Override
    void readCommand(String command, int stepNum, AbstractMeasure[] measures) {
        defaultReadCommand(command, stepNum, measures);
    }

    @Override
    void modifyEnvironmentVars(Map<String, String> env) {
    }

    @Override
    void startOfStep(SolutionStep step) {
    }

    @Override
    void endOfStep(SolutionStep step, boolean interrupted, boolean allStepsCompleted) {
        step.completed = (!interrupted && allStepsCompleted);
    }
}
