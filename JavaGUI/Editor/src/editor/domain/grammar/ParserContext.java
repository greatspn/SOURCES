/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.grammar;

import editor.domain.values.EvaluatedFormula;
import editor.Main;
import editor.domain.elements.DtaSignature;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Stack;
import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.BailErrorStrategy;
import org.antlr.v4.runtime.BaseErrorListener;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.Recognizer;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.misc.ParseCancellationException;
import org.antlr.v4.runtime.tree.ParseTree;

/** Contains the state of the context where expressions are parsed for correctness.
 *
 * @author elvio
 */
public class ParserContext implements NodeNamespace {
    
    // ============== Parse flags ===================
    // Do not accept marking-dependent expressions (evaluation of constants)
    public static final int PF_CONST_EXPR = 1;
    // Untimed evaluation (clock constraints are always true)
    public static final int PF_UNTIMED_SUMULATION = 2;
    // Do not consider static subclass IDs as typed IDs
    public static final int PF_NO_STATIC_SUBCLASS_ID = 4;
    // Consider CTL/LTL keywords (A, E, X, F, G)
    public static final int PF_CTL_KEYWORDS = 8;
    public static final int PF_LTL_KEYWORDS = 16;
    public static final int PF_CTLSTAR_KEYWORDS = 32;
    // Consider special function for the description of probability
    // distribution functions of general events
    public static final int PF_GENERAL_FUNCTION_KEYWORDS = 64;
    // Clock variables may appear in a realExpr
    public static final int PF_CLOCK_VARS_IN_EXPR = 128;
    // Accept empty text
    public static final int PF_ALLOW_EMPTY_EXPR = 256;
    
    // Verbatim text starts with this prefix
    public static final String VERBATIM_PREFIX = "FN:";
    
    // Namespace where nodes are look-up by name. Typically, it will be a NetPage.
    // Known place names, int and real constants of the model (GSPN, DTA, ...)
    private final NodeNamespace nodeNamespace;
    
    // List of known DTA names that can be referenced
    public DtaSignature[] knownDtas;
    public Stack<DtaSignature> dtaSigStack;
        
    // Found errors during parse?
    private ArrayList<String> foundErrors = null;
    private String whatIsParsing;
    
    // Should we collect the color vars encountered?
    public Set<ColorVar> colorVarsInUse = null;
    
    // Do not evaluate mark/rate parameters, keep unevaluated.
    public boolean partialEvaluation = false;
    
    // Special mode for the evaluation of bounds of probability density functions
    public boolean evaluatePdfBounds = false;
    
    // Special mode to draw samples from statistical distributions of the general transitions
    public boolean drawStatisticalDistribSamples = false;
    
    // ======= Fields used only by the FormulaEvaluator parser ======
    
    // Evaluation stack - avoids infinite evaluation depth
    Stack<String> evalStack;
    public static final int MAX_STACK_DEPTH = 100;
    
    // Current joint state of a GSPN and a DTA (current marking and DTA location)
    //public JointState jointState;
    
    // The node that has fired (null if this is not a transition evalutation context).
    //public Node firedNode;
    
    // Should we cache evaluated results
    public Map<String, EvaluatedFormula> evaluationCache = null;
    public Map<String, String> reduceCache = null;
    
    // ============== Binding of template variables =============
    
    // Context where binded variables are evaluated
    public ParserContext bindingContext;
    
    // Current binding of the template variables
    public TemplateBinding templateVarsBinding;
    
    
    // ======= ColorDomain of the parsed expression =======

    public ColorClass colorDomainOfExpr = null;

    // ======= "This" entry used by multiset filter expressions =======
    
    public DomainElement filterThisDomain = null;
    public EvaluatedFormula filterThisValue = null;

    // ================ State of the ANTLR parser ===============

    // Parser object, available when building the parse tree.
    private ExprLangParser parser;

    
    
    public ParserContext(ProjectPage page) {
        assert page == null || page instanceof NodeNamespace;
        this.nodeNamespace = (NodeNamespace)page;
    }
    
//    // Evaluate a simple constant node, with chaining
//    public EvaluatedFormula evaluateConst(String name, ConstantID.ConstType type) {
//        ConstantID c = (ConstantID)page.getNodeByUniqueName(name);
//        if (c != null) {
//            if (c.getConstType() != type)
//                throw new EvaluationException("Constant "+name+" should have type "+
//                                              type+" instead of "+c.getConstType());
//            return c.evaluate(this);
//        }
//        else if (chainedContext != null) {
//            return chainedContext.evaluateConst(name, type);
//        }
//        
//        throw new EvaluationException("Constant "+name+" is not defined.");
//    } 
    
    @Override
    public Node getNodeByUniqueName(String id) {
        return nodeNamespace.getNodeByUniqueName(id);
    }
    
    @Override
    public Iterator<ColorClass> colorClassIterator() {
        return nodeNamespace.colorClassIterator();
    }
    
    public boolean hasNamespacePage() {
        return (nodeNamespace != null);
    }
   
    public enum ParserEntryPoint {
        SKIP_PARSE, VERBATIM_TEXT, VERBATIM_TEXT_REMOVE_PREFIX,
        INT_EXPR, REAL_EXPR, BOOLEAN_EXPR, INT_OR_INFINITE_EXPR,
        INT_LIST_EXPR, REAL_LIST_EXPR,
        INT_MULTISET_EXPR, REAL_MULTISET_EXPR,
        COLOR_CLASS_DEF, COLOR_VAR_DEF, PLACE_COLOR_DOMAIN_DEF,
        CLOCK_CONSTRAINT, ACTION_SET, STATE_PROP_EXPR, VAR_FLOW,
        ACTION_BINDING, CLOCK_DEF,
        PERFORMANCE_MEASURE, CSLTA_EXPR, TAG_REWRITE_RULES_LIST
    }
    
    // Generate a proper parse tree for a formula in a given parser entry point
    private ParseTree generateParseTreeFor(String formulaText, ParserEntryPoint entry, 
                                           int parseFlags, BaseErrorListener errListener) 
    {
        CharStream stream = new ANTLRInputStream(formulaText);
        ExprLangLexer lexer = new ExprLangLexer(stream);
        TokenStream tokens = new CommonTokenStream(lexer);
        parser = new ExprLangParser(tokens);
        lexer.context = parser.context = this;
        lexer.parseFlags = parser.parseFlags = parseFlags;
        parser.clockVarInExpr = ((parser.parseFlags & PF_CLOCK_VARS_IN_EXPR) != 0);

        parser.setBuildParseTree(true);
        parser.removeErrorListeners(); // remove ConsoleErrorListener
        parser.addErrorListener(errListener);
        parser.setErrorHandler(new BailErrorStrategy());
        ParseTree tree; 
        switch (entry) {
            case INT_EXPR:
                tree = parser.mainIntExpr();
                break;
            case REAL_EXPR:
                tree = parser.mainRealExpr();
                break;
            case BOOLEAN_EXPR:
                tree = parser.mainBoolExpr();
                break;
            case INT_OR_INFINITE_EXPR:
                tree = parser.mainIntOrInfiniteExpr();
                break;
            case INT_LIST_EXPR:
                tree = parser.mainIntExprList();
                break;
            case REAL_LIST_EXPR:
                tree = parser.mainRealExprList();
                break;
            case CLOCK_CONSTRAINT:
                tree = parser.mainClockConstr();
                break;
            case ACTION_SET:
                tree = parser.mainActSet();
                break;
            case STATE_PROP_EXPR:
                tree = parser.mainStatePropExpr();
                break;
            case VAR_FLOW:
                tree = parser.mainVarFlow();
                break;
            case ACTION_BINDING:
                tree = parser.mainActBinding();
                break;
            case CLOCK_DEF:
                tree = parser.mainClockDef();
                break;
            case PERFORMANCE_MEASURE:
                tree = parser.mainMeasure();
                break;
            case CSLTA_EXPR:
                tree = parser.mainCsltaExpr();
                break;
            case COLOR_CLASS_DEF:
                tree = parser.mainColorClassDef();
                break;
            case COLOR_VAR_DEF:
                tree = parser.mainColorVarDef();
                break;
            case PLACE_COLOR_DOMAIN_DEF:
                tree = parser.mainPlaceColorDomainDef();
                break;
            case INT_MULTISET_EXPR:
                tree = parser.mainIntMSetExpr();
                break;
            case REAL_MULTISET_EXPR:
                tree = parser.mainRealMSetExpr();
                break;
            case TAG_REWRITE_RULES_LIST:
                tree = parser.mainTagRewriteList();
                break;
            default:
                throw new IllegalStateException("Invalid parser entry point.");
        }
        parser = null;
        return tree;
    }
    
    private class ParseErrorListener extends BaseErrorListener {
        @Override
        public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, 
                                int charPositionInLine, String msg, RecognitionException e) 
        {
//            List<String> stack = ((Parser)recognizer).getRuleInvocationStack(); 
//            Collections.reverse(stack); 
//            System.err.println("rule stack: "+stack); 
//            System.err.println("line "+line+":"+charPositionInLine+" at "+
//                               offendingSymbol+": "+msg);
            addNewError("Position "+line+":"+charPositionInLine+": "+msg);
        }
    }
    private ParseErrorListener parseErrListener = null;
    private ParseErrorListener getParseErrListener() {
        if (parseErrListener == null)
            parseErrListener = new ParseErrorListener();
        return parseErrListener;
    }
    
    
    // Parse a formula
    public FormattedFormula parseFormula(String formulaText, ParserEntryPoint entry, 
                                         String whatIsParsing, int parseFlags) 
    {
        this.whatIsParsing = whatIsParsing;
        foundErrors = null;
        FormattedFormula result = null;
        
        if (entry == null) { // Unspecified grammar entry point
            addNewError("Cannot parse.");
        }
        else if (entry == ParserEntryPoint.SKIP_PARSE) {
            // Silently skip parsing this expression.
        }
        else if (entry == ParserEntryPoint.VERBATIM_TEXT || 
                 entry == ParserEntryPoint.VERBATIM_TEXT_REMOVE_PREFIX) 
        {
            String txt = formulaText;
            if (entry == ParserEntryPoint.VERBATIM_TEXT_REMOVE_PREFIX && txt.startsWith(VERBATIM_PREFIX))
                txt = txt.substring(VERBATIM_PREFIX.length());
            return new FormattedFormula(ExpressionLanguage.LATEX, true, "\\textbf{"+txt+"}");
        }
        else if (!hasNamespacePage()) { // Missing namespace in evaluation context
            addNewError("Missing evaluation context.");
        }
        else if ((formulaText == null || formulaText.length() == 0) && 
                 (parseFlags & PF_ALLOW_EMPTY_EXPR) == 0) 
        {
            addNewError("The expression has no text.");
        }
        else try {
            ParseTree tree = generateParseTreeFor(formulaText, entry, parseFlags, getParseErrListener());
            ExprLangVisitor<FormattedFormula> semanticParser = new SemanticParser(this, parseFlags);
            result = semanticParser.visit(tree);
        }
        catch (ExprLangParserException e) {
//            System.err.println("ExprLangParserException: "+e.getMessage());
            // Runtime parser exception - add a new error to the error list
            addNewError(e.getMessage());
        }
        catch (ParseCancellationException e) {
            // Bail-out strategy of the parser - do nothing.
            // The syntax error has already been inserted by the error listener.
            if (e.getCause() != null) {
                // Generated by the BailOut strategy
                RecognitionException re = (RecognitionException)e.getCause();
                Token tok = re.getOffendingToken();
                if (tok != null)
                    addNewError("Parse error at <"+tok.getText()+">, position "
                                +tok.getLine()+":"+tok.getStartIndex()+"-"+tok.getStopIndex()+".");
                else
                    addNewError("Incomplete expression.");
            }
        }
        catch (Exception e) {
            Main.logException(e, true);
            addNewError("Invalid expression.");
        }
        
        this.whatIsParsing = null;
        this.colorDomainOfExpr = null;
        this.colorVarsInUse = null;
        return result;
    }
    
    public boolean hasParseErrors() {
        return (foundErrors != null);
    }
    
    // Format any generated error
    public PageErrorWarning[] formatErrors(ProjectPage page, Selectable selObj) {
        assert hasParseErrors();
        
        PageErrorWarning[] allErrs = new PageErrorWarning[foundErrors.size()];
        for (int i=0; i<foundErrors.size(); i++)
            allErrs[i] = PageErrorWarning.newError(foundErrors.get(i), selObj);
        
        return allErrs;
    }
    
    // Insert a new error into the current parse context
    public void addNewError(String text) {
        if (foundErrors == null)
            foundErrors = new ArrayList<>();
        
        foundErrors.add("Invalid "+whatIsParsing+" "+text);
    }
    
    
    
    private class ThrowingErrorListener extends BaseErrorListener {
        @Override
        public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, 
                                int charPositionInLine, String msg, RecognitionException e) 
        {
//            List<String> stack = ((Parser)recognizer).getRuleInvocationStack(); 
//            Collections.reverse(stack); 
//            System.err.println("rule stack: "+stack); 
            System.err.println("line "+line+":"+charPositionInLine+" at "+
                               offendingSymbol+": "+msg);
            throw new EvaluationException("Position "+line+":"+charPositionInLine+": "+msg);
        }
    }
    private ThrowingErrorListener throwingErrListener = null;
    private ThrowingErrorListener getThrowingErrListener() {
        if (throwingErrListener == null)
            throwingErrListener = new ThrowingErrorListener();
        return throwingErrListener;
    }
    
    
    // Evaluate a formula
    public EvaluatedFormula evaluate(String formulaText, ParserEntryPoint entry,
                                     EvaluationArguments args,
                                     String stackRecordName, int parseFlags)
    {
        String cacheKey = null;
        if (evaluationCache != null) {
            cacheKey = args.toString() + formulaText + "@@" + parseFlags + " " + entry;
            EvaluatedFormula val = evaluationCache.get(cacheKey);
            if (val != null) {
                return val;
            }
        }
        if (evalStack == null)
            evalStack = new Stack<>();
        evalStack.push(stackRecordName);
        if (evalStack.size() > MAX_STACK_DEPTH)
            throw new EvaluationException("Evaluation stack depth is too high.");
        
        EvaluatedFormula result = null;
        
        try {
            ParseTree tree = generateParseTreeFor(formulaText, entry, parseFlags, getThrowingErrListener());

            ExprLangVisitor<EvaluatedFormula> formatter = new FormulaEvaluator(this, args);

            result = formatter.visit(tree);
            
//            System.out.println("("+evalStack.size()+") evaluate("+formulaText+") = "+result);
        }
        catch (EvaluationException e) {
            throw e;
        }
        catch (Exception e) {
            Main.logException(e, true);
            throw new EvaluationException("Could not evaluate: "+stackRecordName);
        }
        finally {
            evalStack.pop();
        }
        if (evaluationCache != null) { // save the result for later use
            evaluationCache.put(cacheKey, result);
        }
        
        return result;
    }

    // Can be called only during parse tree generation.
    public ExprLangParser getParser() {
        assert parser != null;
        return parser;
    }
    
    
    // Evaluate a formula
    public String reduce(String formulaText, ParserEntryPoint entry,
                         EvaluationArguments args,
                         String stackRecordName, int parseFlags,
                         ExpressionLanguage lang, boolean applyReduce)
    {
        String cacheKey = null;
        if (reduceCache != null) {
            cacheKey = args.toString() + formulaText + "@@" + parseFlags + " " + entry + "+"+lang+applyReduce;
            String val = reduceCache.get(cacheKey);
            if (val != null) {
                return val;
            }
        }

        if (evalStack == null)
            evalStack = new Stack<>();
        evalStack.push(stackRecordName);
        if (evalStack.size() > MAX_STACK_DEPTH)
            throw new EvaluationException("Evaluation stack depth is too high.");
        
        String result = null;
        
        if (entry == ParserEntryPoint.VERBATIM_TEXT || 
            entry == ParserEntryPoint.VERBATIM_TEXT_REMOVE_PREFIX) 
        {
            // Verbatim expressions (like transitions with "FN:<text>" rate) cannot
            // be reduced and are returned as-is.
            result = formulaText;
        }
        else {
            try {
                ParseTree tree = generateParseTreeFor(formulaText, entry, parseFlags, getThrowingErrListener());

                ExprLangVisitor<FormattedFormula> reducer = new FormulaReducer(this, parseFlags, args, lang, applyReduce);
                result = reducer.visit(tree).getFormula();

    //            System.out.println("("+evalStack.size()+") evaluate("+formulaText+") = "+result);
            }
            catch (EvaluationException e) {
                throw e;
            }
            catch (Exception e) {
                Main.logException(e, true);
                throw new EvaluationException("Could not evaluate: "+stackRecordName);
            }
            catch (Throwable t) { // In case of assertions or other unexpected exceptions
                System.out.println("Unrecoverable error while reducing: "+formulaText);
                throw t;
            }
        }
        
        if (reduceCache != null)
            reduceCache.put(cacheKey, result);
        
//        System.out.println("Reducing: "+formulaText+"\n   with: "+args.colorVarsBinding);
//        System.out.println("   ==> "+result);
        
        evalStack.pop();
        return result;
    }
    
    // Evaluate a formula
    public String dropSubterms(String formulaText, ParserEntryPoint entry,
                               Set<String> knownColorVars,
                               String stackRecordName, int parseFlags,
                               ExpressionLanguage lang)
    {
        if (evalStack == null)
            evalStack = new Stack<>();
        evalStack.push(stackRecordName);
        if (evalStack.size() > MAX_STACK_DEPTH)
            throw new EvaluationException("Evaluation stack depth is too high.");
        
        String result = null;
        
        if (entry == ParserEntryPoint.VERBATIM_TEXT || 
            entry == ParserEntryPoint.VERBATIM_TEXT_REMOVE_PREFIX) 
        {
            // Verbatim expressions (like transitions with "FN:<text>" rate) cannot
            // be reduced and are returned as-is.
            result = formulaText;
        }
        else {
            try {
                ParseTree tree = generateParseTreeFor(formulaText, entry, parseFlags, getThrowingErrListener());

                ExprLangVisitor<FormattedFormula> dropper = new FormulaDropper(this, parseFlags, lang, knownColorVars);
                result = dropper.visit(tree).getFormula();

    //            System.out.println("("+evalStack.size()+") evaluate("+formulaText+") = "+result);
            }
            catch (EvaluationException e) {
                throw e;
            }
            catch (Exception e) {
                Main.logException(e, true);
                throw new EvaluationException("Could not evaluate: "+stackRecordName);
            }
            catch (Throwable t) { // In case of assertions or other unexpected exceptions
                System.out.println("Unrecoverable error while reducing: "+formulaText);
                throw t;
            }
        }
        
        evalStack.pop();
        return result;
    }
    
    // Rewrite a formula
    public String rewrite(String formulaText, ParserEntryPoint entry, 
                         int parseFlags, ExprRewriter rewriter) 
    {
        try {
            ParseTree tree = generateParseTreeFor(formulaText, entry, parseFlags, getThrowingErrListener());
            rewriter.startRewriting();
            rewriter.visit(tree);
            String newFormula = rewriter.getRewrittenExpr(formulaText);
            return newFormula;
        }
        catch (EvaluationException e) {
            throw e;
        }
        catch (ParseCancellationException e) {
            // There is something that has made the expression non parseable (like a
            // change of the initial rule). This may happen, for instance, when the arc
            // multiplicity expression passes from a neutral place to a colored place,
            // such that an integer expression is not a color tuple.
            // This triggers a parse error, which in this context just means non rewritable.
            throw new EvaluationException("Parse cancellation. Could not rewrite: "+formulaText);
        }
        catch (Exception e) {
            Main.logException(e, true);
            throw new EvaluationException("Could not rewrite: "+formulaText);
        }
    }
    
    // Visit a formula tree using a custom ANTLR visitor
    public <T> T visitTree(String formulaText, ParserEntryPoint entry, 
                              int parseFlags, ExprLangBaseVisitor<T> visitor) 
            throws EvaluationException
    {
        try {
            ParseTree tree = generateParseTreeFor(formulaText, entry, parseFlags, getThrowingErrListener());
            T obj = visitor.visit(tree);
            return obj;
        }
        catch (EvaluationException e) {
            throw e;
        }
        catch (ParseCancellationException e) {
            throw new EvaluationException("Parse cancellation. Could not visit: "+formulaText);
        }
        catch (Exception e) {
            Main.logException(e, true);
            throw new EvaluationException("Could not visit: "+formulaText);
        }
    }
}
