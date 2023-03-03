/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import editor.Main;
import editor.domain.elements.DtaSignature;
import editor.domain.Expr;
import editor.domain.NetObject;
import editor.domain.elements.TemplateVariable;
import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.grammar.VarMultiAssignment;
import editor.domain.io.DtaFormat;
import editor.domain.io.GreatSpnFormat;
import editor.gui.NoOpException;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NoSuchElementException;
import java.util.Scanner;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

/** The class that manages the parallel thread that invokes the
 *  external solver on thw command line, and reads its output.
 *
 * @author elvio
 */
public abstract class SolverInvokator  implements SolverDialog.InterruptibleSolverListener {
    
    private final ArrayList<SolutionStep> steps = new ArrayList<>();
    
    // Parameters that are available during solver invokation
    
    // The swing component that will show the log
    private SolverDialog dlg; 
    private MeasurePage page;
    private ParserContext context;
    private Thread solutionThread;
    private File projectFile;
    private File solutionDir;
    private File gspnFile;
    private AbstractMeasure[] measures;
    private int numReportedNoSuchElemError = 0;
    
    private int currStepNum = -1;
//    private ResultTable[] results;
    
    
    private static class StepCmd {
        // The command line, split into separate arguments
        public ArrayList<String> cmd;
        // Is it optional? (i.e. it may fail and the computation is not stopped)
        public boolean mayFail;

        public StepCmd(ArrayList<String> cmd, boolean mayFail) {
            this.cmd = cmd;
            this.mayFail = mayFail;
        }
    }
    
    // Single solver invokation in a multi-solution measure
    public class SolutionStep {
        // Incremental step number
        public final int stepNum;
        // The invoked command line
        private ArrayList<StepCmd> cmdLines = new ArrayList<>();
        // The entries that will interpret the results
        public ArrayList<ResultEntry> entries = new ArrayList<>();
        // This value indicates that the process has not been completed
        public boolean completed = false;
        
        Process proc;
        BufferedInputStream errStrm, outStrm;
        Thread errLogThread, outLogThread;
        SolverInvokator theInvokator;

        public SolutionStep(int stepNum, SolverInvokator theInvokator) {
            this.stepNum = stepNum;
            this.theInvokator = theInvokator;
        }
        
        public void addCmd(ArrayList<String> cmd) {
            cmdLines.add(new StepCmd(cmd, false));
        }

        public void addOptionalCmd(ArrayList<String> cmd) {
            cmdLines.add(new StepCmd(cmd, true));
        }
        
        private boolean invokeSolutionStep(int stepNum) throws Exception {
            theInvokator.startOfStep(this);
            
            // Prepare the environment variables
            String[] envp = prepareRuntimeEnvironmentVars(theInvokator);

            if (stepNum > 0)
                asyncLogStdout("\033[2X\n"); // line separator
            
            boolean interrupted = false;
            int numCompletedCmds = 0;
            for (StepCmd step : cmdLines) {
                asyncLogStdout("\033[0mEXEC: "+cmdToString(step.cmd)+"\n");

                proc = Runtime.getRuntime().exec(step.cmd.toArray(new String[step.cmd.size()]), envp);

                errStrm = new BufferedInputStream(proc.getErrorStream());
                outStrm = new BufferedInputStream(proc.getInputStream());
                errLogThread = new Thread(makeStreamReader(errStrm, false), "stderr reader");
                outLogThread = new Thread(makeStreamReader(outStrm, true), "stdout reader");
                errLogThread.start();
                outLogThread.start();

                // Wait for process termination, and stdout/err thread completion
                while (outLogThread != null) {
                    try {
                        outLogThread.join();
                        outLogThread = null;
                    }
                    catch (InterruptedException ie) {
                        interrupted = true;
                        proc.destroy();
                    }
                }
                while (errLogThread != null) {
                    try {
                        errLogThread.join();
                        errLogThread = null;
                    }
                    catch (InterruptedException ie) {
                        interrupted = true;
                        proc.destroy();
                    }
                }
                int retVal = proc.waitFor();
                if (interrupted) {
                    asyncLogStdout("\033[1X\033[31m PROCESS INTERRUPTED.\033[0m\n");
                    break; // do not execute other commands
                }
                else if (retVal != 0) {
                    asyncLogStdout("\033[1X\033[31m PROCESS EXITED WITH VALUE "+retVal+".\033[0m\n");
                    if (!step.mayFail)
                        break; // do not execute other commands
                    else 
                        numCompletedCmds++; // continue anyway
                }
                else {
                    asyncLogStdout("\033[0X\033[32m PROCESS EXITED NORMALLY.\033[0m\n");
                    numCompletedCmds++;
                }                
            }
            // Notify the end of this step.
            theInvokator.endOfStep(this, interrupted, (numCompletedCmds == cmdLines.size()));
            asyncLogStdout(CMD_PREFIX+" END_STEP\n");            
            if (!completed) {
                asyncLogStdout("\033[1X\033[31m DID NOT COMPLETE THE SOLUTION.\033[0m\n");
            }
            // Ensure that all the output has been processed before 
            // going to the next solution step
            syncFlushLog();
            
            return interrupted;
        }
    }
    
    private Runnable makeStreamReader(final BufferedInputStream stream, final boolean isStdOut) {
        return new Runnable() {
            @Override
            public void run() {
                try {
                    byte[] buffer = new byte[512];
                    StringBuilder sb = new StringBuilder();
                    int len;
                    while ((len = stream.read(buffer)) != -1) {
                        for (int i=0; i<len; i++) {
                            sb.append((char)buffer[i]);
                            if (buffer[i] == '\n') {
                                String line = sb.toString();
                                // send the text buffer to the log
                                asyncLog(isStdOut, line);
                                sb.delete(0, sb.length());
                                assert sb.length() == 0;
                            }
                        }
                    }
                    // Last non-terminated line
                    if (sb.length() > 0) {
                        asyncLog(isStdOut, sb.toString());
                        sb.delete(0, sb.length());
                    }
                }
                catch (IOException e) {
                    asyncLog(isStdOut, "Broken pipe.");
                }
                try {
                    stream.close();
                }
                catch (IOException e) { /* ignore */ }
//                System.out.println("reader thread closed.");
            }
        };
    }
    
//    // Create the argument array from a single command line String
//    public static String[] splitCommandLine(String str) {
////        boolean runFromBash = true;
//        
//        // Indirectly invoke the command using bash
//        // Only for Windows 10+ 64bit with Linux Subsystem installed.
////        if (Util.isWindows()) {
////            return new String[] { "bash", "-c", "FROM_GUI=1 "+str.replace("\"", "\\\"") };
////        }
//        
//        str += " "; // To detect last token when not quoted...
//        ArrayList<String> strings = new ArrayList<>();
//        char inQuote = 0;
//        StringBuilder sb = new StringBuilder();
//        for (int i = 0; i < str.length(); i++) {
//            char c = str.charAt(i);
//            if (((c == '\"' || c == '\'') && inQuote==0) || // opening quote
//                    (c == inQuote) || // closing quote
//                    (c == ' ' && inQuote==0)) // closing non-quoted argument
//            {
//                if (c == inQuote)
//                    inQuote = 0;
//                else if (inQuote == 0 && (c == '\"' || c == '\''))
//                    inQuote = c;
//                if (inQuote == 0 && sb.length() > 0) {
//                    strings.add(sb.toString());
//                    sb.delete(0, sb.length());
//                }
//            } 
//            else
//                sb.append(c);
//        }
//        return strings.toArray(new String[strings.size()]);
//    }
    
//    public static void main(String[] args) {
//        splitCommandLine("start start 'in in \" inside \" in  ' end end");
//    }
    
    // The invokation thread that runs in parallel to the GUI thread
    // when the application is calling the solvers
    private final Runnable invokationProcess = new Runnable() {
        @Override
        public void run() {
            int stepNum = 0;
            for (SolutionStep step : steps) {
                try {
                    currStepNum = stepNum;
                    boolean interrupted = step.invokeSolutionStep(stepNum++);
                    if (interrupted)
                        break;
                }
                catch (Exception e) {
                    asyncLogStdout("\033[1X\033[31m COULD NOT COMPLETE THE STEP:\033[0m "
                                   +e.getMessage()+"\n");
                }
                finally {
                    currStepNum = -1;
                }
            }
            
            dlg.setInterruptListener(null);
            page = null;
            measures = null;
            context = null;
            dlg = null;
            projectFile = null;
            solutionDir = null;
            gspnFile = null;
        }
    };
    
    //=========================================================================
    // Asynch management of the reader threads that relaunch the read lines
    // in the GUI thread with the logLater() method
    //=========================================================================

    public static final String CMD_PREFIX = "#{GUI}#";
    private synchronized void asyncLog(final boolean isStdOut, final String line) {
//        System.out.println((isStdOut ? "OUT: [" : "ERR: [") + line+"]");
        final SolverDialog dialog = dlg;
        if (line.startsWith(CMD_PREFIX))
            command_lines.add(line.substring(CMD_PREFIX.length() + 1));
        else if (isStdOut)
            stdout_lines.add(line);
        else
            stderr_lines.add(line);
        
        if (!invokingLater) {
            SwingUtilities.invokeLater(new Runnable() {
                @Override
                public void run() {
//                  try { Thread.sleep(50); } catch (InterruptedException i) { }
                    _logLinesLater(dialog);
                }
            });
            invokingLater = true;
        }
    }
    
    protected void syncFlushLog() {
        try {
            final SolverDialog dialog = dlg;
            SwingUtilities.invokeAndWait(new Runnable() {
                @Override
                public void run() {
                    _logLinesLater(dialog);
                }
            });
        } catch (InterruptedException | InvocationTargetException ie) {
            Main.logException(ie, true);
        }
    }
    
    protected synchronized void asyncLogStdout(final String line) {
        asyncLog(true, line);
    }
    protected synchronized void asyncLogStderr(final String line) {
        asyncLog(false, line);
    }
    
    private boolean invokingLater = false;
    private final ArrayList<String> command_lines = new ArrayList<>();
    private final ArrayList<String> stdout_lines = new ArrayList<>();
    private final ArrayList<String> stderr_lines = new ArrayList<>();
    private synchronized void _logLinesLater(SolverDialog dialog) {
        //assert invokingLater;
        for (String line : stdout_lines) {
            if (dialog != null) {
                dialog.append(line, true);
            }
        }
        for (String line : stderr_lines) {
            if (dialog != null) {
                dialog.append(line, true);
            }
        }
        // Process command lines sent by the solver
        for (String cmd : command_lines) {
//            System.out.println("COMMAND: "+cmd);
            readCommand(cmd, getCurrStepNum(), measures);
        }
        stdout_lines.clear();
        stderr_lines.clear();
        command_lines.clear();
        invokingLater = false;
    }
    

    //=========================================================================
    
    public void invoke(MeasurePage page, AbstractMeasure[] measures, 
                       ParserContext context, File projectFile,
                       SolverDialog _dlg) throws Exception
    {
        this.page = page;
        this.measures = measures;
        this.context = context;
        this.projectFile = projectFile;
        this.dlg = _dlg;
        
        // Prepare the files read by the solvers
        if (projectFile == null) {
            projectFile = File.createTempFile("temporary", ".PNPRO"); 
        }
        // Prepare the directory
        solutionDir = page.getSolutionDirectoryName(projectFile);
        solutionDir.mkdir();
        // Export the target GSPN
        gspnFile = new File(solutionDir.getAbsoluteFile() + File.separator +
                            page.targetGspnName);
        String err = GreatSpnFormat.exportGspn(page.targetGspn, 
                                  new File(gspnFile.getAbsolutePath()+".net"), 
                                  new File(gspnFile.getAbsolutePath()+".def"),
                                  true,
                                  enableSupportForMDepArcsInNetDef());
        if (err != null) {
            asyncLogStdout("Encountered the following problems during GSPN exportation:\n"+err+"\n");
        }
        // Export the DTAs
        if (page.knownDtas != null) {
            for (DtaSignature sig : page.knownDtas) {
                if (sig.isUsed) {
                    File dtaFile = new File(solutionDir.getAbsoluteFile() + File.separator +
                                            sig.dta.getPageName()+".dta");
                    err = DtaFormat.export(sig.dta, dtaFile);
                    if (err != null) {
                        asyncLogStdout("Encountered the following problems during DTA exportation:\n"+err+"\n");
                    }
                }
            }
        }
        
        // Prepare the steps (one step for each parameter binding)
        try {
            bindParams(0);
        }
        catch (MeasureEvaluationException mee) {
            JOptionPane.showMessageDialog(dlg, mee.getMessage(), 
                                          "Error during measure evaluation.",
                                          JOptionPane.ERROR_MESSAGE);
            mee.printStackTrace();
            throw new NoOpException();
        }
        
        // Launch the thread that will invoke the solvers
        solutionThread = new Thread(invokationProcess, "Solver invokation thread.");
        // Make the thread interruptible by the solver dialog window.
        dlg.setInterruptListener(this);

        solutionThread.start();
    }
    

    @Override
    public void interruptSolution() {
        if (solutionThread != null)
            solutionThread.interrupt();
    }
    
    
    private void bindParams(int param) 
            throws MeasureEvaluationException, IOException 
    {
        // All bindings are assigned
        if (param >= page.varListAssignments.assigns.size()) {
            computeMeasuresWithBinding();
            return;
        }
        
        // Some variables are still unbounded
        VarMultiAssignment vma = page.varListAssignments.assigns.get(param);
        switch (vma.bindModel) {
            case SINGLE_VALUE:
                vma.singleVal.checkExprCorrectness(context, page.targetGspn, null);
                context.templateVarsBinding.bindSingleValue(vma.varName, vma.singleVal);
                bindParams(param+1);
                break;
            case MULTIPLE_VALUES:
                String[] multivals = vma.multipleVals.getExpr().split(",");
                for (String val : multivals) {
                    Expr expr = vma.createExpr(val);
                    expr.checkExprCorrectness(context, page.targetGspn, null);
                    context.templateVarsBinding.bindSingleValue(vma.varName, expr);
                    bindParams(param+1);
                }
                break;
            case RANGE:
                vma.rangeFrom.checkExprCorrectness(context, page.targetGspn, null);
                EvaluatedFormula eFrom = vma.rangeFrom.evaluate(context, EvaluationArguments.NO_ARGS);
                vma.rangeTo.checkExprCorrectness(context, page.targetGspn, null);
                EvaluatedFormula eTo = vma.rangeTo.evaluate(context, EvaluationArguments.NO_ARGS);
                vma.rangeStep.checkExprCorrectness(context, page.targetGspn, null);
                EvaluatedFormula eStep = vma.rangeStep.evaluate(context, EvaluationArguments.NO_ARGS);
                
                if (vma.type == TemplateVariable.Type.INTEGER) {
                    // Integer loop
                    int from = eFrom.getScalarInt(), to = eTo.getScalarInt(), step = eStep.getScalarInt();
                    if ((from < to && step <= 0) || (from > to && step > 0))
                        throw new MeasureEvaluationException("Cannot evaluate integer range assigned to <"+vma.varName+
                                                             "> from "+from+" to "+to+" with step "+step);
                    for (int i=from; i<=to; i+=step) {
                        Expr expr = vma.createExpr(""+i);
                        expr.checkExprCorrectness(context, page.targetGspn, null);
                        context.templateVarsBinding.bindSingleValue(vma.varName, expr);
                        bindParams(param+1);
                    }
                }
                else if (vma.type == TemplateVariable.Type.REAL) {
                    // Floating point loop
                    double from = eFrom.getScalarReal(), to = eTo.getScalarReal(), step = eStep.getScalarReal();
                    if ((from < to && step <= 0) || (from > to && step > 0))
                        throw new MeasureEvaluationException("Cannot evaluate real range assigned to <"+vma.varName+
                                                             "> from "+from+" to "+to+" with step "+step);
                    for (double i=from; i<=to; i+=step) {
                        Expr expr = vma.createExpr(""+i);
                        expr.checkExprCorrectness(context, page.targetGspn, null);
                        context.templateVarsBinding.bindSingleValue(vma.varName, expr);
                        bindParams(param+1);
                    }
                }
                else throw new UnsupportedOperationException("Only integer/real ranges are supported.");
                
                break;
            default:
                throw new UnsupportedOperationException();
        }
        context.templateVarsBinding.unbind(vma.varName);
    }
    
    //==========================================================================
    // Interface that should be implemented by the tool-specific solver classes
    //==========================================================================
    
    abstract void prepareStepForBinding(SolutionStep step, TemplateBinding currBind,
                                        AbstractMeasure[] measures) throws IOException;
    
    abstract void readCommand(String command, int stepNum, AbstractMeasure[] measures);
    
    abstract void modifyEnvironmentVars(Map<String, String> env);
    
    abstract void startOfStep(SolutionStep step);
    
    abstract void endOfStep(SolutionStep step, boolean interrupted, boolean allStepsCompleted);
    
    abstract boolean enableSupportForMDepArcsInNetDef();
    
    //==========================================================================
    
    private void computeMeasuresWithBinding() 
            throws MeasureEvaluationException, IOException
    {
        SolutionStep step = new SolutionStep(steps.size(), this);
        TemplateBinding currBind = (TemplateBinding)Util.deepCopy(context.templateVarsBinding);
        
        // Re-check expression correctness (lost during deepCopy())
        for (Expr expr : currBind.binding.values())
            expr.checkExprCorrectness(context, page, null);
        
        prepareStepForBinding(step, currBind, measures);
        steps.add(step);
        
//        System.out.println("CMDLINE: "+step.cmdLine);
    }

    public SolverDialog getDlg() {
        return dlg;
    }

    public MeasurePage getPage() {
        return page;
    }

    public ParserContext getContext() {
        return context;
    }

    public File getProjectFile() {
        return projectFile;
    }

    public File getSolutionDir() {
        return solutionDir;
    }

    public File getGspnFile() {
        return gspnFile;
    }

    public int getCurrStepNum() {
        return currStepNum;
    }
    
    //==========================================================================
    // Wrapping of filenames
    //==========================================================================
    
    public static String makeFilenameForCmd(String fname, String ext) {
        String fn = fname + (ext==null ? "" : ext);
        if (Util.useWSL()) {
            // step 1:    C:  ->  /mnt/c/
            if (fn.length()>2 && fn.charAt(1)==':') {
                char devLetter = Character.toLowerCase(fn.charAt(0));
                fn = "/mnt/"+devLetter+"/"+fn.substring(2);
            }
            // step 2:   replace backslash with slash
            fn = fn.replace("\\", "/");
        }
//        else if (Main.useAppImage() && Util.isWindows()) { // CYGWIN
//            // step 1:    C:  ->  /cygdrive/c/
//            if (fn.length()>2 && fn.charAt(1)==':') {
//                char devLetter = Character.toLowerCase(fn.charAt(0));
//                fn = "/cygdrive/"+devLetter+"/"+fn.substring(2);
//            }
//            // step 2:   replace backslash with slash
//            fn = fn.replace("\\", "/");
//        }
        return fn;        
    }
    public static String makeFilenameForCmd(String fname) {
        return makeFilenameForCmd(fname, null);
    }
    public static String makeFilenameForCmd(File fname) {
        return makeFilenameForCmd(fname.getAbsolutePath(), null);
    }

    protected String makeDefaultModelFilenameForCmd(String ext) {
        return makeFilenameForCmd(getGspnFile().getAbsolutePath(), ext);
    }

    //==========================================================================
    // Helper methods for derived solver classes
    //==========================================================================

    // Compose the argument list with the parametric mark/rate parameters
    protected ArrayList<String> getParamBindingCmdArgs(TemplateBinding currBind, boolean writeMarkPars, boolean writeRatePars) {
        ArrayList<String> cmdArgs = new ArrayList<>();
        // Prepare the bindings
        for (Map.Entry<String, Expr> bind : currBind.binding.entrySet()) {
            TemplateVariable var = (TemplateVariable)getPage().targetGspn.getNodeByUniqueName(bind.getKey());
            if (var.getType() == TemplateVariable.Type.INTEGER && writeMarkPars) {
                int value = bind.getValue().evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarInt();
                cmdArgs.add("-mpar");
                cmdArgs.add(var.getUniqueName());
                cmdArgs.add(""+value);
            }
            else if (var.getType() == TemplateVariable.Type.REAL && writeRatePars) {
                double value = bind.getValue().evaluate(getContext(), EvaluationArguments.NO_ARGS).getScalarRealOrIntAsReal();
                cmdArgs.add("-rpar");
                cmdArgs.add(var.getUniqueName());
                cmdArgs.add(""+value);
            }
        }
        return cmdArgs;
    }
    
    // Standard readCommand implementation
    protected void defaultReadCommand(String commandLine, int stepNum, AbstractMeasure[] measures) {
        assert stepNum >= 0;
        assert measures != null;
        assert commandLine != null;
        
//        System.out.print(stepNum+ ": Reading command: "+commandLine);
        
        Scanner scanner = new Scanner(commandLine);
        if (!scanner.hasNext())
            return;
        
        try {
            String command = scanner.next();
            switch (command) {
                case "RESULT":
                    // Get the measure name
                    String measName = scanner.next();
                    // Search if such measure exists
                    for (AbstractMeasure meas : measures) {
                        if (meas.getResults().table.get(stepNum).entryName.equals(measName)) {
                            ResultEntry entry = meas.getResults().table.get(stepNum);
                            if (!entry.isNotYetComputed() && !entry.isMultiLine()) {
                                asyncLogStderr("MULTIPLE RESULTS FOR MEASURE: "+measName);
                                return;
                            }
                            // Let the entry itself read the result from the scanner
                            entry.parseResult(scanner, page.targetGspn);
                            return;
                        }
                    }
                    // The GreatSPN toolchain always computes the ALL measure, so silently
                    // avoid skip the results if there is no ALL measure defined.
                    // Same goes for statistics, which can be safely ignored if the user 
                    // did not select a STAT target.
                    if (measName.equals("ALL") || measName.equals("STAT"))
                        return;
                    asyncLogStderr("UNKNOWN MEASURE: "+measName);
                    break;

                case "END_STEP":
                    // Mark all the uncomputed measures as failed
                    for (AbstractMeasure meas : measures) {
                        if (meas.getResults().table == null ||
                            stepNum >= meas.getResults().table.size())
                            continue;
                        if (meas.getResults().table.get(stepNum).isNotYetComputed())
                            meas.getResults().table.get(stepNum).setComputeFailed("<not computed>");
                    }   
                    break;

                default:
                    asyncLogStderr("UNKNOWN COMMAND: "+command);
                    break;
            }
        }
        catch (NoSuchElementException e) {
            // May happen if the output has been truncated (by tool interruption), 
            // or if the tool is sending a misformatted output.
            if (numReportedNoSuchElemError < 5) {
                asyncLogStderr("TRUNCATED TOOL OUTPUT.");
            }
            else if (numReportedNoSuchElemError == 5) {
                asyncLogStderr("TRUNCATED TOOL OUTPUT. Other errors of this kind will be silently suppressed.");
            }
            numReportedNoSuchElemError++;
        }
    }
    
    //==========================================================================
    // GreatSPN tool installation directory finder
    //==========================================================================

    public static String useGreatSPN_binary(String binName) {
        String cmd;
        if (Util.useAppImage()) {
            // Use the portable GreatSPN app-image distribution
            cmd = Main.getAppImageGreatSPN_dir().getAbsolutePath() + File.separator + "bin" + File.separator + binName;
            if (Util.isWindows())
                cmd += ".exe";
        }
        else {
            // Use the system-level GreatSPN
            cmd = Main.getPathToGreatSPN() + File.separator + "bin" + File.separator + binName;
        }
        // Append the command to the step.cmd string
        if (Util.useAppImage()) { // Using portable cygwin-based GreatSPN
            if (!new File(cmd).canExecute()) {
                throw new IllegalStateException("The path to the "+binName+" solver of GreatSPN is not set.\n"
                        + "Verify that the GreatSPN installation is not damaged.");
            }
        }
        else if (Util.useWSL()) { // Using WSL
            if (!Util.checkWSL()) { // Verify that the file exists in the WSL subsystem
                throw new IllegalStateException("Windows Subsystem for Linux (WSL) is required "
                        + "to run GreatSPN on Windows.\nInstall WSL and GreatSPN and retry.");
            }
            if (!Util.checkWSLcanExecute(cmd))
                throw new IllegalStateException("The path to the "+binName+" solver of GreatSPN is not set.\n"
                        + "Check that the GreatSPN path is set correctly (from the Edit > Options menu) and "
                        + "verify that the GreatSPN installation in the Windows Subsystem for Linux (WSL) "
                        + "is not damaged.");
        }
        else { // On Unix verify the file directly
            if (!new File(cmd).canExecute()) {
                throw new IllegalStateException("The path to the "+binName+" solver of GreatSPN is not set.\n"
                        + "Check that the GreatSPN path is set correctly (from the Edit > Options menu) and "
                        + "verify that the GreatSPN installation is not damaged.");
            }
        }
        return cmd;
    }
    
    public static ArrayList<String> startOfCommand() {
        ArrayList<String> cmd = new ArrayList<>();
        if (Util.useWSL()) {
            cmd.add("wsl");
            cmd.add("--");
        }
        return cmd;
    }
    
    // Does the command line argument @arg need quotes to be run without problems?
    // NOTE: we only allows alphanumeric & '-' - everything else becomes quoted.
    private static boolean argumentNeedQuotes(String arg) {
        for (int i=0; i<arg.length(); i++) {
            char ch = arg.charAt(i);
            if (!NetObject.isAsciiLetterOrDigit(ch) && ch!='-')
                return true;
        }
        return false;
    }
    
    public static String cmdToString(ArrayList<String> cmd) {
        StringBuffer sb =  new StringBuffer();
        for (String s : cmd) {
            if (argumentNeedQuotes(s))
                sb.append("\"").append(s).append("\"");
            else
                sb.append(s);
            sb.append(" ");
        }
        return sb.toString();
    }
    
    public static ArrayList<String> createEmptyFileCmd(String filename) {
        ArrayList<String> cmd;
        cmd = startOfCommand();
        if (Util.isWindows() && !Util.useWSL()) { // DOS command
            cmd.add("cmd.exe");
            cmd.add("/c");
            cmd.add("COPY");
            cmd.add("NUL");
            cmd.add(filename);            
        }
        else { // Unix command
            cmd.add("/bin/cp");
            cmd.add("/dev/null");
            cmd.add(filename);
        }
        return cmd;
    }
    
    public static ArrayList<String> createCopyFileCmd(String src, String dst) {
        ArrayList<String> cmd;
        cmd = startOfCommand();
        if (Util.isWindows() && !Util.useWSL()) { // DOS command
            cmd.add("cmd.exe");
            cmd.add("/c");
            cmd.add("COPY");
            cmd.add(src);
            cmd.add(dst);            
        }
        else { // Unix command
            cmd.add("/bin/cp");
            cmd.add(src);
            cmd.add(dst);            
        }
        return cmd;
    }
    
    //==========================================================================
    // ADDITIONAL PATHS for the toolchain invokation
    //==========================================================================

    private static final String ADDPATH_DIR = "additional_path_dir";
    public static void setAdditionalPathDir(String path) {
        Util.getPreferences().put(ADDPATH_DIR, path);
    }
    public static String getAdditionalPathDir() {
        return Util.getPreferences().get(ADDPATH_DIR, "");
    }
    
    private static final String ADD_LD_LIBRARY_PATH_DIR = "additional_ld_library_path_dir";
    public static void setAdditionalLibraryPathDir(String path) {
        Util.getPreferences().put(ADD_LD_LIBRARY_PATH_DIR, path);
    }
    public static String getAdditionalLibraryPathDir() {
        return Util.getPreferences().get(ADD_LD_LIBRARY_PATH_DIR, "/usr/local/lib");
    }
    
    //==========================================================================
    private static String addToPath(String path, String dir) {
        if (path.isEmpty())
            return dir;
        else if (dir.isEmpty())
            return path;
        else return path + File.pathSeparator + dir;
    }
    //==========================================================================
    public static String[] prepareRuntimeEnvironmentVars() {
        return prepareRuntimeEnvironmentVars(null);
    }

    public static String[] prepareRuntimeEnvironmentVars(SolverInvokator solver) {
        // Prepare the environment variables
        Map<String, String> env = System.getenv(), nenv = new HashMap<>();
        for (Entry<String, String> e : env.entrySet())
            nenv.put(e.getKey(), e.getValue());
        nenv.put("TERM", "xterm");   // Accept VT-100 escape sequences
        nenv.put("FREQUENCY", "10"); // Frequency of terminal updates, in tenths of seconds
        nenv.put("FROM_GUI", "1");   // Tell the solver that is being called from the GUI
        
        if (Util.useWSL()) {
//            nenv.put("LD_LIBRARY_PATH", "/usr/local/lib");
            nenv.put("WSLENV", "TERM:FREQUENCY:FROM_GUI:LD_LIBRARY_PATH");
        }
//        if (Util.isWindows() && Main.isAppImageDistribution()) {
//            nenv.put("PAR_GLOBAL_TEMP ",  System.getProperty("java.io.tmpdir"));
//        }
        
        // Add missing variables
        final String LD_LIBRARY_PATH = Util.isOSX() ? "DYLD_LIBRARY_PATH" : "LD_LIBRARY_PATH";
        boolean hasPath = false;
        boolean hasLibraryPath = false;
        for (Entry<String, String> e : nenv.entrySet()) {
            if (e.getKey().equalsIgnoreCase("PATH")) // ignore case is needed in Windows
                hasPath = true;
            if (e.getKey().equalsIgnoreCase(LD_LIBRARY_PATH))
                hasLibraryPath = true;
        }         
        if (!hasPath)
            nenv.put("PATH", "");
        if (!hasLibraryPath && !Util.isWindows())
            nenv.put(LD_LIBRARY_PATH, "");

        // Add AppImage root
        if (Util.useAppImage() && !nenv.containsKey("GREATSPN_APPIMAGE_DIR")) {
            nenv.put("GREATSPN_APPIMAGE_DIR", Main.getAppImageGreatSPN_dir().getAbsolutePath());
        }
        if (/*Util.useAppImage() &&*/ !nenv.containsKey("GREATSPN_TEMP_DIR")) {
            // Pass the custom TEMP directory. Ensure that is ended by the path separator.
            // NOTE: this is used by the portable_mkstemp() function in platform_utils.c
            File tmpDir = new File(System.getProperty("java.io.tmpdir"));
            String tmpDirAbs = tmpDir.getAbsolutePath();
//            if (!tmpDirAbs.endsWith(File.separator))
//                tmpDirAbs += File.separator;
            nenv.put("GREATSPN_TEMP_DIR", tmpDirAbs);
//            System.out.println("tmpDir.getAbsolutePath()=" + tmpDirAbs);
        }
        
        if (solver != null)
            solver.modifyEnvironmentVars(nenv);
        
        String[] envp = new String[nenv.size()];
        int pos = 0;
        for (Entry<String, String> e : nenv.entrySet()) {
            String value = e.getValue();
            if (e.getKey().equalsIgnoreCase("PATH")) {
                if (!getAdditionalPathDir().isEmpty())
                    value = addToPath(value, getAdditionalPathDir());
                if (Util.isOSX() && Main.isAppImageDistribution()) {
                    // FIX: currently the macOS bundle starts with a very basic PATH.
                    // Add some common directories
                    value = addToPath(value, Main.getAppImageGreatSPN_dir().getAbsolutePath() + "/bin");
                    value = addToPath(value, "/usr/local/bin");
                    value = addToPath(value, "/opt/homebrew/bin");
                    value = addToPath(value, "/opt/local/bin");
                }
//                System.out.println("PATH="+value);
//                JOptionPane.showMessageDialog(null, "PATH="+value);

            }
            if (e.getKey().equalsIgnoreCase(LD_LIBRARY_PATH)) {
                if (!getAdditionalLibraryPathDir().isEmpty()) {
                    value = addToPath(value, getAdditionalLibraryPathDir());
                }
                if (Util.useAppImage()) {
                    value = addToPath(value, 
                            Main.getAppImageGreatSPN_dir().getAbsolutePath()+File.separator+"lib");
                }
//                System.out.println(LD_LIBRARY_PATH+"="+value);
            }
            envp[pos++] = e.getKey()+"="+value;
        }
        
//        for (String envset : envp)
//            System.out.println("export "+envset);
        
        return envp;
    }
    
    private static final String RGMEDD_EXECNAME = "rgmedd_exec_name";
    public static void setRGMEDDName(String path) {
        Util.getPreferences().put(RGMEDD_EXECNAME, path);
    }
    public static String getRGMEDDName() {
        return Util.getPreferences().get(RGMEDD_EXECNAME, "RGMEDD3");
    }
    
    //==========================================================================
    // Cosmos tool installation directory finder
    //==========================================================================

    private static final String COSMOS_DIR = "cosmos_dir";
    public static void setPathToCosmos(String path) {
        Util.getPreferences().put(COSMOS_DIR, path);
    }
    public static String getPathToCosmos() {
        return Util.getPreferences().get(COSMOS_DIR, "/usr/local/Cosmos");
    }
    
    public static String useCosmos_binary(String binName) {
        // Append the command to the step.cmd string
        String cmd = getPathToCosmos() + "/bin/" + binName;
        if (!new File(cmd).canExecute()) {
            throw new IllegalStateException("The path to the "+binName+" solver of Cosmos is not set.\n"
                    + "Check that the Cosmos path is set correctly (from the Edit > Options menu) and"
                    + "verify that the Cosmos installation is not damaged.");
        }
        return cmd;
    }
}
