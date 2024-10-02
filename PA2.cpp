//Jason Tencio
//Spring 24 semester
//PA 2
// 5 April 2024

#include <iostream>
#include "parser.h"
#include <string>
#include <map>

using namespace std;

map<string, Token> SymTable;
map<string, bool> defVar;
bool defOfVar = false;
bool varOftheTemp;
static int error_count = 0;
string s;

int valVlaue = 1;

namespace Parser {
    bool pushed_back = false;
    LexItem pushed_token;

    static LexItem GetNextToken(istream& in, int& line){
        if( pushed_back ){
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem & t){
        if( pushed_back ) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }
}

int ErrCount(){
    return error_count;
}

void ParseError(int line, string msg){
    ++error_count;
    cout << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line) {
    LexItem tokenToken = Parser::GetNextToken(in, line);
    switch (tokenToken.GetToken()) {
        case PROGRAM:
            break;
        default:
            ParseError(line, "Expected PROGRAM");
            return false;
    }

    tokenToken = Parser::GetNextToken(in, line);
    switch (tokenToken.GetToken()) {
        case IDENT:
            break;
        default:
            ParseError(line, "Expected IDENT after PROGRAM Prog");
            return false;
    }
    while (true) {
        bool booleAN = Decl(in, line);
        if (!booleAN) {
            break;
        }
    }
    while (true) {
        bool booleAN = Stmt(in, line);
        if (!booleAN) {
            break;
        }
    }
    tokenToken = Parser::GetNextToken(in, line);
    switch (tokenToken.GetToken()) {
        case END:
            break;
        default:
            ParseError(line, "Expected 'END' keyword");
            return false;
    }
    tokenToken = Parser::GetNextToken(in, line);
    switch (tokenToken.GetToken()) {
        case PROGRAM:
            break;
        default:
            ParseError(line, "Expected 'PROGRAM' keyword after 'END'");
            return false;
    }

    tokenToken = Parser::GetNextToken(in, line);
    switch (tokenToken.GetToken()) {
        case IDENT:
            break;
        default:
            ParseError(line, "Expected identifier after 'END PROGRAM'");
            return false;
    }
    cout << "(DONE)" << endl;
    return true;
}
bool PrintStmt(istream& in, int& line){
    LexItem petDog;

    petDog = Parser::GetNextToken(in, line);
    if (petDog != PRINT){
        Parser::PushBackToken(petDog);
        return false;
    }
    petDog = Parser::GetNextToken(in, line);
    if(petDog != DEF){
        Parser::PushBackToken(petDog);
        ParseError(line, "Syntax error in print statement.");
        return false;
    }
    petDog = Parser::GetNextToken(in, line);
    if(petDog != COMMA){
        Parser::PushBackToken(petDog);
        ParseError(line, "Syntax error due to missing comma.");
        return false;
    }
    bool expre = ExprList(in, line);
    if(!expre) {
        ParseError(line, "Missing or incomplete expression after Print Statement");
        return false;
    }
    return expre;
}
bool VarList(istream& in, int& line) {
    LexItem tokEN = Parser::GetNextToken(in, line);
    string d = tokEN.GetLexeme();
    Parser::PushBackToken(tokEN);
    bool booleanOfRandom = Var(in, line);
    if (!booleanOfRandom) {
        ParseError(line, "Missing var expression");
        return false;
    }
    if (defOfVar) {
        ParseError(line, "Variable Redefinition");
        return false;
    }
    tokEN = Parser::GetNextToken(in, line);
    switch (tokEN.GetToken()) {
        case COMMA:
            booleanOfRandom = VarList(in, line);
            if (!booleanOfRandom) {
                ParseError(line, "Variable expected after comma");
                return false;
            }
            break;
        case ASSOP:
            booleanOfRandom = Expr(in, line);
            cout << "Initialization of the variable " << d << " in the declaration statement." << endl;
            if (varOftheTemp) {
                cout << "Definition of Strings with length of " << s << " in declaration statement." << endl;
            }
            if (!booleanOfRandom) {
                ParseError(line, "Error in expression");
                return false;
            }
            tokEN = Parser::GetNextToken(in, line);
            if (tokEN == COMMA) {
                booleanOfRandom = VarList(in, line);
                if (!booleanOfRandom) {
                    ParseError(line, "Something went wrong!");
                    return false;
                }
                return true;
            }
            Parser::PushBackToken(tokEN);
            break;
        default:
            Parser::PushBackToken(tokEN);
            break;
    }
    return booleanOfRandom;
}

bool Stmt(istream& in, int& line){
    bool booleanOfFirst = false;
    booleanOfFirst = AssignStmt(in, line);
    if (!booleanOfFirst){
        bool booleanOfSecond = false;
        booleanOfSecond = BlockIfStmt(in, line);
        if (!booleanOfSecond){
            bool booleanOfThird = false;
            booleanOfThird = PrintStmt(in, line);
            if (!booleanOfThird){
                return false;
            }
        }
    }
    return true;
}

bool Decl(istream& in, int& line){
    bool boolean = Type(in, line);
    if (!boolean) {
        return false;
    }
    LexItem tokEN = Parser::GetNextToken(in, line);
    if (tokEN == DCOLON){
        bool sBooleaN = VarList(in, line);
        if (!sBooleaN){
            ParseError(line, "Missing Expression from DCOLON");
            return false;
        }
        return true;
    }
    else{
        Parser::PushBackToken(tokEN);
        return false;
    }
}
bool ExprList(istream& in, int& line){
    bool doggo;

    doggo = Expr(in, line);
    if(!doggo){
        ParseError(line, "Missing Expression(s)");
        return false;
    }

    LexItem tokenORandom = Parser::GetNextToken(in, line);
    if (tokenORandom == COMMA){
        doggo = ExprList(in, line);
    }
    else if(tokenORandom.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tokenORandom.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tokenORandom);
        return true;
    }
    return doggo;
}
bool Type(istream& in, int& line) {
    LexItem tokeToke = Parser::GetNextToken(in, line);
    switch (tokeToke.GetToken()) {
        case INTEGER:
        case REAL:
            return true;
        case CHARACTER:
            tokeToke = Parser::GetNextToken(in, line);
            if (tokeToke != LPAREN) {
                Parser::PushBackToken(tokeToke);
                return true;
            }
            tokeToke = Parser::GetNextToken(in, line);
            if (tokeToke != LEN) {
                ParseError(line, "Missing Expression in LEN");
                return false;
            }
            tokeToke = Parser::GetNextToken(in, line);
            if (tokeToke != ASSOP) {
                ParseError(line, "Missing Assignment Operator in LEN");
                return false;
            }
            tokeToke = Parser::GetNextToken(in, line);
            if (tokeToke != ICONST) {
                ParseError(line, "Missing Integer Constant in LEN");
                return false;
            }
            s = tokeToke.GetLexeme();
            tokeToke = Parser::GetNextToken(in, line);
            if (tokeToke != RPAREN) {
                ParseError(line, "Missing Right Parenthesis in LEN");
                return false;
            }
            varOftheTemp = true;
            return true;
        default:
            Parser::PushBackToken(tokeToke);
            return false;
    }
}

bool BlockIfStmt(istream& in, int& line){
    LexItem tokTok = Parser::GetNextToken(in, line);
    if (tokTok != IF){
        Parser::PushBackToken(tokTok);
        return false;
    }
    tokTok = Parser::GetNextToken(in, line);
    if (tokTok != LPAREN){
        Parser::PushBackToken(tokTok);
        ParseError(line, "Expected '(' after 'IF' statement");
        return false;
    }

    bool oneBooleanF = false;
    oneBooleanF = RelExpr(in, line);
    if(!oneBooleanF){
        ParseError(line, "Missing expression in 'IF' statement");
        return false;
    }

    tokTok = Parser::GetNextToken(in, line);
    if(tokTok != RPAREN){
        Parser::PushBackToken(tokTok);
        ParseError(line, "Expected ')' after expression in the 'IF' block");
        return false;
    }

    tokTok = Parser::GetNextToken(in, line);
    if (tokTok != THEN){
        cout << "Print statement in a Simple If statement." << endl;
        Parser::PushBackToken(tokTok);
        return true;
    }

    valVlaue++;
    
    oneBooleanF = BlockIfStmt(in, line);

    valVlaue--;
    while(true){
        oneBooleanF = Stmt(in, line);
        tokTok = Parser::GetNextToken(in, line);
        if (tokTok == END){
            LexItem nextToken = Parser::GetNextToken(in, line);
            if (nextToken == IF) {
                cout << "End of Block If statement at nesting level " << valVlaue << endl;
                break;
            }
            else{
                Parser::PushBackToken(nextToken);
                return false;
            }
        }
        else if(tokTok == ELSE){
            while(true){
                oneBooleanF = Stmt(in, line);
                tokTok = Parser::GetNextToken(in, line);
                if (tokTok == END){
                    LexItem nextToken = Parser::GetNextToken(in, line);
                    if (nextToken == IF){
                        cout << "End of Block If statement at nesting level " << valVlaue << endl;
                        break;
                    }
                    else{
                        Parser::PushBackToken(nextToken);
                        ParseError(line, "After 'IF' keyword 'END' is expected");
                        break;
                    }
                }
                Parser::PushBackToken(tokTok);
                if (!oneBooleanF) break;
            }
        }
        Parser::PushBackToken(tokTok);
        if (!oneBooleanF) break;
    }
    return oneBooleanF;
}
bool Expr(istream& in, int& line) {
    bool booleAN = MultExpr(in, line);
    if (!booleAN) {
        ParseError(line, "Syntax Error: Missing Expression in expr");
        return false;
    }

    while (true) {
        LexItem tokenOfWorthy = Parser::GetNextToken(in, line);
        switch (tokenOfWorthy.GetToken()) {
            case MINUS:
            case PLUS:
            case CAT:
                if (!Expr(in, line)) {
                    ParseError(line, "Syntax Error: Missing Expression after operator");
                    return false;
                }
                break;
            default:
                Parser::PushBackToken(tokenOfWorthy);
                return true;
        }
    }
}

bool Var(istream& in, int& line) {
    LexItem tokeToken = Parser::GetNextToken(in, line);
    switch (tokeToken.GetToken()) {
        case IDENT:
            if (defVar.find(tokeToken.GetLexeme()) != defVar.end()) {
                defOfVar = true;
            }
            defVar[tokeToken.GetLexeme()] = true;
            return true;
        default:
            Parser::PushBackToken(tokeToken);
            return false;
    }
}

bool RelExpr(istream& in, int& line) {
    bool booleAN = false;
    booleAN = Expr(in, line);
    if (!booleAN) {
        ParseError(line, "Missing Expression within the relexpr expression.");
        return false;
    }

    LexItem tokeToke = Parser::GetNextToken(in, line);
    switch (tokeToke.GetToken()) {
        case EQ:
        case LTHAN:
        case GTHAN: {
            bool anotherOne = false;
            anotherOne = Expr(in, line);
            if (!anotherOne) {
                ParseError(line, "Missing Expression in again relexpr statement.");
                return false;
            }
            return true;
        }
        default:
            Parser::PushBackToken(tokeToke);
            return booleAN;
    }
}

bool AssignStmt(istream& in, int& line) {
    if (!Var(in, line)) {
        return false;
    }
    LexItem tokeTok = Parser::GetNextToken(in, line);
    if (tokeTok != ASSOP){
        ParseError(line, "No Assignment Operator");
        return false;
    }
    if (!Expr(in, line)){
        ParseError(line, "Invalid expression in assignment statement");
        return false;
    }
    return true;
}

bool MultExpr(istream& in, int& line) {
    bool booleAN = TermExpr(in, line); 
    if (!booleAN){
        ParseError(line, "Syntax Error: Missing Expression in multexpr expression.");
        return false;
    }

    LexItem time = Parser::GetNextToken(in, line);
    if (time == DIV || time == MULT) {
        return MultExpr(in, line);
    } else {
        Parser::PushBackToken(time);
        return true;
    }
}

bool TermExpr(istream& in, int& line) {
    bool termOfTheCurrent = SFactor(in, line);
    if (!termOfTheCurrent) {
        ParseError(line, "Syntax Error: Missing Expression in termExpr");
        return false;
    }
    LexItem tokeToken = Parser::GetNextToken(in, line);
    if(tokeToken == POW){
        if(!TermExpr(in, line)){
            ParseError(line, "Syntax Error: Missing Expression after pow statement");
            return false;
        }
    } else {
        Parser::PushBackToken(tokeToken);
    }
    return termOfTheCurrent;
}

bool Factor(istream& in, int& line, int sign) {
    bool booleAN;
    LexItem tokeTok = Parser::GetNextToken(in, line);
    if (tokeTok == MINUS) {
        return Factor(in, line, -sign);
    }
    if (tokeTok == IDENT) {
        auto it = defVar.find(tokeTok.GetLexeme());
        if (it != defVar.end()) {
            return true;
        }
        ParseError(line, "Error: Use of an undefined variable");
        return false;
    }
    if (tokeTok == RCONST || tokeTok == SCONST || tokeTok == ICONST) {
        return true;
    }
    if (tokeTok == LPAREN) {
        booleAN = Expr(in, line);
        if (!booleAN) {
            ParseError(line, "Error: Missing left LPAREN in statment");
            return false;
        }
        tokeTok = Parser::GetNextToken(in, line);
        if (tokeTok == RPAREN) {
            return true;
        }
    }
    else {
        Parser::PushBackToken(tokeTok);
    }

    return false;
}

bool SFactor(istream& in, int& line) {
    return Factor(in, line, 1);
}
