/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import editor.gui.ResourceFactory;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.ImageIcon;
import javax.swing.JToggleButton;

/**
 *
 * @author Elvio
 */
public class JToggleTriState extends JToggleButton {
    
    enum State {
        DESELECTED, TOGGLE_1, TOGGLE_N
    }
    State state = State.DESELECTED;

    public JToggleTriState() {
        setHideActionText(true);
        addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                //repaint();
                /*if (isSelected() && getIcon()==iconUntoggled) {
                    setIcon(iconToggled1);
                }
                else if (!isSelected() && getIcon()==iconToggled1) {
                    setIcon(iconToggledN);
                    JToggleTriState.super.setSelected(true);
                }
                else if (!isSelected() && getIcon()==iconToggledN) {
                    setIcon(iconToggled1);
                    JToggleTriState.super.setSelected(true);                    
                }
                else setIcon(iconUntoggled);*/
            }
        });
    }
    
    public boolean isFirstToggleState() { 
        return isSelected() && state == State.TOGGLE_1;
    }

    @Override
    public void setSelected(boolean selected) {
        setSelected(selected, selected && state != State.TOGGLE_1);
//        if (selected && isSelected()) {
//            System.out.println(getAction().getValue(Action.NAME)+" (1)");
//            setIcon(isFirstToggleState() ? iconToggledN : iconToggled1);
//        }
//        else 
        
//        if (selected) 
//            setIcon(getIcon()==iconToggled1 ? iconToggledN : iconToggled1);
//        else 
//            setIcon(iconUntoggled);
//        
//        super.setSelected(selected);
    }
    
    public void setSelected(boolean selected, boolean isFirstToggle) {
        if (selected) 
            state = (isFirstToggle ? State.TOGGLE_1 : State.TOGGLE_N);
        else 
            state = State.DESELECTED;
        
        super.setSelected(selected);
        repaint();
    }

    //    @Override
    //    public void setAction(Action a) {
    //        super.setAction(a);
    //        if (a == null)
    //            return;
    //        Object iconObject = a.getValue(Action.SMALL_ICON);
    //        if (iconObject==null || !(iconObject instanceof ImageIcon))
    //            return;
    //        iconUntoggled = (ImageIcon)iconObject;
    //
    //        if (OVERLAY_1 != null && iconUntoggled != null) {
    //            BufferedImage img1 = new BufferedImage(iconUntoggled.getIconWidth(), iconUntoggled.getIconHeight(),
    //                                                   BufferedImage.TYPE_INT_ARGB);
    //            Graphics2D g2 = img1.createGraphics();
    //            g2.drawImage(iconUntoggled.getImage(), 0, 0, null);
    //            g2.drawImage(OVERLAY_1, 0, 0, null);
    //            iconToggled1 = new ImageIcon(img1);
    //        }
    //
    //        if (OVERLAY_N != null && iconUntoggled != null) {
    //            BufferedImage imgN = new BufferedImage(iconUntoggled.getIconWidth(), iconUntoggled.getIconHeight(),
    //                                                   BufferedImage.TYPE_INT_ARGB);
    //            Graphics2D g2 = imgN.createGraphics();
    //            g2.drawImage(iconUntoggled.getImage(), 0, 0, null);
    //            g2.drawImage(OVERLAY_N, 0, 0, null);
    //            iconToggledN = new ImageIcon(imgN);
    //        }
    //    }
    @Override
    protected void paintComponent(Graphics grphcs) {
        super.paintComponent(grphcs); //To change body of generated methods, choose Tools | Templates.
        if (state != State.DESELECTED) {
            Graphics2D g2 = (Graphics2D)grphcs;
            ImageIcon overlay = (state == State.TOGGLE_1 ? 
                    ResourceFactory.getInstance().getOverlayOne16() :
                    ResourceFactory.getInstance().getOverlayPlus16());
            int x = (getWidth() - getIcon().getIconWidth()) / 2;
            int y = (getHeight()- getIcon().getIconHeight()) / 2;
            g2.drawImage(overlay.getImage(), x, y, null);
        }
    }

    private String text = "";
    @Override public void setText(String text) { super.setText(""); this.text = text; }
    @Override public String getText() { return text; }
    
}
