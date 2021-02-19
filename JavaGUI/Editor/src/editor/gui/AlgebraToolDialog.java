/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.gui;

import common.EnabledJComboBoxRenderer;
import common.Util;
import editor.Main;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import editor.domain.ViewProfile;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import editor.domain.io.GreatSpnFormat;
import editor.domain.measures.SolverInvokator;
import static editor.domain.measures.SolverInvokator.makeFilenameCmd;
import static editor.domain.measures.SolverInvokator.splitCommandLine;
import static editor.domain.measures.SolverInvokator.startOfCommand;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;
import javax.swing.DefaultListModel;
import javax.swing.DefaultListSelectionModel;
import javax.swing.JOptionPane;
import javax.swing.Timer;

/**
 *
 * @author elvio
 */
public class AlgebraToolDialog extends javax.swing.JDialog {

    /**
     * Creates new form AlgebraToolDialog
     */
    public AlgebraToolDialog(java.awt.Frame parent, boolean modal) {
        super(parent, modal);
        initComponents();
        
        textFieldPlaceTags.addKeyListener(new KeyAdapter() {
        });
    }
    
    class TagsFieldListener implements FocusListener, KeyListener, ActionListener {
        @Override public void focusGained(FocusEvent e) { }

        @Override
        public void focusLost(FocusEvent e) {
            rebuildMessageList();
        }

        @Override
        public void keyTyped(KeyEvent e) {
            rebuildListTimer.setRepeats(false);
            rebuildListTimer.restart();
        }

        @Override public void keyPressed(KeyEvent e) { }
        @Override public void keyReleased(KeyEvent e) { }

        @Override
        public void actionPerformed(ActionEvent e) {
            rebuildMessageList();
        }
    }
    final private TagsFieldListener tagsFieldListener = new TagsFieldListener();
    Timer rebuildListTimer = new Timer(200, tagsFieldListener);
    
    private GspnPage net1;
    private ArrayList<GspnPage> listOfNets2;
    boolean runAlgebra = false;
    boolean initializing = true;
    
    public static final String KEY_NET2 = "algebra.net2";
    public static final String KEY_OPERATOR = "algebra.operator";
    public static final String KEY_PLACE_TAGS = "algebra.place_tags";
    public static final String KEY_TRANS_TAGS = "algebra.transition_tags";
    public static final String KEY_PLACEMENT = "algebra.placement";
    public static final String KEY_BROKEN_EDGES = "algebra.broken_edges";
    public static final String KEY_DX_SHIFT = "algebra.dx";
    public static final String KEY_DY_SHIFT = "algebra.dy";

    
    public GspnPage openDialogCompose(GspnPage net1, ProjectData listOfNets) {
        GspnPage composed = null;
        try {
            this.net1 = net1;
            initializing = true;
            // Read back algebra properties used last time
            String propNet2Name = Util.getPreferences().get(KEY_NET2, "");
            char propOperator = Util.getPreferences().get(KEY_OPERATOR, "p").charAt(0);
            String propPlaceTags = Util.getPreferences().get(KEY_PLACE_TAGS, "");
            String propTransTags = Util.getPreferences().get(KEY_TRANS_TAGS, "");
            int propPlacement = Util.getPreferences().getInt(KEY_PLACEMENT, 1);
            boolean propBrokenEdges = Util.getPreferences().getBoolean(KEY_BROKEN_EDGES, false);
            int propDxShift = Util.getPreferences().getInt(KEY_DX_SHIFT, 5);
            int propDyShift = Util.getPreferences().getInt(KEY_DY_SHIFT, 5);
            
            // Initialize the net2 GSPN combo box
            listOfNets2 = new ArrayList<>();
            comboNet2Name.removeAllItems();
            DefaultListSelectionModel model = new DefaultListSelectionModel();
            for (int p=0; p<listOfNets.getPageCount(); p++) {
                ProjectPage page = listOfNets.getPageAt(p);
                if (page.pageSupportsAlgebraTool()) {
                    comboNet2Name.addItem(page.getPageName());
                    listOfNets2.add((GspnPage)page);
                    if (page.isPageCorrect()) {
                        model.addSelectionInterval(comboNet2Name.getItemCount() - 1, 
                                                   comboNet2Name.getItemCount() - 1);
                        if (page.getPageName().equals(propNet2Name)) {
                            comboNet2Name.setSelectedIndex(comboNet2Name.getItemCount() - 1);
                        }
                    }
                }
            }
            EnabledJComboBoxRenderer<String> enableRenderer = new EnabledJComboBoxRenderer<>(model);
            comboNet2Name.setRenderer(enableRenderer);
            
            labelNet1Name.setText(net1.getPageName());
            
            // Initialize the other Swing controls
            radioPlaceSuperpos.setSelected(propOperator == 'p');
            radioTransitionSuperpos.setSelected(propOperator == 't');
            radioPlaceAndTransitionSuperpos.setSelected(propOperator == 'b');
            textFieldPlaceTags.setText(propPlaceTags);
            textFieldTransitionTags.setText(propTransTags);
            checkBoxBrokenEdges.setSelected(propBrokenEdges);
            switch (propPlacement) {
                case 1:  radioHorizontal.setSelected(true); break;
                case 2:  radioVertical.setSelected(true); break;
                case 3:  radioShift.setSelected(true); break;
            }
            
            textFieldPlaceTags.addFocusListener(tagsFieldListener);
            textFieldTransitionTags.addFocusListener(tagsFieldListener);
            textFieldPlaceTags.addKeyListener(tagsFieldListener);
            textFieldTransitionTags.addKeyListener(tagsFieldListener);
            
            textFieldDxShift.setText(""+propDxShift);
            textFieldDyShift.setText(""+propDyShift);
            
            // Pack and open the dialog window
            getRootPane().setDefaultButton(buttonRun);
            pack();
            setLocationRelativeTo(getParent());
            initializing = false;
            operatorButtonGroupChanged();
            placementButtonGroupChanged();
            rebuildMessageList();
            
            while (true) {
                setVisible(true);
                if (!runAlgebra)
                    break; // user pressed the 'Cancel' button

                try {
                    StringBuilder cmd = new StringBuilder();
                    
                    cmd.append(startOfCommand());
                    cmd.append(SolverInvokator.useGreatSPN_binary("algebra")).append(" ");
                    String transitionTags = "", placeTags = "";

                    // Read properties from the dialog controls
                    if (radioPlaceAndTransitionSuperpos.isSelected())
                        propOperator = 'b';
                    if (radioPlaceSuperpos.isSelected())
                        propOperator = 'p';
                    if (radioTransitionSuperpos.isSelected())
                        propOperator = 't';
                    
                    if (propOperator == 'p' || propOperator == 'b') {
                        propPlaceTags = textFieldPlaceTags.getText();
                        placeTags = "place={"+propPlaceTags.replace(",", "|").replace(" ", "") + "}\n";
                    }
                    if (propOperator == 't' || propOperator == 'b') {
                        propTransTags = textFieldTransitionTags.getText();
                        transitionTags = "transition={"+propTransTags.replace(",", "|").replace(" ", "") + "}\n";
                    }
                    propBrokenEdges = checkBoxBrokenEdges.isSelected();

                    if (radioHorizontal.isSelected())   propPlacement = 1;
                    if (radioVertical.isSelected())   propPlacement = 2;
                    if (radioShift.isSelected())   propPlacement = 3;
                    
                    // Save the two nets and the restfile into temporary files
                    File tmpName1 = File.createTempFile("net1", "");
                    File tmpNet1 = new File(tmpName1.getAbsolutePath()+".net");
                    File tmpDef1 = new File(tmpName1.getAbsolutePath()+".def");
                    File tmpName2 = File.createTempFile("net2", "");
                    File tmpNet2 = new File(tmpName2.getAbsolutePath()+".net");
                    File tmpDef2 = new File(tmpName2.getAbsolutePath()+".def");
                    GspnPage net2 = listOfNets2.get(comboNet2Name.getSelectedIndex());
                    GreatSpnFormat.exportGspn(net1, tmpNet1, tmpDef1, false);
                    GreatSpnFormat.exportGspn(net2, tmpNet2, tmpDef2, false);

                    File tmpRestfile = File.createTempFile("tags", ".restfile");
                    PrintWriter restFileOut = new PrintWriter(new BufferedWriter(new FileWriter(tmpRestfile)));
                    restFileOut.print(transitionTags);
                    restFileOut.print(placeTags);
                    restFileOut.close();
                    
                    File tmpResult = File.createTempFile("netResult", "");
                    File tmpNetRes = new File(tmpResult.getAbsolutePath()+".net");
                    File tmpDefRes = new File(tmpResult.getAbsolutePath()+".def");
                    
                    // Compose the algebra command
                    if (!propBrokenEdges)
                        cmd.append("-no_ba ");
                    cmd.append(makeFilenameCmd(tmpName1)).append(" ");
                    cmd.append(makeFilenameCmd(tmpName2)).append(" ");
                    cmd.append(propOperator).append(" ");
                    cmd.append(makeFilenameCmd(tmpRestfile)).append(" ");
                    cmd.append(makeFilenameCmd(tmpResult)).append(" ");
                    cmd.append(propPlacement).append(" ");
                    if (propPlacement == 3) {
                        propDxShift = Integer.parseInt(textFieldDxShift.getText());
                        propDyShift = Integer.parseInt(textFieldDyShift.getText());
                        cmd.append(propDxShift).append(" ").append(propDyShift).append(" ");
                    }
//                    System.out.println("NET1: "+tmpName1.getAbsolutePath());
//                    System.out.println("NET2: "+tmpName2.getAbsolutePath());
//                    System.out.println("REST: "+tmpRestfile.getAbsolutePath());
//                    System.out.println("RESULT: "+tmpResult.getAbsolutePath());
//                    System.out.println("CMD: "+cmd.toString());
                    
                    // Run the tool
                    String[] envp = SolverInvokator.prepareRuntimeEnvironmentVars();
                    System.out.println(cmd);
                    Process pr = Runtime.getRuntime().exec(splitCommandLine(cmd.toString()), envp);
                    int retVal = pr.waitFor();
                    if (retVal != 0)
                        throw new IllegalStateException("algebra returned an exit code of "+retVal);
                    
                    // Read back the composed GSPN
                    GspnPage netComp = new GspnPage();
                    netComp.setPageName(net1.getPageName()+"+"+net2.getPageName());
                    GreatSpnFormat.importGspn(netComp, tmpNetRes, tmpDefRes);
                    netComp.viewProfile = (ViewProfile)Util.deepCopy(net1.viewProfile);
                    
                    // Make all place/transition names unique
                    Set<String> names = new HashSet<>();
                    for (Node node : netComp.nodes) {
                        if (names.contains(node.getUniqueName())) {
                            int i=0;
                            String newName;
                            do {
                                newName = node.getUniqueName() + "_" + (i++);
                            } while (names.contains(newName));
                            node.setUniqueName(newName);
                        }
                        names.add(node.getUniqueName());
                    }
                    
                    // Clear intermediate temporary files.
                    tmpNet1.delete();
                    tmpDef1.delete();
                    tmpNet2.delete();
                    tmpDef2.delete();
                    tmpRestfile.delete();
                    tmpNetRes.delete();
                    tmpDefRes.delete();

                    // Save preferences back
                    Util.getPreferences().put(KEY_NET2, comboNet2Name.getSelectedItem().toString());
                    Util.getPreferences().put(KEY_OPERATOR, ""+propOperator);
                    Util.getPreferences().put(KEY_PLACE_TAGS, propPlaceTags);
                    Util.getPreferences().put(KEY_TRANS_TAGS, propTransTags);
                    Util.getPreferences().putBoolean(KEY_BROKEN_EDGES, propBrokenEdges);
                    Util.getPreferences().putInt(KEY_PLACEMENT, propPlacement);
                    Util.getPreferences().putInt(KEY_DX_SHIFT, propDxShift);
                    Util.getPreferences().putInt(KEY_DY_SHIFT, propDyShift);

                    // Ready to return the composed net
                    composed = netComp;
                    break;
                }
                catch (Exception e) {
                    JOptionPane.showMessageDialog(this, 
                            "Could not complete the operation:\n"+e.getMessage(), 
                            "Algebra error", JOptionPane.ERROR_MESSAGE);
                    Main.logException(e, true);
                }
            }
        }
        finally {
            listOfNets2 = null;
            this.net1= null;
            textFieldPlaceTags.removeFocusListener(tagsFieldListener);
            textFieldTransitionTags.removeFocusListener(tagsFieldListener);
            textFieldPlaceTags.removeKeyListener(tagsFieldListener);
            textFieldTransitionTags.removeKeyListener(tagsFieldListener);
        }
        return composed;
    }
    
    void verifyTags(GspnPage net1, GspnPage net2, String tagList, char type,
                    DefaultListModel<String> list)
    {
        if (tagList.trim().length() == 0)
            return; // The empty tag list is always accepted
        
        Set<String> allTags1 = new HashSet<>();
        Set<String> allTags2 = new HashSet<>();
        String what = (type == 'P') ? "Place" : "Transition";
        
        // Get tags of net1
        for (Node node : net1.nodes) {
            if ((type == 'P' && node instanceof Place) ||
                (type == 'T' && node instanceof Transition)) 
            {
                for (int t=0; t<node.numTags(); t++)
                    allTags1.add(node.getTag(t));
            }
        }
        // Tags of net2
        for (Node node : net2.nodes) {
            if ((type == 'P' && node instanceof Place) ||
                (type == 'T' && node instanceof Transition)) 
            {
                for (int t=0; t<node.numTags(); t++)
                    allTags2.add(node.getTag(t));
            }
        }
        // Verify that all the tags in tagList belongs to both the nets
        String[] tags = tagList.split(",");
        boolean emptyWarn = false;
        boolean missingTags = false;
        for (String tag : tags) {
            tag = tag.trim();
            if (tag.length() == 0 && !emptyWarn) {
                list.addElement("Error: An empty "+what+" tag is not allowed.");
                emptyWarn = true;
            }
            boolean c1 = allTags1.contains(tag);
            boolean c2 = allTags2.contains(tag);
            if (!c1 && !c2)
                list.addElement("Error: "+what+" tag '"+tag+"' does not appear in both "+net1.getPageName()+" and "+net2.getPageName()+".");
            else if (!c1)
                list.addElement("Error: "+what+" tag '"+tag+"' does not appear in "+net1.getPageName()+".");
            else if (!c2)
                list.addElement("Error: "+what+" tag '"+tag+"' does not appear in "+net2.getPageName()+".");
            missingTags = missingTags || (!c1 || !c2);
        }
        if (missingTags) {
            StringBuilder msg = new StringBuilder();
            int k=0;
            for (String tag : allTags1)
                if (allTags2.contains(tag)) {
                    msg.append(k++ == 0 ? "" : ", ").append("'").append(tag).append("'");
                }
            msg.append(".");
            if (k == 0)
                list.addElement("There are no shared "+what+" tags.");
            else if (k == 1)
                list.addElement("The only shared "+what+" tag is: "+msg.toString());
            else
                list.addElement("Shared "+what+" tags are: "+msg.toString());
        }
    }
    
    void rebuildMessageList() {
        DefaultListModel<String> list = new DefaultListModel<>();
        
        GspnPage net2 = listOfNets2.get(comboNet2Name.getSelectedIndex());
        
        boolean place= radioPlaceSuperpos.isSelected() || radioPlaceAndTransitionSuperpos.isSelected();
        boolean trans = radioTransitionSuperpos.isSelected() || radioPlaceAndTransitionSuperpos.isSelected();
  
        if (place)
            verifyTags(net1, net2, textFieldPlaceTags.getText(), 'P', list);
        if (trans)
            verifyTags(net1, net2, textFieldTransitionTags.getText(), 'T', list);
        
        listOfMessages.setModel(list);
        
        buttonRun.setEnabled(list.getSize() == 0);
    }
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        resourceFactory = new editor.gui.ResourceFactory();
        buttonGroupPlacement = new javax.swing.ButtonGroup();
        buttonGroupOperator = new javax.swing.ButtonGroup();
        jLabel2 = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        labelNet1Name = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        comboNet2Name = new javax.swing.JComboBox<String>();
        jPanel2 = new javax.swing.JPanel();
        jLabel4 = new javax.swing.JLabel();
        textFieldPlaceTags = new javax.swing.JTextField();
        jLabel1 = new javax.swing.JLabel();
        textFieldTransitionTags = new javax.swing.JTextField();
        radioPlaceSuperpos = new javax.swing.JRadioButton();
        radioTransitionSuperpos = new javax.swing.JRadioButton();
        radioPlaceAndTransitionSuperpos = new javax.swing.JRadioButton();
        jPanel3 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        listOfMessages = new javax.swing.JList<String>();
        jPanel4 = new javax.swing.JPanel();
        radioHorizontal = new javax.swing.JRadioButton();
        jLabel5 = new javax.swing.JLabel();
        radioVertical = new javax.swing.JRadioButton();
        jLabel6 = new javax.swing.JLabel();
        radioShift = new javax.swing.JRadioButton();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();
        jLabel9 = new javax.swing.JLabel();
        textFieldDxShift = new javax.swing.JTextField();
        textFieldDyShift = new javax.swing.JTextField();
        checkBoxBrokenEdges = new javax.swing.JCheckBox();
        buttonRun = new javax.swing.JButton();
        buttonCancel = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosed(java.awt.event.WindowEvent evt) {
                dialogWindowClosed(evt);
            }
        });

        jLabel2.setIcon(resourceFactory.getSum32());
        jLabel2.setText("<html>\n<b>Algebra:</b><br/> Composition of SWN nets by place/transition superposition.\n</html>");

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Operand Nets"));

        labelNet1Name.setIcon(resourceFactory.getPageNet16());
        labelNet1Name.setText("  NET1");

        jLabel3.setIcon(resourceFactory.getPageNet16());

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelNet1Name)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(jLabel3)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(comboNet2Name, javax.swing.GroupLayout.PREFERRED_SIZE, 383, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(labelNet1Name)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(comboNet2Name)
                    .addComponent(jLabel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder("Algebra Operator"));

        jLabel4.setText("Comma-separated list of superposed places tags (example:  tag1, tag2, tag3):");

        jLabel1.setText("Comma-separated list of superposed transition tags (example:  tag1, tag2, tag3):");

        buttonGroupOperator.add(radioPlaceSuperpos);
        radioPlaceSuperpos.setText("Superposition over Places.");
        radioPlaceSuperpos.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                radioPlaceSuperposActionPerformed(evt);
            }
        });

        buttonGroupOperator.add(radioTransitionSuperpos);
        radioTransitionSuperpos.setText("Superposition over Transitions.");
        radioTransitionSuperpos.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                radioTransitionSuperposActionPerformed(evt);
            }
        });

        buttonGroupOperator.add(radioPlaceAndTransitionSuperpos);
        radioPlaceAndTransitionSuperpos.setText("Superposition over Places and Transitions.");
        radioPlaceAndTransitionSuperpos.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                radioPlaceAndTransitionSuperposActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(textFieldPlaceTags)
                    .addComponent(textFieldTransitionTags)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel4)
                            .addComponent(jLabel1)
                            .addComponent(radioPlaceSuperpos)
                            .addComponent(radioTransitionSuperpos)
                            .addComponent(radioPlaceAndTransitionSuperpos))
                        .addGap(0, 2, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(radioPlaceSuperpos)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(radioTransitionSuperpos)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(radioPlaceAndTransitionSuperpos)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel4)
                .addGap(2, 2, 2)
                .addComponent(textFieldPlaceTags, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(textFieldTransitionTags, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(0, 0, Short.MAX_VALUE))
        );

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder("Messages"));

        jScrollPane1.setViewportView(listOfMessages);

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addComponent(jScrollPane1)
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 97, Short.MAX_VALUE)
                .addContainerGap())
        );

        jPanel4.setBorder(javax.swing.BorderFactory.createTitledBorder("Placement"));
        jPanel4.setToolTipText("");

        buttonGroupPlacement.add(radioHorizontal);
        radioHorizontal.setText("Horizontal");
        radioHorizontal.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                radioHorizontalActionPerformed(evt);
            }
        });

        jLabel5.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12H.png"))); // NOI18N

        buttonGroupPlacement.add(radioVertical);
        radioVertical.setText("Vertical");
        radioVertical.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                radioVerticalActionPerformed(evt);
            }
        });

        jLabel6.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12V.png"))); // NOI18N

        buttonGroupPlacement.add(radioShift);
        radioShift.setText("Shift");
        radioShift.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                radioShiftActionPerformed(evt);
            }
        });

        jLabel7.setIcon(new javax.swing.ImageIcon(getClass().getResource("/editor/gui/icons/12HV.png"))); // NOI18N

        jLabel8.setText("Shift dx:");

        jLabel9.setText("Shift dy:");

        checkBoxBrokenEdges.setText("Use broken edges between subnets.");

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(radioHorizontal)
                            .addGroup(jPanel4Layout.createSequentialGroup()
                                .addGap(29, 29, 29)
                                .addComponent(jLabel5)))
                        .addGap(18, 18, 18)
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(radioVertical)
                            .addGroup(jPanel4Layout.createSequentialGroup()
                                .addGap(29, 29, 29)
                                .addComponent(jLabel6)))
                        .addGap(18, 18, 18)
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(radioShift)
                            .addGroup(jPanel4Layout.createSequentialGroup()
                                .addGap(29, 29, 29)
                                .addComponent(jLabel7)))
                        .addGap(18, 18, 18)
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addGroup(jPanel4Layout.createSequentialGroup()
                                .addComponent(jLabel9)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(textFieldDyShift))
                            .addGroup(jPanel4Layout.createSequentialGroup()
                                .addComponent(jLabel8)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(textFieldDxShift, javax.swing.GroupLayout.PREFERRED_SIZE, 123, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addComponent(checkBoxBrokenEdges))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(jLabel8)
                            .addComponent(textFieldDxShift, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(jLabel9)
                            .addComponent(textFieldDyShift, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(radioHorizontal)
                            .addComponent(radioVertical)
                            .addComponent(radioShift))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel6)
                            .addComponent(jLabel5)
                            .addComponent(jLabel7))))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(checkBoxBrokenEdges)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        buttonRun.setText("Run algebra");
        buttonRun.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonRunActionPerformed(evt);
            }
        });

        buttonCancel.setText("Cancel");
        buttonCancel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonCancelActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(jLabel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addComponent(jPanel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel4, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(buttonCancel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(buttonRun)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(buttonRun)
                    .addComponent(buttonCancel))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void buttonRunActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonRunActionPerformed
        runAlgebra = true;
        setVisible(false);
    }//GEN-LAST:event_buttonRunActionPerformed

    private void buttonCancelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonCancelActionPerformed
        runAlgebra = false;
        setVisible(false);
    }//GEN-LAST:event_buttonCancelActionPerformed

    private void dialogWindowClosed(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_dialogWindowClosed
        runAlgebra = false;
    }//GEN-LAST:event_dialogWindowClosed

    void operatorButtonGroupChanged() {
        boolean place= radioPlaceSuperpos.isSelected() || radioPlaceAndTransitionSuperpos.isSelected();
        boolean trans = radioTransitionSuperpos.isSelected() || radioPlaceAndTransitionSuperpos.isSelected();
        
        textFieldPlaceTags.setEnabled(place);
        textFieldPlaceTags.setEditable(place);
        textFieldTransitionTags.setEnabled(trans);
        textFieldTransitionTags.setEditable(trans);
        
        rebuildMessageList();
    }
    
    void placementButtonGroupChanged() {
        textFieldDxShift.setEnabled(radioShift.isSelected());
        textFieldDxShift.setEditable(radioShift.isSelected());
        textFieldDyShift.setEnabled(radioShift.isSelected());
        textFieldDyShift.setEditable(radioShift.isSelected());
        if (textFieldDxShift.isEnabled() && textFieldDxShift.getText().length() == 0)
            textFieldDxShift.setText("10");
        if (textFieldDyShift.isEnabled() && textFieldDyShift.getText().length() == 0)
            textFieldDyShift.setText("10");
    }
    
    private void radioHorizontalActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_radioHorizontalActionPerformed
        placementButtonGroupChanged();
    }//GEN-LAST:event_radioHorizontalActionPerformed

    private void radioVerticalActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_radioVerticalActionPerformed
        placementButtonGroupChanged();
    }//GEN-LAST:event_radioVerticalActionPerformed

    private void radioShiftActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_radioShiftActionPerformed
        placementButtonGroupChanged();
    }//GEN-LAST:event_radioShiftActionPerformed

    private void radioPlaceSuperposActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_radioPlaceSuperposActionPerformed
        operatorButtonGroupChanged();
    }//GEN-LAST:event_radioPlaceSuperposActionPerformed

    private void radioTransitionSuperposActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_radioTransitionSuperposActionPerformed
        operatorButtonGroupChanged();
    }//GEN-LAST:event_radioTransitionSuperposActionPerformed

    private void radioPlaceAndTransitionSuperposActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_radioPlaceAndTransitionSuperposActionPerformed
        operatorButtonGroupChanged();
    }//GEN-LAST:event_radioPlaceAndTransitionSuperposActionPerformed

    

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton buttonCancel;
    private javax.swing.ButtonGroup buttonGroupOperator;
    private javax.swing.ButtonGroup buttonGroupPlacement;
    private javax.swing.JButton buttonRun;
    private javax.swing.JCheckBox checkBoxBrokenEdges;
    private javax.swing.JComboBox<String> comboNet2Name;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelNet1Name;
    private javax.swing.JList<String> listOfMessages;
    private javax.swing.JRadioButton radioHorizontal;
    private javax.swing.JRadioButton radioPlaceAndTransitionSuperpos;
    private javax.swing.JRadioButton radioPlaceSuperpos;
    private javax.swing.JRadioButton radioShift;
    private javax.swing.JRadioButton radioTransitionSuperpos;
    private javax.swing.JRadioButton radioVertical;
    private editor.gui.ResourceFactory resourceFactory;
    private javax.swing.JTextField textFieldDxShift;
    private javax.swing.JTextField textFieldDyShift;
    private javax.swing.JTextField textFieldPlaceTags;
    private javax.swing.JTextField textFieldTransitionTags;
    // End of variables declaration//GEN-END:variables
}
