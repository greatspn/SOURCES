/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui.net;

import common.Util;
import editor.Main;
import editor.domain.AlternateNameFunction;
import editor.domain.elements.ClockVar;
import editor.domain.elements.DtaEdge;
import editor.domain.elements.DtaLocation;
import editor.domain.Edge;
import editor.domain.EdgeAttachmentModel;
import editor.domain.EditableMultiValue;
import editor.domain.EditableValue;
import editor.domain.elements.GspnEdge;
import editor.domain.NetPage;
import editor.domain.ProjectPage;
import editor.domain.Node;
import editor.domain.elements.Place;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectImageResource;
import editor.domain.elements.ColorClass;
import editor.domain.elements.ColorVar;
import editor.domain.elements.Transition;
import editor.domain.elements.ConstantID;
import editor.domain.elements.DtaPage;
import editor.domain.elements.GspnPage;
import editor.domain.elements.TemplateVariable;
import editor.domain.elements.TextBox;
import editor.domain.elements.TokenType;
import editor.gui.MainWindowInterface;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Insets;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Set;
import java.util.TreeSet;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JFileChooser;
import javax.swing.ListCellRenderer;
import javax.swing.SwingConstants;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

/**
 *
 * @author elvio
 */
public class NetPropertyPanel extends javax.swing.JPanel {

    NetEditorPanel editor;
    MainWindowInterface mainInterface;
    ProjectFile currFile;
    ProjectPage currPage;
    
    public NetPropertyPanel(NetEditorPanel editor) {
        this.editor = editor;
        initComponents();
        
        Util.reformatPanelsForPlatformLookAndFeel(this);
        
        // Prepare the property panel components
        ListCellRenderer<Object> listRenderer = new IndeterminateListCellRenderer();

        comboBox_EdgeAttachment.setUpdating(true);
        comboBox_EdgeAttachment.setModel(new DefaultComboBoxModel<>(EdgeAttachmentModel.values()));
        comboBox_EdgeAttachment.setRenderer(listRenderer);
        comboBox_EdgeAttachment.setUpdating(false);
        
        comboBox_FinalLocation.setUpdating(true);
        comboBox_FinalLocation.setModel(new DefaultComboBoxModel<>(DtaLocation.FinalType.values()));
        comboBox_FinalLocation.setRenderer(listRenderer);
        comboBox_FinalLocation.setUpdating(false);
        
        comboBox_TransitionType.setUpdating(true);
        comboBox_TransitionType.setModel(new DefaultComboBoxModel<>(Transition.Type.values()));
        comboBox_TransitionType.setRenderer(listRenderer);
        comboBox_TransitionType.setUpdating(false);
        
        comboBox_ConstType.setUpdating(true);
        comboBox_ConstType.setModel(new DefaultComboBoxModel<>(ConstantID.ConstType.values()));
        comboBox_ConstType.setRenderer(listRenderer);
        comboBox_ConstType.setUpdating(false); 
        
        comboBox_placeType.setUpdating(true);
        comboBox_placeType.setModel(new DefaultComboBoxModel<>(TokenType.values()));
        comboBox_placeType.setRenderer(listRenderer);
        comboBox_placeType.setUpdating(false); 

        comboBox_TemplateVarType.setUpdating(true);
        comboBox_TemplateVarType.setModel(new DefaultComboBoxModel<>(TemplateVariable.Type.values()));
        comboBox_TemplateVarType.setRenderer(listRenderer);
        comboBox_TemplateVarType.setUpdating(false); 
        
        comboBox_altNameFn.setUpdating(true);
        comboBox_altNameFn.setModel(new DefaultComboBoxModel<>(
                new AlternateNameFunction[]{
                    AlternateNameFunction.PLAIN, 
                    AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS, 
                    AlternateNameFunction.LATEX_TEXT}));
        comboBox_altNameFn.setRenderer(listRenderer);
        comboBox_altNameFn.setUpdating(false);
        
        comboBox_textSize.setUpdating(true);
        comboBox_textSize.setModel(new DefaultComboBoxModel<>(TextBox.TextSize.values()));
        comboBox_textSize.setRenderer(listRenderer);
        comboBox_textSize.setUpdating(false);
        
        comboBox_textBoxShape.setUpdating(true);
        comboBox_textBoxShape.setModel(new DefaultComboBoxModel<>(Node.ShapeType.values()));
        comboBox_textBoxShape.setRenderer(listRenderer);
        comboBox_textBoxShape.setUpdating(false);
        
        comboBox_colorDomain.setRenderer(listRenderer);
        comboBox_colorVarDomain.setRenderer(listRenderer);
        
        if (Util.isOSX()) {
            Dimension nodeRotComboSize = comboBox_NodeRotation.getPreferredSize();
            nodeRotComboSize.width = nodeRotComboSize.height * 4;
            comboBox_NodeRotation.setMinimumSize(nodeRotComboSize);
            
            toggle_textHorizLeft.putClientProperty("JButton.buttonType", "segmented");
            toggle_textHorizLeft.putClientProperty("JButton.segmentPosition", "first");
            toggle_textHorizCenter.putClientProperty("JButton.buttonType", "segmented");
            toggle_textHorizCenter.putClientProperty("JButton.segmentPosition", "middle");
            toggle_textHorizRight.putClientProperty("JButton.buttonType", "segmented");
            toggle_textHorizRight.putClientProperty("JButton.segmentPosition", "last");
            
            toggle_textVertTop.putClientProperty("JButton.buttonType", "segmented");
            toggle_textVertTop.putClientProperty("JButton.segmentPosition", "first");
            toggle_textVertCenter.putClientProperty("JButton.buttonType", "segmented");
            toggle_textVertCenter.putClientProperty("JButton.segmentPosition", "middle");
            toggle_textVertBottom.putClientProperty("JButton.buttonType", "segmented");
            toggle_textVertBottom.putClientProperty("JButton.segmentPosition", "last");
            
            toggle_textBold.putClientProperty("JButton.buttonType", "segmented");
            toggle_textBold.putClientProperty("JButton.segmentPosition", "first");
            toggle_textItalic.putClientProperty("JButton.buttonType", "segmented");
            toggle_textItalic.putClientProperty("JButton.segmentPosition", "last");
        }
        Insets iconButtonInsets = null;
        if (Util.isWindows())
            iconButtonInsets = new Insets(2, 2, 2, 2);
        else if (Util.isOSX())
            iconButtonInsets = new Insets(4, 0, 4, 0);
        if (iconButtonInsets != null) {
            toggle_textHorizLeft.setMargin(iconButtonInsets);
            toggle_textHorizCenter.setMargin(iconButtonInsets);
            toggle_textHorizRight.setMargin(iconButtonInsets);
            toggle_textVertTop.setMargin(iconButtonInsets);
            toggle_textVertCenter.setMargin(iconButtonInsets);
            toggle_textVertBottom.setMargin(iconButtonInsets);
            toggle_textBold.setMargin(iconButtonInsets);
            toggle_textItalic.setMargin(iconButtonInsets);
        }
    }
    
    public void setPanelEnabledFor(ProjectFile pf, ProjectPage page, 
                                   MainWindowInterface newMainInterface)
    {
        currFile = pf;
        currPage = page;
        mainInterface = newMainInterface;
    }
    
    public void setPanelDisabled() {
        currFile = null;
        currPage = null;
        mainInterface = null;
    }
    
    public void updatePropertyPanel(ProjectData pf, NetPage page, boolean canBeUsed) {
        assert mainInterface != null;
        currPage = page;
        
        boolean isGspn = (page instanceof GspnPage);
        boolean isDta = (page instanceof DtaPage);
        comboBox_TemplateVarType.setUpdating(true);
        if (isGspn)
            comboBox_TemplateVarType.setModel(new DefaultComboBoxModel<>(TemplateVariable.GSPN_TEMPLATE_VAR_TYPES));
        else if (isDta)
            comboBox_TemplateVarType.setModel(new DefaultComboBoxModel<>(TemplateVariable.DTA_TEMPLATE_VAR_TYPES));
        else
            throw new IllegalStateException("Add a new list of supported template variables for this page type.");
        comboBox_TemplateVarType.setUpdating(false);        

        
        int selPlacesCount = 0;
        int selTransitionsCount = 0;
        int selGspnArcsCount = 0;
        int selDtaLocationsCount = 0;
        int selDtaEdgesCount = 0;
        int selNodesCount = 0;
        int selEdgesCount = 0;
        int selConstantsCount = 0;
        int selTemplateVars = 0;
        int selClockVars = 0;
        int selTextBoxes = 0;
        int selColorClasses = 0;
        int selColorVars = 0;
        int selObjWithColorDomain = 0;
        
        Node singleSelNode = null;
        final String indeterminate = "---";
        EditableMultiValue emvWidth = new EditableMultiValue(indeterminate);
        EditableMultiValue emvHeight = new EditableMultiValue(indeterminate);
        EditableMultiValue emvRotation = new EditableMultiValue(indeterminate);
        EditableMultiValue emvInitMarking = new EditableMultiValue(indeterminate);
        EditableMultiValue emvEdgeAttachment = new EditableMultiValue(indeterminate);
        EditableMultiValue emvAltNameFn = new EditableMultiValue(indeterminate);
        EditableMultiValue emvSuperPosTagsFn = new EditableMultiValue(indeterminate);
        EditableMultiValue emvPlaceType = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        EditableMultiValue emvPlacePartition = new EditableMultiValue(indeterminate);
        
        EditableMultiValue emvColorDom = new EditableMultiValue(indeterminate);
        
        EditableMultiValue emvTrnType = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTrnDelay = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTrnPriority = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTrnWeight = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTrnNumSrvs = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTrnGuard = new EditableMultiValue(indeterminate);
                
        EditableMultiValue emvDtaInitial = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        EditableMultiValue emvDtaFinalType = new EditableMultiValue(indeterminate);
        EditableMultiValue emvDtaSpExpr = new EditableMultiValue(indeterminate);
        EditableMultiValue emvDtaVarFlow = new EditableMultiValue(indeterminate);
        
        EditableMultiValue emvConstType = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        EditableMultiValue emvConstValue = new EditableMultiValue(indeterminate);
        boolean constToParamConvertible = true;

        EditableMultiValue emvTemplVarType = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        boolean paramToConstConvertible = true;
        
        EditableMultiValue emvTextBoxText = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTextBoxHorizLeft = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxHorizCenter = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxHorizRight = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxVertTop = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxVertCenter = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxVertBottom = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxTextColor = new EditableMultiValue(ColorPickerForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxTextSize = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTextBoxFillColor = new EditableMultiValue(ColorPickerForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxBorderColor = new EditableMultiValue(ColorPickerForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxShadow = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxShapeType = new EditableMultiValue(indeterminate);
        EditableMultiValue emvTextBoxLocked = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxBold = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxItalic = new EditableMultiValue(ToggleButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxSelectImg = new EditableMultiValue(ButtonForEditable.INDETERMINATE);
        EditableMultiValue emvTextBoxClearImg = new EditableMultiValue(ButtonForEditable.INDETERMINATE);
        
        EditableMultiValue emvColorClassDef = new EditableMultiValue(indeterminate);

        EditableMultiValue emvColorVarDomain = new EditableMultiValue(indeterminate);
        
        DefaultComboBoxModel<String> colorDomainModel = new DefaultComboBoxModel<>();
        colorDomainModel.addElement(ColorClass.UNCOLORED_DOMAIN_NAME);
        
        DefaultComboBoxModel<String> simpleColorClassModel = new DefaultComboBoxModel<>();
        
        Set<String> uniqueKroneckerPartitions = new TreeSet<>();
;
        for (Node n : page.nodes) {
            if (n instanceof ColorClass) {
                ColorClass cc = (ColorClass)n;
                colorDomainModel.addElement(cc.getUniqueName());
                if (!cc.isCrossDomain())
                    simpleColorClassModel.addElement(cc.getUniqueName());
            }
            if (n instanceof Place) {
                String part = ((Place)n).getKroneckerPartitionNameEditable().getValue().toString();
                if (part.length() > 0 && !uniqueKroneckerPartitions.contains(part))
                    uniqueKroneckerPartitions.add(part);
            }
            // Take properties of selected nodes.
            if (!n.isSelected())
                continue;
            selNodesCount++;
            singleSelNode = (selNodesCount == 1) ? n : null;
            emvWidth.add(n.getWidthEditable());
            emvHeight.add(n.getHeightEditable());
            emvRotation.add(n.getRotationEditable());
            emvEdgeAttachment.add(n.getEdgeAttachmentEditable());
            emvAltNameFn.add(n.getAlternateNameFunctionEditable());
            emvSuperPosTagsFn.add(n.getSuperPosTagsLabelEditable());
            if (n instanceof ColorClass.DomainHolder) { // places, constantIDs, ...
                selObjWithColorDomain++;
                emvColorDom.add(((ColorClass.DomainHolder)n).getColorDomainEditable());
            }
            if (n instanceof Place) {
                selPlacesCount++;
                emvInitMarking.add(((Place)n).getCentralEditable(n.getCenterX(), n.getCenterY()));
                emvPlaceType.add(((Place)n).getTypeEditable());
                emvPlacePartition.add(((Place)n).getKroneckerPartitionNameEditable());
            }
            if (n instanceof DtaLocation) {
                selDtaLocationsCount++;
                emvDtaInitial.add(((DtaLocation)n).getInitialEditable());
                emvDtaFinalType.add(((DtaLocation)n).getFinalTypeEditable());
                emvDtaSpExpr.add(((DtaLocation)n).getStatePropositionEditable());
                emvDtaVarFlow.add(((DtaLocation)n).getVarFlowEditable());
            }
            if (n instanceof Transition) {
                selTransitionsCount++;
                emvTrnType.add(((Transition)n).getTransitionTypeEditable());
                emvTrnDelay.add(((Transition)n).getDelayEditable());
                emvTrnPriority.add(((Transition)n).getPriorityEditable());
                emvTrnWeight.add(((Transition)n).getWeightEditable());
                emvTrnNumSrvs.add(((Transition)n).getNumberOfServersEditable());
                emvTrnGuard.add(((Transition)n).getGuardEditable());
            }
            if (n instanceof ConstantID) {
                selConstantsCount++;
                emvConstType.add(((ConstantID)n).getTypeEditable());
                emvConstValue.add(((ConstantID)n).getConstantExpr().getEditableValue());
                constToParamConvertible = constToParamConvertible && ((ConstantID)n).isCovertibleToTemplate();
            }
            if (n instanceof TemplateVariable) {
                selTemplateVars++;
                emvTemplVarType.add(((TemplateVariable)n).getTypeEditable());
                paramToConstConvertible = paramToConstConvertible && ((TemplateVariable)n).isCovertibleToConst();
            }
            if (n instanceof  ClockVar) {
                selClockVars++;
            }
            if (n instanceof TextBox) {
                selTextBoxes++;
                emvTextBoxText.add(((TextBox)n).getCentralEditable(0, 0));
                emvTextBoxHorizLeft.add(((TextBox)n).getHorizPosEditable(SwingConstants.LEFT));
                emvTextBoxHorizCenter.add(((TextBox)n).getHorizPosEditable(SwingConstants.CENTER));
                emvTextBoxHorizRight.add(((TextBox)n).getHorizPosEditable(SwingConstants.RIGHT));
                emvTextBoxVertTop.add(((TextBox)n).getVertPosEditable(SwingConstants.TOP));
                emvTextBoxVertCenter.add(((TextBox)n).getVertPosEditable(SwingConstants.CENTER));
                emvTextBoxVertBottom.add(((TextBox)n).getVertPosEditable(SwingConstants.BOTTOM));
                emvTextBoxTextColor.add(((TextBox)n).getTextColorEditable());
                emvTextBoxTextSize.add(((TextBox)n).getTextSizeEditable());
                emvTextBoxFillColor.add(((TextBox)n).getFillColorEditable());
                emvTextBoxBorderColor.add(((TextBox)n).getBorderColorEditable());
                emvTextBoxShadow.add(((TextBox)n).getHasShadowEditable());
                emvTextBoxShapeType.add(((TextBox)n).getShapeTypeEditable());
                emvTextBoxLocked.add(((TextBox)n).getLockEditable());
                emvTextBoxBold.add(((TextBox)n).getTextBoldEditable());
                emvTextBoxItalic.add(((TextBox)n).getTextItalicEditable());
                emvTextBoxSelectImg.add(((TextBox)n).getSelectImageEditable());
                emvTextBoxClearImg.add(((TextBox)n).getClearImageEditable());
            }
            if (n instanceof ColorClass) {
                selColorClasses++;
                emvColorClassDef.add(((ColorClass)n).getColorClassDef().getEditableValue());
            }
            if (n instanceof ColorVar) {
                selColorVars++;
                emvColorVarDomain.add(((ColorVar)n).getDomainExpr().getEditableValue());
            }
        }
        
        comboBox_colorDomain.setUpdating(true);
        comboBox_colorDomain.setModel(colorDomainModel);
        comboBox_colorDomain.setUpdating(false);
        
        comboBox_colorVarDomain.setUpdating(true);
        comboBox_colorVarDomain.setModel(simpleColorClassModel);
        comboBox_colorVarDomain.setUpdating(false);
        
        DefaultComboBoxModel<String> kroneckerPartitions = new DefaultComboBoxModel<>();
        for (String part : uniqueKroneckerPartitions)
            kroneckerPartitions.addElement(part);
        comboBox_placePartition.setUpdating(true);
        comboBox_placePartition.setModel(kroneckerPartitions);
        comboBox_placePartition.setUpdating(false);

        EditableMultiValue emvArcMult = new EditableMultiValue(indeterminate);
        EditableMultiValue emvBrokenEdge = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        
        EditableMultiValue emvBoundaryEdge = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        EditableMultiValue emvResetClockEdge = new EditableMultiValue(CheckBoxForEditable.INDETERMINATE);
        EditableMultiValue emvActSetEdge = new EditableMultiValue(indeterminate);
        EditableMultiValue emvClockGuardEdge = new EditableMultiValue(indeterminate);
        
        for (Edge e : page.edges) {
            if (!e.isSelected())
                continue;
            selEdgesCount++;
            emvBrokenEdge.add(e.getBrokenFlagEditable());
            if (e instanceof GspnEdge) {
                selGspnArcsCount++;
                emvArcMult.add(((GspnEdge)e).getMultiplicityEditable());
            }
            if (e instanceof DtaEdge) {
                selDtaEdgesCount++;
                emvBoundaryEdge.add(((DtaEdge)e).getBoundaryEditable());
                emvResetClockEdge.add(((DtaEdge)e).getResetClockEditable());
                emvActSetEdge.add(((DtaEdge)e).getActSetEditable());
                emvClockGuardEdge.add(((DtaEdge)e).getClockGuardEditable());
            }
        }
        
        jPanelPlaceProperties.setVisible(canBeUsed && selPlacesCount > 0);
        jPanelTransitionProperties.setVisible(canBeUsed && selTransitionsCount > 0);
        jPanelDtaLocationProperty.setVisible(canBeUsed && selDtaLocationsCount > 0);
        jPanelGspnArcProperties.setVisible(canBeUsed && selGspnArcsCount > 0);
        jPanelDtaEdgeProperty.setVisible(canBeUsed && selDtaEdgesCount > 0);
        jPanelNodeProperties.setVisible(canBeUsed && selNodesCount > 0);
        jPanelEdgeProperties.setVisible(canBeUsed && selEdgesCount > 0);
        jPanelConstants.setVisible(canBeUsed && selConstantsCount > 0);
        jPanelTemplateVariables.setVisible(canBeUsed && selTemplateVars > 0);
        jPanelTextBoxes.setVisible(canBeUsed && selTextBoxes > 0);
        jPanelColorClasses.setVisible(canBeUsed && selColorClasses > 0);
        jPanelColorVars.setVisible(canBeUsed && selColorVars > 0);
        jPanelColorDomain.setVisible(canBeUsed && selObjWithColorDomain > 0);
        
        // Unique ID
        boolean hasUniqueID = (selNodesCount == 1) && !(singleSelNode instanceof TextBox); // TODO: should be a hasUniqueName method.
        if (hasUniqueID) {
            assert singleSelNode != null;
            textField_UniqueName.setEditableProxy(pf, page, true, singleSelNode.getUniqueNameEditable());
//            String altName = singleSelNode.getAlternateName();
            AlternateNameFunction anf = singleSelNode.getAlternateNameFunction();
            if (anf.requiresAlternateText) {
                textField_alternateName.setEditableProxy(pf, page, true, singleSelNode.getAlternateNameEditable());
                jLabel_altName.setEnabled(true);
            }
            else {
                textField_alternateName.setEditableProxy(null, null, false, null);
                jLabel_altName.setEnabled(false);
            }
        }
        else {
            textField_UniqueName.setEditableProxy(null, null, false, null);
            textField_alternateName.setEditableProxy(null, null, false, null);
            jLabel_altName.setEnabled(false);
        }
        
        // Edge Attachment Model
        comboBox_EdgeAttachment.setEditableProxy(pf, page, (selNodesCount > 0), emvEdgeAttachment);
        // Altername name function
        comboBox_altNameFn.setEditableProxy(pf, page, (selNodesCount > 0), emvAltNameFn);
        // Width, height, rotation
        textField_NodeWidth.setEditableProxy(pf, page, true, emvWidth);
        textField_NodeHeight.setEditableProxy(pf, page, true, emvHeight);
        comboBox_NodeRotation.setEditableProxy(pf, page, true, emvRotation);
        // super-position tags
        textField_superPos.setEditableProxy(pf, page, true, emvSuperPosTagsFn);
        
        // GSPN places
        textField_initMarking.setEditableProxy(pf, page, (selPlacesCount == selNodesCount), emvInitMarking);
        comboBox_placeType.setEditableProxy(pf, page, (selPlacesCount == selNodesCount), emvPlaceType);
        comboBox_placePartition.setEditableProxy(pf, page, (selPlacesCount == selNodesCount), emvPlacePartition);
        
        comboBox_colorDomain.setEditableProxy(pf, page, (selObjWithColorDomain == selNodesCount), emvColorDom);
        
        // DTA location
        checkBox_InitialLocation.setEditableProxy(pf, page, (selDtaLocationsCount == selNodesCount), emvDtaInitial);
        comboBox_FinalLocation.setEditableProxy(pf, page, (selDtaLocationsCount == selNodesCount), emvDtaFinalType);
        textField_StatePropExpr.setEditableProxy(pf, page, (selDtaLocationsCount == selNodesCount), emvDtaSpExpr);
        textField_varFlowExpr.setEditableProxy(pf, page, (selDtaLocationsCount == selNodesCount), emvDtaVarFlow);
        // GSPN transitions
        comboBox_TransitionType.setEditableProxy(pf, page, (selTransitionsCount == selNodesCount), emvTrnType);
        textField_TransitionDelay.setEditableProxy(pf, page, (selTransitionsCount == selNodesCount), emvTrnDelay);
        textField_TransitionPriority.setEditableProxy(pf, page, (selTransitionsCount == selNodesCount), emvTrnPriority);
        textField_TransitionWeight.setEditableProxy(pf, page, (selTransitionsCount == selNodesCount), emvTrnWeight);
        textField_TransitionNumServers.setEditableProxy(pf, page, (selTransitionsCount == selNodesCount), emvTrnNumSrvs);
        textField_TransitionGuard.setEditableProxy(pf, page, (selTransitionsCount == selNodesCount), emvTrnGuard);
        
        // TextBox styles
        textField_textBoxText.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxText);
        toggle_textHorizLeft.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxHorizLeft);
        toggle_textHorizCenter.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxHorizCenter);
        toggle_textHorizRight.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxHorizRight);
        toggle_textVertTop.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxVertTop);
        toggle_textVertCenter.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxVertCenter);
        toggle_textVertBottom.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxVertBottom);
        color_textBoxText.setEditableProxy((selTextBoxes == selNodesCount), emvTextBoxTextColor);
        comboBox_textSize.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxTextSize);
        color_textBoxFill.setEditableProxy((selTextBoxes == selNodesCount), emvTextBoxFillColor);
        color_textBoxBorder.setEditableProxy((selTextBoxes == selNodesCount), emvTextBoxBorderColor);
        checkBox_textBoxShadow.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxShadow);
        checkBox_textBoxShadow.setEnabled((selTextBoxes == selNodesCount) && emvTextBoxFillColor.getValue()!=null);
        comboBox_textBoxShape.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxShapeType);
        checkBox_textBoxLocked.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxLocked);
        toggle_textBold.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxBold);
        toggle_textItalic.setEditableProxy(pf, page, (selTextBoxes == selNodesCount), emvTextBoxItalic);
        button_textBoxImageSelect.setEditableProxy((selTextBoxes == selNodesCount), emvTextBoxSelectImg);
        button_textBoxImageClear.setEditableProxy((selTextBoxes == selNodesCount), emvTextBoxClearImg);
        
        Object commonTrnType = emvTrnType.getValue();
        if (commonTrnType == emvTrnType.getIndeterminateObj())
            jLabel_TransitionDelay.setText("Rate/Delay:");
        else
            jLabel_TransitionDelay.setText((commonTrnType == Transition.Type.EXP) ?"Rate:" : "Delay:");
        
        // Constants
        textField_ConstValue.setEditableProxy(pf, page, (selConstantsCount == selNodesCount), emvConstValue);
        comboBox_ConstType.setEditableProxy(pf, page, (selConstantsCount == selNodesCount), emvConstType);
        buttonConstantsToTemplates.setEnabled(selConstantsCount == selNodesCount && constToParamConvertible);
        
        // Template variables
        comboBox_TemplateVarType.setEditableProxy(pf, page, (selTemplateVars == selNodesCount), emvTemplVarType);
        buttonTemplatesToConstants.setEnabled(selTemplateVars == selNodesCount && paramToConstConvertible);
        
        // Color classes
        textField_colorClassDef.setEditableProxy(pf, page, (selColorClasses == selNodesCount), emvColorClassDef);
        
        // Color variables
        comboBox_colorVarDomain.setEditableProxy(pf, page, (selColorVars == selNodesCount), emvColorVarDomain);
        
        // Broken edge flag
        checkBox_BrokenEdge.setEditableProxy(pf, page, true, emvBrokenEdge);
        
        // GSPN arc multiplicity
        textField_ArcMultiplicity.setEnabled(selEdgesCount == selGspnArcsCount);
        textField_ArcMultiplicity.setEditableProxy(pf, page, (selEdgesCount == selGspnArcsCount), emvArcMult);
        
        // DTA Edge fields
        checkBox_BoundaryEdge.setEditableProxy(pf, page, (selEdgesCount==selDtaEdgesCount), emvBoundaryEdge);
        checkBox_ResetClockEdge.setEditableProxy(pf, page, (selEdgesCount==selDtaEdgesCount), emvResetClockEdge);
        textField_ActSet.setEditableProxy(pf, page, (selEdgesCount==selDtaEdgesCount), emvActSetEdge);
        textField_ClockGuard.setEditableProxy(pf, page, (selEdgesCount==selDtaEdgesCount), emvClockGuardEdge);
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

        resourceFactory = new editor.gui.ResourceFactory();
        jPanelNodeProperties = new javax.swing.JPanel();
        jLabel_ID = new javax.swing.JLabel();
        jLabel_EdgeAttachment = new javax.swing.JLabel();
        textField_UniqueName = new editor.gui.net.TextFieldForEditable();
        jPanel3 = new javax.swing.JPanel();
        jLabel_NodeRotation = new javax.swing.JLabel();
        comboBox_NodeRotation = new editor.gui.net.ComboBoxForEditable();
        jLabel_altNameFn = new javax.swing.JLabel();
        comboBox_altNameFn = new editor.gui.net.ComboBoxForEditable<AlternateNameFunction>();
        jPanel4 = new javax.swing.JPanel();
        jLabel_NodeWidth = new javax.swing.JLabel();
        jLabel_NodeHeight = new javax.swing.JLabel();
        textField_NodeWidth = new editor.gui.net.TextFieldForEditable();
        textField_NodeHeight = new editor.gui.net.TextFieldForEditable();
        comboBox_EdgeAttachment = new editor.gui.net.ComboBoxForEditable<EdgeAttachmentModel>();
        jPanel6 = new javax.swing.JPanel();
        textField_alternateName = new editor.gui.net.TextFieldForEditable();
        jLabel_altName = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        textField_superPos = new editor.gui.net.TextFieldForEditable();
        jLabel_superPos = new javax.swing.JLabel();
        jPanelPlaceProperties = new javax.swing.JPanel();
        jLabel_InitMarking = new javax.swing.JLabel();
        textField_initMarking = new editor.gui.net.TextFieldForEditable();
        jLabel_placeType = new javax.swing.JLabel();
        comboBox_placeType = new editor.gui.net.ComboBoxForEditable<TokenType>();
        jLabel_placePartition = new javax.swing.JLabel();
        comboBox_placePartition = new editor.gui.net.ComboBoxForEditable<String>();
        jPanelTransitionProperties = new javax.swing.JPanel();
        jLabel_TransitionType = new javax.swing.JLabel();
        comboBox_TransitionType = new editor.gui.net.ComboBoxForEditable<Transition.Type>();
        jLabel_TransitionPriority = new javax.swing.JLabel();
        textField_TransitionPriority = new editor.gui.net.TextFieldForEditable();
        jLabel_TransitionDelay = new javax.swing.JLabel();
        textField_TransitionDelay = new editor.gui.net.TextFieldForEditable();
        jLabel_TransitionWeight = new javax.swing.JLabel();
        textField_TransitionWeight = new editor.gui.net.TextFieldForEditable();
        jLabel_TransitionNumServers = new javax.swing.JLabel();
        textField_TransitionNumServers = new editor.gui.net.TextFieldForEditable();
        jLabel_TransitionGuard = new javax.swing.JLabel();
        textField_TransitionGuard = new editor.gui.net.TextFieldForEditable();
        jPanelEdgeProperties = new javax.swing.JPanel();
        checkBox_BrokenEdge = new editor.gui.net.CheckBoxForEditable();
        jPanelGspnArcProperties = new javax.swing.JPanel();
        textField_ArcMultiplicity = new editor.gui.net.TextFieldForEditable();
        jLabel_ArcMultiplicity = new javax.swing.JLabel();
        jPanelTemplateVariables = new javax.swing.JPanel();
        jLabelTemplateVarType = new javax.swing.JLabel();
        comboBox_TemplateVarType = new editor.gui.net.ComboBoxForEditable<TemplateVariable.Type>();
        buttonTemplatesToConstants = new javax.swing.JButton();
        jPanel_FillVert = new javax.swing.JPanel();
        jPanelDtaLocationProperty = new javax.swing.JPanel();
        checkBox_InitialLocation = new editor.gui.net.CheckBoxForEditable();
        jLabel_FinalDtaLocation = new javax.swing.JLabel();
        comboBox_FinalLocation = new editor.gui.net.ComboBoxForEditable<DtaLocation.FinalType>();
        jLabel_StatePropExpr = new javax.swing.JLabel();
        textField_StatePropExpr = new editor.gui.net.TextFieldForEditable();
        textField_varFlowExpr = new editor.gui.net.TextFieldForEditable();
        jLabel_varFlowExpr = new javax.swing.JLabel();
        jPanelDtaEdgeProperty = new javax.swing.JPanel();
        checkBox_BoundaryEdge = new editor.gui.net.CheckBoxForEditable();
        textField_ActSet = new editor.gui.net.TextFieldForEditable();
        checkBox_ResetClockEdge = new editor.gui.net.CheckBoxForEditable();
        textField_ClockGuard = new editor.gui.net.TextFieldForEditable();
        jLabel_ClockGuard = new javax.swing.JLabel();
        jLabel_ActSet = new javax.swing.JLabel();
        jPanelConstants = new javax.swing.JPanel();
        jLabel_ConstValue = new javax.swing.JLabel();
        textField_ConstValue = new editor.gui.net.TextFieldForEditable();
        comboBox_ConstType = new editor.gui.net.ComboBoxForEditable<ConstantID.ConstType>();
        buttonConstantsToTemplates = new javax.swing.JButton();
        jPanelTextBoxes = new javax.swing.JPanel();
        jLabel_textBoxText = new javax.swing.JLabel();
        textField_textBoxText = new editor.gui.net.TextFieldForEditable();
        jPanel_textBoxHorizVerts = new javax.swing.JPanel();
        toggle_textHorizLeft = new editor.gui.net.ToggleButtonForEditable();
        toggle_textHorizCenter = new editor.gui.net.ToggleButtonForEditable();
        toggle_textHorizRight = new editor.gui.net.ToggleButtonForEditable();
        toggle_textVertTop = new editor.gui.net.ToggleButtonForEditable();
        toggle_textVertCenter = new editor.gui.net.ToggleButtonForEditable();
        toggle_textVertBottom = new editor.gui.net.ToggleButtonForEditable();
        jPanel2 = new javax.swing.JPanel();
        color_textBoxText = new editor.gui.net.ColorPickerForEditable();
        comboBox_textSize = new editor.gui.net.ComboBoxForEditable<TextBox.TextSize>();
        toggle_textBold = new editor.gui.net.ToggleButtonForEditable();
        toggle_textItalic = new editor.gui.net.ToggleButtonForEditable();
        jPanel5 = new javax.swing.JPanel();
        label_fillColor = new javax.swing.JLabel();
        color_textBoxFill = new editor.gui.net.ColorPickerForEditable();
        checkBox_textBoxShadow = new editor.gui.net.CheckBoxForEditable();
        jLabel_textBoxBorder = new javax.swing.JLabel();
        color_textBoxBorder = new editor.gui.net.ColorPickerForEditable();
        comboBox_textBoxShape = new editor.gui.net.ComboBoxForEditable<Node.ShapeType>();
        jPanel7 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        checkBox_textBoxLocked = new editor.gui.net.CheckBoxForEditable();
        jPanel8 = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        button_textBoxImageSelect = new editor.gui.net.ButtonForEditable();
        button_textBoxImageClear = new editor.gui.net.ButtonForEditable();
        jPanelColorClasses = new javax.swing.JPanel();
        jLabel_colorClassDef = new javax.swing.JLabel();
        textField_colorClassDef = new editor.gui.net.TextFieldForEditable();
        jPanelColorVars = new javax.swing.JPanel();
        jLabel_colorVarDomain = new javax.swing.JLabel();
        comboBox_colorVarDomain = new editor.gui.net.ComboBoxForEditable<String>();
        jPanelColorDomain = new javax.swing.JPanel();
        comboBox_colorDomain = new editor.gui.net.ComboBoxForEditable<String>();
        jLabel_colorDomain = new javax.swing.JLabel();

        setLayout(new java.awt.GridBagLayout());

        jPanelNodeProperties.setBorder(javax.swing.BorderFactory.createTitledBorder("Node properties"));
        jPanelNodeProperties.setLayout(new java.awt.GridBagLayout());

        jLabel_ID.setText("ID:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelNodeProperties.add(jLabel_ID, gridBagConstraints);

        jLabel_EdgeAttachment.setText("Magnets:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelNodeProperties.add(jLabel_EdgeAttachment, gridBagConstraints);

        textField_UniqueName.setText("unique name editable");
        textField_UniqueName.setSynchLabel(jLabel_ID);
        textField_UniqueName.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_UniqueNameActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelNodeProperties.add(textField_UniqueName, gridBagConstraints);

        jPanel3.setLayout(new java.awt.GridBagLayout());

        jLabel_NodeRotation.setIcon(resourceFactory.getRotation24());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 3);
        jPanel3.add(jLabel_NodeRotation, gridBagConstraints);

        comboBox_NodeRotation.setEditable(true);
        comboBox_NodeRotation.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "0", "90", "180", "270" }));
        comboBox_NodeRotation.setSelectedIndex(3);
        comboBox_NodeRotation.setMinimumSize(new java.awt.Dimension(45, 20));
        comboBox_NodeRotation.setSynchLabel(jLabel_NodeRotation);
        comboBox_NodeRotation.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_NodeRotationActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.VERTICAL;
        gridBagConstraints.weighty = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel3.add(comboBox_NodeRotation, gridBagConstraints);

        jLabel_altNameFn.setText("Label:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanel3.add(jLabel_altNameFn, gridBagConstraints);

        comboBox_altNameFn.setSynchLabel(jLabel_altNameFn);
        comboBox_altNameFn.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_altNameFnActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.weighty = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel3.add(comboBox_altNameFn, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        jPanelNodeProperties.add(jPanel3, gridBagConstraints);

        jPanel4.setLayout(new java.awt.GridBagLayout());

        jLabel_NodeWidth.setIcon(resourceFactory.getResizeHoriz24());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 2);
        jPanel4.add(jLabel_NodeWidth, gridBagConstraints);

        jLabel_NodeHeight.setIcon(resourceFactory.getResizeVert24());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 8, 0, 2);
        jPanel4.add(jLabel_NodeHeight, gridBagConstraints);

        textField_NodeWidth.setText("w");
        textField_NodeWidth.setSynchLabel(jLabel_NodeWidth);
        textField_NodeWidth.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_NodeWidthActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel4.add(textField_NodeWidth, gridBagConstraints);

        textField_NodeHeight.setText("h");
        textField_NodeHeight.setSynchLabel(jLabel_NodeHeight);
        textField_NodeHeight.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_NodeHeightActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel4.add(textField_NodeHeight, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        jPanelNodeProperties.add(jPanel4, gridBagConstraints);

        comboBox_EdgeAttachment.setMaximumRowCount(10);
        comboBox_EdgeAttachment.setSynchLabel(jLabel_EdgeAttachment);
        comboBox_EdgeAttachment.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_EdgeAttachmentActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelNodeProperties.add(comboBox_EdgeAttachment, gridBagConstraints);

        jPanel6.setLayout(new java.awt.GridBagLayout());

        textField_alternateName.setText("latex");
        textField_alternateName.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_alternateNameActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel6.add(textField_alternateName, gridBagConstraints);

        jLabel_altName.setIcon(resourceFactory.getLatexText24());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanel6.add(jLabel_altName, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        jPanelNodeProperties.add(jPanel6, gridBagConstraints);

        jPanel1.setLayout(new java.awt.GridBagLayout());

        textField_superPos.setText("super position tags");
        textField_superPos.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_superPosActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel1.add(textField_superPos, gridBagConstraints);

        jLabel_superPos.setText("Tags:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(3, 0, 3, 3);
        jPanel1.add(jLabel_superPos, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 0);
        jPanelNodeProperties.add(jPanel1, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        add(jPanelNodeProperties, gridBagConstraints);

        jPanelPlaceProperties.setBorder(javax.swing.BorderFactory.createTitledBorder("Place properties"));
        jPanelPlaceProperties.setLayout(new java.awt.GridBagLayout());

        jLabel_InitMarking.setText("Initial marking:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelPlaceProperties.add(jLabel_InitMarking, gridBagConstraints);

        textField_initMarking.setText("init marking");
        textField_initMarking.setSynchLabel(jLabel_InitMarking);
        textField_initMarking.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_initMarkingActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelPlaceProperties.add(textField_initMarking, gridBagConstraints);

        jLabel_placeType.setText("Type:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelPlaceProperties.add(jLabel_placeType, gridBagConstraints);

        comboBox_placeType.setSynchLabel(jLabel_placeType);
        comboBox_placeType.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_placeTypeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelPlaceProperties.add(comboBox_placeType, gridBagConstraints);

        jLabel_placePartition.setText("Kronecker partition:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelPlaceProperties.add(jLabel_placePartition, gridBagConstraints);

        comboBox_placePartition.setEditable(true);
        comboBox_placePartition.setSynchLabel(jLabel_placePartition);
        comboBox_placePartition.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_placePartitionActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelPlaceProperties.add(comboBox_placePartition, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        add(jPanelPlaceProperties, gridBagConstraints);

        jPanelTransitionProperties.setBorder(javax.swing.BorderFactory.createTitledBorder("Transition properties"));
        jPanelTransitionProperties.setLayout(new java.awt.GridBagLayout());

        jLabel_TransitionType.setText("Type:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelTransitionProperties.add(jLabel_TransitionType, gridBagConstraints);

        comboBox_TransitionType.setSynchLabel(jLabel_TransitionType);
        comboBox_TransitionType.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_TransitionTypeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTransitionProperties.add(comboBox_TransitionType, gridBagConstraints);

        jLabel_TransitionPriority.setText("Priority:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelTransitionProperties.add(jLabel_TransitionPriority, gridBagConstraints);

        textField_TransitionPriority.setText("priority");
        textField_TransitionPriority.setSynchLabel(jLabel_TransitionPriority);
        textField_TransitionPriority.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_TransitionPriorityActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTransitionProperties.add(textField_TransitionPriority, gridBagConstraints);

        jLabel_TransitionDelay.setText("Delay:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelTransitionProperties.add(jLabel_TransitionDelay, gridBagConstraints);

        textField_TransitionDelay.setText("delay");
        textField_TransitionDelay.setSynchLabel(jLabel_TransitionDelay);
        textField_TransitionDelay.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_TransitionDelayActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTransitionProperties.add(textField_TransitionDelay, gridBagConstraints);

        jLabel_TransitionWeight.setText("Weight:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelTransitionProperties.add(jLabel_TransitionWeight, gridBagConstraints);

        textField_TransitionWeight.setText("weight");
        textField_TransitionWeight.setSynchLabel(jLabel_TransitionWeight);
        textField_TransitionWeight.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_TransitionWeightActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTransitionProperties.add(textField_TransitionWeight, gridBagConstraints);

        jLabel_TransitionNumServers.setText("N. servers:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelTransitionProperties.add(jLabel_TransitionNumServers, gridBagConstraints);

        textField_TransitionNumServers.setText("number of servers");
        textField_TransitionNumServers.setSynchLabel(jLabel_TransitionNumServers);
        textField_TransitionNumServers.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_TransitionNumServersActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTransitionProperties.add(textField_TransitionNumServers, gridBagConstraints);

        jLabel_TransitionGuard.setText("Guard:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelTransitionProperties.add(jLabel_TransitionGuard, gridBagConstraints);

        textField_TransitionGuard.setText("guard");
        textField_TransitionGuard.setSynchLabel(jLabel_TransitionGuard);
        textField_TransitionGuard.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_TransitionGuardActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTransitionProperties.add(textField_TransitionGuard, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        add(jPanelTransitionProperties, gridBagConstraints);

        jPanelEdgeProperties.setBorder(javax.swing.BorderFactory.createTitledBorder("Edge visualization"));
        jPanelEdgeProperties.setLayout(new java.awt.GridBagLayout());

        checkBox_BrokenEdge.setText("Broken");
        checkBox_BrokenEdge.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_BrokenEdgeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelEdgeProperties.add(checkBox_BrokenEdge, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        add(jPanelEdgeProperties, gridBagConstraints);

        jPanelGspnArcProperties.setBorder(javax.swing.BorderFactory.createTitledBorder("GSPN arc properties"));
        jPanelGspnArcProperties.setLayout(new java.awt.GridBagLayout());

        textField_ArcMultiplicity.setText("arc multiplicity");
        textField_ArcMultiplicity.setSynchLabel(jLabel_ArcMultiplicity);
        textField_ArcMultiplicity.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_ArcMultiplicityActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelGspnArcProperties.add(textField_ArcMultiplicity, gridBagConstraints);

        jLabel_ArcMultiplicity.setText("Multiplicity:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelGspnArcProperties.add(jLabel_ArcMultiplicity, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        add(jPanelGspnArcProperties, gridBagConstraints);

        jPanelTemplateVariables.setBorder(javax.swing.BorderFactory.createTitledBorder("Template variable"));
        jPanelTemplateVariables.setLayout(new java.awt.GridBagLayout());

        jLabelTemplateVarType.setText("Type:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 6);
        jPanelTemplateVariables.add(jLabelTemplateVarType, gridBagConstraints);

        comboBox_TemplateVarType.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_TemplateVarTypeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTemplateVariables.add(comboBox_TemplateVarType, gridBagConstraints);

        buttonTemplatesToConstants.setText("Make constant...");
        buttonTemplatesToConstants.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonTemplatesToConstantsActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        jPanelTemplateVariables.add(buttonTemplatesToConstants, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 8;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(jPanelTemplateVariables, gridBagConstraints);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 17;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(6, 6, 6, 6);
        add(jPanel_FillVert, gridBagConstraints);

        jPanelDtaLocationProperty.setBorder(javax.swing.BorderFactory.createTitledBorder("Location properties"));
        jPanelDtaLocationProperty.setLayout(new java.awt.GridBagLayout());

        checkBox_InitialLocation.setText("Initial");
        checkBox_InitialLocation.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_InitialLocationActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaLocationProperty.add(checkBox_InitialLocation, gridBagConstraints);

        jLabel_FinalDtaLocation.setText("Final:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 3);
        jPanelDtaLocationProperty.add(jLabel_FinalDtaLocation, gridBagConstraints);

        comboBox_FinalLocation.setSynchLabel(jLabel_FinalDtaLocation);
        comboBox_FinalLocation.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_FinalLocationActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaLocationProperty.add(comboBox_FinalLocation, gridBagConstraints);

        jLabel_StatePropExpr.setText("State proposition:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelDtaLocationProperty.add(jLabel_StatePropExpr, gridBagConstraints);

        textField_StatePropExpr.setText("state proposition expression");
        textField_StatePropExpr.setSynchLabel(jLabel_StatePropExpr);
        textField_StatePropExpr.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_StatePropExprActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaLocationProperty.add(textField_StatePropExpr, gridBagConstraints);

        textField_varFlowExpr.setText("variable flows");
        textField_varFlowExpr.setSynchLabel(jLabel_varFlowExpr);
        textField_varFlowExpr.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_varFlowExprActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaLocationProperty.add(textField_varFlowExpr, gridBagConstraints);

        jLabel_varFlowExpr.setText("Flow of variables:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelDtaLocationProperty.add(jLabel_varFlowExpr, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        add(jPanelDtaLocationProperty, gridBagConstraints);

        jPanelDtaEdgeProperty.setBorder(javax.swing.BorderFactory.createTitledBorder("DTA edge properties"));
        jPanelDtaEdgeProperty.setLayout(new java.awt.GridBagLayout());

        checkBox_BoundaryEdge.setText("Boundary edge");
        checkBox_BoundaryEdge.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_BoundaryEdgeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaEdgeProperty.add(checkBox_BoundaryEdge, gridBagConstraints);

        textField_ActSet.setText("ActSet");
        textField_ActSet.setSynchLabel(jLabel_ActSet);
        textField_ActSet.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_ActSetActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaEdgeProperty.add(textField_ActSet, gridBagConstraints);

        checkBox_ResetClockEdge.setText("Reset");
        checkBox_ResetClockEdge.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_ResetClockEdgeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaEdgeProperty.add(checkBox_ResetClockEdge, gridBagConstraints);

        textField_ClockGuard.setText("Clock guard");
        textField_ClockGuard.setSynchLabel(jLabel_ClockGuard);
        textField_ClockGuard.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_ClockGuardActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelDtaEdgeProperty.add(textField_ClockGuard, gridBagConstraints);

        jLabel_ClockGuard.setText("Clock guard:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelDtaEdgeProperty.add(jLabel_ClockGuard, gridBagConstraints);

        jLabel_ActSet.setText("Actions:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelDtaEdgeProperty.add(jLabel_ActSet, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 6;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 1.0;
        add(jPanelDtaEdgeProperty, gridBagConstraints);

        jPanelConstants.setBorder(javax.swing.BorderFactory.createTitledBorder("Constant definition"));
        jPanelConstants.setLayout(new java.awt.GridBagLayout());

        jLabel_ConstValue.setText("Value:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 6, 3, 3);
        jPanelConstants.add(jLabel_ConstValue, gridBagConstraints);

        textField_ConstValue.setText("value");
        textField_ConstValue.setSynchLabel(jLabel_ConstValue);
        textField_ConstValue.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_ConstValueActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelConstants.add(textField_ConstValue, gridBagConstraints);

        comboBox_ConstType.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_ConstTypeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelConstants.add(comboBox_ConstType, gridBagConstraints);

        buttonConstantsToTemplates.setText("Make parametric...");
        buttonConstantsToTemplates.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonConstantsToTemplatesActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        jPanelConstants.add(buttonConstantsToTemplates, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 7;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(jPanelConstants, gridBagConstraints);

        jPanelTextBoxes.setBorder(javax.swing.BorderFactory.createTitledBorder("Text Boxes"));
        jPanelTextBoxes.setLayout(new java.awt.GridBagLayout());

        jLabel_textBoxText.setText("Text:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelTextBoxes.add(jLabel_textBoxText, gridBagConstraints);

        textField_textBoxText.setText("textBox' text");
        textField_textBoxText.setSynchLabel(jLabel_textBoxText);
        textField_textBoxText.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_textBoxTextActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTextBoxes.add(textField_textBoxText, gridBagConstraints);

        jPanel_textBoxHorizVerts.setLayout(new java.awt.GridBagLayout());

        toggle_textHorizLeft.setIcon(resourceFactory.getTextAlignLeft16());
        toggle_textHorizLeft.setRadioToggle(true);
        toggle_textHorizLeft.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textHorizLeftActionPerformed(evt);
            }
        });
        jPanel_textBoxHorizVerts.add(toggle_textHorizLeft, new java.awt.GridBagConstraints());

        toggle_textHorizCenter.setIcon(resourceFactory.getTextAlignCenter16());
        toggle_textHorizCenter.setRadioToggle(true);
        toggle_textHorizCenter.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textHorizCenterActionPerformed(evt);
            }
        });
        jPanel_textBoxHorizVerts.add(toggle_textHorizCenter, new java.awt.GridBagConstraints());

        toggle_textHorizRight.setIcon(resourceFactory.getTextAlignRight16());
        toggle_textHorizRight.setRadioToggle(true);
        toggle_textHorizRight.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textHorizRightActionPerformed(evt);
            }
        });
        jPanel_textBoxHorizVerts.add(toggle_textHorizRight, new java.awt.GridBagConstraints());

        toggle_textVertTop.setIcon(resourceFactory.getTextVertTop16());
        toggle_textVertTop.setRadioToggle(true);
        toggle_textVertTop.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textVertTopActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 12, 0, 0);
        jPanel_textBoxHorizVerts.add(toggle_textVertTop, gridBagConstraints);

        toggle_textVertCenter.setIcon(resourceFactory.getTextVertCenter16());
        toggle_textVertCenter.setRadioToggle(true);
        toggle_textVertCenter.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textVertCenterActionPerformed(evt);
            }
        });
        jPanel_textBoxHorizVerts.add(toggle_textVertCenter, new java.awt.GridBagConstraints());

        toggle_textVertBottom.setIcon(resourceFactory.getTextVertBottom16());
        toggle_textVertBottom.setRadioToggle(true);
        toggle_textVertBottom.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textVertBottomActionPerformed(evt);
            }
        });
        jPanel_textBoxHorizVerts.add(toggle_textVertBottom, new java.awt.GridBagConstraints());

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelTextBoxes.add(jPanel_textBoxHorizVerts, gridBagConstraints);

        jPanel2.setLayout(new java.awt.GridBagLayout());

        color_textBoxText.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                color_textBoxTextActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel2.add(color_textBoxText, gridBagConstraints);

        comboBox_textSize.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_textSizeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel2.add(comboBox_textSize, gridBagConstraints);

        toggle_textBold.setIcon(resourceFactory.getTextBold16());
        toggle_textBold.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textBoldActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 0);
        jPanel2.add(toggle_textBold, gridBagConstraints);

        toggle_textItalic.setIcon(resourceFactory.getTextItalic16());
        toggle_textItalic.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                toggle_textItalicActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 4;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(3, 0, 3, 3);
        jPanel2.add(toggle_textItalic, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        jPanelTextBoxes.add(jPanel2, gridBagConstraints);

        jPanel5.setBorder(javax.swing.BorderFactory.createMatteBorder(1, 0, 0, 0, new java.awt.Color(102, 102, 102)));
        jPanel5.setLayout(new java.awt.GridBagLayout());

        label_fillColor.setText("Fill:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel5.add(label_fillColor, gridBagConstraints);

        color_textBoxFill.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                color_textBoxFillActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel5.add(color_textBoxFill, gridBagConstraints);

        checkBox_textBoxShadow.setText("Drop shadow");
        checkBox_textBoxShadow.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_textBoxShadowActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel5.add(checkBox_textBoxShadow, gridBagConstraints);

        jLabel_textBoxBorder.setText("Border:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel5.add(jLabel_textBoxBorder, gridBagConstraints);

        color_textBoxBorder.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                color_textBoxBorderActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel5.add(color_textBoxBorder, gridBagConstraints);

        comboBox_textBoxShape.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_textBoxShapeActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel5.add(comboBox_textBoxShape, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        jPanelTextBoxes.add(jPanel5, gridBagConstraints);

        jPanel7.setLayout(new java.awt.GridBagLayout());

        jLabel1.setIcon(resourceFactory.getLock16());
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel7.add(jLabel1, gridBagConstraints);

        checkBox_textBoxLocked.setText("Lock text box to the background.");
        checkBox_textBoxLocked.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                checkBox_textBoxLockedActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel7.add(checkBox_textBoxLocked, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        jPanelTextBoxes.add(jPanel7, gridBagConstraints);

        jPanel8.setLayout(new java.awt.GridBagLayout());

        jLabel2.setText("Image:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanel8.add(jLabel2, gridBagConstraints);

        button_textBoxImageSelect.setText("Select...");
        button_textBoxImageSelect.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_textBoxImageSelectActionPerformed(evt);
            }
        });
        jPanel8.add(button_textBoxImageSelect, new java.awt.GridBagConstraints());

        button_textBoxImageClear.setText("  Clear  ");
        button_textBoxImageClear.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                button_textBoxImageClearActionPerformed(evt);
            }
        });
        jPanel8.add(button_textBoxImageClear, new java.awt.GridBagConstraints());

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 0.1;
        jPanelTextBoxes.add(jPanel8, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 9;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(jPanelTextBoxes, gridBagConstraints);

        jPanelColorClasses.setBorder(javax.swing.BorderFactory.createTitledBorder("Color classes"));
        jPanelColorClasses.setLayout(new java.awt.GridBagLayout());

        jLabel_colorClassDef.setText("Definition:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelColorClasses.add(jLabel_colorClassDef, gridBagConstraints);

        textField_colorClassDef.setText("color class definition");
        textField_colorClassDef.setSynchLabel(jLabel_colorClassDef);
        textField_colorClassDef.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                textField_colorClassDefActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelColorClasses.add(textField_colorClassDef, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 10;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(jPanelColorClasses, gridBagConstraints);

        jPanelColorVars.setBorder(javax.swing.BorderFactory.createTitledBorder("Color variables"));
        jPanelColorVars.setLayout(new java.awt.GridBagLayout());

        jLabel_colorVarDomain.setText("Color class:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelColorVars.add(jLabel_colorVarDomain, gridBagConstraints);

        comboBox_colorVarDomain.setEditable(true);
        comboBox_colorVarDomain.setSynchLabel(jLabel_colorVarDomain);
        comboBox_colorVarDomain.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_colorVarDomainActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelColorVars.add(comboBox_colorVarDomain, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 11;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(jPanelColorVars, gridBagConstraints);

        jPanelColorDomain.setBorder(javax.swing.BorderFactory.createTitledBorder("Color domain"));
        jPanelColorDomain.setLayout(new java.awt.GridBagLayout());

        comboBox_colorDomain.setEditable(true);
        comboBox_colorDomain.setSynchLabel(jLabel_colorDomain);
        comboBox_colorDomain.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                comboBox_colorDomainActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        gridBagConstraints.insets = new java.awt.Insets(3, 3, 3, 3);
        jPanelColorDomain.add(comboBox_colorDomain, gridBagConstraints);

        jLabel_colorDomain.setText("Color domain:");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_END;
        gridBagConstraints.insets = new java.awt.Insets(0, 6, 0, 3);
        jPanelColorDomain.add(jLabel_colorDomain, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.weightx = 0.1;
        add(jPanelColorDomain, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents

            
    private void executeComponentAction(String descr, final java.awt.event.ActionEvent evt) {
//        System.out.println("entering executeComponentAction... "+evt);
        final Component sourceComponent = (Component)evt.getSource();
        if (sourceComponent == null)
            return;
        if (!editor.mainInterface.isGUIvalid())
            return; // Inside GUI update functions....
//        if (evt.getModifiers() == 0)
//            return; // mouse button expected
        
        
        // Verify that the value is valid
        final Object newValue;
        final EditableValue editable;
        if (sourceComponent instanceof CheckBoxForEditable) {
            CheckBoxForEditable ch = (CheckBoxForEditable)sourceComponent;
            newValue = ch.isSelected();
            editable = ch.getEditableProxy();
        }
        else if (sourceComponent instanceof ComboBoxForEditable) {
            ComboBoxForEditable cb = (ComboBoxForEditable)sourceComponent;
            if (cb.isUpdating())
                return; // false action
//            if (!cb.isValueValid()) {
//                mainInterface.invalidateGUI(); // reset the panel
//                return;
//            }
            newValue = cb.getSelectedItem();
            editable = cb.getEditableProxy();
        }
        else if (sourceComponent instanceof TextFieldForEditable) {
            TextFieldForEditable tf = (TextFieldForEditable)sourceComponent;
//            if (!tf.isValueValid()) {
//                mainInterface.invalidateGUI(); // reset the panel
//                return;
//            }
            newValue = tf.getText();
            editable = tf.getEditableProxy();
        }
        else if (sourceComponent instanceof ToggleButtonForEditable) {
            ToggleButtonForEditable tb = (ToggleButtonForEditable)sourceComponent;
            newValue = tb.isRadioToggle() ? Boolean.TRUE : tb.isSelected();
            editable = tb.getEditableProxy();
        }
        else if (sourceComponent instanceof ColorPickerForEditable) {
            ColorPickerForEditable cp = (ColorPickerForEditable)sourceComponent;
            newValue = cp.getColor();
            editable = cp.getEditableProxy();
        }
        else if (sourceComponent instanceof ButtonForEditable) {
            ButtonForEditable b = (ButtonForEditable)sourceComponent;
            editable = b.getEditableProxy();
            newValue = b.getRetrievedValue();
        }
        else {
            throw new IllegalStateException("Unknown source component.");
        }
        
        if (editable != null) {
            Object currValue = editable.getValue();
            if (currValue != null && currValue.equals(newValue))
                return; // No changes
            mainInterface.executeUndoableCommand(descr, (ProjectData proj, ProjectPage page) -> {
                editable.setValue(proj, (NetPage)page, newValue);
            });
        }
        else {
            System.out.println("editable == null: should not happen");
        }
    }
        
    private void textField_UniqueNameActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_UniqueNameActionPerformed
        if (!textField_UniqueName.isValueValid())
            return;
        mainInterface.executeUndoableCommand("change unique ID", (ProjectData proj, ProjectPage page) -> {
            NetPage currPage1 = (NetPage)page;
            for (Node n : currPage1.nodes) {
                if (n.isSelected()) {
                    n.getUniqueNameEditable().setValue(proj, page, textField_UniqueName.getText());
                    return;
                }
            }
        });
    }//GEN-LAST:event_textField_UniqueNameActionPerformed

    private void textField_NodeWidthActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_NodeWidthActionPerformed
        executeComponentAction("change width.", evt);
    }//GEN-LAST:event_textField_NodeWidthActionPerformed

    private void textField_NodeHeightActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_NodeHeightActionPerformed
        executeComponentAction("change height.", evt);
    }//GEN-LAST:event_textField_NodeHeightActionPerformed

    private void comboBox_NodeRotationActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_NodeRotationActionPerformed
        executeComponentAction("change rotation.", evt);
    }//GEN-LAST:event_comboBox_NodeRotationActionPerformed
    
    private void textField_initMarkingActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_initMarkingActionPerformed
        executeComponentAction("change initial marking.", evt);
    }//GEN-LAST:event_textField_initMarkingActionPerformed

    private void textField_ArcMultiplicityActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_ArcMultiplicityActionPerformed
        executeComponentAction("change arc multiplicity.", evt);
    }//GEN-LAST:event_textField_ArcMultiplicityActionPerformed

    private void checkBox_BrokenEdgeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_BrokenEdgeActionPerformed
        executeComponentAction("change broken edge flag.", evt);
    }//GEN-LAST:event_checkBox_BrokenEdgeActionPerformed

    private void checkBox_InitialLocationActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_InitialLocationActionPerformed
        executeComponentAction("change initial location.", evt);
    }//GEN-LAST:event_checkBox_InitialLocationActionPerformed

    private void comboBox_FinalLocationActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_FinalLocationActionPerformed
        executeComponentAction("change final type.", evt);
    }//GEN-LAST:event_comboBox_FinalLocationActionPerformed

    private void textField_StatePropExprActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_StatePropExprActionPerformed
        executeComponentAction("change state proposition.", evt);
    }//GEN-LAST:event_textField_StatePropExprActionPerformed

    private void checkBox_BoundaryEdgeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_BoundaryEdgeActionPerformed
        executeComponentAction("change DTA edge type.", evt);
    }//GEN-LAST:event_checkBox_BoundaryEdgeActionPerformed

    private void checkBox_ResetClockEdgeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_ResetClockEdgeActionPerformed
        executeComponentAction("change reset clock flag.", evt);
    }//GEN-LAST:event_checkBox_ResetClockEdgeActionPerformed

    private void textField_ActSetActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_ActSetActionPerformed
        executeComponentAction("change action set.", evt);
    }//GEN-LAST:event_textField_ActSetActionPerformed

    private void textField_ClockGuardActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_ClockGuardActionPerformed
        executeComponentAction("change clock guard.", evt);
    }//GEN-LAST:event_textField_ClockGuardActionPerformed

    private void comboBox_TransitionTypeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_TransitionTypeActionPerformed
        executeComponentAction("change transition type.", evt);
    }//GEN-LAST:event_comboBox_TransitionTypeActionPerformed

    private void textField_TransitionPriorityActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_TransitionPriorityActionPerformed
        executeComponentAction("change transition priority.", evt);
    }//GEN-LAST:event_textField_TransitionPriorityActionPerformed

    private void textField_TransitionDelayActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_TransitionDelayActionPerformed
        executeComponentAction("change transition delay.", evt);
    }//GEN-LAST:event_textField_TransitionDelayActionPerformed

    private void comboBox_EdgeAttachmentActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_EdgeAttachmentActionPerformed
        executeComponentAction("change edge attachment model.", evt);
    }//GEN-LAST:event_comboBox_EdgeAttachmentActionPerformed

    private void textField_alternateNameActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_alternateNameActionPerformed
        executeComponentAction("change visible ID.", evt);
    }//GEN-LAST:event_textField_alternateNameActionPerformed

    private void comboBox_ConstTypeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_ConstTypeActionPerformed
        executeComponentAction("change type of constant.", evt);
    }//GEN-LAST:event_comboBox_ConstTypeActionPerformed

    private void textField_ConstValueActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_ConstValueActionPerformed
        executeComponentAction("change value of constant.", evt);
    }//GEN-LAST:event_textField_ConstValueActionPerformed

    private void textField_TransitionWeightActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_TransitionWeightActionPerformed
        executeComponentAction("change transition weight.", evt);
    }//GEN-LAST:event_textField_TransitionWeightActionPerformed

    private void textField_TransitionNumServersActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_TransitionNumServersActionPerformed
        executeComponentAction("change transition's number of servers.", evt);
    }//GEN-LAST:event_textField_TransitionNumServersActionPerformed

    private void comboBox_TemplateVarTypeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_TemplateVarTypeActionPerformed
        executeComponentAction("change template variable type.", evt);
    }//GEN-LAST:event_comboBox_TemplateVarTypeActionPerformed

    private void comboBox_altNameFnActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_altNameFnActionPerformed
        executeComponentAction("change visible name function.", evt);
    }//GEN-LAST:event_comboBox_altNameFnActionPerformed

    private void textField_superPosActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_superPosActionPerformed
        executeComponentAction("change superposition tags.", evt);
    }//GEN-LAST:event_textField_superPosActionPerformed

    private void textField_textBoxTextActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_textBoxTextActionPerformed
        executeComponentAction("change text label of the box.", evt);
    }//GEN-LAST:event_textField_textBoxTextActionPerformed

    private void toggle_textHorizLeftActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textHorizLeftActionPerformed
        executeComponentAction("left horizontal alignment", evt);
    }//GEN-LAST:event_toggle_textHorizLeftActionPerformed

    private void toggle_textHorizCenterActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textHorizCenterActionPerformed
        executeComponentAction("center horizontal alignment", evt);
    }//GEN-LAST:event_toggle_textHorizCenterActionPerformed

    private void toggle_textHorizRightActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textHorizRightActionPerformed
        executeComponentAction("right horizontal alignment", evt);
    }//GEN-LAST:event_toggle_textHorizRightActionPerformed

    private void toggle_textVertTopActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textVertTopActionPerformed
        executeComponentAction("top vertical alignment", evt);
    }//GEN-LAST:event_toggle_textVertTopActionPerformed

    private void toggle_textVertCenterActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textVertCenterActionPerformed
        executeComponentAction("center vertical alignment", evt);
    }//GEN-LAST:event_toggle_textVertCenterActionPerformed

    private void toggle_textVertBottomActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textVertBottomActionPerformed
        executeComponentAction("bottom vertical alignment", evt);
    }//GEN-LAST:event_toggle_textVertBottomActionPerformed

    private void color_textBoxTextActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_color_textBoxTextActionPerformed
        executeComponentAction("change text color", evt);
    }//GEN-LAST:event_color_textBoxTextActionPerformed

    private void comboBox_textSizeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_textSizeActionPerformed
        executeComponentAction("change text size", evt);
    }//GEN-LAST:event_comboBox_textSizeActionPerformed

    private void color_textBoxFillActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_color_textBoxFillActionPerformed
        executeComponentAction("change text box fill color", evt);
    }//GEN-LAST:event_color_textBoxFillActionPerformed

    private void color_textBoxBorderActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_color_textBoxBorderActionPerformed
        executeComponentAction("change text box border color", evt);
    }//GEN-LAST:event_color_textBoxBorderActionPerformed

    private void checkBox_textBoxShadowActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_textBoxShadowActionPerformed
        executeComponentAction("change text box shadow", evt);
    }//GEN-LAST:event_checkBox_textBoxShadowActionPerformed

    private void comboBox_textBoxShapeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_textBoxShapeActionPerformed
        executeComponentAction("change text box shape type", evt);
    }//GEN-LAST:event_comboBox_textBoxShapeActionPerformed

    private void checkBox_textBoxLockedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_checkBox_textBoxLockedActionPerformed
        executeComponentAction("change text box locking", evt);
    }//GEN-LAST:event_checkBox_textBoxLockedActionPerformed

    private void toggle_textBoldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textBoldActionPerformed
        executeComponentAction("change text bold flag", evt);
    }//GEN-LAST:event_toggle_textBoldActionPerformed

    private void toggle_textItalicActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_toggle_textItalicActionPerformed
        executeComponentAction("change text italic flag", evt);
    }//GEN-LAST:event_toggle_textItalicActionPerformed

    private static final FileFilter imgFileFilter = new FileNameExtensionFilter("Known image files", 
            new String[]{"jpg", "jpeg", "gif", "png", "bmp", "tif", "tiff"});

    
    private void button_textBoxImageSelectActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_textBoxImageSelectActionPerformed
        try {
            JFileChooser fileChooser = new JFileChooser();
            fileChooser.addChoosableFileFilter(imgFileFilter);
            fileChooser.setFileFilter(imgFileFilter);
            String curDir = Util.getPreferences().get("image-file-dir", System.getProperty("user.home"));
            fileChooser.setCurrentDirectory(curDir!=null ? new File(curDir) : null);
            fileChooser.setDialogTitle("Select an image for the TextBox");
            if (fileChooser.showOpenDialog(mainInterface.getWindowFrame()) != JFileChooser.APPROVE_OPTION) 
                return;
            curDir = fileChooser.getCurrentDirectory().getAbsolutePath();
            Util.getPreferences().put("image-file-dir", curDir);

            // Load the image and prepare the resource
            File file = fileChooser.getSelectedFile();
            byte[] fileData = new byte[(int) file.length()];
            DataInputStream dis = new DataInputStream(new FileInputStream(file));
            dis.readFully(fileData);
            dis.close();
            ProjectImageResource img = new ProjectImageResource(fileData);
            
            button_textBoxImageSelect.setRetrievedValue(img);
            executeComponentAction("set new textbox image", evt);
        }
        catch (IOException e) { Main.logException(e, true); }
    }//GEN-LAST:event_button_textBoxImageSelectActionPerformed

    private void button_textBoxImageClearActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_button_textBoxImageClearActionPerformed
        executeComponentAction("clear textbox image", evt);
    }//GEN-LAST:event_button_textBoxImageClearActionPerformed

    private void comboBox_placeTypeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_placeTypeActionPerformed
        executeComponentAction("change place type", evt);
    }//GEN-LAST:event_comboBox_placeTypeActionPerformed

    private void comboBox_colorDomainActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_colorDomainActionPerformed
        executeComponentAction("change color domain of place", evt);
    }//GEN-LAST:event_comboBox_colorDomainActionPerformed

    private void textField_colorClassDefActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_colorClassDefActionPerformed
        executeComponentAction("change color color class definition", evt);
    }//GEN-LAST:event_textField_colorClassDefActionPerformed

    private void comboBox_colorVarDomainActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_colorVarDomainActionPerformed
        executeComponentAction("change color class of variable", evt);
    }//GEN-LAST:event_comboBox_colorVarDomainActionPerformed

    private void textField_TransitionGuardActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_TransitionGuardActionPerformed
        executeComponentAction("change transition guard.", evt);
    }//GEN-LAST:event_textField_TransitionGuardActionPerformed

    private void comboBox_placePartitionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_comboBox_placePartitionActionPerformed
        executeComponentAction("change place partition/component label.", evt);
    }//GEN-LAST:event_comboBox_placePartitionActionPerformed

    private void buttonConstantsToTemplatesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonConstantsToTemplatesActionPerformed
        mainInterface.executeUndoableCommand("constants to parameters.", (ProjectData proj, ProjectPage page) -> {
            NetPage np = (NetPage)page;
            for (int i=0; i<np.nodes.size(); i++) {
                if (np.nodes.get(i).isSelected() && np.nodes.get(i) instanceof ConstantID) {
                    TemplateVariable tvar = new TemplateVariable((ConstantID)np.nodes.get(i));
                    tvar.setSelected(true);
                    np.nodes.set(i, tvar);
                }
            }
        });
    }//GEN-LAST:event_buttonConstantsToTemplatesActionPerformed

    private void buttonTemplatesToConstantsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonTemplatesToConstantsActionPerformed
        mainInterface.executeUndoableCommand("parameters to constants.", (ProjectData proj, ProjectPage page) -> {
            NetPage np = (NetPage)page;
            for (int i=0; i<np.nodes.size(); i++) {
                if (np.nodes.get(i).isSelected() && np.nodes.get(i) instanceof TemplateVariable) {
                    ConstantID con = new ConstantID((TemplateVariable)np.nodes.get(i));
                    con.setSelected(true);
                    np.nodes.set(i, con);
                }
            }
        });
    }//GEN-LAST:event_buttonTemplatesToConstantsActionPerformed

    private void textField_varFlowExprActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_textField_varFlowExprActionPerformed
        executeComponentAction("change flow of variables.", evt);
    }//GEN-LAST:event_textField_varFlowExprActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton buttonConstantsToTemplates;
    private javax.swing.JButton buttonTemplatesToConstants;
    private editor.gui.net.ButtonForEditable button_textBoxImageClear;
    private editor.gui.net.ButtonForEditable button_textBoxImageSelect;
    private editor.gui.net.CheckBoxForEditable checkBox_BoundaryEdge;
    private editor.gui.net.CheckBoxForEditable checkBox_BrokenEdge;
    private editor.gui.net.CheckBoxForEditable checkBox_InitialLocation;
    private editor.gui.net.CheckBoxForEditable checkBox_ResetClockEdge;
    private editor.gui.net.CheckBoxForEditable checkBox_textBoxLocked;
    private editor.gui.net.CheckBoxForEditable checkBox_textBoxShadow;
    private editor.gui.net.ColorPickerForEditable color_textBoxBorder;
    private editor.gui.net.ColorPickerForEditable color_textBoxFill;
    private editor.gui.net.ColorPickerForEditable color_textBoxText;
    private editor.gui.net.ComboBoxForEditable<ConstantID.ConstType> comboBox_ConstType;
    private editor.gui.net.ComboBoxForEditable<EdgeAttachmentModel> comboBox_EdgeAttachment;
    private editor.gui.net.ComboBoxForEditable<DtaLocation.FinalType> comboBox_FinalLocation;
    private editor.gui.net.ComboBoxForEditable comboBox_NodeRotation;
    private editor.gui.net.ComboBoxForEditable<TemplateVariable.Type> comboBox_TemplateVarType;
    private editor.gui.net.ComboBoxForEditable<Transition.Type> comboBox_TransitionType;
    private editor.gui.net.ComboBoxForEditable<AlternateNameFunction> comboBox_altNameFn;
    private editor.gui.net.ComboBoxForEditable<String> comboBox_colorDomain;
    private editor.gui.net.ComboBoxForEditable<String> comboBox_colorVarDomain;
    private editor.gui.net.ComboBoxForEditable<String> comboBox_placePartition;
    private editor.gui.net.ComboBoxForEditable<TokenType> comboBox_placeType;
    private editor.gui.net.ComboBoxForEditable<Node.ShapeType> comboBox_textBoxShape;
    private editor.gui.net.ComboBoxForEditable<TextBox.TextSize> comboBox_textSize;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabelTemplateVarType;
    private javax.swing.JLabel jLabel_ActSet;
    private javax.swing.JLabel jLabel_ArcMultiplicity;
    private javax.swing.JLabel jLabel_ClockGuard;
    private javax.swing.JLabel jLabel_ConstValue;
    private javax.swing.JLabel jLabel_EdgeAttachment;
    private javax.swing.JLabel jLabel_FinalDtaLocation;
    private javax.swing.JLabel jLabel_ID;
    private javax.swing.JLabel jLabel_InitMarking;
    private javax.swing.JLabel jLabel_NodeHeight;
    private javax.swing.JLabel jLabel_NodeRotation;
    private javax.swing.JLabel jLabel_NodeWidth;
    private javax.swing.JLabel jLabel_StatePropExpr;
    private javax.swing.JLabel jLabel_TransitionDelay;
    private javax.swing.JLabel jLabel_TransitionGuard;
    private javax.swing.JLabel jLabel_TransitionNumServers;
    private javax.swing.JLabel jLabel_TransitionPriority;
    private javax.swing.JLabel jLabel_TransitionType;
    private javax.swing.JLabel jLabel_TransitionWeight;
    private javax.swing.JLabel jLabel_altName;
    private javax.swing.JLabel jLabel_altNameFn;
    private javax.swing.JLabel jLabel_colorClassDef;
    private javax.swing.JLabel jLabel_colorDomain;
    private javax.swing.JLabel jLabel_colorVarDomain;
    private javax.swing.JLabel jLabel_placePartition;
    private javax.swing.JLabel jLabel_placeType;
    private javax.swing.JLabel jLabel_superPos;
    private javax.swing.JLabel jLabel_textBoxBorder;
    private javax.swing.JLabel jLabel_textBoxText;
    private javax.swing.JLabel jLabel_varFlowExpr;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanelColorClasses;
    private javax.swing.JPanel jPanelColorDomain;
    private javax.swing.JPanel jPanelColorVars;
    private javax.swing.JPanel jPanelConstants;
    private javax.swing.JPanel jPanelDtaEdgeProperty;
    private javax.swing.JPanel jPanelDtaLocationProperty;
    private javax.swing.JPanel jPanelEdgeProperties;
    private javax.swing.JPanel jPanelGspnArcProperties;
    private javax.swing.JPanel jPanelNodeProperties;
    private javax.swing.JPanel jPanelPlaceProperties;
    private javax.swing.JPanel jPanelTemplateVariables;
    private javax.swing.JPanel jPanelTextBoxes;
    private javax.swing.JPanel jPanelTransitionProperties;
    private javax.swing.JPanel jPanel_FillVert;
    private javax.swing.JPanel jPanel_textBoxHorizVerts;
    private javax.swing.JLabel label_fillColor;
    private editor.gui.ResourceFactory resourceFactory;
    private editor.gui.net.TextFieldForEditable textField_ActSet;
    private editor.gui.net.TextFieldForEditable textField_ArcMultiplicity;
    private editor.gui.net.TextFieldForEditable textField_ClockGuard;
    private editor.gui.net.TextFieldForEditable textField_ConstValue;
    private editor.gui.net.TextFieldForEditable textField_NodeHeight;
    private editor.gui.net.TextFieldForEditable textField_NodeWidth;
    private editor.gui.net.TextFieldForEditable textField_StatePropExpr;
    private editor.gui.net.TextFieldForEditable textField_TransitionDelay;
    private editor.gui.net.TextFieldForEditable textField_TransitionGuard;
    private editor.gui.net.TextFieldForEditable textField_TransitionNumServers;
    private editor.gui.net.TextFieldForEditable textField_TransitionPriority;
    private editor.gui.net.TextFieldForEditable textField_TransitionWeight;
    private editor.gui.net.TextFieldForEditable textField_UniqueName;
    private editor.gui.net.TextFieldForEditable textField_alternateName;
    private editor.gui.net.TextFieldForEditable textField_colorClassDef;
    private editor.gui.net.TextFieldForEditable textField_initMarking;
    private editor.gui.net.TextFieldForEditable textField_superPos;
    private editor.gui.net.TextFieldForEditable textField_textBoxText;
    private editor.gui.net.TextFieldForEditable textField_varFlowExpr;
    private editor.gui.net.ToggleButtonForEditable toggle_textBold;
    private editor.gui.net.ToggleButtonForEditable toggle_textHorizCenter;
    private editor.gui.net.ToggleButtonForEditable toggle_textHorizLeft;
    private editor.gui.net.ToggleButtonForEditable toggle_textHorizRight;
    private editor.gui.net.ToggleButtonForEditable toggle_textItalic;
    private editor.gui.net.ToggleButtonForEditable toggle_textVertBottom;
    private editor.gui.net.ToggleButtonForEditable toggle_textVertCenter;
    private editor.gui.net.ToggleButtonForEditable toggle_textVertTop;
    // End of variables declaration//GEN-END:variables

}
