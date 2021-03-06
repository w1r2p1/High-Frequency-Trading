/* The project is broken up into four files:
 * main.c - Basic app logic
 * conf.c/.h - Parses .conf file for stock prices and budget info
 * stocktable.c/.h - Hashtable implementation that stores StockEntry structs, which have stock info in them (price, shares owned, etc)
 * command.c/.h - Parses and executes command strings
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "stocktable.h"
#include "conf.h"
#include "command.h"

/*
 * Will get either the input file specified or return stdin, depending on the commandline arguments.
 */
FILE* getInputFile(int argc, char* argv[]){
	FILE* inputFile = NULL;
	
	if (argc == 1) {
		inputFile = stdin;
	} else if (argc == 2){
		inputFile = fopen(argv[1], "r");

		if (inputFile == NULL){
			printf("Error: Invalid input file specificed\n");
			exit(EXIT_FAILURE);
		}
	} else {
		printf("Usage: hft [input file]\n");
		exit(EXIT_FAILURE);
	}
	return inputFile;
}

/*
 * Gets user input and prints a '>' to the screen.
 * Requires a FILE* parameter to match function pointer
 */
char* getInputFromUser(char* buf, int size, FILE* file){
	printf("> ");
	return fgets(buf, size, file);
}

/*
 * Prepares stocktable, parses prices.conf, and handles input / program logic.
 */
int main(int argc, char* argv[]){
	// Inital file IO
	FILE* inputFile = getInputFile(argc, argv);
	FILE* outputFile = fopen("output.txt", "w");
	FILE* executedFile = fopen("executed.txt", "w");

	if (outputFile == NULL || executedFile == NULL) {
		printf("Error: could not create necessary output files\n");
	}

	// Setting up hash table and parsing conf
	struct StockTable table;
	stockTableNew(&table, 10000);

	double budget = 0, threshold = 0;
	parseConf("prices.conf", &budget, &threshold, &table);	

	double originalBudget = budget;

	// Entering program loop
	char buffer[4096];
	struct Command cmd;

	char* (*getInput)(char*, int, FILE*);
	if (inputFile == stdin) {
		getInput = &getInputFromUser;
	} else {
		getInput = &fgets;
	}
	
	char* initialMessage = "";
	char* closingMessage = "";

	// if getting input from user, let me them know how to cancel input (send EOF)
	// also, when closing, we need to add an extra line so the stock table dump doesn't begin after the "> "
	if (inputFile == stdin) {
		initialMessage = "Press Ctrl + D to exit\n";
		closingMessage = "\n";
	} else {
	//	fclose(stderr); // suppress warning messages if input is not from cmdline
	}
	
	// Using puts adds a newline
	fputs(initialMessage, stdout);

	while (true) {
		char* line = getInput(buffer, 4096, inputFile);
		
		if (line == NULL) {
			break;
		}

		if (parseCommand(line, &cmd)){
			if (execCommand(&cmd, &table, &budget, threshold)){
				// Output executed command string as is
				fputs(line, executedFile);	
			}	
		}

	}

	fputs(closingMessage, stdout);
	
	stockTableDump(&table, originalBudget, budget, threshold, outputFile);
	stockTableDump(&table, originalBudget, budget, threshold, stdout);

	fclose(inputFile);
	fclose(executedFile);
	fclose(outputFile);

	return EXIT_SUCCESS;
}	
