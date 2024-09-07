#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <ctime>
using namespace std;


int counter = 0;


struct procedure {
    vector<string> signature;
    unordered_map<string, pair<string, int>> variables; // symbol table the int is the offset that it's stored in the stack
    procedure() : signature{}, variables{} {}
};

struct registerTable{
    bool empty;
};

registerTable regArray[32];

// Function to initialize the register table
void initializeRegisterTable() {
    regArray[0].empty = false;
    regArray[1].empty = false;
    regArray[2].empty = false;
    regArray[3].empty = false;
    regArray[4].empty = false;
    regArray[5].empty = false;
    regArray[6].empty = false;
    regArray[7].empty = false;
    regArray[8].empty = true;
    regArray[9].empty = true;
    regArray[10].empty = true;
    regArray[11].empty = false;
    regArray[12].empty = true;
    regArray[13].empty = true;
    regArray[14].empty = true;
    regArray[15].empty = true;
    regArray[16].empty = true;
    regArray[17].empty = true;
    regArray[18].empty = true;
    regArray[19].empty = true;
    regArray[20].empty = true;
    regArray[21].empty = true;
    regArray[22].empty = true;
    regArray[23].empty = true;
    regArray[24].empty = true;
    regArray[25].empty = true;
    regArray[26].empty = true;
    regArray[27].empty = true;
    regArray[28].empty = true;
    regArray[29].empty = false;
    regArray[30].empty = false;
    regArray[31].empty = false;
}

int firstAvailable() {
    for (int i = 0; i < 32; ++i) {
        if (regArray[i].empty == true) {
            regArray[i].empty = false;
            return i;
        } 
    }
}

void setAvailable(int regNum) {
    regArray[regNum].empty = true;
}

// The key of the procedure is the id for the procedure
unordered_map<string, procedure*> table;

class node {
public:
    string rule;
    string ruleWoType;
    string ruleComplete;
    string type;
    vector<node*> children;

    node(string rule) : rule{rule}, ruleWoType{}, ruleComplete{}, type{}, children{} {}

    ~node() {
        for (int i = 0; i < this->children.size(); ++i) {
            delete this->children[i];
        }
    }
};

string getLexeme(const node *curNode) {
    return curNode->children[0]->rule;
}

string getType (const node *curNode) {
    return curNode->children[0]->type;
}

void getSignature(const node *paramlist, vector<string>& signature, unordered_map<string, pair<string, int>> *symbolTable) {
    if (paramlist->ruleComplete == "params paramlist") {
        getSignature(paramlist->children[0], signature, symbolTable);
    }
    else if (paramlist->ruleComplete == "paramlist dcl") {
        signature.push_back(paramlist->children[0]->children[0]->type);
    }
    else if (paramlist->ruleComplete == "paramlist dcl COMMA paramlist") {
        signature.push_back(paramlist->children[0]->children[0]->type);
        getSignature(paramlist->children[2], signature, symbolTable);
    }
    else {
        return;
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
        currentNode->ruleWoType += word;
        currentNode->ruleWoType += " ";

        while(getWord >> word && word != ":") {
            currentNode->ruleWoType += word;
            currentNode->ruleWoType += " ";

            node* child = new node{word};
            currentNode->children.push_back(child);
            buildTree(child, inputLine, used);
        }
        currentNode->ruleWoType = currentNode->ruleWoType.substr(0, (currentNode->ruleWoType.length() - 1));
        if (word == ":" && getWord >> word) {
            currentNode->type = word;
        }
    }
    else {
        return;
    }
}


// You dont need to keep track of what's in the stack, just need to generate the code to do the push and pull
void push(int regNum, int &offset) {
    cout << "sw $" << regNum << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    offset -= 4;
}

void pop(int regNum, int &offset) {
    cout << "add $30, $30, $4" << endl;
    cout << "lw $" << regNum << ", -4($30)" << endl;
    offset += 4;
}


void generateCode(node *curNode, procedure* curProcedure, int& offset) {
    if (curNode->ruleWoType == "dcl type ID") {
        cout << "lw $3, " << curProcedure->variables[getLexeme(curNode->children[1])].second << "($29)" << endl;
        return;
    }
    if (curNode->ruleWoType == "dcls .EMPTY") {
        return;
    }
    if (curNode->ruleWoType == "statements .EMPTY") {
        return;
    }
    else if (curNode->ruleWoType == "expr term") {
        generateCode(curNode->children[0], curProcedure, offset);
    }
    else if (curNode->ruleWoType == "term factor") {
        generateCode(curNode->children[0], curProcedure, offset);
    }
    else if (curNode->ruleWoType == "factor NUM") {
         cout << "lis $3" << endl;
         cout << ".word " << curNode->children[0]->children[0]->rule << endl; 
    }
    else if (curNode->ruleWoType == "factor ID") {
        cout << "lw $3, " << curProcedure->variables[getLexeme(curNode->children[0])].second << "($29)" << endl;
    }
    else if (curNode->ruleWoType == "factor LPAREN expr RPAREN") {
        generateCode(curNode->children[1], curProcedure, offset);
    }
    else if (curNode->ruleWoType == "dcls dcls dcl BECOMES NUM SEMI") {
        generateCode(curNode->children[0], curProcedure, offset);

        curProcedure->variables[getLexeme(curNode->children[1]->children[1])] = {getType(curNode->children[1]->children[1]), 0};
        curProcedure->variables[getLexeme(curNode->children[1]->children[1])].second = offset;

        cout << "lis $3" << endl;
        cout << ".word " << curNode->children[3]->children[0]->rule << endl; 
        push(3, offset);
    }
    else if (curNode->ruleWoType == "statements statements statement") {
        generateCode(curNode->children[0], curProcedure, offset);
        generateCode(curNode->children[1], curProcedure, offset);
    }
    else if (curNode->ruleWoType == "statement lvalue BECOMES expr SEMI") {
        node *curLvalue = curNode->children[0];

        while (curLvalue->ruleWoType == "lvalue LPAREN lvalue RPAREN") {
            curLvalue = curLvalue->children[1];
        }

        if (curLvalue->ruleWoType == "lvalue STAR factor") {
            generateCode(curNode->children[2], curProcedure, offset);
            //push(3, offset);
            int storeRegister = firstAvailable();

            cout << "add $" << storeRegister << ", $3, $0" << endl;
            generateCode(curLvalue->children[1], curProcedure, offset);
            // We would return an address where we should store expr at
            //pop(5, offset);
            cout << "sw $" << storeRegister << ", 0($3)" << endl;
            setAvailable(storeRegister);
        }
        else {
            // if the rule we have is lvalue ID
            // We first generate the result of the expressions,
            generateCode(curNode->children[2], curProcedure, offset);
            cout << "sw $3, ";
            // Then we generate the code for lvalue, where we print out the offset
            // However, since we are only updating the value of the lvalue, we dont change the offset here
            generateCode(curLvalue, curProcedure, offset);
            cout << "($29)" << endl;
        }
    }
    else if (curNode->ruleWoType == "lvalue ID") {
        cout << curProcedure->variables[getLexeme(curNode->children[0])].second;
    }
    else if (curNode->ruleWoType == "lvalue LPAREN lvalue RPAREN") {
        generateCode(curNode->children[1], curProcedure, offset);
    }
    else if (curNode->ruleWoType == "expr expr PLUS term") {
        if (curNode->children[0]->type == "int" && curNode->children[2]->type == "int*") {
            generateCode(curNode->children[0], curProcedure, offset);

            int storeRegister = firstAvailable();

            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            //push(3, offset);
            cout << "add $" << storeRegister << ", $3, $0" << endl;
            generateCode(curNode->children[2], curProcedure, offset);
            //pop(5, offset);
            cout << "add $3, $" << storeRegister << ", $3" << endl;
            setAvailable(storeRegister);
        }
        else if (curNode->children[0]->type == "int*" && curNode->children[2]->type == "int") {
            generateCode(curNode->children[0], curProcedure, offset);
            
            int storeRegister = firstAvailable();
            cout << "add $" << storeRegister << ", $3, $0" << endl;
            // push(3, offset);
            generateCode(curNode->children[2], curProcedure, offset);
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            //pop(5, offset);
            cout << "add $3, $" << storeRegister << ", $3" << endl;
            
            setAvailable(storeRegister);
        }
        else {
            // if they are both integers
            generateCode(curNode->children[0], curProcedure, offset);
            
            int storeRegister = firstAvailable();
            cout << "add $" << storeRegister << ", $3, $0" << endl;
            //push(3, offset);
            generateCode(curNode->children[2], curProcedure, offset);
            //pop(5, offset);
            cout << "add $3, $" << storeRegister << ", $3" << endl;
            
            setAvailable(storeRegister);
        }
    }
    else if (curNode->ruleWoType == "expr expr MINUS term") {
        if (curNode->children[0]->type == "int*" && curNode->children[2]->type == "int") {
            generateCode(curNode->children[0], curProcedure, offset);
            push(3, offset);
            generateCode(curNode->children[2], curProcedure, offset);
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            pop(5, offset);
            cout << "sub $3, $5, $3" << endl;
        }
        else if (curNode->children[0]->type == "int*" && curNode->children[2]->type == "int*") {
            generateCode(curNode->children[0], curProcedure, offset);
            push(3, offset);
            generateCode(curNode->children[2], curProcedure, offset);
            pop(5, offset);
            cout << "sub $3, $5, $3" << endl;
            cout << "div $3, $4" << endl;
            cout << "mflo $3" << endl;
        }
        else {
            // if they are both integers
            generateCode(curNode->children[0], curProcedure, offset);
            push(3, offset);
            generateCode(curNode->children[2], curProcedure, offset);
            pop(5, offset);
            // The one we pop to register is the value we subtract from
            cout << "sub $3, $5, $3" << endl;
        }
    }
    else if (curNode->ruleWoType == "term term STAR factor") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        //cout << "add $9, $3, $0" << endl;
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        cout << "mult $3, $5" << endl;
        cout << "mflo $3" << endl;
    }
    else if (curNode->ruleWoType == "term term SLASH factor") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        //cout << "add $10, $3, $0" << endl;
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
    }
    else if (curNode->ruleWoType == "term term PCT factor") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
    }
    else if (curNode->ruleWoType == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        int currentCounter = counter++;
        generateCode(curNode->children[2], curProcedure, offset);
        cout << "beq $3, $0, statementsElse" << currentCounter << endl;
        generateCode(curNode->children[5], curProcedure, offset);
        cout << "beq $0, $0, endIf" << currentCounter << endl;
        cout << "statementsElse" << currentCounter << ":" << endl;
        generateCode(curNode->children[9], curProcedure, offset);
        cout << "endIf" << currentCounter << ":" << endl;
        
    }
    else if (curNode->ruleWoType == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        int currentCounter = counter++;
        cout << "while" << currentCounter << ":" << endl;
        generateCode(curNode->children[2], curProcedure, offset);
        cout << "beq $3, $0, endWhile" << currentCounter << endl;
        generateCode(curNode->children[5], curProcedure, offset);
        cout << "beq $0, $0, while" << currentCounter << endl;
        cout << "endWhile" << currentCounter << ":" << endl;
    }
    else if (curNode->ruleWoType == "test expr EQ expr") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        // Maybe store $6 and $7 if needed 
        if (curNode->children[0]->type == "int" && curNode->children[2]->type == "int") {
            cout << "slt $6, $3, $5" << endl;
            cout << "slt $7, $5, $3" << endl;
        }
        else {
            cout << "sltu $6, $3, $5" << endl;
            cout << "sltu $7, $5, $3" << endl;
        }
        cout << "add $3, $6, $7" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
    else if (curNode->ruleWoType == "test expr NE expr") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        // Maybe store $6 and $7 if needed 
        if (curNode->children[0]->type == "int" && curNode->children[2]->type == "int") {
            cout << "slt $6, $3, $5" << endl;
            cout << "slt $7, $5, $3" << endl;
        }
        else {
            cout << "sltu $6, $3, $5" << endl;
            cout << "sltu $7, $5, $3" << endl;
        }
        cout << "add $3, $6, $7" << endl;
    }
    else if (curNode->ruleWoType == "test expr LT expr") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        if (curNode->children[0]->type == "int" && curNode->children[2]->type == "int") {
            cout << "slt $3, $5, $3" << endl;
        }
        else {
            cout << "sltu $3, $5, $3" << endl;
        }
    }
    else if (curNode->ruleWoType == "test expr LE expr") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        if (curNode->children[0]->type == "int" && curNode->children[2]->type == "int") {
            cout << "slt $3, $3, $5" << endl;
        }
        else {
            cout << "sltu $3, $3, $5" << endl;
        }
        cout << "sub $3, $11, $3" << endl;
    }
    else if (curNode->ruleWoType == "test expr GT expr") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        if (curNode->children[0]->type == "int" && curNode->children[2]->type == "int") {
            cout << "slt $3, $3, $5" << endl;
        }
        else {
            cout << "sltu $3, $3, $5" << endl;
        }
    }
    else if (curNode->ruleWoType == "test expr GE expr") {
        generateCode(curNode->children[0], curProcedure, offset);
        push(3, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        pop(5, offset);
        if (curNode->children[0]->type == "int" && curNode->children[2]->type == "int") {
            cout << "slt $3, $5, $3" << endl;
        }
        else {
            cout << "sltu $3, $5, $3" << endl;
        }
        cout << "sub $3, $11, $3" << endl;
    }
    else if (curNode->ruleWoType == "statement PUTCHAR LPAREN expr RPAREN SEMI") {
        generateCode(curNode->children[2], curProcedure, offset);
        cout << "lis $5" << endl;
        cout << ".word 0xffff000c" << endl;
        cout <<  "sw $3, 0($5)" << endl;
    }
    else if (curNode->ruleWoType == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        push(1, offset);
        generateCode(curNode->children[2], curProcedure, offset);
        cout << "add $1, $3, $0" << endl;
        push(31, offset);
        cout << "lis $5" << endl;
        cout << ".word print" << endl;
        cout << "jalr $5" << endl;
        pop(31, offset);
        pop(1, offset);
    }
    else if (curNode->ruleWoType == "factor GETCHAR LPAREN RPAREN") {
        cout << "lis $5" << endl;
        cout << ".word 0xffff0004" << endl;
        cout <<  "lw $3, 0($5)" << endl;
    }
    else if (curNode->ruleWoType == "factor ID LPAREN RPAREN") {
        string procedureId = getLexeme(curNode->children[0]);

        // we only need to load the id for the function that we are calling, since it's a label, we can just jump there
        cout << "add $27, $29, $0" << endl;
        cout << "add $28, $31, $0" << endl;
        //push(29, offset);
        // the caller need to perserve the return register
        //push(31, offset);

        cout << "lis $5" << endl;
        cout << ".word " << "procedure" << procedureId << endl;
        cout << "jalr $5" << endl;

        //pop(31, offset);
        //pop(29, offset); 
        cout << "add $29, $27, $0" << endl;
        cout << "add $31, $28, $0" << endl;
    }
    else if (curNode->ruleWoType == "factor ID LPAREN arglist RPAREN") {
        string procedureId = getLexeme(curNode->children[0]);
        // we only need to load the id for the function that we are calling, since it's a label, we can just jump there
        push(29, offset);
        // the caller need to perserve the return register
        push(31, offset);

        //cout << "add $25, $29, $0" << endl;
        //cout << "add $26, $31, $0" << endl;

        int numArgs = 0;
        node *traverseArglist = curNode->children[2];

        while (traverseArglist->ruleWoType == "arglist expr COMMA arglist") {
            generateCode(traverseArglist->children[0], curProcedure, offset);
            push(3, offset);
            traverseArglist = traverseArglist->children[2];
        }
        if (traverseArglist->ruleWoType == "arglist expr") {
            generateCode(traverseArglist->children[0], curProcedure, offset);
            push(3, offset);
        }
        cout << "lis $5" << endl;
        cout << ".word " << "procedure" << procedureId << endl;
        cout << "jalr $5" << endl;
        traverseArglist = curNode->children[2];
        while (traverseArglist->ruleWoType == "arglist expr COMMA arglist") {
            cout << "add $30, $30, $4" << endl;
            offset += 4;
            traverseArglist = traverseArglist->children[2];
        }
        if (traverseArglist->ruleWoType == "arglist expr") {
            cout << "add $30, $30, $4" << endl;
            offset += 4;
        }
        pop(31, offset);
        pop(29, offset); 
        //cout << "add $29, $25, $0" << endl;
        //cout << "add $31, $26, $0" << endl;
    }
    else if (curNode->ruleWoType == "params .EMPTY") {
        return;
    }
    else if (curNode->ruleWoType == "params paramlist") {
        // We need to store the offset of each variable into the table
        // The offset of the first variable is 4n

        int curOffset = curProcedure->signature.size() * 4;
        node *paramlist = curNode->children[0];
        
        while (paramlist->ruleWoType == "paramlist dcl COMMA paramlist") {
            string variableId = getLexeme(paramlist->children[0]->children[1]);
            curProcedure->variables[variableId].second = curOffset;
            curOffset -= 4;
            paramlist = paramlist->children[2];
        }
        if (paramlist->ruleWoType == "paramlist dcl") {
            string variableId = getLexeme(paramlist->children[0]->children[1]);
            curProcedure->variables[variableId].second = curOffset;
            curOffset -= 4;
        }
    } 
    else if (curNode->ruleWoType == "dcls dcls dcl BECOMES NULL SEMI") {
        generateCode(curNode->children[0], curProcedure, offset);

        string variableId = getLexeme(curNode->children[1]->children[1]);
        curProcedure->variables[variableId].second = offset;       // It's not that the place we store the dcl is null, but the value we store is null?
        cout << "add $3, $0, $11" << endl;                                 // So we are storing a null pointer on the offset frame
                               
        push(3, offset);
    }
    else if (curNode->ruleWoType == "factor STAR factor") {
        generateCode(curNode->children[1], curProcedure, offset);
        cout << "lw $3, 0($3)" << endl;
    }
    else if (curNode->ruleWoType == "factor NULL") {
        // Make the address of null to be invalid so that NULL would crash
        cout << "add $3, $0, $11" << endl;
    }
    else if (curNode->ruleWoType == "factor AMP lvalue") {
        node *curLvalue = curNode->children[1];

        while (curLvalue->ruleWoType == "lvalue LPAREN lvalue RPAREN") {
            curLvalue = curLvalue->children[1];
        }

        if (curLvalue->ruleWoType == "lvalue ID") {
            string lvalueId = getLexeme(curLvalue->children[0]);
            int lvalueAddress = curProcedure->variables[lvalueId].second;
            cout << "lis $3" << endl;
            cout << ".word ";
            generateCode(curLvalue, curProcedure, offset);
            cout << endl;
            // add the offset frame to the returning address
            cout << "add $3, $3, $29" << endl;
        }
        else {
            // curNode->ruleWoType == "lvalue STAR factor"
            generateCode(curLvalue->children[1], curProcedure, offset);
        }
    }
    else if (curNode->ruleWoType == "factor NEW INT LBRACK expr RBRACK") {
        generateCode(curNode->children[3], curProcedure, offset);
        cout << "add $1, $3, $0" << endl;
        
        push(31, offset);
        
        cout << "lis $5" << endl;
        cout << ".word new" << endl;
        cout << "jalr $5" << endl;
        
        pop(31, offset);
        
        cout << "bne $3, $0, 1" << endl;
        cout << "add $3, $11, $0" << endl;
    }
    else if (curNode->ruleWoType == "statement DELETE LBRACK RBRACK expr SEMI") {
        int currentCounter = counter++;

        generateCode(curNode->children[3], curProcedure, offset);
        cout << "beq $3, $11, skipDelete" << currentCounter << endl;
        cout << "add $1, $3, $0" << endl;
        push(31, offset);
        cout << "lis $5" << endl;
        cout << ".word delete" << endl;
        cout << "jalr $5" << endl;
        pop(31, offset);
        cout << "skipDelete" << currentCounter << ":" << endl;
    }
}

void generateCodeForProcedure(node *curNode) {
    if (curNode->ruleWoType == "start BOF procedures EOF") {
        generateCodeForProcedure(curNode->children[1]);
    }
    else if (curNode->ruleWoType == "procedures main") {
        generateCodeForProcedure(curNode->children[0]);
    }
    // There are a couple of operations we only need to do in the main function 
    else if (curNode->ruleWoType == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        string procedureId = getLexeme(curNode->children[1]);

        procedure *curProcedure = new procedure();
        table.insert({procedureId, curProcedure});

        cout << ".import print" << endl;
        cout << ".import init" << endl;
        cout << ".import new" << endl;
        cout << ".import delete" << endl;
        cout << "lis $4" << endl;
        cout << ".word 4" << endl;
        cout << "lis $10" << endl;
        cout << ".word print" << endl;
        cout << "lis $11" << endl;
        cout << ".word 1" << endl;
        cout << "sub $29, $30, $4" << endl;
        // Now we need to save the two parameters of the procedure
        // * Note that every time we save, we always save 4 prior to $30, so we set $29 to be 4 smaller than $30 so that we can just save the off set to be $29
        int offset = 0;
        
        cout << "sw $1, -4($30)" << endl;
        cout << "sub $30, $30, $4" << endl; 
        curProcedure->variables[getLexeme(curNode->children[3]->children[1])] = {getType(curNode->children[3]->children[1]), 0};
        curProcedure->variables[getLexeme(curNode->children[3]->children[1])].second = offset;
        offset -= 4;

        cout << "sw $2, -4($30)" << endl;
        cout << "sub $30, $30, $4" << endl; 
        curProcedure->variables[getLexeme(curNode->children[5]->children[1])] = {getType(curNode->children[5]->children[1]), 0};
        curProcedure->variables[getLexeme(curNode->children[5]->children[1])].second = offset;
        offset -= 4;

        if (curNode->children[3]->type == "int") {
            cout << "add $2, $0, $0" << endl;
        }
        
        push(31, offset);
        cout << "lis $5" << endl;
        cout << ".word init" << endl;
        cout << "jalr $5" << endl;
        pop(31, offset);

        // begin code
        generateCode(curNode->children[3], curProcedure, offset);
        generateCode(curNode->children[5], curProcedure, offset); 
        generateCode(curNode->children[8], curProcedure, offset); 
        generateCode(curNode->children[9], curProcedure, offset); 
        generateCode(curNode->children[11], curProcedure, offset); 


        // Pop the stack frame
        while (offset != 0) {
            cout << "add $30, $30, $4" << endl;
            offset += 4;
        }
        
        cout << "jr $31" << endl;
    }
    else if (curNode->ruleWoType == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        // General Procedures
        // Dont need any imports
        
        string procedureId = getLexeme(curNode->children[1]);

        procedure *curProcedure = new procedure();
        table.insert({procedureId, curProcedure});
        
        getSignature(curNode->children[3], curProcedure->signature, &curProcedure->variables);
        
        cout << "procedure" << procedureId << ":" << endl;
        // Need to update $29
        cout << "sub $29, $30, $4" << endl;
        int offset = 0;
        
        // here we need to store the offset in the symbol table, we will update the offset for each variables for each function call
        generateCode(curNode->children[3], curProcedure, offset); 
        generateCode(curNode->children[6], curProcedure, offset); 
        // Save registers (the caller saved the registers) 
        // push registers
        generateCode(curNode->children[7], curProcedure, offset);
        generateCode(curNode->children[9], curProcedure, offset);
        // Restore registers and stack and jr $31
        
        // Pop the stack frame
        while (offset != 0) {
            cout << "add $30, $30, $4" << endl;
            offset += 4;
        }
        
        cout << "jr $31" << endl;
    }
    else if (curNode->ruleWoType == "procedures procedure procedures") {
        // Generate the code for the main function first
        generateCodeForProcedure(curNode->children[1]);
        generateCodeForProcedure(curNode->children[0]);
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
    
    initializeRegisterTable();

    buildTree(root, inputLine, used);
    generateCodeForProcedure(root);

    delete root;

    for (auto it : table) {
        delete it.second;
    }
    table.clear();  // clear the table
}
    