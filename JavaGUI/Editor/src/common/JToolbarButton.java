/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import javax.swing.Action;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.UIManager;

/** Specialized toolbar button.
 *
 * @author Elvio
 */
public class JToolbarButton extends JButton {
    
    private PropertyChangeListener pcListener;
    
    public JToolbarButton() {
        // Configurazione di base di un toolbar button
        //setBorder(null);
        setBorderPainted(false);
        setFocusable(false);
        setHorizontalTextPosition(RIGHT);
        setVerticalTextPosition(CENTER);
        setHideActionText(true);
        super.setText("");
    }

    @Override
    public void setAction(Action a) {
        if (getAction() != null && pcListener != null) {
            getAction().removePropertyChangeListener(pcListener);
        }
        super.setAction(a);
        if (a != null) {
            // Take current description and icon (if any)
            String tooltipText = (String)a.getValue(Action.SHORT_DESCRIPTION);
            setToolTipText(tooltipText != null ? tooltipText : "");
            Icon toolIcon = (Icon)a.getValue(common.Action.TOOLBAR_ICON);
            if (toolIcon == null)
                toolIcon = (Icon)a.getValue(common.Action.SMALL_ICON);
            setIcon(toolIcon);
            setDisabledIcon(UIManager.getLookAndFeel().getDisabledIcon(this, toolIcon));
            
//            String name = (String)a.getValue(Action.NAME);
//            setText(name != null ? name : "");
            
            // Register a listener for future changes
            if (pcListener == null) {
                pcListener = new PropertyChangeListener() {
                    @Override
                    public void propertyChange(PropertyChangeEvent evt) {
                        switch (evt.getPropertyName()) {
                            case Action.SHORT_DESCRIPTION:
                                setToolTipText((String)evt.getNewValue());
                                break;
                            case common.Action.TOOLBAR_ICON:
                                setIcon((Icon)evt.getNewValue());
                                break;
                            case common.Action.SMALL_ICON:
                                if (getAction().getValue(common.Action.TOOLBAR_ICON) == null)
                                    setIcon((Icon)evt.getNewValue());
                                break;
                        }
                    }
                };
            }
            a.addPropertyChangeListener(pcListener);
        }
    }

//    private String myText = "";
//    @Override public void setText(String text) { 
//        //System.out.println("dropping: "+text+" getText="+super.getText()); 
//        this.myText = text; 
//        super.setText("");
//    }
//    @Override public String getText() { return myText; }

}
