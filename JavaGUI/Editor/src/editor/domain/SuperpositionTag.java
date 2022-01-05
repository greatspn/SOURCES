/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package editor.domain;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Objects;

/** Immutable class of a superposition tag
 *
 * @author elvio
 */
public class SuperpositionTag implements Comparable<SuperpositionTag> {
    
    private String tag; // the tag label
    private int card;   // the cardinality

    public SuperpositionTag(String tag, int card) {
        this.tag = tag;
        this.card = card;
    }

    public String getTag() { 
        return tag;
    }

    public int getCard() {
        return card;
    }
    
    // Parse a tag definition string into a tag definition string
    public static SuperpositionTag[] parseTags(String allTagDefs) {
        String[] baseTags = allTagDefs.split("\\|"); // | is escaped since it is a regex
        
        ArrayList<SuperpositionTag> tags = new ArrayList<>();
        tags.ensureCapacity(baseTags.length);
        for (int tt=0; tt<baseTags.length; tt++) {
            String tagDef = baseTags[tt].trim();
//            System.out.println("tagDef = " + tagDef);
            if (tagDef.isEmpty())
                continue;
            // parse cardinality
            int i=0;
            if (tagDef.charAt(i)=='-')
                i++;
            while (i < tagDef.length() && Character.isDigit(tagDef.charAt(i)))
                ++i;
            int endCard = i;
            // skip '*' or whitespaces
            while (i < tagDef.length() && (tagDef.charAt(i)=='*' || Character.isWhitespace(tagDef.charAt(i))))
                ++i;
            int startTag = i;
            int endTag = tagDef.length();
            // remove final '?'
            int sign = 1;
            if (tagDef.charAt(endTag-1) == '?') {
                --endTag;
                sign *= -1;
            }
            
            String tag = tagDef.substring(startTag, endTag);
            int card = (endCard > 0) ? Integer.parseInt(tagDef.substring(0, endCard)) : 1;
            tags.add(new SuperpositionTag(tag, card * sign));
        }
        
        // Sort and merge the same tags together
        return mergeTags(tags);
    }
    
    // merge tags with the same name, combining the cardinalities
    public static SuperpositionTag[] mergeTags(ArrayList<SuperpositionTag> tags) {
        // Sort and merge the same tags together
        Collections.sort(tags);
        int i=0;
        boolean removed = false;
        for (int j=1; j<tags.size(); j++) {
            if (tags.get(i).tag.equals(tags.get(j).tag)) { // merge
                tags.get(i).card += tags.get(j).card;
                tags.set(j, null);
                removed = true;
            }
            else
                i = j;
        }
        if (removed)
            tags.removeIf(Objects::isNull);
        
        return tags.toArray(new SuperpositionTag[tags.size()]);
    }

    @Override
    public int compareTo(SuperpositionTag t) {
        int c = tag.compareTo(t.tag);
        if (c != 0)
            return c;
        return Integer.compare(card, t.card);
    }

    @Override
    public String toString() {
//        return card + " " + tag;
        return toCanonicalString();
    }
    
    // representation in GreatSPN format
    public String toCanonicalString() {
        return toCanonicalString(tag, card);
    }
    
    public static String toCanonicalString(String tag, int card) {
        if (card==1) return tag;
        if (card==-1) return tag+"?";
        if (card > 1) return card+"*"+tag;
        if (card < 1) return (-card)+"*"+tag+"?";
        return "0*"+tag;
    }
    
    public static void main(String[] args) {
        String def = "a|b|d? | 2*c | 3 * b? | fff | ffg? | -5 xx | -34 zz? ";
        System.out.println(def);
        SuperpositionTag[] tags = parseTags(def);
        for (SuperpositionTag st : tags)
            System.out.println(st);
    }
}
