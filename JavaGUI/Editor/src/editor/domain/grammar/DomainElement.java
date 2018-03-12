/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import editor.domain.AlternateNameFunction;
import editor.domain.NetObject;
import editor.domain.elements.ColorClass;
import editor.domain.elements.NeutralColorClass;
import java.util.Arrays;

/** An immutable element in a color domain, made by N color indexes.
 *
 * @author elvio
 */
public class DomainElement implements Comparable<DomainElement> {
    
    // The color domain
    private final ColorClass domain;
    
    // The indexes of the single colors in the color domain
    private final int[] colors;
    private final String[] colorNames;
    
    // The single element of the neutral domain
    public static DomainElement NEUTRAL_ELEMENT = null;
    
    static {
        NEUTRAL_ELEMENT = new DomainElement(NeutralColorClass.INSTANCE, new int[]{0});
    }
    
    public DomainElement(ColorClass domain, int[] colors) {
        assert domain != null && colors != null;
        assert NEUTRAL_ELEMENT == null || domain != NeutralColorClass.INSTANCE;
        this.domain = domain;
        this.colors = new int[colors.length];
        this.colorNames = new String[colors.length];
        assert this.colors.length == domain.getNumClassesInDomain();
        for (int i=0; i<this.colors.length; i++) {
            assert this.colors[i] >= 0 && this.colors[i] < domain.getColorClass(i).numColors();
            this.colors[i] = colors[i];
            this.colorNames[i] = domain.getColorClass(i).getColorName(colors[i]);
        }
    }
    
    public boolean isNeutralElement() {
        return this == NEUTRAL_ELEMENT;
    }
    
    public int getColor(int i) {
        return colors[i];
    }

    public ColorClass getDomain() {
        return domain;
    }

    @Override
    public String toString() {
        return toStringFormat(ExpressionLanguage.PNPRO);
    }
    
    public String toStringFormat(ExpressionLanguage lang) {
        if (isNeutralElement())
            return "";
        StringBuilder sb = new StringBuilder();
        String open, comma, close;
        switch (lang) {
            case PNPRO:     open = "<";  comma = ",";  close = ">";  break;
            case LATEX:     open = "\\langle";  comma = ",";  close = "\\rangle";  break;
            case APNN:      open = "";  comma = "_";  close = "";  break;
            default:    throw new UnsupportedOperationException();
        }
        sb.append(open);
        for (int i=0; i<colors.length; i++) {
            sb.append(i==0 ? "" : comma);
            if (lang == ExpressionLanguage.LATEX)
                sb.append(AlternateNameFunction.NUMBERS_AS_SUBSCRIPTS.prepareLatexText(colorNames[i], null, NetObject.STYLE_ITALIC));
            else
                sb.append(colorNames[i]);
        }
        sb.append(close);
        return sb.toString();
    }

    @Override
    public int compareTo(DomainElement o) {
        if (o.domain != domain)
            throw new IllegalStateException("Elements of different color domains are not comparable.");
        assert colors.length == o.colors.length;
        for (int i=0; i<colors.length; i++) {
            if (colors[i] < o.colors[i])
                return -1;
            if (colors[i] > o.colors[i])
                return +1;
        }
        return 0;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof DomainElement) {
            DomainElement de = (DomainElement) obj;
            if (de.domain != domain)
                return false;
            assert de.colors.length == colors.length;
            for (int i=0; i<colors.length; i++)
                if (de.colors[i] != colors[i])
                    return false;
            return true;
        }
        return false;
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 73 * hash + this.domain.hashCode();
        hash = 73 * hash + Arrays.hashCode(this.colors);
        return hash;
    }
}
