/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.grammar;

import java.io.Serializable;
import java.util.ArrayList;

/** A set of MultiAssignments to a list of template variables
 *
 * @author elvio
 */
public class VarListMultiAssignment implements Serializable {
    
    public ArrayList<VarMultiAssignment> assigns = new ArrayList<>();
    
    
    public VarMultiAssignment getAssignmentFor(String varName) {
        for (VarMultiAssignment vma : assigns)
            if (vma.varName.equals(varName))
                return vma;
        return null;
    }
}
