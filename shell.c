void printString(char*);
void handleInput(char*);
int compareString(char*, char*);
void getFileName(char*);
void viewFile(char*);
void execute(char*);


int main(){
	char line[32];

	while(1){
		interrupt(0x21, 0, "SHELL>", 0, 0);
		interrupt(0x21, 1, line, 0, 0);
		handleInput(line);
	}

}


// handling input commands
void handleInput(char* command) {

	if(compareString(command, "view\0") == 1) {
		viewFile(command);
	} 
	else if(compareString(command, "execute\0") == 1) {
		execute(command);
	} 
	else {
		interrupt(0x21, 0, "BAD COMMAND\n\0", 0, 0);
	}
}


// checks if x starts with y
int compareString(char* x, char* y) {
	int i = 0;
	while(y[i] != '\0') {
		if(x[i] != y[i])
			return 0;
		i++;
	}

	return 1;
}


// gets the file name from "command" and put it in "out"
void getFileName(char* command, char* out) {
	int i;
	int j = 0;
	while(command[j] != 0x20){
		j++;
	}

	j++;

	for(i = j; i < j + 6; i++){
		out[i - j] = command[i];
	}

	out[6] = "\0";

}


// handling view command
void viewFile(char* command) {
	char fileName[32];
	char fileContent[13312];

	getFileName(command, fileName);
	interrupt(0x21, 3, fileName, fileContent, 0);
	interrupt(0x21, 0, fileContent, 0, 0);

}


// execute file
void execute(char* command) {
	char fileName[32];

	getFileName(command, fileName);
	interrupt(0x21, 4, fileName, 0x2000, 0);

}