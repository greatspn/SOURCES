/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.superposition;

import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

/** A  multimap object.
 *
 * @author elvio
 */
public class MultiMap<K, T> {
    private final Map<K, Set<T>> map = new TreeMap<>();

    public MultiMap() {
    }
    
    public void put(K key, T value) {
        assert value != null && !(value instanceof Set);
        Set<T> set = map.get(key);
        if (set == null) {
            set = new HashSet<>();
            set.add(value);
            map.put(key, set);
        }
        else
            set.add(value);
    }
    
    public Collection<T> get(K key) {
        return map.get(key);
    }
    
    public int numKeys() {
        return map.size();
    }
    
    public void clear() { map.clear(); }
    
    public void remove(K key) {
        map.remove(key);
    }
    
    public void remove(K key, T value) {
        Set<T> set = map.get(key);
        if (set != null) {
            if (set.remove(value)) {
                if (set.isEmpty())
                    map.remove(key);
            }
        }
    }
    
    public Set<K> keySet() {
        return map.keySet();
    }
}
