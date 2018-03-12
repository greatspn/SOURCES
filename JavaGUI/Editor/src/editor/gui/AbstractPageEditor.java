/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import editor.domain.PageErrorWarning;
import editor.domain.ProjectPage;
import editor.domain.ProjectFile;
import java.awt.event.ActionEvent;
import javax.swing.JComponent;

/**
 *
 * @author elvio
 */
public interface AbstractPageEditor {
    
    // Editor enabled for a specified project page in the mainWindow
    public void setEditorEnabledFor(ProjectFile pf, ProjectPage page, 
                                    MainWindowInterface mainInterface);
    
    // Ask if the panel is a modal panel. A modal panel is shown without having all
    // the usual editing commands, the main toolbar and the project tree.
    // Panels like the 'token game' panel are modal, while editor panels are not.
    public boolean isModalEditor();
    
    // Disabled, cleanup code
    public void setEditorDisabled();
 
    // Get the swing panel
    public JComponent getEditorComponent();
    
    // The component that should receive the focus
    public JComponent getFocusTarget();
    
    // The property panel, shown on the left of the main window
    public JComponent getPropertyPanel();
    
    // The additional toolbar
    public JComponent getToolbar();
    
    // Get the list of errors and warnings to be shown in the error table
    public PageErrorWarning[] getAllErrorsAndWarnings();
    
    // Called to update the GUI, by the main updateGUI() function
    public void updateGUI(ProjectFile pf, ProjectPage page);
    
    // The Editor uses the Zoom panel
    public boolean isZoomPanelUsed();    
    public int getPageZoom();    
    public void zoomChanged(int newZoomLevel);    
    
    // Shared actions
    public void updateEnablingOfSharedActions(SharedResourceProvider shResProv);
    public void sharedActionListener(SharedResourceProvider.ActionName shAction, ActionEvent event);
}
