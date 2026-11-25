package editor.gui.net;

import java.io.*;
import java.util.*;
import java.util.regex.*;

public class DotDeadlockHighlighter {

    // Readable shades on white background, distinct from default black and red dead markings
    private static final String GREEN_ATTR = "color=\"#228B22\" fontcolor=\"#228B22\"";
    private static final String ORANGE_ATTR = "color=\"#E66C00\" fontcolor=\"#E66C00\"";

    private static final Pattern EDGE_PATTERN = Pattern.compile("^\\s*([A-Z]\\d+)\\s*->\\s*([A-Z]\\d+)\\s*\\[");
    private static final Pattern NODE_PATTERN = Pattern.compile("^\\s*([A-Z]\\d+)\\s*\\[");

    public static void process(File dotFile) throws IOException {
        List<String> lines = new ArrayList<>();
        try (BufferedReader br = new BufferedReader(new FileReader(dotFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                lines.add(line);
            }
        }

        Map<String, Set<String>> forwardEdges = new HashMap<>();
        Map<String, Set<String>> reverseEdges = new HashMap<>();
        Set<String> allNodes = new HashSet<>();
        Set<String> deadNodes = new HashSet<>();

        for (String line : lines) {
            Matcher em = EDGE_PATTERN.matcher(line);
            if (em.find()) {
                String u = em.group(1);
                String v = em.group(2);
                forwardEdges.computeIfAbsent(u, k -> new HashSet<>()).add(v);
                reverseEdges.computeIfAbsent(v, k -> new HashSet<>()).add(u);
                allNodes.add(u);
                allNodes.add(v);
            } else {
                Matcher nm = NODE_PATTERN.matcher(line);
                if (nm.find()) {
                    String u = nm.group(1);
                    allNodes.add(u);
                    if (u.startsWith("D")) {
                        deadNodes.add(u);
                    }
                }
            }
        }

        // Backward BFS: states that can reach at least one deadlock (EF deadlock)
        Set<String> reachableToDead = new HashSet<>(deadNodes);
        Queue<String> queue = new ArrayDeque<>(deadNodes);
        while (!queue.isEmpty()) {
            String curr = queue.poll();
            for (String prev : reverseEdges.getOrDefault(curr, Collections.emptySet())) {
                if (reachableToDead.add(prev)) {
                    queue.add(prev);
                }
            }
        }

        // Retrograde analysis: states that inevitably reach a deadlock (AF deadlock).
        // Non-dead nodes with 0 outgoing edges (e.g. truncated "..." frontier nodes) are not included.
        Map<String, Integer> remainingOut = new HashMap<>();
        for (String u : allNodes) {
            remainingOut.put(u, forwardEdges.getOrDefault(u, Collections.emptySet()).size());
        }
        Set<String> inevitableDead = new HashSet<>(deadNodes);
        Queue<String> inevitableQueue = new ArrayDeque<>(deadNodes);
        while (!inevitableQueue.isEmpty()) {
            String curr = inevitableQueue.poll();
            for (String prev : reverseEdges.getOrDefault(curr, Collections.emptySet())) {
                if (!inevitableDead.contains(prev)) {
                    int rem = remainingOut.get(prev) - 1;
                    remainingOut.put(prev, rem);
                    if (rem == 0) {
                        inevitableDead.add(prev);
                        inevitableQueue.add(prev);
                    }
                }
            }
        }

        try (BufferedWriter bw = new BufferedWriter(new FileWriter(dotFile))) {
            for (String line : lines) {
                Matcher em = EDGE_PATTERN.matcher(line);
                if (em.find()) {
                    String u = em.group(1);
                    String v = em.group(2);
                    if (!reachableToDead.contains(v)) {
                        line = applyColorAttr(line, GREEN_ATTR);
                    } else if (inevitableDead.contains(u) || (inevitableDead.contains(v) && !deadNodes.contains(v))) {
                        line = applyColorAttr(line, ORANGE_ATTR);
                    }
                } else {
                    Matcher nm = NODE_PATTERN.matcher(line);
                    if (nm.find()) {
                        String u = nm.group(1);
                        if (!deadNodes.contains(u)) {
                            if (!reachableToDead.contains(u)) {
                                line = applyColorAttr(line, GREEN_ATTR);
                            } else if (inevitableDead.contains(u)) {
                                line = applyColorAttr(line, ORANGE_ATTR);
                            }
                        }
                    }
                }
                bw.write(line);
                bw.newLine();
            }
        }
    }

    private static String applyColorAttr(String line, String colorAttr) {
        int bracketIdx = line.indexOf('[');
        if (bracketIdx < 0) {
            return line;
        }
        int labelIdx = line.indexOf("label=", bracketIdx);
        int endIdx = (labelIdx >= 0) ? labelIdx : line.lastIndexOf(']');
        if (endIdx < bracketIdx) {
            endIdx = line.length();
        }
        String prefix = line.substring(0, bracketIdx + 1);
        String attrs = line.substring(bracketIdx + 1, endIdx);
        String suffix = line.substring(endIdx);

        attrs = attrs.replaceAll("\\bcolor=(?:gray|dimgray|red)\\s*", "").trim();
        if (attrs.isEmpty()) {
            return prefix + colorAttr + (labelIdx >= 0 ? " " : "") + suffix;
        }
        return prefix + colorAttr + " " + attrs + (labelIdx >= 0 ? " " : "") + suffix;
    }
}
