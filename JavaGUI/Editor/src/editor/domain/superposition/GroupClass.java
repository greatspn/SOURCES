/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

/** A class of groupable and synchronizable nodes.
 *
 * @author elvio
 */
public enum GroupClass {
    CONSTANT(false), 
    COLOR_CLASS(false), 
    COLOR_VAR(false),
    CLOCK(false),
    LOCATION(false),
    PLACE(true), 
    TRANSITION(true),
    NON_GROUPABLE(false);
    
    // Is this node class a node with tags?
    private final boolean tagSuperpos;

    private GroupClass(boolean tagSuperpos) {
        this.tagSuperpos = tagSuperpos;
    }

    // Is superoposition of this class of nodes guided by tags?
    public boolean hasTagSuperposition() {
        return tagSuperpos;
    }

    public boolean hasNameSuperposition() {
        return !hasTagSuperposition();
    }
    
    public boolean isGroupable() { 
        return this != NON_GROUPABLE;
    }
}
