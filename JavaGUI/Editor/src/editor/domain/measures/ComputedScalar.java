/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import editor.domain.io.XmlExchangeDirection;
import editor.domain.io.XmlExchangeException;
import editor.domain.io.XmlExchangeUtils;
import editor.domain.io.XmlExchangeable;
import java.io.Serializable;
import java.util.Scanner;
import org.w3c.dom.Element;

/** A scalar (double) value computed by a solver. 
 * A computed scalar may be either exact or with a confidence interval.
 *
 * @author elvio
 */
public class ComputedScalar implements Serializable, XmlExchangeable {
    // The computed result
    private final double value;
    // Confidence interval (min_value <= value <= max_value)
    private final double min_value, max_value;
    
    public ComputedScalar() {
        value = min_value = max_value = -1000;
    }

    public ComputedScalar(double min_value, double value, double max_value) {
        this.min_value = min_value;
        this.value = value;
        this.max_value = max_value;
    }

    // Is an interval of confidence, or an exact value?
    public boolean isInterval() {
        return (min_value < value) || (value < max_value);
    }
    
    public final double getMin()     { return min_value; }
    public final double getAverage() { return value; }
    public final double getMax()     { return max_value; }

    @Override
    public String toString() {
        if (isInterval())
            return min_value + " <= " + value + " <= " + max_value;
        else
            return ""+value;
    }
    
    // Allocate a single value from a text stream
    public static ComputedScalar makeFromScanner(Scanner scanner) {
        // Get the type of accuracy
        String acc = scanner.next();
        switch (acc) {
            case "=": // Exact value. Read:   = <value>
                // Get the result in double
                double r = Double.parseDouble(scanner.next());
                return new ComputedScalar(r, r, r);
                
            case "~": // min <= value <= max. Read:   ~ <min> <avg> <max>
                return new ComputedScalar(Double.parseDouble(scanner.next()),
                                          Double.parseDouble(scanner.next()),
                                          Double.parseDouble(scanner.next()));
                
            default:
                System.out.println("Data format error. [scalar]");
                return null;
        }   
    }
    
    // Allocate a table of computed values from a text stream
    // Allocate a single value from a text stream
    public static ComputedScalar[] makeTableFromScanner(Scanner scanner) {
        // Get the number of entries
        int num_entries = scanner.nextInt();
        if (num_entries <= 0)
            return null;
        ComputedScalar[] cstab = new ComputedScalar[num_entries];
        
        // Get the type of accuracy
        String acc = scanner.next();
        switch (acc) {
            case "=": // Exact value. Read:   = <value1> <value2> ...
                // Get the result table in double
                for (int i=0; i<num_entries; i++) {
                    double r = Double.parseDouble(scanner.next());
                    cstab[i] = new ComputedScalar(r, r, r);
                }
                break;
                
            case "~": // min <= value <= max. Read:   ~ <min1> <avg1> <max1> ...
                for (int i=0; i<num_entries; i++) {
                    cstab[i] =  new ComputedScalar(Double.parseDouble(scanner.next()),
                                                   Double.parseDouble(scanner.next()),
                                                   Double.parseDouble(scanner.next()));
                }
                break;
                
            default:
                System.out.println("Data format error. [table]");
                return null;
        }
        
        return cstab;
    }
    
    // Inverse of the makeTableFromScanner() method
    public static String printTableFromScalars(ComputedScalar[] scalars) {
        StringBuilder sb = new StringBuilder();
        sb.append(scalars.length).append(" ");
        boolean all_exact = true;
        for (int i=0; i<scalars.length; i++) {
            if (scalars[i].isInterval()) {
                all_exact = false;
                break;
            }
        }
        
        if (all_exact) {
            sb.append("= ");
            for (ComputedScalar scalar : scalars)
                sb.append(scalar.value).append(" ");
        }
        else {
            sb.append("~ ");
            for (ComputedScalar scalar : scalars) {
                sb.append(scalar.min_value).append(" ").append(scalar.value).append(" ")
                        .append(scalar.max_value).append(" ");
            }
        }
        return sb.toString();
    }

    @Override
    public void exchangeXML(Element el, XmlExchangeDirection exDir) throws XmlExchangeException {
        XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "value", "value", null, double.class);
        XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "min", "min_value", value);
        XmlExchangeUtils.bindXMLAttrib(this, el, exDir, "max", "max_value", value);
    }
}
