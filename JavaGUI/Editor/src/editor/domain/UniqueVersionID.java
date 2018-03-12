/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain;

/** Unique version identifier object
 *
 * @author elvio
 */
public class UniqueVersionID {
    private final int id;
    
    // Static generator of all IDs
    private static int id_gen = 100;

    public UniqueVersionID() {
        this.id = id_gen++;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof UniqueVersionID)
            return id == ((UniqueVersionID)obj).id;
        return false;
    }

    @Override
    public int hashCode() {
        return id ^ super.hashCode(); 
    }

    @Override
    public String toString() {
        return "UID<" + id + ">";
    }
    
}
