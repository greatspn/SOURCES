/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import common.Util;
import static common.Util.UIscaleFactor;
import editor.domain.DrawHelper;
import editor.domain.elements.GspnPage;
import editor.domain.NetObject;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.PageErrorWarning;
import editor.domain.elements.Place;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.domain.Selectable;
import editor.gui.AbstractPageEditor;
import editor.gui.MainWindowInterface;
import editor.gui.ResourceFactory;
import editor.gui.SharedResourceProvider;
import editor.gui.net.NetEditorPanel;
import editor.gui.net.NetViewerPanel;
import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import javax.swing.DefaultListCellRenderer;
import javax.swing.DefaultListModel;
import javax.swing.Icon;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.DefaultTableModel;

/** A panel that shows the measures of a Petri net (mean tokens in a place,
 *  token distributions, transition throughputs).
 *
 * @author elvio
 */
public class PetriNetMeasuresPanel extends javax.swing.JPanel 
                                   implements AbstractPageEditor
{
    // The GSPN reference and its viewer
    private GspnPage gspn;
    private PetriNetMeasureViewer viewerPanel;
    
    private MainWindowInterface mainInterface;
    private MeasurePage currPage;
    
    // Shown measures
    private FormulaMeasure allFormula;
    private AllMeasuresResult[] measures;
    
    // Table of variable bindings
    private JTable tableVars;
    
    /**
     * Creates new form PetriNetMeasuresPanel
     * @param shActProv the set of shared actions.
     */
    public PetriNetMeasuresPanel(SharedResourceProvider shActProv) {
        initComponents();
        Util.reformatPanelsForPlatformLookAndFeel(this);
        
        jScrollPaneNet.getViewport().setOpaque(true);
        jScrollPaneNet.getViewport().setBackground(NetEditorPanel.OUTSIDE_PAGE_COLOR);
    }

    //--------------------------------------------------------------------------

    @Override
    public void setEditorEnabledFor(ProjectFile pf, ProjectPage page, MainWindowInterface _mainInterface) {
        assert page instanceof MeasurePage;
        currPage = (MeasurePage)page;
        assert currPage.targetGspn != null;
        this.mainInterface = _mainInterface;
        
        // Retrive the only 'ALL' measure entry in the measure table of this page
        for (AbstractMeasure am : currPage.measures)
            if (am instanceof FormulaMeasure &&
                ((FormulaMeasure)am).getLanguage() == FormulaLanguage.ALL) {
                allFormula = (FormulaMeasure)am;
                measures = new AllMeasuresResult[allFormula.getResults().table.size()];
                measures = allFormula.getResults().table.toArray(measures);
                break;
            }
        
        assert allFormula != null && measures != null;
        
        gspn = (GspnPage)Util.deepCopyRelink(currPage.targetGspn);
        gspn.preparePageCheck();
        gspn.checkPage(null, null, null, null);
        gspn.setSelectionFlag(false);
        
        
        viewerPanel = new PetriNetMeasureViewer(jScrollPaneNet);
        viewerPanel.initializePanel(gspn, null, false /* never show binding forms */);
        jScrollPaneNet.setViewportView(viewerPanel);
        
        // Prepare table columns
        String[] colNames = new String[1 + currPage.varListAssignments.assigns.size()];
        colNames[0] = "";
        for (int i=0; i<currPage.varListAssignments.assigns.size(); i++)
            colNames[i+1] = currPage.varListAssignments.assigns.get(i).varName;
        
        // Prepare table data
        Object[][] data = new Object[measures.length][colNames.length];
        for (int r=0; r<measures.length; r++) {
            data[r][0] = ResourceFactory.getInstance().getEmpty16();
            for (int c=0; c<currPage.varListAssignments.assigns.size(); c++) {
                String varName = currPage.varListAssignments.assigns.get(c).varName;
                data[r][c+1] = measures[r].assign.binding.get(varName);
            }
        }
        
        DefaultTableModel model = new DefaultTableModel(data, colNames) {
            @Override public boolean isCellEditable(int row, int column) {
                return false;
            }
        };
        tableVars = new JTable(model) {

            @Override
            public Class<?> getColumnClass(int column) {
                if (column == 0)
                    return Icon.class;
                return String.class;
            }
        };
        tableVars.setPreferredScrollableViewportSize(tableVars.getPreferredSize());
        tableVars.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        if (measures.length > 0)
            tableVars.getSelectionModel().setSelectionInterval(0, 0);
        tableVars.getSelectionModel().addListSelectionListener(new ListSelectionListener() {
            @Override
            public void valueChanged(ListSelectionEvent e) {
//                System.out.println("table selection changed!! "+e);
                mainInterface.invalidateGUI();
            }
        });
        tableVars.getColumnModel().getColumn(0).setWidth(ResourceFactory.getInstance().getEmpty16().getIconWidth()+4);
        tableVars.getColumnModel().getColumn(0).setMaxWidth(ResourceFactory.getInstance().getEmpty16().getIconWidth()+4);
        tableVars.getColumnModel().getColumn(0).setMinWidth(ResourceFactory.getInstance().getEmpty16().getIconWidth()+4);
        for (int i=1; i<tableVars.getColumnCount() - 1; i++) {
            tableVars.getColumnModel().getColumn(i).sizeWidthToFit();
        }
        jScrollPaneTableVars.setViewportView(tableVars);
        
        jScrollPaneTableVars.setVisible(!currPage.varListAssignments.assigns.isEmpty());
        jLabel_TemplateVar.setVisible(!currPage.varListAssignments.assigns.isEmpty());
    }

    @Override
    public void setEditorDisabled() {
        currPage = null;
        mainInterface = null;
        measures = null;
        allFormula = null;
        gspn = null;
        viewerPanel.disablePanel();
        viewerPanel = null;
        jScrollPaneNet.setViewportView(null);
        tableVars = null;
        jScrollPaneTableVars.setViewportView(null);
    }

    @Override
    public void updateGUI(ProjectFile pf, ProjectPage page) {
        viewerPanel.repaint();
        
        int numSel = gspn.countSelectedNodes();
        if (numSel == 1) {
            int numVars = tableVars.getSelectedRowCount();
            Selectable sel = gspn.getSingleSelectedObject();
            if (sel instanceof Place) {
                Place place = (Place)sel;
                jPanelPlaceDistrib.setVisible(true);
                jPanelNoSel.setVisible(false);
                
                jLabel_placeName.setText("");
                jLabel_placeName.setIcon(place.getUniqueNameDecor().getLatexFormula().getAsIcon(UIscaleFactor));
                if (numVars == 1) {
                    int row = tableVars.getSelectedRow();
                    jLabel_avgTokens.setText(measures[row].getMeanTokensInPlace(place.getUniqueName()).toString());
                    final ComputedScalar[] distrib = measures[row].getTokenDistribOfPlace(place.getUniqueName());
                    if (distrib != null) {
                        final String[] strDistrib = new String[distrib.length];
                        for (int i=0; i<distrib.length; i++)
                            strDistrib[i] = (i<100?"0":"")+(i<10?"0":"")+i+":  "+distrib[i];
                        listDistrib.setModel(new DefaultListModel<String>() {
                            @Override public int getSize() { return distrib.length; }
                            @Override public String getElementAt(int index) { return strDistrib[index]; }
                        });
                        listDistrib.setCellRenderer(new DefaultListCellRenderer() {
                            int index;
                            boolean isSelected;
                            @Override
                            protected void paintComponent(Graphics g) {
                                // Paint custom background
                                Rectangle rect = getBounds();
                                Color oldColor = g.getColor();
                                g.setColor(getBackground());
                                g.fillRect(rect.x, 0, rect.width, rect.height);
                                final int border = 2;
                                int x1 = rect.x + 2*rect.height - border;
                                int x2 = rect.x + rect.width - border;
                                int xmin = (int)NetObject.linearInterp(x1, x2, distrib[index].getMin());
                                int xavg = (int)NetObject.linearInterp(x1, x2, distrib[index].getAverage());
                                int xmax = (int)NetObject.linearInterp(x1, x2, distrib[index].getMax());
    //                            System.out.println("paintComponent "+index+" x1="+x1+" x2="+x2+" x3="+x3+" y="+rect.y+" opaque="+isOpaque());
                                g.setColor(isSelected ? BAR_COLOR_SEL : BAR_COLOR);
                                g.fillRect(x1, 0+border, xmin-x1, rect.height-2*border);
                                if (xmax != xmin) { // not an exact value, draw the confidence interval
                                    g.setColor(isSelected ? BAR_COLOR_CONFINTERV_SEL : BAR_COLOR_CONFINTERV);
                                    g.fillRect(xmin, 0+border, xmax-xmin, rect.height-2*border);
                                    g.setColor(isSelected ? VBAR_COLOR_SEL : VBAR_COLOR);
                                    g.fillRect(xavg, 0+border, 1, rect.height-2*border);
                                }
                                g.setColor(oldColor);

                                Color bkgnd = getBackground();
                                setBackground(null);
                                setOpaque(false);
                                super.paintComponent( g );
                                setOpaque(true);
                                setBackground(bkgnd);
                            }

                            @Override
                            public Component getListCellRendererComponent(JList<?> list, Object value, int index, 
                                                                          boolean isSelected, boolean cellHasFocus) 
                            {
    //                            System.out.println("getListCellRendererComponent "+index);
                                this.index = index;
                                this.isSelected = isSelected;
                                Component comp = super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus); 
                                ((JLabel)comp).setOpaque(true);
                                return comp;
                            }

                        });
                    }
                    else {
                        // Place distribution is missing (like for WNSIM)
                        listDistrib.setModel(new DefaultListModel<String>());
                    }
                }
                else {
                    jLabel_avgTokens.setText("---");
                    listDistrib.setModel(new DefaultListModel<String>());
                }
            }
            else {
                jPanelPlaceDistrib.setVisible(false);
                jPanelNoSel.setVisible(true);
            }
        }
        else {
            jPanelPlaceDistrib.setVisible(false);
            jPanelNoSel.setVisible(true);
        }
    }
    
    private static final Color BAR_COLOR = new Color(175, 220, 255);
    private static final Color BAR_COLOR_SEL = new Color(0, 130, 180);

    private static final Color BAR_COLOR_CONFINTERV = new Color(210, 240, 255);
    private static final Color BAR_COLOR_CONFINTERV_SEL = new Color(0, 170, 235);

    private static final Color VBAR_COLOR = new Color(100, 100, 100);
    private static final Color VBAR_COLOR_SEL = new Color(200, 200, 200);

    //--------------------------------------------------------------------------
    @Override
    public void updateEnablingOfSharedActions(SharedResourceProvider shResProv) {
        for (SharedResourceProvider.ActionName actName : SharedResourceProvider.ActionName.values()) {
            common.Action act = shResProv.getSharedAction(actName);
            
            switch (actName) {
                default:
                    act.setEnabled(false);
            }
        }    
    }

    @Override
    public void sharedActionListener(SharedResourceProvider.ActionName shAction, ActionEvent event) {
        switch (shAction) {
            default:
                throw new UnsupportedOperationException();
        }
    }

    //--------------------------------------------------------------------------

    @Override
    public boolean isModalEditor() {
        return true;
    }

    @Override
    public JComponent getEditorComponent() {
        return panelNet;
    }

    @Override
    public JComponent getFocusTarget() {
        if (jScrollPaneTableVars.isVisible())
            return tableVars;
        return panelNet;
    }

    @Override
    public JComponent getPropertyPanel() {
        return this;
    }

    @Override
    public JComponent[] getToolbars() {
        return new JComponent[]{};
    }

    private static final PageErrorWarning[] emptyErrorList = new PageErrorWarning[0];
    @Override
    public PageErrorWarning[] getAllErrorsAndWarnings() {
        return emptyErrorList;
    }
    
    @Override
    public boolean isZoomPanelUsed() {
        return true;
    }

    @Override
    public int getPageZoom() {
        return viewerPanel.getZoomLevel();
    }

    @Override
    public void zoomChanged(int newZoomLevel) {
        viewerPanel.zoomChanged(newZoomLevel);
    }
    
    private class PetriNetMeasureViewer extends NetViewerPanel {
        Point2D mousePt = new Point2D.Double();
        public PetriNetMeasureViewer(JScrollPane scrollPane) {
            super(scrollPane);
        }

        @Override
        public void netClicked(JNetPanel panel, NetPage net, MouseEvent evt) {
            int zoom = viewerPanel.getZoomLevel();
            mousePt.setLocation(NetObject.screenToLogic(evt.getX(), zoom) + panel.pageBounds.getX(),
                                 NetObject.screenToLogic(evt.getY(), zoom) + panel.pageBounds.getY());
            // Search for an active node that can fire
            Rectangle2D hitRect = NetObject.makeHitRectangle(mousePt, zoom);
            for (Node n : net.nodes) {
                if (n instanceof Place && n.intersectRectangle(hitRect, net.viewProfile, true))
                {
                    if (!evt.isShiftDown())
                        gspn.setSelectionFlag(false);
                    n.setSelected(!n.isSelected());
                    mainInterface.invalidateGUI();
                }
            }
        }

        @Override
        public String getOverlayMessage(NetPage net) {
            if (tableVars.getSelectedRowCount()== 0)
                return "\\text{Select a variable binding.}";
            return null;
        }

        @Override
        public void setupDrawContext(NetPage net, DrawHelper dh) {
            if (tableVars.getSelectedRowCount() == 1) {
                dh.measures = measures[tableVars.getSelectedRow()];
            }
        }

        @Override
        public void allBindingsOk(boolean allOk) {
            throw new IllegalStateException("Should not reach this method");
        }

        @Override
        public Color getNetBackground(String overlayMsg) {
            if (tableVars.getSelectedRowCount()== 0)
                return editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_DISABLED_COLOR;
            return editor.gui.net.NetEditorPanel.PAGE_BACKGROUND_COLOR;
        }
    }
    

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        panelNet = new javax.swing.JPanel();
        jScrollPaneNet = new javax.swing.JScrollPane();
        resourceFactory = new editor.gui.ResourceFactory();
        jLabel_TemplateVar = new javax.swing.JLabel();
        jScrollPaneTableVars = new javax.swing.JScrollPane();
        jLabel_Distrib = new javax.swing.JLabel();
        jPanelPlaceDistrib = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jLabel_placeName = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        jLabel_avgTokens = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        listDistrib = new javax.swing.JList<>();
        jPanelNoSel = new javax.swing.JPanel();

        panelNet.setLayout(new java.awt.GridLayout(1, 0));
        panelNet.add(jScrollPaneNet);

        setLayout(new java.awt.GridBagLayout());

        jLabel_TemplateVar.setIcon(resourceFactory.getMultiBindings32());
        jLabel_TemplateVar.setText("Variables binding:");
        jLabel_TemplateVar.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 1, 1);
        add(jLabel_TemplateVar, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.3;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 4, 0);
        add(jScrollPaneTableVars, gridBagConstraints);

        jLabel_Distrib.setIcon(resourceFactory.getDistrib32());
        jLabel_Distrib.setText("Distribution:");
        jLabel_Distrib.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 1, 1);
        add(jLabel_Distrib, gridBagConstraints);

        jPanelPlaceDistrib.setLayout(new java.awt.GridBagLayout());

        jLabel1.setText("Place: ");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 6, 3, 3);
        jPanelPlaceDistrib.add(jLabel1, gridBagConstraints);

        jLabel_placeName.setText("XYZ");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelPlaceDistrib.add(jLabel_placeName, gridBagConstraints);

        jLabel2.setText("Average token count:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.insets = new java.awt.Insets(3, 6, 3, 3);
        jPanelPlaceDistrib.add(jLabel2, gridBagConstraints);

        jLabel_avgTokens.setText("NN");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelPlaceDistrib.add(jLabel_avgTokens, gridBagConstraints);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder("Token distribution:"));
        jPanel2.setLayout(new javax.swing.BoxLayout(jPanel2, javax.swing.BoxLayout.LINE_AXIS));

        listDistrib.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        jScrollPane1.setViewportView(listDistrib);

        jPanel2.add(jScrollPane1);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelPlaceDistrib.add(jPanel2, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.2;
        add(jPanelPlaceDistrib, gridBagConstraints);

        javax.swing.GroupLayout jPanelNoSelLayout = new javax.swing.GroupLayout(jPanelNoSel);
        jPanelNoSel.setLayout(jPanelNoSelLayout);
        jPanelNoSelLayout.setHorizontalGroup(
            jPanelNoSelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 285, Short.MAX_VALUE)
        );
        jPanelNoSelLayout.setVerticalGroup(
            jPanelNoSelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 31, Short.MAX_VALUE)
        );

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.2;
        add(jPanelNoSel, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel_Distrib;
    private javax.swing.JLabel jLabel_TemplateVar;
    private javax.swing.JLabel jLabel_avgTokens;
    private javax.swing.JLabel jLabel_placeName;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanelNoSel;
    private javax.swing.JPanel jPanelPlaceDistrib;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPaneNet;
    private javax.swing.JScrollPane jScrollPaneTableVars;
    private javax.swing.JList<String> listDistrib;
    private javax.swing.JPanel panelNet;
    private editor.gui.ResourceFactory resourceFactory;
    // End of variables declaration//GEN-END:variables
}
