/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.elements;

/** How a transition fires.
 *
 * @author elvio
 */
public enum FiringMode {
    INSTANTANEOUS,  // The transition fires in zero time.
    FLOW            // The transition fires continuously, once it is enabled.
}
