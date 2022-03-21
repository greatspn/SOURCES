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
import editor.domain.MovementHandle;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import static editor.domain.NetObject.DEFAULT_ACTIVITY_AURA_SIZE;
import editor.domain.grammar.ClockGuardEvaluationArguments;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.play.AbstractMarking;
import editor.domain.play.ActivityState;
import editor.domain.play.Firable;
import java.awt.Graphics2D;
import java.awt.geom.Point2D;
import java.io.Serializable;
import java.util.ArrayList;
import javax.swing.SwingConstants;
import org.w3c.dom.Element;

/** An edge of the DTA between two locations.
 *
 * @author elvio
 */
public class DtaEdge extends Edge implements Serializable, Firable {
    
    public enum Kind {
        INNER, BOUNDARY
    }
    
    public class ActSetExpr extends Expr {
        public ActSetExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Action set of DTA edge.";
        }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.ACTION_SET; 
        }
    }
    public class ClockGuardExpr extends Expr {
        public ClockGuardExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Clock guard of DTA edge.";
        }
        @Override protected int getParseFlags() { return ParserContext.PF_CLOCK_VARS_IN_EXPR; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.CLOCK_CONSTRAINT; 
        }
    }
    
    
    private Kind edgeKind;
    private final ActSetExpr innerActSet = new ActSetExpr("Act"); // Used only for Inner edges
    private final ClockGuardExpr clockGuard = new ClockGuardExpr("True");
    private boolean reset;
    
    private final ActSetLabel actSetLabel = new ActSetLabel();
    private final ClockGuardLabel clockGuardLabel = new ClockGuardLabel();
    
    public static final Point2D.Double DEFAULT_ACTSET_POS = new Point2D.Double(0, -0.5);
    public static final Point2D.Double DEFAULT_CLOCKGUARD_POS = new Point2D.Double(0, 0.5);
    
    class ActSetLabel extends Edge.EdgeLabelDecor {

        public ActSetLabel() {
            super(DEFAULT_TEXT_SIZE, DEFAULT_ACTSET_POS, 
                  SwingConstants.CENTER, SwingConstants.CENTER);
            setEdgeK(0.5);
        }

        @Override public boolean isCurrentValueValid() { 
            return edgeKind == DtaEdge.Kind.BOUNDARY || innerActSet.isFormattedAndCorrect(); 
        }
        @Override public Object getValue() { return innerActSet.getExpr(); }

        @Override
        public String getVisualizedValue() {
            return isInner() ? innerActSet.getVisualizedExpr() : "\\sharp";
        }

        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return innerActSet.isValidExpr(context, (String)value);
        }

        @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
            innerActSet.setExpr((String)value);
            if (innerActSet.getExpr().equals("#"))
                edgeKind = DtaEdge.Kind.BOUNDARY;
            else
                edgeKind = DtaEdge.Kind.INNER;
        }
        @Override public boolean drawLineBetweenDecorAndEdge() { return false; }
        @Override public boolean isEditable() { return isInner(); }
        @Override public boolean editAsMultiline() { return false; }
        @Override public Size getTextSize() { return getGfxTextSize(); }
    }
    
    class ClockGuardLabel extends Edge.EdgeLabelDecor {

        public ClockGuardLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, DEFAULT_CLOCKGUARD_POS, 
                  SwingConstants.CENTER, SwingConstants.CENTER);
            //setEdgeK(0.5);
        }

        // Synchronized with the ActSet label
        @Override public double getEdgeK() { return actSetLabel.getEdgeK(); }
        @Override public void setEdgeK(double k) { actSetLabel.setEdgeK(k); }
        @Override public MovementHandle getEdgeAttachmentHandle(Edge edge) { return null; }
        
        
        @Override public String getVisualizedValue() { 
            return clockGuard.getVisualizedExpr() + (reset ? "; \\{ x \\}" : ""); 
        }
        @Override public Object getValue() { return clockGuard.getExpr(); }
        @Override public boolean isCurrentValueValid() { return clockGuard.isFormattedAndCorrect(); }

        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return clockGuard.isValidExpr(context, (String)value);
        }

        @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
            clockGuard.setExpr((String)value);
        }
        @Override public boolean drawLineBetweenDecorAndEdge() { return false; }
        @Override public boolean isEditable() { return true; }
        @Override public boolean editAsMultiline() { return false; }
        @Override public Size getTextSize() { return getGfxTextSize(); }
    }

    public DtaEdge(Node tail, int tailMagnet, Node head, int headMagnet, ArrayList<Point2D> points, 
                   Kind edgeKind, String innerActSet, String clockGuard, boolean reset) 
    {
        super(tail, tailMagnet, head, headMagnet, points, false);
        this.edgeKind = edgeKind;
        this.innerActSet.setExpr(innerActSet);
        this.clockGuard.setExpr(clockGuard);
        this.reset = reset;
    }
    
    public DtaEdge() { }

    @Override
    public void checkEdgeCorrectness(NetPage page, ParserContext context) {
        super.checkEdgeCorrectness(page, context);
        
        if (isInner())
            innerActSet.checkExprCorrectness(context, page, this);
        clockGuard.checkExprCorrectness(context, page, this);
        
        if (getTailNode() != null) {
            DtaLocation startLoc = (DtaLocation)getTailNode();
            if (!startLoc.isNotFinal())
                page.addPageError("The edge departs from a final location.", startLoc);
        }
        if (getTailNode() != null && getHeadNode() != null && clockGuard.isFormatted()) {
            // Separate clock guard from variable update ( ';' is the separator)
            String constr = clockGuard.getExpr(), upd;
            int semicolon = constr.indexOf(';');
            if (semicolon != -1) {
                upd = constr.substring(semicolon+1);
                constr = constr.substring(0, semicolon);
            }
            boolean isEqualComp = constr.contains("=");
            if (isEqualComp && isInner())
                page.addPageError("Inner edge has a boundary clock guard.", this);
            if (!isEqualComp && isBoundary())
                page.addPageError("Boundary edge should have a boundary clock guard.", this);
        }
    }

    @Override
    public void rewriteEdge(ParserContext context, ExprRewriter rewriter) {
        if (isInner())
            innerActSet.rewrite(context, rewriter);
        clockGuard.rewrite(context, rewriter);
    }
    
    public EvaluatedFormula evaluateClockGuardExpr(ParserContext context, AbstractMarking state) {
        return clockGuard.evaluate(context, new EvaluationArguments(state));
    }
    
    // Get the next time bound of this edge, computed from the current
    // valuations of the clock variables in @state.
    public double getBoundOf(ParserContext context, AbstractMarking state) {
        ClockGuardEvaluationArguments args = new ClockGuardEvaluationArguments(state);
        clockGuard.evaluate(context, args);
        System.out.println("clock guard bound is "+args.timeBound);
        return args.timeBound;
    }

    public EvaluatedFormula evaluateActionSetExpr(ParserContext context, AbstractMarking state, 
                                                  Firable firedAction, ColorVarsBinding binding) 
    {
        return innerActSet.evaluate(context, new EvaluationArguments(state, firedAction, binding));
    }
    
    public boolean isBoundary() { return edgeKind == Kind.BOUNDARY; }
    public boolean isInner()    { return edgeKind == Kind.INNER; }
    public Kind getEdgeKind()   { return edgeKind; }
    

    @Override
    public ArrowType getArrowType() { return ArrowType.TRIANGLE; }
    
    @Override 
    public LineType getLineType() { return isInner() ? LineType.SOLID : LineType.DASHED; }

    @Override
    public boolean canConnectTo(Node node, EndPoint endPt) {
        return (node instanceof DtaLocation);
    }
    
    @Override public boolean canBeBroken() { return false; }

    @Override
    public double getActivityValue(ActivityState activity) {
        if (activity != null) {
            if (activity.firing != null) {
                if (activity.firing.firedNode != this)
                    return 0.0;
                boolean enabledAfterFiring = activity.state.enabledDtaEdges.contains(this);
                double phi;
                if (enabledAfterFiring) { // phi starts at 1, goes to 2 and fades to 1.0
                    phi = 1.0 + Math.sin(Math.PI * activity.getFiringCoeff());
                }
                else { // phi starts at 1, goes to 2 and fades to zero
                    // remember: sin(PI/6) = 0.5
                    phi = 2.0 * Math.sin(Math.PI * (1.0/6.0 + (5.0/6.0)*activity.getFiringCoeff()));
                }
                return (phi * DEFAULT_ACTIVITY_AURA_SIZE);
            }
            // Stand animation
            if (activity.state.enabledDtaEdges.contains(this))
                return DEFAULT_ACTIVITY_AURA_SIZE * activity.getPulseCoeff();
        }
        return 0.0;
    }

    @Override
    public int getNumDecors() {
        return 2;
    }

    @Override
    public Decor getDecor(int i) {
        if (i==0)
            return actSetLabel;
        else if (i==1)
            return clockGuardLabel;
        throw new IllegalArgumentException("wrong decor index");
    }
    
    @Override public void paintEdgeDrawings(Graphics2D g, DrawHelper dh) { }

    @Override
    public EditableCell getCentralEditable(double px, double py) { return null; }
    
    
    
    public EditableValue getActSetEditable() { return actSetLabel; }
    public EditableValue getClockGuardEditable() { return clockGuardLabel; }

    public EditableValue getBoundaryEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return isBoundary() ? Boolean.TRUE : Boolean.FALSE; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                edgeKind = (Boolean)value ? Kind.BOUNDARY : Kind.INNER;
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return true; }
        };
    }
    public EditableValue getResetClockEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return reset ? Boolean.TRUE : Boolean.FALSE; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) 
            {   reset = (Boolean)value;   }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return true; }
        };
    }
    public boolean isReset() { return reset; }
    
    public EdgeLabelDecor getActSetDecor() { return actSetLabel; }
    public EdgeLabelDecor getClockGuardDecor() { return clockGuardLabel; }
    
    @Override
    public boolean isIdentifiedAs(String name) {
        // There is no specific identifier for a DTA edge
        return false;
    }
    
    @Override
    public boolean isFiringInstantaneous() {
        return true;
    }

    @Override
    public boolean isFiringFlow() {
        return false;
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir);
        
        bindXMLAttrib(this, el, exDir, exDir.arcKindTag, "edgeKind", null, Kind.class);
        bindXMLAttrib(this, el, exDir, "reset", "reset", false);
        bindXMLAttrib(this, el, exDir, "actset", "innerActSet.@Expr", "");
        bindXMLAttrib(this, el, exDir, "actset-x", "actSetLabel.@InternalPosX", DEFAULT_ACTSET_POS.x);
        bindXMLAttrib(this, el, exDir, "actset-y", "actSetLabel.@InternalPosY", DEFAULT_ACTSET_POS.y);
        bindXMLAttrib(this, el, exDir, "actset-clock-k", "actSetLabel.@EdgeK", 0.0);
        bindXMLAttrib(this, el, exDir, "clock-guard", "clockGuard.@Expr", "");
        bindXMLAttrib(this, el, exDir, "clock-guard-x", "clockGuardLabel.@InternalPosX", DEFAULT_CLOCKGUARD_POS.x);
        bindXMLAttrib(this, el, exDir, "clock-guard-y", "clockGuardLabel.@InternalPosY", DEFAULT_CLOCKGUARD_POS.y);
    }
}
