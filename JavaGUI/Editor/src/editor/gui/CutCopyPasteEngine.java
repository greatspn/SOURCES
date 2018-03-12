/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import common.Action;
import editor.Main;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import java.awt.KeyboardFocusManager;
import java.awt.Toolkit;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.FlavorEvent;
import java.awt.datatransfer.FlavorListener;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.event.ActionEvent;
import java.io.IOException;
import javax.swing.ActionMap;
import javax.swing.JComponent;
import javax.swing.TransferHandler;
import static javax.swing.TransferHandler.MOVE;

/*
 * A class that tracks the focused component.  This is necessary
 * to delegate the menu cut/copy/paste commands to the right
 * component.  An instance of this class is listening and
 * when the user fires one of these commands, it calls the
 * appropriate action on the currently focused component.
 */
public class CutCopyPasteEngine implements /*PropertyChangeListener,*/ FlavorListener {
    
    public static interface CutCopyPasteActivation {
        public boolean canCut();
        public boolean canCopy();
        public boolean canPaste();
    }

//    private JComponent focusOwner = null;
    private final javax.swing.Action actionCut, actionCopy, actionPaste;
    
    // Last component that has been registered to be the cut/copy/paste target
    private JComponent ccpComponent = null;

    public CutCopyPasteEngine(javax.swing.Action actionCut, 
                              javax.swing.Action actionCopy, 
                              javax.swing.Action actionPaste) 
    {
        this.actionCut = actionCut;
        this.actionCopy = actionCopy;
        this.actionPaste = actionPaste;

        KeyboardFocusManager manager = KeyboardFocusManager.
                getCurrentKeyboardFocusManager();
//        manager.addPropertyChangeListener("permanentFocusOwner", this);
        Toolkit.getDefaultToolkit().getSystemClipboard().addFlavorListener(this);
        
//        updateCutCopyPasteEnabling(false);
    }

//    @Override
//    public void propertyChange(PropertyChangeEvent e) {
//        Object o = e.getNewValue();
//        System.out.println("focus changed: new="+e.getNewValue()+"\n            old="+e.getOldValue());
//        if (o instanceof JComponent) {
//            focusOwner = (JComponent) o;
//            // TODO: check that the focusOwner is inside the main window.
//        } else {
//            focusOwner = null;
//        }
//        updateCutCopyPasteEnabling(false);
//    }
    
//    public final void updateCutCopyPasteEnabling(boolean alwaysDisable) {
//        if (focusOwner == null || alwaysDisable) {
////            actionCut.setEnabled(false);
////            actionCopy.setEnabled(false);
////            actionPaste.setEnabled(false);
//        }
//        else {
//            System.out.println("updateCutCopyPasteEnabling: focusOwner="+focusOwner+" alwaysDisable="+alwaysDisable);
//            if (focusOwner instanceof CutCopyPasteActivation) {
//                CutCopyPasteActivation ccpa = (CutCopyPasteActivation)focusOwner;
//                actionCut.setEnabled(ccpa.canCut());
//                actionCopy.setEnabled(ccpa.canCopy());
//                actionPaste.setEnabled(ccpa.canPaste());
//            }
//            else if (focusOwner instanceof BaseCellEditor) {
//                // Do nothing
//            }
//            else {
//                actionCut.setEnabled(false);
//                actionCopy.setEnabled(false);
//                actionPaste.setEnabled(false);
//            }
//        }
//    }
    
    public final void updateCutCopyPasteActions(JComponent ccpComp) {
//        System.out.println("updateCutCopyPasteActions("+ccpComp+"}");
        ccpComponent = ccpComp;
        if (ccpComponent == null) {
            actionCut.setEnabled(false);
            actionCopy.setEnabled(false);
            actionPaste.setEnabled(false);
        }
        else {
            CutCopyPasteActivation ccpa = (CutCopyPasteActivation)ccpComp;
            actionCut.setEnabled(ccpa.canCut());
            actionCopy.setEnabled(ccpa.canCopy());
            actionPaste.setEnabled(ccpa.canPaste());
        }
    }

    @Override
    public void flavorsChanged(FlavorEvent fe) {
        // TODO: some workaround needed for MacOSX, since the flavorsChanged() 
        // is never called on that platform.
        System.out.println("flavorsChanged! "+fe);
        updateCutCopyPasteActions(ccpComponent);
//        updateCutCopyPasteEnabling(false);
    }
    
    public void forwardAction(ActionEvent e, String actionName) {
        if (ccpComponent == null) {
//            System.out.println("no focus owner");
            return;
        }
        System.out.println("Forward "+actionName+" to "+ccpComponent.getClass());
        
//        String action;// = (String) e.getActionCommand();
        javax.swing.Action a;
        if (actionName.equals("CUT"))
            a = TransferHandler.getCutAction();
        else if (actionName.equals("COPY"))
            a = TransferHandler.getCopyAction();
        else if (actionName.equals("PASTE"))
            a = TransferHandler.getPasteAction();
        else return;
        
        Object name = a.getValue(Action.NAME);
        javax.swing.Action a2 = ccpComponent.getActionMap().get(name);
        
//        javax.swing.Action a = focusOwner.getActionMap().get(action);
        if (a2 != null && a2.isEnabled()) {
//            System.out.println("sending to the focus owner...");
            a.actionPerformed(new ActionEvent(ccpComponent,
                                              ActionEvent.ACTION_PERFORMED,
                                              null));
        }
//        else {
////            System.out.println("the focus owner does not have an action with name "+name);
//            
//            System.out.println(""+focusOwner.getClass());    
//            Object[] keys = focusOwner.getActionMap().keys();
//            for (Object k : keys)
//                System.out.println("  "+k);
//        }
    }
    
    
    // Transferred object intermediate class
    public static class TransferableData implements Transferable {
        DataFlavor flavor;
        Object     payload;

        public TransferableData(DataFlavor flavor, Object payload) {
            this.flavor = flavor;
            this.payload = payload;
        }        

        @Override
        public DataFlavor[] getTransferDataFlavors() { 
            DataFlavor[] df = new DataFlavor[1];
            df[0] = flavor;
            return df;
        }

        @Override
        public boolean isDataFlavorSupported(DataFlavor df) {
            return df == flavor;
        }

        @Override
        public Object getTransferData(DataFlavor df) throws UnsupportedFlavorException, IOException {
            if (!isDataFlavorSupported(flavor)) {
                throw new UnsupportedFlavorException(flavor);
            }
            return payload;
        }
    }
    
    //-------------------------------------------------------------------------
    public interface CutCopyPasteActuator {
        ProjectPage getCurrPage();
        MainWindowInterface getMainInterface();
    }
    
    public static void setupCutCopyPaste(JComponent comp, final CutCopyPasteActuator ccpe) {
        comp.setTransferHandler(new TransferHandler() {
            @Override
            public boolean canImport(TransferHandler.TransferSupport ts) {
                System.out.println("canImport");
                if (ts.isDataFlavorSupported(ccpe.getCurrPage().getDataFlavour()))
                    return true;
                return false;
            }

            @Override
            public boolean importData(TransferHandler.TransferSupport ts) {
                System.out.println("importData");
                DataFlavor df = ccpe.getCurrPage().getDataFlavour();
                if (!ts.isDataFlavorSupported(df)) {
                    // FIX: This may happen on MacOSX, since the system could lose data
                    // synchronization on the clipboard.
                    ccpe.getMainInterface().invalidateGUI();
                    return false;
                }
                Transferable transferable = ts.getTransferable();
                try {
                    final Object data = transferable.getTransferData(df);
                    // Paste the data in a new Undo record
                    ccpe.getMainInterface().executeUndoableCommand("paste.", new UndoableCommand() {
                        @Override
                        public void Execute(ProjectData proj, ProjectPage page) throws Exception {
                            ccpe.getCurrPage().pasteData(data);
                        }
                    });
                }
                catch (UnsupportedFlavorException ufe) { Main.logException(ufe, true); }
                catch (IOException ioe) { Main.logException(ioe, true); }
                return false; // Data not copied
            }

            @Override
            protected Transferable createTransferable(JComponent jc) {
                System.out.println("createTransferable");
                Object data = ccpe.getCurrPage().copyData();
                Transferable transfer = new CutCopyPasteEngine.TransferableData(ccpe.getCurrPage().getDataFlavour(), data);

                // Re-test enabling of the cut/copy/paste items
                ccpe.getMainInterface().invalidateGUI();

                return transfer;
            }

            @Override
            public int getSourceActions(JComponent jc) {
                System.out.println("getSourceActions");
                return TransferHandler.COPY_OR_MOVE;
            }

            @Override
            protected void exportDone(JComponent c, final Transferable data, int action) {
                if (action == MOVE) {
                    // Delete the cut data
                    try {
                        ccpe.getMainInterface().executeUndoableCommand("cut.", new UndoableCommand() {
                        @Override
                        public void Execute(ProjectData proj, ProjectPage page) throws Exception {
                            Object eraseData = data.getTransferData(ccpe.getCurrPage().getDataFlavour());
                            ccpe.getCurrPage().eraseCutData(eraseData);
                        }
                    });
                    }
                    catch (Exception e) { }

                }
            }
        });

        // Register the Cut/Copy/Paste actions to be called by the engine
        ActionMap map = comp.getActionMap();
        map.put(TransferHandler.getCutAction().getValue(Action.NAME),
                TransferHandler.getCutAction());
        map.put(TransferHandler.getCopyAction().getValue(Action.NAME),
                TransferHandler.getCopyAction());
        map.put(TransferHandler.getPasteAction().getValue(Action.NAME),
                TransferHandler.getPasteAction());
    }
}