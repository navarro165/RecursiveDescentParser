#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <map>

#include "lexer.h"
#include "inputbuf.h"

#define KEYWORDS_COUNT 2
#define DEBUG false

using namespace std;

string reserved[] = { "END_OF_FILE",
    "PUBLIC", "PRIVATE",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRACE", "RBRACE", "ID", "ERROR"
};

string keyword[] = { "public", "private" };

void Debug(char c, string lexeme){
    if (DEBUG){
        cout << "char: " << c << " , lexeme: " << lexeme << endl;
    }
}

void Token::Print(){
    cout << "{" << this->lexeme << " , "
         << reserved[(int)this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer(){
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace(){
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

bool LexicalAnalyzer::SkipComments() {
    char c;
    input.GetChar(c);
    if (c == '/') {
        input.GetChar(c);
        if (c == '/') {
            while (c != '\n' && !input.EndOfInput()) {
                input.GetChar(c);
            }
            line_no++;
            return true;
        } else {
            input.UngetChar(c);
            input.UngetChar('/');
            return false;
        }
    } else {
        input.UngetChar(c);
        return false;
    }
}

bool LexicalAnalyzer::IsKeyword(string s) {
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s) {
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType)(i + 1);
        }
    }
    return ID;
}

Token LexicalAnalyzer::ScanIdOrKeyword(){
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

TokenType LexicalAnalyzer::UngetToken(Token tok){
    tokens.push_back(tok);
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken(){
    char c;

    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    while (SkipComments()) {
        SkipSpace();
    }

    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
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
        case '{':
            tmp.token_type = LBRACE;
            return tmp;
        case '}':
            tmp.token_type = RBRACE;
            return tmp;
        default:
            if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput()) {
                tmp.token_type = END_OF_FILE;
            } else {
                tmp.token_type = ERROR;
            }
            return tmp;
    }
}

struct VariableInfo {
    string name;
    string scope;
    string access_type; // if public or private
};

LexicalAnalyzer lexer;
Token token;

vector<VariableInfo> symbolTable;
vector<string> scopeStack;
vector<string> assignments;

void ParseProgram();
void ParseGlobalVariables();
void ParseVarDeclList(string access_type);
void ParseVariableList(string access_type);
void ParseScopeList();
void ParseScope();
void ParseScopeContent();
void ParseVisibilitySections();
void ParseVisibilitySection();
void ParseVariableListWithVisibility(string access_type);
void ParseAssignmentList();
void ParseAssignment();
string ResolveVariable(string variableName);

void ParseProgram() {
    ParseGlobalVariables();
    ParseScopeList();
}

void ParseGlobalVariables() {
    ParseVarDeclList("public");
}

void ParseVarDeclList(string access_type) {
    ParseVariableList(access_type);
    token = lexer.GetToken();
    if (token.token_type != SEMICOLON) {
        cerr << "Syntax Error: Expected ';' at line " << token.line_no << endl;
        exit(1);
    }
}

void ParseVariableList(string access_type) {
    token = lexer.GetToken();
    if (token.token_type == ID) {
        // add to symbol table
        VariableInfo variableInfo;
        variableInfo.name = token.lexeme;
        variableInfo.scope = (scopeStack.empty() ? "::" : scopeStack.back());
        variableInfo.access_type = access_type;
        symbolTable.push_back(variableInfo);
        token = lexer.GetToken();

        if (token.token_type == COMMA) {
            ParseVariableList(access_type);
        } else {
            lexer.UngetToken(token);
        }
    } else {
        cerr << "Syntax Error: Expected ID at line " << token.line_no << endl;
        exit(1);
    }
}

void ParseScopeList() {
    while (true) {
        token = lexer.GetToken();
        if (token.token_type == ID) {
            Token nextToken = lexer.GetToken();
            if (nextToken.token_type == LBRACE) {
                lexer.UngetToken(nextToken);
                lexer.UngetToken(token);
                ParseScope();
            } else {
                lexer.UngetToken(nextToken);
                lexer.UngetToken(token);
                break; // not a scope
            }
        } else {
            lexer.UngetToken(token);
            break; // out of scopes
        }
    }
}


void ParseScope() {
    token = lexer.GetToken();
    if (token.token_type == ID) {
        string scopeName = token.lexeme;
        scopeStack.push_back(scopeName);
        token = lexer.GetToken();
        if (token.token_type == LBRACE) {
            ParseScopeContent();
            token = lexer.GetToken();
            if (token.token_type != RBRACE) {
                cerr << "Syntax Error: Expected '}' at line " << token.line_no << endl;
                exit(1);
            }
            scopeStack.pop_back();
        } else {
            cerr << "Syntax Error: Expected '{' at line " << token.line_no << endl;
            exit(1);
        }
    } else {
        cerr << "Syntax Error: Expected ID at line " << token.line_no << endl;
        exit(1);
    }
}

void ParseScopeContent() {
    ParseVisibilitySections();
    ParseScopeList();
    ParseAssignmentList();
}


void ParseVisibilitySections() {
    token = lexer.GetToken();
    while (token.token_type == PUBLIC || token.token_type == PRIVATE) {
        lexer.UngetToken(token);
        ParseVisibilitySection();
        token = lexer.GetToken();
    }
    lexer.UngetToken(token);
}

void ParseVisibilitySection() {
    token = lexer.GetToken();
    string access_type;
    if (token.token_type == PUBLIC) {
        access_type = "public";
    } else if (token.token_type == PRIVATE) {
        access_type = "private";
    } else {
        cerr << "Syntax Error: Expected 'public' or 'private' at line " << token.line_no << endl;
        exit(1);
    }

    token = lexer.GetToken();
    if (token.token_type == COLON) {
        ParseVariableListWithVisibility(access_type);
        token = lexer.GetToken();
        if (token.token_type != SEMICOLON) {
            cerr << "Syntax Error: Expected ';' at line " << token.line_no << endl;
            exit(1);
        }
    } else {
        cerr << "Syntax Error: Expected ':' at line " << token.line_no << endl;
        exit(1);
    }
}

void ParseVariableListWithVisibility(string access_type) {
    token = lexer.GetToken();
    if (token.token_type == ID) {
        // add to symbol table
        VariableInfo variableInfo;
        variableInfo.name = token.lexeme;
        variableInfo.scope = (scopeStack.empty() ? "::" : scopeStack.back());
        variableInfo.access_type = access_type;
        
        symbolTable.push_back(variableInfo);
        token = lexer.GetToken();

        if (token.token_type == COMMA) {
            ParseVariableListWithVisibility(access_type);
        } else {
            lexer.UngetToken(token);
        }
    } else {
        cerr << "Syntax Error: Expected ID at line " << token.line_no << endl;
        exit(1);
    }
}

void ParseAssignmentList() {
    while (true) {
        token = lexer.GetToken();
        if (token.token_type == ID) {
            Token nextToken = lexer.GetToken();
            if (nextToken.token_type == EQUAL) {
                lexer.UngetToken(nextToken);
                lexer.UngetToken(token);
                ParseAssignment();
            } else {
                lexer.UngetToken(nextToken);
                lexer.UngetToken(token);
                break; // not an assignment
            }
        } else {
            lexer.UngetToken(token);
            break; // out of assignments
        }
    }
}


void ParseAssignment() {
    token = lexer.GetToken();
    if (token.token_type == ID) {
        string lhs = token.lexeme;
        token = lexer.GetToken();
        if (token.token_type == EQUAL) {
            token = lexer.GetToken();
            if (token.token_type == ID) {
                string rhs = token.lexeme;
                token = lexer.GetToken();
                if (token.token_type == SEMICOLON) {
                    string resolved_lhs = ResolveVariable(lhs);
                    string resolved_rhs = ResolveVariable(rhs);
                    assignments.push_back(resolved_lhs + " = " + resolved_rhs);
                } else {
                    cerr << "Syntax Error: Expected ';' at line " << token.line_no << endl;
                    exit(1);
                }
            } else {
                cerr << "Syntax Error: Expected ID at line " << token.line_no << endl;
                exit(1);
            }
        } else {
            cerr << "Syntax Error: Expected '=' at line " << token.line_no << endl;
            exit(1);
        }
    } else {
        cerr << "Syntax Error: Expected ID at line " << token.line_no << endl;
        exit(1);
    }
}


string ResolveVariable(string variableName) {
    
    // search in all scopes
    auto scopeIt = scopeStack.rbegin();
    for (; scopeIt != scopeStack.rend(); ++scopeIt) {
        string scopeName = *scopeIt;

        // search in scope
        for (VariableInfo &variableInfo : symbolTable) {
            if (variableInfo.name == variableName && variableInfo.scope == scopeName) {
                if (scopeIt == scopeStack.rbegin()) {
                    // can access in current scope
                    return scopeName + "." + variableName;
                } else if (variableInfo.access_type == "public") {
                    // can access variagble in outer scope, public
                    return scopeName + "." + variableName;
                } else {
                    // cant access variable from outer scope, private
                    // keep searching outer scopes
                    break;
                }
            }
        }
    }

    // search global scope
    for (VariableInfo &variableInfo : symbolTable) {
        if (variableInfo.name == variableName && variableInfo.scope == "::") {
            return "::" + variableName;
        }
    }

    // cant find variable
    return "?." + variableName;
}

int main() {
    ParseProgram();
    for (const string &assignment : assignments) {
        cout << assignment << endl;
    }
    return 0;
}
