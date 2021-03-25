/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import static editor.domain.DraggableHandle.HEAD_HANDLE_COLOR;
import static editor.domain.DraggableHandle.MIDPOINT_HANDLE_COLOR;
import static editor.domain.DraggableHandle.TAIL_HANDLE_COLOR;
import static editor.domain.NetObject.ACTIVE_COLOR;
import static editor.domain.NetObject.ALPHA_50;
import static editor.domain.NetObject.SELECTION_COLOR;
import static editor.domain.NetObject.linearInterp;
import editor.domain.elements.GspnEdge;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.ParserContext;
import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.io.XmlExchangeable;
import editor.domain.play.ActivityState;
import editor.domain.semiflows.SemiFlows;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.geom.CubicCurve2D;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Path2D;
import java.awt.geom.PathIterator;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Map;
import java.util.UUID;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/** An edge in a graph connecting two nodes.
 *
 * @author Elvio
 */
public abstract class Edge extends SelectableObject 
        implements Serializable, DecorHolder, PlaceableObject, ResourceHolder, XmlExchangeable
{
    
    public enum ArrowType {
        SIMPLE,
        TRIANGLE,
        EMPTY_CIRCLE
    }
    public enum EndPoint {
        HEAD, TAIL
    }
    public enum LineType {
        SOLID, DASHED
    }
    final double CIRCLE_ARROW_RADIUS = 0.25;

    // Endpoints of this edge - The arrow is on the head
    private Node tailNode = null, headNode = null;
    // Magnet index where the edge tail(head) is attached to.
    private int tailMagnet = -1, headMagnet = -1;
    // List of points. Note that it requires at least two points. The first and 
    // the last are ignored if tailNode and headNode are not-null.
    public ArrayList<Point2D> points;
    // Point selection when in single edge selection mode. null in any other case
    public boolean[] selPoints = null;
    // Is this edge drawn as a broken edge?
    public boolean isBroken = false;
    
    // Type of arrow
    public abstract ArrowType getArrowType();
    public abstract LineType getLineType();
    public abstract boolean canConnectTo(Node node, EndPoint endPt);
    public abstract boolean canBeBroken();
    
    
    //---- Precomputed edge representation ----------------
    public transient double tailK = -1, headK = -1;
    transient ArrayList<Double> edgeK;
    transient Path2D edgePath;
    transient Path2D arrowPath;
    transient boolean arrowIsFilled = false;
    transient Point2D tailBrokenPt = null;
    transient Point2D headBrokenPt = null;
    public transient double tailBrokenK = -1, headBrokenK = -1;

    public Edge(Node tailNode, int tailMagnet, Node headNode, int headMagnet, 
                ArrayList<Point2D> points, boolean isBroken) 
    {
        this.tailNode = tailNode;
        this.tailMagnet = tailMagnet;
        this.headNode = headNode;
        this.headMagnet = headMagnet;
        this.points = points;
        this.isBroken = isBroken;
    }
    
    public Edge() { 
        this.points = new ArrayList<>();
    }
    
    public int numPoints() { return points.size(); }
    
    // Get the effective control point of this edge. 
    // The first and last points, conneted to the tail/head nodes, can be
    // modified by the edgeAttachmentModel of the node.
    // Intermediate points are unaffected.
    private static final Point2D.Double gpP1 = new Point2D.Double();
    private static final Point2D.Double gpbeforeLast = new Point2D.Double();
    public void getPoint(Point2D.Double pt, int pos) {
        if (pos == 0 && tailNode != null) {
            if (tailMagnet >= 0 && tailMagnet < tailNode.getNumMagnets()) {
                // Attach to a node-specific magnet position
                tailNode.getMagnetPosition(pt, tailMagnet);
            }
            else if (points.size() >= 2 && tailNode.getEdgeAttachmentModel().useOrthogonalEdgeAttachment()) {
                // Attach trying to preserve orthogonality of the first edge segment
                gpP1.setLocation(points.get(1));
                if (points.size() == 2 && headNode != null) // point 1 is actually the arrow head
                    gpP1.setLocation(headNode.getCenterX(), headNode.getCenterY());
                tailNode.getNearsetPointOrtho(pt, gpP1, false);
            }
            else { // Attach to the node center
                pt.setLocation(tailNode.getCenterX(), tailNode.getCenterY());
            }
            return;
        }
        else if (pos == points.size()-1 && headNode != null) {
            if (headMagnet >= 0 && headMagnet < headNode.getNumMagnets()) {
                headNode.getMagnetPosition(pt, headMagnet);
            }
            else if (points.size() >= 2 && headNode.getEdgeAttachmentModel().useOrthogonalEdgeAttachment()) {
                gpbeforeLast.setLocation(points.get(points.size()-2));
                if (points.size() == 2 && tailNode != null) // point size()-2 is actually the tail node
                    getPoint(gpbeforeLast, 0);
                headNode.getNearsetPointOrtho(pt, gpbeforeLast, false);
            }
            else { // Get node center
                pt.setLocation(headNode.getCenterX(), headNode.getCenterY());
            }
            return;
        }
        pt.setLocation(points.get(pos));
    }
    
    public Node getHeadNode() { return headNode; }
    public Node getTailNode() { return tailNode; }
    
    public int getTailMagnet() { return tailMagnet; }
    public int getHeadMagnet() { return headMagnet; }
    
    
    public void checkEdgeCorrectness(NetPage page, ParserContext context) {
        if (getTailNode() == null || getHeadNode() == null) {
            // The edge is not fully connected.
            page.addPageError("Edge endpoints are not connected to nodes.", this);
        }
        if (getTailNode() != null) {
            if (getTailNode() == getHeadNode() && numPoints() == 2) {
                page.addPageWarning("A self-loop edge without "+
                                    "intermediate points is invisible.", this);
            }
        }
    }
    
    // Apply a rewriting rule to this edge
    public abstract void rewriteEdge(ParserContext context, ExprRewriter rewriter);
    
    @Override
    public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable) { /* get the resources */ }
    @Override
    public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable) { /* get the resources */ }
    
    // The maximum coordinate in the linear space of this edge
    private double getMaxK() { return points.size() - 1; }
    
    private static final Point2D.Double cp1 = new Point2D.Double();
    private static final Point2D.Double cp2 = new Point2D.Double();
    
    
    // Get the edge point in the linear space [0, MaxK], 
    // without considering round edges. Use getPointAlongTheLine() to have
    // the effective edge points.
    private Point2D getInterpPoint(Point2D out, double k) {
        assert k >= 0.0 && k <= getMaxK();
        getPoint(cp1, (int)Math.floor(k));
        getPoint(cp2, (int)Math.ceil(k));
        linearInterp(cp1, cp2, out, k - Math.floor(k));
        return out;
    }
    
    public double getWidth() {
        prepareEffectiveEdgePath();
        return edgePath.getBounds2D().getWidth();
    }
    public double getHeight() {
        prepareEffectiveEdgePath();
        return edgePath.getBounds2D().getHeight();
    }
    public double getX() {
        prepareEffectiveEdgePath();
        return edgePath.getBounds2D().getX();
    }
    public double getY() {
        prepareEffectiveEdgePath();
        return edgePath.getBounds2D().getY();
    }
    
    // Used in play mode to test for active/firable nodes
    public abstract double getActivityValue(ActivityState activity);
    
    @Override public boolean isLocked() { return false; }
    
    //-------------------------------------------------------
    // Utilities for self-loop arcs
    
    // Rotate point pt around a circle of given center by theta radians
    private static void rotatePointAround(Point2D.Double out, Point2D.Double pt, 
                                           Point2D.Double center, double theta) 
    {
        double cosTheta = Math.cos(theta);
        double sinTheta = Math.sin(theta);
        out.x = center.x + (pt.x - center.x)*cosTheta - (pt.y - center.y)*sinTheta;
        out.y = center.y + (pt.x - center.x)*sinTheta + (pt.y - center.y)*cosTheta;
    }
    
    // Given a circle with the specified center, and a point @pt on the circle,
    // scan the rotation interval [theta1 theta2] to find a point on the perimeter of node.
    private static final Point2D.Double ftoPt = new Point2D.Double();
    private static double findThetaOutside(Point2D.Double pt, Point2D.Double center, 
                                            double theta1, double theta2, Node node) 
    {
        double thetaMid = (theta1 + theta2) / 2;
        final double MAX_PRECISION = 0.0001;
        
        if (Math.abs(theta1 - thetaMid) < MAX_PRECISION)
            return thetaMid;
        
        rotatePointAround(ftoPt, pt, center, thetaMid);
        boolean inside = node.isCircleInside(ftoPt, 0.0);

        if (inside)
            return findThetaOutside(pt, center, thetaMid, theta2, node);
        else
            return findThetaOutside(pt, center, theta1, thetaMid, node);
    }
    
    // Generate a cubic bezier arc that approximates a circumference arc from P1 to P4
    // with center C. The bezier arc is added to the specified path.
    private static void circumferenceArcTo(Path2D path, Point2D.Double P1, 
                                            Point2D.Double P4, Point2D.Double C) 
    {
        double ax = P1.x - C.x;
        double ay = P1.y - C.y;
        double bx = P4.x - C.x;
        double by = P4.y - C.y;
        double q1 = ax*ax + ay*ay;
        double q2 = q1 + ax*bx + ay*by;
        double k2 = (4.0/3.0) * (Math.sqrt(2*q1*q2) - q2) / (ax*by - ay*bx);
        
        double P2x = P1.x - k2*ay;
        double P2y = P1.y + k2*ax;
        double P3x = P4.x + k2*by;
        double P3y = P4.y - k2*bx;
        
        path.curveTo(P2x, P2y, P3x, P3y, P4.x, P4.y);
    }
    
    // Decide if we should draw this edge using the special code for self-loops
    // It will appear as a circumference arc on perimeter of the the connected node.
    public final boolean isDrawnAsSelfLoopEdge() {
        return (isSelfLoop() && points.size() == 3 &&
                headMagnet == -1 && tailMagnet == -1);
    }
    
    // Is a self loop (connecting the same node) ?
    public final boolean isSelfLoop() {
        return headNode != null && headNode == tailNode;
    }
    
    // Does this edge have intermediate points that can be cleared without 
    // compromising the edge visibility?
    public final boolean mayClearEdgePoints() {
        return points.size() > (isSelfLoop() ? 3 : 2);
    }

    //-------------------------------------------------------
    // Precomputed edge path management.
    
    public final void invalidateEffectiveEdgePath() {
        edgeK = null;
        edgePath = null;
        arrowPath = null;
    }
    public final void invalidateIfConnectedTo(Node node) {
        if (node == tailNode || node == headNode)
            invalidateEffectiveEdgePath();
    }
        
    private static final Point2D.Double slp1 = new Point2D.Double();
    private static final Point2D.Double slpPerim = new Point2D.Double();
    private static final Point2D.Double slpCenter = new Point2D.Double();
    private static final Point2D.Double slpA = new Point2D.Double();
    private static final Point2D.Double slpB = new Point2D.Double();
    private static final Point2D.Double slpC = new Point2D.Double();
    private static final Point2D.Double slpD = new Point2D.Double();
    private static final Point2D.Double slpE = new Point2D.Double();
    private static final Point2D.Double eep1 = new Point2D.Double();
    private static final Point2D.Double eep2 = new Point2D.Double();
    private static final CubicCurve2D.Double  eeCurve = new CubicCurve2D.Double();
    private static final double BROKEN_EDGE_LENGTH = 2.0;
    public final void prepareEffectiveEdgePath() {
        assert points.size() >= 2;
        if (edgeK != null)
            return; // edge path data is valid, no need to regenerate
        
        edgeK = new ArrayList<>();
        edgePath = new Path2D.Double();
        arrowPath = new Path2D.Double();
        
        // Find the starting and ending K
        tailK = 0.0;
        headK = getMaxK();
        if (tailNode != null)
            tailK = getCoeffOutside(0.0, getMaxK(), tailNode, tailMagnet, 0.0);
        if (headNode != null) {
            double arrowRadius = 0.0;
            if (getArrowType() == ArrowType.EMPTY_CIRCLE) 
                arrowRadius = CIRCLE_ARROW_RADIUS + DrawHelper.getBaseStrokeWidth()/2;
            headK = getCoeffOutside(getMaxK(), 0.0, headNode, headMagnet, arrowRadius);
        }
        
        // generate the interpolated curve in the range (tailK, headK)
        double curveK = 0.0;
        int iStart = (int)Math.floor(tailK), iEnd = (int)Math.ceil(headK);
        boolean firstSegment = true;
        if (isDrawnAsSelfLoopEdge()) {
            final double CIRCUMFERENCE_OFFSET = 0.25; // how much we enter inside the node.
            // The circle arc is generated with these rules:
            // The 2D path goes through these points:
            //  * A = first point on the node perimeter, on the tail side
            //  * B = first quarter of the circumference
            //  * C = point[1] = half of the circumference
            //  * D = third quarter of the circumference
            //  * E = last point on the node perimeter, where the arrow points.
            // slp1 is the ideal point that would close the circumference, if 
            // connected to the path, while slpCenter is the center.
            slpC.setLocation(points.get(1));
            headNode.getNearsetPointOrtho(slpPerim, slpC, true);
            double distToNode = slpPerim.distance(slpC);
            double radius = (distToNode + CIRCUMFERENCE_OFFSET) / 2.0; 
            double radiusRatio = radius / distToNode;
            slp1.setLocation(points.get(1).getX() + 2*radiusRatio * (slpPerim.x - slpC.x),
                             points.get(1).getY() + 2*radiusRatio * (slpPerim.y - slpC.y));
            slpCenter.setLocation((slpC.x + slp1.x) / 2.0,
                                  (slpC.y + slp1.y) / 2.0);
            rotatePointAround(slpB, slp1, slpCenter, Math.PI / 2);
            rotatePointAround(slpD, slp1, slpCenter, -Math.PI / 2);
            double thetaA = findThetaOutside(slp1, slpCenter, 0, Math.PI / 2, headNode);
            rotatePointAround(slpA, slp1, slpCenter, thetaA);
            double thetaE = findThetaOutside(slp1, slpCenter, 0, -Math.PI / 2, headNode);
            rotatePointAround(slpE, slp1, slpCenter, thetaE);
            
            edgePath.moveTo(slpA.x, slpA.y);
            circumferenceArcTo(edgePath, slpA, slpB, slpCenter);
            circumferenceArcTo(edgePath, slpB, slpC, slpCenter);
            circumferenceArcTo(edgePath, slpC, slpD, slpCenter);
            circumferenceArcTo(edgePath, slpD, slpE, slpCenter);
            edgeK.add(0.5);
            edgeK.add(0.5);
            edgeK.add(0.5);
            edgeK.add(0.5);
            headK = 2.0;
            tailK = 0.0;
        }
        else { // Draw the segments with rounded corners
            for (int i=iStart; i<iEnd; i++) {
                boolean lastSegment = (i==iEnd-1);
                double segmK = 1.0;

                // Get the segment
                getInterpPoint(eep1, Math.max((double)i, tailK));
                getInterpPoint(eep2, Math.min((double)i + 1, headK));

                // kMax is how much we can consume, in terms of k, of the current segment, 
                // to add the arc.
                // For intermediate segments, it is half the segment (0.5). For the end segment,
                // it is the remaining k before reaching the node (could be less than 0.5).
                double kMax = 0.5;
                if (lastSegment && headK != Math.floor(headK))
                    kMax = Math.min(kMax, 0.9*(headK - Math.floor(headK)));

                if (firstSegment) {
                    edgePath.moveTo(eep1.x, eep1.y);
                    segmK -= tailK;
                }
                else {
                    double k = shortenSegment2(eep1, eep2, eeCurve, true, kMax);
                    segmK -= k;
                    curveK += k;
                    edgePath.curveTo(eeCurve.ctrlx1, eeCurve.ctrly1, eeCurve.ctrlx2, 
                                     eeCurve.ctrly2, eeCurve.x2, eeCurve.y2);
                    edgeK.add(curveK);
                }

                if (!lastSegment) {
                    curveK = shortenSegment2(eep1, eep2, eeCurve, false, kMax);
                    segmK -= curveK;
                }
                else { 
                    segmK -= (iEnd - headK);
                }

                // add the segment
                edgePath.lineTo(eep2.x, eep2.y);
                edgeK.add(segmK);
                firstSegment = false;
            }
        }
        prepareArrowPath();
        
        // Generate the endpoints for broken edges
        if (isBroken) {
            double dx, dy, coeff;
            if (tailBrokenPt == null)
                tailBrokenPt = new Point2D.Double();
            getPointAlongTheLine(eep1, tailK);
            getPointAlongTheLine(tailBrokenPt, tailK + 0.1);
            dx = eep1.x - tailBrokenPt.getX();
            dy = eep1.y - tailBrokenPt.getY();
            coeff = -Math.sqrt(dx*dx + dy*dy) / BROKEN_EDGE_LENGTH;
            tailBrokenPt.setLocation(eep1.x + dx/coeff, eep1.y + dy/coeff);
            tailBrokenK = getNearestK_Unbroken(tailBrokenPt);

            if (headBrokenPt == null)
                headBrokenPt = new Point2D.Double();
            getPointAlongTheLine(eep1, headK);
            getPointAlongTheLine(headBrokenPt, headK - 0.1);
            dx = eep1.x - headBrokenPt.getX();
            dy = eep1.y - headBrokenPt.getY();
            coeff = -Math.sqrt(dx*dx + dy*dy) / BROKEN_EDGE_LENGTH;
            headBrokenPt.setLocation(eep1.x + dx/coeff, eep1.y + dy/coeff);
            headBrokenK = getNearestK_Unbroken(headBrokenPt);
        }
        else {
            tailBrokenPt = null;
            headBrokenPt = null;
            tailBrokenK = -1;
            headBrokenK = -1;
        }

        
//        System.out.println("edgeK:");
//        double accum = tailK;
//        for (double h : edgeK) {
//            System.out.println("  "+h+" - "+accum);
//            accum += h;
//        }
//        // At this point, accum has the same value of headK
//        System.out.println("  Total: "+accum+" - max="+getMaxK()+" tailK="+tailK+" headK="+headK);
    }
    
    // Find a point that has a distance @dist from the headPt, if possible
    // The function initializes @headPt, and returns the found point @distPt with its K value.
    // There is no guarantee that distance(headPt, distPt) is actually equal to dist.
    private double findK_withDistToHead(Point2D.Double headPt, Point2D.Double distPt, double dist) {
        getPointAlongTheLine(headPt, headK);
        double k1 = Math.max(tailK, Math.floor(headK-0.00001));
        double k2 = headK;
        boolean firstIter = true;
        while (true) {
            final double MAX_PRECISION = 0.01;
            double kMid = (k1 + k2) / 2;
            if (Math.abs(k1 - kMid) < MAX_PRECISION)
                return kMid;
            
            getPointAlongTheLine(distPt, kMid);
            double d = distPt.distance(headPt);
            if (d < dist && firstIter)
                return kMid; // the segment is too short anyway, return kMid
            else if (d < dist)
                k2 = kMid;
            else
                k1 = kMid;
            firstIter = false;
        }
    }
    
    private static final Point2D.Double arrowPt1 = new Point2D.Double();
    private static final Point2D.Double arrowPt2 = new Point2D.Double();
    private static final Point2D.Double arrowHead = new Point2D.Double();
    private static final Point2D.Double arrowLeft = new Point2D.Double();
    private static final Point2D.Double arrowRight = new Point2D.Double();
    private static final Ellipse2D.Double arrowEllipse = new Ellipse2D.Double();
    private void prepareArrowPath() {
        findK_withDistToHead(arrowPt2, arrowPt1, 0.8);
//        double step_K = 0.2;
//        if (isDrawnAsSelfLoopEdge())
//            step_K = 0.4;
//        
//        getPointAlongTheLine(arrowPt2, headK);
//        double k = headK - step_K;
//        do {
//            getPointAlongTheLine(arrowPt1, k);
//            k -= step_K;
//        } while (arrowPt1.equals(arrowPt2) && k > tailK);
        
        double Dx = arrowPt2.getX() - arrowPt1.getX(), Dy = arrowPt2.getY() - arrowPt1.getY();
        arrowHead.setLocation(arrowPt2.getX(), arrowPt2.getY());
        double norm = Math.sqrt(Dx*Dx + Dy*Dy);
        Dx /= norm;
        Dy /= norm;
        arrowIsFilled = false;

        ArrowType arrowType = getArrowType();
        switch (arrowType) {
            case TRIANGLE:
                arrowHead.x -= Dx * 0.04;
                arrowHead.y -= Dy * 0.04;
                // fall through
            case SIMPLE:
                double DMult = (arrowType== ArrowType.SIMPLE ? 0.7 : 0.7);
                // Modify the arrow for double-border edges (like fluid edges)
                if (getPipeWidth() != null) {
                    DMult = 0.95;
                    arrowHead.x += Dx * 0.25;
                    arrowHead.y += Dy * 0.25;
                }
                Dx *= DMult;
                Dy *= DMult;
                arrowLeft.setLocation(arrowHead.x - Dx + 0.33*Dy, arrowHead.y - Dy - 0.33*Dx);
                arrowRight.setLocation(arrowHead.x - Dx - 0.33*Dy, arrowHead.y - Dy + 0.33*Dx);
                
                arrowPath.moveTo(arrowLeft.x, arrowLeft.y);
                arrowPath.lineTo(arrowHead.x, arrowHead.y);
                arrowPath.lineTo(arrowRight.x, arrowRight.y);

                if (arrowType == ArrowType.TRIANGLE) {
                    arrowPath.closePath();
                    arrowIsFilled = true;
                }
                break;
            case EMPTY_CIRCLE:
                arrowEllipse.setFrame(arrowHead.x - CIRCLE_ARROW_RADIUS, 
                                      arrowHead.y - CIRCLE_ARROW_RADIUS,
                                      2*CIRCLE_ARROW_RADIUS, 2*CIRCLE_ARROW_RADIUS);
                arrowPath.append(arrowEllipse, false);
                arrowIsFilled = true;
                break;
        }        
    }
    
    // Get a point along the line in the linear space [0, MaxK]
    public Point2D getPointAlongTheLine(Point2D out, double k) {
        prepareEffectiveEdgePath();
        if (k < tailK)
            k = tailK;
        if (k > headK)
            k = headK;
                
        double visitK = tailK;
        int i=0;
        PathIterator iter = edgePath.getPathIterator(null);
        double lastX=0, lastY=0;
        while (!iter.isDone()) {
            int type = iter.currentSegment(coords);
            if (type == PathIterator.SEG_MOVETO) {
                lastX = coords[0];
                lastY = coords[1];
                iter.next();
                continue;
            }
            boolean lastSegm = (i==edgeK.size() - 1);
            if (k <= visitK + edgeK.get(i) || lastSegm) {
                // Segment where the point lies
                double effectiveK = (k - visitK) / edgeK.get(i);
                final double EPSILON = 1.0e-5;
                if (Double.isNaN(effectiveK))
                    effectiveK = 0.0;
                if (!(effectiveK >= -EPSILON && effectiveK <= 1.0+EPSILON)) {
                    System.out.println("WARN: effectiveK = "+effectiveK+"  lastSegm="+lastSegm);
                }
//                for (int j=0; j<edgeK.size(); j++)
//                    System.out.print("edgeK("+j+")="+edgeK.get(j)+"  ");
//                System.out.println("   k="+k+" effectiveK="+effectiveK+" i="+i);

                //assert effectiveK >= -EPSILON && effectiveK <= 1.0+EPSILON;
                switch (type) {
                    case PathIterator.SEG_LINETO:
                        linearInterp(lastX, lastY, coords[0], coords[1], out, effectiveK);
                        break;
                    case PathIterator.SEG_CUBICTO:
                        bezierInterp(lastX, lastY, coords[0], coords[1], coords[2], 
                                     coords[3], coords[4], coords[5], out, effectiveK);
                        break;
                }
                return out;
            }
            
            // The point is in a successive segment
            switch (type) {
                case PathIterator.SEG_LINETO:
                    lastX = coords[0];
                    lastY = coords[1];
                    break;
                case PathIterator.SEG_CUBICTO:
                    lastX = coords[4];
                    lastY = coords[5];
                    break;
                default:
                    throw new UnsupportedOperationException();
            }
            visitK += edgeK.get(i);
            iter.next();
            i++;
        }
        
        System.out.println("WARN: Invalid K "+k+" tailK="+tailK+" headK="+headK);
        out.setLocation(2, 2);
        return out;
        //throw new RuntimeException("Invalid K "+k+" tailK="+tailK+" headK="+headK);
        
//        while (k > visitK + edgeK.get(i)) {
//            visitK += edgeK.get(i);
//            i++;
//        }
//        Shape shape = effectiveEdge.get(i);
//        double effectiveK = (k - visitK) / edgeK.get(i);
//        assert effectiveK >= 0.0 && effectiveK <= 1.0;
//        if (shape instanceof Line2D.Double) {
//            Line2D.Double l = (Line2D.Double)shape;
//            linearInterp(l.getP1(), l.getP2(), out, effectiveK);
//        }
//        else if (shape instanceof CubicCurve2D.Double) {
//            CubicCurve2D.Double c = (CubicCurve2D.Double)shape;
//            bezierInterp(c.getP1(), c.getCtrlP1(), c.getCtrlP2(), c.getP2(), out, effectiveK);
//        }
        /*double visitK = startK;
        int i=0;
        while (k > visitK + edgeK.get(i)) {
            visitK += edgeK.get(i);
            i++;
        }
        Shape shape = effectiveEdge.get(i);
        double effectiveK = (k - visitK) / edgeK.get(i);
        assert effectiveK >= 0.0 && effectiveK <= 1.0;
        if (shape instanceof Line2D.Double) {
            Line2D.Double l = (Line2D.Double)shape;
            linearInterp(l.getP1(), l.getP2(), out, effectiveK);
        }
        else if (shape instanceof CubicCurve2D.Double) {
            CubicCurve2D.Double c = (CubicCurve2D.Double)shape;
            bezierInterp(c.getP1(), c.getCtrlP1(), c.getCtrlP2(), c.getP2(), out, effectiveK);
        }*/
        //return out;
    }
    
    public double getNearestK(Point2D pt) {
        if (isBroken)
            return getNearestK_Broken(pt);
        else
            return getNearestK_Unbroken(pt);
    }
    
    private static final Point2D gnkPtEnd = new Point2D.Double();
    private static final Point2D gnkTailPt = new Point2D.Double();
    private static final Point2D gnkHeadPt = new Point2D.Double();
    public double getNearestK_Broken(Point2D pt) {
        assert tailBrokenPt != null;
        double outK = -1;
        double dist = -1;
        
        // Find the nearest K in the segments [tailK -> tailBrokenPt] and [headK -> headBrokenPt]
        for (int i=0; i<2; i++) {
            double k0 = 0, k1 = 1;
            getPointAlongTheLine(gnkPtEnd, i==0 ? tailK : headK);
            gnkPt0.setLocation(gnkPtEnd);
            Point2D brokenEnd = (i==0 ? tailBrokenPt : headBrokenPt);
            gnkPt1.setLocation(brokenEnd);
            final int NUM_REFINEMENTS = 10;
            for (int j=0; j<NUM_REFINEMENTS; j++) {
                if (pt.distance(gnkPt0) < pt.distance(gnkPt1)) {
                    k1 = (k0 + k1) / 2;
                    gnkPt1.setLocation(linearInterp(gnkPtEnd.getX(), brokenEnd.getX(), k1),
                                       linearInterp(gnkPtEnd.getY(), brokenEnd.getY(), k1));
                }
                else {
                    k0 = (k0 + k1) / 2;
                    gnkPt0.setLocation(linearInterp(gnkPtEnd.getX(), brokenEnd.getX(), k0),
                                       linearInterp(gnkPtEnd.getY(), brokenEnd.getY(), k0));
                }
            }
            if (i==0) {
                outK = linearInterp(tailK, tailBrokenK, (k0 + k1) / 2);
                dist = pt.distance(gnkPt0);
            }
            else {
                if (pt.distance(gnkPt0) < dist)
                    outK = linearInterp(headK, headBrokenK, (k0 + k1) / 2);
            }
        }
        return outK;
    }
    
    private static final Point2D gnkPt0 = new Point2D.Double();
    private static final Point2D gnkPt1 = new Point2D.Double();
    // Get the K value that results in the point on the edge nearest to @pt.
    private double getNearestK_Unbroken(Point2D pt) {
        double i0, i1=0;
        double nearestK = 0;
        final double STEP = 0.2;
        while (i1 < getMaxK()) {
            i0 = i1;
            i1 += STEP;
            
            // Find the nearest point in the interval
            double k0=i0, k1=i1;
            getPointAlongTheLine(gnkPt0, k0);
            getPointAlongTheLine(gnkPt1, k1);
            final int NUM_REFINEMENTS = 10;
            for (int j=0; j<NUM_REFINEMENTS; j++) {
                if (pt.distance(gnkPt0) < pt.distance(gnkPt1)) {
                    k1 = (k0 + k1) / 2;
                    getPointAlongTheLine(gnkPt1, k1);
                }
                else {
                    k0 = (k0 + k1) / 2;
                    getPointAlongTheLine(gnkPt0, k0);
                }
            }
            // Check if this is the nearest point of the edge
            double localK = (k0 + k1) / 2;
            getPointAlongTheLine(gnkPt0, localK);
            getPointAlongTheLine(gnkPt1, nearestK);
            if (pt.distance(gnkPt0) < pt.distance(gnkPt1))
                nearestK = localK;
        }
        return nearestK;
    }

    
    private double shortenSegment2(Point2D p1, Point2D p2, CubicCurve2D.Double curve, 
                                   boolean first, double kMax) 
    {
        final double curveLen = 0.75;
        double lineLen = p1.distance(p2);
        double Dx = p2.getX() - p1.getX(), Dy = p2.getY() - p1.getY();
        double k = Math.min(0.5, curveLen / lineLen);
        k = Math.min(k, kMax);
        double hkx = k*Dx*0.33, hky = k*Dy*0.33;
        if (first) {
            curve.ctrlx2 = p1.getX() + hkx;
            curve.ctrly2 = p1.getY() + hky;
            p1.setLocation(p1.getX() + k * Dx, p1.getY() + k * Dy);
            curve.x2 = p1.getX();
            curve.y2 = p1.getY();
        }
        else {
            curve.ctrlx1 = p2.getX() - hkx;
            curve.ctrly1 = p2.getY() - hky;
            p2.setLocation(p2.getX() - k * Dx, p2.getY() - k * Dy);
            curve.x1 = p2.getX();
            curve.y1 = p2.getY();            
        }
        return k;
    }
    
    private static final Point2D.Double cpMid = new Point2D.Double();
    private double getCoeffOutside(double kIn, double kOut, Node node, int nodeMagnet, double arrowRadius) {
        if (nodeMagnet >=0  && nodeMagnet < node.getNumMagnets() && arrowRadius == 0) {
            return kIn;
        }
        
//        assert node.isCircleInside(getInterpPoint(cpMid, kIn), arrowRadius * 1.02);
//        assert !node.isCircleInside(getInterpPoint(cpMid, kOut), arrowRadius * 0.98);
        final double MAX_PRECISION = 0.0001;
        
        if (Math.abs(kIn - kOut) < MAX_PRECISION)
            return kIn;

        double kMid = (kIn + kOut) / 2;
        getInterpPoint(cpMid, kMid);
        boolean inside = node.isCircleInside(cpMid, arrowRadius);
//        System.out.println("getCoeffOutside kIn="+kIn+" kOut="+kOut+
//                           " nodeMagnet="+nodeMagnet+" arrowRadius="+arrowRadius+
//                           "  getMaxK="+getMaxK()+"  kMid="+kMid+" inside="+inside);
        if (inside)
            return getCoeffOutside(kMid, kOut, node, -1, arrowRadius);
        else
            return getCoeffOutside(kIn, kMid, node, -1, arrowRadius);
    }
    
    
    private static final Point2D.Double aelPt0 = new Point2D.Double();
    private static final Point2D.Double aelPt1 = new Point2D.Double();
    public double getApproxEdgeLength() {
        prepareEffectiveEdgePath();
        // compute an approximate of the edge length
        if (isBroken)
            return 2 * BROKEN_EDGE_LENGTH;
        
        double len = 0;
        for (int i=1; i<numPoints(); i++) {
            getPoint(aelPt0, i - 1);
            getPoint(aelPt1, i);
            len += aelPt0.distance(aelPt1);
        }
        return len;
    }
    
    public DrawHelper.StrokeWidth getEdgeWidth() { return DrawHelper.StrokeWidth.BASIC; }
    public DrawHelper.StrokeWidth getPipeWidth() { return null; }
    public DrawHelper.StrokeWidth getArrowWidth() { return DrawHelper.StrokeWidth.BASIC; }

    // Minimum line length between this edge and a decor to start drawing the connecting line
    // between the edge and the decor itself. Lines os smaller length are not drawn.
    public double getMinLineToDecorLength() { return 0.5; }

    private static final Point2D.Double paDecorPt = new Point2D.Double();
    private static final Point2D.Double paLinePt = new Point2D.Double();
    private static final Rectangle2D.Double paRect = new Rectangle2D.Double();
    private static final Point2D.Double paTailPt = new Point2D.Double();
    private static final Point2D.Double paHeadPt = new Point2D.Double();
    public void paintEdge(Graphics2D g, DrawHelper dh) {
        prepareEffectiveEdgePath();
        Stroke oldStroke = g.getStroke();
        DrawHelper.StrokeWidth sw = getEdgeWidth();
        DrawHelper.StrokeWidth pipe = getPipeWidth();
        DrawHelper.StrokeWidth arrow = getArrowWidth();
        
        if (isBroken) {
            getPointAlongTheLine(paTailPt, tailK);
            getPointAlongTheLine(paHeadPt, headK);
        }

        // Draw selection aura
        if (isSelected() && dh.shouldPaintSelection()) {
            Composite oldComp = g.getComposite();
            g.setComposite(ALPHA_50);
            g.setStroke(sw.logicSelectStrokeEdges);
            g.setColor(SELECTION_COLOR);
            if (isBroken) {
                dh.drawLogicLine(g, paTailPt, tailBrokenPt);
                dh.drawLogicLine(g, paHeadPt, headBrokenPt);
            }
            else
                g.draw(edgePath);
            g.draw(arrowPath);
            g.setComposite(oldComp);
        }
                
        // Active/fireable aura
        if (dh.activity != null) {
            double activity = getActivityValue(dh.activity);
            if (activity > 0) {
                Stroke sk = g.getStroke();
                Color clr = g.getColor();
                Composite comp = g.getComposite();
                g.setColor(ACTIVE_COLOR);
                g.setComposite(ALPHA_25);
                for (int i=0; i<NUM_ACTIVITY_AURA_STEPS; i++) {
                    double a = activity * ((i+1.0)/NUM_ACTIVITY_AURA_STEPS);
                    g.setStroke(new BasicStroke((float)(2 * a)));
                    if (isBroken) {
                        dh.drawLogicLine(g, paTailPt, tailBrokenPt);
                        dh.drawLogicLine(g, paHeadPt, headBrokenPt);
                    }
                    else
                        g.draw(edgePath);
                }        
                g.setStroke(sk); 
                g.setColor(clr);
                g.setComposite(comp);
            }
        }
        
        // Highlighted edge (semiflow visualization)
        if (dh.semiflows != null && dh.semiflows.contains(this)) {
            Stroke sk = g.getStroke();
            Color clr = g.getColor();
            Composite comp = g.getComposite();
//            int card;
//            if (dh.semiflows.getType() == SemiFlows.Type.PLACE_FLOW || 
//                dh.semiflows.getType() == SemiFlows.Type.PLACE_SEMIFLOW)
//                card = dh.semiflows.getNodeCardinality(((GspnEdge)this).getConnectedPlace());
//            else
//                card = dh.semiflows.getNodeCardinality(((GspnEdge)this).getConnectedTransition());
            g.setColor(dh.semiflows.getLineColor(1));
            g.setComposite(ALPHA_75);
            int phase = NUM_HIGHLIGHT_PHASES - (dh.semiflows.getDashPhase() % NUM_HIGHLIGHT_PHASES);
            g.setStroke(new BasicStroke((float)(2 * DEFAULT_ACTIVITY_AURA_SIZE), BasicStroke.CAP_SQUARE,
                                        BasicStroke.JOIN_ROUND, 10f, HIGHLIGHT_DASHES, 
                                        HIGHLIGHT_DASH_PHASE_MULT * phase));
            if (isBroken) {
                dh.drawLogicLine(g, paTailPt, tailBrokenPt);
                dh.drawLogicLine(g, headBrokenPt, paHeadPt);
            }
            else
                g.draw(edgePath);
            g.setStroke(sk); 
            g.setColor(clr);
            g.setComposite(comp);
        }
        
        // Determine the edge color
        Color edgeColor = Color.BLACK;
        if (isGrayed())
            edgeColor = GRAYED_BLACK;
        g.setColor(edgeColor);

        // Paint the line connecting the edge with the decors
        for (int d=0; d<getNumDecors(); d++) {
            Decor decor = getDecor(d);
            if (!decor.isVisible(dh.viewProfile))
                continue;
            if (decor.drawLineBetweenDecorAndEdge()) {
                getLineToDecor(decor, paLinePt, paDecorPt);
                if (paLinePt.distance(paDecorPt) > getMinLineToDecorLength()) {
                    g.setStroke(DrawHelper.StrokeWidth.BASIC.logicStroke);
                    dh.drawLogicLine(g, paLinePt, paDecorPt);
                    g.setStroke(oldStroke);
                }
            }            
        }

        // Draw the edge line
        switch (getLineType()) {
            case SOLID:     g.setStroke(sw.logicStroke);        break;
            case DASHED:    g.setStroke(sw.logicStrokeDashed);  break;
        }
        if (isBroken) {
            dh.drawLogicLine(g, paTailPt, tailBrokenPt);
            dh.drawLogicLine(g, paHeadPt, headBrokenPt);
            if (isSelected()) {
                Composite oldComp = g.getComposite();
                g.setComposite(ALPHA_25);
                g.draw(edgePath);
                g.setComposite(oldComp);
            }
        }
        else
            g.draw(edgePath);
        
        // Draw internal edge lines (pipe edges)
        if (pipe != null) {
            Color pipeColor = Color.WHITE;
            g.setColor(pipeColor);
            g.setStroke(pipe.logicStroke);
            paintEdgePipe(g, dh);
        }
        
        // Draw the arrow
        g.setColor(edgeColor);
        g.setStroke(arrow.logicStroke);
        if (arrowIsFilled) {
            g.setColor(Color.WHITE);
            g.fill(arrowPath);
        }
        g.setColor(edgeColor);
        g.draw(arrowPath);
        
//        if (pipe != null) {
//            Color pipeColor = Color.WHITE;
//            g.setStroke(pipe.logicStroke);
//            if (arrowIsFilled) {
//                g.setColor(Color.WHITE);
//                g.fill(arrowPath);
//            }
//            g.setColor(pipeColor);
//            g.draw(arrowPath);
//        }
//        g.setColor(edgeColor);
  
        g.setStroke(oldStroke);
        
        // Draw the node labels (broken edge only)
        if (isBroken) {
            if (tailNode != null) {
                LabelDecor tailNodeName = tailNode.getUniqueNameDecor();
                Point2D labelAttachPt = getNameLabelPoint(tailNodeName, paHeadPt, headBrokenPt);
                tailNodeName.paintDecorAt(g, dh, labelAttachPt.getX(), 
                                          labelAttachPt.getY(), isGrayed(), false);
            }
            if (headNode != null) {
                LabelDecor headNodeName = headNode.getUniqueNameDecor();
                Point2D labelAttachPt = getNameLabelPoint(headNodeName, paTailPt, tailBrokenPt);
                headNodeName.paintDecorAt(g, dh, labelAttachPt.getX(), 
                                          labelAttachPt.getY(), isGrayed(), false);
            }
        }
        
        // Custom edge drawings
        paintEdgeDrawings(g, dh);
        
        // Paint the decors
        for (int d=0; d<getNumDecors(); d++) {
            Decor decor = getDecor(d);
            if (!decor.isVisible(dh.viewProfile))
                continue;
//            if (decor.drawLineBetweenDecorAndEdge()) {
//                getLineToDecor(decor, paLinePt, paDecorPt);
//                g.setStroke(DrawHelper.StrokeWidth.BASIC.logicStroke);
//                dh.drawLogicLine(g, paLinePt, paDecorPt);
//                g.setStroke(oldStroke);
//            }            
            decor.paintDecor(g, dh, isGrayed(), false);
        }
        
//        final int NUMPT = 40;
//        for (int i=0; i<NUMPT+1; i++) {
//            int c = (int)((i / (double)NUMPT) * 255);
//            g.setColor(new Color(c, 0, 255-c));
//            double k = (i / (double)NUMPT) * getMaxK();
//            Point2D.Double p = new Point2D.Double();
//            getPointAlongTheLine(p, k);
//            dh.drawLogicPoint(g, p.x, p.y, 0.07);
//        }
//        g.setColor(Color.black);
    }
    
    public abstract void paintEdgeDrawings(Graphics2D g, DrawHelper dh);
    
    protected void paintEdgePipe(Graphics2D g, DrawHelper dh) {
        if (isBroken) {
            dh.drawLogicLine(g, paTailPt, tailBrokenPt);
            dh.drawLogicLine(g, paHeadPt, headBrokenPt);
        }
        else
            g.draw(edgePath);
    }
    
    private static final Point2D.Double gnlpAuxPt = new Point2D.Double();
    private static final Point2D.Double gnlpFarPt = new Point2D.Double();
    private static final Rectangle2D.Double gnlpLabelRect = new Rectangle2D.Double();
    private Point2D getNameLabelPoint(LabelDecor label, Point2D endPt, Point2D brokenEndPt) {
        double dx = brokenEndPt.getX() - endPt.getX();
        double dy = brokenEndPt.getY() - endPt.getY();
        double norm = Math.sqrt(dx*dx + dy*dy);
        dx /= norm;
        dy /= norm;
        double width = label.getWidth() + 0.5;
        double height = label.getHeight() + 0.5;
        double farDist = (width + height) * 1.5;
        gnlpFarPt.setLocation(brokenEndPt.getX() + farDist * dx,
                              brokenEndPt.getY() + farDist * dy);
        // Find recursively the first attach point for the label that does not contain brokenEndPt
        double k0 = 0, k1 = 1;
        final int NUM_REFINEMENTS = 10;
        for (int i=0; i<NUM_REFINEMENTS; i++) {
            double kmid = (k0 + k1) / 2;
            linearInterp(brokenEndPt, gnlpFarPt, gnlpAuxPt, kmid);
            gnlpLabelRect.setRect(gnlpAuxPt.getX() - width / 2, 
                                  gnlpAuxPt.getY() - height / 2,
                                  width, height);
            if (gnlpLabelRect.contains(brokenEndPt))
                k0 = kmid;
            else
                k1 = kmid;
        }
        gnlpAuxPt.setLocation(gnlpAuxPt.getX() - label.getWidth() / 2, 
                              gnlpAuxPt.getY() - label.getHeight() / 2);
        return gnlpAuxPt;
    }
   
    // Returns the line points that connect the decoration to the edge
    private static final Point2D.Double gltdAuxPt = new Point2D.Double();
    private void getLineToDecor(Decor decor, Point2D linePoint, Point2D decorPoint) {
        getPointAlongTheLine(linePoint, sanitizeK_ForBrokenEdges(decor.getEdgeK()));
        gltdAuxPt.setLocation(linePoint.getX() + decor.getRelativeX() + decor.getWidth()/2,
                               linePoint.getY() + decor.getRelativeY() + decor.getHeight()/2);
        // shorten the segment
        final double FRAME = 0.15;
        paRect.setRect(linePoint.getX() + decor.getRelativeX() - FRAME, 
                       linePoint.getY() + decor.getRelativeY() - FRAME, 
                       decor.getWidth() + 2*FRAME, 
                       decor.getHeight() + 2*FRAME);
        double k1 = 0, k2 = 1;
        final int NUM_REFINEMENTS = 10;
        for (int i=0; i<NUM_REFINEMENTS; i++) {
            double kmid = (k1 + k2) / 2;
            linearInterp(linePoint, gltdAuxPt, decorPoint, kmid);
            if (paRect.contains(decorPoint))
                k2 = kmid;
            else
                k1 = kmid;
        } 
    }
    
    
    
    private static final double[] coords = new double[6];
    private static final Line2D.Double sqTestLine = new Line2D.Double();
    private static final CubicCurve2D.Double sqTestCurve = new CubicCurve2D.Double();
    private static final Point2D.Double irEdgePt = new Point2D.Double();
    private static final Point2D.Double irDecorPt = new Point2D.Double();
    private static final Point2D.Double irTailPt = new Point2D.Double();
    private static final Point2D.Double irHeadPt = new Point2D.Double();
    @Override
    public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                      boolean includeDecors) 
    {
        prepareEffectiveEdgePath();
        
        if (isBroken) {
            // Test intersection with the two broken segments
            getPointAlongTheLine(irTailPt, tailK);
            getPointAlongTheLine(irHeadPt, headK);
            
            if (rect.intersectsLine(tailBrokenPt.getX(), tailBrokenPt.getY(), irTailPt.x, irTailPt.y))
                return true;
            if (rect.intersectsLine(headBrokenPt.getX(), headBrokenPt.getY(), irHeadPt.x, irHeadPt.y))
                return true;
            
            // Test intersection with the node labels
            if (tailNode != null) {
                LabelDecor tailNodeName = tailNode.getUniqueNameDecor();
                Point2D labelAttachPt = getNameLabelPoint(tailNodeName, paHeadPt, headBrokenPt);
                if (tailNodeName.intersectRectangleAt(rect, labelAttachPt.getX(), 
                                          labelAttachPt.getY(), includeDecors))
                    return true;
            }
            if (headNode != null) {
                LabelDecor headNodeName = headNode.getUniqueNameDecor();
                Point2D labelAttachPt = getNameLabelPoint(headNodeName, paTailPt, tailBrokenPt);
                if (headNodeName.intersectRectangleAt(rect, labelAttachPt.getX(), 
                                          labelAttachPt.getY(), includeDecors))
                    return true;
            }
        }
        else {
            PathIterator iter = edgePath.getPathIterator(null);
            double lastX=0, lastY=0;
            while (!iter.isDone()) {
                switch (iter.currentSegment(coords)) {
                    case PathIterator.SEG_MOVETO:
    //                    System.out.println("SEG_MOVETO "+coords[0]+" "+coords[1]);
                        if (rect.contains(coords[0], coords[1]))
                            return true;
                        lastX = coords[0];
                        lastY = coords[1];
                        break;
                    case PathIterator.SEG_LINETO:
    //                    System.out.println("SEG_LINETO "+coords[0]+" "+coords[1]+" "+coords[2]+" "+coords[3]);
                        sqTestLine.setLine(lastX, lastY, coords[0], coords[1]);
                        if (sqTestLine.intersects(rect))
                            return true;
                        lastX = coords[0];
                        lastY = coords[1];
                        break;
                    case PathIterator.SEG_CUBICTO:
    //                    System.out.println("SEG_CUBICTO "+coords[0]+" "+coords[1]+" "+coords[2]+" "+coords[3]+" "+coords[4]+" "+coords[5]);
                        sqTestCurve.setCurve(lastX, lastY, coords[0], coords[1], coords[2], 
                                             coords[3], coords[4], coords[5]);
                        if (sqTestCurve.intersects(rect))
                            return true;
                        lastX = coords[4];
                        lastY = coords[5];
                        break;
                    default:
                        throw new UnsupportedOperationException();
                }
                iter.next();
            }
        }
        if (includeDecors) {
            for (int i=0; i<getNumDecors(); i++) {
                Decor decor = getDecor(i);
                if (!decor.isVisible(viewProfile))
                    continue;
                getLineToDecor(decor, irEdgePt, irDecorPt);
                if (rect.intersectsLine(irEdgePt.x, irEdgePt.y, irDecorPt.x, irDecorPt.y))
                    return true; // intersection with the line between the edge and the decor
                if (decor.intersectRectangle(rect, viewProfile, includeDecors))
                    return true; // intersect with the decor content
            }
        }
        return false;
    }
    
    
    public MovementHandle getPointHandle(final int pos) {
        assert pos >=0 && pos < numPoints();
        return new DraggableHandle() {
            @Override
            public HandlePosition savePosition() {
                return new PointHandlePosition(points.get(pos));
            }
            @Override
            public void restorePosition(HandlePosition hp) {
                PointHandlePosition php = (PointHandlePosition)hp;
                if (!points.get(pos).equals(php.point)) {
                    points.get(pos).setLocation(php.point);
                    invalidateEffectiveEdgePath();
                }
            }
            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                Point2D p = points.get(pos);
                if (p.getX() != x || p.getY() != y) {
                    p.setLocation(x, y);
                    invalidateEffectiveEdgePath();
                }
            }
            @Override public boolean canMoveTo() { return true; }
            @Override public BoxShape getBoxShape() { return null; }
            @Override public EditableCell getEditable() { return null; }
            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { return POINT_GRID; }
        };
    }
    
    static class EndPointHandlePosition extends PointHandlePosition {
        public Node endPointNode;
        public int endPointMagnet;

        public EndPointHandlePosition(Node endPointNode, int endPointMagnet, Point2D point) {
            super(point.getX(), point.getY());
            this.endPointNode = endPointNode;
            this.endPointMagnet = endPointMagnet;
        }
    }
    
    private MovementHandle getEdgePointHandle(final int ptIndex, final NetPage thisPage) {
        return new SelectableHandle() {
            @Override protected Color getHandleColor() { 
                if (ptIndex == 0) return TAIL_HANDLE_COLOR;
                if (ptIndex == numPoints() - 1) return HEAD_HANDLE_COLOR;
                return MIDPOINT_HANDLE_COLOR; 
            }
            @Override protected boolean isEdgeHandle() { return true; }
            @Override
            public HandlePosition savePosition() { 
                if (ptIndex == 0) {
                    if (tailNode == null)
                        return new EndPointHandlePosition(null, -1, points.get(0));
                    else {
                        Point2D position = new Point2D.Double();
                        if (tailMagnet != -1 && tailMagnet < tailNode.getNumMagnets())
                            tailNode.getMagnetPosition(position, tailMagnet);
                        else
                            position = getPointAlongTheLine(position, tailK);
                        return new EndPointHandlePosition(tailNode, -1, position);
                    }
                }
                else if (ptIndex == numPoints() - 1) {
                    if (headNode == null)
                        return new EndPointHandlePosition(null, -1, points.get(ptIndex));
                    else {
                        Point2D position = new Point2D.Double();
                        if (headMagnet != -1 && headMagnet < headNode.getNumMagnets())
                            headNode.getMagnetPosition(position, headMagnet);
                        else
                            position = getPointAlongTheLine(new Point2D.Double(), headK);
                        return new EndPointHandlePosition(headNode, headMagnet, position);
                    }
                }
                return new EndPointHandlePosition(null, -1, points.get(ptIndex));
            }

            private EndPoint getEndPoint() {
                if (ptIndex == 0)  return EndPoint.TAIL;
                if ((ptIndex == numPoints() - 1)) return EndPoint.HEAD;
                return null;
            }

            @Override
            public void restorePosition(HandlePosition hp) {
                EndPointHandlePosition ephp = (EndPointHandlePosition)hp;
                if (!points.get(ptIndex).equals(ephp.point)) {
                    points.get(ptIndex).setLocation(ephp.point);
                    invalidateEffectiveEdgePath();
                }
                EndPoint endPt = getEndPoint();
                if (endPt != null) {
                    //assert canConnectTo(ephp.endPointNode, endPt);
                    connectToNode(ephp.endPointNode, ephp.endPointMagnet, endPt);
                }
            }

            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                EndPoint endPt = getEndPoint();
                if (endPt != null && !isMultiSelMove) {
                    Point2D pointRef = points.get(endPt==EndPoint.TAIL ? 0 : numPoints()-1);
                    pointRef.setLocation(x, y);
                    Node hitNode = thisPage.hitTestNode(NetObject.makeHitpointRectangle(pointRef, thisPage.viewProfile.zoom),
                                                        false /*attach only to nodes, not their decors*/,
                                                        NetPage.HIT_FOREGROUND_NODES,
                                                        false /*precedence to selected*/);
                    int hitMagnet = -1;
                    if (hitNode != null) {
                        hitMagnet = hitNode.findNearestMagnet(pointRef, thisPage.viewProfile.zoom);
                    }
                    if (hitNode != null &&
                        hitNode.canConnectEdges() && 
                        canConnectTo(hitNode, endPt))
                        connectToNode(hitNode, hitMagnet, endPt);
                    else
                        connectToNode(null, -1, endPt);
                }
                else { // Midpoint
                    Point2D p = points.get(ptIndex);
                    if (p.getX() != x || p.getY() != y) {
                        p.setLocation(x, y);
                        invalidateEffectiveEdgePath();
                    }
                }
            }
            @Override public boolean canMoveTo() { return true; }

            @Override
            public Node getNodeWithAura() {
                EndPoint endPt = getEndPoint();
                if (endPt == null)
                    return null;
                return (endPt == EndPoint.TAIL) ? tailNode : headNode;
            }

            @Override
            public int getMagnetWithAura() {
                EndPoint endPt = getEndPoint();
                if (endPt == null)
                    return -1;
                return (endPt == EndPoint.TAIL) ? tailMagnet : headMagnet;
            }
            

            @Override
            public boolean isGrayNodeRuleActive() {
                return getEndPoint() != null;
            }

            @Override
            public boolean isNodeGrayed(Node node) {
                assert getEndPoint() != null;
                return !canConnectTo(node, getEndPoint());
            }
            
            @Override public DraggableHandle.BoxShape getBoxShape() { return BoxShape.DIAMOND; }

            // Selectable interface
            @Override
            public boolean isSelected() { return isSubObjectSelected(ptIndex); }

            @Override
            public void setSelected(boolean isSelected) {
                setSelectedSubObject(ptIndex, isSelected);
            }
            @Override public EditableCell getEditable() { return null; }
            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { 
                // We take the null grid if we are touching a node magnet, Otherwise,
                // we take the usual POINT_GRID.
                EndPoint endPt = getEndPoint();
                if (endPt != null && !isMultiSelMove) {
                    Point2D pointRef = points.get(endPt==EndPoint.TAIL ? 0 : numPoints()-1);
                    Point2D newPt = new Point2D.Double(pointRef.getX() + Dx, pointRef.getY() + Dy);
                    //pointRef.setLocation(x, y);
                    Node hitNode = thisPage.hitTestNode(NetObject.makeHitpointRectangle(newPt, thisPage.viewProfile.zoom),
                                                        false /*attach only to nodes, not their decors*/,
                                                        NetPage.HIT_FOREGROUND_NODES,
                                                        false /*precedence to selected*/);
                    if (hitNode != null) {
                        int hitMagnet = hitNode.findNearestMagnet(newPt, thisPage.viewProfile.zoom);
                        if (hitMagnet != -1)
                            return null;
                    }
                }
                return POINT_GRID; 
            }
        };
    }
    
    class EdgeHandlePosition extends PointHandlePosition {
        public ArrayList<Point2D> points;
        public boolean[] selPoints;
        public Node tailNode, headNode;
        public int tailMagnet, headMagnet;

        public EdgeHandlePosition(ArrayList<Point2D> points, boolean[] selPoints, 
                                  Node tailNode, int tailMagnet, 
                                  Node headNode, int headMagnet, Point2D point) {
            super(point);
            this.points = new ArrayList<>();
            this.points.ensureCapacity(points.size());
            for (int i=0; i<points.size(); i++)
                this.points.add(new Point2D.Double(points.get(i).getX(), points.get(i).getY()));
            if (selPoints != null)
                this.selPoints = (boolean[])selPoints.clone();
            this.tailNode = tailNode;
            this.tailMagnet = tailMagnet;
            this.headNode = headNode;
            this.headMagnet = headMagnet;
        }
        
    }
    
    @Override
    public MovementHandle getCenterHandle(NetPage thisPage) {
        return new ShapeHandle() {
            @Override
            public HandlePosition savePosition() {
                // Note: the ref point should not be the 0 or the last, because they are not
                // moved by the moveBy method.
                int refPointIndex = Math.min(1, numPoints()-1);
                Point2D refPoint = new Point2D.Double(points.get(refPointIndex).getX(), 
                                                      points.get(refPointIndex).getY());
                return new EdgeHandlePosition(points, selPoints, tailNode, tailMagnet, 
                                              headNode, headMagnet, refPoint);
            }

            @Override
            public void restorePosition(HandlePosition hp) {
                EdgeHandlePosition ehp = (EdgeHandlePosition)hp;
                for (int i=0; i<points.size(); i++)
                    points.get(i).setLocation(ehp.points.get(i));
                selPoints = ehp.selPoints;
                tailNode = ehp.tailNode;
                tailMagnet = ehp.tailMagnet;
                headNode = ehp.headNode;
                headMagnet = ehp.headMagnet;
                invalidateEffectiveEdgePath();
            }

            @Override
            public boolean intersectRectangle(Rectangle2D rect, ViewProfile viewProfile,
                                              boolean includeDecors) 
            {
                return Edge.this.intersectRectangle(rect, viewProfile, includeDecors);
            }

            @Override
            public void moveBy(double Dx, double Dy, boolean isMultiSelMove) {
                // Move all the intermediate points
                for (int i=0; i<numPoints(); i++) {
                    if (i == 0 && tailNode != null)
                        continue;
                    if (i == numPoints()-1 && headNode != null)
                        continue;
                    points.get(i).setLocation(points.get(i).getX() + Dx,
                                              points.get(i).getY() + Dy);
                }
                invalidateEffectiveEdgePath();
            }

            @Override
            public void moveTo(double x, double y, boolean isMultiSelMove) {
                throw new UnsupportedOperationException("Not supported for edges.");
            }
            @Override public boolean canMoveTo() { return false; }
            @Override public EditableCell getEditable() { 
                if (Edge.this instanceof EditableCell) 
                    return (EditableCell)Edge.this;
                return null;
            }
            @Override public MeshGridSize getPreferredMeshGridSize(double Dx, double Dy, boolean isMultiSelMove) { return POINT_GRID; }
        };
    }
    
    // Add a new intermediate point to this edge
    public int addNewPoint(Point2D point, EndPoint endPt) {
        points.add(endPt == EndPoint.HEAD ? points.size() : 0, point);
        invalidateEffectiveEdgePath();
        return (endPt == EndPoint.HEAD ? points.size()-1 : 0);
    }
    
    // Change one of the two nodes where this edge is connected
    public void connectToNode(Node node, int magnet, EndPoint endPt) {
        if (endPt == EndPoint.HEAD) {
            headNode = node;
            headMagnet = magnet;
        }
        else {
            tailNode = node;
            tailMagnet = magnet;
        }
        invalidateEffectiveEdgePath();        
    }
    
    // Get all the movement handles of this edge
    @Override
    public MovementHandle[] getSubObjectHandles(NetPage thisPage, ViewProfile viewProfile) {
        ArrayList<MovementHandle> handles = new ArrayList<>();
        // Add decoration handles
        for (int i=0; i<getNumDecors(); i++) {
            Decor decor = getDecor(i);
            if (!decor.isVisible(viewProfile))
                continue;
            MovementHandle attachHandle = decor.getEdgeAttachmentHandle(this);
            if (attachHandle != null)
                handles.add(attachHandle);
            handles.add(decor.getCenterHandle(thisPage));
        }
        // Add edge point handles
        for (int i=0; i<numPoints(); i++) 
            handles.add(getEdgePointHandle(i, thisPage));
        return handles.toArray(new MovementHandle[0]);
    }

    
    
    @Override
    public void setSelected(boolean isSelected) {
        if (!isSelected)
            selPoints = null;
        super.setSelected(isSelected); 
    }
    
    
    // Sub-object selection
    public void setSubObjectSelection(boolean isSel) {
        if (!isSel) {
            selPoints = null;
            return;
        }
        for (int i=0; i<numPoints(); i++)
            setSelectedSubObject(i, true);
    }
    
    public boolean isSubObjectSelected(int ptIndex) {
        if (selPoints != null && selPoints.length == points.size())
            return selPoints[ptIndex];
        return false;
    }

    public void setSelectedSubObject(int ptIndex, boolean isSelected) {
        if (!isSelected && selPoints == null)
            return;
        if (selPoints == null || selPoints.length != points.size())
            selPoints = new boolean[points.size()];
        selPoints[ptIndex] = isSelected;
    }
    
    public abstract class EdgeLabelDecor extends LabelDecor {

        public EdgeLabelDecor(float logicSize, Point2D pos, int horizAlign, int vertAlign) {
            super(logicSize, pos, horizAlign, vertAlign);
        }
        
        @Override
        public void getAttachPoint(Point2D pt, double K) {
            // Decors are attached to a midpoint - obtained by the K coefficient
            getPointAlongTheLine(pt, sanitizeK_ForBrokenEdges(K));
        }
    }
    
    public double sanitizeK_ForBrokenEdges(double K) {
        prepareEffectiveEdgePath();
        if (isBroken) {
            assert tailBrokenK > 0.0;
            if(K >= tailBrokenK && K <= headBrokenK)
                return (headK + headBrokenK) / 2.0;
        }
        return K;
    }
    
    public EditableValue getBrokenFlagEditable() {
        return new EditableValue() {
            @Override public boolean isEditable() { return canBeBroken(); }
            @Override public Object getValue() { return isBroken ? Boolean.TRUE : Boolean.FALSE; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                isBroken = (Boolean)value;
                invalidateEffectiveEdgePath();
            }
            @Override public boolean isCurrentValueValid() { return true; }
        };
    }

    
    private static double safeParseDouble(String str, double defaultVal) {
        if (str == null || str.length() == 0)
            return defaultVal;
        return Double.parseDouble(str);
    }
    
    // Read write edge data in PNPRO format
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        // Exchange head & tail nodes
        if (exDir.FieldsToXml()) {
            if (getHeadNode() != null)
                el.setAttribute("head", getHeadNode().getUniqueName());
            if (getTailNode() != null)
                el.setAttribute("tail", getTailNode().getUniqueName());
        }
        else { // XML -> Fields
            assert exDir.idToNode != null;
            headNode = exDir.idToNode.get(el.getAttribute("head"));
            tailNode = exDir.idToNode.get(el.getAttribute("tail"));
        }
        
        if (exDir.FieldsToXml() || getHeadNode() != null)
            bindXMLAttrib(this, el, exDir, "head-magnet", "headMagnet", -1);
        
        if (exDir.FieldsToXml() || getTailNode() != null)
            bindXMLAttrib(this, el, exDir, "tail-magnet", "tailMagnet", -1);
        
        if (canBeBroken())
            bindXMLAttrib(this, el, exDir, "broken", "isBroken", false);
        
        // Exchange intermediate points
        if (exDir.XmlToFields()) {
            points.clear();
            NodeList pointList = el.getElementsByTagName("point");
            if (getTailNode() != null)
                points.add(new Point2D.Double(getTailNode().getCenterX(), getTailNode().getCenterY()));
            for (int pt=0; pt<pointList.getLength(); pt++) {
                org.w3c.dom.Node pointItem = pointList.item(pt);
                if (pointItem.getNodeType() != org.w3c.dom.Node.ELEMENT_NODE ||
                    !pointItem.getNodeName().equals("point"))
                    continue;
                Element pointElem = (Element)pointItem;
                points.add(new Point2D.Double(safeParseDouble(pointElem.getAttribute("x"), 0.0), 
                                              safeParseDouble(pointElem.getAttribute("y"), 0.0)));
            }
            if (getHeadNode() != null)
                points.add(new Point2D.Double(getHeadNode().getCenterX(), getHeadNode().getCenterY()));
        }
        else { // Fields -> XML
            for (int p=0; p<numPoints(); p++) {
                if (p == 0 && getTailNode() != null)
                    continue;
                if (p == numPoints() - 1 && getHeadNode() != null)
                    continue;
                Element point = exDir.getDocument().createElement("point");
                el.appendChild(point);
                point.setAttribute("x", ""+points.get(p).getX());
                point.setAttribute("y", ""+points.get(p).getY());
            }
        }
    }
}
