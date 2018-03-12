/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import editor.domain.elements.ClockVar;
import editor.domain.elements.DtaLocation;
import editor.domain.HandlePosition;
import editor.domain.MovementHandle;
import editor.domain.NetObject;
import editor.domain.Node;
import editor.domain.elements.Place;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.elements.Transition;
import editor.domain.elements.ConstantID;
import editor.domain.DrawHelper;
import editor.domain.NetPage;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.TextBox;
import editor.domain.elements.TokenType;
import editor.domain.grammar.ParserContext;
import editor.gui.UndoableCommand;
import static editor.gui.net.NetEditorPanel.Tool.NEW_EXP_TRN;
import static editor.gui.net.NetEditorPanel.Tool.NEW_GEN_TRN;
import static editor.gui.net.NetEditorPanel.Tool.NEW_IMM_TRN;
import static editor.gui.net.NetEditorPanel.Tool.NEW_INT_TEMPLATE;
import static editor.gui.net.NetEditorPanel.Tool.NEW_PLACE;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;

/** Interactive tool for adding new nodes to the net.
 *
 * @author elvio
 */
class AddNodeTool extends NetToolBase {
    
    Node newNode = null;
    boolean hitTestFailed = false;
    MovementHandle handle = null;
    
    static int lastRotationDegrees = 0;

    private String generateUniqueNodeName(boolean useZeroSuffix, String prefix) {
        for (int i=0; ; i++) {
            String name;
            if (i == 0 && !useZeroSuffix)
                name = prefix;  // Generate N instead of N0
            else
                name = prefix + i;
//            if (i >= 10)
//                name += "{" + i + "}";
//            else
//                name += i;
            boolean isDup = false;
            for (Node node : editor.currPage.nodes)
                if (node.getUniqueName().equals(name)) {
                    isDup = true;
                    break;
                }
            if (!isDup)
                return name;
        }
    }

    private static final double OUTSIDE_COORD = -100.0;
    private Point2D outsidePoint() { 
        return new Point2D.Double(OUTSIDE_COORD, OUTSIDE_COORD); 
    }

    public AddNodeTool(NetEditorPanel editor, NetEditorPanel.Tool tool) {
        super(editor, tool);
        switch (tool) {
            case NEW_INT_CONST:
                newNode = new ConstantID(ConstantID.ConstType.INTEGER, 
                                         generateUniqueNodeName(false, "N"), "0", "", outsidePoint());
                break;
            case NEW_REAL_CONST:
                newNode = new ConstantID(ConstantID.ConstType.REAL, 
                                         generateUniqueNodeName(false, "R"), "0.0", "", outsidePoint());
                break;
            case NEW_PLACE:
                newNode = new Place(generateUniqueNodeName(true, "P"), "", 
                                    TokenType.DISCRETE, ""/*color domain*/, ""/*partition*/, outsidePoint());
                break;
            case NEW_CONT_PLACE:
                newNode = new Place(generateUniqueNodeName(true, "C"), "", 
                                    TokenType.CONTINUOUS, ""/*color domain*/, ""/*partition*/, outsidePoint());
                break;
            case NEW_EXP_TRN:
                newNode = new Transition(generateUniqueNodeName(true, "T"), Transition.Type.EXP, 
                                         "1.0", "1", "1.0", "Infinite", "True", 0.0, outsidePoint());
                break;
            case NEW_IMM_TRN:
                newNode = new Transition(generateUniqueNodeName(true, "t"), Transition.Type.IMM, 
                                         "1.0", "1", "1.0", "Infinite", "True", 0.0, outsidePoint());
                break;
            case NEW_GEN_TRN:
                newNode = new Transition(generateUniqueNodeName(true, "D"), Transition.Type.GEN, 
                                         "I[1.0]", "1", "1.0", "Infinite", "True", 0.0, outsidePoint());
                break;
            case NEW_CONT_TRN:
                newNode = new Transition(generateUniqueNodeName(true, "F"), Transition.Type.CONT, 
                                         "1.0", "1", "1.0", "Infinite", "True", 0.0, outsidePoint());
                break;
            case NEW_DTA_LOC:
            case NEW_DTA_INIT_LOC:
            case NEW_DTA_ACCEPT_LOC:
            case NEW_DTA_REJECT_LOC:
                boolean isInit = (tool== NetEditorPanel.Tool.NEW_DTA_INIT_LOC);
                DtaLocation.FinalType ft = DtaLocation.FinalType.NON_FINAL;
                if (tool == NetEditorPanel.Tool.NEW_DTA_ACCEPT_LOC)
                    ft = DtaLocation.FinalType.ACCEPTING;
                if (tool == NetEditorPanel.Tool.NEW_DTA_REJECT_LOC)
                    ft = DtaLocation.FinalType.REJECTING;
                newNode = new DtaLocation(generateUniqueNodeName(true, "l"), 
                                          isInit, ft, "True", outsidePoint());
                break;
            case NEW_INT_TEMPLATE:
                newNode = new TemplateVariable(TemplateVariable.Type.INTEGER, 
                                               generateUniqueNodeName(false, "n"), "", outsidePoint());
                break;
            case NEW_REAL_TEMPLATE:
                newNode = new TemplateVariable(TemplateVariable.Type.REAL, 
                                               generateUniqueNodeName(false, "r"), "", outsidePoint());
                break;
            case NEW_STATEPROP:
                newNode = new TemplateVariable(TemplateVariable.Type.STATEPROP, 
                                               generateUniqueNodeName(true, "Phi"), "", outsidePoint());
                break;
            case NEW_ACTION:
                newNode = new TemplateVariable(TemplateVariable.Type.ACTION, 
                                               generateUniqueNodeName(true, "act"), "", outsidePoint());
                break;
            case NEW_COLOR_CLASS_TEMPLATE:
                newNode = new TemplateVariable(TemplateVariable.Type.COLOR_CLASS, 
                                               generateUniqueNodeName(true, "cl"), "", outsidePoint());
                break;
            case NEW_COLOR_CLASS:
                newNode = new ColorClass(generateUniqueNodeName(true, "C"), outsidePoint(), "c{1..3}");
                break;
            case NEW_COLOR_VAR:
                String colClass = "color";
                for (Node node : editor.currPage.nodes)
                    if (node instanceof ColorClass) {
                        colClass = node.getUniqueName();
                        break;
                    }
                newNode = new ColorVar(generateUniqueNodeName(true, "v"), outsidePoint(), colClass);
                break;
            case NEW_CLOCKVAR:
                newNode = new ClockVar(generateUniqueNodeName(false, "x"), outsidePoint());
                break;
            case NEW_TEXT_BOX_NODE:
                newNode = new TextBox("(Click to insert text)", outsidePoint(), 
                                      generateUniqueNodeName(true, "__textBox"));
                break;
            default:
                throw new UnsupportedOperationException("Unsupported tool");
        }
        if (newNode.mayRotate() && tool != NetEditorPanel.Tool.NEW_TEXT_BOX_NODE)
            newNode.setRotation(lastRotationDegrees * 45 * NetObject.DEG_TO_RAD);
        
        newNode.checkNodeCorrectness(editor.currPage, new ParserContext(editor.currPage));
        handle = newNode.getCenterHandle(editor.currPage);
        editor.currPage.setSelectionFlag(false);
    }

    private static final Point2D.Double nhtfCenterPt = new Point2D.Double();
    private boolean nodeHitTestFailed() {
        nhtfCenterPt.setLocation(newNode.getCenterX(), newNode.getCenterY());
        int whichNodesAreHit = newNode.isBackgroundNode() ? 
                NetPage.HIT_BACKGROUND_NODES : NetPage.HIT_FOREGROUND_NODES;
        Node hitNode = editor.currPage.hitTestNode(nhtfCenterPt, //handle.savePosition().getRefPoint(), 
                                                   newNode.getWidth() / 1.9,
                                                   newNode.getHeight() / 1.9,
                                                   true /*consider decors*/,
                                                   whichNodesAreHit);
        return hitTestFailed = (hitNode != null);
    }

    @Override
    public void mouseMoved(MouseEvent e) {
        if (handle == null || !editor.isTopLevelActive())
            return;
        Point2D pt = getLogicPointFromMousePos(e, NetObject.NODE_GRID);
        if (!pt.equals(handle.savePosition().getRefPoint())) {
            handle.moveTo(pt.getX(), pt.getY(), false);
            nodeHitTestFailed();
            editor.netPanel.repaint();
        }
    }

    @Override public void mouseDragged(MouseEvent e) { mouseMoved(e); }

    @Override
    public void mouseExited(MouseEvent e) {
        if (!editor.isTopLevelActive())
            return;
        handle.moveTo(OUTSIDE_COORD, OUTSIDE_COORD, false);
        editor.netPanel.repaint();
    }

    @Override
    public void topWindowActivated(boolean activated) {
        if (!activated) {
            handle.moveTo(OUTSIDE_COORD, OUTSIDE_COORD, false);
            editor.netPanel.repaint();
        }
    }

    @Override
    public void mousePressed(MouseEvent e) {
        if (e.getButton() != MouseEvent.BUTTON2 && e.getButton() != MouseEvent.BUTTON3)
            return;
        if (!newNode.mayRotate())
            return;
        // Rotate the node and remember the last rotation.
        lastRotationDegrees = (lastRotationDegrees + 1) % 4;
        newNode.setRotation(lastRotationDegrees * 45 * NetObject.DEG_TO_RAD);
        editor.netPanel.repaint();
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        if (e.getButton() != MouseEvent.BUTTON1)
            return;
        if (newNode == null)
            return;
        Point2D pt = getLogicPointFromMousePos(e, NetObject.NODE_GRID);
        handle.moveTo(pt.getX(), pt.getY(), false);
        if (nodeHitTestFailed())
            return;
        editor.mainInterface.executeUndoableCommand("node added.", new UndoableCommand() {
            @Override
            public void Execute(ProjectData proj, ProjectPage page) throws Exception {
                editor.currPage.nodes.add(newNode);
                newNode = null;
                editor.toolEnds(false);
            }
        });
    }

    @Override
    public void paintAfterNet(Graphics2D g, DrawHelper dh) {
        if (newNode == null)
            return;
        Composite oldComp = g.getComposite();
        g.setComposite(NetObject.ALPHA_50);
        newNode.paintNode(g, dh, hitTestFailed);
        g.setComposite(oldComp);

        HandlePosition hp = handle.savePosition();
        paintGridLines(g, dh, hp.getRefX(), hp.getRefY(), null);
    }

    @Override
    public void toolDisabled() {
        newNode = null;
    }
}

