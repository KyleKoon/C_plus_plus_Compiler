/*
	Author: Kyle Koon
	Date: 9/24/20
	On my honor, I neither gave nor received unauthorized aid on this assignment
	Description: This program reads a file that the user picks and executes the SML code in that file
*/


#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <array>
#include <cmath>
#include "Assembler.h"
using namespace std;


void Assembler::RunAssembler() {

	cout << "Would you like to write to a file (yes/no): " << endl;
	string write;
	cin >> write;
	if (write == "yes") {
		ofstream myFile("test1.txt"); //creates the output file
		if (myFile.is_open()) {
			string text;
			while (text != "quit") {
				cout << "Enter an sml code and then press enter. Type 'quit' to stop: " << endl; //allows user to write sml code to the file
				cin >> text;
				if (text != "quit") {
					myFile << text << "\n"; //writes the user's input to the file
				}
			}
			myFile.close();
		}
		else {
			cout << "Unable to open file";
		}
	}

	cout << "Would you like to process a file (yes/no): " << endl;
	string process;
	cin >> process;

	if (process == "yes") {
		cout << "Enter the name of the file to be processed: "; //allows user to pick a .txt file to run
		string filename;
		cin >> filename;
		ifstream inFile(filename, ios::in); //opens the file so that we can read from it
		if (!inFile) {
			cerr << "File not able to be opened" << endl;
			exit(EXIT_FAILURE);
		}

		int command;   //holds the 4 digit sml command
		array<int, 100> code; //stores all the sml commands and data
		inFile >> command; //reads a line from the file
		int index = 0;
		bool sizeError = false;
		while (!inFile.eof() && index != 100)     //you can go to end of file and ignore the 0 values 
		{
			code[index] = command; //puts the line from the file into our code array so that we can process it later
			index++;
			inFile >> command; //reads the next line
		}
		if (index == 100) { //there are more than 100 lines of sml instruction codes
			cout << "File is too large to process" << endl;
			sizeError = true;
		}

		//codes for the different switch scenarios
		const int READ = 10;
		const int WRITE = 11;
		const int LOAD = 20;
		const int STORE = 21;
		const int LOADLIT = 22;
		const int ADD = 30;
		const int SUBTRACT = 31;
		const int DIVIDE = 32;
		const int MULTIPLY = 33;
		const int POWEROF = 34;
		const int BRANCH = 40;
		const int BRANCHNEGATIVE = 41;
		const int BRANCHZERO = 42;
		const int HALT = 43;

		int line = 0;
		int address = 0;
		int cmd = 0;
		int accumulator = 0;

		int i = 0;
		int lowestAdr = 100;
		bool memError = false;
		bool numToLarge = false;
		while (i < index && !memError && !sizeError && !numToLarge) { //runs until we reach the last index of sml code
			line = code[i]; //gets a sml code from the code array
			address = line % 100; //the address is the last two digits of the 4 digit code
			cmd = (line - address) / 100; //the command is the first two digits of the 4 digit code

			if (address < lowestAdr && (cmd == 21 || cmd == 10)) {
				lowestAdr = address;
			}

			if (lowestAdr <= index) { //where our intstructions are being stored and where our variables are stored have crossed paths
				cout << "You have run out of memory";
				memError = true;
				break;
			}

			switch (cmd) {

			case READ: {
				int value;
				cout << "Enter an integer: ";
				cin >> value;
				code[address] = value; //stores the user's value into the correct spot in the code array
			}
					   break;

			case WRITE: {
				cout << code[address] << endl; //outputs the data
			}
						break;


			case LOAD: {
				accumulator = code[address]; //loads the data into the accumulator
			}
					   break;


			case STORE: {
				code[address] = accumulator; //stores the accumulators value into a spot in the code array
			}
						break;

			case LOADLIT: {
				accumulator = address;
			}
						  break;


			case ADD: {
				accumulator = accumulator + code[address]; //adds data from a specific spot in the code array to the current accumulator value and updates the accumulator value
			}
					  break;

			case SUBTRACT: {
				accumulator = accumulator - code[address]; //subtracts data from a specific spot in the code array from the current accumulator value and updates the accumulator value
			}
						   break;

			case DIVIDE: {
				accumulator = accumulator / code[address]; //divides the current accumulator value by data from a specific spot in the code array
			}
						 break;

			case MULTIPLY: {
				accumulator = accumulator * code[address]; //multiplies the current accumulator value by data from a specific spot in the code array
			}
						   break;

			case POWEROF: {
				int baseNum = accumulator;
				accumulator = pow(baseNum, code[address]); //Takes the power ofs the current accumulator value by data from a specific spot in the code array
				int badNum = 2147483648;
				if (accumulator == (badNum/-1)) {
					numToLarge = true;
					cout << "The number is too large to process!" << endl;
				}
			}
						  break;

			case BRANCH: {
				i = address - 1; //sets the index value i to the desired address - 1. The minus 1 exists because i is incremented at the end of the switch. -1 and +1 negate.
			}
						 break;

			case BRANCHNEGATIVE: {
				if (accumulator < 0) { //sets the index value i to the desired address only if the accumulator's current value is negative
					i = address - 1;
				}
			}
								 break;

			case BRANCHZERO: {
				if (accumulator == 0) {  //sets the index value i to the desired address only if the accumulator's current value is 0
					i = address - 1;
				}
			}
							 break;

			case HALT: {
				i = index; //sets i = index so that the program will exit the while loop and the program will be terminated
			}
					   break;

			} // end of switch

			i++; //increments the current address i by 1 so that we can read the next line of code
		}
		cin.get(); //keeps the output window open until the user presses any key
		cin.get();
	}
}
