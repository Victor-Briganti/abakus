#include <iostream>
#include <cstdlib>
#include <cctype>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <map>
#include <memory>

// Base function for log error
int LogError(std::string Err) {
    std::cout << "ERROR: " + Err << std::endl;
    return 0;
}

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
    while (Expr[iE]) {
        // Remove the whitespaces
        while (isblank(Expr[iE])) {
            iE++;
            if (iseol(Expr[iE])) {
                return token_eol;
            }
        }
        
        // Verify if is a digit
        // [0-9]+ : Integer
        // [0-9]+[.][0-9]* : Double 
        if (isdigit(Expr[iE])) {
            std::string Buffer;
            bool FloatFlag = false;
            while (isdigit(Expr[iE])) {
                Buffer += Expr[iE];
                iE++;
                if (Expr[iE] == '.' && !FloatFlag) {
                    FloatFlag = true;
                    Buffer += Expr[iE];
                    iE++;
                }
                if (Expr[iE] == '.' && FloatFlag) {
                    NumDouble = strtold(Buffer.c_str(), nullptr);
                    return token_double;
                }
            }
            NumDouble = strtold(Buffer.c_str(), nullptr);
            return token_double;
        }

        // If not a token return ASCII
        if (isascii(Expr[iE])) {
            int Holder = (int)Expr[iE];
            iE++;
            return std::move(Holder);
        }
    }
    if (iseol(Expr[iE])) { 
        return token_eol;
    }
    return LogError("character in expression was not reconized");
}

// ============================================================================
// ==========                ABSTRACT SYNTAX TREE                    ==========
// ============================================================================

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

void LogErrorG (std::string Err) {
    std::cout << "ERROR: " + Err << std::endl;
    exit(1);
}

// +++++++++++++++++++++++ 
// +-----+ GLOBALS +-----+
// +++++++++++++++++++++++

// Holds the precedence of operations.
std::map<char,int> Precedence; 

// Hold the current token
int CurToken;
// Update the current token
int getNextToken() { 
    return CurToken = Tokenizer(); 
} 

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
}

// +++++++++++++++++++++++ 
// +-----+ GRAMMAR +-----+
// +++++++++++++++++++++++

// E ::= E(double)
OperationDoubAST* ParserDoub() {
    OperationDoubAST *E = new OperationDoubAST(NumDouble);
    return E;
}

// E ::= -E
OperationDoubAST* ParserMinus() {
    if (CurToken == token_double) {
        OperationDoubAST *E = new OperationDoubAST(-NumDouble);
        getNextToken(); // eat double;
        return E;
    }
}

// E ::= E+E | E-E | E*E | E/E
OperationDoubAST* ParserExpr(char CurOp) {
    OperationDoubAST *E = new OperationDoubAST;
    // num 
    if (CurToken == token_double) {
        getNextToken(); // eat double 
        E = ParserDoub();
    }

    // Expression 
    if (CurToken == token_eol) {
            return E;
    }

    if(Precedence[CurToken]) {
        if (Precedence[CurOp] >= Precedence[CurToken]) {
            return E;
        }

        E->Op = CurToken;
        getNextToken(); // eat operation

        if (CurToken == token_eol || Precedence[CurToken]) {
            LogErrorG("illegal instruction");
        }

        E->RHS = new OperationDoubAST(NumDouble);
        getNextToken(); // eat double

        if (CurToken == token_double) {
            LogErrorG("expected a operation after number");
        }

        if (CurToken == token_eol) {
            E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
            return E;
        }
        
        if (Precedence[CurToken]) {
            if (Precedence[E->Op] >= Precedence[CurToken]) {
                E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
                return E;
            }
            
            // Saves the current token
            E->RHS->Op = CurToken;
            getNextToken(); // eat operator;
            
            // Reduce expression to E
            E->RHS = ParserExpr(E->RHS->Op);
            E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
            return E;
        }
    }
}

// num ::= E 
double PrimaryParser() {
    OperationDoubAST *E = new OperationDoubAST;

    // -E
    if (CurToken == '-') {
        getNextToken(); // eat '-'
        E = ParserMinus();
    }

    // num
    if (CurToken == token_double) { 
        getNextToken(); // eat double 
        E = ParserDoub();
    }
    
    // expression
    if (Precedence[CurToken]) {
        if (!E->LHS) {
            LogErrorG("illegal instruction");
        }
        
        E->Op = CurToken;
        getNextToken(); // eat operator
        
        while(true){
            // Parse the right side
            E->RHS = ParserExpr(E->Op);
    
            // Reduce to the left side
            if (CurToken == token_eol) {
                return Reduce(E->Op, E->LHS, E->RHS->LHS);
            }
            E->LHS = Reduce(E->Op, E->LHS, E->RHS->LHS);
            
            // Update the tokens
            E->Op = CurToken;
            getNextToken(); // eat operator;
        }
    }

    if(!E) {
        LogErrorG("expression was not reconized");
    }

    return E->LHS;
}

// result ::= num 
void Result() {
    getNextToken();
    if (CurToken == token_eol) {
        LogErrorG("expected a expression");
    }
    if (CurToken == token_double || CurToken == '-') {
        std::cout << PrimaryParser() << std::endl;
    }
}

int main() {
    // Defines the operators and itsprecedences
    // 1 is the lowest.
    Precedence['+'] = 10;
    Precedence['-'] = 10;
    Precedence['/'] = 20;
    Precedence['*'] = 20; 
        
    Expr = "100-10*10+10+20/5";
    Result();

    return 0;
}
