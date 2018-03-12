/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package common;

import java.awt.Image;
import java.beans.BeanInfo;

/**
 *
 * @author elvio
 */
public class JFilenameFieldBeanInfo extends java.beans.SimpleBeanInfo {
    static java.beans.BeanDescriptor beanDescriptor;
    static {
        beanDescriptor = new java.beans.BeanDescriptor(JFilenameField.class);
        beanDescriptor.setShortDescription("A pair textfield / file chooser.");
    }
    @Override
    public java.beans.BeanDescriptor getBeanDescriptor() {
        assert beanDescriptor != null;
        return beanDescriptor;
    }

    @Override
    public Image getIcon(int iconKind) {
        switch (iconKind) {
            case BeanInfo.ICON_COLOR_16x16:
                return Util.loadImage("/common/textfield_filename.png");
            case BeanInfo.ICON_COLOR_32x32:
                return null;
        }
        return null;
    }
}
