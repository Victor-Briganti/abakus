#include <iostream>
#include <cstdlib>
#include <cctype>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <map>
#include <memory>

// ============================================================================
// ==========                         LEXER                          ==========
// ============================================================================

// +++++++++++++++++++++++ 
// +-----+ GLOBALS +-----+
// +++++++++++++++++++++++

std::string Expr; // Expression string.
int iE = 0;       // Expression interator.
double NumDouble;   // Saves the double number.

enum Token {
    // Token for End of Line 
    token_eol = -1,

    // Token for Numbers
    token_double = -3,

    // Token for comparative operators
    token_equal = -11,
    token_diff = -12,
    token_gqual = -13,
    token_lqual = -14
};

// Verify the end of line 
int iseol(int c) {
    if (c == 0) {
        return 1;
    }

    return 0;
}

// Token generator
int Tokenizer() {
    // End of string
    if (iseol(Expr[iE])) {
        return token_eol;
    }
    
    // Remove the whitespaces
    while(isblank(Expr[iE])) {
        iE++;
        
        if (iseol(Expr[iE])) {
            return token_eol;
        }

    }

    // Verify if is a digit 
    // [0-9]+[.][0-9] : Double 
    if (isdigit(Expr[iE])) {
        std::string Buffer;
        Buffer += Expr[iE];
        iE++;
        
        while(true) {
            if(!isdigit(Expr[iE])) {
                break;
            }
            
            Buffer += Expr[iE];
            iE++;
        }

        if (Expr[iE] == '.') {
            Buffer += Expr[iE];
            iE++;
            while (isdigit(Expr[iE])) {
                Buffer  += Expr[iE];
                iE++;
            }
        }

        NumDouble = strtold(Buffer.c_str(), nullptr);
        return token_double;
    }
    
    // Verify the comparion operators
    // ==
    if (Expr[iE] == '=') {
        iE++;
        if (Expr[iE] == '=') {
            iE++;
            return token_equal;
        }
        int Buffer = Expr[iE];
        return Buffer;
    }

    // != 
    if (Expr[iE] == '!') {
        iE++;
        if (Expr[iE] == '=') {
            iE++;
            return token_diff;
        }
        int Buffer = Expr[iE];
        return Buffer;
    }
    
    // <=
    if (Expr[iE] == '<') {
        iE++;
        if (Expr[iE] == '=') {
            iE++;
            return token_lqual;
        }
        int Buffer = Expr[iE];
        return Buffer;
    }

    // >=
    if (Expr[iE] == '>') {
        iE++;
        if (Expr[iE] == '=') {
            iE++;
            return token_gqual;
        }
        int Buffer = Expr[iE];
        return Buffer;
    }

    // If everything fails return the character
    int Buffer = Expr[iE];
    iE++;
    return Buffer;
}

// ============================================================================
// ==========                ABSTRACT SYNTAX TREE                    ==========
// ============================================================================

// Class that holds the AST node of the tree.
class OperationDoubAST {
    public:
        double LHS;
        OperationDoubAST *RHS;
        char Op;
    
        OperationDoubAST() {}

        OperationDoubAST(double Left) {
            LHS = Left;
        }
};

// ============================================================================
// ==========                         PARSER                         ==========
// ============================================================================

OperationDoubAST* LogError (std::string Err) {
    std::cout << "ERROR: " + Err << std::endl;
    return nullptr;
}

// +++++++++++++++++++++++ 
// +-----+ GLOBALS +-----+
// +++++++++++++++++++++++

// Holds the precedence of operations.
std::map<char,int> Precedence; 

// Hold the current token
int CurToken;
// Update the current token
void getNextToken() { CurToken = Tokenizer(); } 

// Execute the common operations
double Reduce(char S, double L, double R) {
    if (S == '+') {
        return L + R;
    }
    if (S == '-') {
        return L - R;
    }
    if (S == '*') {
        return L * R;
    }
    if (S == '/') {
        return L / R;
    }
    if (S == '>') {
        return L > R;
    }
    if (S == '<') {
        return L < R;
    }
    if (S == token_equal) {
        return L == R;
    }
    if (S == token_diff) {
        return L != R;
    }
    if (S == token_lqual) {
        return L <= R;
    }
    if (S == token_gqual) {
        return L >= R;
    }

    // Error Handling
    std::string Err;
    Err += "'";
    Err += S;
    Err += "'";
    Err += " operation not reconized";
    LogError(Err);
    exit(1);
}

// +++++++++++++++++++++++ 
// +-----+ GRAMMAR +-----+
// +++++++++++++++++++++++

// Definition of functions that may need recursion
OperationDoubAST* ParserMinus();
OperationDoubAST* ParserExpr(char CurOp);
OperationDoubAST* ParserParenExpr();

// E ::= (double)num
OperationDoubAST* ParserDoub() {
    OperationDoubAST *E = new OperationDoubAST(NumDouble);
    return E;
}

// E ::= -E
OperationDoubAST* ParserMinus() {
    // -E ::= num
    if (CurToken == token_double) {
        OperationDoubAST *E = new OperationDoubAST(-NumDouble);
        getNextToken(); // eat double;
        return E;
    }

    // -E ::= (E)
    if (CurToken == '(') {
        OperationDoubAST *E = new OperationDoubAST;
        getNextToken(); // eat '('
        E = ParserParenExpr();
        if (!E->LHS) {
            return E;
        }
        else {
            E->LHS = -E->LHS;
            return E;
        }
    }
    return LogError("illegal expression");
}

// E ::= (E)
OperationDoubAST* ParserParenExpr() {
    OperationDoubAST *E = new OperationDoubAST; 
    
    // E ::= (-E)
    if (CurToken == '-') {
        getNextToken(); // eat '-'
        E = ParserMinus();
        if (!E) {
            return E;
        }

        if (CurToken == token_double) {
            LogError("expression was not reconized");
        }
    }
    
    // E ::= (num)
    if (CurToken == token_double) {
        getNextToken(); // eat double
        E = ParserDoub();

        if (CurToken == '(') {
            LogError("expression was not reconized");
        }
    }
   
    // E ::= (E)
    if (CurToken == '(') {
        getNextToken(); // eat '('
        E = ParserParenExpr();
        if (!E) {
            return E;
        }
    }

    // Token error
    if (CurToken == token_double || CurToken == token_eol || CurToken == '(') {
        return LogError("expected a ')'");
    }
    
    // E ::= (E '+' E)
    if (Precedence[CurToken]) {
        E->Op = CurToken;
        getNextToken(); // eat operation
        E->RHS = ParserExpr(E->Op);
        
        while (true) {
            if (CurToken == ')'){
                getNextToken(); // eat ')'
                E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
                delete E->RHS;
                return E;
            }
            
            if (Precedence[CurToken]) {
                // Verify the precedence to see where it can reduce
                if (Precedence[E->Op] >= Precedence[CurToken]) {
                    E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
                    E->Op = CurToken;
                    getNextToken(); // eat operation 
                    E->RHS = ParserExpr(E->Op);
                } else {
                    E->RHS->Op = CurToken;
                    getNextToken(); // eat operation
                    E->RHS = ParserExpr(E->Op);
                }

                if (CurToken == token_eol) {
                    return LogError("illegal instruction after '('");
                }

                if (CurToken == token_double) {
                    E->RHS = ParserExpr(E->Op);
                    if (!E->RHS) {
                        return nullptr;
                    }
                }
                
                if (CurToken == token_double || CurToken == token_eol) {
                    return LogError("expected a ')'");
                }
            }
        }
    }
    if (CurToken == ')') {
        getNextToken(); // eat ')'
    }
    return E;
}

// E ::= E+E | E-E | E*E | E/E | (E)
OperationDoubAST* ParserExpr(char CurOp) {
    OperationDoubAST *E = new OperationDoubAST;
    // E ::= num. ('+' E)?
    if (CurToken == token_double) {
        getNextToken(); // eat double 
        E = ParserDoub();
    }
    
    // E ::= (.E) ('+' E)?
    if (CurToken == '(') {
        getNextToken(); // eat '('
        E = ParserParenExpr();
        if(!E) {
            return E;
        }
    }
    
    // E ::= .
    if (CurToken == token_eol) {
        return E;
    }
    
    // E ::= (E).
    if (CurToken == ')') {
        return E;
    }
    
    // Error Handling
    if (!E->LHS) {
        return LogError("expected a number after operation");
    }
    
    // E :: = '+'. E
    if(Precedence[CurToken]) {
        if (Precedence[CurOp] >= Precedence[CurToken]) {
            return E;
        }
        
        E->Op = CurToken;
        getNextToken(); // eat operation

        // Error Handling
        if (CurToken == token_eol || Precedence[CurToken]) {
            return LogError("illegal instruction");
        }
        
        // E ::= E. ('+' E)?
        if (CurToken == token_double){
            E->RHS = new OperationDoubAST(NumDouble);
            getNextToken(); // eat double
        }
        
        // E ::= (.E)
        if(CurToken == '(') {
            getNextToken(); // eat '('
            E->RHS = ParserParenExpr();
            if (!E->RHS) {
                return nullptr;
            }
        }

        // Error Handling 
        if (CurToken == token_double) {
            return LogError("expected a operation after number");
        }
        
        // E ::= .
        if (CurToken == token_eol) {
            E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
            delete E->RHS;
            return E;
        }
        
        // E ::= (E).
        if (CurToken == ')') {
            //getNextToken(); // eat ')'
            E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
            delete E->RHS;
            return E;
        }

        // E ::= '+'. E
        if (Precedence[CurToken]) {
            if (Precedence[E->Op] >= Precedence[CurToken]) {
                E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
                delete E->RHS;
                return E;
            }
            
            // Saves the current token
            E->RHS->Op = CurToken;
            getNextToken(); // eat operator;
            
            // E ::= E.
            E->RHS = ParserExpr(E->RHS->Op);
            if (!E->RHS) {
                return nullptr;
            }
            E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
            delete E->RHS;
            return E;
        }
    }
    return nullptr;
}

// num ::= E 
double PrimaryParser() {
    OperationDoubAST *E = new OperationDoubAST;

    // num ::= -E
    if (CurToken == '-') {
        getNextToken(); // eat '-'
        E = ParserMinus();
        if (!E) {
            return 0;
        }
        
        if (CurToken == token_double || CurToken == '(') {
            LogError("expression not reconized");
            return 0;
        }
    }
    
    //  num ::= (E)
    if (CurToken == '(') {
        getNextToken(); // eat '('
        E = ParserParenExpr();
        if (!E) {
            return 0;
        }
        
        if (CurToken == token_double || CurToken == '(') {
            LogError("expression not reconized");
            return 0;
        }
    }

    // num ::= num. ('+' E)?
    if (CurToken == token_double) { 
        getNextToken(); // eat double 
        E = ParserDoub();

        if (CurToken == token_double || CurToken == '(') {
            LogError("expression not reconized");
            return 0;
        }
    }
    
    // num ::= E. ('+' E)?
    if (Precedence[CurToken]) {
        if (!E->LHS) {
            LogError("illegal expression");
            return 0;
        }
        
        // num ::= E ('+'. E)?
        E->Op = CurToken;
        getNextToken(); // eat operator
        
        while(true){
            // Parse the right side
            // num ::= E ('+' E.)? 
            E->RHS = ParserExpr(E->Op);
            if (!E->RHS) {
                return 0;
            }
    
            // Reduce to the left side
            // num ::= num. 
            if (CurToken == token_eol) {
                double res = Reduce(E->Op, E->LHS, E->RHS->LHS);
                return res;
            }
            // num ::= num. ('+' E)?
            E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
            delete E->RHS;

            // Update the tokens
            // num ::= num ('+'. E)?
            E->Op = CurToken;
            getNextToken(); // eat operator;
        }
    }

    // Error with expression
    if(!E) {
        LogError("expression was not reconized");
        return 0;
    }

    return E->LHS;
}

// Prints the result
void Result() {
    getNextToken();
    if (CurToken == token_eol) {
        LogError("expected a expression");
    }
    if (CurToken == token_double || CurToken == '-' || CurToken == '(') {
        std::cout << PrimaryParser() << std::endl;
    }
}

int main() {
    // Defines the operators and its precedences
    // 1 is the lowest.
    Precedence[token_equal] = 5; // Precedence of '=='
    Precedence[token_diff] = 5;  // Precedence of '!='
    Precedence[token_gqual] = 5; // Precedence of '>='
    Precedence[token_lqual] = 5; // Precedence of '<='
    Precedence['>'] = 5;
    Precedence['<'] = 5;
    Precedence['+'] = 10;
    Precedence['-'] = 10;
    Precedence['/'] = 20;
    Precedence['*'] = 20;
        
    Expr = "12*123-(15+1231-23.4)+23.1-12.7/1.5+1.1-(-(23)+(3-(4+(4))))";
    Result();

    return 0;
}
