/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import java.awt.AlphaComposite;
import java.awt.Component;
import java.awt.Composite;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.image.BufferedImage;
import javax.swing.Icon;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.Timer;

/** Custom control: un JPanel richiudibile
 *
 * @author Elvio
 */
public class JCollapsiblePanel extends javax.swing.JPanel {

    private static final Icon MINUS = Util.loadIcon("/gui/icons/toggle.png");
    private static final Icon PLUS = Util.loadIcon("/gui/icons/toggle_expand.png");
    private Container contentPane = null;
    
    private java.util.Date startTime;
    private static long durationInMillisec = 300;
    private boolean animated = true;
    private static final int ANIM_TIMER_RES = 30; // millisec. between two animations
    private int height, width;
    private Timer timer;
    private ActionListener timerListener;

    /**
     * @return the animated
     */
    public boolean isAnimated() {
        return animated;
    }

    /**
     * @param animated the animated to set
     */
    public void setAnimated(boolean animated) {
        this.animated = animated;
    }
    
    enum State {
        COLLAPSED, COLLAPSING, EXPANDED, EXPANDING
    }
    private State state = State.EXPANDED;
    
    /**
     * Creates new form JCollapsiblePanel
     */
    public JCollapsiblePanel() {
        initComponents();
        jPanelAnimation.setVisible(false);
        setContentPane(jPanelContent);
        jLabelHeader.setText("");
        Font curFont = jLabelHeader.getFont();
        jLabelHeader.setFont(curFont.deriveFont(Font.BOLD, curFont.getSize()+3));
        jButtonCollapseExpand.setText("");
        jButtonCollapseExpand.setIcon(MINUS);
                
        timerListener = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                JAnimationPanel animPanel = (JAnimationPanel)jPanelAnimation;
                boolean collapsing = (state == State.COLLAPSING);
                long timeInMillisec = new java.util.Date().getTime() - startTime.getTime();
                if (timeInMillisec > durationInMillisec) {
                    // end of animation
                    jPanelContent.setVisible(!collapsing);
                    animPanel.setVisible(false);
                    animPanel.setupImageOf(null);
                    jButtonCollapseExpand.setEnabled(true);
                    jButtonCollapseExpand.setIcon(collapsing ? PLUS : MINUS);
                    state = (collapsing ? State.COLLAPSED : State.EXPANDED);
                    timer.stop();
                }
                else {
                    animPanel.alpha = ((float)timeInMillisec) / durationInMillisec;
                    if (collapsing)
                         animPanel.alpha = 1.0f - animPanel.alpha;
                    int newHeight = (int)(height * animPanel.alpha);
                    
                    Dimension dim = new Dimension(width, newHeight);
                    animPanel.setPreferredSize(dim);
                    animPanel.setMinimumSize(dim);
                    animPanel.setSize(dim);
                }
                JCollapsiblePanel.this.revalidate();
            }
        };
    }

    public java.awt.Container getContentPane() {
        return contentPane;
    }
    public final void setContentPane(java.awt.Container cont) {
        if (cont instanceof JComponent && !((JComponent)cont).isOpaque())
            ((JComponent)cont).setOpaque(true);
        contentPane = cont;
    }
    
    public boolean isHeaderVisible() {
        return jPanelHeader.isVisible();
    }

    public void setHeaderVisible(boolean headerVisible) {
        jPanelHeader.setVisible(headerVisible);
    }    
    
    @Override
    protected void addImpl(Component comp, Object constraints, int index) {
        if (getContentPane() == null)
            super.addImpl(comp, constraints, index);
        else
            getContentPane().add(comp, constraints, index);
    }

    @Override
    public void remove(Component comp) {
        assert getContentPane() != null;
        jPanelContent.remove(comp);
    }

    @Override
    public void remove(int index) {
        assert getContentPane() != null;
        jPanelContent.remove(index);
    }

    @Override
    public void removeAll() {
        assert getContentPane() != null;
        jPanelContent.removeAll();
    }
    
    public String getHeaderText() {
        return jLabelHeader.getText();
    }
    
    public void setHeaderText(String txt) {
        jLabelHeader.setText(txt);
    }
    
    public boolean isCollapsed() {
        return (state == State.COLLAPSED);
    }
    
    public void setCollapsed(boolean collapsed) {
        if (collapsed && (state == State.EXPANDED || state == State.EXPANDING)) {
            state = (isAnimated() ? State.COLLAPSING : State.COLLAPSED);
        }
        else if (!collapsed && (state == State.COLLAPSED || state == State.COLLAPSING)) {
            state = (isAnimated() ? State.EXPANDING : State.EXPANDED);
        }
        else return; // No state change
        
        if (!isAnimated()) {
            jPanelContent.setVisible(state == State.EXPANDED);
            jButtonCollapseExpand.setIcon((state == State.EXPANDED) ? MINUS : PLUS);
        }
        else {
            // start animation
            JAnimationPanel animPanel = (JAnimationPanel)jPanelAnimation;
            animPanel.setupImageOf(jPanelContent);
            Dimension startDim = jPanelContent.getSize();
            if (state == State.EXPANDING) {
                startDim.height = 1;
            }
            else 
                jPanelContent.setVisible(false);
            
            animPanel.setSize(startDim);
            animPanel.setMinimumSize(startDim);
            animPanel.setPreferredSize(startDim);
            height = jPanelContent.getHeight();
            width = jPanelContent.getWidth();            
            animPanel.setVisible(true);
            // restart the timer
            if (timer != null) {
                timer.stop();
            }
            startTime = new java.util.Date();
            jButtonCollapseExpand.setEnabled(false);
            timer = new Timer(ANIM_TIMER_RES, timerListener);
            timer.setInitialDelay(ANIM_TIMER_RES);
            timer.start();
        }
    }
    
    public void setCollapsedNoAnim(boolean collapsed) {
        boolean isAnim = isAnimated();
        setAnimated(false);
        setCollapsed(collapsed);
        setAnimated(isAnim);
    }

    @Override
    public Dimension getPreferredSize() {
        Dimension d = super.getPreferredSize();
//        if (state == State.COLLAPSED)
//            d.width = Math.max(d.width, jPanelContent.getWidth());
        
//        d.width = Math.max(d.width, jPanelContent.getWidth());
//        d.width = Math.max(d.width, jPanelHeader.getWidth());
//        d.width = Math.max(d.width, jPanelAnimation.getWidth());
        return d;
    }
    
    /* Pannello che memorizza l'immagina dei componenti che si stanno
     * aprenso/chiudendo e li disegna con l'appropriato alpha factor
     */    
    private class JAnimationPanel extends JPanel {
        BufferedImage img;
        public float alpha;
        
        private void setupImageOf(JComponent component) {
            img = null;
            if (component != null) {
                Dimension dim = component.getSize();
                if (dim.height > 0 && dim.width > 0) {
                    img = getGraphicsConfiguration().createCompatibleImage(dim.width,
                                                                           dim.height);
                    component.paint(img.getGraphics());
                    alpha = 1.0f;
                }
            }
        }
        
        @Override
        public void paintComponent(Graphics g) {
            assert isAnimated();
            if (g == null || img == null) {
                super.paintComponent(g);
            }
            else {
                 g.drawImage(img, 0, getHeight() - img.getHeight(), null);
            }
        }
        
        @Override
        public void paint(Graphics g) {
            Graphics2D g2d = (Graphics2D) g;
            Composite oldComp = g2d.getComposite();
            Composite alphaComp = AlphaComposite.getInstance(AlphaComposite.SRC_OVER,
                                                             alpha);
            g2d.setComposite(alphaComp);
            super.paint(g2d);
            g2d.setComposite(oldComp);
        }
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        jPanelHeader = new javax.swing.JPanel();
        jButtonCollapseExpand = new javax.swing.JButton();
        jLabelHeader = new javax.swing.JLabel();
        jPanelAnimation = new JAnimationPanel();
        jPanelContent = new javax.swing.JPanel();

        setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(102, 102, 102)));
        setLayout(new java.awt.GridBagLayout());

        jPanelHeader.setBackground(new java.awt.Color(216, 230, 255));
        jPanelHeader.setLayout(new java.awt.GridBagLayout());

        jButtonCollapseExpand.setText("Toggle");
        jButtonCollapseExpand.setBorderPainted(false);
        jButtonCollapseExpand.setContentAreaFilled(false);
        jButtonCollapseExpand.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonCollapseExpandActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(0, 3, 0, 3);
        jPanelHeader.add(jButtonCollapseExpand, gridBagConstraints);

        jLabelHeader.setText("Header");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 4;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.LINE_START;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(3, 0, 3, 3);
        jPanelHeader.add(jLabelHeader, gridBagConstraints);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.PAGE_START;
        gridBagConstraints.weightx = 1.0;
        add(jPanelHeader, gridBagConstraints);

        jPanelAnimation.setBackground(new java.awt.Color(255, 102, 0));
        jPanelAnimation.setBorder(javax.swing.BorderFactory.createMatteBorder(1, 0, 0, 0, new java.awt.Color(102, 102, 102)));
        jPanelAnimation.setLayout(new javax.swing.BoxLayout(jPanelAnimation, javax.swing.BoxLayout.LINE_AXIS));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        add(jPanelAnimation, gridBagConstraints);

        jPanelContent.setBorder(javax.swing.BorderFactory.createMatteBorder(1, 0, 0, 0, new java.awt.Color(102, 102, 102)));

        javax.swing.GroupLayout jPanelContentLayout = new javax.swing.GroupLayout(jPanelContent);
        jPanelContent.setLayout(jPanelContentLayout);
        jPanelContentLayout.setHorizontalGroup(
            jPanelContentLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 268, Short.MAX_VALUE)
        );
        jPanelContentLayout.setVerticalGroup(
            jPanelContentLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 171, Short.MAX_VALUE)
        );

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        add(jPanelContent, gridBagConstraints);
    }// </editor-fold>//GEN-END:initComponents

    private void jButtonCollapseExpandActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonCollapseExpandActionPerformed
        setCollapsed(!isCollapsed());
    }//GEN-LAST:event_jButtonCollapseExpandActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton jButtonCollapseExpand;
    private javax.swing.JLabel jLabelHeader;
    private javax.swing.JPanel jPanelAnimation;
    private javax.swing.JPanel jPanelContent;
    private javax.swing.JPanel jPanelHeader;
    // End of variables declaration//GEN-END:variables
}
