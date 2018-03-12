package common;

import java.awt.Image;
import java.beans.BeanInfo;

/**
 *
 * @author Elvio
 */
public class ActionBeanInfo extends java.beans.SimpleBeanInfo {
    static final java.beans.BeanDescriptor beanDescriptor;
    static {
        beanDescriptor = new java.beans.BeanDescriptor(Action.class);
        beanDescriptor.setShortDescription("A Swing Action.");
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
                return Util.loadImage("/common/flag_red.png");
            case BeanInfo.ICON_COLOR_32x32:
                return Util.loadImage("/common/flag_red32.png");
        }
        return null;
    }
}
