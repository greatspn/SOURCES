/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain;

import editor.domain.elements.TemplateVariable;
import common.Util;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ConstantID;
import editor.domain.elements.ParsedColorSubclass;
import editor.domain.grammar.ExprRewriter;
import editor.domain.grammar.NodeNamespace;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.domain.composition.ComposableNet;
import editor.gui.net.NetEditorPanel;
import java.awt.Component;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.print.PageFormat;
import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

/** A project page with a network of nodes connected with arcs.
 *
 * @author elvio
 */
public abstract class NetPage extends ProjectPage implements Serializable, ComposableNet, NodeNamespace {
    
    // The set of nodes
    public final ArrayList<Node> nodes = new ArrayList<>();
    // The set of edges
    public final ArrayList<Edge> edges = new ArrayList<>();
    
    // The (precomputed) rectangle that envelops all the nodes and edges
    private transient Rectangle2D boundsOfContent = null;
    
    // Current viewport position
    public Point viewportPos = new Point(0, 0);
    // Current view profile
    public ViewProfile viewProfile = new ViewProfile();
    // Is grid snapping active
    public boolean gridSnappingActive = true;
    // Current tool
    public NetEditorPanel.Tool editorTool = NetEditorPanel.Tool.SELECT;
    public int editorToolRepetition = 1;
    
    // Precomputed list of types (color classes)
    private transient Set<ColorClass> colorClassList;
    
    
    public NetPage() { }
    
    @Override public boolean hasEditableName() {
        return true;
    }
    @Override public Class getEditorClass() {
        return NetEditorPanel.class;
    }

    
    
    public double getHitRadius() {
        final int HIT_RADIUS_PIXELS = 5;
        return NetObject.screenToLogic(HIT_RADIUS_PIXELS, viewProfile.zoom);
    }
    
    public static final int HIT_FOREGROUND_NODES = 150;
    public static final int HIT_BACKGROUND_NODES = 160;
    private static final Rectangle2D.Double internalHitTestRect = new Rectangle2D.Double();
    public Node hitTestNode(Point2D pt, double xHitRadius, double yHitRadius, 
                            boolean includeDecors, int foreOrBack) 
    {
        internalHitTestRect.setRect(pt.getX()-xHitRadius, pt.getY()-yHitRadius, 
                                    2*xHitRadius, 2*yHitRadius);
        return hitTestNode(internalHitTestRect, includeDecors, foreOrBack, false);
    }
    public Node hitTestNode(Rectangle2D hitTestRect, boolean includeDecors, 
                            int foreOrBack, boolean precedenceToSelected) 
    { 
        // Read the list in reverse order, which correspond to the visual Z-order
        if (precedenceToSelected) {
            // Give precedence to selected nodes, then test unselected ones
            for (int i=nodes.size()-1; i>=0; i--) {
                Node n = nodes.get(i);
                if (n.isBackgroundNode() == (foreOrBack == HIT_BACKGROUND_NODES)) {
                    if (n.isSelected() && n.intersectRectangle(hitTestRect, viewProfile, includeDecors))
                        return n;
                }
            }
            for (int i = nodes.size()-1; i>=0; i--) {
                Node n= nodes.get(i);
                if (n.isBackgroundNode() == (foreOrBack == HIT_BACKGROUND_NODES)) {
                    if (!n.isSelected() && n.intersectRectangle(hitTestRect, viewProfile, includeDecors))
                        return n;
                }
            }
        }
        else {
            for (int i = nodes.size()-1; i>=0; i--) {
                Node n= nodes.get(i);
                if (n.isBackgroundNode() == (foreOrBack == HIT_BACKGROUND_NODES)) {
                    if (n.intersectRectangle(hitTestRect, viewProfile, includeDecors))
                        return nodes.get(i);
                }
            }
        }
        return null;
    }
    
    public Edge hitTestEdge(Rectangle2D hitTestRect, boolean includeDecors, boolean precedenceToSelected) {
        if (precedenceToSelected) {
            for (int i=edges.size()-1; i>=0; i--) {
                Edge e = edges.get(i);
                if (e.isSelected() && e.intersectRectangle(hitTestRect, viewProfile, includeDecors))
                    return e;
            }
            for (int i=edges.size()-1; i>=0; i--) {
                Edge e = edges.get(i);
                if (!e.isSelected() && e.intersectRectangle(hitTestRect, viewProfile, includeDecors))
                    return e;
            }
        }
        else {
            for (int i=edges.size()-1; i>=0; i--)
                if (edges.get(i).intersectRectangle(hitTestRect, viewProfile, includeDecors))
                    return edges.get(i);      
        }
        return null;
    }
    
    
    
    
    @Override
    public Node getNodeByUniqueName(String uniqueName) {
        for (Node node : nodes)
            if (node.getUniqueName().equals(uniqueName))
                return node;
        return null;
    }
    
    @Override
    public Iterator<ColorClass> colorClassIterator() {
//        assert colorClassList != null;
        if (colorClassList == null)
            return null;
        return colorClassList.iterator();
    }
    
    // Returns the selected object if there is a single selected objects, null otherwise
    public Selectable getSingleSelectedObject() {
        Selectable sel = null;
        for (Node node : nodes) {
            if (node.isSelected()) {
                if (sel == null)
                    sel = node;
                else return null;
            }
        }
        for (Edge edge : edges) {
            if (edge.isSelected()) {
                if (sel == null)
                    sel = edge;
                else return null;
            }
        }
        return sel;
    }
    
    public int countSelectedNodes() {
        int count = 0;
        for (Node node : nodes)
            if (node.isSelected())
                count++;
        return count;
    }
    
    public int countSelectedEdges() {
        int count = 0;
        for (Edge edge : edges)
            if (edge.isSelected())
                count++;
        return count;
    }
    
    public Selectable[] getAllSelectedObjects() {
        Selectable[] sel = new Selectable[countSelectedEdges() + countSelectedNodes()];
        int k = 0;
        for (Node node : nodes)
            if (node.isSelected())
                sel[k++] = node;
        for (Edge edge : edges)
            if (edge.isSelected())
                sel[k++] = edge;
        return sel;
    }
    
    // =========== Resource management ============
    
    @Override
    public void relinkTransientResources(Map<UUID, ProjectResource> resourceTable) {
        for (Node node : nodes)
            node.relinkTransientResources(resourceTable);
        for (Edge edge : edges)
            edge.relinkTransientResources(resourceTable);
    }

    @Override
    public void retrieveLinkedResources(Map<UUID, ProjectResource> resourceTable) {
        for (Node node : nodes)
            node.retrieveLinkedResources(resourceTable);
        for (Edge edge : edges)
            edge.retrieveLinkedResources(resourceTable);
    }
    
    // =========== Unique name generation ============

    public String generateUniqueNodeName(boolean useZeroSuffix, String prefix) {
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
            for (Node node : nodes)
                if (node.getUniqueName().equals(name)) {
                    isDup = true;
                    break;
                }
            if (!isDup)
                return name;
        }
    }
    
    // =========== Test for page content correctness ===========
    
    private static final Point2D.Double cpcPoint = new Point2D.Double();
    public static final double EXTRA_PAGE_BOUNDS = 1.0;
    @Override
    protected boolean checkPageCorrectness(boolean isNewOrModified, ProjectData proj, 
                                           Set<ProjectPage> changedPages, 
                                           ProjectPage invokerPage) 
    {
//        // OPTIMIZATION: (disabled only because I prefer to have everything rechecked everytime, 
//        // but it should work flawlessly.
//        if (!isNewOrModified)
//            return false; // nothing changed, do not rebuild info
        
        // Clear any previous page errors
        clearPageErrorsAndWarnings();
        
        // Recompute the exact page bounds (pageDimension)
        boundsOfContent = null;
        
        // Enumerate color classes *before* initializing the context
        colorClassList = new HashSet<>();
        for (Node node : nodes)
            if (node instanceof ColorClass)
                colorClassList.add((ColorClass)node);
                
        // Create the parser context of this page
        ParserContext pageContext = new ParserContext(this);
        
        // Verify the correctness of normal nodes.
        Set<String> knownNames = new HashSet<>();
        for (Node node : nodes) {
            node.checkNodeCorrectness(this, pageContext);
            knownNames.add(node.getUniqueName());
            if (NetObject.isIdentifierKeyword(node.getUniqueName(), node.getClass())) {
                addPageWarning("Identifier '"+node.getUniqueName()+"' is a keyword and should not be used.", node);
            }

            boolean isOutsideVisibleArea = (node.getX() < 0 || node.getY() < 0);
            for (int d=0; d<node.getNumDecors(); d++) {
                Decor decor = node.getDecor(d);
                if (!decor.isVisible(viewProfile))
                    continue;
                decor.getEffectivePos(cpcPoint);
                isOutsideVisibleArea = isOutsideVisibleArea || (cpcPoint.getX()< 0 || cpcPoint.getY() < 0); 
            }            
            if (isOutsideVisibleArea) {
                addPageWarning("The node is partially or completely "+
                               "outside of the visible area.", node);
            }
        }
        
        // Verify some basic node/edge properties
        for (Edge edge : edges) {
            edge.checkEdgeCorrectness(this, pageContext);
            
            // Test for visibility
            boolean isOutsideVisibleArea = false;
            for (int i=0; i<edge.numPoints(); i++) {
                edge.getPoint(cpcPoint, i);
                isOutsideVisibleArea = isOutsideVisibleArea || (cpcPoint.getX()< 0 || cpcPoint.getY() < 0); 
            }
            for (int d=0; d<edge.getNumDecors(); d++) {
                Decor decor = edge.getDecor(d);
                if (!decor.isVisible(viewProfile))
                    continue;
                decor.getEffectivePos(cpcPoint);
                isOutsideVisibleArea = isOutsideVisibleArea || (cpcPoint.getX()< 0 || cpcPoint.getY() < 0); 
            }
            if (isOutsideVisibleArea) {
                addPageWarning("The edge is partially or completely " +
                               "outside of the visible area.", edge);
            }
        }
        
        // Named static color subclasses and colors must have unique names.
        for (Node node : nodes) {
            if (node instanceof ColorClass) {
                ColorClass cc = (ColorClass)node;
                if (cc.isParseDataOk() && cc.isSimpleClass()) {
                    for (int i=0; i<cc.numSubClasses(); i++) {
                        ParsedColorSubclass pcs = cc.getSubclass(i);
                        if (!pcs.isNamed())
                            continue;
                        if (NetObject.isIdentifierKeyword(pcs.name, pcs.getClass())) {
                            addPageWarning("Identifier '"+pcs.name+"' is a keyword and should not be used.", node);
                        }
                        if (knownNames.contains(pcs.name)) {
                            addPageError("Static color subclass "+pcs.name+" does not have a unique name.", cc);
                        }
                        else knownNames.add(pcs.name);
                        
                        pcs.testHasDuplicatedColorNames(this, cc, knownNames);
                    }
                }
            }
        }
        
        // Check if the page has just one selected object, and keep it in the viewProfile.
        viewProfile.singleSelObject = getSingleSelectedObject();
               
        return true;
    }

    // This method is called when the user select an error in the error window
    // It deselect all the other nodes/edges, and selects the error node
    @Override
    public void onSelectErrorWarning(PageErrorWarning err) {
        setSelectionFlag(false);
        if (err != null)
            err.selectAffectedElements(true);
    }
    
    public void applyRewriteRule(ParserContext context, ExprRewriter rewriter) {
        for (Node node : nodes)
            node.rewriteNode(context, rewriter);
        for (Edge edge : edges)
            edge.rewriteEdge(context, rewriter);
    }
    
    // =========== Management of parsed color classes that require additional parse informations. ============
    
    public void compileParsedInfo(ParserContext context) {
        for (Node node : nodes) {
            if (node instanceof ColorClass) {
                ColorClass cc = (ColorClass)node;
                cc.evaluateColorRange(context);
            }
        }
    }
    
    // =========== List of template variables that require an expression binding. ============
    
    public TemplateBinding getListOfTemplateVariables() {
        TemplateBinding tbind = new TemplateBinding();
        for (Node node : nodes) {
            if (node instanceof TemplateVariable) {
                tbind.bindSingleValue(node.getUniqueName(), 
                                      ((TemplateVariable)node).getLastBindingExpr());
            }
        }
        return tbind;
    }

    
    // =========== Page dimensions and exact bounds. ============
    
    private  Rectangle2D getOrComputeBoundsOfContent() {
        if (boundsOfContent == null) {
            boundsOfContent = computePageBounds(EXTRA_PAGE_BOUNDS, EXTRA_PAGE_BOUNDS);
        }
        return boundsOfContent;
    }
    
    public static final double EDITOR_PAGE_UNIT = 5.0;
    public static final int MIN_PAGE_WIDTH_UNITS = 4;
    public static final int MIN_PAGE_HEIGHT_UNITS = 3;
    
    public double getPageWidthForEditor() {
        int numUnits = Math.max((int)Math.ceil(getOrComputeBoundsOfContent().getMaxX() / EDITOR_PAGE_UNIT + 0.25), MIN_PAGE_WIDTH_UNITS);
        return numUnits * EDITOR_PAGE_UNIT;
    }

    public double getPageHeightForEditor() {
        int numUnits = Math.max((int)Math.ceil(getOrComputeBoundsOfContent().getMaxY() / EDITOR_PAGE_UNIT + 0.25), MIN_PAGE_HEIGHT_UNITS);
        return numUnits * EDITOR_PAGE_UNIT;
    }
    
    private Rectangle2D unionRect(Rectangle2D out, Rectangle2D in) {
        // out = UNION(out, in)
        if (out == null) {
            return new Rectangle2D.Double(in.getX(), in.getY(), 
                                          in.getWidth(), in.getHeight());
        }
        // bottom-right points
        double ax = out.getX()+out.getWidth(), ay = out.getY() + out.getHeight();
        double bx = in.getX()+in.getWidth(), by = in.getY() + in.getHeight();
        // top left point of the intersection
        double x = Math.min(out.getX(), in.getX());
        double y = Math.min(out.getY(), in.getY());
        out.setRect(x, y, Math.max(ax, bx) - x, Math.max(ay, by) - y);
        return out;
    }
    
    private static final Rectangle2D gpbRect = new Rectangle2D.Double();
    private static final Point2D.Double gpbPoint = new Point2D.Double();
    private Rectangle2D computePageBounds(double dx, double dy) {
        Rectangle2D boundRect = null;
        for (Node node : nodes) {
            gpbRect.setRect(node.getX(), node.getY(), node.getWidth(), node.getHeight());
            boundRect = unionRect(boundRect, gpbRect);
            for (int d=0; d<node.getNumDecors(); d++) {
                Decor decor = node.getDecor(d);
                if (!decor.isVisible(viewProfile))
                    continue;
                decor.getEffectivePos(gpbPoint);
                gpbRect.setRect(gpbPoint.getX(), gpbPoint.getY(), decor.getWidth(), decor.getHeight());
                //System.out.println("  gpbRect="+gpbRect+" for decor "+decor);
                boundRect = unionRect(boundRect, gpbRect);
            }
        }
        for (int i=0; i<edges.size(); i++) {
            Edge edge = edges.get(i);
            for (int n=0; n<edge.numPoints(); n++) {
                edge.getPoint(gpbPoint, n);
                gpbRect.setRect(gpbPoint.x, gpbPoint.y, 0.1, 0.1);
                boundRect = unionRect(boundRect, gpbRect);
            }
            for (int d=0; d<edge.getNumDecors(); d++) {
                Decor decor = edge.getDecor(d);
                if (!decor.isVisible(viewProfile))
                    continue;
                decor.getEffectivePos(gpbPoint);
                gpbRect.setRect(gpbPoint.getX(), gpbPoint.getY(), decor.getWidth(), decor.getHeight());
                boundRect = unionRect(boundRect, gpbRect);
            }
        }
        if (boundRect == null)
            return new Rectangle2D.Double(0,0,2*dx,2*dy);
        boundRect.setRect(boundRect.getX() - dx, boundRect.getY() - dy, 
                          boundRect.getWidth() + 2*dx, 
                          boundRect.getHeight() + 2*dy);
        //System.out.println(getPageName()+" boundRect = "+boundRect);
        return boundRect;
    }
    
    public Rectangle2D getPageBounds() {
        Rectangle2D bounds = getOrComputeBoundsOfContent();
        return new Rectangle2D.Double(bounds.getX(), bounds.getY(), 
                                      bounds.getWidth(), bounds.getHeight());
    }
    
    public Rectangle2D computeIntegerPageBounds() {
        Rectangle2D bounds = computePageBounds(0, 0);
        bounds.setFrame((int)Math.floor(bounds.getX()), 
                        (int)Math.floor(bounds.getY()),
                        (int)Math.ceil(bounds.getWidth()),
                        (int)Math.ceil(bounds.getHeight()));
        return bounds;
    }
    
    public void selectionEnds() {
        viewProfile.singleSelObject = getSingleSelectedObject();
    }
    
    public void paintNet(Graphics2D g, DrawHelper dh) {
        // BACKGROUND NODES
        for (Node node : nodes) {
            if (node.isBackgroundNode())
                node.paintNodeShadow(g, dh);
        }
        for (Node node : nodes) {
            if (node.isBackgroundNode())
                node.paintNode(g, dh, false);
        }
        // SHADOWS OF FOREGROUND NODES
        for (Node node : nodes) {
            if (!node.isBackgroundNode())
                node.paintNodeShadow(g, dh);
        }
        // EDGES
        for (Edge edge : edges) {
            edge.paintEdge(g, dh);
        }
        // FOREGROUND NODES
        for (Node node : nodes) {
            if (!node.isBackgroundNode())
                node.paintNode(g, dh, false);
        }
    }
    
    // ---- Flag management -------
    
    public void setAuraFlag(boolean isAuraActive) {
        for (Node node : nodes)
            node.setAuraActive(isAuraActive);
        for (Edge edge : edges)
            edge.setAuraActive(isAuraActive);
    }

    public void setGrayedFlag(boolean isGrayed) {
        for (Node node : nodes)
            node.setGrayed(isGrayed);
        for (Edge edge : edges)
            edge.setGrayed(isGrayed);
    }
    
    @Override
    public void setSelectionFlag(boolean isSelected) {
        for (Node node : nodes)
            node.setSelected(isSelected);
        for (Edge edge : edges)
            edge.setSelected(isSelected);
    }
    
    public void setCopiedFlag(boolean isCopied) {
        for (Node node : nodes)
            node.setCopied(isCopied);
        for (Edge edge : edges)
            edge.setCopied(isCopied);
    }
    
    public void eraseCopied(boolean eraseIfCopiedFlagSet) {
        Iterator<Edge> eit = edges.iterator();
        while (eit.hasNext()) {
            Edge e = eit.next();
            if (e.isCopied() == eraseIfCopiedFlagSet) {
                eit.remove();
                continue;
            }
            boolean connectsWithErasedNode = false;
            if (e.getTailNode() != null && e.getTailNode().isCopied()==eraseIfCopiedFlagSet) {
                e.points.get(0).setLocation(e.getTailNode().getCenterX(), e.getTailNode().getCenterY());
                e.connectToNode(null, -1, Edge.EndPoint.TAIL);
                connectsWithErasedNode = true;
            }
            if (e.getHeadNode() != null && e.getHeadNode().isCopied()==eraseIfCopiedFlagSet) {
                e.points.get(e.numPoints()-1).setLocation(e.getHeadNode().getCenterX(), e.getHeadNode().getCenterY());
                e.connectToNode(null, -1, Edge.EndPoint.HEAD);
                connectsWithErasedNode = true;
            }
            
            if (connectsWithErasedNode && eraseIfCopiedFlagSet)
                eit.remove();
        }

        Iterator<Node> nit = nodes.iterator();
        while (nit.hasNext()) {
            Node n = nit.next();
            if (n.isCopied() == eraseIfCopiedFlagSet)
                nit.remove();
        }        
    }
    
    //------------ Cut/Copy/Paste ------------------------
    @Override
    public boolean canCutOrCopy() {
        int sel = countSelectedEdges() + countSelectedNodes();
        return sel > 0;
    }
    
    private static class NetObjectsInClipboard implements Serializable {
        public ArrayList<Node> nodes;
        public ArrayList<Edge> edges;
        public Map<UUID, ProjectResource> resources;

        public NetObjectsInClipboard(ArrayList<Node> nodes, ArrayList<Edge> edges,
                                     Map<UUID, ProjectResource> resources) 
        {
            this.nodes = nodes;
            this.edges = edges;
            this.resources = resources;
        }
    }

    @Override
    public Object copyData() {
        // Set copied flag to selected nodes/edges
        for (Node node : nodes)
            node.setCopied(node.isSelected());
        for (Edge edge : edges)
            edge.setCopied(edge.isSelected());
        
        NetPage copiedPage = (NetPage)Util.deepCopyRelink(this);
        copiedPage.eraseCopied(false);
        
        Map<UUID, ProjectResource> resources = new HashMap<>();
        copiedPage.retrieveLinkedResources(resources);
        
        NetObjectsInClipboard clip = new NetObjectsInClipboard(copiedPage.nodes, 
                                                                copiedPage.edges,
                                                                resources);
        
        return clip;
    }

    @Override
    public void eraseCutData(Object data) {
        eraseCopied(true);
    }

    @Override
    public void pasteData(Object data) {
        setSelectionFlag(false);
        NetObjectsInClipboard clip = (NetObjectsInClipboard)data;
        final double GRID_OFFSET = 2.0;
        for (Node n : clip.nodes) {
            String uniqueName = n.getUniqueName();
            while (null != getNodeByUniqueName(uniqueName)) {
                uniqueName = "copy_" + uniqueName;
            }
            n.setUniqueName(uniqueName);
            
            n.getCenterHandle(this).moveBy(GRID_OFFSET, GRID_OFFSET, true);
            n.relinkTransientResources(clip.resources);
            nodes.add(n);
        }
        
        for (Edge e : clip.edges) {
            e.getCenterHandle(this).moveBy(GRID_OFFSET, GRID_OFFSET, true);
            e.relinkTransientResources(clip.resources);
            edges.add(e);
        }
    }
    
    //-------------Clear measure cache interface--------------------
    @Override
    public boolean hasClearMeasureCmd() {
        return false;
    }
    @Override
    public boolean canClearMeasure(File projectFile) {
        throw new UnsupportedOperationException(); 
    }
    @Override
    public String clearMeasures(File projectFile, Component wnd) {
        throw new UnsupportedOperationException(); 
    }
    
    //---------------------Print support----------------------------
    @Override
    public boolean canPrint() {
        return true;
    }

    @Override
    public void print(Graphics2D g, PageFormat pf) {
        
        boolean[] selNodes = new boolean[nodes.size()], selEdges = new boolean[edges.size()];
        for (int n=0; n<nodes.size(); n++)
            selNodes[n] = nodes.get(n).isSelected();
        for (int e=0; e<edges.size(); e++)
            selEdges[e] = edges.get(e).isSelected();
        setSelectionFlag(false);
        
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                           RenderingHints.VALUE_ANTIALIAS_ON);
        g.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
                           RenderingHints.VALUE_INTERPOLATION_BICUBIC);
        g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
                           RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
                           RenderingHints.VALUE_FRACTIONALMETRICS_ON);
        g.setRenderingHint(RenderingHints.KEY_RENDERING,
                           RenderingHints.VALUE_RENDER_QUALITY);  
        
        Rectangle2D pageDim = computePageBounds(1, 1); //getPageDimension(new Dimension());
        
        g.translate(pf.getImageableX(), pf.getImageableY());
        double scaleX = pf.getImageableWidth() / pageDim.getWidth();
        double scaleY = pf.getImageableHeight() / pageDim.getHeight();
        double scaleFact = Math.min(scaleX, scaleY);
        final double MAX_SCALE_FACT = 13.0;
        scaleFact = Math.min(MAX_SCALE_FACT, scaleFact) * 0.9;
        double dx = pf.getImageableWidth() - pageDim.getWidth() * scaleFact;
        double dy = pf.getImageableHeight()- pageDim.getHeight() * scaleFact;
//        System.out.println("\ndx = "+dx+"  "+"dy="+dy);
//        System.out.println("pf.getImageableX() = "+pf.getImageableX()+"  pf.getImageableY() = "+pf.getImageableY());
//        System.out.println("pf.getWidth() = "+pf.getWidth()+"  pf.getHeight() = "+pf.getHeight());
//        System.out.println("pageDim.width = "+pageDim.getWidth()+"  pageDim.height = "+pageDim.getHeight());
//        System.out.println("scaleX = "+scaleX+"  scaleY = "+scaleY+"  scaleFact = "+scaleFact+
//                           " zoom = "+(100.0 / scaleFact));
//        g.transform(new AffineTransform(pf.getMatrix()));
        g.translate(dx / 2 -pageDim.getX() * scaleFact, 
                    dy / 2 -pageDim.getY() * scaleFact);
        g.scale(scaleFact, scaleFact);
        
        DrawHelper dh = new DrawHelper(g, viewProfile, 100.0 / scaleFact, 0, 0);
//        dh.baseTransform.translate(-pageDim.getX(), -pageDim.getY());
////        dh.baseTransform.scale(1/scaleFact, 1/scaleFact);
//        dh.baseTransform.scale(dh.logicToScreenFract(scaleFact), dh.logicToScreenFract(scaleFact));
//        System.out.println("dh.unitSize = "+dh.unitSize);
//        System.out.println("scaleFact = "+scaleFact);
//        System.out.println("dh.logicToScreenFract(scaleFact)="+dh.logicToScreenFract(scaleFact));
        paintNet(g, dh);
        
        
        // Restore selection flag
        for (int n=0; n<nodes.size(); n++)
            nodes.get(n).setSelected(selNodes[n]);
        for (int e=0; e<edges.size(); e++)
            edges.get(e).setSelected(selEdges[e]);
    }
    
    //----------------- Net composition interface ---------------------------
//    @Override
//    public Iterator<NodeGroup> groupIterator() {
//        Iterator it = nodes.iterator();
//        @SuppressWarnings("unchecked") Iterator<NodeGroup> nIt = (it);
//        return nIt;
//    }
//
//    @Override
//    public Iterator<ComposableNet> subnetsIterator() {
//        @SuppressWarnings("unchecked") Iterator<ComposableNet> cnIt
//                = common.EmptyIterator.INSTANCE;
//        return cnIt;
//    }
//
//    @Override
//    public int numNodeGroups() { 
//        return nodes.size();
//    }
//
//    @Override
//    public int numSubnets() {
//        return 0;
//    }

    @Override
    public Set<TemplateVariable> enumerateParamsForNetComposition() {
        assert isPageCorrect();
//        Set<TemplateVariable> params = new HashSet<>();
//        for (Node node : nodes)
//            if (node instanceof TemplateVariable)
//                params.add((TemplateVariable)node);
//        return params;

        // Get the parametric color class definitions, 
        // and extract which parameters are template
        Set<String> ccDepVarNames = new HashSet<>();
        Iterator<ColorClass> ccIter = colorClassIterator();
        if (ccIter!=null) {
            while (ccIter.hasNext()) {
                ColorClass cc = ccIter.next();
                cc.getDependentVariables(ccDepVarNames);
            }
        }
        Set<TemplateVariable> colorClassDepTemplateVars = new HashSet<>();
        for (Node node : nodes)
            if (node instanceof TemplateVariable && ccDepVarNames.contains(node.getUniqueName()))
                colorClassDepTemplateVars.add((TemplateVariable)node);
        return colorClassDepTemplateVars;
    }
    
    @Override
    public void instantiateParams(TemplateBinding binding) {
        for (int n=0; n<nodes.size(); n++) {
            if (nodes.get(n) instanceof TemplateVariable) {
                TemplateVariable tvar = (TemplateVariable)nodes.get(n);
                if (binding.binding.containsKey(tvar.getUniqueName())) {
                    // Replace tvar with a constant
                    ConstantID con = new ConstantID(tvar);
                    con.getConstantExpr().setExpr(binding.getSingleValueBoundTo(tvar).getExpr());
                    nodes.set(n, con);
                }
            }
        }
    }

    @Override
    public NetPage getComposedNet() {
        return this;
    }

    @Override
    public boolean hasSubnets() {
        return false;
    }
}
