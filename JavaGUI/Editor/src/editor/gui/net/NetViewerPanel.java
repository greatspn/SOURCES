/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.WrapLayout;
import editor.domain.DrawHelper;
import editor.domain.elements.DtaPage;
import editor.domain.Expr;
import editor.domain.elements.GspnPage;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import editor.domain.elements.TemplateVariable;
import editor.domain.grammar.ParserContext;
import editor.domain.grammar.TemplateBinding;
import editor.gui.MainWindowInterface;
import editor.gui.NoOpException;
import editor.gui.UndoableCommand;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridBagConstraints;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Stroke;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.Map;
import java.util.Map.Entry;
import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;
import latex.LatexFormula;

/** A viewer panel for a GSPN/DTA with the binding forms.
 *  This panel is used by both the Play panel and the semiflows panel
 *  to show the nets.
 *
 * @author elvio
 */
public abstract class NetViewerPanel extends JPanel implements Scrollable {

    final int WRAP = 5;
    WrapLayout wrapLayout;
    JScrollPane scrollPane;
    
    //private int commonZoomLevel = 100;

    public NetViewerPanel(JScrollPane scrollPane) {
        this.scrollPane = scrollPane;
        wrapLayout = new WrapLayout(0, WRAP, WRAP);
        wrapLayout.setAlignOnBaseline(true);
        setLayout(wrapLayout);
        setFocusable(false);
        setOpaque(false);
        scrollPane.addComponentListener(new ComponentAdapter() {
            @Override
            public void componentResized(ComponentEvent ce) {
                revalidate(); // Redo the layout of the components
            }
        });
    }
    
    // The nets (GSPN/DTA) that partecipate in the simulation game
    private NetPage[] pages;
    // The page that is used for evaluation (build parser contexts)
    private NetPage evalPage;
    // The binding of the template variables of the gspn and the dta.
    private TemplateBinding[] bindings;
    
    // Viewer profile for the the zoom level
    private ViewProfile viewProfile;
    
    // The panel that hosts the nets
    private JNetPanel[] netPanels;
    
    // Template variable forms
    private TemplateBindingForm[] bindingForms;
    int bindingFormsCount = 0; // Number of binding forms needed.
    boolean inBindingPhase = false;
    boolean showBindingForms = true;
    
    private final ActionListener allBindingsOkListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent ae) {
            assert inBindingPhase;
            allBindingsOk(areAllBindingsOk());
        }
    };
    
    public void initializePanel(GspnPage gspn, DtaPage dta, boolean showBindingForms) {
        NetPage[] nets;
        assert gspn != null;
        if (dta == null)
            nets = new NetPage[]{gspn};
        else
            nets = new NetPage[]{gspn, dta};
        initializePanel(nets, null, gspn, gspn.viewProfile, showBindingForms);
    }

    public void initializePanel(NetPage[] pages, String[] pageNames, NetPage evalPage, /*gspn evaluation*/ 
                                ViewProfile vp, boolean showBindingForms) 
    {
        this.pages = pages;
        this.viewProfile = vp;
        this.showBindingForms = showBindingForms;
        this.evalPage = evalPage;
        // Prepare the binding forms
        bindingFormsCount = 0;
        boolean allBindingsOk = true;
        
        if (showBindingForms) {
            bindings = new TemplateBinding[pages.length];
            bindingForms = new TemplateBindingForm[pages.length];
            for (int i=0; i<pages.length; i++) {
                assert pages[i] != null;
                TemplateBinding tb = pages[i].getListOfTemplateVariables();
                bindings[i] = tb;
                if (!tb.binding.isEmpty()) {
                    TemplateBindingForm tbform = new TemplateBindingForm();
                    bindingForms[i] = tbform;
                    tbform.initializeFor(tb, pages[i], evalPage);
                    GridBagConstraints gbc = new GridBagConstraints(0, bindingFormsCount, 1, 1, 
                            1.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
                            new Insets(10, 10, 0, 10), 0, 0);
                    tbform.addActionListener(allBindingsOkListener);
                    bindingFormsCount++;
                    allBindingsOk = allBindingsOk && tbform.areAllBindingsValid();
                }
            }
        }
        inBindingPhase = true;
        
        netPanels = new JNetPanel[pages.length];
        for (int i=0; i<pages.length; i++) {
            if (pages[i] instanceof DtaPage) {
                pages[i].setSelectionFlag(false);
                pages[i].setGrayedFlag(false);
                pages[i].setAuraFlag(false);
            }
            String panelTitle = (pageNames==null) ? pages[i].getPageName() : pageNames[i];
            netPanels[i] = new JNetPanel(pages[i], panelTitle);
            TemplateBindingForm tbf = (bindingForms==null) ? null : bindingForms[i];
            this.add(new JNetPanelContainer(netPanels[i], tbf));
        }
        setSize(getPreferredSize());
    }
    
    public boolean areAllBindingsOk() {
        boolean allBindingsOk = true;
        if (bindingForms != null) {
            for (TemplateBindingForm tbform : bindingForms)
                if (tbform != null)
                    allBindingsOk = allBindingsOk && tbform.areAllBindingsValid();
        }
        return allBindingsOk;
    }
    
    public void closeBindingForm() {
        assert inBindingPhase;
        if (bindingForms != null)
            for (TemplateBindingForm tbform : bindingForms)
                if (tbform != null)
                    tbform.setEnabled(false);
        for (JNetPanel panel : netPanels)
            panel.setEnabled(true);

        // Resize the net panels...
        this.invalidate();
        this.revalidate();
        
        // Evaluate all the bindings
        if (bindings != null) {
            for (int i=0; i<pages.length; i++) {
                if (bindings[i] != null) {
                    ParserContext context = new ParserContext(evalPage); // NOTE: gspn and not page!
                    for (Map.Entry<String, Expr> entry : bindings[i].binding.entrySet()) {
                        entry.getValue().checkExprCorrectness(context, pages[i], null);
                    }
                }
                // TODO: what happens if page now has errors?
            }
        }
        inBindingPhase = false;
    }
    
    public void reopenBindingForms() {
        inBindingPhase = true;
        
        if (bindingFormsCount > 0) {
            // Open the binding forms
            if (bindingForms != null)
                for (TemplateBindingForm tbform : bindingForms)
                    if (tbform != null)
                        tbform.setEnabled(true);
            for (JNetPanel panel : netPanels)
                panel.setEnabled(false);
        }
    }
    
    public void disablePanel() {
        pages = null;
        evalPage = null;
        inBindingPhase = false;
    }
    
    public void removeAllNetPanels() {
        this.removeAll();
        disablePanel();
    }
    
    public boolean isInBindingPhase() {
        return inBindingPhase;
    }

    public GspnPage getGspn() {
        assert pages[0] instanceof GspnPage;
        return (GspnPage)pages[0];
    }

    public DtaPage getDta() {
        if (pages.length == 1)
            return null; // No DTA
        assert pages[1] instanceof DtaPage;
        return (DtaPage)pages[1];
    }

    public TemplateBinding getGspnBinding() {
        assert pages[0] instanceof GspnPage;
        return (bindings==null) ? null : bindings[0];
    }

    public TemplateBinding getDtaBinding() {
        if (pages.length == 1)
            return null; // No DTA
        assert pages[1] instanceof DtaPage;
        return (bindings==null) ? null : bindings[1];
    }
    
    public boolean hasBindingForms() {
        return bindingFormsCount > 0;
    }
    
    public int getZoomLevel() {
        return viewProfile.zoom;
    }
    
    public void zoomChanged(int newZoomLevel) {
//        System.out.println("zoomChanged "+newZoomLevel+"   old="+commonZoomLevel);
        if (newZoomLevel != viewProfile.zoom) {
            viewProfile.zoom = newZoomLevel;
            //allNetsPanel.revalidate();
            for (JNetPanel panel : netPanels)
                panel.resizeToAccomodate();;
        }
        setSize(getPreferredSize());
    }
    
    public void saveBindingParameters(MainWindowInterface mainInterface, final GspnPage origGspn, final DtaPage origDta) {
        // Save the template bindings
        mainInterface.executeUndoableCommand("Update template parameter assignments.", new UndoableCommand() {
            @Override public void Execute(ProjectData proj, ProjectPage page) throws Exception {
                boolean changed = false;
                if (origGspn != null && getGspnBinding() != null) {
                    for (Entry<String, Expr> entry : getGspnBinding().binding.entrySet()) {
                        TemplateVariable tvar = (TemplateVariable)origGspn.getNodeByUniqueName(entry.getKey());
                        if (!tvar.getLastBindingExpr().getExpr().equals(entry.getValue().getExpr())) {
                            tvar.getLastBindingExpr().setExpr(entry.getValue().getExpr());
                            changed = true;
                        }
                    }
                }
                if (origDta != null && getDtaBinding() != null) {
                    for (Entry<String, Expr> entry : getDtaBinding().binding.entrySet()) {
                        TemplateVariable tvar = (TemplateVariable)origDta.getNodeByUniqueName(entry.getKey());
                        if (!tvar.getLastBindingExpr().getExpr().equals(entry.getValue().getExpr())) {
                            tvar.getLastBindingExpr().setExpr(entry.getValue().getExpr());
                            changed = true;
                        }
                    }
                }
                if (!changed)
                    throw new NoOpException();
            }
        });
    }
    
    // Scrollable interface implementation

    @Override
    public Dimension getPreferredScrollableViewportSize() {
        return getPreferredSize();
    }

    @Override
    public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) {
        return (int) (12 * Math.max(1.0, viewProfile.zoom / 100));
    }

    @Override
    public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction) {
        if (orientation == SwingConstants.HORIZONTAL) {
            return (int) visibleRect.getWidth();
        }
        return (int) visibleRect.getHeight();
    }

    @Override
    public boolean getScrollableTracksViewportWidth() {
        return getWidth() <= scrollPane.getViewport().getExtentSize().width;
    }

    @Override
    public boolean getScrollableTracksViewportHeight() {
        return false;
    }
    
    class JNetPanelContainer extends JPanel {
        JLabel title;
        JPanel header;
        JNetPanel panel;
        TemplateBindingForm bindingForm;

        public JNetPanelContainer(JNetPanel panel, TemplateBindingForm bindingForm) {
            this.panel = panel;
            this.bindingForm = bindingForm;
            setBorder(BorderFactory.createLineBorder(Color.GRAY));
            setLayout(new BorderLayout());
            
            title = new JLabel(panel.netName, panel.net.getPageIcon(), SwingConstants.LEFT);
            title.setBackground(TITLE_GRAY_BKGND);
            title.setBorder(BorderFactory.createMatteBorder(2, 10, 2, 2, TITLE_GRAY_BKGND));
            title.setOpaque(true);
            
            header = new JPanel();
            header.setBorder(BorderFactory.createMatteBorder(0, 0, 1, 0, Color.GRAY));
            header.setLayout(new GridLayout(1, 1, 5, 2));
            
            header.add(title);
            add(header, BorderLayout.NORTH);
            
            if (bindingForm != null)
                add(bindingForm, BorderLayout.CENTER);
            add(panel, BorderLayout.SOUTH);
        }
    }
    
    public abstract void netClicked(JNetPanel panel, NetPage net, MouseEvent evt);
    
    public abstract String getOverlayMessage(NetPage net);
    
    public abstract void setupDrawContext(NetPage net, DrawHelper dh);
    
    public abstract void allBindingsOk(boolean allOk);
    
    public abstract Color getNetBackground(String overlayMsg);
    
    public final Color VERY_LIGHT_GRAY_BKGND = new Color(240, 240, 240);
    public final Color TITLE_GRAY_BKGND = new Color(224, 224, 230);
    
    public class JNetPanel extends JPanel {
        NetPage net;
        String netName;
        public Rectangle2D pageBounds;
        Point2D mousePt = new Point2D.Double();

        public JNetPanel(NetPage _net, String _netName) {
            this.net = _net;
            this.netName = _netName;
            setBackground(Color.WHITE);
            setFocusable(true);
            setOpaque(true);
            pageBounds = net.getPageBounds();
            resizeToAccomodate();
            
            addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent me) {
                    netClicked(JNetPanel.this, net, me);
                }
            });
        }
        
        // Resize to accomodate the document
        public final void resizeToAccomodate() {
            Dimension pageSize = new Dimension(NetObject.logicToScreen(pageBounds.getWidth(), viewProfile.zoom),
                                               NetObject.logicToScreen(pageBounds.getHeight(), viewProfile.zoom));
            setPreferredSize(pageSize);
            setSize(pageSize);
            setMinimumSize(pageSize);
            setMaximumSize(pageSize);
            revalidate();
        }
        
        @Override
        protected void paintComponent(Graphics g) {
//            System.out.println("NetEditor.paintComponent "+paintCount++);
            assert net != null;
            
            String overlayMsg = getOverlayMessage(net);
            setBackground(getNetBackground(overlayMsg));
//            setBackground(isEnabled() && overlayMsg==null ? Color.WHITE : VERY_LIGHT_GRAY_BKGND);
            super.paintComponent(g); 
            
            // Render the page content
            Graphics2D g2 = (Graphics2D)g;
            g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                                RenderingHints.VALUE_ANTIALIAS_ON);
            g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
                                RenderingHints.VALUE_INTERPOLATION_BICUBIC);
            g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
                                RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
            g2.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
                                RenderingHints.VALUE_FRACTIONALMETRICS_ON);
//            g2.setRenderingHint(RenderingHints.KEY_RENDERING,
//                                RenderingHints.VALUE_RENDER_QUALITY);
            DrawHelper dh = new DrawHelper(g2, net.viewProfile, viewProfile.zoom, 
                                           -pageBounds.getX(), -pageBounds.getY());
            
            setupDrawContext(net, dh);

            // Setup the logic->screen coordinate system
            AffineTransform oldAT = g2.getTransform();
            g2.setTransform(dh.logic2screen);
                        
            // Paint the net and the token game overlays
            net.paintNet(g2, dh);
            
            // Draw the overlay message over the DTA net.
            if (overlayMsg != null) {
                LatexFormula latex = new LatexFormula(overlayMsg, 1.2f * NetObject.getUnitToPixels());
                g2.setColor(Color.WHITE);
                double w = latex.getWidth() / (double)NetObject.getUnitToPixels();
                double h = latex.getHeight() / (double)NetObject.getUnitToPixels();
                Rectangle2D rect = new Rectangle2D.Double(pageBounds.getCenterX() - w/2 - 0.3, 
                                                          pageBounds.getCenterY() - h/2 - 0.3, w + 0.6, h + 0.6);
                g2.fill(rect);
                g2.setColor(Color.BLACK);
                Stroke oldSh = g2.getStroke();
                g2.setStroke(DrawHelper.StrokeWidth.BASIC.logicStroke);
                g2.draw(rect);
                g2.setStroke(oldSh);
                latex.draw(g2,  pageBounds.getCenterX() - w/2, pageBounds.getCenterY() - h/2,
                           1.0 / NetObject.getUnitToPixels(), false);
            }
            
            g2.setTransform(oldAT);
        }
    }
}
