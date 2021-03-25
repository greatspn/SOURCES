/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.unfolding;

/**
 *
 * @author elvio
 */
public enum MatrixMode {
    INPUT_MATRIX("Input matrix (I)"),
    OUTPUT_MATRIX("Output matrix (O)"),
    INHIBITOR_MATRIX("Inhibitor matrix (H)"),
    INCIDENCE_MATRIX("Incidence Matrix (I-O)");

    String visualized;
    private MatrixMode(String visualized) {
        this.visualized = visualized;
    }
    @Override
    public String toString() {
        return visualized;
    }
}
