/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.Decor;
import editor.domain.DrawHelper;
import editor.domain.DrawHelper.StrokeWidth;
import editor.domain.Edge;
import editor.domain.EditableCell;
import editor.domain.Expr;
import static editor.domain.NetObject.ACTIVE_COLOR;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.domain.ViewProfile;
import editor.domain.grammar.ColorVarsBinding;
import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.play.AbstractMarking;
import editor.domain.play.ActivityState;
import editor.domain.values.MultiSet;
import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.Point2D;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;
import javax.swing.SwingConstants;
import org.w3c.dom.Element;

/** An arc in a petri net
 *
 * @author elvio
 */
public class GspnEdge extends Edge implements Serializable {
    
    public enum Kind {
        INPUT_OR_OUTPUT, // Not yet determined, for incomplete edges.
        INPUT, 
        OUTPUT,
        INHIBITOR
    }
    
    class ArcMultiplicityExpr extends Expr {
        // Color variables that appear in the edge multiplicity
        Set<ColorVar> colorVarsInUse = new HashSet<>();
        
        public ArcMultiplicityExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { return "Arc cardinality expression."; }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            ColorClass colorDomain = getColorDomainOfConnectedPlace();
            TokenType placeType = getTypeOfConnectedPlace();
            if (placeType == null || colorDomain == null)
                return ParserContext.ParserEntryPoint.SKIP_PARSE;
            if (colorDomain.isNeutralDomain()) {
                // Uncolored (neutral) edge
                switch (getTypeOfConnectedPlace()) {
                    case CONTINUOUS:    return ParserContext.ParserEntryPoint.REAL_EXPR;
                    case DISCRETE:      return ParserContext.ParserEntryPoint.INT_EXPR;
                }
            }
            else {
                // Colored edge
                switch (getTypeOfConnectedPlace()) {
                    case CONTINUOUS:    return ParserContext.ParserEntryPoint.REAL_MULTISET_EXPR;
                    case DISCRETE:      return ParserContext.ParserEntryPoint.INT_MULTISET_EXPR;
                }
            }
            throw new IllegalStateException();
        }

        @Override
        public boolean prepareContext(ParserContext context) {
            ColorClass colorDomain = getColorDomainOfConnectedPlace();
//            colorVarsInUse.clear();
            if (colorDomain == null) 
                return false;
            if (!colorDomain.isNeutralDomain()) {
//                context.colorVarsInUse = colorVarsInUse;
                context.colorDomainOfExpr = colorDomain;
            }
            return true;
        }

        @Override
        public void checkExprCorrectness(ParserContext context, ProjectPage page, Selectable selObj) {
            colorVarsInUse.clear();
            context.colorVarsInUse = colorVarsInUse;
            super.checkExprCorrectness(context, page, selObj); 
        }
    }
    
    private Kind edgeKind;
    private final ArcMultiplicityExpr multiplicity = new ArcMultiplicityExpr("1");
    private final MultiplicityLabel multiplicityLabel = new MultiplicityLabel();
    
    private static final String DEFAULT_MULTIPLICITY_EXPR = "1";
    private static final Point2D.Double DEFAULT_MULTIPLICITY_OFFSET = new Point2D.Double(1, 1);
    private static final double DEFAULT_MULTIPLICITY_EDGEK = 0.5;
    
    
    class MultiplicityLabel extends EdgeLabelDecor {

        public MultiplicityLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, DEFAULT_MULTIPLICITY_OFFSET, 
                  SwingConstants.CENTER, SwingConstants.CENTER);
            setEdgeK(DEFAULT_MULTIPLICITY_EDGEK);
        }

        @Override public Object getValue() { return multiplicity.getExpr(); }
        @Override public String getVisualizedValue() { 
            return (multiplicity.isFormatted() ? multiplicity.getFormatted() : multiplicity.getExpr());
        }

        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return multiplicity.isValidExpr(context, (String)value);
        }
        
        @Override public boolean isCurrentValueValid() { return multiplicity.isFormattedAndCorrect(); }

        @Override
        public void setValue(ProjectData project, ProjectPage page, Object value) {
            multiplicity.setExpr((String)value);
        }
        @Override public boolean drawLineBetweenDecorAndEdge() { return true; }
        @Override public boolean isEditable() { return true; }
        @Override public boolean isVisible(ViewProfile vp) { 
            ColorClass colorDomain = getColorDomainOfConnectedPlace();
            boolean isColored = (colorDomain!=null && !colorDomain.isNeutralDomain());
            if (isColored)
                return true; // Always visible;
            boolean isSingleSel = (vp.singleSelObject == GspnEdge.this);
            return isSingleSel || !multiplicity.getExpr().equals("1");
//            return !multiplicity.getExpr().equals("1") || isColored;
        }
        @Override public Color getTextColorNormal(DrawHelper dh) {
            ColorClass colorDomain = getColorDomainOfConnectedPlace();
            boolean isColored = (colorDomain!=null && !colorDomain.isNeutralDomain());
            if (!isColored) {
                boolean isSingleSel = (dh.viewProfile.singleSelObject == GspnEdge.this);
                if (isSingleSel && multiplicity.getExpr().equals("1"))
                    return Color.DARK_GRAY;
            }
            return super.getTextColorNormal(dh); 
        }
        @Override public boolean editAsMultiline() { return false; }
    }
    
    public EditableCell getMultiplicityEditable() {
        return multiplicityLabel;
    }
    
    public EdgeLabelDecor getMultiplicityDecor() { return multiplicityLabel; }
    
    public String getMultiplicity() {
        // Should evaluate the expression and give the integer result
        return multiplicity.getExpr();
    }
    public EvaluatedFormula evaluateMultiplicity(ParserContext context, AbstractMarking marking, 
                                                 ColorVarsBinding binding) 
    {
        return multiplicity.evaluate(context, new EvaluationArguments(marking, null, binding));
    }
    public String reduceMultiplicity(ParserContext context, AbstractMarking marking, 
                                     ColorVarsBinding binding, ExpressionLanguage lang) 
    {
        return multiplicity.reduce(context, new EvaluationArguments(marking, null, binding), lang);
    }

    public String convertMultiplicityLang(ParserContext context, ExpressionLanguage lang)
    {
        return multiplicity.convertLang(context, EvaluationArguments.NO_ARGS, lang);
    }
    
    public boolean isDiscrete() { return getTypeOfConnectedPlace() == TokenType.DISCRETE; }
    public boolean isContinuous() { return getTypeOfConnectedPlace() == TokenType.CONTINUOUS; }
    public boolean isFiringInstantaneous() { 
        return getFiringTypeOfConnectedTransition() == FiringMode.INSTANTANEOUS;
    }
    public boolean isFiringFlow() { 
        return getFiringTypeOfConnectedTransition() == FiringMode.FLOW;
    }
    
    public Set<ColorVar> getColorVarsInUse() {
        if (multiplicity.isFormattedAndCorrect()) {
            return multiplicity.colorVarsInUse;
        }
        return null;
    }
    
    @Override
    public DrawHelper.StrokeWidth getEdgeWidth() { 
        if (isContinuous() && isFiringFlow())
            return DrawHelper.StrokeWidth.VERY_THICK;
        return DrawHelper.StrokeWidth.BASIC;
    }
    
    @Override
    public DrawHelper.StrokeWidth getPipeWidth() { 
        if (isContinuous() && isFiringFlow())
            return DrawHelper.StrokeWidth.THICK;
        return null;
    }
    
    // Could return null!
    public Place getConnectedPlace() {
        if (getTailNode() != null && getTailNode() instanceof Place)
            return (Place)getTailNode();
        if (getHeadNode() != null && getHeadNode() instanceof Place)
            return (Place)getHeadNode();
        return null;
//        switch (getEdgeKind()) {
//            case INPUT:
//            case INHIBITOR:
//                return (Place)getTailNode();
//            case OUTPUT:
//                return (Place)getHeadNode();
//            case INPUT_OR_OUTPUT:
//                if (getTailNode() != null && getTailNode() instanceof Place)
//                    return (Place)getTailNode();
//        }
//        throw new IllegalStateException();
    }
    
    // Could return null!
    public Transition getConnectedTransition() {
        if (getTailNode() != null && getTailNode() instanceof Transition)
            return (Transition)getTailNode();
        if (getHeadNode() != null && getHeadNode() instanceof Transition)
            return (Transition)getHeadNode();
        return null;
//        switch (getEdgeKind()) {
//            case INPUT:
//            case INHIBITOR:
//                return (Transition)getHeadNode();
//            case OUTPUT:
//                return (Transition)getTailNode();
//        }
//        throw new IllegalStateException();
    }
    
    // The connected place is continouos or discrete?
    // This method returns null if the edge is not completely connected.
    public TokenType getTypeOfConnectedPlace() {
        Place connPlace = getConnectedPlace();
        return connPlace == null ? null : connPlace.getType();
    }
    
    // The connected transition fires immediately, or continuously?
    // This method returns null if the edge is not completely connected.
    public FiringMode getFiringTypeOfConnectedTransition() {
        Transition connTrn = getConnectedTransition();
        return connTrn == null ? null : connTrn.getFiringMode();
    }
    
    // Get the color domain of the connected place, or null if the edge is not 
    // completely connected or there is some problem in color/place definitions
    public ColorClass getColorDomainOfConnectedPlace() {
        Place connPlace = getConnectedPlace();
        return connPlace == null ? null : connPlace.getColorDomain();
    }
    
    public GspnEdge(Node tail, int tailMagnet, Node head, int headMagnet, 
                    ArrayList<Point2D> points, boolean isBroken, Kind edgeKind, 
                    String mult) 
    {
        super(tail, tailMagnet, head, headMagnet, points, isBroken);
        this.edgeKind = edgeKind;
        multiplicity.setExpr(mult);
    }
    
    public GspnEdge() { }

    @Override
    public ArrowType getArrowType() {
        if (edgeKind == Kind.INHIBITOR)
            return ArrowType.EMPTY_CIRCLE;
        return ArrowType.SIMPLE;
    }

    @Override public LineType getLineType() { return LineType.SOLID; }
    
    @Override public boolean canBeBroken() { return true; }
    
    @Override public double getActivityValue(ActivityState activity) { return -1; }
    
    public Kind getEdgeKind() {
        if (getTailNode() != null && getHeadNode() != null && edgeKind == Kind.INPUT_OR_OUTPUT) {
            if (getTailNode() instanceof Place && getHeadNode() instanceof Transition)
                return Kind.INPUT;
            if (getTailNode() instanceof Transition && getHeadNode() instanceof Place)
                return Kind.OUTPUT;
        }
        return edgeKind;
    }
    public void setEdgeKind(Kind edgeKind) {
        this.edgeKind = edgeKind;
    }
    
    
    public void sanitizeTypeAfterLoad() {
        if (edgeKind == Kind.INPUT || edgeKind == Kind.OUTPUT)
            edgeKind = Kind.INPUT_OR_OUTPUT;
    }

    @Override
    public void checkEdgeCorrectness(NetPage page, ParserContext context) {
        super.checkEdgeCorrectness(page, context);
        
//        Place.PlaceType currExprType = exprType;
//        
//        Transition trn = null;
//        if (getTailNode() instanceof Place && getHeadNode() instanceof Transition) {
//            exprType = ((Place)getTailNode()).getType();
//            trn = (Transition)getHeadNode();
//        }
//        else if (getTailNode() instanceof Transition && getHeadNode() instanceof Place) {
//            exprType = ((Place)getHeadNode()).getType();
//            trn = (Transition)getTailNode();
//        }
//        else
//            exprType = null;
//        
//        if (exprType != currExprType)
//            invalidateEffectiveEdgePath();
//        
//        if (trn != null && exprType == Place.PlaceType.CONTINUOUS) {
//            // Continuous places cannot be connected with immediate transitions.
//            if (trn.isImmediate())
//                page.addPageError("You cannot connect an immediate transition "
//                        + "with a continuous place.", this);
//        }
        
        if (getTypeOfConnectedPlace() != null) {
            multiplicity.checkExprCorrectness(context, page, this);
        }
        
//        SemanticParser.ColorVarsInUse colorVar = getColorVarInUse();
//        if (colorVar != null) {
//            System.out.print(getTailNode().getUniqueName()+" --> "+getHeadNode().getUniqueName()+":  <");
//            int count = 0;
//            for (ColorVar cv : colorVar.vars) {
//                System.out.print((count++==0 ? "" : ",") + cv.getUniqueName());
//            }
//            System.out.println(">");
//        }

    }

    @Override
    public void rewriteEdge(ParserContext context, ExprRewriter rewriter) {
        if (getTypeOfConnectedPlace() != null) {
            multiplicity.rewrite(context, rewriter);
        }
    }
    
    @Override
    public boolean canConnectTo(Node node, EndPoint endPt) {
        if (endPt == EndPoint.HEAD) {
            // Testing for a compatible end node
            if (getEdgeKind() == Kind.INHIBITOR)
                return node instanceof Transition;
            if (getTailNode() instanceof Place)
                return (node instanceof Transition);
            else if (getTailNode() instanceof Transition)
                return (node instanceof Place);
            return (node instanceof Place) || (node instanceof Transition);
        }
        
        // testing for a compatible start node, given the end node
        if (getEdgeKind() == Kind.INHIBITOR)
            return node instanceof Place;
        if (getHeadNode() instanceof Place)
            return (node instanceof Transition);
        else if (getHeadNode() instanceof Transition)
            return (node instanceof Place);
        return (node instanceof Place) || (node instanceof Transition);
    }
    
    private static final Point2D pedTokenCenter = new Point2D.Double();
    private static final Point2D pedPt1 = new Point2D.Double();
    private static final Point2D pedPt2 = new Point2D.Double();
    @Override 
    public void paintEdgeDrawings(Graphics2D g, DrawHelper dh) { 
        if (dh.activity == null || dh.activity.firing == null)
            return;
        
        if (isContinuous() && dh.activity.isFlowPhase()) {
            if (!dh.activity.firing.flowPassed.containsKey(this))
                return; // No flow passing
            
            StrokeWidth pipe = getPipeWidth();
            Stroke sk = g.getStroke();
            Color clr = g.getColor();
            Composite comp = g.getComposite();
            float phi = (float) Math.cos(dh.activity.getFlowCoeff() * Math.PI); // Goes from 1 to 0 and back to 1
            g.setColor(ACTIVE_COLOR);
            g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f - phi*phi));
            float dashPhase = 2.0f - (float)dh.activity.getFlowCoeff() * 1.5f;
            Stroke fsk = new BasicStroke(pipe.getWidth(), BasicStroke.CAP_ROUND,
                                         BasicStroke.JOIN_ROUND, 0.0f, FLOW_DASHES, dashPhase);
            g.setStroke(fsk);            
            paintEdgePipe(g, dh);
            g.setStroke(sk); 
            g.setColor(clr);
            g.setComposite(comp);
            return; // continuous case
        }
        
        // Discrete edge
        if (!dh.activity.firing.tokensPassed.containsKey(this))
            return;
        if (!dh.activity.isFiringPhase())
            return;
        
        // Paint tokens that passes on the edge during a transition firing
        double phase;
        if (getEdgeKind() == Kind.INPUT && dh.activity.getFiringCoeff() < 0.5)
            phase = dh.activity.getFiringCoeff() * 2.0;
        else if (getEdgeKind() == Kind.OUTPUT && dh.activity.getFiringCoeff() > 0.5)
            phase = (dh.activity.getFiringCoeff() - 0.5) * 2.0;
        else 
            return;
        
        double K;
        if (isBroken) {
            if (phase < 0.5) {
                K = linearInterp(tailK, tailBrokenK, phase * 2);
            } else {
                K = linearInterp(headBrokenK, headK, 2 * (phase - 0.5));
            }
        } 
        else {
            //K = tailK + phase * (headK - tailK);
            K = tailK;

            // Enumerate the segments in the K-space in the interval (tailK - headK)
            int startSgm = (int) Math.floor(tailK);
            int numSgm = (int) Math.ceil(headK) - startSgm;
            double[] lenSgm = new double[numSgm];
            double totDist = 0;
            for (int i = 0; i < numSgm; i++) {
                double k1 = Math.max(tailK, startSgm + i);
                double k2 = Math.min(headK, startSgm + i + 1);
                getPointAlongTheLine(pedPt1, k1);
                getPointAlongTheLine(pedPt2, k2);
                lenSgm[i] = pedPt1.distance(pedPt2);
                totDist += lenSgm[i];
            }
            // Get the K position proportional to each segment length.
            double actDist = totDist * phase, dist = 0;
            for (int i = 0; i < numSgm; i++) {
                if (dist + lenSgm[i] > actDist || i == numSgm - 1) {
                    double k1 = Math.max(tailK, startSgm + i);
                    double k2 = Math.min(headK, startSgm + i + 1);
                    K = linearInterp(k1, k2, (actDist - dist) / lenSgm[i]);
                    break;
                }
                dist += lenSgm[i];
            }
        }
        EvaluatedFormula numTokens = dh.activity.firing.tokensPassed.get(this);
        getPointAlongTheLine(pedTokenCenter, K);
        if (numTokens.isScalarInt())
            Place.paintIntegerTokenCountAt(g, dh, numTokens.getScalarInt(), 
                                           pedTokenCenter.getX(), pedTokenCenter.getY());
        else if (numTokens.isScalarReal())
            Place.paintRealTokenCountAt(g, dh, numTokens.getScalarReal(),
                                        pedTokenCenter.getX(), pedTokenCenter.getY());
        else { // multiset of tokens
            assert numTokens.isMultiSet();
            String repr = ((MultiSet)numTokens).toStringFormat(ExpressionLanguage.LATEX, "%.2f");
            Place.paintLatexTokenCountAt(g, dh, null, repr,
                                         pedTokenCenter.getX(), pedTokenCenter.getY());
        }
    }
    
    @Override
    public int getNumDecors() {
        return 1;
    }

    @Override
    public Decor getDecor(int i) {
        if (i==0)
            return multiplicityLabel;
        throw new IllegalArgumentException("Out of bound.");
    }

    @Override
    public EditableCell getCentralEditable(double px, double py) { return null; }

    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        final String mult = exDir.afterXmlCleanup ? "mult" : "multiplicity";
        final String mult_x = exDir.afterXmlCleanup ? "mult-x" : "multiplicity-x";
        final String mult_y = exDir.afterXmlCleanup ? "mult-y" : "multiplicity-y";
        final String mult_k = exDir.afterXmlCleanup ? "mult-k" : "multiplicity-k";

        bindXMLAttrib(this, el, exDir, exDir.arcKindTag, "@EdgeKind", null, Kind.class);
        bindXMLAttrib(this, el, exDir, mult, "multiplicity.@Expr", DEFAULT_MULTIPLICITY_EXPR);
        bindXMLAttrib(this, el, exDir, mult_x, "multiplicityLabel.@InternalPosX", DEFAULT_MULTIPLICITY_OFFSET.x);
        bindXMLAttrib(this, el, exDir, mult_y, "multiplicityLabel.@InternalPosY", DEFAULT_MULTIPLICITY_OFFSET.y);
        bindXMLAttrib(this, el, exDir, mult_k, "multiplicityLabel.@EdgeK", DEFAULT_MULTIPLICITY_EDGEK);
        if (exDir.XmlToFields())
            sanitizeTypeAfterLoad();
    }
}
