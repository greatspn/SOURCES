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
        PLAY_RESTART, PLAY_NEXT, PLAY_PREVIOUS,
        CHANGE_BINDINGS, COMPUTE_PLACE_SEMIFLOWS, COMPUTE_TRANS_SEMIFLOWS, COMPUTE_PLACE_BOUNDS_FROM_PINV,
        EXPORT_GREATSPN_FORMAT, EXPORT_PNML_FORMAT, 
        EXPORT_GRML_FORMAT, EXPORT_APNN_FORMAT, EXPORT_DTA_FORMAT
    }
    
    public common.Action getSharedAction(ActionName shAction);
}
