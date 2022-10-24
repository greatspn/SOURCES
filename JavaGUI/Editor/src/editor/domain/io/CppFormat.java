package editor.domain.io;

import editor.domain.Node;
import editor.domain.elements.GspnPage;
import editor.domain.elements.Transition;
import editor.domain.grammar.ExpressionLanguage;
import editor.domain.grammar.ParserContext;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Set;
import java.util.StringJoiner;

/**
 *
 * @author Irene Write che c++ file with the expressions of the transitions
 */
public class CppFormat {

    public static String export(File file, GspnPage gspn, ParserContext context, ArrayList<String> log, Set<String> filenames)
            throws Exception {

        PrintWriter out = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));

        if (!filenames.isEmpty()) {
            StringJoiner joiner = new StringJoiner(",");
            int i = 0;
            for (String name : filenames) {
                out.print("#define ");
                String defineString = name.substring(1, name.length() - 1).replace('.', '_');
                out.println(defineString + " " + i);
                joiner.add(name);
                i++;
            }
            out.println("vector<string> name_file = {" + joiner + "};");
            out.println("vector<Table> class_files(" + Integer.toString(filenames.size()) + ", Table());\n");

        }

        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition) node;

                ArrayList<String> double_constant_log = new ArrayList<>();
                String cppDelayExpr = trn.convertDelayLang(context, null, ExpressionLanguage.CPP);
                GreatSpnFormat.realOrRpar(cppDelayExpr, "", gspn, double_constant_log);
                if (!double_constant_log.isEmpty()) {

                    out.println("double " + trn.getUniqueName() + "_general(double *Value,\n"
                            + "                         map <string,int>& NumTrans,\n"
                            + "                         map <string,int>& NumPlaces,\n"
                            + "                         const vector<string> & NameTrans,\n"
                            + "                         const struct InfTr* Trans,\n"
                            + "                         const int T,\n"
                            + "                         const double& time) {\n");

                    out.println("   double rate = " + cppDelayExpr + ";");
                    out.println("   return rate;");
                    out.println("}\n");
                }

            }
        }


        return reportLog(log, out);

    }

    //close the streams and return eventual problems
    private static String reportLog(ArrayList<String> log, PrintWriter pw) {
        pw.close();
        if (log.isEmpty()) {
            return null; // Everything went ok
        } else {
            String message = "Detected problems while exporting the cpp file.\n\n";
            for (String s : log) {
                message += s + "\n";
            }
            return message;
        }
    }

}
