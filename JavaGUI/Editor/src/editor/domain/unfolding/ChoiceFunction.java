/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.unfolding;

import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import java.util.HashSet;
import java.util.Set;

/** Selects which nodes are kept in a composed net
 *
 * @author elvio
 */
public class ChoiceFunction {
    // Tags that are synchronized
    private final String[] syncTags12;
    // Tags whose nodes are removed from net1 and net2
    private final String[] remTags1, remTags2;
    
    // Tags that are deleted (and not merged) in the composed net
    private final String[] delTags12, delTags1, delTags2;
    
    // Merge by shared tags or by common name
    private final MergePolicy mergePolicy;
    
    // Node names from net1 and net2
    private final Set<String> names1, names2;
    
    // Tag-based choice function
    public ChoiceFunction(String[] syncTags12, String[] remTags1, String[] remTags2, 
                          String[] delTags12, String[] delTags1, String[] delTags2) 
    {
        this.syncTags12 = syncTags12;
        this.remTags1 = remTags1;
        this.remTags2 = remTags2;
        this.delTags12 = delTags12;
        this.delTags1 = delTags1;
        this.delTags2 = delTags2;
        this.names1 = null;
        this.names2 = null;
        this.mergePolicy = MergePolicy.BY_TAG;
    }
    
    // Name-based choice function
    public ChoiceFunction(GspnPage net1, GspnPage net2, char type) 
    {
        // Get the list of node names
        this.names1 = new HashSet<>();
        this.names2 = new HashSet<>();
        for (Node node : net1.nodes)
            if ((type=='P' && node instanceof Place) || (type=='T' && node instanceof Transition))
                names1.add(node.getUniqueName());
        for (Node node : net2.nodes)
            if ((type=='P' && node instanceof Place) || (type=='T' && node instanceof Transition))
                names2.add(node.getUniqueName());
        
        this.syncTags12 = null;
        this.remTags1 = null;
        this.remTags2 = null;
        this.delTags12 = null;
        this.delTags1 = null;
        this.delTags2 = null;
        this.mergePolicy = MergePolicy.BY_NAME;
    }
    
    
    
    //=========================================================================
    // Determine if two nodes should be merged togheter
    public boolean nodesShouldBeMerged(Node node1, Node node2) {
        switch (mergePolicy) {
            case BY_NAME:
                // node1 merges node2 if they both have the same name
                return node1.getUniqueName().equals(node2.getUniqueName());
                
            case BY_TAG:
                // node1 merges node2 if they both share a common tag in the restricted list
                if (syncTags12 == null)
                    return false;
                for (int n1=0; n1<node1.numTags(); n1++) {
                    for (int n2=0; n2<node2.numTags(); n2++) {
                        if (node1.getTag(n1).equals(node2.getTag(n2))) {
                            for (String tag : syncTags12)
                                if (tag.equals(node1.getTag(n1)))
                                    return true;
                        }
                    }
                }
                return false;
                
            default:
                throw new IllegalStateException();
        }
    }
    
    // Determine if a non-composed node should be kept in the composed net
    private boolean nodeShouldBeKept(Node node, String[] remTags, Set<String> names) {
        switch (mergePolicy) {
            case BY_NAME:
                return !names.contains(node.getUniqueName());
                
            case BY_TAG:
                if (remTags == null)
                    return true;
                for (int n1=0; n1<node.numTags(); n1++) {
                    for (String tag : syncTags12)
                        if (tag.equals(node.getTag(n1)))
                            return false;
                }
                return true;
                
            default:
                throw new IllegalStateException();
        }
    }
    
    public boolean node1ShouldBeKept(Node node1) {
        return nodeShouldBeKept(node1, remTags1, names2);
    }
    public boolean node2ShouldBeKept(Node node2) {
        return nodeShouldBeKept(node2, remTags2, names1);
    }
    
    //=========================================================================
    // Build the union list of all tags of @node1 and @node2
    public String mergeTags(Node node1, Node node2) {
        StringBuilder sb =  new StringBuilder();
        int tagCount = 0;
        
        // Take tags from node 1
        for (int n1=0; n1<node1.numTags(); n1++) {
            boolean keep = true;
            if (delTags12 != null) {
                for (int d=0; d<delTags12.length && keep; d++)
                    if (delTags12[d].equals(node1.getTag(n1)))
                        keep = false;
            }
            if (keep) 
                sb.append(tagCount++==0 ? "" : "|").append(node1.getTag(n1));
        }
        
        // Take remaining tags from node2
        for (int n2=0; n2<node2.numTags(); n2++) {
            boolean found = false;
            for (int n1=0; n1<node1.numTags() && !found; n1++) {
                if (node1.getTag(n1).equals(node2.getTag(n2))) {
                    found = true;
                }
            }
            if (!found && delTags12 != null) {
                for (int d=0; d<delTags12.length && !found; d++)
                    if (delTags12[d].equals(node2.getTag(n2)))
                        found = false;
            }
            if (!found)
                sb.append(tagCount++==0 ? "" : "|").append(node2.getTag(n2));
                
        }
        //System.out.println("mergeTags "+node1.getSuperPosTags()+" "+node2.getSuperPosTags()+" -> "+sb);
        return sb.toString();
    }
    
    //=========================================================================
    // Get the tags that are left to a source node
    private String getKeptTags(Node node, String[] delTags) {
        StringBuilder sb =  new StringBuilder();
        int tagCount = 0;
        
        // Take tags from node and check that are not in the delTags[] list
        for (int n1=0; n1<node.numTags(); n1++) {
            boolean keep = true;
            if (delTags != null) {
                for (int d=0; d<delTags.length && keep; d++)
                    if (delTags[d].equals(node.getTag(n1)))
                        keep = false;
            }
            if (keep) 
                sb.append(tagCount++==0 ? "" : "|").append(node.getTag(n1));
        }
        
        /*System.out.print("getKeptTags "+node.getSuperPosTags()+" -> "+sb+"   delTags="+(delTags==null ? "null": ""));
        if (delTags != null)
            for (int d=0; d<delTags.length; d++)
                System.out.print((d==0 ? "" : ",") + delTags[d]);
        System.out.println();//*/
        return sb.toString();
    }
    
    public String getKeptTags1(Node node1) { return getKeptTags(node1, delTags1); }
    public String getKeptTags2(Node node2) { return getKeptTags(node2, delTags2); }
    
    
    // policy for generating the merged name from the two operand names
    public String getMergedName(String name1, String name2) {
        if (mergePolicy == MergePolicy.BY_NAME)
            return name1;
        else
            return name1+"_"+name2;
    }
}
