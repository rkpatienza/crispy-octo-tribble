#pragma once
#include <fstream>
#include <string>
#include <unordered_map>

enum commandType{
    A_COMMAND,
    C_COMMAND,
    L_COMMAND
};

/****************************************************************
the parser class parses a .asm file one line at a time.
for each line it reads, it will:
    - determine what command type it is
    - break command into underlying components (dest, comp, etc.)
continues to read until end of file
****************************************************************/
class Parser{
    private:
		// SymbolTable class declaration
		class SymbolTable {
		private:
			std::unordered_map<std::string, int> S;
			// std::unordered_map<std::string, int>::iterator it;
			int ramPtr;
		public:
			SymbolTable();
			void addSymbol(std::string symbol, int address);	// for (Xxx) labels assembler encounters
			bool contains(std::string symbol);	// checking if symbol is in the table
			int getAddress(std::string symbol);
			int getRAMPtr() { return ramPtr; }
			void incRAM() { ramPtr++; }
		};

		// data members
        std::ifstream asmFile;
		std::ofstream hackFile;
        std::string symbol;	// @Xxx or (Xxx)
        std::string dest;
        std::string comp;
        std::string jump;   // jump will initially hold the entire command
		std::string binaryCode;	// final binary code to be written to hack file
        commandType T;
		SymbolTable S;
		bool fileNotOpen(){ return !asmFile.is_open(); }
		std::string translateDest();
		std::string translateComp();
		std::string translateJump();
    public:
        Parser();
        ~Parser();
        bool hasMoreCommands() {return !asmFile.eof();}
		commandType getType() {return T;}
		std::string getSymbol() { return symbol; }	// numeric value
		std::string getDest() { return dest; }
		std::string getComp() { return comp; }
		std::string getJump() { return jump; }
		void firstPass();	// building symbol table S
        void advance();
        void displayInfo();
        void parseSymbol();
        void parseDest();
        void parseComp();
		void encode();
};