/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "inputbuf.h"

#define KEYWORDS_COUNT 5
#define DEBUG false

using namespace std;

string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR",
    "REALNUM", "BASE08NUM", "BASE16NUM"
};

string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };

void Debug(char c, string lexme){
    if (DEBUG){
        cout << "char: " << c << " , lexme: " << lexme << endl; 
    }  
}

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

bool LexicalAnalyzer::ScanBase16Num(Token &t)
{
    char c;
    string lexeme = "";
    int start_line_no = line_no;
    vector<char> read_chars;

    input.GetChar(c);
    read_chars.push_back(c);

    // pdigit16
    if ((c >= '1' && c <= '9') || (c >= 'A' && c <= 'F') || c == '0') {
        lexeme += c;

        // digit16
        while (true) {
            input.GetChar(c);
            read_chars.push_back(c);
            if (isdigit(c) || (c >= 'A' && c <= 'F')) {
                lexeme += c;
            } else {
                break;
            }
        }

        // x16
        if (c == 'x') {
            lexeme += c;
            input.GetChar(c);
            read_chars.push_back(c);

            if (c == '1') {
                lexeme += c;
                input.GetChar(c);
                read_chars.push_back(c);

                if (c == '6') {
                    lexeme += c;
                    t.lexeme = lexeme;
                    t.token_type = BASE16NUM;
                    t.line_no = start_line_no;
                    return true;

                } else {
                    // not 6, unget all
                    for (int i = read_chars.size() - 1; i >= 0; i--) {
                        input.UngetChar(read_chars[i]);
                    }
                    return false;
                }

            } else {
                // not 1, unget all
                for (int i = read_chars.size() - 1; i >= 0; i--) {
                    input.UngetChar(read_chars[i]);
                }
                return false;
            }

        } else {
            // not x, unget last char
            input.UngetChar(c);
            // unget all but last one
            for (int i = read_chars.size() - 2; i >= 0; i--) {
                input.UngetChar(read_chars[i]);
            }
            return false;
        }

    } else {
        input.UngetChar(c);
        return false;
    }
}

bool LexicalAnalyzer::ScanBase08Num(Token &t)
{
    char c;
    string lexeme = "";
    int start_line_no = line_no;
    vector<char> read_chars;

    input.GetChar(c);
    read_chars.push_back(c);

    // pdigit8
    if ((c >= '1' && c <= '7') || c == '0') {
        lexeme += c;

        // digit8
        while (true) {
            input.GetChar(c);
            read_chars.push_back(c);
            if (c >= '0' && c <= '7') {
                lexeme += c;
            } else {
                break;
            }
        }

        // x08
        if (c == 'x') {
            lexeme += c;
            input.GetChar(c);
            read_chars.push_back(c);

            if (c == '0') {
                lexeme += c;
                input.GetChar(c);
                read_chars.push_back(c);

                if (c == '8') {
                    lexeme += c;
                    t.lexeme = lexeme;
                    t.token_type = BASE08NUM;
                    t.line_no = start_line_no;
                    return true;

                } else {
                    // not 8, unget all
                    for (int i = read_chars.size() - 1; i >= 0; i--) {
                        input.UngetChar(read_chars[i]);
                    }
                    return false;
                }

            } else {
                // not 0, unget all
                for (int i = read_chars.size() - 1; i >= 0; i--) {
                    input.UngetChar(read_chars[i]);
                }
                return false;
            }

        } else {
            // not x, unget last
            input.UngetChar(c);
            // unget all but last
            for (int i = read_chars.size() - 2; i >= 0; i--) {
                input.UngetChar(read_chars[i]);
            }
            return false;
        }

    } else {
        input.UngetChar(c);
        return false;
    }
}

bool LexicalAnalyzer::ScanRealNum(Token &t)
{
    char c;
    string lexeme = "";
    int start_line_no = line_no;
    vector<char> read_chars;

    input.GetChar(c);
    read_chars.push_back(c);

    if (isdigit(c)) {
        lexeme += c;
        while (true) {
            input.GetChar(c);
            read_chars.push_back(c);
            if (isdigit(c)) {
                lexeme += c;
            } else {
                break;
            }
        }

        if (c == '.') {
            lexeme += c;
            input.GetChar(c);
            read_chars.push_back(c);

            if (isdigit(c)) {
                lexeme += c;
                while (true) {
                    input.GetChar(c);
                    read_chars.push_back(c);
                    if (isdigit(c)) {
                        lexeme += c;
                    } else {
                        break;
                    }
                }
                input.UngetChar(c); // unget last non-digit
                t.lexeme = lexeme;
                t.token_type = REALNUM;
                t.line_no = start_line_no;
                return true;

            } else {
                // unget all characters
                for (int i = read_chars.size() - 1; i >= 0; i--) {
                    input.UngetChar(read_chars[i]);
                }
                return false;
            }

        } else {
            // unget last
            input.UngetChar(c);
            // unget all but last
            for (int i = read_chars.size() - 2; i >= 0; i--) {
                input.UngetChar(read_chars[i]);
            }
            return false;
        }

    } else {
        input.UngetChar(c);
        return false;
    }
}

bool LexicalAnalyzer::ScanNum(Token &t)
{
    char c;
    string lexeme = "";
    int start_line_no = line_no;
    vector<char> read_chars;

    input.GetChar(c);
    read_chars.push_back(c);

    if (isdigit(c)) {
        lexeme += c;
        while (true) {
            input.GetChar(c);
            read_chars.push_back(c);
            if (isdigit(c)) {
                lexeme += c;
            } else {
                break;
            }
        }
        input.UngetChar(c);
        t.lexeme = lexeme;
        t.token_type = NUM;
        t.line_no = start_line_no;
        return true;

    } else {
        input.UngetChar(c);
        return false;
    }
}

bool LexicalAnalyzer::ScanZero(Token &t)
{
    char c;
    string lexeme = "";
    int start_line_no = line_no;
    vector<char> read_chars;

    // first 0
    input.GetChar(c);
    read_chars.push_back(c);
    lexeme += c;

    // check additional
    while (true) {
        input.GetChar(c);
        if (c == '0') {
            read_chars.push_back(c);
            lexeme += c;
        } else {
            break;
        }
    }

    if (c == 'x') {
        // could now be a basenum
        lexeme += c;
        read_chars.push_back(c);
        input.GetChar(c);
        read_chars.push_back(c);

        if (c == '0') {
            lexeme += c;
            input.GetChar(c);
            read_chars.push_back(c);

            if (c == '8') {
                lexeme += c;

                // NUM for zeros
                for (int i = (lexeme.length() - 4) - 1; i >= 0; --i) {
                    Token zeroToken;
                    zeroToken.lexeme = "0";
                    zeroToken.line_no = start_line_no;
                    zeroToken.token_type = NUM;
                    tokens.push_back(zeroToken);
                }

                // base number
                Token baseToken;
                baseToken.lexeme = lexeme.substr(lexeme.length() - 4);
                baseToken.token_type = BASE08NUM;
                baseToken.line_no = start_line_no;
                tokens.push_back(baseToken);

                // first zero
                t = tokens.back();
                tokens.pop_back();
                return true;
            }

        } else if (c == '1') {
            lexeme += c;
            input.GetChar(c);
            read_chars.push_back(c);

            if (c == '6') {
                lexeme += c;

                // NUM for zeros 
                for (int i = (lexeme.length() - 4) - 1; i >= 0; --i) {
                    Token zeroToken;
                    zeroToken.lexeme = "0";
                    zeroToken.line_no = start_line_no;
                    zeroToken.token_type = NUM;
                    tokens.push_back(zeroToken);
                }

                // base number
                Token baseToken;
                baseToken.lexeme = lexeme.substr(lexeme.length() - 4);
                baseToken.token_type = BASE16NUM;
                baseToken.line_no = start_line_no;
                tokens.push_back(baseToken);

                // first zero
                t = tokens.back();
                tokens.pop_back();
                return true;
            }
        }

        // unget all
        for (int i = read_chars.size() - 1; i >= 0; --i) {
            input.UngetChar(read_chars[i]);
        }
        return false;
        
    } else {
        // unget last read
        input.UngetChar(c);

        // NUM for each 0 read
        for (int i = lexeme.length() - 1; i >= 0; --i) {
            Token zeroToken;
            zeroToken.lexeme = "0";
            zeroToken.line_no = start_line_no;
            zeroToken.token_type = NUM;
            tokens.push_back(zeroToken);
        }

        // return first zero back
        t = tokens.back();
        tokens.pop_back();
        return true;
    }
}

Token LexicalAnalyzer::ScanNumber()
{
    // return stored tokens if available
    if (!tokens.empty()) {
        Token tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    Token tmp;
    tmp.lexeme = "";
    tmp.line_no = line_no;

    // peak without consuming
    char c;
    input.GetChar(c);
    input.UngetChar(c);

    if (c == '0') {
        if (ScanZero(tmp)) {
            return tmp;
        }
    } else {
        if (ScanBase16Num(tmp)) {
            return tmp;
        }

        if (ScanBase08Num(tmp)) {
            return tmp;
        }

        if (ScanRealNum(tmp)) {
            return tmp;
        }

        if (ScanNum(tmp)) {
            return tmp;
        }
    }

    // reaching this point means this might be an Id
    input.GetChar(c);
    if (isalpha(c)) {
        input.UngetChar(c);
        return ScanIdOrKeyword();
    } else {
        input.UngetChar(c);
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
