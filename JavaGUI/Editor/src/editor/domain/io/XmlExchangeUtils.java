/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.io;

import java.awt.Color;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.UUID;
import org.w3c.dom.Element;

/** Utilities to implement data exchange to/from XML files
 *
 * @author elvio
 */
public class XmlExchangeUtils {
    
//    // Make type name obtained by field.getType().getName() unique
//    private static String makeTypeNameCanonical(String typeName) {
//        switch (typeName) {
//            case "int":         return Integer.class.getName();
//            case "boolean":     return Boolean.class.getName();
//            case "double":      return Double.class.getName();
//        }
//        return typeName;
//    }
    
    private static Class<?> getPrimitiveType(Class<?> type) {
        if (type.equals(Integer.class))
            return int.class;
        if (type.equals(Boolean.class))
            return boolean.class;
        if (type.equals(Double.class))
            return double.class;
        return type; // not primitive
    }
    
    // Modified parse boolean - accepts also 1/0 as true/false
    private static boolean modifiedParseBoolean(String text) {
        if (text.equals("1"))
            return true;
        if (text.equals("0"))
            return false;
        return Boolean.parseBoolean(text);
    }
    
    private static String colorToString(Color c) {
        if (c == null)
            return "none";
        char[] buf = new char[7];
        buf[0] = '#';
        String s = Integer.toHexString(c.getRed());
        if (s.length() == 1) {
                buf[1] = '0';
                buf[2] = s.charAt(0);
        }
        else {
                buf[1] = s.charAt(0);
                buf[2] = s.charAt(1);
        }
        s = Integer.toHexString(c.getGreen());
        if (s.length() == 1) {
                buf[3] = '0';
                buf[4] = s.charAt(0);
        }
        else {
                buf[3] = s.charAt(0);
                buf[4] = s.charAt(1);
        }
        s = Integer.toHexString(c.getBlue());
        if (s.length() == 1) {
                buf[5] = '0';
                buf[6] = s.charAt(0);
        }
        else {
                buf[5] = s.charAt(0);
                buf[6] = s.charAt(1);
        }
        return String.valueOf(buf);
    }
    
    private static Color stringToColor(String str) {
        if (str.equals("none"))
            return null;
        return Color.decode(str);
    }
    
    private static Field searchField(Class<?> current, String fieldName) throws XmlExchangeException {
        Field field = null;
        do { // Look recursively in this class and in each super class.
           try {
               return current.getDeclaredField(fieldName);
           } 
           catch(NoSuchFieldException | SecurityException e) {
               current = current.getSuperclass();
           }
        } 
        while(current != null);

        throw new XmlExchangeException("Field "+fieldName+" does not exists.");
    }
    
    private static Method searchMethod0(Class<?> current, String methodName) 
        throws XmlExchangeException
    {
        do { // Look recursively in this class and in each super class.
            try {
                return current.getDeclaredMethod(methodName);
            } 
            catch(NoSuchMethodException | SecurityException e) {
                current = current.getSuperclass(); // try 
            }
        } 
        while(current != null);
        throw new XmlExchangeException("Method "+methodName+"() does not exists.");
    }
    
    private static Method searchMethod1(Class<?> current, String methodName, Class<?> arg1Type) 
        throws XmlExchangeException
    {
        arg1Type = getPrimitiveType(arg1Type);
        do { // Look recursively in this class and in each super class.
            try {
                return current.getDeclaredMethod(methodName, arg1Type);
            } 
            catch(NoSuchMethodException | SecurityException e) {
                current = current.getSuperclass(); // try 
            }
        } 
        while(current != null);
        throw new XmlExchangeException("Method "+methodName+"("+arg1Type+") does not exists.");
    }
        
    private static void setProperty(Object target, String propName, Object value, Class propType) 
            throws XmlExchangeException
    {
        try {
            if (propName.contains(".")) {
                int pos = propName.lastIndexOf('.');
                String prefix = propName.substring(0, pos);
                // Change target recursively
                propName = propName.substring(pos + 1);
                target = getProperty(target, prefix);
            }

            if (propName.startsWith("@")) { // Set/get
                Method setter = searchMethod1(target.getClass(), "set"+propName.substring(1), propType);
                setter.setAccessible(true);
                setter.invoke(target, value);
            }
            else { // direct field
                Field field = searchField(target.getClass(), propName);
                field.setAccessible(true);
                field.set(target, value);
            }
        }
        catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            throw new XmlExchangeException(e);
        }
    }
    
    private static Object getProperty(Object target, String propName) 
            throws XmlExchangeException
    {
        try {
            if (propName.contains(".")) {
                int pos = propName.lastIndexOf('.');
                String prefix = propName.substring(0, pos);
                // Change target recursively
                propName = propName.substring(pos + 1);
                target = getProperty(target, prefix);
            }

            if (propName.startsWith("@")) { // Set/get
                Method getter = searchMethod0(target.getClass(), "get"+propName.substring(1));
                getter.setAccessible(true);
                return getter.invoke(target);
            }
            else { // direct field
                Field field = searchField(target.getClass(), propName);
                field.setAccessible(true);
                return field.get(target);
            }
        }
        catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            throw new XmlExchangeException(e);
        }
    }
    
    public static void bindXMLAttrib(Object target, Element el, XmlExchangeDirection exDir,
                                      String xmlAttrName, String propName, Object defaultValue)
            throws XmlExchangeException
    {
        assert defaultValue != null;
        bindXMLData(target, el, exDir, xmlAttrName, propName, defaultValue, 
                    defaultValue.getClass(), XML_SOURCE_ATTRIB);
    }
    
    // Bind an xml attribute @xmlAttrName to an int field @fieldName of class @target
    public static void bindXMLAttrib(Object target, Element el, XmlExchangeDirection exDir,
                                      String xmlAttrName, String propName, Object defaultValue,
                                      Class propType) 
            throws XmlExchangeException
    {
        bindXMLData(target, el, exDir, xmlAttrName, propName, defaultValue, 
                    propType, XML_SOURCE_ATTRIB);
    }
    
    public static void bindXMLContent(Object target, Element el, XmlExchangeDirection exDir,
                                      String propName, Object defaultValue)
            throws XmlExchangeException
    {
        assert defaultValue != null;
        bindXMLData(target, el, exDir, null, propName, defaultValue, 
                    defaultValue.getClass(), XML_SOURCE_TEXT_CONTENT);
    }
    
    private static final int XML_SOURCE_ATTRIB = 1;
    private static final int XML_SOURCE_TEXT_CONTENT = 2;
    
    // Bind an xml attribute @xmlAttrName to an int field @fieldName of class @target
    private static void bindXMLData(Object target, Element el, XmlExchangeDirection exDir,
                                      String xmlAttrName, String propName, Object defaultValue,
                                      Class propType, int xmlSource) 
            throws XmlExchangeException
    {
        assert exDir != null && target != null && el != null;
        assert propName != null && propType != null;
        assert (xmlSource != XML_SOURCE_ATTRIB || xmlAttrName != null);
        assert (xmlSource != XML_SOURCE_TEXT_CONTENT || xmlAttrName == null);

        if (exDir.XmlToFields()) {
            String xmlValue = null;
            boolean useDefault = false;
            if (xmlSource == XML_SOURCE_ATTRIB) {
                if (el.hasAttribute(xmlAttrName)) // Has attribute, use it
                    xmlValue = el.getAttribute(xmlAttrName);
                else // Attribute is not specified, use default value
                    useDefault = true;
            }
            else if (xmlSource == XML_SOURCE_TEXT_CONTENT)
                xmlValue = el.getTextContent();
            else 
                throw new IllegalArgumentException();

//            boolean useDefault = (xmlValue == null || xmlValue.length() == 0);
            Object value;
            if (useDefault) {
                value = defaultValue;
                // If the binder has specified a null value for the @defaultValue,
                // then the attribute is compulsory, and must be found.
                if (value == null) { // Cannot accept null value.
                    throw new XmlExchangeException("Missing fundamental attribute '"+xmlAttrName+
                            "' in XML element <"+el.getNodeName()+">.");
                }
            }
            else {
                assert xmlValue != null;
                if (propType.equals(Integer.class) || propType.equals(int.class))
                    value = Integer.parseInt(xmlValue);
                else if (propType.equals(Boolean.class) || propType.equals(boolean.class))
                    value = modifiedParseBoolean(xmlValue);
                else if (propType.equals(Double.class) || propType.equals(double.class))
                    value = Double.parseDouble(xmlValue);
                else if (propType.equals(String.class))
                    value = xmlValue;
                else if (propType.equals(UUID.class))
                    value = UUID.fromString(xmlValue);
                else if (propType.equals(Color.class))
                    value = stringToColor(xmlValue);
                else if (propType.isEnum()) {
                    // Convert enumerative name from XML into its value
                    Object[] allEnums = propType.getEnumConstants();
                    Object enumerative = null;
                    for (Object e : allEnums) {
                        Enum<?> ee = (Enum<?>)e;
                        if (ee.name().equals(xmlValue)) {
                            enumerative = e;
                            break;
                        }
                    }
                    if (enumerative == null)
                        throw new UnsupportedOperationException("Missing enumerative value for \""+xmlValue+"\"");
                    value = enumerative;
                }
                else
                    throw new UnsupportedOperationException("Add new type serialization rule for "+propType.getName());
            }

            setProperty(target, propName, value, propType);
        }
        else if (exDir.FieldsToXml()) {
            Object value = getProperty(target, propName);
            if (defaultValue != null && value.equals(defaultValue))
                return;

            String xmlValue;
            if (propType.equals(Color.class))
                xmlValue = colorToString((Color)value);
            else if (propType.isEnum())
                xmlValue = ((Enum<?>)value).name();
            else
                xmlValue = value.toString();

            if (xmlSource == XML_SOURCE_ATTRIB)
                el.setAttribute(xmlAttrName, xmlValue);
            else if (xmlSource == XML_SOURCE_TEXT_CONTENT)
                el.setTextContent(xmlValue);
            else 
                throw new IllegalStateException();
        }
    }
}
