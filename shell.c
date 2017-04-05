void printString(char*);
void handleInput(char*);
int compareString(char*, char*);
void getFileName(char*);
void viewFile(char*);
void execute(char*);
void dir();
void create(char*);


int main(){
	char line[32];

	while(1){
		interrupt(0x21, 0, "SHELL>\0", 0, 0);
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
	else if(compareString(command, "dir\0") == 1) {
		dir();
	}
	else if(compareString(command, "create\0") == 1) {
		create(command);
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


// dir command
void dir() {
	int i = 0;
	int k;
	int numberOfSectors = 0;
	int size;
	char directory[512];
	char fileName[6];


	interrupt(0x21,2,directory,2,0);

	for(i = 0; i < 512; i+=32) {
		// skip if entry starts with 0x00(deleted)
		if(directory[i] == 0x00)
			continue;

		// get fileName
		for(k = 0; k < 6; k++)
			fileName[k] = directory[k + i];

		numberOfSectors = 0;

		// get number of sectors used
		for(k = 6; k < 32; k++) {
			if(directory[k + i] != 0x00)
				numberOfSectors++;
		}

		// each sector is 512 byte
		size = numberOfSectors * 512;

		// print 1 result
		interrupt(0x21, 0, fileName, 0, 0);
		interrupt(0x21, 0, size, 0, 0);

	}
}


// create command
void create(char* command) {
	char fileName[6];
	char line[32];
	char file[13312];
	char fileIndex = 0;
	int sectors = 0;
	int i;
	getFileName(command, fileName);

	while(1) {
		line[0] = '\0';
		interrupt(0x21, 0, ">\0", 0, 0);

		// read line from user
		interrupt(0x21, 1, line, 0, 0);

		// until user enters an empty line
		if(line[0] == '\0')
			break;

		for(i = 0; i < 32; i++) {
			if(line[i] == 0x00)
				break;

			file[fileIndex] = line[i];
			fileIndex++;
		}
	}

	// write file
	interrupt(0x21, 8, fileName, file, 1);
}