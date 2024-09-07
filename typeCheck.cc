#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <unordered_map>
using namespace std;

struct procedure {
    vector<string> signature;
    unordered_map<string, string> variables; // symbol table

    procedure() : signature{}, variables{} {}
};

// The key of the procedure is the id for the procedure
unordered_map<string, procedure*> table;

class node {
public:
    string rule;
    string ruleComplete;
    string type;
    vector<node*> children;

    node(string rule) : rule{rule}, ruleComplete{}, type{}, children{} {}

    ~node() {
        for (int i = 0; i < this->children.size(); ++i) {
            delete this->children[i];
        }
    }

    string getLexeme(const node *curNode) const {
        return curNode->children[0]->rule;
    }
};

void traverseTree(node *tree);

void printTree(node *currentNode) {
    cout << currentNode->ruleComplete;

    if (!currentNode->type.empty()) {
        cout << " : " << currentNode->type;
    }
    if (!currentNode->ruleComplete.empty()) {
        cout << endl;
    }
    for (int i = 0; i < currentNode->children.size(); ++i) {
        printTree(currentNode->children[i]);
    }
}

string getType(node *curNode, unordered_map<string, string> *symbolTable) {
    if (curNode->ruleComplete == "type INT") {
        return "int";
    }
    else if (curNode->ruleComplete == "type INT STAR") {
        return "int*";
    }
    else if (curNode->ruleComplete == "expr term") {
        curNode->type = getType(curNode->children[0], symbolTable);
        return curNode->type;
    }
    else if (curNode->ruleComplete == "expr expr PLUS term") {
        string exprType = getType(curNode->children[0], symbolTable);
        string termType = getType(curNode->children[2], symbolTable);
        if (exprType == "int" && termType == "int") {
            curNode->type = "int";
        } else if (exprType == "int*" && termType == "int" || exprType == "int" && termType == "int*") {
            curNode->type = "int*";
        } else {
            throw runtime_error("Invalid types for PLUS operation in expression.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "expr expr MINUS term") {
        string exprType = getType(curNode->children[0], symbolTable);
        string termType = getType(curNode->children[2], symbolTable);
        if (exprType == "int" && termType == "int") {
            curNode->type = "int";
        } else if (exprType == "int*" && termType == "int") {
            curNode->type = "int*";
        } else if (exprType == "int*" && termType == "int*") {
            curNode->type = "int";
        } else {
            throw runtime_error("Invalid types for MINUS operation in expression.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "term term STAR factor") {
        string termType = getType(curNode->children[0], symbolTable);
        string factorType = getType(curNode->children[2], symbolTable);
        if (termType == "int" && factorType == "int") {
            curNode->type = "int";
        } else {
            throw runtime_error("Invalid types for STAR operation in term.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "term term SLASH factor") {
        string termType = getType(curNode->children[0], symbolTable);
        string factorType = getType(curNode->children[2], symbolTable);
        if (termType == "int" && factorType == "int") {
            curNode->type = "int";
        } else {
            throw runtime_error("Invalid types for SLASH operation in term.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "term term PCT factor") {
        string termType = getType(curNode->children[0], symbolTable);
        string factorType = getType(curNode->children[2], symbolTable);
        if (termType == "int" && factorType == "int") {
            curNode->type = "int";
        } else {
            throw runtime_error("Invalid types for PCT operation in term.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "term factor") {
        curNode->type = getType(curNode->children[0], symbolTable);
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor LPAREN expr RPAREN") {
        curNode->type = getType(curNode->children[1], symbolTable);
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor NUM") {
        curNode->type = "int";
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor NULL") {
        curNode->type = "int*";
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor AMP lvalue") {
        string lvalueType = getType(curNode->children[1], symbolTable);
        if (lvalueType == "int") {
            curNode->type = "int*";
        } else {
            throw runtime_error("Invalid type for AMP lvalue.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor STAR factor") {
        string factorType = getType(curNode->children[1], symbolTable);
        if (factorType == "int*") {
            curNode->type = "int";
        } else {
            throw runtime_error("Invalid type for STAR factor.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor NEW INT LBRACK expr RBRACK") {
        string exprType = getType(curNode->children[3], symbolTable);
        if (exprType == "int") {
            curNode->type = "int*";
        } else {
            throw runtime_error("Invalid type for NEW INT LBRACK expr RBRACK.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor GETCHAR LPAREN RPAREN") {
        curNode->type = "int";
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor ID LPAREN RPAREN") {
        curNode->type = "int";
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor ID LPAREN arglist RPAREN") {
        curNode->type = "int";
        return curNode->type;
    }
    else if (curNode->ruleComplete == "factor ID") {
        curNode->type = getType(curNode->children[0], symbolTable);
        return curNode->type;
    }
    else if (curNode->ruleComplete == "lvalue STAR factor") {
        string factorType = getType(curNode->children[1], symbolTable);
        if (factorType == "int*") {
            curNode->type = "int";
        } else {
            throw runtime_error("Invalid type for STAR factor in lvalue.");
        }
        return curNode->type;
    }
    else if (curNode->ruleComplete == "lvalue LPAREN lvalue RPAREN") {
        curNode->type = getType(curNode->children[1], symbolTable);
        return curNode->type;
    }
    else if (curNode->rule == "NUM") {
        curNode->type = "int";
        return curNode->type;
    }
    else if (curNode->ruleComplete == "NULL NULL") {
        curNode->type = "int*";
        return curNode->type;
    }
    else if (curNode->rule == "ID") {
        string idName = curNode->getLexeme(curNode);

        if (symbolTable->count(idName) == 1) {
            curNode->type = (*symbolTable)[idName];
        }
  
        return curNode->type;
    }
    if (curNode->ruleComplete == "lvalue ID" || curNode->ruleComplete == "factor ID") {
        string idName = curNode->getLexeme(curNode->children[0]);
        if (symbolTable->count(idName) != 1) {
            throw runtime_error("Undeclared identifier used: " + idName);
        }
        curNode->type = getType(curNode->children[0], symbolTable);
        return curNode->type;
    }
    else {
        return curNode->type;
    }
}

void typesAndDeclarations(node *curNode, unordered_map<string, string> *symbolTable) {
   /*
   cout << "------------" << endl;
    cout << "Symbol Table: " << endl;
    for (auto it : *symbolTable) {
        cout << it.first << endl;
    }
    cout << "current rule: " << curNode->ruleComplete << endl;
   */ 

    if (curNode->ruleComplete == "dcl type ID") {
        // process the declaration
        string idName = curNode->getLexeme(curNode->children[1]);
        string typeName = getType(curNode->children[0], symbolTable);

        // make sure the declarations are unique
        if (symbolTable->count(idName) != 0) {
            throw runtime_error("Duplicate declaration for variable: " + idName);
        }
        symbolTable->insert({idName, typeName});
        curNode->children[1]->type = typeName;
    }
    // Check for use after declarations
    else if (curNode->ruleComplete == "lvalue ID" || curNode->ruleComplete == "factor ID") {
        string idName = curNode->getLexeme(curNode->children[0]);
        if (symbolTable->count(idName) != 1) {
            throw runtime_error("Undeclared identifier used: " + idName);
        }
        curNode->type = getType(curNode->children[0], symbolTable);
    }
    // Check for function calls inside of the procedure
    else if (curNode->ruleComplete == "factor ID LPAREN RPAREN") {
        string funcCallName = curNode->getLexeme(curNode->children[0]);
        if (table.count(funcCallName) != 1) {
            throw runtime_error("Undefined procedure call: " + funcCallName);
        } else if (symbolTable->count(funcCallName) == 1) {
            throw runtime_error("Identifier used as both variable and function 1: " + funcCallName);
        }
    }
    // We also need to check if the variables are in the symbol table
    else if (curNode->ruleComplete == "factor ID LPAREN arglist RPAREN") {
        string funcCallName = curNode->getLexeme(curNode->children[0]);
        if (table.count(funcCallName) != 1) {
            throw runtime_error("Undefined procedure call: " + funcCallName);
        } else if (symbolTable->count(funcCallName) == 1) {
            throw runtime_error("Identifier used as both variable and function 2: " + funcCallName);
        }
        typesAndDeclarations(curNode->children[2], symbolTable);
    }
    else if (curNode->rule == "ID") {
        string idName = curNode->getLexeme(curNode);

        if (symbolTable->count(idName) != 1 && table.count(idName) != 1) {
            throw runtime_error("Use before declaration: " + idName);
        }
        else if (table.count(idName) == 1){
        }
        else if (symbolTable->count(idName) == 1){    
            curNode->type = (*symbolTable)[idName];
        }
    }
    else if (curNode->ruleComplete == "procedures procedure procedures") {
        traverseTree(curNode);
    }

    // Now we process the types
    getType(curNode, symbolTable);

    for (auto child : curNode->children) {
        typesAndDeclarations(child, symbolTable);
    }

    // Check return value of procedure
    if (curNode->ruleComplete == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        // return expression of any procedure must be type int
        if (curNode->children[9]->type != "int") {
            throw runtime_error("Return type of procedure must be int.");
        }
    }

    // Check signature and return value of wain
    if (curNode->ruleComplete == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        // 2nd dcl var must be type int
        if (getType(curNode->children[5]->children[0], symbolTable) != "int") {
            throw runtime_error("Second parameter of wain must be of type int.");
        }
        // return expression of wain must be type int
        if (getType(curNode->children[11], symbolTable) != "int") {
            throw runtime_error("Return type of wain must be int.");
        }
    }

    // Check that the variable declaration type is the same as the declared value
    if (curNode->ruleComplete == "dcls dcls dcl BECOMES NUM SEMI") {
        if (getType(curNode->children[1]->children[0], symbolTable) != "int") {
            throw runtime_error("Variable declared as int must be assigned an int value.");
        }
    }

    if (curNode->ruleComplete == "dcls dcls dcl BECOMES NULL SEMI") {
        if (getType(curNode->children[1]->children[0], symbolTable) != "int*") {
            throw runtime_error("Variable declared as int* must be assigned a NULL value.");
        }
    }

    if (curNode->ruleComplete == "factor ID LPAREN RPAREN") {
        string procedureName = curNode->getLexeme(curNode->children[0]);
        // Check that the procedure exists
        if (table.count(procedureName) != 1) {
            throw runtime_error("Undefined procedure: " + procedureName);
        }
        // Check if there is a local variable with the same ID
        if (symbolTable->count(procedureName) == 1) {
            throw runtime_error("Identifier used as both variable and function 3: " + procedureName);
        }

        // Check that no args are expected
        if (table.count(procedureName) == 1 && !table[procedureName]->signature.empty()) {
            throw runtime_error("Procedure " + procedureName + " expects arguments, none provided.");
        }
    }

    if (curNode->ruleComplete == "factor ID LPAREN arglist RPAREN") {
        string procedureName = curNode->getLexeme(curNode->children[0]);
        // Check that the procedure exists
        if (table.count(procedureName) != 1) {
            throw runtime_error("Undefined procedure: " + procedureName);
        }
        // Check if there is a local variable with the same ID
        if (symbolTable->count(procedureName) == 1) {
            throw runtime_error("Identifier used as both variable and function 4: " + procedureName);
        }
        // Check that the provided args match the expected args
        const node* argList = curNode->children[2];
        vector<string> vectorArgList;

        const vector<string>& expectedArgList = table[procedureName]->signature;
        
        while (argList->children.size() != 1) {
            vectorArgList.emplace_back(argList->children[0]->type);
            argList = argList->children[2];
        }
        vectorArgList.emplace_back(argList->children[0]->type);

        if (vectorArgList.size() != expectedArgList.size()) {
            cout << "expected: " << expectedArgList.size() << ";" << "actualL: " << vectorArgList.size() << endl;
            throw runtime_error("Incorrect number of arguments for procedure: " + procedureName);
        }
        for (int i = 0; i < vectorArgList.size(); i++) {
            if (vectorArgList[i] != expectedArgList[i]) {
                throw runtime_error("Argument type mismatch for procedure: " + procedureName);
            }
        }
    }

    if (curNode->ruleComplete == "statement lvalue BECOMES expr SEMI") {
        if (getType(curNode->children[0], symbolTable) != getType(curNode->children[2], symbolTable)) {
            throw runtime_error("Type mismatch in assignment statement.");
        }
    }

    if (curNode->ruleComplete == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        if (getType(curNode->children[2]->children[0], symbolTable) != getType(curNode->children[2]->children[2], symbolTable)) {
            throw runtime_error("Type mismatch in IF statement condition.");
        }
    }

    if (curNode->ruleComplete == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        if (getType(curNode->children[2]->children[0], symbolTable) != getType(curNode->children[2]->children[2], symbolTable)) {
            throw runtime_error("Type mismatch in WHILE statement condition.");
        }
    }

    if (curNode->ruleComplete == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        if (getType(curNode->children[2], symbolTable) != "int") {
            throw runtime_error("PRINTLN expects an expression of type int.");
        }
    }

    if (curNode->ruleComplete == "statement DELETE LBRACK RBRACK expr SEMI") {
        if (getType(curNode->children[3], symbolTable) != "int*") {
            throw runtime_error("DELETE expects an expression of type int*.");
        }
    }
}



void getSignature(const node *paramlist, vector<string>& signature, unordered_map<string, string> *symbolTable) {
    if (paramlist->ruleComplete == "params paramlist") {
        getSignature(paramlist->children[0], signature, symbolTable);
    }
    else if (paramlist->ruleComplete == "paramlist dcl") {
        signature.push_back(getType(paramlist->children[0]->children[0], symbolTable));
    }
    else if (paramlist->ruleComplete == "paramlist dcl COMMA paramlist") {
        signature.push_back(getType(paramlist->children[0]->children[0], symbolTable));
        getSignature(paramlist->children[2], signature, symbolTable);
    }
    else {
        return;
    }
}

void traverseTree(node *tree) {
    // Find all procedures and add them all in the symbol table
    if (tree->ruleComplete == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        // cout << "Hihi" << endl;
       string procedureId = tree->getLexeme(tree->children[1]);

        if (table.count(procedureId) == 1) {    
            throw runtime_error("Duplicated procedure id");
        }
        unordered_map<string, string> symbolTable;
        unordered_map<string, string>* symbolTablePtr = &symbolTable;

        procedure *curProcedure = new procedure();

        table.insert({procedureId, curProcedure});
        getSignature(tree->children[3], curProcedure->signature, symbolTablePtr);
        //cout << "Current Procedure: " << procedureId << "Signature Size: " << curProcedure->signature.size() << endl;
        typesAndDeclarations(tree, symbolTablePtr);

    }
    else if (tree->ruleComplete == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        string procedureId = "wain";
        unordered_map<string, string> symbolTable;
        unordered_map<string, string>* symbolTablePtr = &symbolTable;

        procedure *curProcedure = new procedure();
        table.insert({procedureId, curProcedure});
        typesAndDeclarations(tree, symbolTablePtr);
    }
    else if (tree->ruleComplete == "procedures main") {
        traverseTree(tree->children[0]);
    }
    else if (tree->ruleComplete == "procedures procedure procedures") {
        traverseTree(tree->children[0]);
        traverseTree(tree->children[1]);
    }
    else {
        for (int i = 0; i < tree->children.size(); ++i) {
            traverseTree(tree->children[i]);
        }
    }
}

void buildTree(node *currentNode, string &inputLine, bool &used) {
    if ((used && getline(cin, inputLine)) || !used) {
        used = false;
    }
    else {
        return;
    }

    string word;
    istringstream getWord{inputLine};
    getWord >> word;

    if (currentNode->rule == word) {
        used = true;
        currentNode->ruleComplete = inputLine;
        while(getWord >> word) {
            node* child = new node{word};
            currentNode->children.push_back(child);
            buildTree(child, inputLine, used);
        }
    }
    else {
        return;
    }
}




int main () {
    string inputLine;
    string word;
    bool used = false;
    // The base case for the root
    getline(cin, inputLine);

    istringstream rootWord{inputLine};
    rootWord >> word;
    node *root =  new node{word};

    buildTree(root, inputLine, used);

    // After building the whole tree, traverse the tree and build the symbol table 
    try{
        traverseTree(root);
    }
    catch (const runtime_error &e) {
        //cerr << "Runtime error: " << e.what() << endl;
        cerr << "ERROR" << endl;
    } catch (const exception &e) {
        //cerr << "Exception: " << e.what() << endl;
        cerr << "ERROR" << endl;
    } catch (...) {
        //cerr << "Unknown error occurred." << endl;
        cerr << "ERROR" << endl;
    }

    printTree(root);
    delete root;

    for (auto it : table) {
        delete it.second;
    }
    table.clear();  // clear the table
}
