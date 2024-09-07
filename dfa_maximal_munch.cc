#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

const std::string ALPHABET = ".ALPHABET";
const std::string STATES = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT = ".INPUT";
const std::string EMPTY = ".EMPTY";

bool isChar(std::string s) {
    return s.length() == 1;
}

bool isRange(std::string s) {
    return s.length() == 3 && s[1] == '-';
}

int main() {
    std::istream& in = std::cin;
    std::string s;
    std::unordered_set<char> alphabet;
    std::unordered_map<std::string, bool> states;
    std::string INITIAL_STATE = "";
    std::unordered_map<std::string, std::unordered_map<char, std::string>> transitions;

    std::getline(in, s);
    while (in >> s) {
        if (s == STATES) { 
            break; 
        } else {
            if (isChar(s)) {
                alphabet.insert(s[0]);
            } else if (isRange(s)) {
                for (char c = s[0]; c <= s[2]; ++c) {
                    alphabet.insert(c);
                }
            } 
        }
    }

    std::getline(in, s);
    while (in >> s) {
        if (s == TRANSITIONS) { 
            break; 
        } else {
            static bool initial = true;
            bool accepting = false;
            if (s.back() == '!' && !isChar(s)) {
                accepting = true;
                s.pop_back();
            }
            if (initial) {
                INITIAL_STATE = s;
                initial = false;
            }
            if (accepting) {
                states[s] = true;
            } else {
                states[s] = false;
            }
        }
    }

    std::getline(in, s);
    while (std::getline(in, s)) {
        if (s == INPUT) { 
            break; 
        } else {
            std::string fromState, symbols, toState;
            std::istringstream line(s);
            line >> fromState;
            bool last;
            while (line >> s) {
                if (line.peek() == EOF) {
                    toState = s;
                } else {
                    if (isChar(s)) {
                        symbols += s;
                    } else if (isRange(s)) {
                        for (char c = s[0]; c <= s[2]; ++c) {
                            symbols += c;
                        }
                    }
                }
            }
            for (char c : symbols) {
                transitions[fromState][c] = toState;
            }
        }
    }

    in >> s;
    std::string curState = INITIAL_STATE;
    std::string curToken = "";
    int curIndex = 0;
    while (curIndex < s.length()) {
        char c = s[curIndex];
        if (alphabet.count(c) == 0) {
            std::cerr << "ERROR" << std::endl;
            return 0;
        }
        if (transitions.count(curState) == 0 || transitions[curState].count(c) == 0) {
            if (states.count(curState) && states[curState]) {
                std::cout << curToken << std::endl;
                curToken = "";
                curState = INITIAL_STATE;
                continue;
            }
            std::cerr << "ERROR" << std::endl;
            return 0;
        }
        curState = transitions[curState][c];
        curToken += c;
        curIndex += 1;
    }
    if (!states.count(curState) || !states[curState]) {
        std::cerr << "ERROR" << std::endl;
        return 0;
    }
    std::cout << curToken << std::endl;
}
    