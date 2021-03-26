/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.domain.semiflows;

import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Place;
import editor.domain.elements.Transition;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/** Index of net objects (P/T)
 *
 * @author elvio
 */
public class NetIndex {
    
    public final GspnPage net;
    // Place/Transition -> 0-based index
    public Map<Place, Integer> place2index;
    public Map<Transition, Integer> trn2index;
    public ArrayList<Place> places;
    public ArrayList<Transition> transitions;
    
    public NetIndex(GspnPage net) {
        this.net = net;
        place2index = new HashMap<>();
        trn2index = new HashMap<>();
        places = new ArrayList<>();
        transitions = new ArrayList<>();
        
        // Prepare place/transition indexes for semiflow construction.
        for (Node node : net.nodes) {
            if (node instanceof Place) {
                Place place = (Place)node;
                place2index.put(place, place2index.size());
                places.add(place);
            }
            else if (node instanceof Transition) {
                Transition trn = (Transition)node;
                trn2index.put(trn, trn2index.size());
                transitions.add(trn);
            }
        }
    }
    
    public int numPlaces() { return places.size(); }
    
    public int numTransition() { return transitions.size(); }

}
