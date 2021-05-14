/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import editor.Main;
import editor.domain.measures.AllMeasuresResult;
import editor.domain.play.ActivityState;
import editor.domain.composition.MultiNetPage;
import java.awt.BasicStroke;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.AffineTransform;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import editor.domain.semiflows.PTFlows;

//-------------------------------------------------------------------------

// Helper class passed to nodes and edges to assist the drawing
public class DrawHelper 
{

    public double zoom;
    public float unitSize;
    //        public Font unitSansFont;
    //        public Stroke baseStroke;
    //        public Stroke selectionStroke;
    public AffineTransform logic2screen;
    public AffineTransform baseTransform;
    public float pixelSize;
    
    // Basic line unit
    public static float getBaseStrokeWidth() {
        return  1.0f / NetObject.getUnitToPixels() * Main.getUiSize().getScaleMultiplier();
    }

    public static enum StrokeWidth {
        BASIC(1.0f, "Normal"), THICK(2.0f,"Thick"), VERY_THICK(4.0f, "Very thick");
        float widthMult;
        String name;

        public final Stroke logicStroke;
        public final Stroke logicStrokeWider;
        public final Stroke logicStrokeDashed;
        public final Stroke logicSelectStrokeEdges;
        public final Stroke logicSelectStrokeNodes;
        
        public float getWidth() { return widthMult * getBaseStrokeWidth(); }

        private StrokeWidth(float widthMult, String name) {
            this.widthMult = widthMult;
            this.name = name;
             
            float width = getWidth();
            logicStroke = new BasicStroke(width, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, 2f);
            logicStrokeWider = new BasicStroke(width*2, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, 2f);
            logicStrokeDashed = new BasicStroke(width, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, 2f, new float[]{0.4f, 0.3f}, 0.0f);
            float selWidth = ((widthMult-1) * getBaseStrokeWidth()) + NetObject.SELECTION_STROKE_WIDTH;
            logicSelectStrokeEdges = new BasicStroke(2 * selWidth, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND);
            logicSelectStrokeNodes = new BasicStroke(selWidth, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND);        
       }      
    }
//    public static final Stroke logicStroke;
//    public static final Stroke logicStrokeDashed;
//    public static final Stroke logicSelectStrokeEdges;
//    public static final Stroke logicSelectStrokeNodes;
    //        public static final String FONT_NAME = "Times New Roman";
    // Current state of the net in animation mode (play mode)
    public ActivityState activity;
    // Currently visualized flow (flow panel)
    public PTFlows selectedPTFlow;
    // Currently visualized measures
    public AllMeasuresResult measures;
    // What is shown in this drawing
    public ViewProfile viewProfile;
    // The multi-net (if any) that is being drawn
    public MultiNetPage multiNet;
    
    public DrawHelper(Graphics2D g, ViewProfile vp, double zoom, 
                      double pageOffsetX, double pageOffsetY) 
    {
        this.zoom = zoom;
        this.viewProfile = vp;
        unitSize = (float) logicToScreenFract(1.0);
        //            unitSansFont = new Font(FONT_NAME, Font.PLAIN, (int)Math.round(unitSize));
        //            baseStroke = new BasicStroke(unitSize * baseStrokeWidth);
        //            selectionStroke = new BasicStroke(unitSize * 0.50f, BasicStroke.CAP_ROUND,
        //                                              BasicStroke.JOIN_ROUND);
        double scaleFactor = unitSize;
        baseTransform = g.getTransform();
        logic2screen = new AffineTransform(g.getTransform());
        logic2screen.translate(.5, .5);
        logic2screen.scale(scaleFactor, scaleFactor);
        // Append page translation
        baseTransform.translate(logicToScreenFract(pageOffsetX), logicToScreenFract(pageOffsetY));
        logic2screen.translate(pageOffsetX, pageOffsetY);
        pixelSize = 1.0f / unitSize;
    }
    private static final Line2D.Double paLine = new Line2D.Double();

    public void drawLogicLine(Graphics2D g, Point2D p1, Point2D p2) {
        paLine.setLine(p1.getX(), p1.getY(), p2.getX(), p2.getY());
        g.draw(paLine);
    }
    private static final Ellipse2D.Double logicPointEllipse = new Ellipse2D.Double();

    public final void drawLogicPoint(Graphics2D g, double cx, double cy, double radius) {
        logicPointEllipse.setFrame(cx - radius, cy - radius, 2 * radius, 2 * radius);
        g.fill(logicPointEllipse);
    }

    public final double logicToScreenFract(double s) {
        return NetObject.logicToScreenFract(s, zoom);
    }

    public final int logicToScreen(double s) {
        return NetObject.logicToScreen(s, zoom);
    }

    public final double screenToLogic(int l) {
        return NetObject.screenToLogic(l, zoom);
    }
    //        public final Line2D logicToScreen(Line2D logicLine, Line2D screenLine) {
    //            screenLine.setLine(logicToScreenFract(logicLine.getX1()) + 0.5,
    //                               logicToScreenFract(logicLine.getY1()) + 0.5,
    //                               logicToScreenFract(logicLine.getX2()) + 0.5,
    //                               logicToScreenFract(logicLine.getY2()) + 0.5);
    //            return screenLine;
    //        }
    //
    //        public final CubicCurve2D logicToScreen(CubicCurve2D logicCurve, CubicCurve2D screenCurve) {
    //            screenCurve.setCurve(logicToScreenFract(logicCurve.getX1()) + 0.5,
    //                                 logicToScreenFract(logicCurve.getY1()) + 0.5,
    //                                 logicToScreenFract(logicCurve.getCtrlX1()) + 0.5,
    //                                 logicToScreenFract(logicCurve.getCtrlY1()) + 0.5,
    //                                 logicToScreenFract(logicCurve.getCtrlX2()) + 0.5,
    //                                 logicToScreenFract(logicCurve.getCtrlY2()) + 0.5,
    //                                 logicToScreenFract(logicCurve.getX2()) + 0.5,
    //                                 logicToScreenFract(logicCurve.getY2()) + 0.5);
    //            return screenCurve;
    //        }
    
    public boolean shouldPaintSelection() {
        return (multiNet == null);
    }
}
