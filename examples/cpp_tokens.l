%{
#include <iostream>
#include <string>

enum Cpp_token {
    TOK_EOF = 0,
    TOK_KEYWORD,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING_LITERAL,
    TOK_OPERATOR,
    TOK_PUNCTUATOR,
    TOK_COMMENT,
    TOK_UNKNOWN
};
%}

DIGIT       [0-9]
HEXDIGIT    [0-9a-fA-F]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT_LIT     ({DIGIT}+|"0x"{HEXDIGIT}+)

%%

"class"|"struct"|"namespace"|"return"|"if"|"else"|"while"|"for"|"int"|"double"|"bool" {
    std::cout << "KEYWORD: " << yytext << "\n";
    return TOK_KEYWORD;
}

"::"|"->"|"=="|"!="|"<="|">="|"&&"|"||"|"<<"|">>"|"\+\+"|"--" {
    std::cout << "OPERATOR: " << yytext << "\n";
    return TOK_OPERATOR;
}

"+"|"-"|"*"|"/"|"%"|"="|"<"|">"|"!"|"&"|"|"|"^"|"~" {
    std::cout << "OPERATOR: " << yytext << "\n";
    return TOK_OPERATOR;
}

";"|","|"{"|"}"|"("|")"|"["|"]" {
    std::cout << "PUNCTUATOR: " << yytext << "\n";
    return TOK_PUNCTUATOR;
}

{INT_LIT} {
    std::cout << "NUMBER: " << yytext << "\n";
    return TOK_NUMBER;
}

{ID} {
    std::cout << "IDENTIFIER: " << yytext << "\n";
    return TOK_IDENTIFIER;
}

"//"[^\n]* {
    std::cout << "LINE_COMMENT: " << yytext << "\n";
    return TOK_COMMENT;
}

[ \t\r\n]+ {
    // Skip whitespace
}

. {
    std::cout << "UNKNOWN: " << yytext << "\n";
    return TOK_UNKNOWN;
}

%%

int main() {
    std::string code = 
        "// Sample C++ snippet\n"
        "namespace math {\n"
        "    struct Point {\n"
        "        int x = 0x2A;\n"
        "        int y = 100;\n"
        "    };\n"
        "}\n"
        "if (p->x >= 42 && p->y != 0) {\n"
        "    return true;\n"
        "}\n";

    std::cout << "Source Code to Tokenize:\n" << code << "\nTokens:\n";

    Lexer lexer(code);
    int tok = 0;
    while ((tok = lexer.yylex()) != TOK_EOF) {
        // tokens printed in actions
    }

    return 0;
}
