/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import java.awt.Image;
import java.beans.BeanDescriptor;
import static java.beans.BeanInfo.ICON_COLOR_16x16;

/**
 *
 * @author Elvio
 */
public class JCollapsiblePanelBeanInfo extends java.beans.SimpleBeanInfo {

    static java.beans.BeanDescriptor beanDescriptor = null;

    @Override
    public java.beans.BeanDescriptor getBeanDescriptor() {
        if (beanDescriptor == null) {
//            ParamDescriptor[] paramDescriptors = new ParamDescriptor[2];
//            ParamDescriptor[] outputDescriptors = new ParamDescriptor[1];
//
////This parameter is set before we service the output parameter.
//            outputDescriptors[0] = new ParamDescriptor("index", "loop index (0-based)",
//                                                       Integer.class, false, false, null);
//
//            paramDescriptors[0] = new ParamDescriptor("numItems",
//                                                      "number of times to call output",
//                                                      Integer.class, false, false, null);
//            paramDescriptors[1] = new ParamDescriptor("output",
//                                                      "rendered for each iteration",
//                                                      java static constructorServlet.class,
//                                                      false, true, outputDescriptors);

            beanDescriptor = new BeanDescriptor(JCollapsiblePanel.class);
            beanDescriptor.setShortDescription("A Swing Panel that is collapsible.");
//            beanDescriptor.setValue("paramDescriptors", paramDescriptors);
//            beanDescriptor.setValue("componentCategory", "Servlet Beans");
            beanDescriptor.setValue("containerDelegate", "getContentPane");
            beanDescriptor.setValue("isContainer", Boolean.TRUE);

        }
        return beanDescriptor;
    }

    @Override
    public Image getIcon(int iconKind) {
        switch (iconKind) {
            case ICON_COLOR_16x16:
                return Util.loadImage("/common/collapsible_panel16.png");
            case ICON_COLOR_32x32:
                return null;
        }
        return null;
    }
}
