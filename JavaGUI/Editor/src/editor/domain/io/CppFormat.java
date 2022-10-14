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

/**
 *
 * @author Irene 
 * Write che c++ file with the expressions of the
 * transitions
 */
public class CppFormat {

    public static String export(File file, GspnPage gspn, ParserContext context)
            throws Exception {

        ArrayList<String> log = new ArrayList<>();

        PrintWriter out = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));
        
        
        for (Node node : gspn.nodes) {
            if (node instanceof Transition) {
                Transition trn = (Transition) node;

                String cppDelayExpr = trn.convertDelayLang(context, null, ExpressionLanguage.CPP);

                out.println("double " + trn.getUniqueName() + "_general(double *Value,\n"
                        + "                         map <string,int>& NumTrans,\n"
                        + "                         map <string,int>& NumPlaces,\n"
                        + "                         const vector<string> & NameTrans,\n"
                        + "                         const struct InfTr* Trans,\n"
                        + "                         const int T,\n"
                        + "                         const double& time) {\n");

                //out.println("double const = read_constant(\"./" + trn.getUniqueName() + "\", a);\n");



                out.println("   double rate = " + cppDelayExpr + ";");
                out.println("   return rate;");
                out.println("}\n");

            }
        }

        out.println("\n");

        //al log aggiungo messaggi se trovo degli errori durante la stampa.
        //se sarà vuoto tutto bene, se conterrà delle frasi allora ci saranno
        //stati errori e farò return con relativo errore
        return reportLog(log, out);

    }

    //chiude lo stream e ritorna problemi o nulla
    private static String reportLog(ArrayList<String> log, PrintWriter pw) {
        pw.close();
        if (log.isEmpty()) {
            return null; // Everything went ok
        } else {
            String message = "Detected problems while exporting the NetLogo model.\n\n";
            for (String s : log) {
                message += s + "\n";
            }
            return message;
        }
    }

}
