/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.Decor;
import editor.domain.DraggableHandle;
import editor.domain.DrawHelper;
import editor.domain.Edge;
import editor.domain.EditableCell;
import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.HandlePosition;
import editor.domain.LabelDecor;
import editor.domain.ListRenderable;
import editor.domain.MovementHandle;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.PointHandlePosition;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import static editor.domain.NetObject.GRAYED_BLACK;
import static editor.domain.NetObject.STYLE_ITALIC;
import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.play.AbstractMarking;
import editor.domain.play.ActivityState;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.RoundRectangle2D;
import java.io.Serializable;
import java.util.ArrayList;
import javax.swing.Icon;
import javax.swing.SwingConstants;
import org.w3c.dom.Element;

/**
 *
 * @author Elvio
 */
public class DtaLocation extends Node implements Serializable {
    
    final double INITIAL_LOC_WIDTH = 3;
    final double INITIAL_LOC_HEIGHT = 2;
    final double LOC_ROUND = 0.5;
    
    public enum FinalType implements ListRenderable {
        NON_FINAL("Non final"), 
        ACCEPTING("Accepting"), 
        REJECTING("Rejecting");
        
        private final String descr;

        private FinalType(String descr) {
            this.descr = descr;
        }
        @Override public String getDescription() { return descr; }
        @Override public Icon getIcon16() { return null; }
        @Override public int getTextSize() { return -1; /* default*/ }
    }
    
    public class StatePropExpr extends Expr {
        public StatePropExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "State proposition expression of location " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.STATE_PROP_EXPR; 
        }
    }
    
    public class VarFlow extends Expr {
        public VarFlow(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Flow of variables at location " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { return 0; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
            return ParserContext.ParserEntryPoint.VAR_FLOW; 
        }
    }
    
    public FinalType finalType = FinalType.NON_FINAL;
    boolean isInitial = false;
    final StatePropExpr spExpr = new StatePropExpr(""); // State proposition expression
    double width = INITIAL_LOC_WIDTH, height = INITIAL_LOC_HEIGHT;
    
    // This label is not shown directly
    private final UniqueNameLabel nameLabel = new UniqueNameLabel(LATEX_NAME_SIZE, new Point2D.Double(0,0));
    
    final StatePropositionLabel spLabel = new StatePropositionLabel();
    final InitialArrowDecor initArrow = new InitialArrowDecor();
    
    final VarFlow varFlow = new VarFlow("");
    final VarFlowLabel varFlowLabel = new VarFlowLabel();
    
    public static final Point2D.Double DEFAULT_STATE_PROP_POS = new Point2D.Double(0.0, 1.5);
    
    class StatePropositionLabel extends NodeLabelDecor {

        public StatePropositionLabel() {
            super(DEFAULT_TEXT_SIZE, DEFAULT_STATE_PROP_POS, 
                  SwingConstants.CENTER, SwingConstants.CENTER);
        }
        
        @Override public String getVisualizedValue() {
            return spExpr.getVisualizedExpr();
        }
        
        @Override
        public Object getValue() { return spExpr.getExpr(); }

        @Override public boolean isCurrentValueValid() { return spExpr.isFormattedAndCorrect(); }
        
        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            return spExpr.isValidExpr(context, (String)value);
        }
        @Override
        public void setValue(ProjectData project, ProjectPage page, Object value) { 
            spExpr.setExpr((String)value);
        }
        @Override public boolean isEditable() { return true; }
        @Override public boolean editAsMultiline() { return false; }
    }
    
    class VarFlowLabel extends NodeLabelDecor {

        public VarFlowLabel() {
            super(DEFAULT_TEXT_SIZE, new Point2D.Double(0, 0), 
                  SwingConstants.CENTER, SwingConstants.TOP);
        }
        
        @Override public String getVisualizedValue() {
            return varFlow.getVisualizedExpr();
        }

        @Override
        public void getAttachPoint(Point2D pt, double K) {
            pt.setLocation(getCenterX(), getSeparatorLineY());
        }
        
        @Override
        public Object getValue() { return varFlow.getExpr(); }

        @Override public boolean isCurrentValueValid() { return varFlow.isFormattedAndCorrect(); }
        
        @Override
        public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            ParserContext context = new ParserContext(page);
            String text = (String)value;
            return text.isEmpty() || varFlow.isValidExpr(context, text);
        }
        @Override
        public void setValue(ProjectData project, ProjectPage page, Object value) { 
            varFlow.setExpr(((String)value).trim());
        }
        @Override public boolean isEditable() { return true; }
        @Override public boolean editAsMultiline() { return false; }
    }
    
    public class InitialArrowDecor extends Decor {
        Edge edge;

        public InitialArrowDecor() {
            ArrayList<Point2D> points = new ArrayList<>();
            double cx = DtaLocation.this.getCenterX();
            double cy = DtaLocation.this.getCenterY();
            double w = DtaLocation.this.getWidth();
            points.add(new Point2D.Double(cx - w/2 - 2, cy));
            points.add(new Point2D.Double(cx, cy));
            edge = new Edge(null, -1, DtaLocation.this, -1, points, false) {
                @Override
                public Edge.ArrowType getArrowType() { return ArrowType.TRIANGLE; }
                @Override public Edge.LineType getLineType() { return Edge.LineType.SOLID; }
                
                @Override
                public boolean canConnectTo(Node node, Edge.EndPoint endPt) {
                    throw new UnsupportedOperationException("Not supported.");
                }                
                @Override
                public int getNumDecors() { return 0; }                
                @Override
                public Decor getDecor(int i) {
                    throw new UnsupportedOperationException("Not supported."); 
                }
                @Override public boolean canBeBroken() { return false; }
                @Override public EditableCell getCentralEditable(double px, double py) { return null; }
                @Override public void paintEdgeDrawings(Graphics2D g, DrawHelper dh) { }
                @Override public double getActivityValue(ActivityState activity) { return -1; }
                @Override public void rewriteEdge(ParserContext context, ExprRewriter rewriter) { }
            };
        }
        
        @Override public void getEffectivePos(Point2D pt) {
            pt.setLocation(getRelativeX(), getRelativeY());
        }        
        @Override
        public double getWidth() { return edge.getWidth(); }
        @Override
        public double getHeight() { return edge.getHeight(); }
        @Override
        public double getRelativeX() { return edge.getX(); }
        @Override
        public double getRelativeY() { return edge.getY(); }
        @Override
        public double getEdgeK() { throw new UnsupportedOperationException(); }
        @Override
        public void setEdgeK(double k) { throw new UnsupportedOperationException("Not supported."); }
        @Override public boolean drawLineBetweenDecorAndEdge() { throw new UnsupportedOperationException("should not be here."); }
 
        @Override
        public void paintDecor(Graphics2D g, DrawHelper dh, boolean isParentGrayed, boolean isParentError) {
            edge.setGrayed(isParentGrayed);
            edge.paintEdge(g, dh);
        }

        public double getInitPointX() { return initArrow.edge.points.get(0).getX(); }
        public double getInitPointY() { return initArrow.edge.points.get(0).getY(); }
        public void setInitPointX(double x) { initArrow.edge.points.get(0).setLocation(x, getInitPointY()); }
        public void setInitPointY(double y) { initArrow.edge.points.get(0).setLocation(getInitPointX(), y); }

        @Override
        public MovementHandle getCenterHandle(final NetPage thisPage) {
            return new DraggableHandle() {

                @Override
                protected DraggableHandle.BoxShape getBoxShape() { return BoxShape.DIAMOND; }

                @Override
                protected Color getHandleColor() { return TAIL_HANDLE_COLOR; }

                @Override
                public HandlePosition savePosition() {
//                    double ax = initArrow.edge.points.get(0).getX() - getX();
//                    double ay = initArrow.edge.points.get(0).getY() - getY();                    
                    return new PointHandlePosition(initArrow.edge.points.get(0).getX(),
                                                   initArrow.edge.points.get(0).getY());
                }

                @Override
                public void restorePosition(HandlePosition hp) {
                    PointHandlePosition php = (PointHandlePosition)hp;
                    initArrow.edge.points.get(0).setLocation(php.point.getX(),
                                                             php.point.getY());
                    initArrow.edge.invalidateEffectiveEdgePath();
                }

                @Override
                public void moveTo(double x, double y, boolean isMultiSelMove) {
                    initArrow.edge.points.get(0).setLocation(x, y);
                }
                
                @Override
                public boolean canMoveTo() { return true;}

                @Override
                public EditableCell getEditable() { return null; }
                
                @Override 
                public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { return NODE_GRID; }
            };
        }

        @Override
        public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                          boolean includeDecors) 
        {
            return edge.intersectRectangle(rect, viewProfile, includeDecors);
        }

        @Override
        public EditableCell getCentralEditable(double px, double py) { return null; }
    }

    public DtaLocation() { }
    
    public DtaLocation(String name, boolean isInitial, FinalType finalType, String spExpr, Point2D pos) {
        this();
        initializeNode(new Point2D.Double(), name);
        setNodePosition(pos.getX(), pos.getY());
        this.isInitial = isInitial;
        this.finalType = finalType;
        this.spExpr.setExpr(spExpr);
    }

    @Override
    public void checkNodeCorrectness(NetPage page, ParserContext context) {
        super.checkNodeCorrectness(page, context); 
        
        spExpr.checkExprCorrectness(context, page, this);
        if (!varFlow.getExpr().isEmpty())
            varFlow.checkExprCorrectness(context, page, this);
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        spExpr.rewrite(context, rewriter);
        if (!varFlow.getExpr().isEmpty())
            varFlow.rewrite(context, rewriter);
    }
    
    public EvaluatedFormula evaluateStatePropositionExpr(ParserContext context, AbstractMarking state) {
        return spExpr.evaluate(context, new EvaluationArguments(state));
    }
    
    private static final float LATEX_NAME_SIZE = 1.2f;
//    @Override
//    public final void setUniqueName(String newName) {
//        uniqueName = newName;
//        latexName = new LatexFormula(newName, LATEX_NAME_SIZE);
//    }
//
//    @Override
//    public String getUniqueName() { return uniqueName; }
    @Override  public EditableCell getUniqueNameEditable() { 
        return new EditableCell() {
            @Override
            public Point2D getEditorCenter() {
                return new Point2D.Double(getCenterX(), getLocationLabelY() + nameLabel.getHeight()/2.0);
            }
            @Override public Object getValue() { return getUniqueName(); }

            @Override
            public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                assert page instanceof NetPage;
                NetPage npage = (NetPage)page;
                Node n = npage.getNodeByUniqueName((String)value);
                return (n==DtaLocation.this) || (n==null); // check uniqueness
            }
            @Override
            public void setValue(ProjectData project, ProjectPage page, Object value) { setUniqueName((String)value); }
            @Override public boolean isEditable() { return true; }
            @Override public boolean isCurrentValueValid() { return true; }

            @Override
            public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                              boolean includeDecors) 
            {
                return DtaLocation.this.intersectRectangle(rect, viewProfile, includeDecors);
            }
            @Override public boolean editAsMultiline() { return false; }
        };
    }

    @Override public double getWidth() { return width; }
    @Override public double getHeight() { return height; }
    @Override public double getRoundX() { return LOC_ROUND; }
    @Override public double getRoundY() { return LOC_ROUND; }
    
    @Override public double getActivityValue(ActivityState activity) { 
//        if (activity != null) {
//            if (activity.firing != null) {
//                double startVal = (activity.firing.intermState.currLoc == this) ? 1 : 0;
//                double endVal = (activity.state.currLoc == this) ? 1 : 0;
//                return DEFAULT_ACTIVITY_AURA_SIZE * linearInterp(startVal, endVal, activity.phase);
//            }
//            else if (activity.state.currLoc == this) // Stand animation
//                return DEFAULT_ACTIVITY_AURA_SIZE/* * activity.phase */;
//        }
        return 0.0;
    }

    @Override
    public double getRotation() { return 0.0; }

    @Override
    public void setRotation(double rotation) {
        throw new UnsupportedOperationException("Not supported.");
    }

    @Override public boolean mayRotate() { return false; }

    @Override public ShapeType getShapeType() { return ShapeType.ROUND_RECTANGLE; }
    
    @Override
    public LabelDecor getUniqueNameDecor() { throw new IllegalStateException("should not be here"); }
    
    public InitialArrowDecor getInitArrowDecor() { return initArrow; }
    public LabelDecor getStatePropositionDecor() { return spLabel; }

    @Override public int getUniqueNameFontStyle() { return STYLE_ITALIC; }
    
    
    @Override public boolean mayResizeWidth() { return true; }
    @Override public boolean mayResizeHeight() { return true; }
    @Override public void setWidth(double newWidth) {
        Point2D arrowPt = initArrow.edge.points.get(0);
        double arrowX = arrowPt.getX() - getX();
        if (arrowX > 0 && arrowX < width) {
            arrowPt.setLocation(getX() + arrowX * (newWidth/width), arrowPt.getY());
            initArrow.edge.invalidateEffectiveEdgePath();
        }            
        width = newWidth; 
    }
    @Override public void setHeight(double newHeight) { 
        Point2D arrowPt = initArrow.edge.points.get(0);
        double arrowY = arrowPt.getY() - getY();
        if (arrowY > 0 && arrowY < height) {
            arrowPt.setLocation(arrowPt.getX(), getY() + arrowY * (newHeight/height));
            initArrow.edge.invalidateEffectiveEdgePath();
        }       
        height = newHeight;
    }
    @Override public double getDefaultWidth() { return INITIAL_LOC_WIDTH; }
    @Override public double getDefaultHeight() { return INITIAL_LOC_HEIGHT; }


    @Override public boolean hasShadow() { return true; }
    
//    @Override public GroupClass getGroupClass() { return GroupClass.LOCATION; }
    @Override public boolean hasSuperPosTags() { return false; }
    @Override public Point2D getSuperPosTagsDefaultPos() { throw new UnsupportedOperationException(); }

    public static final Color CURRENT_LOCATION_CLR = new Color(194, 235, 255);
    public static final Color MOVING_LOCATION_CLR = new Color(230, 245, 255);
    @Override public Color getFillColor(ActivityState activity) {
        if (activity != null) {
            if (activity.firing != null && activity.firing.firedNode instanceof DtaEdge) {
                boolean isStartLoc = (activity.firing.startLoc == this);
                boolean isEndLoc = (activity.state.currLoc == this);
                Color startColor = isStartLoc ? CURRENT_LOCATION_CLR : Color.WHITE;
                Color endColor = isEndLoc ? CURRENT_LOCATION_CLR : Color.WHITE;
                Color intermColor = (isStartLoc && isEndLoc) ? MOVING_LOCATION_CLR : null;
                if (intermColor == null)
                    return blend(startColor, endColor, activity.getFiringCoeff());
                
                if (activity.getFiringCoeff() <= 0.5)
                    return blend(startColor, intermColor, activity.getFiringCoeff() * 2);
                else
                    return blend(intermColor, endColor, (activity.getFiringCoeff() - 0.5) * 2);
            }
            else if (activity.state.currLoc == this) // Stand animation
                return CURRENT_LOCATION_CLR;
        }
        return Color.WHITE; 
    }
    @Override public Color getFillColorGrayed()  { return Color.WHITE; }
    @Override public Color getFillColorError()   { return Color.WHITE; }
    
    @Override public Color getBorderColor()        { return Color.BLACK; }
    @Override public Color getBorderColorGrayed()  { return GRAYED_BLACK; }
    @Override public Color getBorderColorError()   { return Color.RED; }

    @Override
    public final void setNodePosition(double x, double y) {
        double ax = initArrow.edge.points.get(0).getX() - getX();
        double ay = initArrow.edge.points.get(0).getY() - getY();
        super.setNodePosition(x, y); 
        // reposition the initial arrow points
        initArrow.edge.points.get(0).setLocation(getX() + ax, getY() + ay);
        initArrow.edge.points.get(1).setLocation(getX(), getY());
        initArrow.edge.invalidateEffectiveEdgePath();
    }

    @Override
    public void setSelected(boolean isSelected) {
        super.setSelected(isSelected); 
        initArrow.edge.setSelected(isSelected);
    }
    
    private static final RoundRectangle2D pniDoubleBorder = new RoundRectangle2D.Double();
    private static final Line2D pniLine = new Line2D.Double();
    private static final Rectangle2D pniRect = new Rectangle2D.Double();
    @Override
    protected void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor,
                                     boolean errorFlag, Shape nodeShape) 
    {
        final double a = 0.2;
        
        Stroke oldStroke = g.getStroke();
        g.setStroke(getBorderWidth().logicStroke);

        //finalType = FinalType.REJECTING;
        if (finalType != FinalType.NON_FINAL) {
            RoundRectangle2D rrShape = (RoundRectangle2D)nodeShape;
            pniDoubleBorder.setRoundRect(rrShape.getX() + a, rrShape.getY() + a,
                                      rrShape.getWidth() - 2*a, rrShape.getHeight() - 2*a,
                                      rrShape.getArcWidth()/2, rrShape.getArcHeight()/2);
            g.setColor(errorFlag ? Color.RED : borderColor);
            g.draw(pniDoubleBorder);
            if (finalType == FinalType.REJECTING) {
                pniLine.setLine(getX()+getWidth(), getY(), getX(), getY()+getHeight());
                g.draw(pniLine);
            }
        }
        
        boolean drawVarFlow = drawVarFlows();
        
        // Draw the location name 
        double textX = getCenterX() - nameLabel.getWidth() / 2.0;
        double textY = getLocationLabelY();
//        if (drawVarFlow)
//            textY -= varFlowLabel.getHeight() / 2.0;
        nameLabel.paintDecorAt(g, dh, textX, textY, isGrayed(), errorFlag);
        
        // Draw the variable flows
        double varFlY = getSeparatorLineY();
        if (drawVarFlow) {
            double varFlX = getCenterX() - varFlowLabel.getWidth() / 2.0;
            varFlowLabel.paintDecorAt(g, dh, varFlX, varFlY, isGrayed(), errorFlag);
        }
        
        if (getHeight() > INITIAL_LOC_HEIGHT) {
            // Draw the horizontal separator
            pniLine.setLine(getX(), varFlY, getX()+getWidth(), varFlY);
            g.draw(pniLine);
            // Draw the empty block
            final float INFL = 0.5f, BOXHEIGHT=0.8f;
            if (isSelected() && !drawVarFlow && getHeight() >= INITIAL_LOC_HEIGHT + 2*INFL + BOXHEIGHT) {
                float unit = (float)dh.screenToLogic(1);
                Stroke softStroke = new BasicStroke(unit, 
                        BasicStroke.CAP_SQUARE, BasicStroke.JOIN_MITER, 
                        1.0f, new float[] {6*unit, 3*unit}, 0.0f);
                g.setStroke(softStroke);
                g.setColor(VARFLOW_BOX_COLOR);
                pniRect.setFrame(getX() + INFL, varFlY + 0.25f, getWidth() - 2*INFL, BOXHEIGHT);
                g.draw(pniRect);
            }
        }
        
//        if (latexName != null) {
//            double scaleFact = 1.0 / (double)UNIT_TO_PIXELS;
//            double textX = getCenterX() - (latexName.getWidth() * scaleFact) / 2.0;
//            double textY = getCenterY() - (latexName.getHeight() * scaleFact) / 2.0;
//            latexName.draw(g, textX, textY, scaleFact, false);
//        }
        
        g.setStroke(oldStroke);
    }
    private static final Color VARFLOW_BOX_COLOR = new Color(153, 204, 255);
    
    private boolean drawVarFlows() {
        return !(varFlow.getExpr().isEmpty());
    }
    
    private double getSeparatorLineY() {
        return getY() + INITIAL_LOC_HEIGHT;
//        double varFlowHeight = (drawVarFlows() ? varFlowLabel.getHeight() : 0.0);
//        return getCenterY() + (nameLabel.getHeight() - varFlowHeight) / 2.0;
    }
    
    private double getLocationLabelY() {
        return getY() + (getSeparatorLineY() - getY() - nameLabel.getHeight()) / 2.0;
    }
    

    @Override
    public int getNumDecors() {
        return (isInitial ? 2 : 1);
    }

    @Override
    public Decor getDecor(int i) {
        if (i==0)
            return spLabel;
        if (isInitial && i==1)
            return initArrow;
        throw new IllegalArgumentException("decor index"); 
    }
    
    @Override public EditableCell getCentralEditable(double px, double py) { 
        if (py < getSeparatorLineY())
            return getUniqueNameEditable();
        else
            return varFlowLabel.getCentralEditable(px, py);
    }
    
    
    
    public EditableValue getInitialEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return isInitial ? Boolean.TRUE : Boolean.FALSE; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                isInitial = (Boolean)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return true; }
        };
    }

    public boolean isInitial()          { return isInitial; }
    public boolean isFinalAccepting()   { return finalType == FinalType.ACCEPTING; }
    public boolean isFinalRejecting()   { return finalType == FinalType.REJECTING; }
    public boolean isNotFinal()         { return finalType == FinalType.NON_FINAL; }
    
    
    public EditableValue getFinalTypeEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return finalType; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) 
            { return (finalType instanceof FinalType); }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                finalType = (FinalType)value;
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public boolean isEditable() { return true; }
        };
    }

    public EditableValue getStatePropositionEditable() {
        return spLabel;
    }
    
    public EditableValue getVarFlowEditable() {
        return varFlowLabel;
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
    
        bindXMLAttrib(this, el, exDir, "initial", "isInitial", false);
        bindXMLAttrib(this, el, exDir, "final", "finalType", FinalType.NON_FINAL);
        bindXMLAttrib(this, el, exDir, "state-proposition", "spExpr.@Expr", "True");
        bindXMLAttrib(this, el, exDir, "variable-flow", "varFlow.@Expr", "");
        bindXMLAttrib(this, el, exDir, "state-proposition-x", "spLabel.@InternalPosX", DEFAULT_STATE_PROP_POS.x);
        bindXMLAttrib(this, el, exDir, "state-proposition-y", "spLabel.@InternalPosY", DEFAULT_STATE_PROP_POS.y);
        bindXMLAttrib(this, el, exDir, "init-arrow-x", "initArrow.@InitPointX", -4.0);
        bindXMLAttrib(this, el, exDir, "init-arrow-y", "initArrow.@InitPointY", 0.0);
    }
}
