/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import common.Condition;
import common.LogWindow;
import common.ModalLogDialog;
import editor.Main;
import common.Util;
import static common.Util.isOSX;
import static editor.Main.logException;
import editor.domain.PageErrorWarning;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage.PlayCommand;
import editor.domain.ProjectPage.RgType;
import editor.domain.elements.ClockVar;
import editor.domain.elements.DtaPage;
import editor.domain.elements.GspnPage;
import editor.domain.grammar.TemplateBinding;
import editor.domain.semiflows.SemiFlows;
import editor.domain.io.GreatSpnFormat;
import editor.domain.io.PNMLFormat;
import editor.domain.io.PnProFormat;
import editor.domain.measures.FormulaLanguage;
import editor.domain.measures.FormulaMeasure;
import editor.domain.measures.MeasureEditorPanel;
import editor.domain.measures.MeasurePage;
import editor.domain.superposition.MultiNetEditorPanel;
import editor.domain.superposition.MultiNetPage;
import editor.domain.superposition.NetInstanceDescriptor;
import editor.domain.unfolding.Unfolding;
import editor.gui.net.BaseCellEditor;
import editor.gui.net.NetSemiflowsPanel;
import editor.gui.net.NetViewerPanel;
import editor.gui.net.ParameterAssignmentDialog;
import editor.gui.net.ShowRgDialog;
import java.awt.Color;
import java.awt.Component;
import java.awt.Desktop;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.Taskbar;
import java.awt.desktop.QuitEvent;
import java.awt.desktop.QuitHandler;
import java.awt.desktop.QuitResponse;
import java.awt.desktop.QuitStrategy;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.geom.Point2D;
import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;
import java.io.File;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.EventObject;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;
import java.util.Set;
import java.util.Stack;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.UUID;
import java.util.prefs.Preferences;
import javax.swing.ActionMap;
import javax.swing.BoxLayout;
import javax.swing.DefaultCellEditor;
import javax.swing.DropMode;
import javax.swing.Icon;
import javax.swing.InputMap;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.border.TitledBorder;
import javax.swing.event.CellEditorListener;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeExpansionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.plaf.basic.BasicScrollBarUI;
import javax.swing.tree.DefaultTreeCellEditor;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeCellEditor;
import javax.swing.tree.TreePath;
import latex.LatexProvider;

/**
 *
 * @author Elvio
 */
public final class AppWindow extends javax.swing.JFrame implements MainWindowInterface, QuitHandler {

    protected ZoomPanel jZoomPanel;
    // The list of open projects
    protected ArrayList<ProjectFile> projects = new ArrayList<>();
    // Current 'active' project
    protected ProjectFile activeProject = null;
    
    // The currently visible editor
    protected AbstractPageEditor activeEditor = null;
            
    // Updating status (if is calling updateGUI())
    public enum GuiStatus { UPDATED, INVALID, UPDATING };
    private GuiStatus guiStatus = GuiStatus.INVALID;
    
    // is the GUI suspended (by a cell editor)?
    boolean isGuiSuspended = false;

    // Transfer Cut/Copy/Paste actions to the currently focused component
    private CutCopyPasteEngine ccpEngine;
    
    // List of recently opened files
    private RecentFilesListLRU recentFilesList;

    // Action activation condition
    Condition firingCondition = new Condition();
    
    @Override
    public void invalidateGUI() {
//        System.out.println("invalidateGUI()");
//        try{throw new Exception(); } catch (Exception e) { e.printStackTrace(System.out); System.out.println("\n");}
        if (guiStatus != GuiStatus.INVALID) {
            guiStatus = GuiStatus.INVALID;
            SwingUtilities.invokeLater(guiUpdater);
        }
    }
    private Runnable guiUpdater = new Runnable() {
        @Override
        public void run() {
//            System.out.println("updateGUI() invoked later.");
            updateGUI();
            assert (guiStatus == GuiStatus.UPDATED); // Could be invalidated in the meanwhile?
        }
    };
    @Override public boolean isGUIvalid() { return guiStatus == GuiStatus.UPDATED; }
    
    // All known and instantiated page editors - the key is the editor class or the editor instance
    private Map<Object, AbstractPageEditor> knownEditors = new HashMap<>();
    
    private void registerKnownEditor(Object key, AbstractPageEditor ape) {
        knownEditors.put(key, ape);

        ape.getEditorComponent().setVisible(false);
        jPanelCenter.add(ape.getEditorComponent());
        
        ape.getToolbar().setVisible(false);
        jPanelToolBars.add(ape.getToolbar());
        
        ape.getPropertyPanel().setVisible(false);
        if (ape.isModalEditor())
            jLeftPanel.add(ape.getPropertyPanel());
        else
            jPanelProperty.add(ape.getPropertyPanel());
    }
    
    // Change the current project, project page and its editor
    @Override
    public void switchToProjectPage(ProjectFile pf, ProjectPage page, AbstractPageEditor modalPanel) {
        assert modalPanel == null || (pf != null && page != null);
        assert modalPanel == null || modalPanel.isModalEditor();
//        System.out.println("switch to project="+(pf==null?"null":pf.getCurrent().projName)+
//                           " page="+(page==null?"null":page.getPageName())+"  modalPanel="+
//                           (modalPanel==null?"null":modalPanel.getClass().getName()));
        if (modalPanel != null && !knownEditors.containsKey(modalPanel)) {
            registerKnownEditor(modalPanel, modalPanel);
        }
        AbstractPageEditor newEditor = modalPanel;
        UUID newNodeId;
//        boolean samePage = false;
        if (newEditor == null && pf != null) {
            // Determine the new editor for the current project page
//            samePage = (pf.getCurrent().getActivePage() == page);
            pf.getCurrent().setActivePage(page);
            if (page != null) {
                newNodeId = page.getObjectUUID();
                // show the page editor
                if (knownEditors.get(page.getEditorClass()) == null) {
                    try {
                        Class<?> editorClass = page.getEditorClass();
                        Constructor<?> constr = editorClass
                                .getConstructor(SharedResourceProvider.class, Condition.class);
                        AbstractPageEditor ape = (AbstractPageEditor)constr.newInstance(this, firingCondition);
                        registerKnownEditor(page.getEditorClass(), ape);
                    }
                    catch (Exception e) {
                        logException(e, true);
                        System.exit(-1);
                    }                    
                }
                newEditor = knownEditors.get(page.getEditorClass());
                assert newEditor != null;
            }
            else {
                newNodeId = pf.getObjectUUID();
                // show the project editor
                newEditor = null;
            }
        }
        else newNodeId = null;

        // Ensure that project tree is set to the current page
        selectedNodeId = newNodeId;
        boolean changeEditor = (activeEditor != newEditor);
        
        // change the page editor if requested - first disable the old editor
        if (changeEditor) {
            if (activeEditor != null) {
                activeEditor.getEditorComponent().setVisible(false);
                activeEditor.getToolbar().setVisible(false);
                activeEditor.getPropertyPanel().setVisible(false);
                activeEditor.setEditorDisabled();
                activeEditor = null;
            }
        }
        
        // Change the application state: current project and active editor.
        activeProject = pf;
        activeEditor = newEditor;
 
        // Activate the new editor
        if (activeEditor != null) {
            activeEditor.setEditorEnabledFor(pf, page, this);
            jZoomPanel.setZoom(activeEditor.getPageZoom());
            
            if (changeEditor) {
                // activate the new editor components.
                activeEditor.getEditorComponent().setVisible(true);
                activeEditor.getToolbar().setVisible(true);
                activeEditor.getPropertyPanel().setVisible(true);

                // Update Main GUI
                boolean isModal = activeEditor.isModalEditor();
                jAppToolBar_File.setVisible(!isModal);
                jAppToolBar_Edit.setVisible(!isModal);
                jAppToolBar_Basic.setVisible(!isModal);
                jAppToolBar_Advanced.setVisible(!isModal);
                jPlayToolBar.setVisible(isModal);
                jSplitPaneLeftV.setVisible(!isModal);
            }
            
//            if (jAppToolBar_Basic.isVisible() && !hasVisibleButtons(jAppToolBar_Basic))
//                jAppToolBar_Basic.setVisible(false);
//            if (jAppToolBar_Advanced.isVisible() && !hasVisibleButtons(jAppToolBar_Advanced))
//                jAppToolBar_Advanced.setVisible(false);
        }
    }
    
    protected void hideOrShowToolBarIfEmpty(JToolBar tb) {
        for (int i=0; i<tb.getComponentCount(); i++) {
            Component comp = tb.getComponent(i);
            if (comp instanceof JSeparator)
                continue; // ignore separators
            if (comp.isVisible()) {
                tb.setVisible(true);
                return; // The toolbar has enabled buttons; show it
            }
        }
        tb.setVisible(false); // No enabled buttons: hide the toolbar
    }

    protected boolean hasActiveProject() {
        return activeProject != null;
    }

    @Override
    public JFrame getWindowFrame() { return this; }
    
    // observer for the "Project has changed" event
    Observer projectChangedObs = (Observable o, Object project) -> {
        //System.out.println("Observer: project has changed!");
        // This assertion is not true anymore when moving pages between projects.
        //assert hasActiveProject() && activeProject == project;
        invalidateGUI();
    };
    
    ActionListener openFileListener = (ActionEvent evt) -> {
        System.out.println("openFileListener "+evt.getActionCommand()+" "+evt.paramString());
        openFile(new File(evt.getActionCommand()));
    };

    /**
     * Creates new form AppWindow
     */
    public AppWindow() {
        if (Util.useUnifiedToolbar()) {
            getRootPane().putClientProperty("apple.awt.brushMetalLook", true);
            getRootPane().putClientProperty("apple.awt.application.appearance", "system");
        }
        initComponents();
        if (Util.useUnifiedToolbar()) {
            Util.reformatPanelsForPlatformLookAndFeel(jPanelCenterLeft);
            jTreeProjects.setBackground(Color.white);
            Util.makeFrameDraggableByPanel(this, jPanelToolBars);
            jPanelCenterLeft.setOpaque(true);
            jPanelCenterLeft.setBackground(Util.UNIFIED_GRAY_PANEL_BKGND);
        }
        if (Util.isLinuxGTK()) {
            Util.reformatMenuPanels(jAppMenuBar);
        }

        // Restore window position
        Util.loadFramePosition(this, "main");
        Preferences prefs = Util.getPreferences();
        jSplitPaneLeftV.setDividerLocation(prefs.getInt("frame-divLV", jSplitPaneLeftV.getDividerLocation()));
        jSplitPaneLeftCenterH.setDividerLocation(prefs.getInt("frame-divLCH", jSplitPaneLeftCenterH.getDividerLocation()));
        initialDividerSize = jSplitPaneCenterErrListV.getDividerSize();
        jSplitPaneCenterErrListV.setDividerSize(0);
        jPanelBottomError.setVisible(false);

        // Set application icon
        List<Image> iconList = new LinkedList<>();
        iconList.add(Main.loadImage("icon16.png"));
        iconList.add(Main.loadImage("icon32.png"));
        iconList.add(Main.loadImage("icon64.png"));
        setIconImages(iconList);
        if (Util.isOSX()) {
            Taskbar taskbar = Taskbar.getTaskbar();
            taskbar.setIconImage(Main.loadImage("icon64.png"));
        }
        
        // MacOS integration
        if (isOSX()) {
            java.awt.Desktop desktop = Desktop.getDesktop();
            
            desktop.setAboutHandler(e -> showAboutDialog());
            desktop.setOpenFileHandler(e -> openFileHandler(e.getSearchTerm()));
            desktop.setOpenURIHandler(e -> openFileHandler(e.getURI().getPath()));
            desktop.setQuitHandler(this);
            desktop.setPreferencesHandler(e -> openPreferences());
            desktop.setQuitStrategy(QuitStrategy.NORMAL_EXIT);
        }
        
        ccpEngine = new CutCopyPasteEngine(actionCut, actionCopy, actionPaste);
        
        recentFilesList = new RecentFilesListLRU();
        recentFilesList.loadList();
        
        // Add a filler for the BoxLayout
//        jPanelToolBars.add(Box.createHorizontalGlue());
        jAppToolBar_Basic.setVisible(false);
        jAppToolBar_Advanced.setVisible(false);
        
        jPanelCenter.setOpaque(true);
        jPanelCenter.setBackground(Util.darken(jPanelCenter.getBackground(), 0.6f));
        jPlayToolBar.setVisible(false);
        
        jTreeProjects.setModel(new DefaultTreeModel(null));
        jTreeProjects.setCellRenderer(new ProjPageTreeCellRenderer());

        jZoomPanel = new ZoomPanel();
        jZoomPanelContainer.add(jZoomPanel);
        jZoomPanel.setActionNormalZoom(actionNormalZoom);
        jZoomPanel.setActionZoomIn(actionZoomIn);
        jZoomPanel.setActionZoomOut(actionZoomOut);
        
        Font titleFont = ((TitledBorder)jAppToolBar_File.getBorder()).getTitleFont();
        if (titleFont != null) {
            titleFont = titleFont.deriveFont(titleFont.getSize2D() * 0.9f);
            ((TitledBorder)jAppToolBar_File.getBorder()).setTitleFont(titleFont);
            ((TitledBorder)jAppToolBar_Edit.getBorder()).setTitleFont(titleFont);
            ((TitledBorder)jAppToolBar_Basic.getBorder()).setTitleFont(titleFont);
            ((TitledBorder)jAppToolBar_Advanced.getBorder()).setTitleFont(titleFont);
        }

        Color bkgnd = jAppToolBar_File.getBackground(), title = ((TitledBorder)jAppToolBar_File.getBorder()).getTitleColor();
//        System.out.println("bkgnd="+bkgnd);
        Color titleClr = new Color((bkgnd.getRed() + 2*title.getRed()) / 3,
                                   (bkgnd.getGreen()+ 2*title.getGreen()) / 3,
                                   (bkgnd.getBlue()+ 2*title.getBlue()) / 3);
        ((TitledBorder)jAppToolBar_File.getBorder()).setTitleColor(titleClr);
        ((TitledBorder)jAppToolBar_Edit.getBorder()).setTitleColor(titleClr);
        ((TitledBorder)jAppToolBar_Basic.getBorder()).setTitleColor(titleClr);
        ((TitledBorder)jAppToolBar_Advanced.getBorder()).setTitleColor(titleClr);
        
//        jMenuItemNewMeasurePage.setVisible(false);
        
        InputMap inMap = getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW);
        ActionMap actMap = getRootPane().getActionMap();
        common.Action.registerAllActions(actMap, inMap, this, getClass(), firingCondition);
//        if (Util.isOSX()) {
//            actionZoomIn.mapInto(actMap, inMap, KeyStroke.getKeyStroke(KeyEvent.VK_EQUALS, KeyEvent.META_DOWN_MASK));
//        }
        
        // Disable Ctrl+A from the project page tree
        for (KeyStroke ks : jTreeProjects.getInputMap(JTree.WHEN_FOCUSED).allKeys())
            if (ks.getKeyCode() == KeyEvent.VK_A) {
                jTreeProjects.getInputMap(JTree.WHEN_FOCUSED).put(ks, "none");
            }
        
        sharedActionDeleteSelected.mapInto(actMap, inMap, KeyStroke.getKeyStroke(KeyEvent.VK_DELETE, 0));
        for (final SharedResourceProvider.ActionName shAct : ActionName.values()) {
            getSharedAction(shAct).addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent evt) {
                    if (activeEditor != null)
                        activeEditor.sharedActionListener(shAct, evt);
                }
            });
        }
        jMenuSimulation.addMenuListener(new MenuListener() {
            @Override
            public void menuSelected(MenuEvent me) {
                if (!hasActiveProject()) {
                    jMenu_SimModelCheck.setEnabled(false);
                    return;
                }
                ProjectPage page = activeProject.getCurrent().getActivePage();
                PlayCommand cmd = page.canPlay(activeProject);
                if (cmd == PlayCommand.DROPDOWN) {
                    JPopupMenu popup = page.getPlayDropdownMenu(activeProject, actionStartPlayForPopupMenu);
                    jMenu_SimModelCheck.setEnabled(popup.getComponentCount() > 0);
                    jMenu_SimModelCheck.removeAll();
                    while (popup.getComponentCount() > 0) {
                        Component comp = popup.getComponent(0);
                        popup.remove(comp);
                        jMenu_SimModelCheck.add(comp);
                    }
                } 
                else {
                    jMenu_SimModelCheck.setEnabled(false);
                }
            }
            @Override
            public void menuDeselected(MenuEvent me) {
                jMenu_SimModelCheck.removeAll();
            }
            @Override
            public void menuCanceled(MenuEvent me) {
                jMenu_SimModelCheck.removeAll();
            }
        });
        jMenuFile.addMenuListener(new MenuListener() {
            @Override
            public void menuSelected(MenuEvent me) {
                jMenuItem_FileOpenRecent.removeAll();
                boolean isModalPanelActive = (activeEditor != null && activeEditor.isModalEditor());
                boolean canEdit = (!isModalPanelActive) && (!isGuiSuspended);
                jMenuItem_FileOpenRecent.setEnabled(canEdit && recentFilesList.size() > 0);
                for (int i=0; i<recentFilesList.size(); i++) {
                    JMenuItem recent = new JMenuItem(recentFilesList.get(i).getAbsolutePath());
                    recent.addActionListener(openFileListener);
                    jMenuItem_FileOpenRecent.add(recent);                    
                }
            }

            @Override
            public void menuDeselected(MenuEvent me) {
//                jMenuItem_FileOpenRecent.removeAll();
            }

            @Override
            public void menuCanceled(MenuEvent me) {
//                jMenuItem_FileOpenRecent.removeAll();
            }
        });

        jZoomPanel.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
//                System.out.println("new zoom: " + jZoomPanel.getZoom());
                if (activeEditor != null)
                    activeEditor.zoomChanged(jZoomPanel.getZoom());
            }
        });

        jTreeProjects.addTreeExpansionListener(new TreeExpansionListener() {
            @Override
            public void treeExpanded(TreeExpansionEvent event) {
//                System.out.println("expand: "+event.getPath());
                ProjPageTreeNode lastObj = (ProjPageTreeNode) event.getPath().getLastPathComponent();
                expandedTreeObjects.add(lastObj.getObjectUUID());
            }

            @Override
            public void treeCollapsed(TreeExpansionEvent event) {
//                System.out.println("collapse: "+event.getPath());
                /*if (event.getPath().getPathCount()==1)
                    ((JTree)event.getSource()).expandPath(event.getPath()); // Re-expand the root node*/
                ProjPageTreeNode lastObj = (ProjPageTreeNode) event.getPath().getLastPathComponent();
                expandedTreeObjects.remove(lastObj.getObjectUUID());
            }
        });
        myTreeSelectionListener = new TreeSelectionListener() {
            @Override
            public void valueChanged(TreeSelectionEvent e) {
                common.Action.fireGlobalAction(null);
                ProjPageTreeNode selNode = (ProjPageTreeNode) jTreeProjects.getLastSelectedPathComponent();
                if (selNode != null) {
                    selectedNodeId = selNode.getObjectUUID();
                    if (selNode.getEffectiveUserObject() instanceof ProjectPage) {
                        // A new project page has been selected
                        ProjPageTreeNode projNode = (ProjPageTreeNode) jTreeProjects.getSelectionPath().getPathComponent(1);
                        switchToProjectPage((ProjectFile) projNode.getEffectiveUserObject(), 
                                               (ProjectPage) selNode.getEffectiveUserObject(), null);
                    }
                    else if (selNode.getEffectiveUserObject() instanceof ProjectFile) {
                        // The project itself has been selected in the project tree
                        switchToProjectPage((ProjectFile) selNode.getEffectiveUserObject(), null, null);
                    }
                }
                else {
                    // No selected project/page
                    switchToProjectPage(null, null, null);
                    selectedNodeId = null;
                }                
                setStatus(null, true);
                invalidateGUI();
            }
        };
        jTreeProjects.setEditable(true);
        JTextField jTextField = new JTextField();
        jTextField.setEditable(true);
        TreeCellEditor cellEditor = new DefaultCellEditor(jTextField);
        
        // Drag & drop of pages in the project tree
        jTreeProjects.setDragEnabled(true);
        jTreeProjects.setDropMode(DropMode.ON_OR_INSERT);
        jTreeProjects.setTransferHandler(new ProjPageTreeTransferHandler(this));
        
        jScrollPaneProperties.getViewport().setBackground(jPanelPropertyPane.getBackground());
        jScrollPaneProperties.setBackground(jPanelPropertyPane.getBackground());
        
        DefaultTreeCellEditor treeCellEditor = new DefaultTreeCellEditor(jTreeProjects, editingRenderer, cellEditor) {
            @Override
            public Component getTreeCellEditorComponent(JTree tree, Object value, boolean isSelected, 
                                                        boolean expanded, boolean leaf, int row) 
            {
                // Prepare the editing component
                ProjPageTreeNode node = (ProjPageTreeNode)value;
                Icon icon = node.getIcon();
                editingRenderer.setLeafIcon(icon);
                editingRenderer.setClosedIcon(icon);
                editingRenderer.setOpenIcon(icon);
                Component c = super.getTreeCellEditorComponent(tree, node.getUserObject(), isSelected, 
                                                               expanded, leaf, row); 
                return c;
            }

            @Override
            public boolean isCellEditable(EventObject event) {
                boolean isEditable = super.isCellEditable(event);
                if (isEditable) {
                    Object node = tree.getLastSelectedPathComponent();
                    if ((node != null) && (node instanceof ProjPageTreeNode)) {
                        ProjPageTreeNode treeNode = (ProjPageTreeNode) node;
                        isEditable = treeNode.isEditable();
                    }
                }
                return isEditable;
            }
        };
        treeCellEditor.addCellEditorListener(new CellEditorListener() {
            @Override public void editingStopped(ChangeEvent e) {
                //System.out.println("editingStopped "+e);
                DefaultCellEditor cellEd = (DefaultCellEditor)e.getSource();
                renameActivePage(cellEd.getCellEditorValue());
            }
            @Override public void editingCanceled(ChangeEvent e) { }
        });
        jTreeProjects.setCellEditor(treeCellEditor);
        jTreeProjects.setRowHeight(-1);
        
        jScrollPanePageErrors.getVerticalScrollBar().setUI(new BasicScrollBarUI()
        {   
            @Override protected JButton createDecreaseButton(int orientation) { return createZeroButton(); }
            @Override protected JButton createIncreaseButton(int orientation) { return createZeroButton(); }
            @Override protected void configureScrollBarColors() {
                super.configureScrollBarColors();
                thumbColor = new Color(128, 128, 128);
                thumbDarkShadowColor = new Color(100, 100, 100);
                thumbLightShadowColor = new Color(180, 180, 180);
                thumbHighlightColor = new Color(200, 200, 200);
                trackColor = new Color(240, 240, 240);
                trackHighlightColor = Color.WHITE;
                //scrollBarWidth = 8;
            }
            
            private JButton createZeroButton() {
                JButton jbutton = new JButton();
                jbutton.setPreferredSize(new Dimension(0, 0));
                jbutton.setMinimumSize(new Dimension(0, 0));
                jbutton.setMaximumSize(new Dimension(0, 0));
                return jbutton;
            }
        });
        
        PageErrorWarning.setupPageErrorTable(jTablePageErrors);
        jTablePageErrors.getSelectionModel().addListSelectionListener(new ListSelectionListener() {
            @Override
            public void valueChanged(ListSelectionEvent lse) {
                if (updatingPageErrorTable)
                    return;
                if (lse.getValueIsAdjusting())
                    return;
                PageErrorWarning.TableModel model = (PageErrorWarning.TableModel)jTablePageErrors.getModel();
                int[] rows = jTablePageErrors.getSelectedRows();
                if (rows != null && rows.length >= 0 && hasActiveProject() && 
                    activeProject.getCurrent().getActivePage() != null) 
                {
                    ProjectPage page = activeProject.getCurrent().getActivePage();
                    for (int errIndex : rows)
                        page.onSelectErrorWarning(model.getErrorAt(errIndex));
                    setStatus(null, true);
                    invalidateGUI();
                }
            }
        });
        
        for (final RapidMeasureCmd rmc : RapidMeasureCmd.values()) {
            getRapidMeasureMenuItem(rmc).addActionListener(new ActionListener() {
                @Override public void actionPerformed(ActionEvent e) {
                    onRapidMeasureSelected(rmc);
                }
            });
        }

        // First GUI update is executed immediately, without the invalidateGUI()
        updateGUI();
        if (Main.isCheckForUpdatesAutomatic())
            Main.verifyNewVersion(this, false);
    }
    
    // Cell renderer of the JTree of the project pages
    private DefaultTreeCellRenderer editingRenderer = new DefaultTreeCellRenderer();
    
    private static final String APP_TITLE = "GreatSPN Editor";
    private void setAppTitle() {
        if (!hasActiveProject()) {
            setTitle(APP_TITLE);
            if (Util.isOSX()) {
                getRootPane().putClientProperty("Window.documentModified", Boolean.FALSE);
                getRootPane().putClientProperty("Window.documentFile", null);
            }
        }
        else {
            String descr = activeProject.getLongFileDescr();
            setTitle(descr + " - " + APP_TITLE);
            if (Util.isOSX()) {
                getRootPane().putClientProperty("Window.documentModified", !activeProject.isSaved());
                getRootPane().putClientProperty("Window.documentFile", activeProject.getFilename());
            }
            else {
            }
        }
    }
    
    // remember the set of objects that are expanded in the jTree widget
    Set<UUID> expandedTreeObjects = new TreeSet<>();
    private TreeSelectionListener myTreeSelectionListener;
    UUID selectedNodeId = null;

    // Avoid selection change notification when updating the page error table.
    private boolean updatingPageErrorTable = false;

    // Update GUI pages after a state change in the program or in the project's data
    private void updateGUI() {
        assert guiStatus == GuiStatus.INVALID;
        guiStatus = GuiStatus.UPDATING;
        try {
            // Change window title
            setAppTitle();

            //-----------------------------------
            // Update the project tree
            jTreeProjects.getSelectionModel().removeTreeSelectionListener(myTreeSelectionListener);

            // Fill the project JTree
            ProjPageTreeNode root = new ProjPageTreeNode(false, ProjPageTreeCellRenderer.TREE_HEADER, true);
            for (ProjectFile p : projects) {
                boolean isActiveProject = (p == activeProject);
                ProjPageTreeNode projNode = new ProjPageTreeNode(isActiveProject, p, true);
                root.add(projNode);

                // Add internal project nodes
                for (int i=0; i<p.getCurrent().getPageCount(); i++)
                    projNode.add(new ProjPageTreeNode(isActiveProject, p.getCurrent().getPageAt(i), false));
            }
            // Visit all the nodes and keep track of the selected and the expanded ones
            TreePath selectedPath = null;
            ArrayList<TreePath> expandedPaths = new ArrayList<>();
            Stack<ProjPageTreeNode> visitStack = new Stack<>();
            visitStack.push(root);
            while (!visitStack.isEmpty()) {
                ProjPageTreeNode node = visitStack.pop();
                for (int i = 0; i < node.getChildCount(); i++) {
                    ProjPageTreeNode child = (ProjPageTreeNode) node.getChildAt(i);
                    visitStack.push(child);
                }
                if (node.getObjectUUID().equals(selectedNodeId)) {
                    selectedPath = new TreePath(node.getPath());
                }
                if (expandedTreeObjects.contains(node.getObjectUUID())) {
                    expandedPaths.add(new TreePath(node.getPath()));
                }
            }
            // Set back node selection and expansion
            jTreeProjects.setModel(new DefaultTreeModel(root));
            for (TreePath expandedPath : expandedPaths) {
                jTreeProjects.expandPath(expandedPath);
            }
            ProjPageTreeNode selectedNode = null;
            if (selectedPath != null) {
                //jTreePLC.expandPath(selectedPath);
                jTreeProjects.setSelectionPath(selectedPath);
                selectedNode = (ProjPageTreeNode)selectedPath.getLastPathComponent();
            }
            jTreeProjects.getSelectionModel().addTreeSelectionListener(myTreeSelectionListener);
            
            boolean isModalPanelActive = (activeEditor != null && activeEditor.isModalEditor());
            boolean canEdit = (!isModalPanelActive) && (!isGuiSuspended);
            boolean inModalContext = (isModalPanelActive) && (!isGuiSuspended);

            //----------------------------------
            // Update Actions - all actions must be checked here
            jMenuItemNewMultiPage.setVisible(false);
            actionSave.setEnabled(canEdit && hasActiveProject() && !activeProject.isSaved());
            actionSaveAs.setEnabled(canEdit && hasActiveProject());
            boolean canSaveAll = false;
            for (ProjectFile pf : projects)
                canSaveAll = canSaveAll || !pf.isSaved();
            actionSaveAll.setEnabled(canEdit && canSaveAll);
            actionClose.setEnabled(!isGuiSuspended && hasActiveProject());
            actionUndo.setTooltipDesc("Undo");
            actionRedo.setTooltipDesc("Redo");
            actionRenamePage.setEnabled(canEdit && selectedNode != null &&
                                        selectedNode.isEditable());
            actionNewPage.setEnabled(canEdit && hasActiveProject());
            actionImportGreatSpn.setEnabled(canEdit && hasActiveProject());
            if (hasActiveProject()) {
                actionUndo.setEnabled(canEdit && activeProject.canUndo());
                if (activeProject.canUndo())
                    actionUndo.setTooltipDesc("Undo " + activeProject.getUndoDesc());

                actionRedo.setEnabled(canEdit && activeProject.canRedo());
                if (activeProject.canRedo())
                    actionRedo.setTooltipDesc("Redo " + activeProject.getRedoDesc());

                ProjectPage selectedElem = activeProject.getCurrent().getActivePage();
                int selectedPagePos = activeProject.getCurrent().findPagePosition(selectedElem);
                actionMoveUp.setEnabled(canEdit && selectedPagePos > 0);
                actionMoveDown.setEnabled(canEdit && selectedPagePos >= 0 &&
                                          selectedPagePos < activeProject.getCurrent().getPageCount()-1);
                actionDeletePage.setEnabled(canEdit && selectedElem != null);
                actionDuplicatePage.setEnabled(canEdit && selectedElem != null);
                actionPrint.setEnabled(canEdit && selectedElem != null && selectedElem.canPrint());
                jToolbarButtonStartPlay.setEnabled(canEdit && selectedElem != null && 
                                                   selectedElem.canPlay(activeProject) != ProjectPage.PlayCommand.NO);
                jToolbarButtonStartPlay.setVisible(selectedElem!=null && selectedElem.pageSupportsPlay());
                actionStartPlay.setEnabled(canEdit && selectedElem != null && 
                                           selectedElem.canPlay(activeProject) == ProjectPage.PlayCommand.IMMEDIATE);
                
                boolean hasPTInv = canEdit && selectedElem != null && selectedElem.hasPlaceTransInv();
                boolean isPageCorrect = selectedElem != null && selectedElem.isPageCorrect();
                actionStartPlaceInv.setEnabled(hasPTInv && isPageCorrect);
                actionStartTransitionInv.setEnabled(hasPTInv && isPageCorrect);
                actionStartBoundsFromPinv.setEnabled(hasPTInv && isPageCorrect);
                jToolbarButtonShowPlaceSemiflows.setVisible(hasPTInv);
                jToolbarButtonShowTransitionSemiflows.setVisible(hasPTInv);
                jToolbarButtonShowPlaceBoundsFromPinv.setVisible(hasPTInv);
                
                boolean hasUnfolding = selectedElem != null && selectedElem.pageSupportsUnfolding();
                boolean isUnfoldingEnabled = canEdit && hasUnfolding && selectedElem.canBeUnfolded();
                actionStartUnfolding.setEnabled(isUnfoldingEnabled);
                jToolbarButtonUnfolding.setVisible(hasUnfolding);
                
                boolean hasAlgebraTool = selectedElem != null && selectedElem.pageSupportsAlgebraTool();
                boolean isAlgebraToolEnabled = canEdit && hasAlgebraTool && selectedElem.isPageCorrect();
                actionStartAlgebra.setEnabled(isAlgebraToolEnabled);
                jToolbarButtonAlgebra.setVisible(hasAlgebraTool);
                
                boolean hasRGTool = selectedElem != null && selectedElem.pageSupportsRG(ProjectPage.RgType.RG);
                boolean isRGToolEnabled = canEdit && hasRGTool && selectedElem.canBuildRG(ProjectPage.RgType.RG);
                actionStartRG.setEnabled(isRGToolEnabled);
                jToolbarButtonShowRG.setVisible(hasRGTool);

                boolean hasSRGTool = selectedElem != null && selectedElem.pageSupportsRG(ProjectPage.RgType.SRG);
                boolean isSRGToolEnabled = canEdit && hasSRGTool && selectedElem.canBuildRG(ProjectPage.RgType.SRG);
                actionStartSymRG.setEnabled(isSRGToolEnabled);
                jToolbarButtonShowSymRG.setVisible(hasSRGTool);
                
                boolean hasCTMCTool = selectedElem != null && selectedElem.pageSupportsRG(ProjectPage.RgType.CTMC);
                boolean isCTMCToolEnabled = canEdit && hasCTMCTool && selectedElem.canBuildRG(ProjectPage.RgType.CTMC);
                actionStartCTMC.setEnabled(isCTMCToolEnabled);
                jToolbarButtonShowCTMC.setVisible(hasCTMCTool);
                
                boolean hasClearMeasureDir = canEdit && selectedElem != null && selectedElem.hasClearMeasureCmd();
                jToolbarButtonClearMeasureDir.setVisible(hasClearMeasureDir);
                actionClearMeasureDir.setEnabled(hasClearMeasureDir &&
                        selectedElem.canClearMeasure(activeProject.getFilename()));
                
                // Rapid measures pop-up
                boolean hasRapidMeas = selectedElem != null && selectedElem.hasRapidMesures();
                if (hasRapidMeas) {
                    int num = 0;
                    for (RapidMeasureCmd rmc : RapidMeasureCmd.values()) {
                        boolean supports = selectedElem.pageSupportsRapidMeasure(rmc);
                        boolean enabled = supports && selectedElem.canDoRapidMeasure(rmc);
                        num += (supports ? 1 : 0);
                        getRapidMeasureAction(rmc).setEnabled(enabled);
                        getRapidMeasureMenuItem(rmc).setVisible(supports);
                    }
                    jDropdownToolbarButtonRapidMeasure.setVisible(num > 0);
                }
                else {
                    jDropdownToolbarButtonRapidMeasure.setVisible(false);
                }
            }
            else {
                actionUndo.setEnabled(false);
                actionRedo.setEnabled(false);
                actionMoveUp.setEnabled(false);
                actionMoveDown.setEnabled(false);
                actionDeletePage.setEnabled(false);
                actionDuplicatePage.setEnabled(false);
                actionPrint.setEnabled(false);
                jToolbarButtonStartPlay.setEnabled(false);
                actionStartPlay.setEnabled(false);
                actionStartPlaceInv.setEnabled(false);
                actionStartTransitionInv.setEnabled(false);
                actionStartBoundsFromPinv.setEnabled(false);
                jToolbarButtonShowPlaceSemiflows.setVisible(false);
                jToolbarButtonShowTransitionSemiflows.setVisible(false);
                jToolbarButtonShowPlaceBoundsFromPinv.setVisible(false);
                actionStartUnfolding.setEnabled(false);
                actionStartAlgebra.setEnabled(false);
                jToolbarButtonAlgebra.setVisible(false);
                actionStartRG.setEnabled(false);
                jToolbarButtonShowRG.setVisible(false);
                actionStartSymRG.setEnabled(false);
                jToolbarButtonShowSymRG.setVisible(false);
                actionStartCTMC.setEnabled(false);
                jToolbarButtonShowCTMC.setVisible(false);
                jToolbarButtonUnfolding.setVisible(false);
                jToolbarButtonClearMeasureDir.setVisible(false);
                actionClearMeasureDir.setEnabled(false);
                jDropdownToolbarButtonRapidMeasure.setVisible(false);
            }
            
            // Actions that are always active - reenable because they could be made
            // inactive by the suspendInterface method
            actionNewProject.setEnabled(canEdit);
            actionOpen.setEnabled(canEdit);
            actionAppPreferences.setEnabled(canEdit);
            actionClose.setEnabled(canEdit && hasActiveProject());
            actionExit.setEnabled(true);
            jTreeProjects.setEnabled(canEdit);
            actionEndPlay.setEnabled(inModalContext);
            
            jAppToolBar_File.setMaximumSize(jAppToolBar_File.getPreferredSize());
            jAppToolBar_Edit.setMaximumSize(jAppToolBar_Edit.getPreferredSize());
            jAppToolBar_Basic.setMaximumSize(jAppToolBar_Basic.getPreferredSize());
            jAppToolBar_Advanced.setMaximumSize(jAppToolBar_Advanced.getPreferredSize());
            jPlayToolBar.setMaximumSize(jPlayToolBar.getPreferredSize());
            
            jPanelToolBars.invalidate();
            jPanelToolBars.doLayout();
            
 
            //------------------------------------
            // Update the GUI of the active editor
            if (activeEditor != null) {
                jZoomPanel.setEnabled(!isGuiSuspended && activeEditor.isZoomPanelUsed());
                jZoomPanel.setZoom(activeEditor.getPageZoom());
                activeEditor.updateGUI(activeProject, activeProject.getCurrent().getActivePage());
                //activeEditor.getEditorComponent().requestFocus();
                if (!activeEditor.getFocusTarget().isFocusOwner() &&
                    !(Util.hasParentComponentOfClass(getFocusOwner(), BaseCellEditor.class)) &&
                    !(Util.hasParentComponentOfClass(getFocusOwner(), NetViewerPanel.class)) &&
                    !(Util.hasParentComponentOfClass(getFocusOwner(), MeasureEditorPanel.class)) &&
                    !(Util.hasParentComponentOfClass(getFocusOwner(), MultiNetEditorPanel.class))) 
                {
                    activeEditor.getFocusTarget().requestFocusInWindow();
                }
                
                // Ask for shared actions
                activeEditor.updateEnablingOfSharedActions(this);
            }
            else { // no editor
                jZoomPanel.setEnabled(false);
                jZoomPanel.setZoom(100);
                for (SharedResourceProvider.ActionName act : SharedResourceProvider.ActionName.values()) {
                    getSharedAction(act).setEnabled(false);
                }
            }
            ccpEngine.updateCutCopyPasteActions(canEdit && activeEditor!=null ? activeEditor.getFocusTarget() : null);
            
            // Hide empty toolbars (only in editing mode)
            if (activeEditor != null && !activeEditor.isModalEditor()) {
                hideOrShowToolBarIfEmpty(jAppToolBar_Basic);
                hideOrShowToolBarIfEmpty(jAppToolBar_Advanced);
            }
            
            // Update the page error table (after GUI update)
            updatingPageErrorTable = true;
            if (hasActiveProject()) {
                ProjectPage selectedElem = activeProject.getCurrent().getActivePage();
                jTablePageErrors.setEnabled(canEdit && selectedElem != null);
                PageErrorWarning.TableModel model = (PageErrorWarning.TableModel)jTablePageErrors.getModel();
                if (activeEditor == null) {
                    model.setErrorList(ProjectPage.NO_ERRORS);
                }
                else {
                    PageErrorWarning[] errors = activeEditor.getAllErrorsAndWarnings();
                    model.setErrorList(errors);
                    for (int i=0; i<errors.length; i++) {
                        if (errors[i].areAffectedElementsSelected())
                            jTablePageErrors.getSelectionModel().addSelectionInterval(i, i);
                    }
                }
            }
            else {
                jTablePageErrors.setEnabled(false);
                ((PageErrorWarning.TableModel)jTablePageErrors.getModel()).setErrorList(ProjectPage.NO_ERRORS);
            }
            updatingPageErrorTable = false;                
            // Ensure that the page error table is visible when there are errors.
            setErrorPageTableVisible(jTablePageErrors.getModel().getRowCount() > 0);            
        }
        finally {
            guiStatus = GuiStatus.UPDATED;
        }
    }
    
    // Initial divider size of the central SplitPane
    private int initialDividerSize = -1;
    
    // Shows or hides the table with the list of page errors
    private void setErrorPageTableVisible(boolean isVisible) {
        boolean wasVisible = jPanelBottomError.isVisible();
        if (isVisible == wasVisible)
            return;
        jPanelBottomError.setVisible(isVisible);
//        System.out.print("isVisible="+isVisible+" sizeOfClosed="+sizeOfClosed);
//        System.out.print(" dividerLocation="+jSplitPaneCenterErrListV.getDividerLocation());
//        System.out.print(" dividerSize="+jSplitPaneCenterErrListV.getDividerSize());
//        System.out.print(" rightHeight="+jRightPanel.getHeight());
//        System.out.print(" splitHeight="+jSplitPaneCenterErrListV.getHeight());
//        System.out.println(jSplitPaneCenterErrListV.getDividerLocation()+" "+jRightPanel.getHeight());
        if (isVisible) { // Open the error list
            if (!wasVisible) {
                int sizeOfOpen = jSplitPaneCenterErrListV.getTopComponent().getHeight() - 
                                initialDividerSize - (int)Util.origDefaultUiFontSize * 5;
                jSplitPaneCenterErrListV.setDividerLocation(sizeOfOpen);
                jSplitPaneCenterErrListV.setDividerSize(initialDividerSize);
            }
        }
        else { // close the error list
            // It is not necessary to reposition the divider location, since the
            // bottom panel is already invisible. Just hide the divider.
//            jSplitPaneCenterErrListV.setDividerLocation(sizeOfClosed);
            jSplitPaneCenterErrListV.setDividerSize(0);
        }

//        if (isVisible == jSplitPaneCenterErrListV.isVisible())
//            return;
//        jPanelBottomError.setVisible(isVisible);
//        if (isVisible) { // Turn visible from hidden
//            jRightPanel.remove(jPanelCenter);
//            jSplitPaneCenterErrListV.setTopComponent(jPanelCenter);
//            jSplitPaneCenterErrListV.setBottomComponent(jPanelBottomError);
//            jPanelBottomError.setVisible(true);
//            jSplitPaneCenterErrListV.setVisible(true);
//            if (jRightPanel.getHeight() < 50) { // Highly suspicious
//                System.out.println("setErrorPageTableVisible: jRightPanel.getHeight() = "+jRightPanel.getHeight());
//            }
//            jSplitPaneCenterErrListV.setDividerLocation(jRightPanel.getHeight() - 90);
//            jRightPanel.add(jSplitPaneCenterErrListV);
//        }
//        else { // Turn hidden from visible
//            jSplitPaneCenterErrListV.remove(jPanelCenter);
//            jSplitPaneCenterErrListV.remove(jPanelBottomError);
//            jRightPanel.remove(jSplitPaneCenterErrListV);
//            jSplitPaneCenterErrListV.setVisible(false);
//            jRightPanel.add(jPanelCenter);
//            jPanelBottomError.setVisible(false);
//        }
//        jRightPanel.invalidate();
//        jRightPanel.revalidate();
    }
    
    @Override
    public final common.Action getSharedAction(SharedResourceProvider.ActionName shAction) {
        switch (shAction) {
            case EDIT_SELECT_ALL:           return sharedActionSelectAll;
            case EDIT_DESELECT_ALL:         return sharedActionDeselectAll;
            case EDIT_DELETE_SELECTED:      return sharedActionDeleteSelected;
            case EDIT_INVERT_SELECTION:     return sharedActionInvertSelection;
            case PLAY_NEXT:                 return sharedActionNext;
            case PLAY_PREVIOUS:             return sharedActionPrev;
            case PLAY_RESTART:              return sharedActionRestart;
            case CHANGE_BINDINGS:           return sharedActionChangeBindings;
            case COMPUTE_PLACE_SEMIFLOWS:   return sharedActionComputePlaceSemiflows;
            case COMPUTE_TRANS_SEMIFLOWS:   return sharedActionComputeTransitionSemiflows;
            case COMPUTE_PLACE_BOUNDS_FROM_PINV: return sharedActionComputeBoundsFromPinv;
            case EXPORT_GREATSPN_FORMAT:    return sharedActionExportGreatSpn;
            case EXPORT_PNML_FORMAT:        return sharedActionExportPNML;
            case EXPORT_GRML_FORMAT:        return sharedActionExportGRML;
            case EXPORT_APNN_FORMAT:        return sharedActionExportAPNN;
            case EXPORT_DTA_FORMAT:         return sharedActionExportDta;
            default:  throw new IllegalStateException("Action is not bound.");
        }
    }
    
    private common.Action getRapidMeasureAction(RapidMeasureCmd rmc) {
        switch (rmc) {
            case CTL_MODEL_CHECKING:    return rmcActionCTL;
            case BUILD_RG:              return rmcActionBuildRG;
            case BUILD_SYMRG:           return rmcActionBuildSymRG;
            case BUILD_RS_MDD:          return rmcActionBuildRS_MDD;
            case STEADY_STATE_SOLUTION: return rmcActionSteadyStateSol;
            case TRANSIENT_SOLUTION:    return rmcActionTransientSol;
            case STEADY_STATE_SIM:      return rmcActionSteadyStateSim;
            case TRANSIENT_SIM:         return rmcActionTransientSim;
        }
        throw new IllegalStateException();
    }
    
    private JMenuItem getRapidMeasureMenuItem(RapidMeasureCmd rmc) {
        switch (rmc) {
            case CTL_MODEL_CHECKING:    return rmcMenuItemCTL;
            case BUILD_RG:              return rmcMenuItemBuildRG;
            case BUILD_SYMRG:           return rmcMenuItemBuildSymRG;
            case BUILD_RS_MDD:          return rmcMenuItemBuildRS_DD;
            case STEADY_STATE_SOLUTION: return rmcMenuItemSteadyStateSol;
            case TRANSIENT_SOLUTION:    return rmcMenuItemTransientSol;
            case STEADY_STATE_SIM:      return rmcMenuItemSteadyStateSim;
            case TRANSIENT_SIM:         return rmcMenuItemTransientSim;
        }
        throw new IllegalStateException();
    }
    
    private void onRapidMeasureSelected(final RapidMeasureCmd rmc) {
        executeUndoableCommand("new rapid measure.", (ProjectData proj, ProjectPage elem) -> {
            MeasurePage mPage = rmc.createRapidMeasureFor((GspnPage)elem);
            if (mPage == null)
                throw new NoOpException();
            
            // Add or replace the measure page
            boolean replaced = false;
            for (int i=0; i<activeProject.getCurrent().getPageCount(); i++) {
                ProjectPage p = activeProject.getCurrent().getPageAt(i);
                if (p instanceof MeasurePage) {
                    MeasurePage mp = (MeasurePage)p;
                    if (mp.rapidMeasureType.equals(rmc.name()) &&
                            mp.targetGspnName.equals(elem.getPageName()))
                    {
                        // Replace the (already existing) rapid measure page
                        activeProject.getCurrent().deletePage(i);
                        mPage.setPageName(generateUniquePageName(activeProject, mPage.getPageName()));
                        activeProject.getCurrent().addPageAt(mPage, i);
                        replaced = true;
                        break;
                    }
                }
            }
            if (!replaced) {
                mPage.setPageName(generateUniquePageName(activeProject, mPage.getPageName()));
                activeProject.getCurrent().addPage(mPage);
            }
            switchToProjectPage(activeProject, mPage, null);
        });
    }

    @Override
    public void setSuspended(boolean isSuspended) {
        firingCondition.setSatisfied(!isSuspended);
//        this.isGuiSuspended = isSuspended;
//        invalidateGUI();
    }
    
    @Override
    public boolean isSuspended() { return this.isGuiSuspended; }
    
    // A panel with a more reasonable default Scrollable, used by the property panel
    class ScrollablePropertyPanel extends JPanel implements Scrollable {
        @Override
        public Dimension getPreferredScrollableViewportSize() { 
            return getPreferredSize(); 
        }
        @Override
        public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) { return 12; }
        @Override
        public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction) {
            if (orientation == SwingConstants.HORIZONTAL)
                return (int)visibleRect.getWidth();
            return (int)visibleRect.getHeight();
        }
        @Override
        public boolean getScrollableTracksViewportWidth() { 
            return true; /* resize the panel to occupy all the width of the extent. */ 
        }
        @Override
        public boolean getScrollableTracksViewportHeight() { return false;}
    }
    
    // Used by the OSX Adapter
    public void showAboutDialog() {
        actionAboutActionPerformed(null);
    }
    
//    // Used by the OSX Adapter
//    public boolean quitHandler() {
//        return quitApplication();
//    }
    
    @Override public void handleQuitRequestWith(QuitEvent e, QuitResponse response) {
        quitApplication();
    }
    
    // Used by the OSX Adapter
    public void openFileHandler(String filename) {
        openFile(new File(filename));
    }
    
    // Used by the OSX Adapter
    public void openPreferences() {
        actionAppPreferencesActionPerformed(null);
    }

    private boolean askSaveBeforeClosing(ProjectFile pf) {
        if (pf.isSaved())
            return true;
        
        int ret = JOptionPane.showConfirmDialog
                (AppWindow.this, "Project \"" + pf.getProjectName()
                 + "\" has been changed. Save it?",
                 "Close project",
                 JOptionPane.YES_NO_CANCEL_OPTION, 
                 JOptionPane.QUESTION_MESSAGE);
        switch (ret) {
            case JOptionPane.YES_OPTION:
                return saveProject(pf, false);
            case JOptionPane.NO_OPTION:
                return true;
            default:
                return false;
        }
    }
    
    private boolean saveProject(ProjectFile pf, boolean mayChangeName) {
        final File origFile = pf.getFilename();
        File file = origFile;
        if (file == null || !file.canWrite() || mayChangeName) {
            boolean repeatChooser;
            do {
                repeatChooser = false;
                final JFileChooser fileChooser = new JFileChooser();
                fileChooser.setDialogTitle("Save project \""+pf.getProjectName()+"\"");
                String curDir = Util.getPreferences().get("save-dir", null);
                fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
                if (file == null) // propose the initial file name in the current directory
                    file = new File(pf.getCurrent().projName + "." + ProjectFile.PNPRO_EXT);
                fileChooser.setSelectedFile(file);
                fileChooser.addChoosableFileFilter(ProjectFile.getFileFilter());
                fileChooser.setFileFilter(ProjectFile.getFileFilter());
                if (fileChooser.showSaveDialog(this) != JFileChooser.APPROVE_OPTION)
                    return false;
                file = fileChooser.getSelectedFile();
                if (fileChooser.getFileFilter() instanceof FileNameExtensionFilter) {
                    FileNameExtensionFilter fnef = (FileNameExtensionFilter)fileChooser.getFileFilter();
                    String path = file.getPath();
                    int lastDot = path.lastIndexOf(".");
                    if (lastDot != -1)
                        path = path.substring(0, lastDot);
                    file = new File(path + "." + fnef.getExtensions()[0]);
                }
                curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
                Util.getPreferences().put("save-dir", curDir);
                if (file.exists() && !file.equals(pf.getFilename())) {
                    int r = JOptionPane.showConfirmDialog(AppWindow.this, 
                             "The file \""+file+"\" already exists! Overwrite it?", 
                                                           "Overwrite file", 
                                                           JOptionPane.YES_NO_CANCEL_OPTION, 
                                                           JOptionPane.WARNING_MESSAGE);
                    if (r == JOptionPane.NO_OPTION)
                        repeatChooser = true;
                    else if (r == JOptionPane.CANCEL_OPTION)
                        return false;
                }
            } while (repeatChooser);
        }
        // save to file
        pf.setFilename(file);
        boolean saved = false;
        try {
            pf.save();
            saved = true;
            if (origFile != file)
                recentFilesList.save(file);
            setStatus(file.getAbsolutePath()+" saved.", true);
        }
        catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(AppWindow.this, 
                                          "An error happened while saving the project.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Save project "+pf.getProjectName(), 
                                          JOptionPane.ERROR_MESSAGE);
            setStatus("could not save "+pf.getProjectName(), true);
        }
        invalidateGUI();
        return saved;
    }
    
    protected boolean closeProject(ProjectFile pf) {
        if (!askSaveBeforeClosing(pf))
            return false;
        pf.deleteObserver(projectChangedObs);
        projects.remove(pf);
        if (pf == activeProject) {
            switchToProjectPage(null, null, null);
        }
        invalidateGUI();
        setStatus(null, true);
        return true;
    }
    
    private void renameActivePage(Object cellEditorValue) {
        final String newName = cellEditorValue.toString();
        executeUndoableCommand("page renaming.", (ProjectData proj, ProjectPage page) -> {
            if (page == null) {
                proj.projName = newName;
            }
            else {
                // Pages should have a unique name
                for (int i=0; i<proj.getPageCount(); i++)
                    if (proj.getActivePage() != proj.getPageAt(i) &&
                            proj.getPageAt(i).getPageName().equals(newName)) {
                        throw new InvalidOperationException("Page name \""+newName+"\" is already in use.");
                    }
                // Rename
                if (proj.getActivePage().getPageName().equals(newName))
                    throw new NoOpException();
                String oldName = proj.getActivePage().getPageName();
                proj.getActivePage().setPageName(newName);
                for (int i=0; i<proj.getPageCount(); i++)
                    proj.getPageAt(i).onAnotherPageRenaming(oldName, newName);
            }
        });
    }
    
    
    public void dragDropProjectPage(ProjPageTreeNode pp, ProjPageTreeNode parent, 
                                    int index, boolean isCopy) 
    {
//        System.out.println("dragDropProjectPage: "+pp.getEffectiveUserObject()+" to "+parent.getEffectiveUserObject()+" index="+index);
//        System.out.println(pp.getEffectiveUserObject().getClass()+" to "+parent.getEffectiveUserObject().getClass());
        try {
            ProjectFile dstProj, srcProj;
            ProjectPage srcPage;

            if ((pp.getEffectiveUserObject() instanceof ProjectPage) &&
                (parent.getEffectiveUserObject() instanceof ProjectFile))
            {
                srcPage = (ProjectPage)pp.getEffectiveUserObject();
                srcProj = (ProjectFile)((ProjPageTreeNode)pp.getParent()).getEffectiveUserObject();
                dstProj = (ProjectFile)parent.getEffectiveUserObject();
            }
            else if ((pp.getEffectiveUserObject() instanceof ProjectPage) &&
                (parent.getEffectiveUserObject() instanceof ProjectPage))
            {
                // The user is dropping the page pp onto the page "parent"
                srcPage = (ProjectPage)pp.getEffectiveUserObject();
                srcProj = (ProjectFile)((ProjPageTreeNode)pp.getParent()).getEffectiveUserObject();
                dstProj = (ProjectFile)((ProjPageTreeNode)parent.getParent()).getEffectiveUserObject();
                index = dstProj.getCurrent().findPagePosition((ProjectPage)parent.getEffectiveUserObject());
            }
            else {
                System.out.println("Can't understand drop: "+pp.getEffectiveUserObject()+" to "+parent.getEffectiveUserObject()+" index="+index);
                System.out.println(pp.getEffectiveUserObject().getClass()+" to "+parent.getEffectiveUserObject().getClass());
                return;
            }       

            // Duplicate the page and make it different (new UUID)
            ProjectPage page = (ProjectPage)Util.deepCopyRelink(srcPage);
            page.generateNewUUID();

            // Is this page arriving from another project?
            boolean fromAnotherProj = (-1 == dstProj.getCurrent().findPagePosition(srcPage));
            assert (srcProj != dstProj) == fromAnotherProj;
            if (isCopy || fromAnotherProj) {
                // Rename the page if the project already has another page with the same name
                while (dstProj.getCurrent().findPageByName(page.getPageName()) != null) {
                    page.setPageName("copy of "+page.getPageName());
                }
            }

            if (srcProj == dstProj) { // Moving/copying a page within the same project
                dstProj.startChanges();
                if (!isCopy) {
                    int currIndex = dstProj.getCurrent().findPagePosition(srcPage);
                    assert currIndex != -1;
                    if (currIndex == index) { // Moving in the same location.
                        dstProj.rollbackChanges();
                        setStatus("", true);
                        return;
                    }
                    if (currIndex < index)
                        index--;
                    dstProj.getCurrent().deletePage(currIndex);
                }
                dstProj.getCurrent().addPageAt(page, index);
                dstProj.commitChanges(isCopy ? "copy page." : "move page.");
                setStatus(dstProj.getUndoDesc(), true);
        //            proj.getCurrent().setActivePage(page);
                switchToProjectPage(dstProj, page, null);
            }
            else { // Moving/copying a page between two projects
                if (!isCopy) {
                    // Remove the page from the source project
                    int pagePos = srcProj.getCurrent().findPagePosition(srcPage);
                    int activePos = srcProj.getCurrent().findPagePosition(srcProj.getCurrent().getActivePage());
                    if (pagePos >= 0) {
                        srcProj.startChanges();
                        srcProj.getCurrent().deletePage(pagePos);
                        if (activePos > pagePos) {
                            ProjectPage newActivePage = srcProj.getCurrent().getPageAt(activePos - 1);
                            srcProj.getCurrent().setActivePage(newActivePage);
                        }
                        srcProj.commitChanges("remove dragged page.");
                        setStatus(srcProj.getUndoDesc(), true);
                    }
                }
                // Add the new page to the dstProject
                dstProj.startChanges();
                dstProj.getCurrent().addPageAt(page, index);
                dstProj.commitChanges((isCopy ? "copy" : "move") + " page from project "+srcProj.getProjectName()+".");
                setStatus(dstProj.getUndoDesc(), true);
            }

            switchToProjectPage(dstProj, page, null);
        }
        catch (RuntimeException e) {
            Main.logException(e, true);
            throw e;
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

        actionNewProject = new common.Action();
        actionOpen = new common.Action();
        actionSave = new common.Action();
        actionSaveAs = new common.Action();
        actionSaveAll = new common.Action();
        actionClose = new common.Action();
        actionExit = new common.Action();
        actionUndo = new common.Action();
        actionRedo = new common.Action();
        actionMoveUp = new common.Action();
        actionMoveDown = new common.Action();
        actionDeletePage = new common.Action();
        actionRenamePage = new common.Action();
        actionDuplicatePage = new common.Action();
        actionNewPage = new common.Action();
        actionAppPreferences = new common.Action();
        jPopupMenuAddPage = new javax.swing.JPopupMenu();
        jMenuItemNewNetPage_PT = new javax.swing.JMenuItem();
        jMenuItemNewNetPage_CPN = new javax.swing.JMenuItem();
        jMenuItemNewNetPage_SWN = new javax.swing.JMenuItem();
        jMenuItemNewNetPage_GSPN = new javax.swing.JMenuItem();
        jMenuItemNewNetPage_FullGSPN = new javax.swing.JMenuItem();
        jSeparatorPetriNets = new javax.swing.JPopupMenu.Separator();
        jMenuItemNewDtaPage = new javax.swing.JMenuItem();
        jMenuItemNewMeasurePage = new javax.swing.JMenuItem();
        jMenuItemNewMultiPage = new javax.swing.JMenuItem();
        jSeparatorLibraryModels = new javax.swing.JPopupMenu.Separator();
        jMenuLibraryModels = new javax.swing.JMenu();
        actionNewNetPage_FullGSPN = new common.Action();
        actionNewDtaPage = new common.Action();
        actionNewMeasurePage = new common.Action();
        actionZoomIn = new common.Action();
        actionZoomOut = new common.Action();
        actionNormalZoom = new common.Action();
        actionCut = new common.Action();
        actionCopy = new common.Action();
        actionPaste = new common.Action();
        actionPrint = new common.Action();
        actionAbout = new common.Action();
        actionHelpGrammar = new common.Action();
        actionStartPlay = new common.Action();
        actionStartPlayForPopupMenu = new common.Action();
        actionEndPlay = new common.Action();
        actionStartPlaceInv = new common.Action();
        actionStartTransitionInv = new common.Action();
        actionStartBoundsFromPinv = new common.Action();
        actionClearMeasureDir = new common.Action();
        actionImportGreatSpn = new common.Action();
        actionImportPNML = new common.Action();
        actionStartUnfolding = new common.Action();
        actionStartAlgebra = new common.Action();
        actionStartRG = new common.Action();
        actionStartSymRG = new common.Action();
        actionStartCTMC = new common.Action();
        actionAdvRapidMeasurePopup = new common.Action();
        sharedActionRestart = new common.Action();
        sharedActionPrev = new common.Action();
        sharedActionNext = new common.Action();
        sharedActionDeleteSelected = new common.Action();
        sharedActionSelectAll = new common.Action();
        sharedActionInvertSelection = new common.Action();
        sharedActionDeselectAll = new common.Action();
        sharedActionExportGreatSpn = new common.Action();
        sharedActionExportPNML = new common.Action();
        sharedActionExportGRML = new common.Action();
        sharedActionExportAPNN = new common.Action();
        sharedActionExportDta = new common.Action();
        sharedActionChangeBindings = new common.Action();
        sharedActionComputePlaceSemiflows = new common.Action();
        sharedActionComputeTransitionSemiflows = new common.Action();
        sharedActionComputeBoundsFromPinv = new common.Action();
        actionCaptureSVG = new common.Action();
        actionNewNetPage_PT = new common.Action();
        actionNewNetPage_CPN = new common.Action();
        actionNewNetPage_GSPN = new common.Action();
        actionNewNetPage_SWN = new common.Action();
        actionNewMultiPage = new common.Action();
        resourceFactory = new editor.gui.ResourceFactory();
        rmcActionCTL = new common.Action();
        rmcActionBuildRG = new common.Action();
        rmcActionBuildSymRG = new common.Action();
        rmcActionBuildRS_MDD = new common.Action();
        rmcActionSteadyStateSol = new common.Action();
        rmcActionTransientSol = new common.Action();
        rmcActionSteadyStateSim = new common.Action();
        rmcActionTransientSim = new common.Action();
        jPopupMenuRapidMeaureCmd = new javax.swing.JPopupMenu();
        rmcMenuItemCTL = new javax.swing.JMenuItem();
        rmcMenuItemBuildRG = new javax.swing.JMenuItem();
        rmcMenuItemBuildSymRG = new javax.swing.JMenuItem();
        rmcMenuItemBuildRS_DD = new javax.swing.JMenuItem();
        rmcMenuItemSteadyStateSol = new javax.swing.JMenuItem();
        rmcMenuItemTransientSol = new javax.swing.JMenuItem();
        rmcMenuItemSteadyStateSim = new javax.swing.JMenuItem();
        rmcMenuItemTransientSim = new javax.swing.JMenuItem();
        jStatusBar = new javax.swing.JPanel();
        jStatusLabel = new javax.swing.JLabel();
        jSeparator1 = new javax.swing.JSeparator();
        jZoomPanelContainer = new javax.swing.JPanel();
        jPanelCenterLeft = new javax.swing.JPanel();
        jSplitPaneLeftCenterH = new javax.swing.JSplitPane();
        jLeftPanel = new javax.swing.JPanel();
        jSplitPaneLeftV = new javax.swing.JSplitPane();
        jPanelProjectTree = new javax.swing.JPanel();
        jScrollPaneProjects = new javax.swing.JScrollPane();
        jTreeProjects = new javax.swing.JTree();
        jPanelTreeCmds = new javax.swing.JPanel();
        jToolBarTree = new javax.swing.JToolBar();
        jToolbarButtonMoveUp = new common.JToolbarButton();
        jToolbarButtonMoveDown = new common.JToolbarButton();
        jToolbarButtonDuplicatePage = new common.JToolbarButton();
        jToolbarButtonRenamePage = new common.JToolbarButton();
        jToolbarButtonDeletePage = new common.JToolbarButton();
        jPanelPropertyPane = new javax.swing.JPanel();
        jScrollPaneProperties = new javax.swing.JScrollPane();
        jPanelProperty = new ScrollablePropertyPanel();
        jRightPanel = new javax.swing.JPanel();
        jSplitPaneCenterErrListV = new javax.swing.JSplitPane();
        jPanelCenter = new javax.swing.JPanel();
        jPanelBottomError = new javax.swing.JPanel();
        jScrollPanePageErrors = new javax.swing.JScrollPane();
        jTablePageErrors = new javax.swing.JTable();
        jPanelToolBars = new javax.swing.JPanel();
        jAppToolBar_File = new javax.swing.JToolBar();
        jToolbarButtonNew = new common.JToolbarButton();
        jDropdownAddNewPage = new common.JDropdownToolbarButton();
        jToolbarButtonOpen = new common.JToolbarButton();
        jToolbarButtonSaveAll = new common.JToolbarButton();
        jAppToolBar_Edit = new javax.swing.JToolBar();
        jToolbarButtonUndo = new common.JToolbarButton();
        jToolbarButtonRedo = new common.JToolbarButton();
        jSeparator6 = new javax.swing.JToolBar.Separator();
        jToolbarButtonCut = new common.JToolbarButton();
        jToolbarButtonCopy = new common.JToolbarButton();
        jToolbarButtonPaste = new common.JToolbarButton();
        jToolbarButtonDeleteSelected = new common.JToolbarButton();
        jAppToolBar_Basic = new javax.swing.JToolBar();
        jToolbarButtonStartPlay = new common.JToolbarButton();
        jToolbarButtonShowPlaceSemiflows = new common.JToolbarButton();
        jToolbarButtonShowTransitionSemiflows = new common.JToolbarButton();
        jToolbarButtonShowPlaceBoundsFromPinv = new common.JToolbarButton();
        jToolbarButtonShowRG = new common.JToolbarButton();
        jToolbarButtonShowSymRG = new common.JToolbarButton();
        jToolbarButtonShowCTMC = new common.JToolbarButton();
        jToolbarButtonUnfolding = new common.JToolbarButton();
        jToolbarButtonClearMeasureDir = new common.JToolbarButton();
        jAppToolBar_Advanced = new javax.swing.JToolBar();
        jToolbarButtonAlgebra = new common.JToolbarButton();
        jDropdownToolbarButtonRapidMeasure = new common.JDropdownToolbarButton();
        jPlayToolBar = new javax.swing.JToolBar();
        jToolbarButtonEndPlay = new common.JToolbarButton();
        jAppMenuBar = new javax.swing.JMenuBar();
        jMenuFile = new javax.swing.JMenu();
        jMenuItem_FileNewSchema = new javax.swing.JMenuItem();
        jMenuItem_FileOpen = new javax.swing.JMenuItem();
        jMenuItem_FileOpenRecent = new javax.swing.JMenu();
        jMenuItem_FileSave = new javax.swing.JMenuItem();
        jMenuItem_FileSaveAs = new javax.swing.JMenuItem();
        jMenuItem_FileSaveAll = new javax.swing.JMenuItem();
        jMenu_FileImport = new javax.swing.JMenu();
        jMenuItem_GreatSpnImport = new javax.swing.JMenuItem();
        jMenuItem_PNMLImport = new javax.swing.JMenuItem();
        jMenuItem_FileExport = new javax.swing.JMenu();
        jMenuItem_GreatSpnExport = new javax.swing.JMenuItem();
        jMenuItem_PNMLExport = new javax.swing.JMenuItem();
        jMenuItem_GrMLExport = new javax.swing.JMenuItem();
        jMenuItem_ApnnFormat = new javax.swing.JMenuItem();
        jMenuItem_DtaFormat = new javax.swing.JMenuItem();
        jMenuItem_FilePrint = new javax.swing.JMenuItem();
        jMenuItem_FileCloseProj = new javax.swing.JMenuItem();
        jSeparator2 = new javax.swing.JPopupMenu.Separator();
        jMenuItem_FileExit = new javax.swing.JMenuItem();
        jMenuEdit = new javax.swing.JMenu();
        jMenuItem_EditUndo = new javax.swing.JMenuItem();
        jMenuItem_EditRedo = new javax.swing.JMenuItem();
        jSeparator5 = new javax.swing.JPopupMenu.Separator();
        jMenuItem_EditCut = new javax.swing.JMenuItem();
        jMenuItem_EditCopy = new javax.swing.JMenuItem();
        jMenuItem_EditPaste = new javax.swing.JMenuItem();
        jSeparator8 = new javax.swing.JPopupMenu.Separator();
        jMenuItem_EditSelectAll = new javax.swing.JMenuItem();
        jMenuItem_EditDeselectAll = new javax.swing.JMenuItem();
        jMenuItem_EditInvertSelection = new javax.swing.JMenuItem();
        jMenuItem_EditDeleteSelected = new javax.swing.JMenuItem();
        jSeparator4 = new javax.swing.JPopupMenu.Separator();
        jMenuItemOptions = new javax.swing.JMenuItem();
        jMenuView = new javax.swing.JMenu();
        jMenuItem1 = new javax.swing.JMenuItem();
        jMenuItem_ViewZoomIn = new javax.swing.JMenuItem();
        jMenuItem_ViewZoomOut = new javax.swing.JMenuItem();
        jMenuSimulation = new javax.swing.JMenu();
        jMenuItem_SimStart = new javax.swing.JMenuItem();
        jMenu_SimModelCheck = new javax.swing.JMenu();
        jSeparator10 = new javax.swing.JPopupMenu.Separator();
        jMenuItem_SimRestart = new javax.swing.JMenuItem();
        jMenuItem_SimPrev = new javax.swing.JMenuItem();
        jMenuItem_SimNext = new javax.swing.JMenuItem();
        jSeparator9 = new javax.swing.JPopupMenu.Separator();
        jMenuItem_SimClose = new javax.swing.JMenuItem();
        jMenuHelp = new javax.swing.JMenu();
        jMenuItem_HelpAbout = new javax.swing.JMenuItem();
        jMenuItem_HelpShowGrammar = new javax.swing.JMenuItem();

        actionNewProject.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_N, java.awt.event.InputEvent.CTRL_MASK));
        actionNewProject.setActionName("New");
        actionNewProject.setIcon(resourceFactory.getPnproNew16());
        actionNewProject.setMenuMnemonic(KeyEvent.VK_N);
        actionNewProject.setToolbarIcon(resourceFactory.getPnproNew32());
        actionNewProject.setTooltipDesc("Create a new project.");
        actionNewProject.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewProjectActionPerformed(evt);
            }
        });

        actionOpen.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_O, java.awt.event.InputEvent.CTRL_MASK));
        actionOpen.setActionName("Open...");
        actionOpen.setIcon(resourceFactory.getOpen16());
        actionOpen.setMenuMnemonic(KeyEvent.VK_O);
        actionOpen.setToolbarIcon(resourceFactory.getOpen32());
        actionOpen.setTooltipDesc("Open a project from a file...");
        actionOpen.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionOpenActionPerformed(evt);
            }
        });

        actionSave.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_S, java.awt.event.InputEvent.CTRL_MASK));
        actionSave.setActionName("Save");
        actionSave.setIcon(resourceFactory.getDisk16());
        actionSave.setMenuMnemonic(KeyEvent.VK_S);
        actionSave.setToolbarIcon(resourceFactory.getDisk32());
        actionSave.setTooltipDesc("Save the project to a file.");
        actionSave.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionSaveActionPerformed(evt);
            }
        });

        actionSaveAs.setActionName("Save as...");
        actionSaveAs.setIcon(resourceFactory.getDiskEdit16());
        actionSaveAs.setMenuMnemonic(KeyEvent.VK_A);
        actionSaveAs.setToolbarIcon(resourceFactory.getDiskEdit32());
        actionSaveAs.setTooltipDesc("Save a copy of the project with a new name.");
        actionSaveAs.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionSaveAsActionPerformed(evt);
            }
        });

        actionSaveAll.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_S, java.awt.event.InputEvent.SHIFT_MASK | java.awt.event.InputEvent.CTRL_MASK));
        actionSaveAll.setActionName("Save all");
        actionSaveAll.setIcon(resourceFactory.getDiskMultiple16());
        actionSaveAll.setMenuMnemonic(KeyEvent.VK_V);
        actionSaveAll.setToolbarIcon(resourceFactory.getDiskMultiple32());
        actionSaveAll.setTooltipDesc("Save all the open projects.");
        actionSaveAll.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionSaveAllActionPerformed(evt);
            }
        });

        actionClose.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_W, java.awt.event.InputEvent.CTRL_MASK));
        actionClose.setActionName("Close");
        actionClose.setIcon(resourceFactory.getEmpty16());
        actionClose.setMenuMnemonic(KeyEvent.VK_C);
        actionClose.setTooltipDesc("Close the active project.");
        actionClose.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionCloseActionPerformed(evt);
            }
        });

        actionExit.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_Q, java.awt.event.InputEvent.CTRL_MASK));
        actionExit.setActionName("Exit");
        actionExit.setIcon(resourceFactory.getEmpty16());
        actionExit.setMenuMnemonic(KeyEvent.VK_E);
        actionExit.setTooltipDesc("Exit from the application.");
        actionExit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionExitActionPerformed(evt);
            }
        });

        actionUndo.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_Z, java.awt.event.InputEvent.CTRL_MASK));
        actionUndo.setActionName("Undo");
        actionUndo.setIcon(resourceFactory.getUndo16());
        actionUndo.setMenuMnemonic(KeyEvent.VK_U);
        actionUndo.setToolbarIcon(resourceFactory.getUndo32());
        actionUndo.setTooltipDesc("Undo the last action.");
        actionUndo.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionUndoActionPerformed(evt);
            }
        });

        actionRedo.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_Z, java.awt.event.InputEvent.SHIFT_MASK | java.awt.event.InputEvent.CTRL_MASK));
        actionRedo.setActionName("Redo");
        actionRedo.setIcon(resourceFactory.getRedo16());
        actionRedo.setMenuMnemonic(KeyEvent.VK_R);
        actionRedo.setToolbarIcon(resourceFactory.getRedo32());
        actionRedo.setTooltipDesc("Redo the last action.");
        actionRedo.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionRedoActionPerformed(evt);
            }
        });

        actionMoveUp.setActionName("Move up");
        actionMoveUp.setIcon(resourceFactory.getArrowUp16());
        actionMoveUp.setTooltipDesc("Move up the selected project page.");
        actionMoveUp.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionMoveUpActionPerformed(evt);
            }
        });

        actionMoveDown.setActionName("Move down");
        actionMoveDown.setIcon(resourceFactory.getArrowDown16());
        actionMoveDown.setTooltipDesc("Move down the selected project page.");
        actionMoveDown.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionMoveDownActionPerformed(evt);
            }
        });

        actionDeletePage.setActionName("Delete page");
        actionDeletePage.setIcon(resourceFactory.getCross16());
        actionDeletePage.setTooltipDesc("Delete the selected project page.");
        actionDeletePage.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionDeletePageActionPerformed(evt);
            }
        });

        actionRenamePage.setActionName("Rename");
        actionRenamePage.setIcon(resourceFactory.getRename16());
        actionRenamePage.setTooltipDesc("Rename the selected project page.");
        actionRenamePage.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionRenamePageActionPerformed(evt);
            }
        });

        actionDuplicatePage.setActionName("Duplicate");
        actionDuplicatePage.setIcon(resourceFactory.getPageWhiteCopy16());
        actionDuplicatePage.setTooltipDesc("Duplicate the selected page.");
        actionDuplicatePage.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionDuplicatePageActionPerformed(evt);
            }
        });

        actionNewPage.setActionName("New page");
        actionNewPage.setIcon(resourceFactory.getPageWhiteNew16());
        actionNewPage.setToolbarIcon(resourceFactory.getPageWhiteNew32());
        actionNewPage.setTooltipDesc("Add a new page to the active project.");

        actionAppPreferences.setActionName("Options...");
        actionAppPreferences.setTooltipDesc("Show the application options.");
        actionAppPreferences.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAppPreferencesActionPerformed(evt);
            }
        });

        jPopupMenuAddPage.addPopupMenuListener(new javax.swing.event.PopupMenuListener() {
            public void popupMenuWillBecomeVisible(javax.swing.event.PopupMenuEvent evt) {
                addPagePopupMenuBecomesVisible(evt);
            }
            public void popupMenuWillBecomeInvisible(javax.swing.event.PopupMenuEvent evt) {
            }
            public void popupMenuCanceled(javax.swing.event.PopupMenuEvent evt) {
            }
        });

        jMenuItemNewNetPage_PT.setAction(actionNewNetPage_PT);
        jMenuItemNewNetPage_PT.setText("jMenuItem2");
        jPopupMenuAddPage.add(jMenuItemNewNetPage_PT);

        jMenuItemNewNetPage_CPN.setAction(actionNewNetPage_CPN);
        jMenuItemNewNetPage_CPN.setText("jMenuItem2");
        jPopupMenuAddPage.add(jMenuItemNewNetPage_CPN);

        jMenuItemNewNetPage_SWN.setAction(actionNewNetPage_SWN);
        jMenuItemNewNetPage_SWN.setText("jMenuItem2");
        jPopupMenuAddPage.add(jMenuItemNewNetPage_SWN);

        jMenuItemNewNetPage_GSPN.setAction(actionNewNetPage_GSPN);
        jMenuItemNewNetPage_GSPN.setText("jMenuItem2");
        jPopupMenuAddPage.add(jMenuItemNewNetPage_GSPN);

        jMenuItemNewNetPage_FullGSPN.setAction(actionNewNetPage_FullGSPN);
        jPopupMenuAddPage.add(jMenuItemNewNetPage_FullGSPN);
        jPopupMenuAddPage.add(jSeparatorPetriNets);

        jMenuItemNewDtaPage.setAction(actionNewDtaPage);
        jPopupMenuAddPage.add(jMenuItemNewDtaPage);

        jMenuItemNewMeasurePage.setAction(actionNewMeasurePage);
        jMenuItemNewMeasurePage.setText("jMenuItem2");
        jPopupMenuAddPage.add(jMenuItemNewMeasurePage);

        jMenuItemNewMultiPage.setAction(actionNewMultiPage);
        jPopupMenuAddPage.add(jMenuItemNewMultiPage);
        jPopupMenuAddPage.add(jSeparatorLibraryModels);

        jMenuLibraryModels.setText("Add library model...");
        jPopupMenuAddPage.add(jMenuLibraryModels);

        actionNewNetPage_FullGSPN.setActionName("Add a new Complete Petri net (all extensions)");
        actionNewNetPage_FullGSPN.setIcon(resourceFactory.getNewNet24());
        actionNewNetPage_FullGSPN.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewNetPage_FullGSPNActionPerformed(evt);
            }
        });

        actionNewDtaPage.setActionName("Add a new DTA");
        actionNewDtaPage.setIcon(resourceFactory.getNewDta24());
        actionNewDtaPage.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewDtaPageActionPerformed(evt);
            }
        });

        actionNewMeasurePage.setActionName("Add a new list of measures");
        actionNewMeasurePage.setIcon(resourceFactory.getNewMeasure24());
        actionNewMeasurePage.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewMeasurePageActionPerformed(evt);
            }
        });

        actionZoomIn.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_PLUS, java.awt.event.InputEvent.CTRL_MASK));
        actionZoomIn.setActionName("Zoom in");
        actionZoomIn.setIcon(resourceFactory.getZoomIn16());
        actionZoomIn.setMenuMnemonic(KeyEvent.VK_I);
        actionZoomIn.setTooltipDesc("Zoom in the current page.");

        actionZoomOut.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_MINUS, java.awt.event.InputEvent.CTRL_MASK));
        actionZoomOut.setActionName("Zoom out");
        actionZoomOut.setIcon(resourceFactory.getZoomOut16());
        actionZoomOut.setMenuMnemonic(KeyEvent.VK_O);
        actionZoomOut.setTooltipDesc("Zoom out the current page");

        actionNormalZoom.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_0, java.awt.event.InputEvent.CTRL_MASK));
        actionNormalZoom.setActionName("Normal zoom");
        actionNormalZoom.setIcon(resourceFactory.getZoomSame16());
        actionNormalZoom.setMenuMnemonic(KeyEvent.VK_N);
        actionNormalZoom.setTooltipDesc("Set the normal 100% zoom for the current page.");

        actionCut.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_X, java.awt.event.InputEvent.CTRL_MASK));
        actionCut.setActionName("Cut");
        actionCut.setIcon(resourceFactory.getCut16());
        actionCut.setMenuMnemonic(KeyEvent.VK_U);
        actionCut.setToolbarIcon(resourceFactory.getCut32());
        actionCut.setTooltipDesc("Cut the selected items into the clipboard.");
        actionCut.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionCutActionPerformed(evt);
            }
        });

        actionCopy.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_C, java.awt.event.InputEvent.CTRL_MASK));
        actionCopy.setActionName("Copy");
        actionCopy.setIcon(resourceFactory.getPageWhiteCopy16());
        actionCopy.setMenuMnemonic(KeyEvent.VK_C);
        actionCopy.setToolbarIcon(resourceFactory.getPageWhiteCopy32());
        actionCopy.setTooltipDesc("Copy the selected items into the clipboard.");
        actionCopy.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionCopyActionPerformed(evt);
            }
        });

        actionPaste.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_V, java.awt.event.InputEvent.CTRL_MASK));
        actionPaste.setActionName("Paste");
        actionPaste.setIcon(resourceFactory.getPageWhitePaste16());
        actionPaste.setMenuMnemonic(KeyEvent.VK_P);
        actionPaste.setToolbarIcon(resourceFactory.getPageWhitePaste32());
        actionPaste.setTooltipDesc("Paste items from the clipboard.");
        actionPaste.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionPasteActionPerformed(evt);
            }
        });

        actionPrint.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_P, java.awt.event.InputEvent.CTRL_MASK));
        actionPrint.setActionName("Print...");
        actionPrint.setIcon(resourceFactory.getPrinter16());
        actionPrint.setMenuMnemonic(KeyEvent.VK_P);
        actionPrint.setTooltipDesc("Print the current page.");
        actionPrint.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionPrintActionPerformed(evt);
            }
        });

        actionAbout.setActionName("About");
        actionAbout.setMenuMnemonic(KeyEvent.VK_A);
        actionAbout.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionAboutActionPerformed(evt);
            }
        });

        actionHelpGrammar.setActionName("Show grammar...");
        actionHelpGrammar.setTooltipDesc("Show the reference table of the grammar of expressions.");
        actionHelpGrammar.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionHelpGrammarActionPerformed(evt);
            }
        });

        actionStartPlay.setActionName("Start");
        actionStartPlay.setIcon(resourceFactory.getPlay16());
        actionStartPlay.setToolbarIcon(resourceFactory.getPlay32());
        actionStartPlay.setTooltipDesc("Start the simulation of the current net.");
        actionStartPlay.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartPlayActionPerformed(evt);
            }
        });

        actionStartPlayForPopupMenu.setActionName("Start Play");
        actionStartPlayForPopupMenu.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartPlayForPopupMenuActionPerformed(evt);
            }
        });

        actionEndPlay.setActionName("Close panel");
        actionEndPlay.setIcon(resourceFactory.getMediaStopRed16());
        actionEndPlay.setToolbarIcon(resourceFactory.getMediaStopRed32());
        actionEndPlay.setTooltipDesc("Close the current panel and return to the edit mode.");
        actionEndPlay.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionEndPlayActionPerformed(evt);
            }
        });

        actionStartPlaceInv.setActionName("P-semiflows");
        actionStartPlaceInv.setIcon(resourceFactory.getPinv32());
        actionStartPlaceInv.setTooltipDesc("Show the minimal P-semiflows of the current GSPN page.");
        actionStartPlaceInv.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartPlaceInvActionPerformed(evt);
            }
        });

        actionStartTransitionInv.setActionName("T-semiflows");
        actionStartTransitionInv.setIcon(resourceFactory.getTinv32());
        actionStartTransitionInv.setTooltipDesc("Show the minimal T-semiflows of the current GSPN page.");
        actionStartTransitionInv.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartTransitionInvActionPerformed(evt);
            }
        });

        actionStartBoundsFromPinv.setActionName("Place bounds");
        actionStartBoundsFromPinv.setIcon(resourceFactory.getBound32());
        actionStartBoundsFromPinv.setTooltipDesc("Show the place bounds of the current GSPN page.");
        actionStartBoundsFromPinv.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartBoundsFromPinvActionPerformed(evt);
            }
        });

        actionClearMeasureDir.setActionName("Clear cache directory");
        actionClearMeasureDir.setIcon(resourceFactory.getBroom32());
        actionClearMeasureDir.setTooltipDesc("Clear the cache directory with the intermediate solver files.");
        actionClearMeasureDir.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionClearMeasureDirActionPerformed(evt);
            }
        });

        actionImportGreatSpn.setActionName("Import GreatSPN file (net/def)...");
        actionImportGreatSpn.setTooltipDesc("Import a new GSPN page from a GreatSPN file (net/def).");
        actionImportGreatSpn.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionImportGreatSpnActionPerformed(evt);
            }
        });

        actionImportPNML.setActionName("Import PNML file...");
        actionImportPNML.setTooltipDesc("Import a new GSPN page from a PNML file.");
        actionImportPNML.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionImportPNMLActionPerformed(evt);
            }
        });

        actionStartUnfolding.setActionName("Unfold this Petri net");
        actionStartUnfolding.setToolbarIcon(resourceFactory.getUnfolding32());
        actionStartUnfolding.setTooltipDesc("Unfold the colors of this Petri net.");
        actionStartUnfolding.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartUnfoldingActionPerformed(evt);
            }
        });

        actionStartAlgebra.setActionName("Composition of nets.");
        actionStartAlgebra.setIcon(resourceFactory.getSum32());
        actionStartAlgebra.setTooltipDesc("Net composition using the tool 'algebra'.");
        actionStartAlgebra.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartAlgebraActionPerformed(evt);
            }
        });

        actionStartRG.setActionName("Show the Reachability Graph of the Petri net.");
        actionStartRG.setIcon(resourceFactory.getBuildRG32());
        actionStartRG.setTooltipDesc("Show the Reachability Graph of the Petri net model.");
        actionStartRG.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartRGActionPerformed(evt);
            }
        });

        actionStartSymRG.setActionName("Show the Symbolic Reachability Graph of the Petri net.");
        actionStartSymRG.setIcon(resourceFactory.getBuildSymRG32());
        actionStartSymRG.setTooltipDesc("Show the Symbolic Reachability Graph of the Petri net model.");
        actionStartSymRG.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartSymRGActionPerformed(evt);
            }
        });

        actionStartCTMC.setActionName("Show the Continuous Time Markov Chain of the Petri net.");
        actionStartCTMC.setIcon(resourceFactory.getBuildCTMC32());
        actionStartCTMC.setTooltipDesc("Show the Continuous Time Markov Chain of the Petri net model.");
        actionStartCTMC.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionStartCTMCActionPerformed(evt);
            }
        });

        actionAdvRapidMeasurePopup.setActionName("RapidMeasure");
        actionAdvRapidMeasurePopup.setIcon(resourceFactory.getAdvancedTools32());
        actionAdvRapidMeasurePopup.setTooltipDesc("Analyse the model with a solution tool.");

        sharedActionRestart.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_R, java.awt.event.InputEvent.CTRL_MASK));
        sharedActionRestart.setActionName("Restart");
        sharedActionRestart.setIcon(resourceFactory.getBegin16());
        sharedActionRestart.setToolbarIcon(resourceFactory.getBegin32());
        sharedActionRestart.setTooltipDesc("Restart from the initial state.");

        sharedActionPrev.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_UP, java.awt.event.InputEvent.CTRL_MASK));
        sharedActionPrev.setActionName("Previous firing");
        sharedActionPrev.setIcon(resourceFactory.getPrevious16());
        sharedActionPrev.setToolbarIcon(resourceFactory.getPrevious32());
        sharedActionPrev.setTooltipDesc("Rewind to the previous transition firing in the path.");

        sharedActionNext.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_DOWN, java.awt.event.InputEvent.CTRL_MASK));
        sharedActionNext.setActionName("Next firing");
        sharedActionNext.setIcon(resourceFactory.getNext16());
        sharedActionNext.setToolbarIcon(resourceFactory.getNext32());
        sharedActionNext.setTooltipDesc("Replay next transition firing in the path.");

        sharedActionDeleteSelected.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_BACK_SPACE, 0));
        sharedActionDeleteSelected.setActionName("Delete selected");
        sharedActionDeleteSelected.setIcon(resourceFactory.getCross16());
        sharedActionDeleteSelected.setToolbarIcon(resourceFactory.getCross32());
        sharedActionDeleteSelected.setTooltipDesc("Delete the selected object(s) in the current page.");

        sharedActionSelectAll.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_A, java.awt.event.InputEvent.CTRL_MASK));
        sharedActionSelectAll.setActionName("Select all");
        sharedActionSelectAll.setTooltipDesc("Select all the object in the current page.");

        sharedActionInvertSelection.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_A, java.awt.event.InputEvent.SHIFT_MASK | java.awt.event.InputEvent.CTRL_MASK));
        sharedActionInvertSelection.setActionName("Invert selection");
        sharedActionInvertSelection.setTooltipDesc("Invert the current selection.");

        sharedActionDeselectAll.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_ESCAPE, java.awt.event.InputEvent.SHIFT_MASK));
        sharedActionDeselectAll.setActionName("Deselect all");
        sharedActionDeselectAll.setTooltipDesc("Deselect all the selected object in the current page.");

        sharedActionExportGreatSpn.setActionName("Export in GreatSPN format...");
        sharedActionExportGreatSpn.setTooltipDesc("Export the current net in the GreatSPN file format (net/def).");

        sharedActionExportPNML.setActionName("Export in PNML format...");
        sharedActionExportPNML.setTooltipDesc("Export the current net in the PNML file format (pnml).");

        sharedActionExportGRML.setActionName("Export in GRML format...");
        sharedActionExportGRML.setTooltipDesc("Export the current net in the GRML file format (grml).");

        sharedActionExportAPNN.setActionName("Export in APNN format...");
        sharedActionExportAPNN.setTooltipDesc("Export the current net in the Abstract Petri Net Notation (APNN) file format.");

        sharedActionExportDta.setActionName("Export DTA in MC4CSLTA format...");
        sharedActionExportDta.setTooltipDesc("Export the current DTA in the MC4CSLTA format (.dta).");

        sharedActionChangeBindings.setActionName("Change bindings");
        sharedActionChangeBindings.setIcon(resourceFactory.getChangeBindings32());
        sharedActionChangeBindings.setTooltipDesc("Chnage the values bounded to the template Ids");

        sharedActionComputePlaceSemiflows.setActionName("Compute place semiflows");
        sharedActionComputePlaceSemiflows.setIcon(resourceFactory.getPinv32());
        sharedActionComputePlaceSemiflows.setTooltipDesc("Recompute the place semiflows of the GSPN.");

        sharedActionComputeTransitionSemiflows.setActionName("Compute transition semiflows");
        sharedActionComputeTransitionSemiflows.setIcon(resourceFactory.getTinv32());
        sharedActionComputeTransitionSemiflows.setTooltipDesc("Recompute the transition semiflows of the GSPN.");

        sharedActionComputeBoundsFromPinv.setActionName("Compute place bounds from P-invariants");
        sharedActionComputeBoundsFromPinv.setIcon(resourceFactory.getBound32());
        sharedActionComputeBoundsFromPinv.setTooltipDesc("Recompute the place bounds of the GSPN using the P-invariants.");

        actionCaptureSVG.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_J, java.awt.event.InputEvent.ALT_MASK | java.awt.event.InputEvent.CTRL_MASK));
        actionCaptureSVG.setActionName("Save an SVG capture of the main window.");
        actionCaptureSVG.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionCaptureSVGActionPerformed(evt);
            }
        });

        actionNewNetPage_PT.setActionName("Add a new Place/Transition Petri net");
        actionNewNetPage_PT.setIcon(resourceFactory.getNewNet24());
        actionNewNetPage_PT.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewNetPage_PTActionPerformed(evt);
            }
        });

        actionNewNetPage_CPN.setActionName("Add a new Colored Petri net");
        actionNewNetPage_CPN.setIcon(resourceFactory.getNewNet24());
        actionNewNetPage_CPN.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewNetPage_CPNActionPerformed(evt);
            }
        });

        actionNewNetPage_GSPN.setActionName("Add a new Generalized Stochastic Petri net");
        actionNewNetPage_GSPN.setIcon(resourceFactory.getNewNet24());
        actionNewNetPage_GSPN.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewNetPage_GSPNActionPerformed(evt);
            }
        });

        actionNewNetPage_SWN.setActionName("Add a new Stochastic Well-Formed (SWN) Petri net");
        actionNewNetPage_SWN.setIcon(resourceFactory.getNewNet24());
        actionNewNetPage_SWN.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewNetPage_SWNActionPerformed(evt);
            }
        });

        actionNewMultiPage.setActionName("Add a new multi page.");
        actionNewMultiPage.setIcon(resourceFactory.getNewMultiNet24());
        actionNewMultiPage.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                actionNewMultiPageActionPerformed(evt);
            }
        });

        rmcActionCTL.setActionName("Start CTL model checking.");

        rmcActionBuildRG.setActionName("Build the full Reachability Graph.");

        rmcActionBuildSymRG.setActionName("Build the full Symbolic Reachability Graph.");

        rmcActionBuildRS_MDD.setActionName("Reachability Set using Decision Diagrams.");

        rmcActionSteadyStateSol.setActionName("Start Steady State analysis.");

        rmcActionTransientSol.setActionName("Start transient analysis.");

        rmcActionSteadyStateSim.setActionName("Start Steady State simulation.");

        rmcActionTransientSim.setActionName("Start Transient simulation.");

        rmcMenuItemCTL.setAction(rmcActionCTL);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemCTL);

        rmcMenuItemBuildRG.setAction(rmcActionBuildRG);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemBuildRG);

        rmcMenuItemBuildSymRG.setAction(rmcActionBuildSymRG);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemBuildSymRG);

        rmcMenuItemBuildRS_DD.setAction(rmcActionBuildRS_MDD);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemBuildRS_DD);

        rmcMenuItemSteadyStateSol.setAction(rmcActionSteadyStateSol);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemSteadyStateSol);

        rmcMenuItemTransientSol.setAction(rmcActionTransientSol);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemTransientSol);

        rmcMenuItemSteadyStateSim.setAction(rmcActionSteadyStateSim);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemSteadyStateSim);

        rmcMenuItemTransientSim.setAction(rmcActionTransientSim);
        jPopupMenuRapidMeaureCmd.add(rmcMenuItemTransientSim);

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("Editor");
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                formWindowClosing(evt);
            }
        });

        jStatusBar.setOpaque(false);
        jStatusBar.setLayout(new java.awt.GridBagLayout());

        jStatusLabel.setText("Ok.");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.2;
        gridBagConstraints.insets = new java.awt.Insets(0, 10, 0, 0);
        jStatusBar.add(jStatusLabel, gridBagConstraints);

        jSeparator1.setOrientation(javax.swing.SwingConstants.VERTICAL);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        jStatusBar.add(jSeparator1, gridBagConstraints);

        jZoomPanelContainer.setLayout(new java.awt.GridLayout(1, 0));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 20);
        jStatusBar.add(jZoomPanelContainer, gridBagConstraints);

        getContentPane().add(jStatusBar, java.awt.BorderLayout.SOUTH);

        jPanelCenterLeft.setBorder(javax.swing.BorderFactory.createMatteBorder(1, 0, 1, 0, new java.awt.Color(153, 153, 153)));
        jPanelCenterLeft.setLayout(new java.awt.GridLayout(1, 0));

        jSplitPaneLeftCenterH.setBorder(null);

        jLeftPanel.setMinimumSize(new java.awt.Dimension(120, 120));
        jLeftPanel.setLayout(new javax.swing.BoxLayout(jLeftPanel, javax.swing.BoxLayout.LINE_AXIS));

        jSplitPaneLeftV.setBorder(null);
        jSplitPaneLeftV.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);

        jPanelProjectTree.setLayout(new java.awt.GridBagLayout());

        jScrollPaneProjects.setMinimumSize(new java.awt.Dimension(120, 120));

        javax.swing.tree.DefaultMutableTreeNode treeNode1 = new javax.swing.tree.DefaultMutableTreeNode("JTree");
        javax.swing.tree.DefaultMutableTreeNode treeNode2 = new javax.swing.tree.DefaultMutableTreeNode("project 1");
        javax.swing.tree.DefaultMutableTreeNode treeNode3 = new javax.swing.tree.DefaultMutableTreeNode("page1");
        treeNode2.add(treeNode3);
        treeNode3 = new javax.swing.tree.DefaultMutableTreeNode("page2");
        treeNode2.add(treeNode3);
        treeNode1.add(treeNode2);
        treeNode2 = new javax.swing.tree.DefaultMutableTreeNode("project 2");
        treeNode3 = new javax.swing.tree.DefaultMutableTreeNode("page1");
        treeNode2.add(treeNode3);
        treeNode3 = new javax.swing.tree.DefaultMutableTreeNode("page2");
        treeNode2.add(treeNode3);
        treeNode1.add(treeNode2);
        treeNode2 = new javax.swing.tree.DefaultMutableTreeNode("project 3");
        treeNode3 = new javax.swing.tree.DefaultMutableTreeNode("page1");
        treeNode2.add(treeNode3);
        treeNode3 = new javax.swing.tree.DefaultMutableTreeNode("page2");
        treeNode2.add(treeNode3);
        treeNode1.add(treeNode2);
        jTreeProjects.setModel(new javax.swing.tree.DefaultTreeModel(treeNode1));
        jTreeProjects.setRootVisible(false);
        jTreeProjects.setShowsRootHandles(true);
        jScrollPaneProjects.setViewportView(jTreeProjects);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        jPanelProjectTree.add(jScrollPaneProjects, gridBagConstraints);

        jPanelTreeCmds.setLayout(new javax.swing.BoxLayout(jPanelTreeCmds, javax.swing.BoxLayout.X_AXIS));

        jToolBarTree.setFloatable(false);
        jToolBarTree.setRollover(true);

        jToolbarButtonMoveUp.setAction(actionMoveUp);
        jToolBarTree.add(jToolbarButtonMoveUp);

        jToolbarButtonMoveDown.setAction(actionMoveDown);
        jToolBarTree.add(jToolbarButtonMoveDown);

        jToolbarButtonDuplicatePage.setAction(actionDuplicatePage);
        jToolbarButtonDuplicatePage.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonDuplicatePage.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolBarTree.add(jToolbarButtonDuplicatePage);

        jToolbarButtonRenamePage.setAction(actionRenamePage);
        jToolBarTree.add(jToolbarButtonRenamePage);

        jToolbarButtonDeletePage.setAction(actionDeletePage);
        jToolBarTree.add(jToolbarButtonDeletePage);

        jPanelTreeCmds.add(jToolBarTree);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        jPanelProjectTree.add(jPanelTreeCmds, gridBagConstraints);

        jSplitPaneLeftV.setLeftComponent(jPanelProjectTree);

        jPanelPropertyPane.setLayout(new java.awt.GridLayout(1, 0));

        jScrollPaneProperties.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPaneProperties.setMinimumSize(new java.awt.Dimension(120, 120));

        jPanelProperty.setLayout(new javax.swing.BoxLayout(jPanelProperty, javax.swing.BoxLayout.Y_AXIS));
        jScrollPaneProperties.setViewportView(jPanelProperty);

        jPanelPropertyPane.add(jScrollPaneProperties);

        jSplitPaneLeftV.setRightComponent(jPanelPropertyPane);

        jLeftPanel.add(jSplitPaneLeftV);

        jSplitPaneLeftCenterH.setLeftComponent(jLeftPanel);

        jRightPanel.setLayout(new java.awt.GridLayout(1, 0));

        jSplitPaneCenterErrListV.setBorder(null);
        jSplitPaneCenterErrListV.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);
        jSplitPaneCenterErrListV.setResizeWeight(1.0);

        jPanelCenter.setLayout(new javax.swing.BoxLayout(jPanelCenter, javax.swing.BoxLayout.LINE_AXIS));
        jSplitPaneCenterErrListV.setTopComponent(jPanelCenter);

        jPanelBottomError.setLayout(new java.awt.GridLayout(1, 0));

        jTablePageErrors.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null}
            },
            new String [] {
                "Title 1", "Title 2", "Title 3", "Title 4"
            }
        ));
        jTablePageErrors.setSelectionMode(javax.swing.ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        jScrollPanePageErrors.setViewportView(jTablePageErrors);

        jPanelBottomError.add(jScrollPanePageErrors);

        jSplitPaneCenterErrListV.setBottomComponent(jPanelBottomError);

        jRightPanel.add(jSplitPaneCenterErrListV);

        jSplitPaneLeftCenterH.setRightComponent(jRightPanel);

        jPanelCenterLeft.add(jSplitPaneLeftCenterH);

        getContentPane().add(jPanelCenterLeft, java.awt.BorderLayout.CENTER);

        jPanelToolBars.setOpaque(false);
        jPanelToolBars.setLayout(new javax.swing.BoxLayout(jPanelToolBars, javax.swing.BoxLayout.LINE_AXIS));

        jAppToolBar_File.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "File", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.BOTTOM));
        jAppToolBar_File.setFloatable(false);
        jAppToolBar_File.setRollover(true);

        jToolbarButtonNew.setAction(actionNewProject);
        jAppToolBar_File.add(jToolbarButtonNew);

        jDropdownAddNewPage.setAction(actionNewPage);
        jDropdownAddNewPage.setPopupMenu(jPopupMenuAddPage);
        jAppToolBar_File.add(jDropdownAddNewPage);

        jToolbarButtonOpen.setAction(actionOpen);
        jAppToolBar_File.add(jToolbarButtonOpen);

        jToolbarButtonSaveAll.setAction(actionSaveAll);
        jAppToolBar_File.add(jToolbarButtonSaveAll);

        jPanelToolBars.add(jAppToolBar_File);

        jAppToolBar_Edit.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Edit", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.BOTTOM));
        jAppToolBar_Edit.setFloatable(false);
        jAppToolBar_Edit.setRollover(true);

        jToolbarButtonUndo.setAction(actionUndo);
        jToolbarButtonUndo.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonUndo.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Edit.add(jToolbarButtonUndo);

        jToolbarButtonRedo.setAction(actionRedo);
        jToolbarButtonRedo.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonRedo.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Edit.add(jToolbarButtonRedo);
        jAppToolBar_Edit.add(jSeparator6);

        jToolbarButtonCut.setAction(actionCut);
        jToolbarButtonCut.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonCut.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Edit.add(jToolbarButtonCut);

        jToolbarButtonCopy.setAction(actionCopy);
        jToolbarButtonCopy.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonCopy.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Edit.add(jToolbarButtonCopy);

        jToolbarButtonPaste.setAction(actionPaste);
        jToolbarButtonPaste.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonPaste.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Edit.add(jToolbarButtonPaste);

        jToolbarButtonDeleteSelected.setAction(sharedActionDeleteSelected);
        jToolbarButtonDeleteSelected.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonDeleteSelected.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Edit.add(jToolbarButtonDeleteSelected);

        jPanelToolBars.add(jAppToolBar_Edit);

        jAppToolBar_Basic.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Basic", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.BOTTOM));
        jAppToolBar_Basic.setFloatable(false);
        jAppToolBar_Basic.setRollover(true);

        jToolbarButtonStartPlay.setIcon(resourceFactory.getPlay32());
        jToolbarButtonStartPlay.setToolTipText("Start the simulation of the current page.");
        jToolbarButtonStartPlay.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonStartPlay.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jToolbarButtonStartPlay.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jToolbarButtonStartPlayActionPerformed(evt);
            }
        });
        jAppToolBar_Basic.add(jToolbarButtonStartPlay);

        jToolbarButtonShowPlaceSemiflows.setAction(actionStartPlaceInv);
        jToolbarButtonShowPlaceSemiflows.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonShowPlaceSemiflows.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonShowPlaceSemiflows);

        jToolbarButtonShowTransitionSemiflows.setAction(actionStartTransitionInv);
        jToolbarButtonShowTransitionSemiflows.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonShowTransitionSemiflows.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonShowTransitionSemiflows);

        jToolbarButtonShowPlaceBoundsFromPinv.setAction(actionStartBoundsFromPinv);
        jToolbarButtonShowPlaceBoundsFromPinv.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonShowPlaceBoundsFromPinv.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonShowPlaceBoundsFromPinv);

        jToolbarButtonShowRG.setAction(actionStartRG);
        jToolbarButtonShowRG.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonShowRG.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonShowRG);

        jToolbarButtonShowSymRG.setAction(actionStartSymRG);
        jToolbarButtonShowSymRG.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonShowSymRG.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonShowSymRG);

        jToolbarButtonShowCTMC.setAction(actionStartCTMC);
        jToolbarButtonShowCTMC.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonShowCTMC.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonShowCTMC);

        jToolbarButtonUnfolding.setAction(actionStartUnfolding);
        jToolbarButtonUnfolding.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonUnfolding.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonUnfolding);

        jToolbarButtonClearMeasureDir.setAction(actionClearMeasureDir);
        jToolbarButtonClearMeasureDir.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonClearMeasureDir.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Basic.add(jToolbarButtonClearMeasureDir);

        jPanelToolBars.add(jAppToolBar_Basic);

        jAppToolBar_Advanced.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Advanced", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.BOTTOM));
        jAppToolBar_Advanced.setFloatable(false);
        jAppToolBar_Advanced.setRollover(true);

        jToolbarButtonAlgebra.setAction(actionStartAlgebra);
        jToolbarButtonAlgebra.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jToolbarButtonAlgebra.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Advanced.add(jToolbarButtonAlgebra);

        jDropdownToolbarButtonRapidMeasure.setAction(actionAdvRapidMeasurePopup);
        jDropdownToolbarButtonRapidMeasure.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jDropdownToolbarButtonRapidMeasure.setPopupMenu(jPopupMenuRapidMeaureCmd);
        jDropdownToolbarButtonRapidMeasure.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jAppToolBar_Advanced.add(jDropdownToolbarButtonRapidMeasure);

        jPanelToolBars.add(jAppToolBar_Advanced);

        jPlayToolBar.setFloatable(false);
        jPlayToolBar.setRollover(true);

        jToolbarButtonEndPlay.setAction(actionEndPlay);
        jToolbarButtonEndPlay.setText("Close.");
        jPlayToolBar.add(jToolbarButtonEndPlay);

        jPanelToolBars.add(jPlayToolBar);

        getContentPane().add(jPanelToolBars, java.awt.BorderLayout.NORTH);

        jMenuFile.setMnemonic(KeyEvent.VK_F);
        jMenuFile.setText("File");

        jMenuItem_FileNewSchema.setAction(actionNewProject);
        jMenuFile.add(jMenuItem_FileNewSchema);

        jMenuItem_FileOpen.setAction(actionOpen);
        jMenuFile.add(jMenuItem_FileOpen);

        jMenuItem_FileOpenRecent.setIcon(resourceFactory.getEmpty16());
        jMenuItem_FileOpenRecent.setText("Open recent");
        jMenuFile.add(jMenuItem_FileOpenRecent);

        jMenuItem_FileSave.setAction(actionSave);
        jMenuFile.add(jMenuItem_FileSave);

        jMenuItem_FileSaveAs.setAction(actionSaveAs);
        jMenuFile.add(jMenuItem_FileSaveAs);

        jMenuItem_FileSaveAll.setAction(actionSaveAll);
        jMenuFile.add(jMenuItem_FileSaveAll);

        jMenu_FileImport.setIcon(resourceFactory.getEmpty16());
        jMenu_FileImport.setText("Import");

        jMenuItem_GreatSpnImport.setAction(actionImportGreatSpn);
        jMenu_FileImport.add(jMenuItem_GreatSpnImport);

        jMenuItem_PNMLImport.setAction(actionImportPNML);
        jMenu_FileImport.add(jMenuItem_PNMLImport);

        jMenuFile.add(jMenu_FileImport);

        jMenuItem_FileExport.setIcon(resourceFactory.getEmpty16());
        jMenuItem_FileExport.setText("Export");

        jMenuItem_GreatSpnExport.setAction(sharedActionExportGreatSpn);
        jMenuItem_FileExport.add(jMenuItem_GreatSpnExport);

        jMenuItem_PNMLExport.setAction(sharedActionExportPNML);
        jMenuItem_FileExport.add(jMenuItem_PNMLExport);

        jMenuItem_GrMLExport.setAction(sharedActionExportGRML);
        jMenuItem_GrMLExport.setText("Export in GrML format...");
        jMenuItem_FileExport.add(jMenuItem_GrMLExport);

        jMenuItem_ApnnFormat.setAction(sharedActionExportAPNN);
        jMenuItem_FileExport.add(jMenuItem_ApnnFormat);

        jMenuItem_DtaFormat.setAction(sharedActionExportDta);
        jMenuItem_FileExport.add(jMenuItem_DtaFormat);

        jMenuFile.add(jMenuItem_FileExport);

        jMenuItem_FilePrint.setAction(actionPrint);
        jMenuFile.add(jMenuItem_FilePrint);

        jMenuItem_FileCloseProj.setAction(actionClose);
        jMenuFile.add(jMenuItem_FileCloseProj);
        jMenuFile.add(jSeparator2);

        jMenuItem_FileExit.setAction(actionExit);
        jMenuFile.add(jMenuItem_FileExit);

        jAppMenuBar.add(jMenuFile);

        jMenuEdit.setMnemonic(KeyEvent.VK_E);
        jMenuEdit.setText("Edit");

        jMenuItem_EditUndo.setAction(actionUndo);
        jMenuEdit.add(jMenuItem_EditUndo);

        jMenuItem_EditRedo.setAction(actionRedo);
        jMenuEdit.add(jMenuItem_EditRedo);
        jMenuEdit.add(jSeparator5);

        jMenuItem_EditCut.setAction(actionCut);
        jMenuEdit.add(jMenuItem_EditCut);

        jMenuItem_EditCopy.setAction(actionCopy);
        jMenuEdit.add(jMenuItem_EditCopy);

        jMenuItem_EditPaste.setAction(actionPaste);
        jMenuEdit.add(jMenuItem_EditPaste);
        jMenuEdit.add(jSeparator8);

        jMenuItem_EditSelectAll.setAction(sharedActionSelectAll);
        jMenuEdit.add(jMenuItem_EditSelectAll);

        jMenuItem_EditDeselectAll.setAction(sharedActionDeselectAll);
        jMenuEdit.add(jMenuItem_EditDeselectAll);

        jMenuItem_EditInvertSelection.setAction(sharedActionInvertSelection);
        jMenuEdit.add(jMenuItem_EditInvertSelection);

        jMenuItem_EditDeleteSelected.setAction(sharedActionDeleteSelected);
        jMenuEdit.add(jMenuItem_EditDeleteSelected);
        jMenuEdit.add(jSeparator4);

        jMenuItemOptions.setAction(actionAppPreferences);
        jMenuEdit.add(jMenuItemOptions);

        jAppMenuBar.add(jMenuEdit);

        jMenuView.setMnemonic(KeyEvent.VK_V);
        jMenuView.setText("View");

        jMenuItem1.setAction(actionNormalZoom);
        jMenuView.add(jMenuItem1);

        jMenuItem_ViewZoomIn.setAction(actionZoomIn);
        jMenuView.add(jMenuItem_ViewZoomIn);

        jMenuItem_ViewZoomOut.setAction(actionZoomOut);
        jMenuView.add(jMenuItem_ViewZoomOut);

        jAppMenuBar.add(jMenuView);

        jMenuSimulation.setText("Simulation");

        jMenuItem_SimStart.setAction(actionStartPlay);
        jMenuSimulation.add(jMenuItem_SimStart);

        jMenu_SimModelCheck.setIcon(resourceFactory.getPlay16());
        jMenu_SimModelCheck.setText("Model check with...");
        jMenuSimulation.add(jMenu_SimModelCheck);
        jMenuSimulation.add(jSeparator10);

        jMenuItem_SimRestart.setAction(sharedActionRestart);
        jMenuSimulation.add(jMenuItem_SimRestart);

        jMenuItem_SimPrev.setAction(sharedActionPrev);
        jMenuSimulation.add(jMenuItem_SimPrev);

        jMenuItem_SimNext.setAction(sharedActionNext);
        jMenuSimulation.add(jMenuItem_SimNext);
        jMenuSimulation.add(jSeparator9);

        jMenuItem_SimClose.setAction(actionEndPlay);
        jMenuSimulation.add(jMenuItem_SimClose);

        jAppMenuBar.add(jMenuSimulation);

        jMenuHelp.setText("Help");

        jMenuItem_HelpAbout.setAction(actionAbout);
        jMenuHelp.add(jMenuItem_HelpAbout);

        jMenuItem_HelpShowGrammar.setAction(actionHelpGrammar);
        jMenuHelp.add(jMenuItem_HelpShowGrammar);

        jAppMenuBar.add(jMenuHelp);

        setJMenuBar(jAppMenuBar);

        pack();
    }// </editor-fold>//GEN-END:initComponents
    
    private void formWindowClosing(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_formWindowClosing
        quitApplication();
    }//GEN-LAST:event_formWindowClosing

    private static GspnPage newGspnPageOfType(ProjectFile proj, NewProjectDialog.PetriNetType type) {
        GspnPage page = new GspnPage();
        String pageName = type.netPrefix;
        page.viewProfile.setProfileForNetType(type);
        if (proj != null)
            pageName = generateUniquePageName(proj, pageName);
        page.setPageName(pageName);
//        
//        Place p;
//        page.nodes.add(p = new Place("P0", "2", TokenType.DISCRETE, "", "", new Point2D.Double(2, 2)));
//        page.nodes.add(new NodeRef(p, "REF0", new Point2D.Double(5, 2)));
//        
        return page;
    }
    
    private void actionNewProjectActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewProjectActionPerformed
        NewProjectDialog dlg = new NewProjectDialog(this, true);
        dlg.setVisible(true);
        if (dlg.canceled)
            return;

        ArrayList<ProjectPage> schemas = new ArrayList<>();
        GspnPage gspnPage = null;
        if (dlg.getStartWithGspn()) {
            gspnPage = newGspnPageOfType(null, dlg.getStartPNType());
            schemas.add(gspnPage);
        }
        if (dlg.getStartWithDta()) {
            DtaPage dtaPage = new DtaPage();
            dtaPage.setPageName("DTA");
            schemas.add(dtaPage);
            dtaPage.nodes.add(new ClockVar("x", new Point2D.Double(2, 2)));
        }
        if (dlg.getStartWithMeasures() && gspnPage != null) {
            MeasurePage mPage = new MeasurePage();
            mPage.setPageName("Measures");
            if (dlg.getStartWithGspn())
                mPage.targetGspnName = gspnPage.getPageName();
            mPage.measures.add(new FormulaMeasure("Basic statistics of the toolchain execution.", 
                               false, "", FormulaLanguage.STAT));
            if (gspnPage != null && 
                (gspnPage.viewProfile.showTimedCommands||
                 gspnPage.viewProfile.viewRatesDelays))
                mPage.measures.add(new FormulaMeasure("All the basic Petri net measures", 
                                false, "", FormulaLanguage.ALL));
            schemas.add(mPage);
        }
        String text = dlg.getProjectName();
        ProjectData data = new ProjectData(text.length() == 0 ? "New project" : text, schemas);
        ProjectFile proj = new ProjectFile(data);
        proj.getCurrent().checkProjectCorrectness(null);

        // Add the new project in the observed list of the GUI, and switch to it.
        proj.addObserver(projectChangedObs);
        projects.add(proj);
        if (proj.getCurrent().getPageCount() > 0)
            proj.getCurrent().setActivePage(proj.getCurrent().getPageAt(0));
        switchToProjectPage(proj, proj.getCurrent().getActivePage(), null);
        expandedTreeObjects.add(proj.getObjectUUID());
        setStatus(null, true);
        invalidateGUI();
    }//GEN-LAST:event_actionNewProjectActionPerformed

    private void actionOpenActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionOpenActionPerformed
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.addChoosableFileFilter(ProjectFile.getFileFilter());
        fileChooser.setFileFilter(ProjectFile.getFileFilter());
        String curDir = Util.getPreferences().get("open-dir", null);
        fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
        if (fileChooser.showOpenDialog(AppWindow.this) != JFileChooser.APPROVE_OPTION) 
            return;
        File file = fileChooser.getSelectedFile();
        curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
        System.out.println("curDir: "+curDir);
        Util.getPreferences().put("open-dir", curDir);
        
        // Check if the file is already open
        for (ProjectFile pf : projects) {
            if (pf.getFilename() != null && pf.getFilename().equals(file)) {
                JOptionPane.showMessageDialog(AppWindow.this,
                                          "The project "+file+" is already open.", 
                                          "Open project.",
                                          JOptionPane.ERROR_MESSAGE);
                // Select it
                selectedNodeId = pf.getObjectUUID();
                activeProject = pf;
                invalidateGUI();
                setStatus(null, true);
                return;
            }
        }
        // load from file
        openFile(file);
    }//GEN-LAST:event_actionOpenActionPerformed
        
    private void actionSaveActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionSaveActionPerformed
        saveProject(activeProject, false);
    }//GEN-LAST:event_actionSaveActionPerformed

    private void actionSaveAsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionSaveAsActionPerformed
        saveProject(activeProject, true);
    }//GEN-LAST:event_actionSaveAsActionPerformed

    private void actionSaveAllActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionSaveAllActionPerformed
        for (ProjectFile pf : projects) {
            if (!pf.isSaved()) {
                if (!saveProject(pf, false)) 
                    return; // The user canceled one of the savings
            }
        }
    }//GEN-LAST:event_actionSaveAllActionPerformed

    private void actionCloseActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionCloseActionPerformed
        if (activeProject != null)
            closeProject(activeProject);
    }//GEN-LAST:event_actionCloseActionPerformed

    private void actionExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionExitActionPerformed
        if (evt.getActionCommand().equals(actionExit.getActionName()))
            quitApplication();
    }//GEN-LAST:event_actionExitActionPerformed

    private void actionUndoActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionUndoActionPerformed
        if (!hasActiveProject() || !activeProject.canUndo())
            return;
        String descr = activeProject.getUndoDesc();
        activeProject.undo();
        switchToProjectPage(activeProject, activeProject.getCurrent().getActivePage(), null);
        setStatus("undo "+descr, true);
    }//GEN-LAST:event_actionUndoActionPerformed

    private void actionRedoActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionRedoActionPerformed
        if (!hasActiveProject() || !activeProject.canRedo())
            return;
        String descr = activeProject.getRedoDesc();
        activeProject.redo();
        switchToProjectPage(activeProject, activeProject.getCurrent().getActivePage(), null);
        setStatus("redo "+descr, true);
    }//GEN-LAST:event_actionRedoActionPerformed

    private void actionMoveUpActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionMoveUpActionPerformed
        moveSelectedPage(-1, "move page up.");
    }//GEN-LAST:event_actionMoveUpActionPerformed

    private void actionMoveDownActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionMoveDownActionPerformed
        moveSelectedPage(+1, "move page down.");
    }//GEN-LAST:event_actionMoveDownActionPerformed

    private void actionDeletePageActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionDeletePageActionPerformed
        executeUndoableCommand("delete page.", (ProjectData proj, ProjectPage elem) -> {
            assert elem != null;
            int pos = proj.findPagePosition(elem);
            proj.deletePage(pos);
            if (proj.getPageCount() == 0)
                switchToProjectPage(activeProject, null, null);
            else
                switchToProjectPage(activeProject, proj.getPageAt(Math.max(0,pos-1)), null);
        });
    }//GEN-LAST:event_actionDeletePageActionPerformed

    private void actionRenamePageActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionRenamePageActionPerformed
        jTreeProjects.startEditingAtPath(jTreeProjects.getSelectionPath());
    }//GEN-LAST:event_actionRenamePageActionPerformed

    private void actionNewNetPage_FullGSPNActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewNetPage_FullGSPNActionPerformed
        executeUndoableCommand("new Petri net.", (ProjectData proj, ProjectPage elem) -> {
            ProjectPage newPage = newGspnPageOfType(activeProject, NewProjectDialog.PetriNetType.FullPN);
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    };//GEN-LAST:event_actionNewNetPage_FullGSPNActionPerformed

    private void actionNewDtaPageActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewDtaPageActionPerformed
        executeUndoableCommand("new DTA.", (ProjectData proj, ProjectPage elem) -> {
            DtaPage newPage = new DtaPage();
            newPage.setPageName(generateUniquePageName(activeProject, "DTA"));
            newPage.nodes.add(new ClockVar("x", new Point2D.Double(2, 2)));
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    }//GEN-LAST:event_actionNewDtaPageActionPerformed

    private void actionAppPreferencesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAppPreferencesActionPerformed
        PreferencesDialog prefDlg = new PreferencesDialog(this, true);
        setStatus(null, true);
        prefDlg.setVisible(true);
    }//GEN-LAST:event_actionAppPreferencesActionPerformed
    
    private void actionCutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionCutActionPerformed
        ccpEngine.forwardAction(evt, "CUT");
    }//GEN-LAST:event_actionCutActionPerformed

    private void actionCopyActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionCopyActionPerformed
        ccpEngine.forwardAction(evt, "COPY");
    }//GEN-LAST:event_actionCopyActionPerformed

    private void actionPasteActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionPasteActionPerformed
        ccpEngine.forwardAction(evt, "PASTE");
    }//GEN-LAST:event_actionPasteActionPerformed

    private void actionPrintActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionPrintActionPerformed
        PrinterJob printJob = PrinterJob.getPrinterJob();
        printJob.setPrintable(new Printable() {
            @Override
            public int print(Graphics grphcs, PageFormat pf, int pageIndex) throws PrinterException {
                if (pageIndex > 0)
                    return NO_SUCH_PAGE;
                
                Graphics2D g2d = (Graphics2D)grphcs;
                activeProject.getCurrent().getActivePage().print(g2d, pf);
                    
                return PAGE_EXISTS; // Print the first page
            }
        });
        printJob.setJobName("Print of "+activeProject.getCurrent().getActivePage().getPageName());
        if (printJob.printDialog()) {
            try {
                printJob.print();
                setStatus("printed.", true);
            } catch (PrinterException pe) {
                JOptionPane.showMessageDialog(AppWindow.this, "Could not print the document.\n"+pe.getMessage(), 
                                              "Print error.", JOptionPane.ERROR_MESSAGE);
                setStatus("could not print.", false);
            }
        }
    }//GEN-LAST:event_actionPrintActionPerformed

    private void actionAboutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionAboutActionPerformed
        AboutDialog aboutDlg = new AboutDialog(this, true);
        setStatus(null, true);
        aboutDlg.setVisible(true);
    }//GEN-LAST:event_actionAboutActionPerformed

    private void jToolbarButtonStartPlayActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jToolbarButtonStartPlayActionPerformed
        assert hasActiveProject();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        PlayCommand cmd = page.canPlay(activeProject);
        assert cmd != PlayCommand.NO;
        if (cmd == PlayCommand.IMMEDIATE) {
            actionStartPlayForPopupMenuActionPerformed(null);
        }
        else { // DROPDOWN
            JPopupMenu popup = page.getPlayDropdownMenu(activeProject, actionStartPlayForPopupMenu);
            popup.setLayout(new BoxLayout(popup, BoxLayout.PAGE_AXIS));
            Util.reformatMenuPanels(popup);
            popup.show(jToolbarButtonStartPlay, 0, jToolbarButtonStartPlay.getHeight());
        }
    }//GEN-LAST:event_jToolbarButtonStartPlayActionPerformed

    private void actionStartPlayForPopupMenuActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartPlayForPopupMenuActionPerformed
        assert hasActiveProject();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        AbstractPageEditor modalPanel = page.getPlayWindow(this, evt != null ? (JMenuItem)evt.getSource() : null);
        assert modalPanel != null && modalPanel.isModalEditor();
        // Activate the modal panel
        switchToProjectPage(activeProject, page, modalPanel);
        setStatus(null, true);
        invalidateGUI();
    }//GEN-LAST:event_actionStartPlayForPopupMenuActionPerformed

    private void actionEndPlayActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionEndPlayActionPerformed
        assert hasActiveProject() && activeEditor != null && activeEditor.isModalEditor();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        assert (page != null);
        switchToProjectPage(activeProject, page, null/* close the modal play panel */);
        setStatus(null, true);
        invalidateGUI();
    }//GEN-LAST:event_actionEndPlayActionPerformed

    private void actionStartPlayActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartPlayActionPerformed
        assert hasActiveProject();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        PlayCommand cmd = page.canPlay(activeProject);
        assert cmd == PlayCommand.IMMEDIATE;
        actionStartPlayForPopupMenuActionPerformed(null);
    }//GEN-LAST:event_actionStartPlayActionPerformed

    private NetSemiflowsPanel netSemiflowsPanel;
    
    private void actionStartPlaceInvActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartPlaceInvActionPerformed
        assert hasActiveProject();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        assert page != null && page.isPageCorrect() && page.hasPlaceTransInv();
        if (netSemiflowsPanel == null)
            netSemiflowsPanel = new NetSemiflowsPanel(this);
        netSemiflowsPanel.setSemiflowType(SemiFlows.Type.PLACE_SEMIFLOW);
        // Activate the modal panel
        switchToProjectPage(activeProject, page, netSemiflowsPanel);
        setStatus(null, true);
        invalidateGUI();
    }//GEN-LAST:event_actionStartPlaceInvActionPerformed

    private void actionStartTransitionInvActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartTransitionInvActionPerformed
        assert hasActiveProject();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        assert page != null && page.isPageCorrect() && page.hasPlaceTransInv();
        if (netSemiflowsPanel == null)
            netSemiflowsPanel = new NetSemiflowsPanel(this);
        netSemiflowsPanel.setSemiflowType(SemiFlows.Type.TRANSITION_SEMIFLOWS);
        // Activate the modal panel
        switchToProjectPage(activeProject, page, netSemiflowsPanel);
        setStatus(null, true);
        invalidateGUI();
    }//GEN-LAST:event_actionStartTransitionInvActionPerformed

    private void actionImportGreatSpnActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionImportGreatSpnActionPerformed
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.addChoosableFileFilter(GreatSpnFormat.fileFilter);
        fileChooser.setFileFilter(GreatSpnFormat.fileFilter);
        String curDir = Util.getPreferences().get("greatspn-import-dir", System.getProperty("user.home"));
        fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
        fileChooser.setDialogTitle("Import GSPN in GreatSPN format (net/def)...");
        if (fileChooser.showOpenDialog(AppWindow.this) != JFileChooser.APPROVE_OPTION) 
            return;
        curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
        Util.getPreferences().put("greatspn-import-dir", curDir);
        
        // Generate the .net and .def filenames
        File netFile = fileChooser.getSelectedFile(), defFile;
        String path = netFile.getPath();
        int lastDot = path.lastIndexOf(".");
        //System.out.println("path.substring(lastDot) = "+path.substring(lastDot));
        if (lastDot != -1 && path.substring(lastDot).equalsIgnoreCase(".net"))
            path = path.substring(0, lastDot);
        netFile = new File(path + ".net");
        defFile = new File(path + ".def");
        
        System.out.println("netFile = "+netFile);
        System.out.println("defFile = "+defFile);
        
        try {
            final GspnPage gspn = new GspnPage();
            gspn.setPageName(generateUniquePageName(activeProject, new File(path).getName()));
            String log = GreatSpnFormat.importGspn(gspn, netFile, defFile);
            if (log != null)
                new ModalLogDialog(this, log).setVisible(true);
            
            executeUndoableCommand("import GSPN from GreatSPN format.", (ProjectData proj, ProjectPage elem) -> {
                activeProject.getCurrent().addPage(gspn);
                switchToProjectPage(activeProject, gspn, null);
            });
            setStatus(netFile.getAbsolutePath()+" imported.", true);
        }
        catch (Exception e) {
            Main.logException(e, true);
            JOptionPane.showMessageDialog(this, 
                                          "An error happened while imporing the GSPN from GreatSPN format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Import \""+netFile.getAbsolutePath()+"\"...", 
                                          JOptionPane.ERROR_MESSAGE);            
            setStatus("could not import "+netFile.getAbsolutePath(), false);
        }
    }//GEN-LAST:event_actionImportGreatSpnActionPerformed

    private void actionNewMeasurePageActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewMeasurePageActionPerformed
        executeUndoableCommand("new list of measures.", (ProjectData proj, ProjectPage elem) -> {
            ProjectPage newPage = new MeasurePage();
            newPage.setPageName(generateUniquePageName(activeProject, "Measures"));
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    }//GEN-LAST:event_actionNewMeasurePageActionPerformed

    private void actionCaptureSVGActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionCaptureSVGActionPerformed
        // Keycode is Ctrl+Alt+J or Cmd+Alt+J
//        System.out.println("Capture!!");
//        // Need all the batik libraries, including the ones in lib/ of the batik distribution
//        DOMImplementation domImpl = GenericDOMImplementation.getDOMImplementation();
//        String svgNS = "http://www.w3.org/2000/svg";
//        org.w3c.dom.Document document = domImpl.createDocument(svgNS, "svg", null);
//        SVGGeneratorContext ctx = SVGGeneratorContext.createDefault(document);
//
//        SVGGraphics2D g2 = new SVGGraphics2D(ctx, false);
//        g2.setSVGCanvasSize(getSize());
//        paint(g2);
//        boolean useCSS = true;
//        try {
//            int i=0;
//            File outSVG;
//            do {
//                outSVG = new File("capture-"+(i++)+".svg");
//            } while (outSVG.exists());
//            FileOutputStream svgs = new FileOutputStream(outSVG);
//            Writer out = new OutputStreamWriter(svgs, "UTF-8");
//            g2.stream(out, useCSS);
//            svgs.flush();
//            svgs.close();
//            
////            AbstractFOPTranscoder trans = new PDFTranscoder();
////            TranscoderInput input = new TranscoderInput(new FileInputStream(outSVG));
////            OutputStream os = new FileOutputStream(outPDF);
////            TranscoderOutput output = new TranscoderOutput(os);
////            trans.transcode(input, output);
////            os.flush();
////            os.close();
//            System.out.println("Capture "+outSVG.getAbsolutePath()+" taken.");
//        }
//        catch (IOException e) {
//            Main.logException(e, true);
//        }          
    }//GEN-LAST:event_actionCaptureSVGActionPerformed

    private void actionClearMeasureDirActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionClearMeasureDirActionPerformed
        assert hasActiveProject();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        assert page != null && page.hasClearMeasureCmd();
        String err = page.clearMeasures(activeProject.getFilename(), this);
        setStatus(err, true);
        invalidateGUI();
    }//GEN-LAST:event_actionClearMeasureDirActionPerformed

    private void actionNewNetPage_PTActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewNetPage_PTActionPerformed
        executeUndoableCommand("new Place/Transition Petri net.", (ProjectData proj, ProjectPage elem) -> {
            ProjectPage newPage = newGspnPageOfType(activeProject, NewProjectDialog.PetriNetType.PT);
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    }//GEN-LAST:event_actionNewNetPage_PTActionPerformed

    private void actionNewNetPage_SWNActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewNetPage_SWNActionPerformed
        executeUndoableCommand("new Stochastic Colored (Well-Formed) Petri net.", (ProjectData proj, ProjectPage elem) -> {
            ProjectPage newPage = newGspnPageOfType(activeProject, NewProjectDialog.PetriNetType.SWN);
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    }//GEN-LAST:event_actionNewNetPage_SWNActionPerformed

    private void actionNewNetPage_GSPNActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewNetPage_GSPNActionPerformed
        executeUndoableCommand("new Generalized Stochastic Petri net.", (ProjectData proj, ProjectPage elem) -> {
            ProjectPage newPage = newGspnPageOfType(activeProject, NewProjectDialog.PetriNetType.GSPN);
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    }//GEN-LAST:event_actionNewNetPage_GSPNActionPerformed

    private void actionNewNetPage_CPNActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewNetPage_CPNActionPerformed
        executeUndoableCommand("new Colored (Well-Formed) Petri net.", (ProjectData proj, ProjectPage elem) -> {
            ProjectPage newPage = newGspnPageOfType(activeProject, NewProjectDialog.PetriNetType.CPN);
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    }//GEN-LAST:event_actionNewNetPage_CPNActionPerformed

    private void actionStartUnfoldingActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartUnfoldingActionPerformed
        assert hasActiveProject();
//        final TemplateBinding binding = ParameterAssignmentDialog.askParamAssignment(this, 
//                (GspnPage)activeProject.getCurrent().getActivePage());
        
        executeUndoableCommand("Unfold Petri net.", (ProjectData proj, ProjectPage elem) -> {
            assert elem != null && elem.canBeUnfolded();
            final Unfolding u = new Unfolding((GspnPage)elem);
            u.unfold();
            String uniqueName = generateUniquePageName(activeProject, "Unfolding of "+elem.getPageName());
            u.unfolded.setPageName(uniqueName);
            activeProject.getCurrent().addPage(u.unfolded);
            switchToProjectPage(activeProject, u.unfolded, null);
        });
//        catch (CouldNotUnfoldException e) {
//            Main.logException(e, true);
//        }
    }//GEN-LAST:event_actionStartUnfoldingActionPerformed

    private void actionNewMultiPageActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionNewMultiPageActionPerformed
        executeUndoableCommand("new Multi net page.", (ProjectData proj, ProjectPage elem) -> {
            MultiNetPage newPage = new MultiNetPage();
            newPage.viewProfile.setProfileForNetType(NewProjectDialog.PetriNetType.FullPN);
            newPage.setPageName(generateUniquePageName(activeProject, "MultiNet"));
            newPage.netsDescr.add(new NetInstanceDescriptor());
            newPage.netsDescr.add(new NetInstanceDescriptor());
            newPage.netsDescr.get(0).targetNetName = "PN1";
            newPage.netsDescr.get(1).targetNetName = "PN2";
            activeProject.getCurrent().addPage(newPage);
            switchToProjectPage(activeProject, newPage, null);
        });
    }//GEN-LAST:event_actionNewMultiPageActionPerformed

    private void actionDuplicatePageActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionDuplicatePageActionPerformed
        executeUndoableCommand("duplicate page.", (ProjectData proj, ProjectPage page) -> {
            // Duplicate the page and make it different (new UUID)
            ProjectPage dupPage = (ProjectPage)Util.deepCopyRelink(page);
            dupPage.generateNewUUID();
            // Rename the page.
            while (proj.findPageByName(dupPage.getPageName()) != null) {
                dupPage.setPageName("copy of "+dupPage.getPageName());
            }
            proj.addPageAt(dupPage, proj.findPagePosition(page)+1);
            switchToProjectPage(activeProject, dupPage, null);
        });
    }//GEN-LAST:event_actionDuplicatePageActionPerformed

    private void actionHelpGrammarActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionHelpGrammarActionPerformed
        HtmlPageReader.openPage(this);
    }//GEN-LAST:event_actionHelpGrammarActionPerformed

    private void addPagePopupMenuBecomesVisible(javax.swing.event.PopupMenuEvent evt) {//GEN-FIRST:event_addPagePopupMenuBecomesVisible
        jMenuLibraryModels.removeAll();
        // Add all the models in the library directory
        File dir = new File(Main.getModelLibraryDirectory());
        File[] models = dir.listFiles(new java.io.FileFilter() {
            @Override
            public boolean accept(File pathname) {
                return pathname.getName().endsWith(ProjectFile.PNPRO_EXT);
            }
        });
        if (models == null || models.length == 0) {
            jMenuLibraryModels.setEnabled(false);
            return;
        }
        for (final File model : models) {
            final String modelName = model.getName().replace("."+ProjectFile.PNPRO_EXT, "");
            JMenuItem item = new JMenuItem(modelName);
            item.addActionListener((ActionEvent event) -> {
                try {
                    final ProjectFile newProj = PnProFormat.readXML(model);
                    
                    executeUndoableCommand("add "+modelName, (ProjectData proj, ProjectPage page) -> {
                        boolean first = true;
                        for (int p = 0; p < newProj.getCurrent().getPageCount(); p++) {
                            ProjectPage newPage = newProj.getCurrent().getPageAt(p);
                            String newPageName = newPage.getPageName();
                            newPageName = generateUniquePageName(activeProject, newPageName);
                            if (!newPageName.equals(newPage.getPageName())) {
                                for (int p2 = 0; p2 < newProj.getCurrent().getPageCount(); p2++)
                                    if (p != p2)
                                        newProj.getCurrent().getPageAt(p2).onAnotherPageRenaming(newPage.getPageName(), newPageName);
                            }
                            newPage.setPageName(newPageName);
                            proj.addPage(newPage);
                            if (first) {
                                switchToProjectPage(activeProject, newPage, null);
                                first = false;
                            }
                        }
                    });
                    setStatus("Library model "+modelName+" added.", true);
                }
                catch (Exception e) {
                    Main.logException(e, true);
                    JOptionPane.showMessageDialog(AppWindow.this,
                            "Could not open the Library model "+modelName+".\n"+
                                    "Reason: "+e.getMessage(),
                            "Could not open file.",
                            JOptionPane.ERROR_MESSAGE);
                    setStatus("could not open library model "+modelName, true);    
                }
            });
            jMenuLibraryModels.add(item);
        }
    }//GEN-LAST:event_addPagePopupMenuBecomesVisible

    private void actionStartAlgebraActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartAlgebraActionPerformed
        AlgebraToolDialog dlg = new AlgebraToolDialog(this, true);
        final GspnPage newPage = dlg.openDialogCompose(
                (GspnPage)activeProject.getCurrent().getActivePage(),
                activeProject.getCurrent());
        if (newPage != null) {
            executeUndoableCommand("net composition.", (ProjectData proj, ProjectPage page) -> {
                newPage.setPageName(generateUniquePageName(activeProject, newPage.getPageName()));
                activeProject.getCurrent().addPage(newPage);
                switchToProjectPage(activeProject, newPage, null);
                setStatus("Net \""+newPage.getPageName()+"\" composed with algebra.", true);
            });
        }
    }//GEN-LAST:event_actionStartAlgebraActionPerformed

    private void actionStartBoundsFromPinvActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartBoundsFromPinvActionPerformed
        assert hasActiveProject();
        ProjectPage page = activeProject.getCurrent().getActivePage();
        assert page != null && page.isPageCorrect() && page.hasPlaceTransInv();
        if (netSemiflowsPanel == null)
            netSemiflowsPanel = new NetSemiflowsPanel(this);
        netSemiflowsPanel.setSemiflowType(SemiFlows.Type.PLACE_BOUNDS_FROM_PINV);
        // Activate the modal panel
        switchToProjectPage(activeProject, page, netSemiflowsPanel);
        setStatus(null, true);
        invalidateGUI();
    }//GEN-LAST:event_actionStartBoundsFromPinvActionPerformed

    private void actionImportPNMLActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionImportPNMLActionPerformed
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.addChoosableFileFilter(PNMLFormat.fileFilter);
        fileChooser.setFileFilter(PNMLFormat.fileFilter);
        String curDir = Util.getPreferences().get("pnml-import-dir", System.getProperty("user.home"));
        fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
        fileChooser.setDialogTitle("Import GSPN in PNML format...");
        if (fileChooser.showOpenDialog(AppWindow.this) != JFileChooser.APPROVE_OPTION) 
            return;
        curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
        Util.getPreferences().put("pnml-import-dir", curDir);
        
        // Generate the .net and .def filenames
        File pnmlFile = fileChooser.getSelectedFile();
        String path = pnmlFile.getPath();
        int lastDot = path.lastIndexOf(".");
        //System.out.println("path.substring(lastDot) = "+path.substring(lastDot));
        if (lastDot != -1 && path.substring(lastDot).equalsIgnoreCase(".pnml"))
            path = path.substring(0, lastDot);
        pnmlFile = new File(path + ".pnml");
        
        try {
            final GspnPage gspn = new GspnPage();
//            gspn.setPageName(generateUniquePageName(activeProject, new File(path).getName()));
            Map<String, String> id2name = new TreeMap<>();
            String log = PNMLFormat.importPNML(gspn, pnmlFile, id2name, null);
            if (log != null)
                new ModalLogDialog(this, log).setVisible(true);
            // check that a project is open
            if (activeProject == null) {
                throw new InvalidOperationException("Import of PNML files is only available inside a project. Please create one.");

            // Avoid duplicate page names
            } else if (activeProject.getCurrent().findPageByName(gspn.getPageName()) != null)
                gspn.setPageName(generateUniquePageName(activeProject, gspn.getPageName()));
            
            executeUndoableCommand("import GSPN from PNML format.", (ProjectData proj, ProjectPage elem) -> {
                activeProject.getCurrent().addPage(gspn);
                switchToProjectPage(activeProject, gspn, null);
            });
            setStatus(pnmlFile.getAbsolutePath()+" imported.", true);
        }
        catch (Exception e) {
            Main.logException(e, true);
            JOptionPane.showMessageDialog(this, 
                                          "An error happened while imporing the GSPN from PNML format.\n"
                                          + "Reason: "+e.getMessage(),
                                          "Import \""+pnmlFile.getAbsolutePath()+"\"...", 
                                          JOptionPane.ERROR_MESSAGE);            
            setStatus("could not import "+pnmlFile.getAbsolutePath(), false);
        }
    }//GEN-LAST:event_actionImportPNMLActionPerformed

    private void actionStartRGActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartRGActionPerformed
        startRGTool(RgType.RG);
    }//GEN-LAST:event_actionStartRGActionPerformed

    private void actionStartSymRGActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartSymRGActionPerformed
        startRGTool(RgType.SRG);
    }//GEN-LAST:event_actionStartSymRGActionPerformed

    private void actionStartCTMCActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_actionStartCTMCActionPerformed
        startRGTool(RgType.CTMC);
    }//GEN-LAST:event_actionStartCTMCActionPerformed

    
    
    public void startRGTool(RgType rgType) {
        assert hasActiveProject();
        final GspnPage page =  (GspnPage)activeProject.getCurrent().getActivePage();
        final TemplateBinding binding = ParameterAssignmentDialog.askParamAssignment(this, page);
        if (binding == null)
            return; // cancel RG
        
        ShowRgDialog dlg = new ShowRgDialog(this, page, binding, rgType);
        String result = dlg.showRG();
        
        if (result == null) {
            setStatus("Reachability Graph shown.", true);
        }
        else {
            JOptionPane.showMessageDialog(this, result, "Could not generate the RG.", 
                                          JOptionPane.ERROR_MESSAGE);
            setStatus("Could not show the Reachability Graph.", false);
        }
    }

    public void openFile(File file) {
        ProjectFile proj;
        try {
            proj = PnProFormat.readXML(file);
            
            // Set up the project object
            proj.setFilename(file);
            proj.getCurrent().checkProjectCorrectness(null);
            
            // Set the project observer and attach it to the project tree
            proj.addObserver(projectChangedObs);
            projects.add(proj);
            if (proj.getCurrent().getPageCount() > 0) 
                proj.getCurrent().setActivePage(proj.getCurrent().getPageAt(0));
            switchToProjectPage(proj, proj.getCurrent().getActivePage(), null);
            expandedTreeObjects.add(proj.getObjectUUID());
            recentFilesList.open(file);
            invalidateGUI();
            setStatus(file.getAbsolutePath()+" opened.", true);
        }
        catch (Exception e) {
            Main.logException(e, true);
            JOptionPane.showMessageDialog(AppWindow.this, 
                                          "Could not open the project "+file+".\n"+
                                          "Reason: "+e.getMessage(), 
                                          "Could not open file.",
                                          JOptionPane.ERROR_MESSAGE);
            recentFilesList.remove(file);
            setStatus("could not open "+file.getAbsolutePath(), true);
        }        
    }

    private boolean quitApplication() {
        // Test if there are still unsaved documents
        while (!projects.isEmpty()) {
            if (!closeProject(projects.get(projects.size() - 1)))
                return false; // Interrupt the closing operation
        }

        // save the window geometry to the preferences
        Preferences prefs = Util.getPreferences();
        Util.saveFramePosition(this, "main");
        prefs.putInt("frame-divLV", jSplitPaneLeftV.getDividerLocation());
        prefs.putInt("frame-divLCH", jSplitPaneLeftCenterH.getDividerLocation());
        recentFilesList.saveList();

        // close the window and exit the application
        setVisible(false);
        dispose();
        
        LatexProvider.getProvider().quit();
        LogWindow.closeLogWindow();
        
        // Ensure that other windowClose() listeners are called before exiting.
        SwingUtilities.invokeLater(() -> System.exit(0) );
        return true;
    }
    
    private static String generateUniquePageName(ProjectFile project, String prefix) {
        for (int i=0; ; i++) {
            String candidate = prefix + (i==0 ? "" : " " + i);
            boolean found = false;
            for (int j=0; j<project.getCurrent().getPageCount(); j++)
                if (project.getCurrent().getPageAt(j).getPageName().equals(candidate)) {
                    found = true;
                    break;
                }
            if (!found)
                return candidate; // Unique name
        }        
    }
    
    private void moveSelectedPage(final int incr, String descr) {
        executeUndoableCommand(descr, (ProjectData proj, ProjectPage elem) -> {
            int pos = proj.findPagePosition(elem);
            proj.movePage(pos, pos + incr);
        });
    }
    
    @Override
    public void setStatus(String str, boolean isOk) {
        if (isOk) {
            if (str == null)
                jStatusLabel.setText("Ok.");
            else
                jStatusLabel.setText("Ok: "+str);
        }
        else {
            if (str == null)
                jStatusLabel.setText("Fail.");
            else
                jStatusLabel.setText("Fail: "+str);
        }
    }
    
    @Override
    public void executeUndoableCommand(String descr, UndoableCommand cmd) {
//        System.out.println("NEW COMMAND: "+descr);
        //assert guiStatus == GuiStatus.UPDATED;
        assert hasActiveProject();
        try {
            activeProject.startChanges();
            // change the project data
            cmd.Execute(activeProject.getCurrent(), 
                        activeProject.getCurrent().getActivePage());
            activeProject.commitChanges(descr);
            setStatus(descr, true);
        }
        catch (NoOpException noe) {
            // Nothing done, do not show any error message
            activeProject.rollbackChanges();
            setStatus(null, true);
        }
        catch (IllegalStateException ise) {
            // Invalid operation made by the user, notify the error
            JOptionPane.showMessageDialog(AppWindow.this, ise.getMessage(), 
                                            "Invalid operation.", JOptionPane.ERROR_MESSAGE);
            activeProject.rollbackChanges();
            setStatus(descr, false);
        }
        catch (Exception e) {
            // Internal error
            JOptionPane.showMessageDialog(AppWindow.this, 
                                          "An internal error happened.\n"+
                                          "Reason: "+e.getMessage(), 
                                          "Exception.",
                                          JOptionPane.ERROR_MESSAGE);
            logException(e, true);
            activeProject.rollbackChanges();
            setStatus(descr, false);
        }
    }
    
    // methods
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private common.Action actionAbout;
    private common.Action actionAdvRapidMeasurePopup;
    private common.Action actionAppPreferences;
    private common.Action actionCaptureSVG;
    private common.Action actionClearMeasureDir;
    private common.Action actionClose;
    private common.Action actionCopy;
    private common.Action actionCut;
    private common.Action actionDeletePage;
    private common.Action actionDuplicatePage;
    private common.Action actionEndPlay;
    private common.Action actionExit;
    private common.Action actionHelpGrammar;
    private common.Action actionImportGreatSpn;
    private common.Action actionImportPNML;
    private common.Action actionMoveDown;
    private common.Action actionMoveUp;
    private common.Action actionNewDtaPage;
    private common.Action actionNewMeasurePage;
    private common.Action actionNewMultiPage;
    private common.Action actionNewNetPage_CPN;
    private common.Action actionNewNetPage_FullGSPN;
    private common.Action actionNewNetPage_GSPN;
    private common.Action actionNewNetPage_PT;
    private common.Action actionNewNetPage_SWN;
    private common.Action actionNewPage;
    private common.Action actionNewProject;
    private common.Action actionNormalZoom;
    private common.Action actionOpen;
    private common.Action actionPaste;
    private common.Action actionPrint;
    private common.Action actionRedo;
    private common.Action actionRenamePage;
    private common.Action actionSave;
    private common.Action actionSaveAll;
    private common.Action actionSaveAs;
    private common.Action actionStartAlgebra;
    private common.Action actionStartBoundsFromPinv;
    private common.Action actionStartCTMC;
    private common.Action actionStartPlaceInv;
    private common.Action actionStartPlay;
    private common.Action actionStartPlayForPopupMenu;
    private common.Action actionStartRG;
    private common.Action actionStartSymRG;
    private common.Action actionStartTransitionInv;
    private common.Action actionStartUnfolding;
    private common.Action actionUndo;
    private common.Action actionZoomIn;
    private common.Action actionZoomOut;
    private javax.swing.JMenuBar jAppMenuBar;
    private javax.swing.JToolBar jAppToolBar_Advanced;
    private javax.swing.JToolBar jAppToolBar_Basic;
    private javax.swing.JToolBar jAppToolBar_Edit;
    private javax.swing.JToolBar jAppToolBar_File;
    private common.JDropdownToolbarButton jDropdownAddNewPage;
    private common.JDropdownToolbarButton jDropdownToolbarButtonRapidMeasure;
    private javax.swing.JPanel jLeftPanel;
    private javax.swing.JMenu jMenuEdit;
    private javax.swing.JMenu jMenuFile;
    private javax.swing.JMenu jMenuHelp;
    private javax.swing.JMenuItem jMenuItem1;
    private javax.swing.JMenuItem jMenuItemNewDtaPage;
    private javax.swing.JMenuItem jMenuItemNewMeasurePage;
    private javax.swing.JMenuItem jMenuItemNewMultiPage;
    private javax.swing.JMenuItem jMenuItemNewNetPage_CPN;
    private javax.swing.JMenuItem jMenuItemNewNetPage_FullGSPN;
    private javax.swing.JMenuItem jMenuItemNewNetPage_GSPN;
    private javax.swing.JMenuItem jMenuItemNewNetPage_PT;
    private javax.swing.JMenuItem jMenuItemNewNetPage_SWN;
    private javax.swing.JMenuItem jMenuItemOptions;
    private javax.swing.JMenuItem jMenuItem_ApnnFormat;
    private javax.swing.JMenuItem jMenuItem_DtaFormat;
    private javax.swing.JMenuItem jMenuItem_EditCopy;
    private javax.swing.JMenuItem jMenuItem_EditCut;
    private javax.swing.JMenuItem jMenuItem_EditDeleteSelected;
    private javax.swing.JMenuItem jMenuItem_EditDeselectAll;
    private javax.swing.JMenuItem jMenuItem_EditInvertSelection;
    private javax.swing.JMenuItem jMenuItem_EditPaste;
    private javax.swing.JMenuItem jMenuItem_EditRedo;
    private javax.swing.JMenuItem jMenuItem_EditSelectAll;
    private javax.swing.JMenuItem jMenuItem_EditUndo;
    private javax.swing.JMenuItem jMenuItem_FileCloseProj;
    private javax.swing.JMenuItem jMenuItem_FileExit;
    private javax.swing.JMenu jMenuItem_FileExport;
    private javax.swing.JMenuItem jMenuItem_FileNewSchema;
    private javax.swing.JMenuItem jMenuItem_FileOpen;
    private javax.swing.JMenu jMenuItem_FileOpenRecent;
    private javax.swing.JMenuItem jMenuItem_FilePrint;
    private javax.swing.JMenuItem jMenuItem_FileSave;
    private javax.swing.JMenuItem jMenuItem_FileSaveAll;
    private javax.swing.JMenuItem jMenuItem_FileSaveAs;
    private javax.swing.JMenuItem jMenuItem_GrMLExport;
    private javax.swing.JMenuItem jMenuItem_GreatSpnExport;
    private javax.swing.JMenuItem jMenuItem_GreatSpnImport;
    private javax.swing.JMenuItem jMenuItem_HelpAbout;
    private javax.swing.JMenuItem jMenuItem_HelpShowGrammar;
    private javax.swing.JMenuItem jMenuItem_PNMLExport;
    private javax.swing.JMenuItem jMenuItem_PNMLImport;
    private javax.swing.JMenuItem jMenuItem_SimClose;
    private javax.swing.JMenuItem jMenuItem_SimNext;
    private javax.swing.JMenuItem jMenuItem_SimPrev;
    private javax.swing.JMenuItem jMenuItem_SimRestart;
    private javax.swing.JMenuItem jMenuItem_SimStart;
    private javax.swing.JMenuItem jMenuItem_ViewZoomIn;
    private javax.swing.JMenuItem jMenuItem_ViewZoomOut;
    private javax.swing.JMenu jMenuLibraryModels;
    private javax.swing.JMenu jMenuSimulation;
    private javax.swing.JMenu jMenuView;
    private javax.swing.JMenu jMenu_FileImport;
    private javax.swing.JMenu jMenu_SimModelCheck;
    private javax.swing.JPanel jPanelBottomError;
    private javax.swing.JPanel jPanelCenter;
    private javax.swing.JPanel jPanelCenterLeft;
    private javax.swing.JPanel jPanelProjectTree;
    private javax.swing.JPanel jPanelProperty;
    private javax.swing.JPanel jPanelPropertyPane;
    private javax.swing.JPanel jPanelToolBars;
    private javax.swing.JPanel jPanelTreeCmds;
    private javax.swing.JToolBar jPlayToolBar;
    private javax.swing.JPopupMenu jPopupMenuAddPage;
    private javax.swing.JPopupMenu jPopupMenuRapidMeaureCmd;
    private javax.swing.JPanel jRightPanel;
    private javax.swing.JScrollPane jScrollPanePageErrors;
    private javax.swing.JScrollPane jScrollPaneProjects;
    private javax.swing.JScrollPane jScrollPaneProperties;
    private javax.swing.JSeparator jSeparator1;
    private javax.swing.JPopupMenu.Separator jSeparator10;
    private javax.swing.JPopupMenu.Separator jSeparator2;
    private javax.swing.JPopupMenu.Separator jSeparator4;
    private javax.swing.JPopupMenu.Separator jSeparator5;
    private javax.swing.JToolBar.Separator jSeparator6;
    private javax.swing.JPopupMenu.Separator jSeparator8;
    private javax.swing.JPopupMenu.Separator jSeparator9;
    private javax.swing.JPopupMenu.Separator jSeparatorLibraryModels;
    private javax.swing.JPopupMenu.Separator jSeparatorPetriNets;
    private javax.swing.JSplitPane jSplitPaneCenterErrListV;
    private javax.swing.JSplitPane jSplitPaneLeftCenterH;
    private javax.swing.JSplitPane jSplitPaneLeftV;
    private javax.swing.JPanel jStatusBar;
    private javax.swing.JLabel jStatusLabel;
    private javax.swing.JTable jTablePageErrors;
    private javax.swing.JToolBar jToolBarTree;
    private common.JToolbarButton jToolbarButtonAlgebra;
    private common.JToolbarButton jToolbarButtonClearMeasureDir;
    private common.JToolbarButton jToolbarButtonCopy;
    private common.JToolbarButton jToolbarButtonCut;
    private common.JToolbarButton jToolbarButtonDeletePage;
    private common.JToolbarButton jToolbarButtonDeleteSelected;
    private common.JToolbarButton jToolbarButtonDuplicatePage;
    private common.JToolbarButton jToolbarButtonEndPlay;
    private common.JToolbarButton jToolbarButtonMoveDown;
    private common.JToolbarButton jToolbarButtonMoveUp;
    private common.JToolbarButton jToolbarButtonNew;
    private common.JToolbarButton jToolbarButtonOpen;
    private common.JToolbarButton jToolbarButtonPaste;
    private common.JToolbarButton jToolbarButtonRedo;
    private common.JToolbarButton jToolbarButtonRenamePage;
    private common.JToolbarButton jToolbarButtonSaveAll;
    private common.JToolbarButton jToolbarButtonShowCTMC;
    private common.JToolbarButton jToolbarButtonShowPlaceBoundsFromPinv;
    private common.JToolbarButton jToolbarButtonShowPlaceSemiflows;
    private common.JToolbarButton jToolbarButtonShowRG;
    private common.JToolbarButton jToolbarButtonShowSymRG;
    private common.JToolbarButton jToolbarButtonShowTransitionSemiflows;
    private common.JToolbarButton jToolbarButtonStartPlay;
    private common.JToolbarButton jToolbarButtonUndo;
    private common.JToolbarButton jToolbarButtonUnfolding;
    private javax.swing.JTree jTreeProjects;
    private javax.swing.JPanel jZoomPanelContainer;
    private editor.gui.ResourceFactory resourceFactory;
    private common.Action rmcActionBuildRG;
    private common.Action rmcActionBuildRS_MDD;
    private common.Action rmcActionBuildSymRG;
    private common.Action rmcActionCTL;
    private common.Action rmcActionSteadyStateSim;
    private common.Action rmcActionSteadyStateSol;
    private common.Action rmcActionTransientSim;
    private common.Action rmcActionTransientSol;
    private javax.swing.JMenuItem rmcMenuItemBuildRG;
    private javax.swing.JMenuItem rmcMenuItemBuildRS_DD;
    private javax.swing.JMenuItem rmcMenuItemBuildSymRG;
    private javax.swing.JMenuItem rmcMenuItemCTL;
    private javax.swing.JMenuItem rmcMenuItemSteadyStateSim;
    private javax.swing.JMenuItem rmcMenuItemSteadyStateSol;
    private javax.swing.JMenuItem rmcMenuItemTransientSim;
    private javax.swing.JMenuItem rmcMenuItemTransientSol;
    private common.Action sharedActionChangeBindings;
    private common.Action sharedActionComputeBoundsFromPinv;
    private common.Action sharedActionComputePlaceSemiflows;
    private common.Action sharedActionComputeTransitionSemiflows;
    private common.Action sharedActionDeleteSelected;
    private common.Action sharedActionDeselectAll;
    private common.Action sharedActionExportAPNN;
    private common.Action sharedActionExportDta;
    private common.Action sharedActionExportGRML;
    private common.Action sharedActionExportGreatSpn;
    private common.Action sharedActionExportPNML;
    private common.Action sharedActionInvertSelection;
    private common.Action sharedActionNext;
    private common.Action sharedActionPrev;
    private common.Action sharedActionRestart;
    private common.Action sharedActionSelectAll;
    // End of variables declaration//GEN-END:variables

}
