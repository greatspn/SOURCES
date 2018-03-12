package editor.gui;

import common.Util;
import java.awt.Image;
import java.beans.BeanInfo;

/**
 *
 * @author Elvio
 */
public class ResourceFactoryBeanInfo extends java.beans.SimpleBeanInfo {
    static final java.beans.BeanDescriptor beanDescriptor;
    static {
        beanDescriptor = new java.beans.BeanDescriptor(ResourceFactory.class);
        beanDescriptor.setShortDescription("Factory of common resources.");
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
                return Util.loadImage("/editor/gui/icons/resource_factory16.png");
            case BeanInfo.ICON_COLOR_32x32:
                return Util.loadImage("/editor/gui/icons/resource_factory32.png");
        }
        return null;
    }
}
