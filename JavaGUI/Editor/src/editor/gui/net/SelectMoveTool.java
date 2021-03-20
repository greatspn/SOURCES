/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.DrawHelper;
import editor.domain.Edge;
import editor.domain.EditableCell;
import editor.domain.HandlePosition;
import editor.domain.MovementHandle;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.gui.NoOpException;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Stroke;
import java.awt.event.MouseEvent;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import javax.swing.event.CellEditorListener;
import javax.swing.event.ChangeEvent;

/** Select/Move nodes, edges and edge midpoints.
 *
 * @author elvio
 */
class SelectMoveTool extends NetToolBase {
    
    // Drag rectangular selection
    public boolean isDragSelecting = false;
    public Point startSelPt = new Point();
    public Point endSelPt = new Point();
    public boolean isInvertSelectionOp = false;
    
    // Single-selected sub-object management
    public MovementHandle[] visibleHandles = null;
    Selectable singleSelObject = null;
    
    // Handle dragging
    public boolean isDragHandle = false;
    public MovementHandle[] draggedHandles = null;
    Point2D startDragPos = new Point2D.Double(), currDragPos = new Point2D.Double();
    

    public SelectMoveTool(NetEditorPanel editor, NetEditorPanel.Tool activeTool) {
        super(editor, activeTool);
        NetPage page = editor.currPage;
        singleSelObject = page.getSingleSelectedObject();
        if (singleSelObject != null && !singleSelObject.isLocked()) {
            // A single object is selected. Get object handles
            visibleHandles = singleSelObject.getSubObjectHandles(page, editor.currPage.viewProfile);
        }
        else 
            visibleHandles = null;
    }
    
    // Minimum number of pixels before starting a drag selection
    private static final int SELECT_START_TOLERANCE = 3;

    @Override
    public void mousePressed(MouseEvent e) {
        if (e.getButton() != MouseEvent.BUTTON1)
            return;
        startDragPos = getLogicPointFromMousePos(e, null);
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        if (e.getButton() != MouseEvent.BUTTON1)
            return;
        if (isDragSelecting)
            return; // Do drag selection instead
        
        boolean isEditing = (e.getClickCount() == 2);
        
        // Hit test
        NetPage page = editor.currPage;
        Point2D clickPt = new Point2D.Double(NetObject.screenToLogic(e.getX(), page.viewProfile.zoom),
                                             NetObject.screenToLogic(e.getY(), page.viewProfile.zoom));
        Rectangle2D hitRect = NetObject.makeHitRectangle(clickPt, page.viewProfile.zoom);

//        System.out.println("mouseClicked "+isEditing+" hitRect="+hitRect+
//                " w="+NetObject.logicToScreen(hitRect.getWidth(), page.viewProfile.zoom));

        EditableCell editable = null;
        if (isEditing) {
            if (visibleHandles != null) {
                for (MovementHandle h : visibleHandles) {
                    EditableCell edt = h.getEditable();
                    if (edt != null && edt.isEditable() &&
                        edt.intersectRectangle(hitRect, editor.currPage.viewProfile, false)) 
                    {
                        editable = edt;
                        break;
                    }
                }
            }
            if (editable == null && singleSelObject != null && !singleSelObject.isLocked()) {
                EditableCell edt = singleSelObject.getCentralEditable(clickPt.getX(), clickPt.getY());
                if (edt != null && edt.isEditable() && 
                    edt.intersectRectangle(hitRect, editor.currPage.viewProfile, false))
                    editable = edt;
            }
            
            // Edit the doubly clicked object
            if (editable != null) {
                editObject(editable);
                editor.toolEnds(true);
                return;
            }
            // Add a new intermediate point if it is an Edge
            else if (singleSelObject instanceof Edge) {
                if (addEdgePointByDoubleClick((Edge)singleSelObject, clickPt))
                    return;
            }
        }
            
        // Try selecting if we cannot edit
        Selectable sel = null;
        boolean isSubObject = false;
        // Try visible handles first
        if (visibleHandles != null) {
            for (MovementHandle h : visibleHandles) {
                if (h instanceof Selectable) {
                    Selectable selH = (Selectable)h;
                    if (!selH.isLocked() && 
                        selH.intersectRectangle(hitRect, editor.currPage.viewProfile, false)) 
                    {
//                        System.out.println("clicked visibleHandle");
                        sel = selH;
                        isSubObject = true;
                        break;
                    }
                }
            }
        }
        
//        Node nodeSel = page.hitTestNode(hitRect, true, true, /*precedence to selected*/false);
//        Edge edgeSel = page.hitTestEdge(hitRect, true, /*precedence to selected*/false);
//        if (nodeSel != null && !nodeSel.isBackgroundNode())
//            sel = nodeSel;
//        else if (edgeSel != null && (nodeSel==null || nodeSel.isBackgroundNode()))
//            sel = edgeSel;
//        else
//            sel = nodeSel;
        if (sel == null)
            sel = page.hitTestNode(hitRect, true, NetPage.HIT_FOREGROUND_NODES,
                                   /*precedence to selected*/false);
        if (sel == null)
            sel = page.hitTestEdge(hitRect, true, /*precedence to selected*/false);
        if (sel == null)
            sel = page.hitTestNode(hitRect, true, NetPage.HIT_BACKGROUND_NODES,
                                   /*precedence to selected*/false);
        if (sel != null) {
            // Select the clicked object
            if (e.isShiftDown() && singleSelObject!=null && singleSelObject.isLocked()) {
                singleSelObject.setSelected(false);
                sel.setSelected(sel != singleSelObject);
            }
            else if (e.isShiftDown() && !sel.isLocked())
                sel.setSelected(!sel.isSelected());
            else {
                if (isSubObject) {
                    assert (singleSelObject instanceof Edge);
                    ((Edge)singleSelObject).setSubObjectSelection(false);
                }
                else {
                    // Do not use page.setSelectionFlag(false); because it breaks
                    // sub-object selection. Select nodes and edges one by one
                    for (Node node : page.nodes)
                        node.setSelected(node == sel);
                    for (Edge edge : page.edges)
                        edge.setSelected(edge == sel);
                }
                sel.setSelected(true);
            }
            editor.mainInterface.setStatus("object selected.", true);
        }
        else {
            // No object has been selected, deselect all
            page.setSelectionFlag(false);
            editor.mainInterface.setStatus("deselect all.", true);
        }
        //editor.netPanel.repaint();
        editor.toolEnds(true);
    }
    
    private void editObject(final EditableCell editable) {
        // Show the editable text editor
        NetPage page = editor.currPage;
        Point2D ec = editable.getEditorCenter();
        Point p = new Point(NetObject.logicToScreen(ec.getX(), page.viewProfile.zoom),
                            NetObject.logicToScreen(ec.getY(), page.viewProfile.zoom));
        
        BaseCellEditor bcEditor;
        if (editable.editAsMultiline())
            bcEditor = new MultilineTextCellEditor(editor.currProject.getCurrent(), editor.currPage, editable, p);
        else
            bcEditor = new TextCellEditor(editor.currProject.getCurrent(), editor.currPage, editable, p);
        final BaseCellEditor cellEditor = bcEditor;
        
        cellEditor.addCellEditorListener(new CellEditorListener() {
            @Override
            public void editingStopped(ChangeEvent ce) {
                // Modify the edited value
                editor.mainInterface.executeUndoableCommand("change value.", (ProjectData proj, ProjectPage page1) -> {
                    editable.setValue(editor.currProject.getCurrent(), (NetPage) page1, cellEditor.getEditedString());
                });

                // Disable the cell editor
                editor.activeEditor = null;
                editor.netPanel.remove(cellEditor.getEditorComponent());
                editor.mainInterface.setSuspended(false);
                editor.netPanel.requestFocusInWindow();
//                editor.netPanel.repaint();
            }

            @Override
            public void editingCanceled(ChangeEvent ce) {
                editor.activeEditor = null;
                editor.netPanel.remove(cellEditor.getEditorComponent());
                editor.mainInterface.setSuspended(false);
                editor.netPanel.requestFocusInWindow();
                editor.mainInterface.invalidateGUI();
//                editor.netPanel.repaint();
            }
        });
        editor.activeEditor = cellEditor;
        editor.netPanel.add(cellEditor.getEditorComponent());
        cellEditor.getEditorComponent().requestFocusInWindow();
        editor.mainInterface.setSuspended(true);
    }
    

    @Override
    public void mouseReleased(MouseEvent e) {
        if (e.getButton() != MouseEvent.BUTTON1)
             return;

        NetPage page = editor.currPage;
        currDragPos = getLogicPointFromMousePos(e, null);
        
        if (isDragSelecting) {
            // End rectangular selection
            isDragSelecting = false;
            endSelPt.setLocation(e.getX(), e.getY());
            Rectangle selRect = makeSelectionRectangle();
            Rectangle2D logicSelrect = makeLogicSelectionRectangle(selRect);
            selectNodes(logicSelrect);
            //editor.netPanel.repaint();
            editor.mainInterface.setStatus("selection changed.", true);
            editor.toolEnds(true);
        }
        else if (isDragHandle) {
            // End handle movement
            isDragHandle = false;
            editor.mainInterface.executeUndoableCommand("move selected object(s).", 
                    (ProjectData proj, ProjectPage page1) -> 
            {
                double deltaX = currDragPos.getX() - startDragPos.getX();
                double deltaY = currDragPos.getY() - startDragPos.getY();
                if (!moveDraggedHandles(deltaX, deltaY))
                    throw new NoOpException();
            });
        }
    }

    @Override
    public void mouseDragged(MouseEvent e) {
        NetPage page = editor.currPage;
        int dragDistance = Math.abs(e.getX()-startDragX) + Math.abs(e.getY()-startDragY);
        
            // Test if we can start a new drag
        if (dragDistance > SELECT_START_TOLERANCE && !isDragSelecting && !isDragHandle) {
            // Hit test
            MovementHandle[] handles = getHitHandles(startDragX, startDragY);
            if (handles != null) {
                // Start dragging the handle
                isDragHandle = true;
                draggedHandles = handles;
            }
            else {
                //no handle has been hit, start rectangular selection
                isDragSelecting = true;
                startSelPt.setLocation(e.getX(), e.getY());
                isInvertSelectionOp = e.isShiftDown();
                visibleHandles = null;
                singleSelObject = null;
            }
        }

        currDragPos = getLogicPointFromMousePos(e, null);
        if (isDragSelecting) {
            String status = "start drag selection. Start dragging with SHIFT "
                            + "pressed to invert selection.";
            if (isInvertSelectionOp)
                status = "start inverse drag selection.";
            editor.mainInterface.setStatus(status, true);
            endSelPt.setLocation(e.getX(), e.getY());
            editor.netPanel.scrollRectToVisible(makeSelectionRectangle());
            editor.netPanel.repaint();
        }
        if (isDragHandle) {
            editor.mainInterface.setStatus("drag selected object(s). Press ALT to "
                                + "invert grid snapping while dragging.", true);
            _selRect.setRect(e.getX()-10, e.getY()-10, 20, 20);
            editor.netPanel.scrollRectToVisible(_selRect);
            editor.netPanel.repaint();
        }
    }
    
    private MovementHandle[] newHandleArray(MovementHandle h) {
        MovementHandle[] mh = new MovementHandle[1];
        mh[0] = h;
        return mh;
    }
    
    private MovementHandle[] getHitHandles(int px, int py) {
        NetPage page = editor.currPage;
        Point2D point = new Point2D.Double(NetObject.screenToLogic(px, page.viewProfile.zoom), 
                                           NetObject.screenToLogic(py, page.viewProfile.zoom));
        Rectangle2D hitRect = NetObject.makeHitRectangle(point, page.viewProfile.zoom);
        // special handles (edge points, etc)
        if (visibleHandles != null) {
            for (MovementHandle h : visibleHandles) {
                if (h.intersectRectangle(hitRect, editor.currPage.viewProfile, false)) {
                    if (h instanceof Selectable && ((Selectable)h).isSelected()) {
                        // Move all the other selected visible handles
                        ArrayList<MovementHandle> allSelHandles = new ArrayList<>();
                        allSelHandles.add(h); //  Let h be the first handle
                        for (MovementHandle h2 : visibleHandles)
                            if (h2!=h && h2 instanceof Selectable && ((Selectable)h2).isSelected())
                                allSelHandles.add(h2);
                        return allSelHandles.toArray(new MovementHandle[0]);
                    }
                    else {
                        // Return the sub-object handle, after having removed other selected sub-objects
                        if (singleSelObject instanceof Edge)
                            ((Edge)singleSelObject).setSubObjectSelection(false);
                        if (h instanceof Selectable)
                            ((Selectable)h).setSelected(true);
                        return newHandleArray(h);
                    }
                }
                    
            }
        }
        // the single selected object
        if (singleSelObject != null && !singleSelObject.isLocked()) {
            MovementHandle centerHandle = singleSelObject.getCenterHandle(page);
            if (centerHandle!=null && 
                centerHandle.intersectRectangle(hitRect, editor.currPage.viewProfile, true))
                return newHandleArray(centerHandle);
        }
        // any object that becomes immediately selected
        Selectable sel = page.hitTestNode(hitRect, true, NetPage.HIT_FOREGROUND_NODES,
                                          /*precedence to selected*/true);
        if (sel == null)
            sel = page.hitTestEdge(hitRect, true, /*precedence to selected*/true);
        if (sel == null)
            sel = page.hitTestNode(hitRect, true, NetPage.HIT_BACKGROUND_NODES,
                                   /*precedence to selected*/true);
        if (sel != null && !sel.isLocked()) {
            if (sel.isSelected()) {
                // Move all the selected objects
                int numSel = 0, j = 0;
                for (Node n : page.nodes)
                    if (n.isSelected())
                        numSel++;
                for (Edge e : page.edges)
                    if (e.isSelected())
                        numSel++;
                MovementHandle[] allSelHandles = new MovementHandle[numSel];
                allSelHandles[j++] = sel.getCenterHandle(page); // let sel be the first object
                for (Node n : page.nodes)
                    if (n.isSelected() && n != sel)
                        allSelHandles[j++] = n.getCenterHandle(page);
                for (Edge e : page.edges)
                    if (e.isSelected() && e != sel)
                        allSelHandles[j++] = e.getCenterHandle(page);
                return allSelHandles;
            }
            else {
                // Select the dragged object and deselect the others
                page.setSelectionFlag(false);
                sel.setSelected(true);
                visibleHandles = null;
                return newHandleArray(sel.getCenterHandle(page));
            }
        }
        
        return null;
    }

    private static final Rectangle _selRect = new Rectangle();
    private Rectangle makeSelectionRectangle() {
        _selRect.setRect(Math.min(startSelPt.x, endSelPt.x), 
                         Math.min(startSelPt.y, endSelPt.y), 
                         Math.max(1, Math.abs(startSelPt.x - endSelPt.x)), 
                         Math.max(1, Math.abs(startSelPt.y - endSelPt.y)));
        return _selRect;
    }
    
    private static final Rectangle2D _logicSelRect = new Rectangle2D.Double();
    private Rectangle2D makeLogicSelectionRectangle(Rectangle selRect) {
        NetPage page = editor.currPage;
        _logicSelRect.setRect(NetObject.screenToLogic(selRect.x, page.viewProfile.zoom), 
                              NetObject.screenToLogic(selRect.y, page.viewProfile.zoom),
                              NetObject.screenToLogic(selRect.width, page.viewProfile.zoom),
                              NetObject.screenToLogic(selRect.height, page.viewProfile.zoom));
        return _logicSelRect;
    }
    
    private void selectNodes(Rectangle2D logicSelRect) {
        NetPage page = editor.currPage;
        for (Node node : page.nodes)
            changeSelectionStatus(logicSelRect, node);
        for (Edge edge : page.edges)
            changeSelectionStatus(logicSelRect, edge);
    }
    private void changeSelectionStatus(Rectangle2D logicSelRect, Selectable obj) {
        boolean b = !obj.isLocked() && obj.intersectRectangle(logicSelRect, editor.currPage.viewProfile, true);
        if (isInvertSelectionOp)
            obj.setSelected((obj.isSelected() && !b) || (!obj.isSelected() && b));
        else
            obj.setSelected(b);
    }
    
    // Called when double-clicking an edge to add a new point.
    private boolean addEdgePointByDoubleClick(final Edge edge, Point2D clickPt) {
        if (edge.isBroken)
            return false; // Unsupported for broken edges.

        // The new point is taken as an interpolation of existing points
        final double nearK = edge.getNearestK(clickPt);
        final Point2D newPtPos = edge.getPointAlongTheLine(new Point2D.Double(), nearK);

        // Avoid creating a new point that is too near to an existing one
        final double MIN_DISTANCE = 0.5;
        for (int i=0; i<edge.numPoints(); i++)
            if (newPtPos.distance(edge.points.get(i)) < MIN_DISTANCE)
                return false;
        
        editor.mainInterface.executeUndoableCommand("add new edge point.", (ProjectData proj, ProjectPage page) -> {
            // Save decor positions
            Point2D[] decorAnchors = new Point2D[edge.getNumDecors()];
            for (int d=0; d<edge.getNumDecors(); d++)
                decorAnchors[d] = edge.getPointAlongTheLine(new Point2D.Double(),
                        edge.getDecor(d).getEdgeK());
            // Add the new point
            int ptPos = (int)Math.ceil(nearK);
            edge.points.add(ptPos, newPtPos);
            edge.setSubObjectSelection(false);
            edge.setSelectedSubObject(ptPos, true);
            edge.invalidateEffectiveEdgePath();
            // Compute new anchor points (near the old anchors)
            for (int d=0; d<edge.getNumDecors(); d++)
                edge.getDecor(d).setEdgeK(edge.getNearestK(decorAnchors[d]));
        });
        return true;
    }
    

    boolean[] currNodeSel, currEdgeSel;
    HandlePosition[] currHandlesPos;
    @Override
    public void paintBeforeNet(Graphics2D g, DrawHelper dh) {
        NetPage page = editor.currPage;
        if (isDragSelecting) {
            // Store the current selection status
            if (currNodeSel == null || currNodeSel.length!=page.nodes.size())
                currNodeSel = new boolean[page.nodes.size()];
            for (int i=0; i<page.nodes.size(); i++)
                currNodeSel[i] = page.nodes.get(i).isSelected();
            if (currEdgeSel == null || currEdgeSel.length!=page.edges.size())
                currEdgeSel = new boolean[page.edges.size()];
            for (int i=0; i<page.edges.size(); i++)
                currEdgeSel[i] = page.edges.get(i).isSelected();
            
            // Change selection of nodes and edges
            Rectangle selRect = makeSelectionRectangle();
            Rectangle2D logicSelrect = makeLogicSelectionRectangle(selRect);
            selectNodes(logicSelrect);
        }
        if (isDragHandle) {
            if (currHandlesPos==null || currHandlesPos.length!=draggedHandles.length)
                currHandlesPos = new HandlePosition[draggedHandles.length];
            for (int i=0; i<draggedHandles.length; i++)
                currHandlesPos[i] = draggedHandles[i].savePosition();
            moveDraggedHandles(currDragPos.getX() - startDragPos.getX(), 
                               currDragPos.getY() - startDragPos.getY());
            if (draggedHandles.length == 1) {
                MovementHandle singleDraggedHandle = draggedHandles[0];
                Node auraNode = singleDraggedHandle.getNodeWithAura();
                if (auraNode != null) {
                    auraNode.setAuraActive(true);
                    auraNode.setMagnetWithAura(singleDraggedHandle.getMagnetWithAura());
                }
                if (singleDraggedHandle.isGrayNodeRuleActive()) {
                    page.setGrayedFlag(true);
                    singleSelObject.setGrayed(false);
                    for (Node node : page.nodes)
                        node.setGrayed(singleDraggedHandle.isNodeGrayed(node));
                }
            }
        }
    }
    
    private NetObject.MeshGridSize lastMoveGridSize = null;
    private final Point2D mdhDelta = new Point2D.Double();
    private boolean moveDraggedHandles(double Dx, double Dy) {
        boolean hasMoved = false;
        if (draggedHandles.length == 1 && draggedHandles[0].canMoveTo()) {
            MovementHandle h = draggedHandles[0];
            // Move this single object with the moveTo primitive
            HandlePosition pos = h.savePosition();
            Point2D ref = pos.getRefPoint();
            ref.setLocation(ref.getX() + Dx, ref.getY() + Dy);
            lastMoveGridSize = h.getPreferredMeshGridSize(Dx, Dy, false);
            snapPointToGrid(ref, lastMoveGridSize);
            h.moveTo(ref.getX(), ref.getY(), false);
            if (!h.savePosition().getRefPoint().equals(pos.getRefPoint()))
                hasMoved = true;
        }
        else {
            lastMoveGridSize = draggedHandles[0].getPreferredMeshGridSize(Dx, Dy, true);
            for (MovementHandle h : draggedHandles)
                lastMoveGridSize = NetObject.mcdGridSize(lastMoveGridSize, h.getPreferredMeshGridSize(Dx, Dy, true));
            mdhDelta.setLocation(Dx, Dy);
            snapPointToGrid(mdhDelta, lastMoveGridSize);
            for (MovementHandle h : draggedHandles) {
                Point2D ref = h.savePosition().getRefPoint();
                h.moveBy(mdhDelta.getX(), mdhDelta.getY(), (draggedHandles.length != 1));
                if (!h.savePosition().getRefPoint().equals(ref))
                    hasMoved = true;
            }
        }
        return hasMoved;
    }
    
    private static final Color SEL_RECT_COLOR = new Color(0, 0, 204);
    private static final Stroke SEL_STROKE = new BasicStroke(1.0f, BasicStroke.CAP_SQUARE, 
                                                             BasicStroke.JOIN_MITER, 
                                                             1.0f, new float[] {6.0f, 3.0f}, 0.0f);
    @Override
    public void paintAfterNet(Graphics2D g, DrawHelper dh) {
        NetPage page = editor.currPage;

        // Custom drawing
        if (visibleHandles != null) {
            MovementHandle clickedHandle = (draggedHandles!=null && draggedHandles.length==1 ? 
                                            draggedHandles[0] : null);
            for (MovementHandle h : visibleHandles)
                h.paintHandle(g, dh, h == clickedHandle);
        }

        if (isDragSelecting) {
            // set back the selection status, without changes
            assert currNodeSel.length == page.nodes.size() && currEdgeSel.length == page.edges.size();
            for (int i=0; i<page.nodes.size(); i++)
                page.nodes.get(i).setSelected(currNodeSel[i]);
            for (int i=0; i<page.edges.size(); i++)
                page.edges.get(i).setSelected(currEdgeSel[i]);
            
            // Draw the selection rectangle
            AffineTransform oldAT = g.getTransform();
            g.setTransform(dh.baseTransform);
            g.setColor(SEL_RECT_COLOR);
            Stroke oldStroke = g.getStroke();
            g.setStroke(SEL_STROKE);
            g.draw(makeSelectionRectangle());
            g.setStroke(oldStroke);
            g.setTransform(oldAT);
        }
        
        if (isDragHandle) {
            // Paint the grid lines
            HandlePosition actualPos = draggedHandles[0].savePosition();
            paintGridLines(g, dh, actualPos.getRefX(), actualPos.getRefY(), lastMoveGridSize);
            
            // restore the dragged handle position
            for (int i=0; i<draggedHandles.length; i++)
                draggedHandles[i].restorePosition(currHandlesPos[i]);
            page.setGrayedFlag(false);
            page.setAuraFlag(false);
        }
    }


}
