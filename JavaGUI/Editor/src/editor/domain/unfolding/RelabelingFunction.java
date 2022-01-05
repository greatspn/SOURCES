/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.unfolding;

import editor.domain.Node;
import editor.domain.SuperpositionTag;
import java.util.Map;

/** Relabels the tags of a node in a composed net
 *
 * @author elvio
 */
public class RelabelingFunction {
     
    // Rewriting rules for tags
    private final Map<String, String> rewriteRules;

    public RelabelingFunction(Map<String, String> rewrite) {
        this.rewriteRules = rewrite;
    }
    

    //=========================================================================
    // Tag rewriting rules
    public String rewriteTag(String tag) {
        if (rewriteRules == null || rewriteRules.isEmpty())
            return tag; // nothing to apply
        String newTag = rewriteRules.get(tag);
        return newTag == null ? tag : newTag;
    }
    
    //=========================================================================
    // Get the tags that are left to a source node
    public String getKeptTags(Node node) {
        StringBuilder sb =  new StringBuilder();
        int tagCount = 0;
        
        // Take tags from node and check that are not in the delTags[] list
        for (int t=0; t<node.numTags(); t++) {
            String tag = rewriteTag(node.getTag(t));
            boolean keep = true;
            if (keep) {
                if (tagCount > 0)
                    sb.append("|");
                sb.append(SuperpositionTag.toCanonicalString(tag, node.getTagCard(t)));
                tagCount++;
            }
        }
        
        return sb.toString();
    }
    

}
