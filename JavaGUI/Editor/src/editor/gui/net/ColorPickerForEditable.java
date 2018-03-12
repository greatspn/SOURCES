/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.gui.net;

import common.Util;
import editor.domain.EditableValue;
import editor.domain.ProjectData;
import editor.domain.ProjectPage;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.event.EventListenerList;

/**
 *
 * @author elvio
 */
public class ColorPickerForEditable extends JButton implements ActionListener{
    
    private EditableValue editable;
    private Color color = null;
    
    public static final Color INDETERMINATE = new Color(0,0,0);

    public ColorPickerForEditable() {
        super();
        setIcon(buttonIcon);
        super.addActionListener(this);
        if (Util.isWindows()) {
            setMargin(new Insets(2, 2, 2, 2));
        }
    }
    
    public void setEditableProxy(boolean condition, EditableValue editable) {
        if (!condition)
            editable = null;
        this.editable = editable;
        if (editable == null || !editable.isEditable()) {
            setEnabled(false);
        }
        else {
            setEnabled(true);
            Object value = editable.getValue();
            if (value == INDETERMINATE)
                setColor(INDETERMINATE);
            else
                setColor((Color)editable.getValue());
        }
    }

    public void setColor(Color color) {
        this.color = color;
        repaint();
    }

    public Color getColor() {
        return color;
    }
    
    public EditableValue getEditableProxy() {
        return editable;
    }
        
    private static final int ICN_W = 16, ICN_H = 12, X_BORDER = 2, Y_BORDER = 1;
    private final Icon buttonIcon = makeConstantColorIcon();
    
    private final static Color SELECTED_COLOR = new Color(100, 100, 255);
    private final static Color HOVER_COLOR = new Color(64, 64, 255);
    
    private Icon makeConstantColorIcon() {
        return new Icon() {
            @Override
            public void paintIcon(Component comp, Graphics g, int x, int y) {
                g.translate(x+X_BORDER, y+Y_BORDER);
                Color oldClr = g.getColor();
                g.setColor(Color.BLACK);
                g.drawRect(0, 0, ICN_W, ICN_H);
                g.setColor(Color.WHITE);
                g.drawRect(1, 1, ICN_W-2, ICN_H-2);
                
                if (color == INDETERMINATE) { // Indeterminate
                    g.setColor(Color.LIGHT_GRAY);
                    g.fillRect(2, 2, ICN_W-3, ICN_H-3);
                    g.setColor(Color.DARK_GRAY);
                    g.fillRect(5, 5, ICN_W-9, ICN_H-9);
                }
                else if (color != null) { // paint the color
                    g.setColor(color);
                    g.fillRect(2, 2, ICN_W-3, ICN_H-3);
                }
                else { // null, No color
                    g.setColor(Color.LIGHT_GRAY);
                    g.fillRect(2, 2, ICN_W-3, ICN_H-3);
                    g.setColor(Color.BLACK);
                    g.drawLine(2, 2, ICN_W-2, ICN_H-2);
                    g.drawLine(ICN_W-2, 2, 2, ICN_H-2);
                }

//                // draw the arrow
//                g.setColor(Color.BLACK);
//                g.fillPolygon(xPoints, yPoints, xPoints.length);

                // restore position
                g.translate(-x-X_BORDER, -y-Y_BORDER);
                g.setColor(oldClr);
            }

            @Override public int getIconWidth()   { 
                return ICN_W + 2*X_BORDER;
            }
            @Override public int getIconHeight()  { 
                return ICN_H + 2*Y_BORDER;
            }
        };
    }
    
    public static void main(String[] args) {
        JFrame frame = new JFrame("prova");
        JPanel panel = new JPanel();
        final ColorPickerForEditable cp = new ColorPickerForEditable();
        cp.setEditableProxy(true, new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return Color.BLUE; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                System.out.println("set color "+value);
            }
        });
        cp.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println("new color: "+cp.getColor());
            }
        });
        panel.add(cp);
        final ColorPickerForEditable cp2 = new ColorPickerForEditable();
        cp2.setEditableProxy(true, new EditableValue() {
            @Override public boolean isEditable() { return true; }
            @Override public Object getValue() { return INDETERMINATE; }
            @Override public boolean isValueValid(ProjectData proj, ProjectPage page, Object value) {
                return true;
            }
            @Override public boolean isCurrentValueValid() { return true; }
            @Override public void setValue(ProjectData project, ProjectPage page, Object value) {
                System.out.println("set color "+value);
            }
        });
        cp2.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println("new color: "+cp2.getColor());
            }
        });
        panel.add(cp2);
        frame.add(panel);
        frame.pack();
        frame.setLocationRelativeTo(null);
        
        frame.setVisible(true);
    }
    
    JPopupMenu popup = null;
    @Override
    public void actionPerformed(ActionEvent e) {
        // create the popup menu
        popup = new JPopupMenu();
        JMenuItem noColor = new JMenuItem("No color");
        noColor.addActionListener(noColorListener);
        popup.add(noColor);
        
        ColorTable colTab = new ColorTable();
        popup.add(colTab);
        
        // Open the popup menu
        Util.reformatMenuPanels(popup);
        popup.show(this, 0, getHeight());
    }
    
    private final ActionListener noColorListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            color = null;
            fireMyAction();
        }
    };
    
    private static final float[] COEFFICIENTS = { -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 0.95f };
    private static final int ROWS = COEFFICIENTS.length, COLS = 17;
    private static final int MENU_BORDER_X = 5, MENU_BORDER_Y = 2;
    private static final Color[][] palette;
    static {
        // Create the color palette
        palette = new Color[ROWS][COLS];
        
        for (int r=0; r<ROWS; r++) {
            float grayscale = r / (float)(ROWS - 1);
            palette[r][0] = new Color(grayscale, grayscale, grayscale);
            for (int c=1; c<COLS; c++) {
                float hue = (c - 1) / (float)(COLS - 1);
                Color preparedClr = Color.getHSBColor(hue, 1.0f, 1.0f);
                if (COEFFICIENTS[r] < 0)
                    preparedClr =  Util.darken(preparedClr, 1.0f + COEFFICIENTS[r]);
                else if (COEFFICIENTS[r] > 0)
                    preparedClr =  Util.lighten(preparedClr, COEFFICIENTS[r]);
                palette[r][c] = preparedClr;
            }            
        }
    }
    private class ColorTable extends JPanel {
        
        private int hoverX = -1, hoverY = -1;

        public ColorTable() {
            setOpaque(false);
            addMouseListener(new MouseAdapter() {
                @Override public void mouseClicked(MouseEvent e) {
                    int col = (e.getX() - MENU_BORDER_X) / ICN_W;
                    int row = (e.getY() - MENU_BORDER_Y) / ICN_H;
                    if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                        color = palette[row][col];
                        fireMyAction();
                    }
                    popup.setVisible(false);
                }
                @Override public void mouseEntered(MouseEvent e) {
                    setHover(e);
                }

                @Override public void mouseExited(MouseEvent e) {
                    setHover(null);
                }
            });
            addMouseMotionListener(new MouseMotionAdapter() {
                @Override public void mouseMoved(MouseEvent e) {
                    setHover(e);
                }
            });
        }
        
        private void setHover(MouseEvent e) {
            int newHoverX, newHoverY;
            if (e == null) {
                newHoverX = newHoverY = -1;
            }
            else {
                newHoverX = (e.getX() - MENU_BORDER_X) / ICN_W;
                newHoverY = (e.getY() - MENU_BORDER_Y) / ICN_H;
            }
            if (hoverX != newHoverX || hoverY != newHoverY) {
                hoverX = newHoverX;
                hoverY = newHoverY;
                repaint();
            }
        }
        
        @Override
        public Dimension getPreferredSize() {
            Dimension dim = new Dimension(COLS * (ICN_W) + 1 + 2*MENU_BORDER_X,
                                          ROWS * (ICN_H) + 1 + 2*MENU_BORDER_Y);
            return dim;
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            g.translate(MENU_BORDER_X, MENU_BORDER_Y);
            Color oldClr = g.getColor();
            g.setColor(Color.BLACK);
            for (int r=0; r<=ROWS; r++) // draw horizontal lines
                g.drawLine(0, r * ICN_H, COLS * ICN_W, r * ICN_H);
            for (int c=0; c<=COLS; c++) // draw vertical lines
                g.drawLine(c * ICN_W, 0, c * ICN_W, ROWS * ICN_H);
            for (int r=0; r<ROWS; r++) { // draw blocks
                for (int c=0; c<COLS; c++) {
                    if (r == hoverY && c == hoverX)
                        g.setColor(HOVER_COLOR);
                    else
                        g.setColor(palette[r][c] == color ? SELECTED_COLOR : Color.WHITE);
                    g.drawRect(1+c*ICN_W, 1+r*ICN_H, ICN_W-2, ICN_H-2);
                    g.setColor(palette[r][c]);
                    g.fillRect(2+c*ICN_W, 2+r*ICN_H, ICN_W-3, ICN_H-3);
                }
            }
            g.translate(-MENU_BORDER_X, -MENU_BORDER_Y);
            g.setColor(oldClr);
        }
    }

    
    private final EventListenerList myListenerList = new EventListenerList();
       
    @Override public void addActionListener(ActionListener al) {
        myListenerList.add(ActionListener.class, al);
    }
    
    @Override public void removeActionListener(ActionListener al) {
        myListenerList.remove(ActionListener.class, al);
    }
    
    @Override public ActionListener[] getActionListeners() {
        return myListenerList.getListeners(ActionListener.class);
    }
    
    protected void fireMyAction() {
        ActionEvent event = new ActionEvent(ColorPickerForEditable.this, 0, "change color");
        ActionListener[] list = getActionListeners();
        for (ActionListener al : list) {
            al.actionPerformed(event);
        }
    }
}
