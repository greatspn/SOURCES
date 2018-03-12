/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.measures;

import java.io.Serializable;
import java.util.ArrayList;

/** A table of scalar values, computed with different variable bindings.
 *
 * @author elvio
 */
public class ResultTable implements Serializable {
    
    public ArrayList<ResultEntry> table;

    public ResultTable() {
        this.table = new ArrayList<>();
    }
}
