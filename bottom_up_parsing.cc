#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include "wlp4data.h"

using namespace std;

struct ProductionRule {
    string lhs;
    int rhsLength;
    string rule;
};

struct Token {
    string type;
    string lexeme;
};

struct ParseNode {
    string type;
    string lexeme;
    string rule;
    vector<ParseNode*> children;

    // Destructor to clean up allocated memory
    ~ParseNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

void printParseTree(const ParseNode* node) {
    if (node->children.empty()) {
        cout << node->type << " " << node->lexeme << endl;
    } else {
        cout << node->rule << endl;
        for (int i = node->children.size() - 1; i >= 0; --i) {
            printParseTree(node->children[i]);
        }
    }
}

void cleanUpParseStack(deque<ParseNode*>& parseStack) {
    while (!parseStack.empty()) {
        delete parseStack.front();
        parseStack.pop_front();
    }
}

int main() {
    istringstream configStream {WLP4_COMBINED};
    string line;

    vector<ProductionRule> productionRules;
    deque<Token> inputSequence;
    deque<ParseNode*> parseStack;
    deque<int> stateStack;
    map<int, map<string, int>> transitionTable;
    map<int, map<string, int>> reductionTable;

    // Load production rules
    getline(configStream, line); // Skip CFG header
    while (getline(configStream, line) && line != ".TRANSITIONS") {
        istringstream lineStream {line};
        string lhs, symbol;
        int rhsLength = 0;
        lineStream >> lhs;
        while (lineStream >> symbol && symbol != ".EMPTY") {
            ++rhsLength;
        }
        productionRules.push_back({lhs, rhsLength, line});
    }

    // Load transitions
    while (configStream >> line && line != ".REDUCTIONS") {
        int fromState;
        string symbol;
        int toState;
        fromState = stoi(line);
        configStream >> symbol >> toState;
        transitionTable[fromState][symbol] = toState;
    }

    // Load reductions
    while (configStream >> line && line != ".END") {
        int state;
        int ruleIndex;
        string lookahead;
        state = stoi(line);
        configStream >> ruleIndex >> lookahead;
        reductionTable[state][lookahead] = ruleIndex;
    }

    // Read input sequence from stdin
    istream& in = cin;
    string tokenType, lexeme;
    inputSequence.push_back({"BOF", "BOF"});
    while (in >> tokenType >> lexeme) {
        inputSequence.push_back({tokenType, lexeme});
    }
    inputSequence.push_back({"EOF", "EOF"});

    stateStack.push_front(0);
    int read_tokens = 0;

    while (!inputSequence.empty()) {
        Token currentToken = inputSequence.front();

        // Perform reductions
        while (reductionTable[stateStack.front()].count(currentToken.type)) {
            int ruleIndex = reductionTable[stateStack.front()][currentToken.type];
            ProductionRule rule = productionRules[ruleIndex];

            vector<ParseNode*> children;
            for (int i = 0; i < rule.rhsLength; ++i) {
                children.push_back(parseStack.front());
                parseStack.pop_front();
                stateStack.pop_front();
            }

            parseStack.push_front(new ParseNode{rule.lhs, ".EMPTY", rule.rule, children});
            stateStack.push_front(transitionTable[stateStack.front()][rule.lhs]);
        }

        parseStack.push_front(new ParseNode{currentToken.type, currentToken.lexeme});
        inputSequence.pop_front();

        if (!transitionTable[stateStack.front()].count(currentToken.type)) {
            cerr << "ERROR at " << read_tokens << endl;
            cleanUpParseStack(parseStack);
            return 5;
        }

        stateStack.push_front(transitionTable[stateStack.front()][currentToken.type]);
        ++read_tokens;
    }

    // Check for acceptance
    if (reductionTable[stateStack.front()].count(".ACCEPT")) {
        int ruleIndex = reductionTable[stateStack.front()][".ACCEPT"];
        ProductionRule rule = productionRules[ruleIndex];

        vector<ParseNode*> children;
        for (int i = 0; i < rule.rhsLength; ++i) {
            children.push_back(parseStack.front());
            parseStack.pop_front();
            stateStack.pop_front();
        }

        parseStack.push_front(new ParseNode{"start", "start", rule.rule, children});
        stateStack.push_front(transitionTable[stateStack.front()][rule.lhs]);
    } else {
        cerr << "ERROR at " << read_tokens << endl;
        cleanUpParseStack(parseStack);
        return 5;
    }

    printParseTree(parseStack.front());
    delete parseStack.front(); // Clean up the final parse tree root node

    return 0;
}
