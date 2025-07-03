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
import java.nio.charset.StandardCharsets;

import static common.Util.loadTextDoc;
/**
 *
 * @author Irene 
 * Write che c++ file with the expressions of the transitions
 */
 
public class CppFormat {

    public static void export(File file, GspnPage gspn, boolean fluxBalance, ParserContext context, Set<String> filenames, Set<String> rnames, String baseName) throws Exception {
        try (PrintWriter out = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)))) {
        
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

            if(fluxBalance){
							if (baseName.endsWith("_unf")) {
									baseName = baseName.substring(0, baseName.length() - "_unf".length());
							}
							String fbaPath = "./" + baseName + ".fbainfo";
							out.println(generateFBAClass(fbaPath));    
           	}
           		
            for (Node node : gspn.nodes) {
                if (node instanceof Transition) {
                    Transition trn = (Transition) node;
                    ArrayList<String> double_constant_log = new ArrayList<>();
                    String cppDelayExpr = trn.convertDelayLang(context, null, ExpressionLanguage.CPP);
                    GreatSpnFormat.realOrRpar(cppDelayExpr, "", gspn, double_constant_log);
                    if (!double_constant_log.isEmpty()) {
                        out.println(generateFunction(trn, cppDelayExpr, fluxBalance));
                    }
                }
            }
            
        }
    }

    private static String generateFunction(Transition trn, String cppDelayExpr, boolean fluxBalance) {
        StringBuilder sb = new StringBuilder();
        sb.append("double ").append(trn.getUniqueName()).append("_general(double *Value, ");
        if (fluxBalance) {
            sb.append("vector<class FBGLPK::LPprob>& vec_fluxb, ");
        }
        sb.append("map <string,int>& NumTrans, ");
        sb.append("map <string,int>& NumPlaces, ");
        sb.append("const vector<string> & NameTrans, ");
        sb.append("const struct InfTr* Trans, ");
        sb.append("const int T, ");
        sb.append("const double& time) {\n");
        sb.append("    double rate = ").append(cppDelayExpr).append(";\n");
        sb.append("    return (rate);\n");
        sb.append("}\n");
        return sb.toString();
    }
    
    

	/**
	 * @Author: Chiabrando
	 *
	 *
	 * Generates the complete class definition for FBAProcessor in C++ syntax.
	 *
	 * @return A string containing the formatted class definition for FBAProcessor, ready to be used in C++ code.
	 */

		private static String generateFBAClass(String fbaInfoFilePath) {

				final String TEMPLATE_PATH = "/templates/FBAprocessor.cpp";

				String cpp = common.Util.loadTextDoc(
				                 TEMPLATE_PATH,
				                 StandardCharsets.UTF_8.name());

				cpp = cpp.replace("${FBA_INFO_FILE}", fbaInfoFilePath);

				return cpp;
		}

}
