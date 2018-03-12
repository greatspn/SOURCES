/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.elements;

import editor.Main;
import editor.domain.NetPage;
import editor.domain.Node;
import editor.domain.ProjectData;
import editor.domain.ProjectFile;
import editor.domain.ProjectPage;
import editor.gui.AbstractPageEditor;
import editor.gui.RapidMeasureCmd;
import editor.gui.ResourceFactory;
import editor.gui.net.NetPlayPanel;
import editor.gui.SharedResourceProvider;
import java.awt.datatransfer.DataFlavor;
import java.io.Serializable;
import java.util.Set;
import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

/**
 *
 * @author elvio
 */
public class DtaPage extends NetPage implements Serializable {

    public DtaPage() { }
    
    @Override
    protected boolean checkPageCorrectness(boolean isNewOrModified, ProjectData proj, 
                                           Set<ProjectPage> changedPages, 
                                           ProjectPage invokerPage) 
    {
        if (!super.checkPageCorrectness(isNewOrModified, proj, changedPages, invokerPage))
            return false;
        
        // Extra DTA rules that has to be checked.
        return true;
    }
    
    @Override public Icon getPageIcon() {
        return ResourceFactory.getInstance().getPageDta16();
    }
    @Override public String getPageTypeName() {
        return "DTA";
    }
    
    private static final DataFlavor dataFlavour = new DataFlavor(DtaPage.class, "DTA Objects");
    @Override public DataFlavor getDataFlavour() { return dataFlavour; }
    
    @Override public boolean pageSupportsPlay() { return true; }
    
    @Override public PlayCommand canPlay(ProjectFile project) {
        if (!isPageCorrect())
            return PlayCommand.NO;
        int numGspn = 0;
        for (int p=0; p<project.getCurrent().getPageCount(); p++) {
            ProjectPage page = project.getCurrent().getPageAt(p);
            if (page instanceof GspnPage && ((GspnPage)page).isPageCorrect())
                numGspn++;
        }
        return (numGspn > 0) ? PlayCommand.DROPDOWN : PlayCommand.NO;
    }
    private class JModelCheckMenuItem extends JMenuItem {
        GspnPage gspn;

        public JModelCheckMenuItem(GspnPage gspn, String string, javax.swing.Action action) {
            super(action);
            setHideActionText(true);
            setText(string);
            setIcon(gspn.getPageIcon());
            this.gspn = gspn;
        }
        
    }
    @Override public JPopupMenu getPlayDropdownMenu(ProjectFile project, javax.swing.Action action) {
        JPopupMenu popup = new JPopupMenu();
        for (int p=0; p<project.getCurrent().getPageCount(); p++) {
            ProjectPage page = project.getCurrent().getPageAt(p);
            if (page instanceof GspnPage) {
                GspnPage gspn = (GspnPage)page;
                JMenuItem item = new JModelCheckMenuItem(gspn, "Model check "+gspn.getPageName(), action);
                item.setEnabled(gspn.isPageCorrect());
                popup.add(item);
            }
        }return popup;
    }
    private static NetPlayPanel playPanel;
    @Override public AbstractPageEditor getPlayWindow(SharedResourceProvider shActProv, JMenuItem menuItem) {
        if (playPanel == null)
            playPanel = new NetPlayPanel(shActProv);
        JModelCheckMenuItem item = (JModelCheckMenuItem)menuItem;
        playPanel.initializeFor(item.gspn, this);
        return playPanel;
    }
    
    @Override
    public boolean hasPlaceTransInv() {
        return false;
    }

    @Override
    public boolean pageSupportsUnfolding() {
        return false;
    }
    @Override public boolean pageSupportsAlgebraTool() { return false; }


    @Override
    public boolean canBeUnfolded() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean pageSupportsRG(RgType rgType) {
        return false;
    }

    @Override
    public boolean canBuildRG(RgType rgType) {
        throw new UnsupportedOperationException(); 
    }
    
    @Override
    public boolean hasRapidMesures() { return false; }
    @Override
    public boolean pageSupportsRapidMeasure(RapidMeasureCmd rmc) {
        throw new UnsupportedOperationException(""); 
    }
    @Override
    public boolean canDoRapidMeasure(RapidMeasureCmd rmc) {
        throw new UnsupportedOperationException(""); 
    }
   
    
    // Generate the DTA signature
    public DtaSignature createSignature() {
        DtaSignature sig = new DtaSignature(this, isPageCorrect());

        for (Node node : nodes) {
            if (node instanceof TemplateVariable) {
                TemplateVariable tvar = (TemplateVariable) node;
                if (tvar.getType() == TemplateVariable.Type.REAL) {
                    sig.clockVals.add(tvar.getUniqueName());
                } else if (tvar.getType() == TemplateVariable.Type.ACTION) {
                    sig.actNames.add(tvar.getUniqueName());
                } else if (tvar.getType() == TemplateVariable.Type.STATEPROP) {
                    sig.stateProps.add(tvar.getUniqueName());
                }
            }
        }
        
        return sig;
    }
}
