/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.domain.io;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *
 * @author elvio
 */
public class SimpleLexer {
    
    public static enum TokenType {
        // Token types cannot have underscores
        REAL("-?[0-9]*\\.[0-9]+([eE][-+][0-9]+?)?"),
        INT("-?[0-9]+"), 
//        KW("All|is|in|d|Card|S"), // keywords, must appear before ID
        ID("[a-zA-Z_][a-zA-Z0-9_]*"),
        OP("\\+\\+|\\-\\-|<=|>=|==|/=|<>|!=|!in|&&|\\|\\||[-(){}\\[\\]\\|\\^*/+&~!#<>=\\.\\:\\;\\,\\?]"), 
        WS("[ \t\f\r\n]+"), // whitespace
        ANY("."); // must be the last rule

        public final String pattern;

        private TokenType(String pattern) {
            this.pattern = pattern;
        }
    }
    
    public static class Token {
        public TokenType type;
        public String data;
        public String whitespacesAfter;

        public Token(TokenType type, String data) {
            this.type = type;
            this.data = data;
            this.whitespacesAfter = "";
        }
        
        public boolean is(TokenType type, String data) {
            return this.type == type && this.data.equals(data);
        }
        
        public void clear() {
            data = whitespacesAfter = "";
        }

        @Override
        public String toString() {
            return String.format("(%s %s '%s')", type.name(), data, whitespacesAfter);
        }
    }
    
    private static final String PATTERN_STR;
    private static final Pattern tokenPatterns;
    static {
        StringBuilder tokenPatternsBuffer = new StringBuilder();
        for (TokenType tokenType : TokenType.values())
            tokenPatternsBuffer.append(tokenPatternsBuffer.length()==0 ? "" : "|")
                    .append(String.format("(?<%s>%s)", tokenType.name(), tokenType.pattern));
        PATTERN_STR = tokenPatternsBuffer.toString();
//        System.out.println("patternStr = "+PATTERN_STR);
        tokenPatterns = Pattern.compile(PATTERN_STR);
    }

    public static ArrayList<Token> lex(String input) {
        // The tokens to return
        ArrayList<Token> tokens = new ArrayList<>();

        // Begin matching tokens
        Matcher matcher = tokenPatterns.matcher(input);
        while (matcher.find()) {
            String match = matcher.group(TokenType.WS.name());
            if (match != null) {
                if (tokens.size() > 0) {
                    tokens.get(tokens.size() - 1).whitespacesAfter = match;
                }
                continue;
            }
            for (TokenType tt : TokenType.values()) {
                match = matcher.group(tt.name());
                if (match != null) {
                    tokens.add(new Token(tt, match));
                    break;
                }
            }
        }

        return tokens;
    }
    
    public static String unlex(ArrayList<Token> input) {
        StringBuilder sb = new StringBuilder();
        for (Token tk : input)
            sb.append(tk.data).append(tk.whitespacesAfter);
        return sb.toString();
    }
    
    public static boolean switchParen(ArrayList<Token> input, int pos, String from, String to) {
        assert input.get(pos).data.charAt(0) == from.charAt(0);
        
        int nest = 0;
        for (int i=pos + 1; i<input.size(); i++) {
            if (input.get(i).data.charAt(0) == from.charAt(0))
                nest++;
            else if (input.get(i).data.charAt(0) == from.charAt(1)) {
                if (nest > 0)
                    nest--;
                else {
                    input.get(pos).data = ""+to.charAt(0);
                    input.get(i).data = ""+to.charAt(1);
                    return true;
                }
            }
        }
        return false; // nothing done
    }

    public static void main(String[] args) {
        String input = "11 ++ 2.2 -- # 33.5e-4~! && in !in || + (AS)}[[ .4]} .4e-4 != >= > <> >=% - 89 -4.5, e,f?";
        
        // Create tokens and print them
        ArrayList<Token> tokens = lex(input);
        for (Token token : tokens)
            System.out.println(token);
    }
}
