/******************************************************************************
* Abakus 
*
* Name: abakus
* Author: John Mag0
* Date: 2023-01-08
* Version: 1.0 
*******************************************************************************
* MIT Lincense 
* 
* Copyright 2023 John Mag0
*
* Permission is hereby granted, free of charge, to any person obtaining a copy 
* of this software and associated documentation files (the "Software"), to deal 
* in the Software without restriction, including without limitation the rights 
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
* SOFTWARE.
******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <cctype>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>

// GNU Readline
#include <readline/readline.h>
#include <readline/history.h>

// ============================================================================
// ==========                         LEXER                          ==========
// ============================================================================

// +++++++++++++++++++++++ 
// +-----+ GLOBALS +-----+
// +++++++++++++++++++++++

std::string Expr; // Expression string.
int iE = 0;       // Expression interator.
double NumDouble; // Saves the double number.
std::string Var;  // Saves the variable name

enum Token {
    // Token for End of Line 
    token_eol = -1,

    // Token for Numbers
    token_double = -3,

    // Token for variables
    token_var = -4,

    // Token for comparative operators
    token_equal = -11,
    token_diff = -12,
    token_gqual = -13,
    token_lqual = -14
};

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
    // Verify if is a variable 
    // [A-Za-z][A-Za-z0-9_]*
    if (isalpha(Expr[iE])) {
        std::string Buffer;
        Buffer += Expr[iE];
        iE++;
        
        while (isalnum(Expr[iE]) || Expr[iE] == '_') {
            Buffer += Expr[iE];
            iE++;
        }
        
        Var = Buffer;
        return token_var;
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
        return 61; // ASCII for '='
    }

    // != 
    if (Expr[iE] == '!') {
        iE++;
        if (Expr[iE] == '=') {
            iE++;
            return token_diff;
        }
        return 33; // ASCII for '!'
    }
    
    // <=
    if (Expr[iE] == '<') {
        iE++;
        if (Expr[iE] == '=') {
            iE++;
            return token_lqual;
        }
        return 60; // ASCII for '<'
    }

    // >=
    if (Expr[iE] == '>') {
        iE++;
        if (Expr[iE] == '=') {
            iE++;
            return token_gqual;
        }
        return 62; // ASCII for '>'
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
std::unordered_map<char,int> Precedence; 
// Holds the variables and its values
std::unordered_map<std::string, double> Variables;

// Hold the current token
int CurToken;
// Update the current token
void getNextToken() { CurToken = Tokenizer(); } 
// Flag to determine possible error 
int ExprError = 0;

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
    
    // -E ::= var 
    if (CurToken == token_var) {
        if (Variables.find(Var) != Variables.end()) {
            OperationDoubAST *E = new OperationDoubAST(-Variables[Var]);
            return E;
        }

        ExprError = 1;
        return LogError(Var + " was not found");
    }

    ExprError = 1;
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

        if (CurToken == token_double || CurToken == token_var) {
            ExprError = 1;
            LogError("expression was not reconized");
        }
    }
    
    // E ::= (num)
    if (CurToken == token_double) {
        getNextToken(); // eat double
        E = ParserDoub();

        if (CurToken == '(' || CurToken == token_var) {
            ExprError = 1;
            LogError("expression was not reconized");
        }
    }

    // E ::= (var)
    if (CurToken == token_var) {
        getNextToken(); // eat variable

        if (Variables.find(Var) == Variables.end()) {
            ExprError = 1;
            LogError(Var + " was not found");
            return 0;
        }
        
        E = new OperationDoubAST(Variables[Var]);
        
        if (CurToken == '(' || CurToken == token_double || CurToken == token_var) 
        {
            ExprError = 1;
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
    if (CurToken == token_double || CurToken == token_eol || CurToken == '(' || CurToken == token_var) 
    {
        ExprError = 1;
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
                    ExprError = 1;
                    return LogError("illegal instruction after '('");
                }

                if (CurToken == token_double) {
                    E->RHS = ParserExpr(E->Op);
                    if (!E->RHS) {
                        return nullptr;
                    }
                }
                
                if (CurToken == token_var || CurToken == token_double || CurToken == token_eol) {
                    ExprError = 1;
                    return LogError("expected a ')'");
                }

                if (CurToken == token_var) {
                    if (Variables.find(Var) == Variables.end()) {
                        ExprError = 1;
                        return LogError(Var + " was not found");
                    }
                    E->RHS = ParserExpr(E->Op);
                    if (!E->RHS) {
                        return nullptr;
                    }
                }
                
                if (CurToken == token_var || CurToken == token_double || CurToken == token_eol) {
                    ExprError = 1;
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
    
    // E ::= var. ('+' E)?
    if (CurToken == token_var) {
        if (Variables.find(Var) == Variables.end()) {
            ExprError = 1;
            LogError(Var + " was not found");
            return E;
        }
        getNextToken(); // eat variable
        E = new OperationDoubAST(Variables[Var]);
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
        ExprError = 1;
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
            ExprError = 1;
            return LogError("illegal instruction");
        }
        
        // E ::= num. ('+' E)?
        if (CurToken == token_double){
            E->RHS = new OperationDoubAST(NumDouble);
            getNextToken(); // eat double
        }
        
        // E ::= var. ('+' E)?
        if (CurToken == token_var) {
            if (Variables.find(Var) == Variables.end()) {
                E->RHS = new OperationDoubAST(Variables[Var]);
                getNextToken(); // eat variable
            }
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
            ExprError = 1;
            return LogError("expected a operation after number");
        }
        
        if (CurToken == token_var) {
            ExprError = 1;
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
        
        if (CurToken == token_double || CurToken == '(' || CurToken == token_var) 
        {
            LogError("expression not reconized");
            ExprError = 1;
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
        
        if (CurToken == token_double || CurToken == '(' || CurToken == token_var) 
        {
            ExprError = 1;
            LogError("expression not reconized");
            return 0;
        }
    }

    // num ::= num. ('+' E)?
    if (CurToken == token_double) { 
        getNextToken(); // eat double 
        E = ParserDoub();

        if (CurToken == token_double || CurToken == '(' || CurToken == token_var) 
        {
            ExprError = 1;
            LogError("expression not reconized");
            return 0;
        }
    }

    // num ::= var
    if (CurToken == token_var) {
        if (Variables.find(Var) == Variables.end()) {
            ExprError = 1;
            LogError(Var + " was not found");
        }
        
        getNextToken(); // eat var 
        E = new OperationDoubAST(Variables[Var]);

        if (CurToken == token_double || CurToken == '(' || CurToken == token_var) 
        {
            ExprError = 1;
            LogError("expression not reconized");
            return 0;
        }
    }
    
    // num ::= E. ('+' E)?
    if (Precedence[CurToken]) {
        if (!E->LHS) {
            ExprError = 1;
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
        ExprError = 1;
        return 0;
    }

    return E->LHS;
}

void ParserVar() {
    // If exist print it
    if (Variables.find(Var) != Variables.end()) {
        std::cout << Variables[Var] << std::endl;
    } else {
        getNextToken(); // eat variable
        if (CurToken == '=') {
            getNextToken(); // eat '='
            double res = PrimaryParser();
            if (!ExprError) {
                Variables[Var] = res;
            }
        }
    }
}

// Prints the result
void Result() {
    getNextToken();
    if (CurToken == token_eol) {
        LogError("expected a expression");
    }
    if (CurToken == token_double || CurToken == '-' || CurToken == '(') {
        double res = PrimaryParser();
        if (!ExprError){
            std::cout << res << std::endl;
        }
    }
    if (CurToken == token_var) {
        ParserVar();
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


    char *tmp;
    while ((tmp = readline("? ")) != 0) {
        if (strlen(tmp) > 0 && !ExprError) {
            add_history(tmp);
        }

        Expr = tmp;
        Result();

        ExprError = 0;
        iE = 0;
        
        free(tmp);
    }

    return 0;
}
