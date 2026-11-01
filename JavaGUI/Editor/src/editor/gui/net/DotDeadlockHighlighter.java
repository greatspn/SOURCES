package editor.gui.net;

import java.io.*;
import java.util.*;
import java.util.regex.*;

public class DotDeadlockHighlighter {

    public static void process(File dotFile) throws IOException {
        List<String> lines = new ArrayList<>();
        try (BufferedReader br = new BufferedReader(new FileReader(dotFile))) {
            String line;
            while ((line = br.readLine()) != null) {
                lines.add(line);
            }
        }

        Map<String, List<String>> reverseEdges = new HashMap<>();
        Set<String> allNodes = new HashSet<>();
        Set<String> deadNodes = new HashSet<>();

        Pattern edgePattern = Pattern.compile("^\\s*([A-Z]\\d+)\\s*->\\s*([A-Z]\\d+)\\s*\\[");
        Pattern nodePattern = Pattern.compile("^\\s*([A-Z]\\d+)\\s*\\[");

        for (String line : lines) {
            Matcher em = edgePattern.matcher(line);
            if (em.find()) {
                String u = em.group(1);
                String v = em.group(2);
                reverseEdges.computeIfAbsent(v, k -> new ArrayList<>()).add(u);
                allNodes.add(u);
                allNodes.add(v);
            } else {
                Matcher nm = nodePattern.matcher(line);
                if (nm.find()) {
                    String u = nm.group(1);
                    allNodes.add(u);
                    if (u.startsWith("D")) {
                        deadNodes.add(u);
                    }
                }
            }
        }

        Set<String> reachableToDead = new HashSet<>(deadNodes);
        Queue<String> queue = new LinkedList<>(deadNodes);
        while (!queue.isEmpty()) {
            String curr = queue.poll();
            List<String> rev = reverseEdges.getOrDefault(curr, Collections.emptyList());
            for (String prev : rev) {
                if (reachableToDead.add(prev)) {
                    queue.add(prev);
                }
            }
        }

        try (BufferedWriter bw = new BufferedWriter(new FileWriter(dotFile))) {
            for (String line : lines) {
                Matcher em = edgePattern.matcher(line);
                if (em.find()) {
                    String u = em.group(1);
                    String v = em.group(2);
                    if (!reachableToDead.contains(u) || !reachableToDead.contains(v)) {
                        // Append to the end of the edge attributes, replacing ];
                        line = line.replace("];", " color=\"#b0b0b040\" fontcolor=\"#b0b0b0\"];");
                        // Also remove any existing color
                        line = line.replace("color=dimgray", "");
                        line = line.replace("color=red", "");
                    }
                } else {
                    Matcher nm = nodePattern.matcher(line);
                    if (nm.find()) {
                        String u = nm.group(1);
                        if (!reachableToDead.contains(u)) {
                            // Replace color=... in the node definition
                            line = line.replace("color=gray", "color=\"#b0b0b040\" fontcolor=\"#b0b0b0\"");
                            line = line.replace("color=red", "color=\"#b0b0b040\" fontcolor=\"#b0b0b0\"");
                            // Also if it had shape=none, let's keep it but just in case
                            line = line.replaceFirst("\\[(?!(.*color=))", "[color=\"#b0b0b040\" fontcolor=\"#b0b0b0\" ");
                        }
                    }
                }
                bw.write(line);
                bw.newLine();
            }
        }
    }
}
