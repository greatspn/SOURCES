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

    public static void export(File file, GspnPage gspn, ParserContext context, Set<String> filenames)
            throws Exception {

        try ( PrintWriter out = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)))) {
            
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
            } else {
                out.println("vector<string> name_file = {};");
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

                        if (trn.isGeneral()) {
                            String[] splitted = cppDelayExpr.split("\n");
                            out.println("   " + splitted[0]);
                            out.println("   double rate = " + splitted[1] + ";");

                        } else {
                            out.println("   double rate = " + cppDelayExpr + ";");
                        }
                        
                        out.println("cout << class_files[BirthPredator].getConstantFromList(0) << \"la a \" << endl;");
                        out.println("cout << class_files[BirthPredator].getConstantFromList(1) << \"la h \" << endl;");
                        out.println("cout << \"RATE CON CUI LAVORO\" << rate << endl;");


                        out.println("   return rate;");
                        out.println("}\n");
                    }

                }
            }

        }
        

    }

}
