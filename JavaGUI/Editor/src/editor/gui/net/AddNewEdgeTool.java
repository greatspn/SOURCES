/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.DrawHelper;
import editor.domain.elements.DtaEdge;
import editor.domain.elements.DtaLocation;
import editor.domain.Edge;
import editor.domain.elements.GspnEdge;
import editor.domain.HandlePosition;
import editor.domain.MovementHandle;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.elements.Place;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.elements.Transition;
import editor.gui.UndoableCommand;
import static editor.gui.net.NetEditorPanel.Tool.NEW_GSPN_ARC;
import static editor.gui.net.NetEditorPanel.Tool.NEW_GSPN_INHIB_ARC;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.util.ArrayList;

/**
 *
 * @author elvio
 */
class AddNewEdgeTool extends NetToolBase {
    private static final double OUTSIDE_COORD = -100.0;
    Point2D initPt = new Point2D.Double(OUTSIDE_COORD, OUTSIDE_COORD);
    Edge newEdge = null;
    MovementHandle handle = null;
    Node hitNode = null;
    int hitMagnet = -1;
    boolean hitNodeIsConnectible = false;
    //boolean appendPointsToTail = true;
    Edge.EndPoint appendEndPoint = Edge.EndPoint.HEAD;

    public AddNewEdgeTool(NetEditorPanel editor, NetEditorPanel.Tool activeTool) {
        super(editor, activeTool);
        handle = NetObject.newPoint2DHandle(initPt, NetObject.POINT_GRID);
    }

    private boolean doHitTest(Point2D exactPt) {
        hitNode = editor.currPage.hitTestNode(NetObject.makeHitpointRectangle(exactPt, editor.currPage.viewProfile.zoom), false, NetPage.HIT_FOREGROUND_NODES, false);
        hitMagnet = -1;
        hitNodeIsConnectible = false;
        if (hitNode != null) {
            if (newEdge != null) {
                hitNodeIsConnectible = hitNode.canConnectEdges() && 
                                       newEdge.canConnectTo(hitNode, appendEndPoint);
            }
            else {
                if (activeTool == NetEditorPanel.Tool.NEW_GSPN_ARC || 
                    activeTool == NetEditorPanel.Tool.NEW_GSPN_INHIB_ARC)
                    hitNodeIsConnectible = (hitNode instanceof Place || hitNode instanceof Transition);
                else if (activeTool == NetEditorPanel.Tool.NEW_DTA_INNER_EDGE ||
                         activeTool == NetEditorPanel.Tool.NEW_DTA_BOUNDARY_EDGE)
                    hitNodeIsConnectible = (hitNode instanceof DtaLocation && 
                                            ((DtaLocation)hitNode).finalType == DtaLocation.FinalType.NON_FINAL);
            }
            hitMagnet = hitNode.findNearestMagnet(exactPt, editor.currPage.viewProfile.zoom);
        }
        return (hitNode != null);
    }

    @Override
    public void toolDisabled() { newEdge = null; } 

    @Override
    public void mouseDragged(MouseEvent e) { mouseMoved(e); }


    @Override
    public void mouseMoved(MouseEvent e) {
        if (!editor.isTopLevelActive())
            return;
        Point2D snappedPt = getLogicPointFromMousePos(e, NetObject.POINT_GRID);
        Point2D exactPt = getLogicPointFromMousePos(e, null);
        doHitTest(exactPt);
        if (!snappedPt.equals(handle.savePosition().getRefPoint()) ||
            handle.getNodeWithAura() != hitNode ||
            handle.getMagnetWithAura() != hitMagnet) 
        {
            if (newEdge != null) {
                Node connectNode = (hitNodeIsConnectible ? hitNode : null);
//                int magnet = -1;
//                if (connectNode != null)
//                    magnet = connectNode.findNearestMagnet(snappedPt, editor.currPage.zoomLevel);
                newEdge.connectToNode(connectNode, hitMagnet, appendEndPoint);
            }
            handle.moveTo(snappedPt.getX(), snappedPt.getY(), false); // after the hit test, having newEdge.end set
            editor.netPanel.repaint();
        }
    }

    @Override
    public void mouseExited(MouseEvent e) {
        if (!editor.isTopLevelActive())
            return;
        setOutsidePoint();
        editor.netPanel.repaint();
    }

    @Override public void mousePressed(MouseEvent e) { onClick(e); }

    @Override public void mouseReleased(MouseEvent e) { onClick(e); }

    public void onClick(MouseEvent e) { 
        if (e.getButton() != MouseEvent.BUTTON1)
            return;
        Point2D snappedPt = getLogicPointFromMousePos(e, NetObject.POINT_GRID);
        Point2D exactPt = getLogicPointFromMousePos(e, null);
        handle.moveTo(snappedPt.getX(), snappedPt.getY(), false);
        boolean hitTest = doHitTest(exactPt);
        if (newEdge == null && hitTest) {
            if (e.getID() != MouseEvent.MOUSE_PRESSED)
                return;
            if (!hitNodeIsConnectible)
                return;
            // Start a new edge
            editor.currPage.setSelectionFlag(false);
            ArrayList<Point2D> points = new ArrayList<>();
            points.add(handle.savePosition().getRefPoint());
            points.add(handle.savePosition().getRefPoint());
            switch (activeTool) {
                case NEW_GSPN_ARC:
                    newEdge = new GspnEdge(hitNode, hitMagnet, null, -1, points, false,
                                           GspnEdge.Kind.INPUT_OR_OUTPUT, "1");
                    break;
                case NEW_GSPN_INHIB_ARC:
                    // appendPointsToTail is true if is a Place -> Transition edge
                    appendEndPoint = (hitNode instanceof Place) ? Edge.EndPoint.HEAD : Edge.EndPoint.TAIL;
                    newEdge = new GspnEdge(appendEndPoint==Edge.EndPoint.HEAD ? hitNode : null, 
                                           appendEndPoint==Edge.EndPoint.HEAD ? hitMagnet : -1,
                                           appendEndPoint==Edge.EndPoint.TAIL ? hitNode : null, 
                                           appendEndPoint==Edge.EndPoint.TAIL ? hitMagnet : -1,
                                           points, false, GspnEdge.Kind.INHIBITOR, "1");
                    break;
                case NEW_DTA_INNER_EDGE:
                    newEdge = new DtaEdge(hitNode, hitMagnet, null, -1, points, DtaEdge.Kind.INNER, "Act", "x > 0", false);
                    break;
                case NEW_DTA_BOUNDARY_EDGE:
                    newEdge = new DtaEdge(hitNode, hitMagnet, null, -1, points, DtaEdge.Kind.BOUNDARY, "Act", "x = 0", false);
                    break;
                default:
                    throw new UnsupportedOperationException("Unsupported edge type");
            }
            handle = newEdge.getPointHandle(appendEndPoint==Edge.EndPoint.HEAD ? 1 : 0);
            editor.netPanel.repaint();
            return;
        }
        if (newEdge == null)
            return;

        // Avoid duplicate edges
        Point2D newPoint = handle.savePosition().getRefPoint();
        int lastPt = (appendEndPoint==Edge.EndPoint.HEAD ? newEdge.numPoints()-2 : 1);
        if (newEdge.points.get(lastPt).equals(newPoint))
            return; // Duplicate point

        if (newEdge != null && !hitTest) {
            int ptInd = newEdge.addNewPoint(newPoint, appendEndPoint);
            handle = newEdge.getPointHandle(ptInd);
            editor.netPanel.repaint();
        }
        else if (newEdge != null && hitTest) {
            // End this edge
            if (!hitNodeIsConnectible)
                return;
            newEdge.connectToNode(hitNode, hitMagnet, appendEndPoint);
            editor.mainInterface.executeUndoableCommand("edge added.", new UndoableCommand() {
                @Override
                public void Execute(ProjectData proj, ProjectPage page) throws Exception {
                    editor.currPage.edges.add(newEdge);
                    newEdge = null;
                    editor.toolEnds(false);
                }
            });
        }
    }

    private void setOutsidePoint() { 
        if (newEdge != null) {
            int pos = (appendEndPoint==Edge.EndPoint.HEAD ? newEdge.numPoints() - 2 : 1);
            handle.moveTo(newEdge.points.get(pos).getX(), newEdge.points.get(pos).getY(), false);
        }
        else
            handle.moveTo(OUTSIDE_COORD, OUTSIDE_COORD, false);
    }


    @Override
    public void topWindowActivated(boolean activated) {
        if (!activated) {
            setOutsidePoint();
            hitNode = null;
            editor.netPanel.repaint();
        }
    }

    @Override
    public void paintBeforeNet(Graphics2D g, DrawHelper dh) {
        if (hitNode != null && hitNodeIsConnectible) {
            hitNode.setAuraActive(true);
            hitNode.setMagnetWithAura(hitMagnet);
        }

        if (newEdge != null) {
            for (Node node : editor.currPage.nodes) {
                node.setGrayed(!newEdge.canConnectTo(node, appendEndPoint));
                if ((appendEndPoint==Edge.EndPoint.HEAD && node == newEdge.getTailNode()) || 
                    (appendEndPoint==Edge.EndPoint.TAIL && node == newEdge.getHeadNode())) {
                    node.setGrayed(false);
                    //node.isAuraActive = true;
                }
            }
            for (Edge edge : editor.currPage.edges)
                edge.setGrayed(true);
        }
    }

    @Override
    public void paintAfterNet(Graphics2D g, DrawHelper dh) {
        editor.currPage.setGrayedFlag(false);
        editor.currPage.setAuraFlag(false);
        // TODO: should call also setMagnetWithAura(-1) ???

        if (newEdge != null) {
            Composite oldComp = g.getComposite();
            g.setComposite(NetObject.ALPHA_50);
            newEdge.paintEdge(g, dh);
            g.setComposite(oldComp);
        }

        HandlePosition hp = handle.savePosition();
        paintGridLines(g, dh, hp.getRefX(), hp.getRefY(), null);
    }
}

