/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.Decor;
import editor.domain.DrawHelper;
import editor.domain.Edge;
import editor.domain.EditableCell;
import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.LabelDecor;
import editor.domain.ListRenderable;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import static editor.domain.LabelDecor.DEFAULT_SMALL_TEXT_SIZE;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import editor.domain.NetObject;
import static editor.domain.NetObject.GRAYED_BLACK;
import static editor.domain.Node.DEFAULT_ACTIVITY_AURA_SIZE;
import editor.domain.Selectable;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import static editor.domain.grammar.ParserContext.VERBATIM_PREFIX;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.play.AbstractMarking;
import editor.domain.play.ActivityState;
import editor.domain.play.Firable;
import editor.domain.superposition.GroupClass;
import editor.domain.values.ListOfBounds;
import editor.domain.values.RealScalarValue;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.HashSet;
import java.util.Set;
import javax.swing.Icon;
import javax.swing.SwingConstants;
import latex.LatexFormula;
import org.w3c.dom.Element;

/** A transition in a GSPN.
 *
 * @author Elvio
 */
public class Transition extends Node implements Serializable, Firable {

    public enum Type implements ListRenderable {
        EXP("Exponential"), IMM("Immediate"), 
        GEN("General"), CONT("Continuous");
        
        private final String descr;

        private Type(String descr) {
            this.descr = descr;
        }
        @Override public String getDescription() { return descr; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    abstract class TransitionExpr extends Expr {
        public TransitionExpr() { }
        public TransitionExpr(String expr) { super(expr); } 
        @Override
        public void checkExprCorrectness(ParserContext context, ProjectPage page, Selectable selObj) {
            context.colorVarsInUse = colorVarsInUse;
            super.checkExprCorrectness(context, page, selObj); 
        }        
    }
    
    class DelayExpr extends TransitionExpr {
        public DelayExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Delay of transition " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { 
            if (isGeneral()) // enable general function PDF description
                return ParserContext.PF_GENERAL_FUNCTION_KEYWORDS;
            return 0;
        }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            if (isGeneral() && exprText.startsWith(VERBATIM_PREFIX)) {
                return ParserContext.ParserEntryPoint.VERBATIM_TEXT_REMOVE_PREFIX;                
            }
            return ParserContext.ParserEntryPoint.REAL_EXPR; 
        }
    }
    class PriorityExpr extends TransitionExpr {
        public PriorityExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Priority of transition " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.INT_EXPR; 
        }
    }
    class WeightExpr extends TransitionExpr {
        public WeightExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Weight of transition " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.REAL_EXPR; 
        }
    }
    class NumServersExpr extends TransitionExpr {
        public NumServersExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Number of servers of transition " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.INT_OR_INFINITE_EXPR; 
        }
        @Override public String getDefaultEmptyExpr() { return "Infinite"; }
    }
    class GuardExpr extends TransitionExpr {
        public GuardExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Guard of transition " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.BOOLEAN_EXPR; 
        }
//        @Override
//        public void checkExprCorrectness(ParserContext context, ProjectPage page, Selectable selObj) {
//            colorVarsInUse.clear();
//            context.colorVarsInUse = colorVarsInUse;
//            super.checkExprCorrectness(context, page, selObj); 
//        }
        @Override public String getDefaultEmptyExpr() { return "True"; }
    }
    
    Type type = Type.EXP;
    // Rotation of the rectangle, in radiants
    private double rotation = 0.0;
    // Height of the transition rectangle
    private double height = DEFAULT_TRANSITION_HEIGHT;
    
    // Delay of exponential/general transitions
    private final DelayExpr delayExpr = new DelayExpr("1.0");
    private final DelayLabel delayLabel = new DelayLabel();
    
    // Priority (immediate and maybe general transitions)
    private final PriorityExpr priorityExpr = new PriorityExpr("1");    
    private final PriorityLabel priorityLabel = new PriorityLabel();
    
    // Weight of immediate transitions
    private final WeightExpr weightExpr = new WeightExpr("1.0");
    private final WeightLabel weightLabel = new WeightLabel();
    
    // Number of servers of exponential transitions
    private final NumServersExpr numServersExpr = new NumServersExpr("Infinite");
    private final NumServersLabel numServersLabel = new NumServersLabel();
    
    // Transition guard
    private final GuardExpr guardExpr = new GuardExpr("True");
    private final GuardLabel guardLabel = new GuardLabel();
        
    // Superposition tags label
    private String superPosTags = "";
    private final SuperPosTagsLabel superPosTagLabel
            = new SuperPosTagsLabel(DEFAULT_SMALL_TEXT_SIZE, DEFAULT_SUPERPOS_TAGS_POS);
    
    public static final double TIMED_TRANSITION_WIDTH = 0.9;
    public static final double IMMED_TRANSITION_WIDTH = 0.3;
    public static final double DEFAULT_TRANSITION_HEIGHT = 2.0;

    // Color variables that appear in any transition expression
    private transient Set<ColorVar> colorVarsInUse = null;

    // Name of the transition
    final UniqueNameLabel nameLabel = new UniqueNameLabel(DEFAULT_TEXT_SIZE, DEFAULT_UNIQUE_NAME_POS);;
    
    class DelayLabel extends NodeLabelDecor {
        public DelayLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, new Point2D.Double(0.5, +1.0), 
                  SwingConstants.LEFT, SwingConstants.CENTER);
        }
        @Override public String getVisualizedValue() { 
            if (type== Transition.Type.EXP) return delayExpr.getVisualizedExpr();
            if (type== Transition.Type.GEN) {
                if (isGeneralFunctionVerbatim())
                    return delayExpr.getVisualizedExpr();
                return "f_g(x) = " + delayExpr.getVisualizedExpr();
            }
            return delayExpr.getVisualizedExpr();
//            throw new InternalError("Specify how to visualize the transition delay.");
        }
        @Override public Object getValue() { return delayExpr.getExpr(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return delayExpr.isValidExpr(context, (String)value);
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
            delayExpr.setExpr((String)value);
        }
        @Override public boolean isCurrentValueValid() { return delayExpr.isFormattedAndCorrect(); }
        @Override public boolean isEditable() { return hasDelay(); }
        @Override public boolean isVisible(ViewProfile vp)  { 
            return vp.viewRatesDelays && hasDelay(); 
        }
        @Override public boolean editAsMultiline() { return false; }
    }

    class PriorityLabel extends NodeLabelDecor {
        public PriorityLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, new Point2D.Double(0.5, +1.0), 
                  SwingConstants.LEFT, SwingConstants.CENTER);
        }
        @Override public String getVisualizedValue() { return "\\pi\\!=\\!"+priorityExpr.getVisualizedExpr(); }
        @Override public Object getValue() { return priorityExpr.getExpr(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return priorityExpr.isValidExpr(context, (String)value);
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
            priorityExpr.setExpr((String)value);
        }
        @Override public boolean isCurrentValueValid() { return priorityExpr.isFormattedAndCorrect(); }
        @Override public boolean isEditable() { return hasPriority(); }
        @Override public boolean isVisible(ViewProfile vp)  { 
            return vp.viewPriorities && hasPriority() && !priorityExpr.getExpr().equals("1");
        }
        @Override public boolean editAsMultiline() { return false; }
    }
    
    class WeightLabel extends NodeLabelDecor {
        public WeightLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, new Point2D.Double(-0.5, +1.0), 
                  SwingConstants.RIGHT, SwingConstants.CENTER);
        }
        @Override public String getVisualizedValue() { return "w\\!=\\!"+weightExpr.getVisualizedExpr(); }
        @Override public Object getValue() { return weightExpr.getExpr(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return weightExpr.isValidExpr(context, (String)value);
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
            weightExpr.setExpr((String)value);
        }
        @Override public boolean isCurrentValueValid() { return weightExpr.isFormattedAndCorrect(); }
        @Override public boolean isEditable() { return hasWeight(); }
        @Override public boolean isVisible(ViewProfile vp)  { 
            return vp.viewRatesDelays && hasWeight() && !(weightExpr.getExpr().equals("1.0") || weightExpr.getExpr().equals("1"));
        }
        @Override public boolean editAsMultiline() { return false; }
    }
    
    class NumServersLabel extends NodeLabelDecor {
        public NumServersLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, new Point2D.Double(0.5, +2.0), 
                  SwingConstants.LEFT, SwingConstants.CENTER);
        }

        @Override public String getVisualizedValue() { 
            if (numServersExpr.getExpr().equals("1"))
                return "1\\ \\mathrm{server}";
            else if (numServersExpr.getExpr().equals("Infinite"))
                return "\\infty\\ \\mathrm{servers}";
            else
                return numServersExpr.getVisualizedExpr()+"\\ \\mathrm{servers}";
        }
        @Override public Object getValue() { return numServersExpr.getExpr(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return numServersExpr.isValidExpr(context, (String)value);
        }
        @Override public boolean isCurrentValueValid() { return numServersExpr.isFormattedAndCorrect(); }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
            numServersExpr.setExpr((String)value);
        }
        @Override public boolean isEditable() { return hasNumServers(); }
        @Override public boolean isVisible(ViewProfile vp)  { 
            return vp.viewRatesDelays && hasNumServers() && !isInfiniteServers();
        }
        @Override public boolean editAsMultiline() { return false; }
    }
    
    class GuardLabel extends NodeLabelDecor {
        public GuardLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, new Point2D.Double(0.0, +1.5), 
                  SwingConstants.CENTER, SwingConstants.CENTER);
        }
        @Override public String getVisualizedValue() { 
            return "\\left[" + guardExpr.getVisualizedExpr() + "\\right]";
        }
        @Override public Object getValue() { return guardExpr.getExpr(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return guardExpr.isValidExpr(context, (String)value);
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
            guardExpr.setExpr((String)value);
        }
        @Override public boolean isCurrentValueValid() { return guardExpr.isFormattedAndCorrect(); }
        @Override public boolean isEditable() { return hasGuard(); }
        @Override public boolean isVisible(ViewProfile vp)  { 
            return vp.viewGuards && hasGuard() && !getGuard().equals("True"); 
        }
        @Override public boolean editAsMultiline() { return false; }
    }

    public static final Point2D.Double DEFAULT_UNIQUE_NAME_POS = new Point2D.Double(0.0, -1.5);
    public static final Point2D.Double DEFAULT_SUPERPOS_TAGS_POS = new Point2D.Double(0.0, -2.5);

    public Transition() { }
    
    public Transition(String name, Type type, String delayExpr, String priorityExpr, 
                      String weightExpr, String numServersExpr, String guardExpr, 
                      double rotation, Point2D pos) 
    {
        initializeNode(pos, name);
        this.type = type;
        this.delayExpr.setExpr(delayExpr);
        this.priorityExpr.setExpr(priorityExpr);
        this.weightExpr.setExpr(weightExpr);
        this.numServersExpr.setExpr(numServersExpr);
        this.guardExpr.setExpr(guardExpr);
        this.rotation = rotation;
    }

    @Override
    public void checkNodeCorrectness(NetPage page, ParserContext context) {
        super.checkNodeCorrectness(page, context);
        
        colorVarsInUse = new HashSet<>();
        
        if (hasDelay()) {
            delayExpr.checkExprCorrectness(context, page, this);
            if (type == Type.GEN) {
                if (NetObject.isDouble(delayExpr.getExpr()))
                    page.addPageWarning("Transition has a constant probability density function. "
                            + "If you want a deterministic event at time "+delayExpr.getExpr()
                            + ", specify the delay as: I["+delayExpr.getExpr()+"]", this);
            }
        }
        if (hasPriority())
            priorityExpr.checkExprCorrectness(context, page, this);
        if (hasWeight())
            weightExpr.checkExprCorrectness(context, page, this);
        if (hasNumServers())
            numServersExpr.checkExprCorrectness(context, page, this);
        if (hasGuard())
            guardExpr.checkExprCorrectness(context, page, this);
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        if (hasDelay()) 
            delayExpr.rewrite(context, rewriter);
        if (hasPriority())
            priorityExpr.rewrite(context, rewriter);
        if (hasWeight())
            weightExpr.rewrite(context, rewriter);
        if (hasNumServers())
            numServersExpr.rewrite(context, rewriter);
        if (hasGuard())
            guardExpr.rewrite(context, rewriter);
    }
    
    @Override
    public double getWidth() {
        return (type == Type.IMM ? IMMED_TRANSITION_WIDTH : TIMED_TRANSITION_WIDTH);
    }

    @Override
    public double getHeight() { return height; }

    @Override
    public double getRotation() { return rotation; }

    @Override
    public void setRotation(double rotation) {
        this.rotation = rotation;
    }
    
    public Type getType()           { return type; }

    public String getDelay()        { return delayExpr.getExpr(); }
    public boolean hasDelay()       { return isTimed() && getFiringMode() == FiringMode.INSTANTANEOUS; }

    public String getPriority()     { return priorityExpr.getExpr(); }
    public boolean hasPriority()    { return isImmediate(); }

    public String getWeight()       { return weightExpr.getExpr(); }
    public boolean hasWeight()      { return isImmediate(); }

    public String getNumServers()   { return numServersExpr.getExpr(); }
    public boolean hasNumServers()  { return isExponential(); }
    public boolean isInfiniteServers() { 
        assert hasNumServers(); 
        return numServersExpr.getExpr().equals("Infinite");
    }
    
    public String getGuard()        { return guardExpr.getExpr(); }
    public boolean hasGuard()       { return isFiringInstantaneous(); }
    
    public Set<ColorVar> getColorVarsInUse() {
        return colorVarsInUse; // could be null
    }

    public boolean isImmediate()    { return (type == Type.IMM); }
    public boolean isTimed()        { return (type != Type.IMM); }
    public boolean isExponential()  { return (type == Type.EXP); }
    public boolean isGeneral()      { return (type == Type.GEN); }
    public boolean isContinuous()   { return (type == Type.CONT); }
    
    public FiringMode getFiringMode() { 
        return (type == Type.CONT) ? FiringMode.FLOW : FiringMode.INSTANTANEOUS;
    }
    @Override
    public boolean isFiringInstantaneous() { 
        return getFiringMode() == FiringMode.INSTANTANEOUS;
    }
    @Override
    public boolean isFiringFlow() { 
        return getFiringMode() == FiringMode.FLOW;
    }
    
    
    public ListOfBounds evaluateDelayBound(ParserContext context, AbstractMarking marking, ColorVarsBinding binding) {
        assert hasDelay() && isGeneral();
        try {
            context.evaluatePdfBounds = true;
            if (isGeneralFunctionVerbatim())
                return ListOfBounds.makeNew(1); // Unparsed transition functions are considered unbounded
            EvaluatedFormula ef = delayExpr.evaluate(context, new EvaluationArguments(marking, null, binding));
            if (ef.getType() == EvaluatedFormula.Type.LIST_OF_BOUNDS)
                return (ListOfBounds)ef;
            return ListOfBounds.UNBOUNDED;
        }
        finally {
            context.evaluatePdfBounds = false;            
        }
    }
    public EvaluatedFormula evaluateDelay(ParserContext context, AbstractMarking marking, ColorVarsBinding binding) {
        assert hasDelay();
        if (isGeneralFunctionVerbatim())
            return RealScalarValue.ONE;
        return delayExpr.evaluate(context, new EvaluationArguments(marking, null, binding));
    }
    public EvaluatedFormula evaluatePriority(ParserContext context, AbstractMarking marking, ColorVarsBinding binding) {
        assert hasPriority();
        return priorityExpr.evaluate(context, new EvaluationArguments(marking, null, binding));
    }
    public EvaluatedFormula evaluateWeight(ParserContext context, AbstractMarking marking) {
        assert hasWeight();
        return weightExpr.evaluate(context, new EvaluationArguments(marking));
    }
    public EvaluatedFormula evaluateNumServers(ParserContext context, AbstractMarking marking) {
        assert hasNumServers();
        return numServersExpr.evaluate(context, new EvaluationArguments(marking));
    }
    public EvaluatedFormula evaluateGuard(ParserContext context, AbstractMarking marking, ColorVarsBinding binding) {
        assert hasGuard();
        return guardExpr.evaluate(context, new EvaluationArguments(marking, null, binding));
    }
    

    public String reduceDelay(ParserContext context, AbstractMarking marking, ColorVarsBinding binding, ExpressionLanguage lang) {
        assert hasDelay();
        return delayExpr.reduce(context, new EvaluationArguments(marking, null, binding), lang);
    }
    public String reducePriority(ParserContext context, AbstractMarking marking, ColorVarsBinding binding, ExpressionLanguage lang) {
        assert hasPriority();
        return priorityExpr.reduce(context, new EvaluationArguments(marking, null, binding), lang);
    }
    public String reduceWeight(ParserContext context, AbstractMarking marking, ExpressionLanguage lang) {
        assert hasWeight();
        return weightExpr.reduce(context, new EvaluationArguments(marking), lang);
    }
    public String reduceNumServers(ParserContext context, AbstractMarking marking, ExpressionLanguage lang) {
        assert hasNumServers();
        return numServersExpr.reduce(context, new EvaluationArguments(marking), lang);
    }
    public String reduceGuard(ParserContext context, AbstractMarking marking, ColorVarsBinding binding, ExpressionLanguage lang) {
        assert hasGuard();
        return guardExpr.reduce(context, new EvaluationArguments(marking, null, binding), lang);
    }
    
    public String convertDelayLang(ParserContext context, AbstractMarking marking, ExpressionLanguage lang) {
        assert hasDelay();
        return delayExpr.convertLang(context, new EvaluationArguments(marking), lang);
    }
    
    public String convertWeightLang(ParserContext context, AbstractMarking marking, ExpressionLanguage lang) {
        assert hasWeight();
        return weightExpr.convertLang(context, new EvaluationArguments(marking), lang);
    }
    
    public String convertPriorityLang(ParserContext context, AbstractMarking marking, ExpressionLanguage lang) {
        assert hasPriority();
        return priorityExpr.convertLang(context, new EvaluationArguments(marking), lang);
    }
    
    public String convertNumServersLang(ParserContext context, AbstractMarking marking, ExpressionLanguage lang) {
        assert hasNumServers();
        return numServersExpr.convertLang(context, new EvaluationArguments(marking), lang);
    }
    
    public String convertGuardLang(ParserContext context, AbstractMarking marking, ExpressionLanguage lang) {
        assert hasGuard();
        return guardExpr.convertLang(context, new EvaluationArguments(marking), lang);
    }
    
    // returns the deterministic delay expression if this transition is a general transition
    // with firing function: I[ expr ]. Otherwise returns null.
    // Accepted expressions are restricted to:  I[ real-value ]   and:  I[ constant-id ]
    public String getSimpleDeterministicDelay() {
        if (!isGeneral())
            return null;
        String expr = delayExpr.getExpr().trim();
        if (expr.length() >= 4 && expr.charAt(0)=='I' && 
            expr.charAt(1)=='[' && expr.charAt(expr.length()-1)==']') {
            expr = expr.substring(2, expr.length()-1);
            if (NetObject.isAlphanumericIdentifier(expr) ||
                    NetObject.isDouble(expr))
                return expr;
        }
        return null;
    }
    
    // Returns true if the general function is a verbatim function (unparsed and unchecked)
    public boolean isGeneralFunctionVerbatim() {
        return isGeneral() && delayExpr.getExpr().startsWith(VERBATIM_PREFIX);
    }

    
    @Override
    public boolean mayRotate() { return true; }
    
    @Override public double getRoundX() { throw new IllegalStateException("should not be here"); }
    @Override public double getRoundY() { throw new IllegalStateException("should not be here"); }
    
    @Override public double getActivityValue(ActivityState activity) { 
        if (activity != null) {
            if (activity.firing != null) {
                boolean enabledAfterFiring = (activity.state.getEnabledFirableBindingsOf(this) != null);
                double phi;
                if (isFiringFlow()) {
                    boolean hasFlow = activity.firing.getEnabledFlowBindingsOf(this) != null;
                    if (hasFlow && activity.isFlowPhase()) {
                        // phi starts at 0.5, goes to 2 and fades to 0.5
                        phi = 0.5 + 1.5 * Math.sin(Math.PI * activity.getFlowCoeff());
                    }
                    else if (activity.isFiringPhase()) {
                        if (hasFlow && enabledAfterFiring) // remains enabled
                            phi = 0.5;
                        else if (hasFlow) // becomes disabled, phi fades from 0.5 to 0
                            phi = 0.5 * Math.cos(Math.PI * activity.getFiringCoeff()); 
                        else
                            phi = 0.0; // not enabled
                    }
                    else 
                        phi = 0.0;
                }
                else {
                    if (activity.firing.firedNode != this)
                        return 0.0;
                    if (enabledAfterFiring) { // phi starts at 1, goes to 2 and fades to 1.0
                        phi = 1.0 + Math.sin(Math.PI * activity.getFiringCoeff());
                    }
                    else { // phi starts at 1, goes to 2 and fades to zero
                        // remember: sin(PI/6) = 0.5
                        phi = 2.0 * Math.sin(Math.PI * (1.0/6.0 + (5.0/6.0)*activity.getFiringCoeff()));
                    }
                }
                return (phi * DEFAULT_ACTIVITY_AURA_SIZE);
            }
            // Stand animation
            if (activity.state.getEnabledFirableBindingsOf(this) != null) {
                if (isFiringFlow())
                    return DEFAULT_ACTIVITY_AURA_SIZE * 0.5;
                return DEFAULT_ACTIVITY_AURA_SIZE * activity.getPulseCoeff();
            }
        }
        return 0.0;
    }

    @Override public EditableCell getUniqueNameEditable() { return nameLabel; }
    @Override public int getUniqueNameFontStyle() { return STYLE_ITALIC; }

    @Override
    public ShapeType getShapeType() { return ShapeType.RECTANGLE; }

    @Override 
    public LabelDecor getUniqueNameDecor() { return nameLabel; }
    public LabelDecor getDelayDecor() { return delayLabel; }
    public LabelDecor getPriorityDecor() { return priorityLabel; }
    public LabelDecor getWeightDecor() { return weightLabel; }
    public LabelDecor getNumServersDecor() { return numServersLabel; }
    public LabelDecor getGuardDecor() { return guardLabel; }
    
    @Override public boolean mayResizeWidth() { return false; }
    @Override public boolean mayResizeHeight() { return true; }
    @Override public double getDefaultWidth() { throw new UnsupportedOperationException("Not supported."); }
    @Override public double getDefaultHeight() { return DEFAULT_TRANSITION_HEIGHT; }
    @Override public void setWidth(double newWidth) { throw new IllegalStateException("should not be here"); }
    @Override public void setHeight(double newHeight) { 
//        for (int d=0; d<getNumDecors(); d++) {
//            Decor decor = getDecor(d);
//            NodeLabelDecor nld;
//            nld.
//            if (decor.getRelativeY() > )
//        }
        height = newHeight;
    }
    
    @Override public boolean hasShadow() { return true; }


    private static final Color GEN_COLOR = new Color(70, 70, 70);
    private static final Color GEN_COLOR_GRAYED = new Color(220, 220, 220);
    private static final Color GEN_VERBATIM_COLOR = new Color(100, 100, 100);
    private static final Color GEN_VERBATIM_COLOR_GRAYED = new Color(240, 240, 240);

    @Override public Color getFillColor(ActivityState activity) { 
        if (type == Type.EXP || type == Type.CONT)
            return Color.WHITE;
        else if (type == Type.IMM)
            return Color.BLACK;
        else if (isGeneralFunctionVerbatim())
            return GEN_VERBATIM_COLOR;
        return GEN_COLOR;
    }
    @Override public Color getFillColorGrayed() { 
        if (type == Type.EXP || type == Type.CONT)
            return Color.WHITE;
        else if (type == Type.IMM)
            return GRAYED_BLACK;
        else if (isGeneralFunctionVerbatim())
            return GEN_VERBATIM_COLOR_GRAYED;
        return GEN_COLOR_GRAYED;
    }
    @Override public Color getFillColorError() { 
        if (type == Type.EXP)
            return Color.WHITE;
        return Color.RED;
    }
    
    @Override public Color getBorderColor()        { return Color.BLACK; }
    @Override public Color getBorderColorGrayed()  { return GRAYED_BLACK; }
    @Override public Color getBorderColorError()   { return Color.RED; }


    private static final Rectangle2D doubleBorder = new Rectangle2D.Double();
    @Override
    protected void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor,
                                     boolean errorFlag, Shape nodeShape) 
    {
        if (isContinuous()) {
            // Paint the double border
            Stroke sk = g.getStroke();
            g.setStroke(getBorderWidth().logicStroke);
            g.setColor(borderColor);
            Rectangle2D rect = nodeShape.getBounds2D();
            doubleBorder.setFrame(rect.getX() + Place.DOUBLE_BORDER_WIDTH,
                                   rect.getY() + Place.DOUBLE_BORDER_WIDTH,
                                   rect.getWidth() - 2 * Place.DOUBLE_BORDER_WIDTH,
                                   rect.getHeight() - 2 * Place.DOUBLE_BORDER_WIDTH);
            g.draw(doubleBorder);
            g.setStroke(sk);
        }
        // Write the function sugnal in the transition box.
        if (isGeneralFunctionVerbatim()) {
            // Prepare a new LaTeX text
            LatexFormula fnText = new LatexFormula("\\textrm{FN}", 
                                                   0.6f  * (float)getUnitToPixels());
            double scaleFact = 1.0 / (double)getUnitToPixels();
            double textX = getCenterX() - (fnText.getWidth() * scaleFact) / 2.0;
            double textY = getCenterY() - (fnText.getHeight() * scaleFact * 0.9) / 2.0;
            
            AffineTransform trn = g.getTransform();
            g.rotate(-getRotation(), getCenterX(), getCenterY());
            Color oldColor = g.getColor();
            g.setColor(isGrayed() ? GRAYED_BLACK : Color.WHITE);
            fnText.draw(g, textX, textY, scaleFact, false);
            g.setTransform(trn);
            g.setColor(oldColor);
        }
    }
    
    @Override public EditableValue getSuperPosTagsLabelEditable() { return superPosTagLabel; }
    @Override public GroupClass getGroupClass() { return GroupClass.TRANSITION; }
    @Override public String getSuperPosTags() { return superPosTags; }
    @Override public void setSuperPosTags(String superPosTags) { 
        assert isValidTagList(superPosTags);
        this.superPosTags = superPosTags;
        invalidateTagList();
    }
    @Override public LabelDecor getSuperPosTagsDecor() { return superPosTagLabel; }
    
    @Override
    public int getNumDecors() {
        return 7; // name + delay + priority + weight + numServers + superPosTag + guard
    }

    @Override
    public Decor getDecor(int i) {
        switch (i) {
            case 0:   return nameLabel;
            case 1:   return priorityLabel;
            case 2:   return delayLabel;
            case 3:   return weightLabel;
            case 4:   return numServersLabel;
            case 5:   return superPosTagLabel;
            case 6:   return guardLabel;
            default:
                throw new UnsupportedOperationException("Wrong decor index."); 
        }
    }
    
    // Transitions are not editable with a double click.
    @Override public EditableCell getCentralEditable(double px, double py) { return null; }
    
    @Override
    public boolean isIdentifiedAs(String name) {
        return getUniqueName().equals(name);
    }
    
    //------------- editable properties ---------------
    public EditableValue getTransitionTypeEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return type; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                assert page instanceof NetPage;
                NetPage npage = (NetPage)page;
                double oldW = getWidth();
                Type oldType = type, newType = (Type)value;
                // Special case: if we are passing from an EXP to a GEN, convert the delay
                // to the deterministic delay function. Vice versa, if we have a simple deterministic
                // we may convert it back to the transition rate
                if (newType == Type.GEN && oldType == Type.EXP) {
                    if (NetObject.isAlphanumericIdentifier(delayExpr.getExpr()) ||
                        NetObject.isDouble(delayExpr.getExpr()))
                        delayExpr.setExpr("I[" + delayExpr.getExpr() +"]");
                }
                else if (newType == Type.EXP && oldType == Type.GEN) {
                    String expr = getSimpleDeterministicDelay();
                    if (expr != null)
                        delayExpr.setExpr(expr);
                }
                // Change the type
                type = newType;
                double dx = oldW - getWidth();
                setNodePosition(getX() + dx/2, getY());
                // Update connected edges
                for (Edge e : npage.edges)
                    e.invalidateIfConnectedTo(Transition.this);
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }
    public EditableValue getDelayEditable()    { return delayLabel; }
    public EditableValue getPriorityEditable() { return priorityLabel; }
    public EditableValue getWeightEditable()   { return weightLabel; }
    public EditableValue getNumberOfServersEditable() { return numServersLabel; }
    public EditableValue getGuardEditable()    { return guardLabel; }

    private static final Point2D.Double DEFAULT_DELAY_LABEL_POS = new Point2D.Double(0.5, 1.0);
    private static final Point2D.Double DEFAULT_PRIO_LABEL_POS = new Point2D.Double(0.5, 1.0);
    private static final Point2D.Double DEFAULT_WEIGHT_LABEL_POS = new Point2D.Double(-0.5, 1.0);
    private static final Point2D.Double DEFAULT_NSERVERS_LABEL_POS = new Point2D.Double(-0.5, 2.0);
    private static final Point2D.Double DEFAULT_GUARD_LABEL_POS = new Point2D.Double(0.0, 1.5);
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        bindXMLAttrib(this, el, exDir, "type", "type", null, Type.class);
        bindXMLAttrib(this, el, exDir, "label-x", "@UniqueNameDecor.@InternalPosX", DEFAULT_UNIQUE_NAME_POS.x);
        bindXMLAttrib(this, el, exDir, "label-y", "@UniqueNameDecor.@InternalPosY", DEFAULT_UNIQUE_NAME_POS.y);
        if (hasDelay()) {
            bindXMLAttrib(this, el, exDir, "delay", "delayExpr.@Expr", "1.0");
            bindXMLAttrib(this, el, exDir, "delay-x", "delayLabel.@InternalPosX", DEFAULT_DELAY_LABEL_POS.x);
            bindXMLAttrib(this, el, exDir, "delay-y", "delayLabel.@InternalPosY", DEFAULT_DELAY_LABEL_POS.y);
        }
        if (hasPriority()) {
            bindXMLAttrib(this, el, exDir, "priority", "priorityExpr.@Expr", "1");
            bindXMLAttrib(this, el, exDir, "priority-x", "priorityLabel.@InternalPosX", DEFAULT_PRIO_LABEL_POS.x);
            bindXMLAttrib(this, el, exDir, "priority-y", "priorityLabel.@InternalPosY", DEFAULT_PRIO_LABEL_POS.y);
        }
        if (hasWeight()) {
            bindXMLAttrib(this, el, exDir, "weight", "weightExpr.@Expr", "1.0");
            bindXMLAttrib(this, el, exDir, "weight-x", "weightLabel.@InternalPosX", DEFAULT_WEIGHT_LABEL_POS.x);
            bindXMLAttrib(this, el, exDir, "weight-y", "weightLabel.@InternalPosY", DEFAULT_WEIGHT_LABEL_POS.y);
        }
        if (hasNumServers()) {
            bindXMLAttrib(this, el, exDir, "nservers", "numServersExpr.@Expr", "Infinite");
            bindXMLAttrib(this, el, exDir, "nservers-x", "numServersLabel.@InternalPosX", DEFAULT_NSERVERS_LABEL_POS.x);
            bindXMLAttrib(this, el, exDir, "nservers-y", "numServersLabel.@InternalPosY", DEFAULT_NSERVERS_LABEL_POS.y);
        }
        if (hasGuard()) {
            bindXMLAttrib(this, el, exDir, "guard", "guardExpr.@Expr", "True");
            bindXMLAttrib(this, el, exDir, "guard-x", "guardLabel.@InternalPosX", DEFAULT_GUARD_LABEL_POS.x);
            bindXMLAttrib(this, el, exDir, "guard-y", "guardLabel.@InternalPosY", DEFAULT_GUARD_LABEL_POS.y);
        }
    }

    @Override public Point2D getSuperPosTagsDefaultPos() { return DEFAULT_SUPERPOS_TAGS_POS; }
}
