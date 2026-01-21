#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stack>
#include <sstream>
using namespace std;

struct Production {
    string left;
    string right;
};

struct Grammar {
    vector<Production> productii;
    map<int, map<string, string>> tabelActiuni;
    map<int, map<string, int>> tabelSalt;
};

struct StackElement {
    string symbol;
    int state;
};

Grammar gr;
stack<string> attributeStack;


int tempCount = 0;

string newTemp() {
    return "t" + to_string(tempCount++);
}

void emit(const string& s) {
    cout << s << endl;
}


vector<string> split(string str) {
    vector<string> result;
    stringstream ss(str);
    string word;
    while (ss >> word) {
        result.push_back(word);
    }
    return result;
}

void initializeGrammar(string filenameG1) {
    ifstream fin(filenameG1);
    string line;

    while (getline(fin, line)) {
        if (line == "") continue;

        int arrow = line.find("->");
        if (arrow != -1) {
            string left = line.substr(0, arrow);
            string right = line.substr(arrow + 2);
            gr.productii.push_back({ left, right });
        }
    }
    fin.close();
}

void initializeTables(string filenameTA, string filenameTS) {
    ifstream finTA(filenameTA);
    string line;

    while (getline(finTA, line)) {
        if (line == "") continue;
        vector<string> parts = split(line);
        int stare = stoi(parts[0]);
        gr.tabelActiuni[stare][parts[1]] = parts[2];
    }
    finTA.close();

    ifstream finTS(filenameTS);
    while (getline(finTS, line)) {
        if (line == "") continue;
        vector<string> parts = split(line);
        int stare = stoi(parts[0]);
        int stareNoua = stoi(parts[2]);
        gr.tabelSalt[stare][parts[1]] = stareNoua;
    }
    finTS.close();
}

void printStack(stack<StackElement> stiva) {
    vector<StackElement> elements;
    while (!stiva.empty()) {
        elements.push_back(stiva.top());
        stiva.pop();
    }

    //cout << "$";
    for (int i = elements.size() - 1; i >= 0; i--) {
        cout << elements[i].symbol << elements[i].state;
    }
}

vector<string> tokenize(string input) {
    vector<string> tokens;
    int i = 0;

    while (i < input.length()) {
        if (isspace(input[i])) {
            i++;
            continue;
        }

        if (i + 1 < input.length() && input[i] == 'i' && input[i + 1] == 'd') {
            int start = i;
            i += 2;

            while (i < input.length() && isdigit(input[i])) {
                i++;
            }

            tokens.push_back(input.substr(start, i - start)); // ex: id23
        }
        else {
            tokens.push_back(string(1, input[i]));
            i++;
        }
    }

    return tokens;
}


int countSymbols(string right) {
    int count = 0;
    int i = 0;
    while (i < right.length()) {
        if (i + 1 < right.length() && right[i] == 'i' && right[i + 1] == 'd') {
            count++;
            i += 2;
        }
        else if (right[i] != ' ') {
            count++;
            i++;
        }
        else {
            i++;
        }
    }
    return count;
}

void printAttributeStack(stack<string> stiva) {
    vector<string> elems;
    while (!stiva.empty()) {
        elems.push_back(stiva.top());
        stiva.pop();
    }

    cout << "[ ";
    for (int i = elems.size() - 1; i >= 0; i--) {
        cout << elems[i] << " ";
    }
    cout << "]";
}


bool analyze(string input) {
    stack<StackElement> stiva;
    stiva.push({ "$", 0 });


    vector<string> tokens = tokenize(input);
    tokens.push_back("$");
    int pos = 0;

    cout << "\n=== Analiza sirului: " << input << " ===\n\n";
    cout << "Stiva APD\t\tStiva Atribute\t\tInput\t\tActiune\n";
    cout << "---------------------------------------------------------------------\n";


    while (true) {
        int stare = stiva.top().state;
        string simbol = (tokens[pos].substr(0, 2) == "id") ? "id" : tokens[pos];


        printStack(stiva);
        cout << "\t\t";

        printAttributeStack(attributeStack);
        cout << "\t\t";

        for (int i = pos; i < tokens.size(); i++) {
            cout << tokens[i];
        }


        cout << "\t\t";

        if (gr.tabelActiuni[stare].find(simbol) == gr.tabelActiuni[stare].end()) {
            cout << "EROARE!\n";
            return false;
        }

        string actiune = gr.tabelActiuni[stare][simbol];
        cout << actiune << "\n";

        if (actiune[0] == 'd') {
            int stareNoua = stoi(actiune.substr(1));
            stiva.push({ simbol, stareNoua });

            if (simbol == "id") {
                attributeStack.push(tokens[pos]);
            }


            pos++;
        }
        else if (actiune[0] == 'r') {
            int nrProductie = stoi(actiune.substr(1));
            Production prod = gr.productii[nrProductie - 1];

            if (nrProductie == 1) { // E -> E + T
                string T = attributeStack.top(); attributeStack.pop();
                string E = attributeStack.top(); attributeStack.pop();

                string temp = newTemp();
                emit(temp + " := " + E + " + " + T);

                attributeStack.push(temp);
            }
            else if (nrProductie == 3) { // T -> T * F
                string F = attributeStack.top(); attributeStack.pop();
                string T = attributeStack.top(); attributeStack.pop();

                string temp = newTemp();
                emit(temp + " := " + T + " * " + F);

                attributeStack.push(temp);
            }
            else if (nrProductie == 6) { // F -> id
                // nimic
            }

            int n = countSymbols(prod.right);
            for (int i = 0; i < n; i++) {
                stiva.pop();
            }

            int stareAnterioara = stiva.top().state;
            int stareNoua = gr.tabelSalt[stareAnterioara][prod.left];
            stiva.push({ prod.left, stareNoua });
        }
        else if (actiune == "acc") {
            cout << "\nSirul este CORECT!\n";
            return true;
        }
    }
    return false;
}

int main() {
    initializeGrammar("G1.txt");
    initializeTables("TA.txt", "TS.txt");

    string input;
    cout << "Introduceti sirul de analizat (folositi 'id' pentru identificatori): ";
    cin >> input;

    analyze(input);

}