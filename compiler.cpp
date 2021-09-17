/* Authors: Kyle Koon and Matthew Jett
   Date: 9/29/20
   On my honor, I neither gave nor received unauthorized aid on this assignment
   Description: This program serves as the compiler that works with the SML assembler. It takes in a text file written in the SIMPLE language and outputs the appropriate SML code in a text file.
*/

#include <iostream>
#include <string.h>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <array>
#include <stdio.h>
#include <sstream>
#include <stack>
#include "Assembler.h"
using namespace std;

int CurrentDataCounter = 99;
int CurrentInstructionPointer = 0;
int CurrentSYMCounter = 0;
string inputFileText[1000][20];
string output[1000];
int parallel[1000] = {};
string symboltable[100][3];
int expNum = 1;
string initVars[100];
int initVarIndex = 0;
int varArrayIndex = 0;


string getAdr(string symbol, string symboltable[][3], int CurrentSYMCounter) { //gets the address of a variable/literal/expression from the symboltable
	for (int a = 0; a < CurrentSYMCounter; a++) {
		if (symboltable[a][0] == symbol && symboltable[a][1] != "L") {
			return symboltable[a][2];
		}
	}
	return "NULL"; //returns NULL if the variable/literal/expression has no address (it is not in the symboltable)
}
string getLAdr(string symbol, string symboltable[][3], int CurrentSYMCounter) { //gets the address of a variable/literal/expression from the symboltable
	for (int a = 0; a < CurrentSYMCounter; a++) {
		if (symboltable[a][0] == symbol && symboltable[a][1] == "L") {
			return symboltable[a][2];
		}
	}
	return "NULL"; //returns NULL if the variable/literal/expression has no address (it is not in the symboltable)
}

int getPrec(string Operator) { //returns the precedence value for various operators. This is necessary for the postfix conversion

	if (Operator == "^") {
		return 3;
	}

	else if (Operator == "*" || Operator == "/") {
		return 2;
	}

	else if (Operator == "+" || Operator == "-") {
		return 1;
	}
}


bool isComparison(string item) {  //tests if a string is a comparison symbol. This is necesssary for the if evaluation
	string comparisons[] = { "<", "<=", ">", ">=", "==", "!=" };

	int length = sizeof(comparisons) / sizeof(comparisons[0]);
	int i = 0;

	for (int i = 0; i < length; i++) {
		if (item == comparisons[i]) {
			return true;
		}
	}
	return false;
}


bool isOperator(string item) {  //tests if a string is an operator. This is necesssary for the postfix conversion
	string operators[] = { "(", ")", "^", "*", "/", "+", "-" };

	int length = sizeof(operators) / sizeof(operators[0]);
	int i = 0;

	for (int i = 0; i < length; i++) {
		if (item == operators[i]) {
			return true;
		}
	}
	return false;
}


void infixToPostfix(string s[], string output[]) { //converts infix string s into postfix string output

	int i = 0;
	int a = 0;

	stack <string> stk;

	while (s[i] != "") { //reads through the entirety of s until we get to a blank space

		if (!isOperator(s[i])) { //variables/literals are immediately put into the output array
			output[a] = s[i];
			a++;
		}
		else { //operators must be evaluated and stacked based on precedence
			if (stk.empty()) {
				stk.push(s[i]); //puts the current operator onto the stack because the stack is empty
			}

			else if (stk.top() == "(") {
				stk.push(s[i]); //puts an open parentheses onto the stack no matter what
			}

			else if (s[i] == ")") { //puts the operators between the ( and ) into the output array
				while (stk.top() != "(") {
					output[a] = stk.top();
					stk.pop();
					a++;
				}
				stk.pop();
			}

			else if (s[i] != "") {
				int prec1 = getPrec(s[i]); //gets the precedence of the newest operator
				int prec2 = getPrec(stk.top()); //gets the precedence of the top operator on the stack
				if (prec1 > prec2) {
					stk.push(s[i]);
				}
				else {
					while (!stk.empty()) { //puts all the operators in the stack into the output array by popping off the tops
						output[a] = stk.top();
						stk.pop();
						a++;
					}
					stk.push(s[i]); //puts the newest operator onto the stack
				}
			}
		}
		i++; //checks the next index of the infix string
	}

	//runs after checking all indeces of the infix string
	while (!stk.empty()) { //puts all the operators in the stack into the output array by popping off the tops
		output[a] = stk.top();
		stk.pop();
		a++;
	}
}



void calculate(string expr[], int Explength, string storeAdr, string variable) { //stores literals, converts infix to postfix, and then creates the necessary sml output

	for (int k = 0; k < Explength; k++) {
		if (!isOperator(expr[k])) { //finds the variables/literals in the expression (not operators)
			string adr = getAdr(expr[k], symboltable, CurrentSYMCounter); //gets the address of the current variable/literal
			if (adr == "NULL") { //runs if the current variable/literal does not exist yet
				if (stoi(expr[k]) < 10) { //checks if the literal is one digit
					output[CurrentInstructionPointer] = "220" + expr[k]; //load lit command with the single digit literal
					CurrentInstructionPointer++;
				}
				else {
					output[CurrentInstructionPointer] = "22" + expr[k]; //load lit command with the two digit literal
					CurrentInstructionPointer++;
				}
				output[CurrentInstructionPointer] = "21" + to_string(CurrentDataCounter); //store command into next available address 
				CurrentInstructionPointer++;

				symboltable[CurrentSYMCounter][0] = expr[k];
				symboltable[CurrentSYMCounter][1] = "C";
				symboltable[CurrentSYMCounter][2] = to_string(CurrentDataCounter);
				CurrentSYMCounter++;
				CurrentDataCounter--;
			}
		}
	}


	string PostFix[100]; //will store the converted postfix expression
	infixToPostfix(expr, PostFix); //converts infix to postfix and stores in PostFix

	int w = 0;
	while (PostFix[w] != "") { //counts how many items are in the postfix array
		w++;
	}


	for (int i = 0; i < w; i++) { //loops through the PostFix array
		if (w == 1) { //runs if the postfix expression is just one value
			string Adr = getAdr(PostFix[i], symboltable, CurrentSYMCounter);

			output[CurrentInstructionPointer] = "20" + Adr;
			CurrentInstructionPointer++;

			if (storeAdr != "NULL") { //runs if the storage address exists (the variable is already stored somewhere)
				output[CurrentInstructionPointer] = "21" + storeAdr;
			}
			else {
				output[CurrentInstructionPointer] = "21" + to_string(CurrentDataCounter); //stores the variable at the next available address
				symboltable[CurrentSYMCounter][0] = variable;
				symboltable[CurrentSYMCounter][1] = "var";
				symboltable[CurrentSYMCounter][2] = to_string(CurrentDataCounter);

				CurrentDataCounter--;
				CurrentSYMCounter++;

			}
			CurrentInstructionPointer++;
		}
		else if (!isOperator(PostFix[i])) { //checks the literals and variables in the postfix array
			string Adr = getAdr(PostFix[i], symboltable, CurrentSYMCounter);

			output[CurrentInstructionPointer] = "20" + Adr; //loads the variable/literal
			CurrentInstructionPointer++;

			output[CurrentInstructionPointer] = "21" + to_string(CurrentDataCounter); //stores the variable/literal into the next available address
			CurrentInstructionPointer++;

			symboltable[CurrentSYMCounter][0] = "val" + PostFix[i];
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentDataCounter);
			CurrentSYMCounter++;
			CurrentDataCounter--;
		}
		else { //checks the operators
			CurrentDataCounter += 2;
			string FirstAdr = to_string(CurrentDataCounter); //address of first operand
			CurrentDataCounter--;
			string SecondAdr = to_string(CurrentDataCounter); //address of second operand
			string operation = PostFix[i]; //operation character

			output[CurrentInstructionPointer] = "20" + FirstAdr; //loads the first operand
			CurrentInstructionPointer++;

			//writes to the output string array depending on the operation
			if (operation == "+") {
				output[CurrentInstructionPointer] = "30" + SecondAdr; //adds the second operand
				CurrentInstructionPointer++;

			}
			else if (operation == "-") {
				output[CurrentInstructionPointer] = "31" + SecondAdr; //subtracts the second operand
				CurrentInstructionPointer++;

			}
			else if (operation == "/") {
				output[CurrentInstructionPointer] = "32" + SecondAdr; //divides the second operand
				CurrentInstructionPointer++;

			}
			else if (operation == "*") {
				output[CurrentInstructionPointer] = "33" + SecondAdr; //multiplies the second operand
				CurrentInstructionPointer++;

			}
			else if (operation == "^") {
				output[CurrentInstructionPointer] = "34" + SecondAdr; //multiplies the second operand
				CurrentInstructionPointer++;
			}

			if (i + 1 == w) { //runs if we are at the last index of the postfix array
				if (storeAdr != "NULL") { //runs if the storage address exists
					output[CurrentInstructionPointer] = "21" + storeAdr; //stores the final value into the appropriate address
				}
				else { //runs if the storage address does not exist
					output[CurrentInstructionPointer] = "21" + FirstAdr; //stores the final value into the address of the most recent first operand
					symboltable[CurrentSYMCounter][0] = variable;
					symboltable[CurrentSYMCounter][1] = "var";
					symboltable[CurrentSYMCounter][2] = FirstAdr;

					CurrentSYMCounter++;

				}
			}

			else { //runs if we are not at the last index of the postfix array
				output[CurrentInstructionPointer] = "21" + FirstAdr; //stores the intermediate value into the address of the most recent first operand
			}
			CurrentInstructionPointer++;
		}
	}
}



bool lineError(int prev, string current) { //checks for duplicate line numbers, non-increasing line numbers, and invalid line numbers (such as 4k or Q3)
	int currentLine;
	int i = 0;
	while (i < current.length()) {
		if (!isdigit(current[i])) { //if the current character in the string is not a number, then it is invalid
			return true;
		}
		else {
			i++; //evaluates the next character
		}
	}

	stringstream lNum(current); //converts the string to a number
	lNum >> currentLine;


	if (currentLine <= prev) {
		return true;
	}
	else {
		return false;
	}
}

bool isCommand(string command) { //checks if a string equals one of the allowed commands
	string commands[] = { "rem", "input", "print", "let", "if", "goto", "end" };

	int length = sizeof(commands) / sizeof(commands[0]);
	int i = 0;

	for (int i = 0; i < length; i++) {
		if (command == commands[i]) {
			return true;
		}
	}
	return false;
}

bool commandError(int currentLineIndex) { //tests for a command that does not exist
	if (!isCommand(inputFileText[currentLineIndex][1])) {
		return true;
	}
	else {
		return false;
	}
}

bool gotoError(int currentLineIndex) { //tests for a goto with invalid destinations or no destination at all
	int currentStringNum = 0;
	int tempCounter;
	string gotoLineNum;
	bool isLNum = false;
	bool hasGoto = false;
	bool isRem = false;
	while (inputFileText[currentLineIndex][currentStringNum] != "" && !isLNum) {
		if (inputFileText[currentLineIndex][currentStringNum] == "rem") {
			isRem = true;
		}
		if (inputFileText[currentLineIndex][currentStringNum] == "goto" && !isRem) {
			hasGoto = true;
			tempCounter = 0;
			gotoLineNum = inputFileText[currentLineIndex][currentStringNum + 1];
			while (inputFileText[tempCounter][0] != "" && !isLNum) { //searches for the goto line in the file making sure that the goto destination exists
				if (inputFileText[tempCounter][0] == gotoLineNum) {
					isLNum = true;
				}
				tempCounter++;
			}
		}
		currentStringNum++; //continues to search through the current line until we get to the goto or nothing
	}
	if (!hasGoto) { //if the file does not have any gotos there will be no goto errors
		isLNum = true;
	}
	return !isLNum; //isLNum being true means that the goto has a valid destination or that there are no gotos, so we want to return false signifying that there are no errors
}


bool badVar(int currentLineIndex) { //tests for invalid variable name or missing variable
	string tempVarName;
	if (inputFileText[currentLineIndex][1] == "print" || inputFileText[currentLineIndex][1] == "let" || inputFileText[currentLineIndex][1] == "input") {
		if (inputFileText[currentLineIndex][1] == "let" && (inputFileText[currentLineIndex][3] == "" || inputFileText[currentLineIndex][4] == "")) { //checks if there is no equal sign or there is nothing after the equal sign
			return true;
		}
		if (inputFileText[currentLineIndex][2] != "") { //runs if there is something after the command
			return isdigit(inputFileText[currentLineIndex][2][0]); //if it is a digit or first character is a digit then it is an error
		}
		else { //if there is nothing after the command that is also an error
			return true;
		}
	}
	return false; //if none of the conditions are true than there is no error
}


bool comparisonError(int currentLineIndex) { //tests to make sure that there is a valid comparison sign in the if expression
	if (inputFileText[currentLineIndex][1] == "if") {
		int i = currentLineIndex;
		int j = 0;
		while (!isComparison(inputFileText[i][j]) && inputFileText[i][j] != "") { //goes through every string after the function call "if" until it gets to the comparison or the end of the line
			j++; //increments to get the next string on the current line of the file
		}
		string comparison = inputFileText[i][j]; //this will either be the comparison sign or an empty string

		if (comparison == "") { //this means that there are no comparison signs or all of them are invalid
			return true;
		}
		else { //this means that a valid comparison was found
			return false;
		}
	}
	return false;
}


bool invalidOp(int currentLineIndex) { //tests that there is a valid number of operators in the if and let expressions (x > y y   vs.   x > y + y)
	int numOps = 0;
	int numRest = 0;
	int j = 2;
	if (inputFileText[currentLineIndex][1] == "if") {
		while (!isComparison(inputFileText[currentLineIndex][j]) && inputFileText[currentLineIndex][j] != "goto") { //checks everything before the comparison
			if (isOperator(inputFileText[currentLineIndex][j])) {
				numOps++;
			}
			else {
				numRest++;
			}
			j++;
		}
		if (numRest - numOps != 1) { //there should be one more variable or literal than operators. If there isnt than it is an error
			return true;
		}
		else { //runs if there is one more variable or literal than operators
			numOps = 0;
			numRest = 0;
			j++; //makes it so that now we are checking after the comparison sign
			while (inputFileText[currentLineIndex][j] != "goto" && inputFileText[currentLineIndex][j] != "") {
				if (isOperator(inputFileText[currentLineIndex][j]) && inputFileText[currentLineIndex][j] != ")") {
					numOps++;
				}
				else {
					numRest++;
				}
				j++;
			}

		}
		if (numRest - numOps != 1) { //there should be one more variable or literal than operators. If there isnt than it is an error
			return true;
		}
		return false; //runs if there is one more variable or literal than operators
	}

	if (inputFileText[currentLineIndex][1] == "let") {
		j = 4; //we start evaluating after the equal sign
		while (inputFileText[currentLineIndex][j] != "") { //reads up until the end of the line
			if (isOperator(inputFileText[currentLineIndex][j]) && inputFileText[currentLineIndex][j] != ")") {
				numOps++;
			}
			else {
				numRest++;
			}
			j++;
		}
		if (numRest - numOps != 1) {
			return true;
		}
		else {
			return false;
		}
	}
}

bool isDeclared(string var, string vars[]) { //tests if a string (var) is stored in the array vars
	int i = 0;
	while (vars[i] != "") {
		if (var == vars[i]) {
			return true;
		}
		i++;
	}
	return false;
}


bool undeclaredVar(int currentLineIndex) { //tests for variables being used without having been declared
	if (inputFileText[currentLineIndex][1] == "input") {
		initVars[initVarIndex] = inputFileText[currentLineIndex][2]; //store the variable after input into a global array of initialized variables
		initVarIndex++;
	}

	if (inputFileText[currentLineIndex][1] == "print") {
		try {
			int digits = 0;
			int var = stoi(inputFileText[currentLineIndex][2]); //will move into the catch block if this conversion fails. This means there is a non-numeric character. Note that this will not throw an error if it is a number followed by a char (ex: 2k will return 2)
			while (var / 10 != 0) { //counts the number of digits in the integer
				var = var / 10;
				digits++;
			}
			digits++;
			if (digits == inputFileText[currentLineIndex][2].length()) { //if number of digits after conversion = original length of string
				return false; //normal digit
			}
			else { //the string must be a number followed by a character
				return true; //bad variable name
			}
		}
		catch (invalid_argument& e) {
			if (!isDeclared(inputFileText[currentLineIndex][2], initVars)) {
				return true;
			}
		}
	}

	//modeled similarly to previous section of code for the print command
	if (inputFileText[currentLineIndex][1] == "if") {
		int j = 2;
		while (inputFileText[currentLineIndex][j] != "goto" && inputFileText[currentLineIndex][j] != "") {
			if (isComparison(inputFileText[currentLineIndex][j])) {
				j++;
			}
			while (isOperator(inputFileText[currentLineIndex][j])) {
				j++;
			}
			if (inputFileText[currentLineIndex][j] == "") {
				break;
			}
			try {
				int digits = 0;
				int var = stoi(inputFileText[currentLineIndex][j]);
				while (var / 10 != 0) {
					var = var / 10;
					digits++;
				}
				digits++;
				if (digits == inputFileText[currentLineIndex][j].length()) {
					return false; //normal digit
				}
				else {
					return true; //bad variable name
				}
			}
			catch (invalid_argument& e) {
				if (!isDeclared(inputFileText[currentLineIndex][j], initVars)) {
					return true;
				}
			}
			j++;
		}
	}

	//modeled similarly to previous sections of code for the print and if commands
	if (inputFileText[currentLineIndex][1] == "let") {
		if (inputFileText[currentLineIndex][2] == "=") {
			return true;
		}
		int j = 4;
		while (inputFileText[currentLineIndex][j] != "") {
			while (isOperator(inputFileText[currentLineIndex][j])) {
				j++;
			}
			if (inputFileText[currentLineIndex][j] == "") {
				break;
			}
			try {
				int digits = 0;
				int var = stoi(inputFileText[currentLineIndex][j]);
				while (var / 10 != 0) {
					var = var / 10;
					digits++;
				}
				digits++;

				if (digits == inputFileText[currentLineIndex][j].length()) {
					//do nothing because its a normal digit
				}
				else {
					return true; //bad variable name
				}
			}
			catch (invalid_argument& e) {
				if (!isDeclared(inputFileText[currentLineIndex][j], initVars)) { //check if its declared for variables on the right side of the equal
					cout << inputFileText[currentLineIndex][j] << endl;
					return true;
				}
			}
			j++;
		}
		initVars[initVarIndex] = inputFileText[currentLineIndex][2]; //store the variable after let into a global array of initialized variables
		initVarIndex++;
	}
	return false;
}







int main() {

	string filename;
	string filepath;
	cout << "Enter the name of the file to be processed ";
	cin >> filename;
	cout << "Enter the storage path of the outfile ";
	cin.get(); //waits for input from user
	getline(cin, filepath); //gets the filepath from the user.
	cout << filepath << endl;

	string line;
	ifstream myfile(filename); //sets up the file to be read

	for (int i = 0; i < sizeof(inputFileText) / sizeof(inputFileText[0]); i++) { //iterates through the lines of the file
		string tempstr; //initializes a temporary line storage
		if (myfile.is_open()) {
			getline(myfile, tempstr); //stores the current line of the file into the temporary string
		}
		istringstream ss(tempstr); //converts the string into a string stream
		for (int j = 0; j < sizeof(inputFileText[0]) / sizeof(inputFileText[0][0]); j++) { //iterates through the whole row
			if (inputFileText[i][1] == "rem") { //If the operation is remarks then it stores the rest of the line into a single string rather than spliting it up
				string restOfString = "";
				while (ss) {
					string word; //initializes a string to temporarily store each element from the sstream
					ss >> word; //stores the next element of the sstream into the string word
					restOfString += word; //concatenates each element in the sstream after the remarks function to make it one string 
					restOfString += " "; //Adds a space between each word/element within the remarks
				}
				inputFileText[i][j] = restOfString; //stores the remark string into the inputFile 2d Array
			}
			else {
				string word; //initializes a string to temporarily store each element from the sstream
				ss >> word; //stores the next element of the sstream into the string word
				inputFileText[i][j] = word; //stores each individual element into the inputFile 2d Array
			}
		}
	}
	myfile.close();
	ofstream outfile(filepath); //sets up the file that we will write to



	int i = 0;
	int prevLineNum = -1;
	string currentLineNum;
	bool ErrorFound = false;
	string vars[100];
	string initVars[100];
	bool hasEnd = false;

	while (inputFileText[i][0] != "") {
		currentLineNum = inputFileText[i][0];

		if (lineError(prevLineNum, currentLineNum)) {
			cout << "Invalid/Duplicate Line Num: " << currentLineNum << endl;
			ErrorFound = true;
		}

		else if (commandError(i)) {
			cout << "Invalid Function at line " << currentLineNum << endl;
			ErrorFound = true;
		}

		else if (gotoError(i)) {
			cout << "Invalid Goto Destination at line " << currentLineNum << endl;
			ErrorFound = true;
		}


		else if (badVar(i)) {
			cout << "Invalid/missing variable name at line " << currentLineNum << endl;
			ErrorFound = true;
		}

		else if (comparisonError(i)) {
			cout << "Incorrect or lack of comparison at line " << currentLineNum << endl;
			ErrorFound = true;
		}

		else if (invalidOp(i)) {
			cout << "Invalid/missing operator at line " << currentLineNum << endl;
			ErrorFound = true;
		}

		else if (undeclaredVar(i)) {
			cout << "Undeclared variable at line " << currentLineNum << endl;
			ErrorFound = true;
		}
		if (inputFileText[i][1] == "end") {
			hasEnd = true;
		}

		prevLineNum = stoi(currentLineNum);
		i++;
	}
	if (!hasEnd) {
		cout << "The Program has no End" << endl;
		ErrorFound = true;
	}

	i = 0;
	while (inputFileText[i][0] != "" && !ErrorFound) { //runs until we get to a blank line
		string operation = inputFileText[i][1]; //the operation is the first word after the line number

		if (operation == "rem") {
			//stores line number into symbol table
			symboltable[CurrentSYMCounter][0] = inputFileText[i][0];
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentInstructionPointer);

			CurrentSYMCounter++;
		}


		else if (operation == "input") {
			//stores line number into symbol table
			symboltable[CurrentSYMCounter][0] = inputFileText[i][0];
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentInstructionPointer);

			//stores variable into symbol table
			symboltable[CurrentSYMCounter + 1][0] = inputFileText[i][2];
			symboltable[CurrentSYMCounter + 1][1] = "V";
			symboltable[CurrentSYMCounter + 1][2] = to_string(CurrentDataCounter);

			output[CurrentInstructionPointer] = "10" + to_string(CurrentDataCounter); //writes the input command into the output string array

			CurrentDataCounter--;
			CurrentSYMCounter += 2;
			CurrentInstructionPointer++;
		}


		else if (operation == "print") {
			//stores line number into symbol table
			symboltable[CurrentSYMCounter][0] = inputFileText[i][0];
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentInstructionPointer);

			CurrentSYMCounter++;

			//finds the address of the variable name in the symbol table
			string Adr = getAdr(inputFileText[i][2], symboltable, CurrentSYMCounter);
			output[CurrentInstructionPointer] = "11" + Adr; //writes the write command to the output array
			CurrentInstructionPointer++;
		}


		else if (operation == "let") {

			//stores line number into symbol table
			symboltable[CurrentSYMCounter][0] = inputFileText[i][0];
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentInstructionPointer);

			string variable = inputFileText[i][2];
			string varAdr = getAdr(variable, symboltable, CurrentSYMCounter);
			string expression[100];

			CurrentSYMCounter++;

			int j = 4;
			int a = 0;
			//put everything after the equals sign into an expression array
			while (inputFileText[i][j] != "") {
				expression[a] = inputFileText[i][j];
				j++;
				a++;
			}

			calculate(expression, a, varAdr, variable); //writes neccessary SML output based on the infix expression and stores the final value into memory
		}



		else if (operation == "if") {

			string exp1[100];
			string exp2[100];
			int j = 2;
			int a = 0; //declares the counter for the first expression size

			while (!isComparison(inputFileText[i][j]) && inputFileText[i][j] != "") { //goes through every string after the function call "if" until it gets to the comparison
				exp1[a] = inputFileText[i][j];//stores the string into the expression array
				j++;//increments to get the next string on the current line of the file
				a++;//counter for the size of the first expression
			}


			string comparison = inputFileText[i][j];
			j++;


			int b = 0; //declares the counter for the second expression size

			while (inputFileText[i][j] != "goto") { //goes through every string after the if comparison until it reaches the goto function
				exp2[b] = inputFileText[i][j]; //stores the string into the expression array
				j++; //increments to get the next string on the current line of the file
				b++; //counter for the size of the second expression
			}
			j++;//gets the next string from the input file
			string lineNum = inputFileText[i][j]; //gets the line number that the goto portion of the if goto function is calling

			calculate(exp1, a, "NULL", "exp" + to_string(expNum)); //calculates value of the expression and stores it into memory
			expNum++; //increments the expression number for identification of the expressiong
			calculate(exp2, b, "NULL", "exp" + to_string(expNum));//calculates value of the expression and stores it into memory
			expNum++; //increments the expression number for identification of the expressiong

			symboltable[CurrentSYMCounter][0] = inputFileText[i][0];//line number of the if goto function from the txt file
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentInstructionPointer);  //index of the if goto function in the outfile

			string sign = comparison; //comparison for the if function
			string leftExpr = "exp" + to_string(expNum - 2); //left expresion variable name
			string rightExpr = "exp" + to_string(expNum - 1); //right expresion variable name

			string leftAdr = getAdr(leftExpr, symboltable, CurrentSYMCounter); //gets the address of the left expression for the comparison
			string rightAdr = getAdr(rightExpr, symboltable, CurrentSYMCounter);//gets the address of the right expression for the comparison


			if (sign == ">") {
				output[CurrentInstructionPointer] = "20" + rightAdr;//adds load function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "31" + leftAdr;//adds subtraction function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "41"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile
			}

			else if (sign == "<") {
				output[CurrentInstructionPointer] = "20" + leftAdr;//adds load function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "31" + rightAdr;//adds subtraction function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "41"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);
				CurrentInstructionPointer++;//increments for next line in the outfile
			}

			else if (sign == ">=") {
				output[CurrentInstructionPointer] = "20" + rightAdr;//adds load function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "31" + leftAdr;//adds subtraction function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "41"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "42"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile
			}

			else if (sign == "<=") {
				output[CurrentInstructionPointer] = "20" + leftAdr;//adds load function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "31" + rightAdr;//adds subtraction function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "41"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "42"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile
			}

			else if (sign == "==") {
				output[CurrentInstructionPointer] = "20" + leftAdr;//adds load function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "31" + rightAdr;//adds subtraction function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile
				output[CurrentInstructionPointer] = "42"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile
			}

			else if (sign == "!=") {
				output[CurrentInstructionPointer] = "20" + leftAdr;//adds load function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile

				output[CurrentInstructionPointer] = "31" + rightAdr;//adds subtraction function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile

				output[CurrentInstructionPointer] = "41"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile

				output[CurrentInstructionPointer] = "20" + rightAdr; //adds load function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile

				output[CurrentInstructionPointer] = "31" + leftAdr;//adds subtraction function to outfile array
				CurrentInstructionPointer++;//increments for next line in the outfile

				output[CurrentInstructionPointer] = "41"; //placeholder in the outfile array with specific goto function
				parallel[CurrentInstructionPointer] = stoi(lineNum);//stores the line number into a parallel array in case the goto calls a line that hasn't been read yet
				CurrentInstructionPointer++;//increments for next line in the outfile
			}
			CurrentSYMCounter++;//increments for next item in symbol table
		}

		else if (operation == "goto") {

			parallel[CurrentInstructionPointer] = stoi(inputFileText[i][2]); //stores the line number into a parallel array in case the goto calls a line that hasn't been read yet

			symboltable[CurrentSYMCounter][0] = inputFileText[i][0]; //line number of the goto function from the txt file
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentInstructionPointer); //index of the goto function in the outfile

			output[CurrentInstructionPointer] = "40"; //placeholder in the outfile array with specific goto function

			CurrentSYMCounter++; //increments for next item in symbol table
			CurrentInstructionPointer++;//increments for next line in the outfile
		}



		else if (operation == "end") {
			symboltable[CurrentSYMCounter][0] = inputFileText[i][0]; //line number of the end function from the txt file
			symboltable[CurrentSYMCounter][1] = "L";
			symboltable[CurrentSYMCounter][2] = to_string(CurrentInstructionPointer); //index of the end function in the outfile

			output[CurrentInstructionPointer] = "4300"; //End (halt) function

			CurrentSYMCounter++; //increments for next item in symbol table
			CurrentInstructionPointer++; //increments for next line in the outfile
		}

		i++;  //checks the next line
	}



	for (int i = 0; i < CurrentInstructionPointer; i++) {
		string Adr;
		if (parallel[i] != 0) {//loops through the parallel array to get the address for the goto statements and add them to the outfile for the assembler
			Adr = getLAdr(to_string(parallel[i]), symboltable, CurrentSYMCounter);//gets the address for the goto function
			if (stoi(Adr) % 10 == stoi(Adr)) { //tests to see if the address is one digit or not
				output[i] = output[i] + "0" + Adr; //placed at the address of the goto command
			}
			else {
				output[i] = output[i] + Adr; //placed at the address of the goto command
			}
		}
	}

	//prints the contents of the symbol table
	for (int i = 0; i < CurrentSYMCounter; i++) { //loops through the array storing the symbol table
		cout << symboltable[i][0] << " " << symboltable[i][1] << " " << symboltable[i][2] << endl; //displays each item in the symbol table
	}
	cout << endl;



	//writes to an output file and prints its contents
	for (int i = 0; i < CurrentInstructionPointer; i++) { //loops through every item in the output array
		outfile << output[i] << endl; //writes to outfile
		cout << output[i] << endl; //prints what is written to the outfile for preview
	}

	cout << endl;

	Assembler callAssembler;
	callAssembler.RunAssembler();
}
