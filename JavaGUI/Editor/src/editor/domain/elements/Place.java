/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.domain.AlternateNameFunction;
import editor.domain.Decor;
import editor.domain.DrawHelper;
import editor.domain.EditableCell;
import editor.domain.EditableValue;
import editor.domain.Expr;
import editor.domain.LabelDecor;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import static editor.domain.LabelDecor.DEFAULT_TEXT_SIZE;
import static editor.domain.LabelDecor.DEFAULT_SMALL_TEXT_SIZE;
import static editor.domain.NetObject.GRAYED_BLACK;
import editor.domain.values.EvaluatedFormula;
import editor.domain.grammar.EvaluationArguments;
import editor.domain.grammar.ExprLangParser;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.play.ActivityState;
import editor.domain.values.IntScalarValue;
import editor.domain.values.MultiSet;
import editor.domain.values.RealScalarValue;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.font.GlyphVector;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.Locale;
import javax.swing.SwingConstants;
import latex.LatexFormula;
import org.w3c.dom.Element;

/** A place in a GSPN.
 *
 * @author Elvio
 */
public class Place extends Node implements Serializable, ColorClass.DomainHolder {
    
    public static final double PLACE_RADIUS = 1.0;
    public static final double TOKEN_RADIUS = 0.2;
    public static final double DOUBLE_BORDER_WIDTH = 0.2;
    
    public class InitMarkingExpr extends Expr {
        public InitMarkingExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Initial marking expression of place " + nameLabel.getValue() + ".";
        }
        @Override protected int getParseFlags() { return ParserContext.PF_CONST_EXPR; }
        @Override protected ParserContext.ParserEntryPoint getParseRule(String exprText) { 
//            if (!colorDomainExpr.isFormattedAndCorrect())
//                return ParserContext.ParserEntryPoint.SKIP_PARSE;
            if (isInNeutralDomain()) {
                return type == TokenType.DISCRETE ?
                        ParserContext.ParserEntryPoint.INT_EXPR :
                        ParserContext.ParserEntryPoint.REAL_EXPR; 
            }
            else {
                return type == TokenType.DISCRETE ?
                        ParserContext.ParserEntryPoint.INT_MULTISET_EXPR :
                        ParserContext.ParserEntryPoint.REAL_MULTISET_EXPR; 
            }
        }

        @Override
        public void setExpr(String newExpr) {
            if (newExpr.equals("0"))
                newExpr = "";
            super.setExpr(newExpr); 
        }

        @Override
        public boolean isValidExpr(ParserContext context, String text) {
            if (text.equals(""))
                return true;
            return super.isValidExpr(context, text); 
        }

        @Override
        public EvaluatedFormula evaluate(ParserContext context, EvaluationArguments args) {
            if (getExpr().equals("")) {
                if (isInNeutralDomain())
                    return isDiscrete() ? IntScalarValue.ZERO : RealScalarValue.ZERO;
                else
                    return MultiSet.makeNew(type.getFormulaType(), getColorDomain(), EvaluatedFormula.EMPTY_MAP);
            }
            return super.evaluate(context, args); 
        }

        @Override
        public String reduce(ParserContext context, EvaluationArguments args, ExpressionLanguage lang) {
            if (getExpr().equals(""))
                return evaluate(context, args).toStringFormat(lang, "%.10f");
            return super.reduce(context, args, lang); 
        }

        @Override
        public boolean prepareContext(ParserContext context) {
            if (isInColorDomain()) {
                context.colorDomainOfExpr = getColorDomain();
                return (context.colorDomainOfExpr != null);
            }
            return true;
        }
    }
    class ColorDomainExpr extends ColorClass.ColorDomainExprBase {
        public ColorDomainExpr(String expr) { super(expr); }
        @Override protected String getExprDescr() { 
            return "Color domain of place " + getUniqueName() + ".";
        }
    }
    
    // The type of this place (discrete or continuous)
    private TokenType type;
    
    // The initial marking expression.
    private final InitMarkingExpr initMarkingExpr = new InitMarkingExpr("");

    private static final float MARKING_TEXT_FORMULA_SIZE = 0.8f;
    private static final float MARKING_TEXT_NUMBER_SIZE = 1.0f;
    
    // The name label
    final PlaceUniqueNameLabel nameLabel = new PlaceUniqueNameLabel(DEFAULT_UNIQUE_NAME_POS);
    
    // Superposition tags label
    private String superPosTags = "";
    private final SuperPosTagsLabel superPosTagLabel = new SuperPosTagsLabel(DEFAULT_SMALL_TEXT_SIZE, 
                                                                             DEFAULT_SUPERPOS_TAGS_POS);
    
    // Color domain
    private final ColorDomainExpr colorDomainExpr = new ColorDomainExpr("");
    private final ColorDomainLabel colorDomainLabel = new ColorDomainLabel();

    // Name of the Kronecker partition/component (APNN file format)
    private String partitionName;
    private final KroneckerPartitionLabel partitionNameLabel = new KroneckerPartitionLabel();
    
    public static final Point2D.Double DEFAULT_UNIQUE_NAME_POS = new Point2D.Double(0.0, 1.5);
    public static final Point2D.Double DEFAULT_SUPERPOS_TAGS_POS = new Point2D.Double(0.0, 2.5);
    public static final Point2D.Double DEFAULT_KRONECKER_PART_POS = new Point2D.Double(0.0, 2.5);

    public Place(String name, String initMarking, TokenType type, String colorDomain, 
                 String partitionName, Point2D pos) 
    {
        initializeNode(pos, name);
        this.type = type;
        this.initMarkingExpr.setExpr(initMarking);
        colorDomainExpr.setExpr(colorDomain);
        this.partitionName = partitionName;
    }
    
    public Place() { }
    
    // Modified name label, that is centered for neutral places,
    // and draw side-by-side to the color domain label for colored places.
    class PlaceUniqueNameLabel extends UniqueNameLabel {

        public PlaceUniqueNameLabel(Point2D pos) {
            super(DEFAULT_TEXT_SIZE, pos);
        }
        
        @Override public int getHorizontalAlignment() { 
            if (isInNeutralDomain())
                return SwingConstants.CENTER;
            return SwingConstants.RIGHT;
        }
        @Override public double getRelativeX() {
            if (isInNeutralDomain())
                return super.getRelativeX();
            return super.getRelativeX() - 0.1;
        }
    }
    
    class ColorDomainLabel extends NodeLabelDecor {
        public ColorDomainLabel() {
            super(DEFAULT_TEXT_SIZE, new Point2D.Double(0.0, -1.5), 
                  SwingConstants.LEFT, SwingConstants.CENTER);
        }

        // Keep position in synch with the place name label.
        @Override public double getRelativeX() {
            return nameLabel.getRelativeX() + nameLabel.getWidth() + 0.2; 
        }
        @Override public double getRelativeY() {
            return nameLabel.getRelativeY(); 
        }
        @Override public double getInternalPosX() {
            return nameLabel.getInternalPosX(); 
        }
        @Override public double getInternalPosY() {
            return nameLabel.getInternalPosY(); 
        }
        @Override public void setInternalPos(double x, double y) {
            nameLabel.setInternalPos(x, y);
        }
        
        @Override public String getVisualizedValue() { 
            return colorDomainExpr.getVisualizedExpr();
        }

        @Override public boolean isCurrentValueValid() { return colorDomainExpr.isFormattedAndCorrect(); }
        
        // When this label is shown, draw a ':' between this label and the name label.
        LatexFormula semicolon;
        @Override
        public void paintDecorAt(Graphics2D g, DrawHelper dh, double textX, double textY, 
                                 boolean isParentGrayed, boolean isParentError) 
        {
            super.paintDecorAt(g, dh, textX, textY, isParentGrayed, isParentError); 
            // Paint the ':' between the name label and the domain label.
            float semicolonSize = DEFAULT_TEXT_SIZE * (float)getUnitToPixels();
            if (semicolon == null || semicolon.getSize() != semicolonSize) {
                semicolon = new LatexFormula(":", semicolonSize);
            }
            if (semicolon != null) {
                double scaleFact = 1.0 / (double)getUnitToPixels();
//                double eY = textY;
//                if (semicolon.getHeight() < semicolon.getLineHeight()) {
                double incr = (semicolon.getLineHeight() - semicolon.getLineDepth()) -
                              (semicolon.getHeight() - semicolon.getDepth());
                textY += incr / (double)getUnitToPixels();
//                }
                Color oldColor = g.getColor();
                Color textColor = isParentError ? Color.RED : 
                        (isParentGrayed ? GRAYED_BLACK : Color.BLACK);
                g.setColor(textColor);
                semicolon.draw(g, textX - semicolon.getWidth()*scaleFact*0.65,
                               textY, scaleFact, false);
                g.setColor(oldColor);
            }
        }
        
        @Override public Object getValue() { return colorDomainExpr.getExpr(); }
        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            if (((String)value).isEmpty())
                return true; // Accept the empty color domain, which is the neutral domain.
            ParserContext context = new ParserContext(page);
            return colorDomainExpr.isValidExpr(context, (String)value);
        }
        @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
            colorDomainExpr.setExpr((String)value);
        }
        @Override public boolean isEditable() { return true; }
        @Override public boolean isVisible(ViewProfile vp)  { 
            return vp.viewNames && isInColorDomain(); 
        }
        @Override public boolean editAsMultiline() { return false; }
        @Override public Size getTextSize() { return getGfxTextSize(); }
    }
        
    // A simple string that denotes the Kronecker partition (used in hierarchical PN)
    class KroneckerPartitionLabel extends NodeLabelDecor {
        public KroneckerPartitionLabel() {
            super(DEFAULT_SMALL_TEXT_SIZE, DEFAULT_KRONECKER_PART_POS, 
                  SwingConstants.CENTER, SwingConstants.CENTER);
        }
        
        @Override public boolean isEditable() { return true; }
        @Override public Object getValue() { return partitionName; }
        @Override public boolean isCurrentValueValid() {
            return partitionName.length() == 0 || isAlphanumericIdentifier(partitionName);
        }

        @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
            String newVal = (String)value;
            return newVal.length() == 0 || isAlphanumericIdentifier(newVal);
        }

        @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
            partitionName = (String)value;
        }

        @Override public boolean isVisible(ViewProfile vp) {
            return vp.viewSuperPosTags && partitionName.length() > 0;
        }
        @Override public String getVisualizedValue() { 
            return AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS.prepareLatexText(partitionName, null, STYLE_ROMAN);
        }
        @Override public boolean editAsMultiline() { return false; }
        @Override public Size getTextSize() { return getGfxTextSize(); }
    }
    
    // Return the domain name, or null if it is neutral
    @Override public String getColorDomainName() {
        return colorDomainExpr.getColorDomainName();
    }
    
    // this method return the parsed color domain of this place, which could
    // be the neutral color if the place is uncolored. It returns null if there
    // are syntactic/semantic errors in the place definition.
    @Override public ColorClass getColorDomain() { 
        return colorDomainExpr.getColorDomain(); 
    }
    @Override public boolean isInNeutralDomain() {
        return colorDomainExpr.isInNeutralDomain();
    }
    @Override public boolean isInColorDomain() {
        return colorDomainExpr.isInColorDomain();
    }
    @Override public EditableValue getColorDomainEditable() {
        return colorDomainExpr.getColorDomainEditable();
    }
    
    public EditableValue getKroneckerPartitionNameEditable() {
        return partitionNameLabel;
    }
    public LabelDecor getKroneckerPartitionNameDecor() { 
        return partitionNameLabel;
    }
    public String getKroneckerPartition() { return partitionName; }

    @Override
    public double getWidth() { return 2 * PLACE_RADIUS ; }

    @Override
    public double getHeight() { return 2 * PLACE_RADIUS ; }

    @Override
    public double getRotation() { return 0.0; }
    
    @Override
    public void setRotation(double rotation) {
        throw new UnsupportedOperationException("Not supported.");
    }

    @Override
    public boolean mayRotate() { return false; }

    @Override public double getRoundX() { throw new IllegalStateException("should not be here"); }
    @Override public double getRoundY() { throw new IllegalStateException("should not be here"); }

    @Override public double getActivityValue(ActivityState activity) { return 0.0; }

    @Override  public EditableCell getUniqueNameEditable() { return nameLabel; }    

    @Override
    public ShapeType getShapeType() { return ShapeType.CIRCLE; }

    @Override
    public LabelDecor getUniqueNameDecor() { return nameLabel; }
    
    @Override public boolean mayResizeWidth() { return false; }
    @Override public boolean mayResizeHeight() { return false; }
    @Override public void setWidth(double newWidth) { throw new IllegalStateException("should not be here"); }
    @Override public void setHeight(double newHeight) { throw new IllegalStateException("should not be here"); }
    @Override public double getDefaultWidth() { throw new UnsupportedOperationException("Not supported."); }
    @Override public double getDefaultHeight() { throw new UnsupportedOperationException("Not supported."); }

    @Override
    public Point2D getSuperPosTagsDefaultPos() { return DEFAULT_SUPERPOS_TAGS_POS; }

    @Override public boolean hasShadow() { return true; }
    
    public boolean isDiscrete() { return type == TokenType.DISCRETE; }
    public boolean isContinuous() { return type == TokenType.CONTINUOUS; }
    public TokenType getType() { return type; }
    
    
    // Token positions (when draw as dots)
    final static Point2D.Double tokenPos[][] = {
        {},
        {new Point2D.Double(0, 0)},
        {new Point2D.Double(-1.5, 0), new Point2D.Double(1.5, 0)},
        {new Point2D.Double(0.000, -1.600), new Point2D.Double(1.386, 0.800),
         new Point2D.Double(-1.386, 0.800)},
        {new Point2D.Double(-1.5, -1.5), new Point2D.Double(1.5, -1.5),
         new Point2D.Double(-1.5, 1.5), new Point2D.Double(1.5, 1.5)},
        {new Point2D.Double(0.000, -2.100), new Point2D.Double(1.997, -0.649),
         new Point2D.Double(1.234, 1.699), new Point2D.Double(-1.234, 1.699),
         new Point2D.Double(-1.997, -0.649)},
        {new Point2D.Double(0.000, -2.400), new Point2D.Double(2.078, -1.200),
         new Point2D.Double(2.078, 1.200), new Point2D.Double(0.000, 2.400),
         new Point2D.Double(-2.078, 1.200), new Point2D.Double(-2.078, -1.200)},
        {new Point2D.Double(0.000, -2.400), new Point2D.Double(2.078, -1.200),
         new Point2D.Double(2.078, 1.200), new Point2D.Double(0.000, 2.400),
         new Point2D.Double(-2.078, 1.200), new Point2D.Double(-2.078, -1.200),
         new Point2D.Double(0, 0)},
        {new Point2D.Double(1.148, -2.772), new Point2D.Double(2.772, -1.148),
         new Point2D.Double(2.772, 1.148), new Point2D.Double(1.148, 2.772),
         new Point2D.Double(-1.148, 2.772), new Point2D.Double(-2.772, 1.148),
         new Point2D.Double(-2.772, -1.148), new Point2D.Double(-1.148, -2.772)},
        {new Point2D.Double(-0.000, -3.111), new Point2D.Double(-1.556, -1.556), new Point2D.Double(-3.111, 0.000),
         new Point2D.Double(1.556, -1.556), new Point2D.Double(0.000, 0.000), new Point2D.Double(-1.556, 1.556),
         new Point2D.Double(3.111, -0.000), new Point2D.Double(1.556, 1.556), new Point2D.Double(0.000, 3.111)}
    };
    
    @Override public Color getFillColor(ActivityState activity)        { return Color.WHITE; }
    @Override public Color getFillColorGrayed()  { return Color.WHITE; }
    @Override public Color getFillColorError()   { return Color.WHITE; }
    
    @Override public Color getBorderColor()        { return Color.BLACK; }
    @Override public Color getBorderColorGrayed()  { return GRAYED_BLACK; }
    @Override public Color getBorderColorError()   { return Color.RED; }
    
    private enum PaintMode {
        INT_TOKENS, FLUID_LEVEL, LATEX_TEXT
    }

    private static final Ellipse2D doubleBorderEllipse = new Ellipse2D.Double();
    @Override
    protected void paintNodeInterior(Graphics2D g, DrawHelper dh, Color borderColor,
                                     boolean errorFlag, Shape nodeShape) 
    {
        // Paint the double border for continuous places
        if (isContinuous()) {
            Stroke sk = g.getStroke();
            g.setStroke(getBorderWidth().logicStroke);
            g.setColor(borderColor);
            Rectangle2D bound = nodeShape.getBounds2D();
            doubleBorderEllipse.setFrame(bound.getX() + DOUBLE_BORDER_WIDTH,
                                          bound.getY() + DOUBLE_BORDER_WIDTH,
                                          bound.getWidth() - 2 * DOUBLE_BORDER_WIDTH,
                                          bound.getHeight()- 2 * DOUBLE_BORDER_WIDTH);
            g.draw(doubleBorderEllipse);
            g.setStroke(sk);
        }
        
        // Get the marking to draw
        int marking = -200;
        double fluidLevel = -200.0;
        String latexText = null;
        EvaluatedFormula efMarking = null;
        PaintMode paintMode = null;
        boolean isError = false;
        if (dh.selectedPTFlow != null) {
            // Do not draw any marking in P/T semiflow mode.
            return;
        }
        if (dh.measures != null) {
            return; // Do not draw the tokens in measure view mode.
        }
        else if (dh.activity == null) { // Edit mode, show the initial Marking
            if (isDiscrete() && NetObject.isInteger(initMarkingExpr.getExpr())) {
                try {
                    marking = Integer.parseInt(initMarkingExpr.getExpr());
                    paintMode = PaintMode.INT_TOKENS;
                }
                catch (NumberFormatException e) {
                    latexText = "??";
                    paintMode = PaintMode.LATEX_TEXT;
                    isError = true;
                }
            }
            else {
                paintMode = PaintMode.LATEX_TEXT;
                latexText = (initMarkingExpr.isFormatted()) ? initMarkingExpr.getFormatted() : initMarkingExpr.getExpr();
                isError = !initMarkingExpr.isFormattedAndCorrect();
            }
        }
        else { // Show the current marking.
            if (isDiscrete()) {
                if (dh.activity.firing != null) { // Marking during the firing
                    if (dh.activity.isFlowPhase())
                        efMarking = dh.activity.firing.markingAtFlowBegin.get(this);
                    else // firing phase
                        efMarking = dh.activity.firing.markingDuringFiring.get(this);
                }
                else // Marking in stand mode
                    efMarking = dh.activity.state.getMarkingOfPlace(this);
            }
            else { // is continuous
                if (dh.activity.firing != null) { // Fluid level during the firing
                    double lvl;
                    if (dh.activity.isFlowPhase()) {
                        // Fluid moving continuously
                        EvaluatedFormula start = dh.activity.firing.markingAtFlowBegin.get(this);
                        EvaluatedFormula end = dh.activity.firing.markingAtFlowEnd.get(this);
                        EvaluatedFormula diff = end.evalBinaryFn(ExprLangParser.SUB, start);
                        EvaluatedFormula diffK = diff.evalBinaryFn(ExprLangParser.MUL, 
                                                       RealScalarValue.makeNew(dh.activity.getFlowCoeff()));
                        efMarking = start.evalBinaryFn(ExprLangParser.ADD, diffK);
                    }
                    else { // Fluid moving over the edges
                        assert dh.activity.isFiringPhase();
                        efMarking = dh.activity.firing.markingDuringFiring.get(this);
                    }
                }
                else // Fluid level in stand mode
                    efMarking = dh.activity.state.getMarkingOfPlace(this);
            }
        }
        if (efMarking != null) {
            if (efMarking.isScalarInt()) {
                paintMode = PaintMode.INT_TOKENS;
                marking = efMarking.getScalarInt();
            }
            else if (efMarking.isScalarReal()) {
                fluidLevel = efMarking.getScalarReal();
                paintMode = PaintMode.FLUID_LEVEL;
            }
            else if (efMarking.isMultiSet()) {
                paintMode = PaintMode.LATEX_TEXT;
                latexText = ((MultiSet)efMarking).toStringFormat(ExpressionLanguage.LATEX, "%.2f");
            }
            else throw new IllegalStateException("unimplemented!");
        }
        
        // Set the color
        Color oldColor = g.getColor();
        g.setColor(isError || errorFlag ? Color.RED : (isGrayed() ? GRAYED_BLACK : Color.black));
        
        // Draw the marking
        assert paintMode != null;
        switch (paintMode) {
            case INT_TOKENS:
                paintIntegerTokenCountAt(g, dh, marking, getCenterX(), getCenterY());
                break;
                
            case FLUID_LEVEL:
                paintTextAt(g, dh, fluidDecimalFormat.format(fluidLevel), getCenterX(), getCenterY());
                break;
                
            case LATEX_TEXT:
                LatexFormula cache = paintLatexTokenCountAt(g, dh, initMarkingExpr.getLatexExpr(), latexText, 
                                                            getCenterX(), getCenterY());
                initMarkingExpr.setLatexExpr(cache);
                break;
        }
        g.setColor(oldColor);
    }
    private static final DecimalFormat fluidDecimalFormat = new DecimalFormat("#.##", new DecimalFormatSymbols(Locale.US));
    
    
    public static void paintIntegerTokenCountAt(Graphics2D g, DrawHelper dh, 
                                                int tokenCount, double cx, double cy) 
    {
        if (tokenCount >= tokenPos.length || tokenCount < 0) {
            paintTextAt(g, dh, "" + tokenCount, cx, cy);
            return;
        }
        for (Point2D.Double item : tokenPos[tokenCount]) {
            dh.drawLogicPoint(g, cx + (item.x) * TOKEN_RADIUS, 
                                 cy + (item.y) * TOKEN_RADIUS, TOKEN_RADIUS);
        }
    }
    
    public static void paintRealTokenCountAt(Graphics2D g, DrawHelper dh,
                                             double fluidLevel, double cx, double cy) 
    {
        paintTextAt(g, dh, "" + fluidLevel, cx, cy);
    }
    
    static Font markingTextFont = new Font("jlm_cmr10", Font.PLAIN, 1);
    public static void paintTextAt(Graphics2D g, DrawHelper dh,
                                   String line, double cx, double cy) 
    {
        Font f = markingTextFont.deriveFont(MARKING_TEXT_FORMULA_SIZE);
        
        g.setFont(f);
//        float width = fm.stringWidth(line);
//        float bx = dh.logicToScreen(cx) - width / 2.0f;
//        float by = dh.logicToScreen(cy) + fm.getHeight() / 2.0f;
//        by -= (fm.getDescent());
        
//        AffineTransform oldAt = g.getTransform();
//        g.setTransform(dh.baseTransform);
//        g.scale(dh.screenToLogic(MARKING_TEXT_FORMULA_SIZE), 
//                dh.logicToScreen(MARKING_TEXT_FORMULA_SIZE));
//        g.scale(1/dh.unitSize, 1/dh.unitSize);
        
        GlyphVector gv = f.layoutGlyphVector(g.getFontRenderContext(), line.toCharArray(), 
                                             0, line.length(), Font.LAYOUT_LEFT_TO_RIGHT);
        Rectangle2D bound = gv.getLogicalBounds();
//        g.drawString(line, bx, by);

        float bx = (float)(cx - bound.getWidth() / 2.0);
        float by = (float)(cy + bound.getHeight()/ 2.0);
        by -= f.getLineMetrics(line, g.getFontRenderContext()).getDescent();
        g.drawGlyphVector(gv, bx, by);
        
//        g.setTransform(oldAt);
    }
    
    
    public static LatexFormula paintLatexTokenCountAt(Graphics2D g, DrawHelper dh, LatexFormula tokenText,
                                                      String tokenNumber, double cx, double cy) 
    {
        boolean rebuildFormula = false;
        if (tokenText == null || !tokenText.getLatex().equals(tokenNumber))
            rebuildFormula = true;
        
        // Determine size
        int i;
        for (i=0; i<tokenNumber.length(); i++)
            if (tokenNumber.charAt(i) < '0' || tokenNumber.charAt(i) > '9')
                break;
        // Change the size for pure numbers or for formulas
        float size = (i==tokenNumber.length()) ? MARKING_TEXT_NUMBER_SIZE : MARKING_TEXT_FORMULA_SIZE;
        if (tokenText != null && size != tokenText.getSize())
            rebuildFormula = true;
        
        if (rebuildFormula)
            tokenText = new LatexFormula(tokenNumber, size  * (float)getUnitToPixels());
//        if (tokenText == null || !tokenText.getLatex().equals(tokenNumber)) {
//            int i;
//            for (i=0; i<tokenNumber.length(); i++)
//                if (tokenNumber.charAt(i) < '0' || tokenNumber.charAt(i) > '9')
//                    break;
//            // Change the size for pure numbers or for formulas
//            float size = (i==tokenNumber.length()) ? MARKING_TEXT_NUMBER_SIZE : MARKING_TEXT_FORMULA_SIZE;
//                
//            // Prepare a new LaTeX text
//            tokenText = new LatexFormula(tokenNumber, size  * (float)getUnitToPixels());
//        }
        double scaleFact = 1.0 / (double)getUnitToPixels();
        double textX = cx - (tokenText.getWidth() * scaleFact) / 2.0;
        double textY = cy - (tokenText.getHeight() * scaleFact * 0.9) / 2.0;
        tokenText.draw(g, textX, textY, scaleFact, false);
        return tokenText;
    }

    @Override public boolean hasSuperPosTags() { return true; }
//    @Override public GroupClass getGroupClass() { return GroupClass.PLACE; }
    @Override public String getSuperPosTags() { return superPosTags; }
    @Override public void setSuperPosTags(String superPosTags) { 
//        assert isValidTagList(superPosTags);
        this.superPosTags = superPosTags;
        invalidateTagList();
    }
    
    @Override
    public EditableValue getSuperPosTagsLabelEditable() { return superPosTagLabel; }

    @Override public LabelDecor getSuperPosTagsDecor() { return superPosTagLabel; }

    @Override
    public int getNumDecors() {
        return 4; // name label + superPosTags + colorDomain + partitionName
    }

    @Override
    public Decor getDecor(int i) {
        switch (i) {
            case 0:
                return nameLabel;
            case 1:
                return superPosTagLabel;
            case 2:
                return colorDomainLabel;
            case 3:
                return partitionNameLabel;
        }
        throw new UnsupportedOperationException("Wrong decor index."); 
    }

    @Override
    public void checkNodeCorrectness(NetPage page, ParserContext context) {
        super.checkNodeCorrectness(page, context);
        
        if (isInColorDomain())
            colorDomainExpr.checkExprCorrectness(context, page, this);
        
        if (!initMarkingExpr.getExpr().isEmpty()) {
            initMarkingExpr.checkExprCorrectness(context, page, this);
        }
    }

    @Override
    public void rewriteNode(ParserContext context, ExprRewriter rewriter) {
        if (isInColorDomain())
            colorDomainExpr.rewrite(context, rewriter);
        if (!initMarkingExpr.getExpr().isEmpty()) {
            initMarkingExpr.rewrite(context, rewriter);
        }
    }
    
    public EvaluatedFormula evaluateInitMarking(ParserContext context) {
        return initMarkingExpr.evaluate(context, EvaluationArguments.NO_ARGS);
    }
    
    public String reduceInitMarking(ParserContext context, ExpressionLanguage lang) {
        return initMarkingExpr.reduce(context, EvaluationArguments.NO_ARGS, lang);
    }
    
    public String convertInitMarkingLang(ParserContext context, ExpressionLanguage lang) {
        return initMarkingExpr.convertLang(context, EvaluationArguments.NO_ARGS, lang);
    }
    
    public String getInitMarkingExpr() {
        return initMarkingExpr.getExpr();
    }
    
    
    public EditableCell getInitMarkingEditable() {
        return new EditableCell() {
            @Override public Object getValue() { return initMarkingExpr.getExpr(); }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                ParserContext context = new ParserContext(page);
                return initMarkingExpr.isValidExpr(context, (String)value);
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) { 
                initMarkingExpr.setExpr((String)value);
            }
            @Override public boolean isEditable() { return true; }
            @Override public boolean isCurrentValueValid() { return initMarkingExpr.isFormattedAndCorrect(); }
            
            @Override public Point2D getEditorCenter() {
                return new Point2D.Double(getCenterX(), getCenterY());
            }
            @Override
            public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                              boolean includeDecors) 
            {
                return Place.this.intersectRectangle(rect, viewProfile, includeDecors);
            }
            @Override public boolean editAsMultiline() { return false; }
        };
    }
    
    @Override public EditableCell getCentralEditable(double px, double py) { 
        return getInitMarkingEditable();
    }
    
    public EditableValue getTypeEditable() {
        return new EditableValue() {
            @Override public Object getValue() { return type; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object nvalue) {
                ParserContext context = new ParserContext(page);
                TokenType currType = type;
                type = (TokenType)nvalue;
                boolean isValid = initMarkingExpr.isValidExpr(context, initMarkingExpr.getExpr());
                type = currType; //  Set back the actual type.
                return isValid;
            }
            @Override public void setValue(ProjectData project, ProjectPage page, Object nvalue) { 
                type = (TokenType)nvalue;
            }
            @Override public boolean isEditable() { 
                return true;
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }

    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        super.exchangeXML(el, exDir); 
        
        bindXMLAttrib(this, el, exDir, "type", "type", TokenType.DISCRETE);
        bindXMLAttrib(this, el, exDir, "marking", "initMarkingExpr.@Expr", "");
        if (exDir.XmlToFields() || isInColorDomain())
            bindXMLAttrib(this, el, exDir, "domain", "colorDomainExpr.@Expr", "");
        bindXMLAttrib(this, el, exDir, "partition", "partitionName", "");
        bindXMLAttrib(this, el, exDir, "label-x", "@UniqueNameDecor.@InternalPosX", DEFAULT_UNIQUE_NAME_POS.x);
        bindXMLAttrib(this, el, exDir, "label-y", "@UniqueNameDecor.@InternalPosY", DEFAULT_UNIQUE_NAME_POS.y);
        bindXMLAttrib(this, el, exDir, "partition-x", "partitionNameLabel.@InternalPosX", DEFAULT_KRONECKER_PART_POS.x);
        bindXMLAttrib(this, el, exDir, "partition-y", "partitionNameLabel.@InternalPosY", DEFAULT_KRONECKER_PART_POS.y);
    }
}
