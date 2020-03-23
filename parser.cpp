#include "parser.h"
#include <iostream>
#include <algorithm>

using namespace std;

// Parser class definition
Parser::Parser(){
	binaryCode = "";
	T = A_COMMAND;
    string filename;
    cout << "Enter the path of the .asm file: ";
    cin >> filename;
    asmFile.open(filename);
	if (fileNotOpen())
		return;
	string hackFilename = filename.substr(0, filename.length() - 3) + "hack";
	hackFile.open(hackFilename);
	firstPass();
}

Parser::~Parser(){
    asmFile.close();
	hackFile.close();
}

void Parser::advance(){
    if(!hasMoreCommands() || fileNotOpen())
        return;
    getline(asmFile, jump);
	// removing whitespaces
	jump.erase(remove_if(jump.begin(), jump.end(), isspace), jump.end());
	// ignore if only comment
	if (jump.substr(0,2) == "//" || jump == "")
		return;
	// remove comments to the side of the instruction
	size_t comment = jump.find("//");
	if (comment != string::npos)
		jump = jump.substr(0, comment);
	if (jump[0] == '@')
		T = A_COMMAND;
	else if (jump[0] == '(')
		T = L_COMMAND;
    else
        T = C_COMMAND;
	if (T != L_COMMAND)
	{
		parseSymbol();
		parseDest();
		parseComp();
		encode();
		// displayInfo(); // for debugging purposes
	}
}

// retrieve the dest portion of the C_COMMAND
void Parser::parseDest(){
    if(T != C_COMMAND)
        return;
    size_t equals = jump.find_first_of('=');
    // if can't find equals, make dest NULL
    if(equals == string::npos)
        dest = "";
    else{
        dest = jump.substr(0, equals);
        // remove dest from jump?
        jump = jump.substr(equals + 1, jump.length() - dest.length() - 1);
    }
    return;
}

// retrieve the comp portion and, if exists, jump portion
void Parser::parseComp(){
    if(T != C_COMMAND)
        return;
    size_t semcol = jump.find_first_of(';');
    // if can't find semicolon, take whole jump as comp
    if(semcol == string::npos){
        comp = jump;
        jump = "";
    }
    else{
        comp = jump.substr(0, semcol);
        // remove comp from jump?
        jump = jump.substr(semcol + 1, jump.length() - comp.length() - 1);
    }
    return;
}

// parses the constant symbol after '@' (need to expand later to add (Xxx))
void Parser::parseSymbol(){
	if (T == C_COMMAND)
		return;
	else if (T == A_COMMAND)
		jump = jump.substr(1, jump.length() - 1);
	else // L command
		jump = jump.substr(1, jump.length() - 2);
    return;
}

void Parser::displayInfo(){
	string type;
	if (T == A_COMMAND) type = "A Command";
	else if (T == C_COMMAND) type = "C Command";
	else type = "L Command";
    cout << "Command type: " << type << '\n';
    if(T == C_COMMAND){
        cout << "Destination: " << dest << '\n';
        cout << "Computation: " << comp << '\n';
        cout << "Jump condition: " << jump << '\n';
    }
    else if(T == A_COMMAND)
        cout << "Address/constant: " << jump << '\n';
	cout << endl;
}

void Parser::encode() {
	string binary = "";
	bool isDigit = true;
	unsigned int i = 0;
	if (T == A_COMMAND) {
		while (isDigit && i < jump.length()) {
			if (isdigit(jump[i]) == 0)
				isDigit = false;
			i++;
		}
		if (!isDigit) {
			if (S.contains(jump)) {			// look up symbol in table
				jump = to_string(S.getAddress(jump));
			}
			else {
				cout << "symbol not found! oops!\n";
				cout << "let's look for " << jump << "!\n";
				S.addSymbol(jump, S.getRAMPtr());
				jump = to_string(S.getRAMPtr());
				S.incRAM();
			}
		}
		int x = stoi(jump);
		while (x > 0) {
			if (x % 2 == 1)
				binary.insert(0,"1");
			else
				binary.insert(0,"0");
			x = x / 2;
		}
		while (binary.length() < 16)
			binary.insert(0, "0");
	}
	else     // C command
	{
		binary = "111";
		binary += translateComp();
		binary += translateDest();
		binary += translateJump();
	}
	hackFile << binary << endl;
	binary.clear();
}

string Parser::translateComp() {
	string binComp;
	if (comp.find('M') != string::npos)
		binComp += '1';	// a = 1
	else
		binComp += '0';	// a = 0
	if (comp == "0")
		binComp += "101010";
	else if (comp == "1")
		binComp += "111111";
	else if (comp == "-1")
		binComp += "111010";
	else if (comp == "D")
		binComp += "001100";
	else if (comp == "A" || comp == "M")
		binComp += "110000";
	else if (comp == "!D")
		binComp += "001101";
	else if (comp == "!A" || comp == "!M")
		binComp += "110001";
	else if (comp == "-D")
		binComp += "001111";
	else if (comp == "-A" || comp == "-M")
		binComp += "110011";
	else if (comp == "D+1")
		binComp += "011111";
	else if (comp == "A+1" || comp == "M+1")
		binComp += "110111";
	else if (comp == "D-1")
		binComp += "001110";
	else if (comp == "A-1" || comp == "M-1")
		binComp += "110010";
	else if (comp == "D+A" || comp == "D+M")
		binComp += "000010";
	else if (comp == "D-A" || comp == "D-M")
		binComp += "010011";
	else if (comp == "A-D" || comp == "M-D")
		binComp += "000111";
	else if (comp == "D&A" || comp == "D&M")
		binComp += "000000";
	else // D|A or D|M
		binComp += "010101";
	return binComp;
}

string Parser::translateDest() {
	if (dest == "")
		return "000";
	else if (dest == "M")
		return "001";
	else if (dest == "D")
		return "010";
	else if (dest == "MD")
		return "011";
	else if (dest == "A")
		return "100";
	else if (dest == "AM")
		return "101";
	else if (dest == "AD")
		return "110";
	else // AMD
		return "111";
}

string Parser::translateJump() {
	if (jump == "")
		return "000";
	else if (jump == "JGT")
		return "001";
	else if (jump == "JEQ")
		return "010";
	else if (jump == "JGE")
		return "011";
	else if (jump == "JLT")
		return "100";
	else if (jump == "JNE")
		return "101";
	else if (jump == "JLE")
		return "110";
	else // jump = "JMP"
		return "111";
}

void Parser::firstPass() {
	if (fileNotOpen())
		return;
	int romPtr = 0;
	size_t comment;
	while (hasMoreCommands()) {
		getline(asmFile, jump);
		// removing whitespaces
		jump.erase(remove_if(jump.begin(), jump.end(), isspace), jump.end());
		// ignore if only comment
		if (jump.substr(0, 2) != "//" && jump != "") {
			// remove comments to the side
			comment = jump.find("//");
			if (comment != string::npos)
				jump = jump.substr(0, comment);
			if (jump[0] == '(') {
				jump = jump.substr(1, jump.length() - 2);
				if (!S.contains(jump))
					S.addSymbol(jump, romPtr);
			}
			else
				romPtr++;
		}
	}
	// go back to beginning of file
	asmFile.clear();
	asmFile.seekg(0, ios::beg);
}

// SymbolTable class definition
Parser::SymbolTable::SymbolTable() {
	ramPtr = 16;
	// adding predefined symbols
	S.insert(pair<string,int>("SP",0));
	S.insert(pair<string, int>("LCL", 1));
	S.insert(pair<string, int>("ARG", 2));
	S.insert(pair<string, int>("THIS", 3));
	S.insert(pair<string, int>("THAT", 4));
	string x;	// Ri
	for (int i = 0; i < 16; i++) {
		x = "R" + to_string(i);
		S.insert(pair<string, int>(x, i));
		x.clear();
	}
	S.insert(pair<string, int>("SCREEN", 16384));
	S.insert(pair<string, int>("KBD", 24576));
}

bool Parser::SymbolTable::contains(string symbol) {
	if (S.find(symbol) == S.end())
		return false;
	else
		return true;
}

int Parser::SymbolTable::getAddress(string symbol) {
	return S.at(symbol);
}

void Parser::SymbolTable::addSymbol(string symbol, int address) {
	if (contains(symbol) == true)
		return;
	S.insert(pair<string, int>(symbol, address));
}
