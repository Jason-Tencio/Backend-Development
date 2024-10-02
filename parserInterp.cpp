/* Implementation of Recursive-Descent Parser
 * for the SFort95 Language
 * parser(SP24).cpp
 * Programming Assignment 2
 * Spring 2024
*/

//Jason Tencio
// CS280 S24
//parserInterp.cpp file
//April 24, 2024
#include "parserInterp.h"

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> myTemporaryResults; // name of temporary locations
queue<Value> *queueValuesFor; // my pointer variables
bool _log = false;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << line << ": " << msg << endl;
}

bool IdentList(istream &in, int &line);

//Program is: Prog = PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream& in, int& line){
	bool dl = false, sl = false;
	LexItem tok = Parser::GetNextToken(in, line);
		
	if (tok.GetToken() == PROGRAM) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
			dl = Decl(in, line);
			if( !dl  )
			{
				ParseError(line, "Incorrect Declaration in Program");
				return false;
			}
			sl = Stmt(in, line);
			if( !sl  )
			{
				ParseError(line, "Incorrect Statement in program");
				return false;
			}	
			tok = Parser::GetNextToken(in, line);
			
			if (tok.GetToken() == END) {
				tok = Parser::GetNextToken(in, line);
				
				if (tok.GetToken() == PROGRAM) {
					tok = Parser::GetNextToken(in, line);
					
					if (tok.GetToken() == IDENT) {
						cout << "(DONE)" << endl;
						return true;
					}
					else
					{
						ParseError(line, "Missing Program Name");
						return false;
					}	
				}
				else
				{
					ParseError(line, "Missing PROGRAM at the End");
					return false;
				}	
			}
			else
			{
				ParseError(line, "Missing END of Program");
				return false;
			}	
		}
		else
		{
			ParseError(line, "Missing Program name");
			return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	else
	{
		ParseError(line, "Missing Program keyword");
		return false;
	}
}

// VarList ::= Var [= Expr] {, Var [= Expr]}
bool VarList(istream &in, int &line, LexItem &theIdOfTok, int strlen){
    bool status = false, exprstatus = false;
    string identstr;
    Value returnMyValue;

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == IDENT)
    {

        identstr = tok.GetLexeme();
        if (!(defVar.find(identstr)->second))
        {
            defVar[identstr] = false;
            if (theIdOfTok == CHARACTER)
            {
                defVar[identstr] = true;
            }
        }
        else
        {
            ParseError(line, "Variable Redefinition");
            return false;
        }
        SymTable[identstr] = theIdOfTok.GetToken();
        myTemporaryResults[identstr] = returnMyValue;

        if (theIdOfTok == CHARACTER)
        {
            string str(strlen, ' ');
            myTemporaryResults[identstr] = Value(str);
            myTemporaryResults[identstr].SetstrLen(strlen);
        }
    }
    else{

        ParseError(line, "Missing Variable Name");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok == ASSOP){

        exprstatus = Expr(in, line, returnMyValue);
        if (!exprstatus){
            ParseError(line, "Incorrect initialization for a variable.");
            return false;
        }

        if (theIdOfTok == INTEGER && returnMyValue.IsInt() == false){
            ParseError(line, "Illegal Mixed Type Operands  1 ");
            return false;
        }

        if (theIdOfTok == REAL){
            if (returnMyValue.IsInt()){
                returnMyValue = Value(static_cast<double>(returnMyValue.GetInt()));
            }
            else if (returnMyValue.IsReal()){

                returnMyValue = Value(returnMyValue.GetReal());
            }
            else{
                ParseError(line, "Illegal Mixed Type Operands  [real]");
                

                return false;
            }
        }

        if (theIdOfTok == CHARACTER){
            if (returnMyValue.IsString() == false){
                ParseError(line, "Illegal Mixed Type Operands  [string]");
                return false;
            }

            if (returnMyValue.GetstrLen() < strlen){
                string a = returnMyValue.GetString();
                a.append(strlen - a.length(), ' ');
                returnMyValue.SetString(a);
            }

            else{
                string a = returnMyValue.GetString();
                returnMyValue.SetString(a.substr(0, strlen));
            }
        }

        if (returnMyValue.IsString()){
            returnMyValue.SetstrLen(strlen);
        }

        myTemporaryResults[identstr] = returnMyValue;
        defVar[identstr] = true;
        tok = Parser::GetNextToken(in, line);

        if (tok == COMMA){
            status = VarList(in, line, theIdOfTok, strlen);
        }
        else{
            Parser::PushBackToken(tok);
            return true;
        }
    }
    else if (tok == COMMA){
        status = VarList(in, line, theIdOfTok, strlen);
    }
    else if (tok == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        return false;
    }
    else{
        Parser::PushBackToken(tok);
        return true;
    }

    return status;

} // End of VarList

bool SimpleStmt(istream &in, int &line){
    bool status;
    LexItem t = Parser::GetNextToken(in, line);

    switch (t.GetToken()){

    case PRINT:
        status = PrintStmt(in, line);

        if (!status){
            ParseError(line, "Incorrect Print Statement");
            return false;
        }
        cout << "Print statement in a Simple If statement." << endl;
        break;

    case IDENT:
        Parser::PushBackToken(t);
        status = AssignStmt(in, line);
        if (!status){
            ParseError(line, "Incorrect Assignent Statement");
            return false;
        }
        /*cout << "Assignment statement in a Simple If statement." << endl; */
        break;

    default:
        Parser::PushBackToken(t);
        return true;
    }

    return status;
} // End of SimpleStmt

// BlockIfStmt:= if (Expr) then {Stmt} [Else Stmt]
// SimpleIfStatement := if (Expr) Stmt
bool BlockIfStmt(istream &in, int &line){
    Value returnMyValue;
    LexItem t;
    bool ex = false, status;

    t = Parser::GetNextToken(in, line);
    if (t != LPAREN)
    {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    ex = RelExpr(in, line, returnMyValue);
    if (!ex)
    {
        ParseError(line, "Missing if statement condition");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN){
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != THEN){
        Parser::PushBackToken(t);
        if (!returnMyValue.GetBool()){

            t = Parser::GetNextToken(in, line);

            if (t != IDENT && t != PRINT){
                ParseError(line, "Not a stmt");
                return false;
            }
            else if (t == IDENT && Parser::GetNextToken(in, line) != ASSOP){
                ParseError(line, "Missing Assignment Operator");
                return false;
            }
            while (true){
                LexItem newtok = Parser::GetNextToken(in, line);
                if (newtok == PRINT){
                    Parser::PushBackToken(newtok);
                    break;
                }
                if (newtok == IDENT){
                    LexItem newtok2 = Parser::GetNextToken(in, line);
                    if (newtok2 == ASSOP){
                        Parser::PushBackToken(newtok2);
                        Parser::PushBackToken(newtok);
                        break;
                    }
                }
            }
            return true;
        }

        status = SimpleStmt(in, line);
        if (status){
            return true;
        }else{
            ParseError(line, "If-Stmt Syntax Error");
            return false;
        }
    }
    // nestlevel++;
    // level = nestlevel;
    if (returnMyValue.GetBool()){
        status = Stmt(in, line);
        if (!status){
            ParseError(line, "Missing Statement for If-Stmt Then-Part");
            return false;
        }
        t = Parser::GetNextToken(in, line);
        if (t == ELSE){
            while (true){
                t = Parser::GetNextToken(in, line);
                if (t == END){
                    break;
                }
            }
        }
    }
    else{
        while(true){
            t = Parser::GetNextToken(in, line);
            if (t == ELSE || t == END){
                break;
            }
        }
    }

    if (t == ELSE){
        status = Stmt(in, line);
        if (!status){
            ParseError(line, "Missing Statement for If-Stmt Else-Part");
            return false;
        }
        else
            t = Parser::GetNextToken(in, line);
    }

    if (t != END){

        ParseError(line, "Missing END of IF");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t == IF){
        return true;
    }
    Parser::PushBackToken(t);
    ParseError(line, "Missing IF at End of IF statement");
    return false;
} // End of IfStmt function

// PrintStmt:= PRINT *, ExpreList
bool PrintStmt(istream &in, int &line)
{
    LexItem t;
    queueValuesFor = new queue<Value>;

    t = Parser::GetNextToken(in, line);

    if (t != DEF)
    {

        ParseError(line, "Print statement syntax error.");
        return false;
    }
    t = Parser::GetNextToken(in, line);

    if (t != COMMA)
    {

        ParseError(line, "Missing Comma.");
        return false;
    }
    bool ex = ExprList(in, line);

    if (!ex)
    {
        ParseError(line, "Missing expression after Print Statement");
        return false;
    }

    while (!(*queueValuesFor).empty())
    {
        Value valueOfNext = (*queueValuesFor).front();
        cout << valueOfNext;
        queueValuesFor->pop();
    }
    cout << endl;
    return ex;
} // End of PrintStmt

// Var:= ident
bool Var(istream &in, int &line, LexItem &theIdOfTok){
    string identstr;
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == IDENT){
        identstr = tok.GetLexeme();

        if (defVar.find(identstr) == defVar.end())
        {
            ParseError(line, "Undeclared Variable");
            return false;
        }
        theIdOfTok = tok;
        return true;
    }
    else if (tok.GetToken() == ERR)
    {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    return false;
} // End of Var

// Stmt ::= AssigStmt | BlockIfStmt | PrintStmt | SimpleIfStmt
bool Stmt(istream &in, int &line){
    bool status;
    LexItem t = Parser::GetNextToken(in, line);

    switch (t.GetToken()){

    case PRINT:
        status = PrintStmt(in, line);

        if (status)
            status = Stmt(in, line);
        break;

    case IF:
        status = BlockIfStmt(in, line);
        if (status)
            status = Stmt(in, line);
        break;

    case IDENT:
        Parser::PushBackToken(t);
        status = AssignStmt(in, line);
        if (status)
            status = Stmt(in, line);
        break;

    default:
        Parser::PushBackToken(t);
        return true;
    }
    return status;
} // End of Stmt
// AssignStmt:= Var = Expr
bool AssignStmt(istream &in, int &line){
    bool varstatus = false, status = false;
    LexItem t;
    LexItem theIdOfTok;
    Value returnMyValue;
    varstatus = Var(in, line, theIdOfTok);
    string identstr = theIdOfTok.GetLexeme();

    if (varstatus){
        t = Parser::GetNextToken(in, line);
        if (t == ASSOP){
            status = Expr(in, line, returnMyValue);
            if (!status){
                ParseError(t.GetLinenum(), "Missing Expression in Assignment Statment");
                return status;
            }
            if (SymTable[identstr] == INTEGER && returnMyValue.IsInt() == false){
                // cout << "Run-Time Error: Illegal Mixed Type Operands [Int]" << endl;
                ParseError(t.GetLinenum(), "Illegal Mixed Type Operands [Int]");
                return false;
            }
            if (SymTable[identstr] == REAL){
                if (returnMyValue.IsInt()){
                    returnMyValue = Value(returnMyValue.GetInt());
                }
                else if (returnMyValue.IsReal()){
                    returnMyValue = Value(returnMyValue.GetReal());
                }
                else{
                    ParseError(t.GetLinenum(), "Illegal Mixed Type Operands [Real]");
                    return false;
                }
            }

            if (SymTable[identstr] == CHARACTER && returnMyValue.IsString() == false){
                ParseError(t.GetLinenum(), "Illegal Mixed Type Operands [string]");
                return false;
            }
            if (returnMyValue.IsString()){
                int strlen = myTemporaryResults[identstr].GetstrLen();
                if (returnMyValue.GetstrLen() < strlen){
                    string theString = returnMyValue.GetString();
                    theString.append(strlen - theString.length(), ' ');
                    returnMyValue.SetString(theString);
                }else{
                    string theString = returnMyValue.GetString();
                    returnMyValue.SetString(theString.substr(0, strlen));
                }
            }
            myTemporaryResults[identstr] = returnMyValue;
            defVar[identstr] = true;
        }
        else if (t.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
        }
        else{
            ParseError(line, "Missing Assignment Operator");
            return false;
        }
    }
    else{
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
        return false;
    }
    return status;
} // End of AssignStmt
// MultExpr ::= TermExpr { ( * | / ) TermExpr }
bool MultExpr(istream &in, int &line, Value &returnMyValue){
    bool t1 = TermExpr(in, line, returnMyValue);
    LexItem tok;

    if (!t1){
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    while (tok == MULT || tok == DIV){
        Value theNewValue;
        t1 = TermExpr(in, line, theNewValue);

        if (!t1){
            ParseError(line, "Missing operand after operator");
            return false;
        }

        if (tok == MULT){
            returnMyValue = returnMyValue * theNewValue;
        }
        else if (tok == DIV){
            if (theNewValue.IsInt() && theNewValue.GetInt() == 0){
                ParseError(tok.GetLinenum(), "Division by Zero");
                return false;
            }
            else if (theNewValue.IsReal() && theNewValue.GetReal() == 0.0){
                ParseError(tok.GetLinenum(), "Division by Zero");
                return false;
            }
            returnMyValue = returnMyValue / theNewValue;
        }

        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
} // End of MultExpr
// Decl ::= Type :: VarList
// Type ::= INTEGER | REAL | CHARARACTER [(LEN = ICONST)]
bool Decl(istream &in, int &line){
    bool status = false;
    LexItem tok;
    LexItem dog;

    int theLengthOfString = 1;
    LexItem t = Parser::GetNextToken(in, line);

    if (t == INTEGER || t == REAL || t == CHARACTER){
        dog = t;

        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == DCOLON){
            status = VarList(in, line, dog, theLengthOfString);

            if (status){
                status = Decl(in, line);
                if (!status){
                    ParseError(line, "Declaration Syntactic Error.");
                    return false;
                }
                return status;
            }else {
                ParseError(line, "Missing Variables List.");
                return false;
            }
        }
        else if (t == CHARACTER && tok.GetToken() == LPAREN){
            tok = Parser::GetNextToken(in, line);
            if (tok.GetToken() == LEN){
                tok = Parser::GetNextToken(in, line);
                if (tok.GetToken() == ASSOP){
                    tok = Parser::GetNextToken(in, line);
                    if (tok.GetToken() == ICONST){
                        theLengthOfString = stoi(tok.GetLexeme());
                        tok = Parser::GetNextToken(in, line);
                        if (tok.GetToken() == RPAREN){
                            tok = Parser::GetNextToken(in, line);
                            if (tok.GetToken() == DCOLON){
                                status = VarList(in, line, dog, theLengthOfString);
                                if (status){
                                    status = Decl(in, line);
                                    if (!status){
                                        ParseError(line, "Declaration Syntactic Error.");
                                        return false;
                                    }
                                    return status;
                                }
                                else{
                                    ParseError(line, "Missing Variables List.");
                                    return false;
                                }
                            }
                        }
                        else{
                            ParseError(line, "Missing Right Parenthesis for String Length definition.");
                            return false;
                        }
                    }
                    else{
                        ParseError(line, "Incorrect Initialization of a String Length");
                        return false;
                    }
                }
            }
        }
        else{
            ParseError(line, "Missing Double Colons");
            return false;
        }
    }

    Parser::PushBackToken(t);
    return true;
} // End of Decl
// RelExpr ::= Expr  [ ( == | < | > ) Expr ]
bool RelExpr(istream &in, int &line, Value &returnMyValue){
    bool t1 = Expr(in, line, returnMyValue);
    LexItem tok;
    if (!t1){
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    if (tok == EQ || tok == LTHAN || tok == GTHAN){
        Value theNewValue;
        t1 = Expr(in, line, theNewValue);
        if (!t1){
            ParseError(line, "Missing operand after operator");
            return false;
        }
        // cout << "the value of the left expression is " << returnMyValue << " and the value of the right expression is " << newVal << endl;
        try{
            if (tok == EQ){
                returnMyValue = returnMyValue == theNewValue;
            }
            else if (tok == LTHAN){

                returnMyValue = returnMyValue < theNewValue;
            }
            else if (tok == GTHAN){
                returnMyValue = returnMyValue > theNewValue;
            }
        }
        catch (const std::exception &e){

            ParseError(line, e.what());
            return false;
            
        }
    }
    return true;
} // End of RelExpr

// TermExpr ::= SFactor { ** SFactor }
bool TermExpr(istream &in, int &line, Value &returnMyValue){
    bool t1 = SFactor(in, line, returnMyValue);

    LexItem tok;
    if (!t1){
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    if (tok == POW){
        Value valueThatIs;
        t1 = TermExpr(in, line, valueThatIs);

        if (!t1){
            ParseError(line, "Missing exponent operand");
            return false;
        }
        returnMyValue = returnMyValue.Power(valueThatIs);
    }
    else{
        Parser::PushBackToken(tok);
    }
    return true;
} // End of TermExpr

// Expr ::= MultExpr { ( + | - | // ) MultExpr }
bool Expr(istream &in, int &line, Value &returnMyValue){
    bool t1 = MultExpr(in, line, returnMyValue);
    LexItem tok;
    if (!t1){
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    while (tok == PLUS || tok == MINUS || tok == CAT){
        Value theNewValue;
        t1 = MultExpr(in, line, theNewValue);
        if (!t1){
            ParseError(line, "Missing operand after operator");
            return false;
        }

        if (tok == PLUS){
            returnMyValue = returnMyValue + theNewValue;
        }
        else if (tok == MINUS){
            returnMyValue = returnMyValue - theNewValue;
        }
        else if (tok == CAT){
            returnMyValue = returnMyValue + theNewValue;
        }

        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
} // End of Expr

// Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream &in, int &line, int sign, Value &returnMyValue){

    LexItem tok = Parser::GetNextToken(in, line);

    if (tok == IDENT){
        string lexeme = tok.GetLexeme();
        if (!(defVar.find(lexeme)->second)){
            ParseError(line, "Using Undefined Variable");
            return false;
        }

        if (SymTable[lexeme] == INTEGER){
            if (sign != 0){
                returnMyValue = Value(sign * myTemporaryResults[lexeme].GetInt());
            }
            else{
                returnMyValue = myTemporaryResults[lexeme];
            }
            return true;
        }

        if (SymTable[lexeme] == CHARACTER){
            if (sign != 0){
                ParseError(line, "Illegal Operation on Character Type");
                return false;
            }
            returnMyValue = myTemporaryResults[lexeme];
            return true;
        }

        if (SymTable[lexeme] == REAL){
            if (sign != 0){
                returnMyValue = Value(sign * myTemporaryResults[lexeme].GetReal());
            }
            else{
                returnMyValue = myTemporaryResults[lexeme];
            }
            return true;
        }

        if (_log){
            cout << "the token is " << tok.GetLexeme() << " and the value is " << myTemporaryResults[lexeme] << endl;
        }

        returnMyValue = myTemporaryResults[lexeme];

        return true;
    }
    else if (tok == ICONST){
        if (sign == -1){
            returnMyValue = Value(-stoi(tok.GetLexeme()));
        }
        else{
            returnMyValue = Value(stoi(tok.GetLexeme()));
        }
        return true;
    }
    else if (tok == SCONST){
        if (sign != 0){
            ParseError(line, "Illegal Operation on Character Type");
            return false;
        }
        returnMyValue = Value(tok.GetLexeme());
        returnMyValue.SetstrLen(returnMyValue.GetString().length());
        return true;
    }
    else if (tok == RCONST){

        if (sign == -1){
            returnMyValue = Value(-stod(tok.GetLexeme()));
        }
        else{
            returnMyValue = Value(stod(tok.GetLexeme()));
        }
        return true;
    }
    else if (tok == LPAREN){
        bool ex = Expr(in, line, returnMyValue);
        if (!ex){
            ParseError(line, "Missing expression after (");
            return false;
        }
        if (Parser::GetNextToken(in, line) == RPAREN )
            return ex;
        else{
            Parser::PushBackToken(tok);
            ParseError(line, "Missing ) after expression");
            return false;
        }
    }
    else if (tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }

    return false;
}

// SFactor = Sign Factor | Factor
bool SFactor(istream &in, int &line, Value &returnMyValue){
    LexItem t = Parser::GetNextToken(in, line);

    bool status;
    int sign = 0;
    if (t == MINUS){
        sign = -1;
    }
    else if (t == PLUS){
        sign = 1;
    }
    else{
        Parser::PushBackToken(t);
    }

    status = Factor(in, line, sign, returnMyValue);
    return status;
} // End of SFactor

// ExprList:= Expr {,Expr}
bool ExprList(istream &in, int &line){
    bool status = false;
    Value returnMyValue;

    status = Expr(in, line, returnMyValue);
    if (!status){
        ParseError(line, "Missing Expression");
        return false;
    }
    queueValuesFor->push(returnMyValue);
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == COMMA){

        status = ExprList(in, line);
    }
    else if (tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
} // End of ExprList
