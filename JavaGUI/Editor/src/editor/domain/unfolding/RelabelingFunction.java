/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.unfolding;

import editor.domain.Node;
import editor.domain.SuperpositionTag;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/** Relabels the tags of a node in a composed net
 *
 * @author elvio
 */
public class RelabelingFunction {
    
    private static class TagId {
        public boolean complementary;
        public String tag;

        public TagId(boolean complementary, String tag) {
            this.complementary = complementary;
            this.tag = tag;
        }

        @Override
        public String toString() {
            return tag + (complementary ? "?" : "");
        }

        @Override
        public int hashCode() {
            int hash = 7;
            hash = 53 * hash + (this.complementary ? 1 : 0);
            hash = 53 * hash + Objects.hashCode(this.tag);
            return hash;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) {
                return true;
            }
            if (obj == null) {
                return false;
            }
            if (getClass() != obj.getClass()) {
                return false;
            }
            final TagId other = (TagId) obj;
            if (this.complementary != other.complementary) {
                return false;
            }
            return Objects.equals(this.tag, other.tag);
        }
    }
     
    // Rewriting rules for tags
    private final Map<TagId, TagId> rewriteRules = new HashMap<>();
    
    private static final TagId searchTagId = new TagId(true, "");

    public RelabelingFunction(String rulesListText) {
        String[] rulesList = rulesListText.split(",");
        for (String ruleText : rulesList) {
            if (ruleText.isBlank())
                continue;
            String[] beforeAfter = ruleText.split("->");
            String before = beforeAfter[0].trim();
            String after = beforeAfter[1].trim();
            
            TagId beforeTag, afterTag;
            if (before.endsWith("?"))
                beforeTag = new TagId(true, before.substring(0, before.length()-1));
            else
                beforeTag = new TagId(false, before);
            
            if (after.endsWith("?"))
                afterTag = new TagId(true, after.substring(0, after.length()-1));
            else
                afterTag = new TagId(false, after);
            
            rewriteRules.put(beforeTag, afterTag);
        }
    }
    

    //=========================================================================
    // Tag rewriting rules
    public String rewriteSingleTag(String tag, int card) {
        searchTagId.tag = tag;
        searchTagId.complementary = card < 0;
        TagId replacement = rewriteRules.get(searchTagId);
        return (replacement == null) ? tag : replacement.tag;
    }
    
    //=========================================================================
    // Get the tags that are left to a source node
    public String getKeptTags(Node node) {
        StringBuilder sb =  new StringBuilder();
        int tagCount = 0;
        
        // Take tags from node and check that are not in the delTags[] list
        for (int t=0; t<node.numTags(); t++) {
            String tag = node.getTag(t);
            int card = node.getTagCard(t);
            // search for replacement
            searchTagId.tag = tag;
            searchTagId.complementary = card < 0;
            TagId replacement = rewriteRules.get(searchTagId);
            if (replacement != null) {
                // rewrite tag
                tag = replacement.tag;
                if (searchTagId.complementary != replacement.complementary)
                    card *= -1;
            }
            boolean keep = true;
            if (keep) {
                if (tagCount > 0)
                    sb.append("|");
                sb.append(SuperpositionTag.toCanonicalString(tag, card));
                tagCount++;
            }
        }
        
        return sb.toString();
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("{ ");
        boolean first = true;
        for (Map.Entry<TagId, TagId> entry : rewriteRules.entrySet()) {
            if (first)
                first = false;
            else
                sb.append(", ");
            sb.append(entry.getKey()).append(" -> ").append(entry.getValue());
        }
        sb.append(" }");
        return sb.toString();
    }
    
    public static void main(String[] args) {
        System.out.println(new RelabelingFunction(""));
        System.out.println(new RelabelingFunction("aa->bb"));
        System.out.println(new RelabelingFunction("aa->bb, cc->dd"));
        System.out.println(new RelabelingFunction("aa->bb?, cc?->dd"));
    }

}
