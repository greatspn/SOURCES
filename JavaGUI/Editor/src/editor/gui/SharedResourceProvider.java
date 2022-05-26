/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

/** Shared actions provided by tha main GUI that can be used by the editor panels.
 *
 * @author elvio
 */
public interface SharedResourceProvider {
    // Actions
    public enum ActionName {
        EDIT_SELECT_ALL, EDIT_DESELECT_ALL, EDIT_DELETE_SELECTED, EDIT_INVERT_SELECTION,
        PLAY_RESTART, PLAY_NEXT, PLAY_PREVIOUS, MAKE_EDITABLE_NET, RELAYOUT_OGDF, SHOW_NET_MATRICES,
        CHANGE_BINDINGS, COMPUTE_PLACE_SEMIFLOWS, COMPUTE_TRANS_SEMIFLOWS, COMPUTE_PLACE_BOUNDS_FROM_PINV,
        PLACE_SEMIFLOWS, PLACE_FLOWS, TRANS_SEMIFLOWS, TRANS_FLOWS, PLACE_BOUNDS_FROM_SEMIFLOWS,
        PLACE_BASIS, TRANS_BASIS, SIPHONS, TRAPS,
        EXPORT_GREATSPN_FORMAT, EXPORT_PNML_FORMAT, 
        EXPORT_GRML_FORMAT, EXPORT_APNN_FORMAT, EXPORT_NETLOGO_FORMAT, EXPORT_DTA_FORMAT,
        EXPORT_AS_PDF, EXPORT_AS_PNG
    }
    
    public common.Action getSharedAction(ActionName shAction);
}
