%{
#include <iostream>
#include <string>

enum Token_type {
    TOKEN_EOF = 0,
    TOKEN_INT = 1,
    TOKEN_FLOAT = 2,
    TOKEN_IDENT = 3,
    TOKEN_PLUS = 4,
    TOKEN_MINUS = 5,
    TOKEN_MUL = 6,
    TOKEN_DIV = 7,
    TOKEN_ASSIGN = 8,
    TOKEN_LPAREN = 9,
    TOKEN_RPAREN = 10,
    TOKEN_SEMICOLON = 11,
    TOKEN_ERROR = 99
};
%}

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT         {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+

%%

{FLOAT}         { std::cout << "[FLOAT: " << yytext << "]\n"; return TOKEN_FLOAT; }
{INT}           { std::cout << "[INT: " << yytext << "]\n"; return TOKEN_INT; }
{ID}            { std::cout << "[ID: " << yytext << "]\n"; return TOKEN_IDENT; }
"+"             { std::cout << "[PLUS]\n"; return TOKEN_PLUS; }
"-"             { std::cout << "[MINUS]\n"; return TOKEN_MINUS; }
"*"             { std::cout << "[MUL]\n"; return TOKEN_MUL; }
"/"             { std::cout << "[DIV]\n"; return TOKEN_DIV; }
"="             { std::cout << "[ASSIGN]\n"; return TOKEN_ASSIGN; }
"("             { std::cout << "[LPAREN]\n"; return TOKEN_LPAREN; }
")"             { std::cout << "[RPAREN]\n"; return TOKEN_RPAREN; }
";"             { std::cout << "[SEMICOLON]\n"; return TOKEN_SEMICOLON; }
[ \t\r\n]+      { /* ignore whitespace */ }
.               { std::cout << "[ERROR: " << yytext << "]\n"; return TOKEN_ERROR; }

%%

int main() {
    std::string sample = "total = 42 + 3.14159 * (radius - 10);";
    std::cout << "Input expression: " << sample << "\n\nTokens:\n";

    Lexer lexer(sample);
    int tok = 0;
    while ((tok = lexer.yylex()) != TOKEN_EOF) {
        if (tok == TOKEN_ERROR) {
            std::cerr << "Lexical error at line " << lexer.line() << ", col " << lexer.column() << "\n";
            return 1;
        }
    }
    std::cout << "\nLexical analysis completed successfully.\n";
    return 0;
}
