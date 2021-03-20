/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import static editor.domain.io.XmlExchangeUtils.bindXMLAttrib;
import editor.domain.io.XmlExchangeable;
import editor.gui.NewProjectDialog;
import java.io.Serializable;
import org.w3c.dom.Element;

/** Determines how net objects are displayed.
 *
 * @author elvio
 */
public class ViewProfile implements Serializable, XmlExchangeable {
    // View rate decorations (rates, delays, weights, ...)
    public boolean viewRatesDelays = true;
    // View transition guards
    public boolean viewGuards = true;
    // View priorities of immediate transitions
    public boolean viewPriorities = true;
    // View names of places and transitions
    public boolean viewNames = true;
    // View superposition tags
    public boolean viewSuperPosTags = true;
    
    // commands
    
    // Show timed/stochastic elements
    public boolean showTimedCommands = true;
    // Show colors/SWN commands
    public boolean showColorCommands = true;
    // Show hybrid/fluid commands
    public boolean showFluidCommands = true;
    
    //--------------------------------------------------
    // Graphical view informations
    //--------------------------------------------------
    
    // Zoom level of the page window
    public int zoom = 100;
    
    // The single selected object, available only during a paintNet cycle
    public transient Selectable singleSelObject = null;
//    public transient boolean hasSingleSelObject = false;
    
    // Combine two profiles into a new one
    public ViewProfile combineWith(ViewProfile vp) {
        ViewProfile newProfile = new ViewProfile();
        
        newProfile.zoom = this.zoom;
        newProfile.viewRatesDelays   = this.viewRatesDelays   || vp.viewRatesDelays;
        newProfile.viewGuards        = this.viewGuards        || vp.viewGuards;
        newProfile.viewPriorities    = this.viewPriorities    || vp.viewPriorities;
        newProfile.viewNames         = this.viewNames         || vp.viewNames;
        newProfile.viewSuperPosTags  = this.viewSuperPosTags  || vp.viewSuperPosTags;
        newProfile.showTimedCommands = this.showTimedCommands || vp.showTimedCommands;
        newProfile.showColorCommands = this.showColorCommands || vp.showColorCommands;
        newProfile.showFluidCommands = this.showFluidCommands || vp.showFluidCommands;
        
        return newProfile;
    }
    
    public void copyFrom(ViewProfile vp) {
        this.zoom              = vp.zoom;
        this.viewRatesDelays   = vp.viewRatesDelays;
        this.viewGuards        = vp.viewGuards;
        this.viewPriorities    = vp.viewPriorities;
        this.viewNames         = vp.viewNames;
        this.viewSuperPosTags  = vp.viewSuperPosTags;
        this.showTimedCommands = vp.showTimedCommands;
        this.showColorCommands = vp.showColorCommands;
        this.showFluidCommands = vp.showFluidCommands;
    }

    
    // Profile for type of petri nets
    public void setProfileForNetType(NewProjectDialog.PetriNetType type) {
        // reset profile
        viewRatesDelays = viewGuards = viewPriorities = viewNames = viewSuperPosTags = true;
        showColorCommands = showFluidCommands = showTimedCommands = true;
        
        switch (type) {
            case PT:
                showColorCommands = false;
                showFluidCommands = false;
                showTimedCommands = false;
                viewRatesDelays = false;
                break;
                
            case CPN:
                showFluidCommands = false;
                showTimedCommands = false;
                viewRatesDelays = false;
                break;
                
            case SWN:
                showFluidCommands = false;
                break;
                
            case GSPN:
                showFluidCommands = false;
                showColorCommands = false;
                break;
                
            case FullPN:
                break;
                
            default:
                throw new UnsupportedOperationException();
        }
    }
    
    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        bindXMLAttrib(this, el, exDir, "view-rates", "viewRatesDelays", true);
        bindXMLAttrib(this, el, exDir, "view-guards", "viewGuards", true);
        bindXMLAttrib(this, el, exDir, "view-priorities", "viewPriorities", true);
        bindXMLAttrib(this, el, exDir, "view-names", "viewNames", true);
        bindXMLAttrib(this, el, exDir, "view-superpos-tags", "viewSuperPosTags", true);
        bindXMLAttrib(this, el, exDir, "show-timed-cmd", "showTimedCommands", true);
        bindXMLAttrib(this, el, exDir, "show-color-cmd", "showColorCommands", true);
        bindXMLAttrib(this, el, exDir, "show-fluid-cmd", "showFluidCommands", true);
        bindXMLAttrib(this, el, exDir, "zoom", "zoom", 100);
    }
}
