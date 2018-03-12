/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import editor.Main;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.lang.reflect.Field;
import javax.swing.AbstractAction;
import static javax.swing.Action.ACCELERATOR_KEY;
import static javax.swing.Action.MNEMONIC_KEY;
import static javax.swing.Action.SHORT_DESCRIPTION;
import static javax.swing.Action.SMALL_ICON;
import javax.swing.ActionMap;
import javax.swing.Icon;
import javax.swing.InputMap;
import javax.swing.KeyStroke;
import javax.swing.event.EventListenerList;

/** Generic action
 *
 * @author Elvio
 */
public class Action extends AbstractAction {
    
    // The global listener list (one for all the actions)
    private static final EventListenerList globalListenerList = new EventListenerList();
    
    // I Listeners registrati a questo oggetto Action
    private final EventListenerList listenerList = new EventListenerList();
    
    // Proprieta' aggiuntiva di questa classe Action
    public static final String TOOLBAR_ICON = "MyToolbarIcon";
    
    // Condition object that determines if fire actions should be dispatched or ignored
    private Condition condition;
    
    @Override
    public void actionPerformed(ActionEvent e) {
        if (condition != null && !condition.isSatisfied())
            return; // Ignore the event
        // reinoltra l'evento ai Listeners registrati
        fireGlobalAction(e);
        fireAction(e);        
    }
    
    public void addActionListener(ActionListener al) {
        listenerList.add(ActionListener.class, al);
    }
    
    public void removeActionListener(ActionListener al) {
        listenerList.remove(ActionListener.class, al);
    }
    
    public ActionListener[] getActionListeners() {
        return listenerList.getListeners(ActionListener.class);
    }
    
    protected void fireAction(ActionEvent evt) {
        ActionListener[] list = getActionListeners();
        for (ActionListener al : list) {
            al.actionPerformed(evt);
        }
    }
    
    public static void addGlobalActionListener(ActionListener al) {
        globalListenerList.add(ActionListener.class, al);
    }
    
    public static void removeGlobalActionListener(ActionListener al) {
        globalListenerList.remove(ActionListener.class, al);
    }
    
    public static ActionListener[] getGlobalActionListeners() {
        return globalListenerList.getListeners(ActionListener.class);
    }
    
    public static void fireGlobalAction(ActionEvent evt) {
        ActionListener[] list = getGlobalActionListeners();
        for (ActionListener al : list) {
            al.actionPerformed(evt);
        }
    }
    
    private void mapInto2(ActionMap actMap, InputMap inMap, KeyStroke accelerator) {
        inMap.put(accelerator, getActionName());
        if (actMap.get(getActionName()) == null)
            actMap.put(getActionName(), this);
    }
    
    public void mapInto(ActionMap actMap, InputMap inMap, KeyStroke accelerator) {
        mapInto2(actMap, inMap, modifyKeyStroke(accelerator));
    }
    public void mapInto(ActionMap actMap, InputMap inMap) {
        mapInto2(actMap, inMap, getAccelerator());
    }
    
    // Property manipulation
    
    public String getActionName() {
        return (String)getValue(NAME);
    }
    
    public void setActionName(String name) {
        putValue(NAME, name);
    }
    
    public Integer getMenuMnemonic() {
        return (Integer)getValue(MNEMONIC_KEY);
    }

    public void setMenuMnemonic(Integer menuMnemonic) {
        putValue(MNEMONIC_KEY, menuMnemonic);
    }

    public String getTooltipDesc() {
        return (String)getValue(SHORT_DESCRIPTION);
    }

    public void setTooltipDesc(String tooltipDesc) {
        putValue(SHORT_DESCRIPTION, tooltipDesc);
    }

    public KeyStroke getAccelerator() {
        return (KeyStroke)getValue(ACCELERATOR_KEY);
    }
    
    private KeyStroke modifyKeyStroke(KeyStroke accelerator) {
        if (System.getProperty("os.name").equals("Mac OS X")) {
            int modifiers = accelerator.getModifiers();
            // Convert CTRL -> Command on OSX
            if (0 != (modifiers & (InputEvent.CTRL_DOWN_MASK | InputEvent.CTRL_MASK))) {
                modifiers &= ~(InputEvent.CTRL_DOWN_MASK | InputEvent.CTRL_MASK);
                modifiers |= Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
                accelerator = KeyStroke.getKeyStroke(accelerator.getKeyCode(), 
                                                     modifiers);
            }
        }
        return accelerator;
    }

    public void setAccelerator(KeyStroke accelerator) {
        putValue(ACCELERATOR_KEY, modifyKeyStroke(accelerator));
    }

    public Icon getIcon() {
        return (Icon)getValue(SMALL_ICON);
    }

    public void setIcon(Icon icon) {
        putValue(SMALL_ICON, icon);
    }
    
    public Icon getToolbarIcon() {
        return (Icon)getValue(TOOLBAR_ICON);
    }

    public void setToolbarIcon(Icon icon) {
        putValue(TOOLBAR_ICON, icon);
    }

    public Condition getCondition() {
        return condition;
    }

    public void setCondition(Condition condition) {
        this.condition = condition;
    }
    
    // Quick method for registering all the actions of a JComponent
    public static void registerAllActions(ActionMap actMap, InputMap inMap, 
                                          Object comp, Class compClass, Condition cond) {
        Field[] fields = compClass.getDeclaredFields();
        try {
            for (Field f : fields) {
                if (f.getType() == Action.class) {
                    f.setAccessible(true);
                    Action act = (Action)f.get(comp);
                    act.setCondition(cond);
                    act.mapInto(actMap, inMap);
                    //System.out.println("  "+f.getName()+" "+act);
                }
            }
        }
        catch(IllegalAccessException iae) {
            Main.logException(iae, true);
            System.exit(-1);
        }
    }
}
