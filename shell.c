void printString(char*);
void handleInput(char*);
int compareString(char*, char*);
void getFileName(char*, char*);
void getFileNameCopy(char*, char*);
void viewFile(char*);
void execute(char*);
void dir();
int DIV(int, int);
void create(char*);
void delete(char*);
void copy(char*);


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
	else if(compareString(command, "delete\0") == 1) {
		delete(command);
	}
	else if(compareString(command, "copy\0") == 1) {
		copy(command);
	}  
	else {
		interrupt(0x21, 0, "BAD COMMAND\n\0", 0, 0);
	}
}


int DIV(int x, int y) {
	int c = 0;
	if(x == 0) {
		return c;
	}
	while(x >= y) {
		x = x - y;
		c = c + 1;
	}
	return c;
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


// gets the second file name from "command" and put it in "out"
void getFileNameCopy(char* command, char* out) {
	int i;
	int j = 0;
	while(command[j] != 0x20){
		j++;
	}

	j++;

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
	char fileName[7];
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
	char size[3];
	char directory[512];
	char fileName[7];


	interrupt(0x21,2,directory,2,0);

	for(i = 0; i < 512; i+=32) {
		// skip if entry starts with 0x00(deleted)
		if(directory[i] == 0x00)
			continue;

		// get fileName
		for(k = 0; k < 6; k++) {
			fileName[k] = directory[k + i];
			//interrupt(0x21, 0, directory[k + i], 0, 0);
		}
		fileName[7] = '\0';

		numberOfSectors = 0;

		// get number of sectors used
		for(k = 6; k < 32; k++) {
			if(directory[k + i] != 0x00)
				numberOfSectors++;
		}

		switch(numberOfSectors){
			case 1: size[0] = '1'; size[1] = '\0'; break;
			case 2: size[0] = '2'; size[1] = '\0'; break;
			case 3: size[0] = '3'; size[1] = '\0'; break;
			case 4: size[0] = '4'; size[1] = '\0'; break;
			case 5: size[0] = '5'; size[1] = '\0'; break;
			case 6: size[0] = '6'; size[1] = '\0'; break;
			case 7: size[0] = '7'; size[1] = '\0'; break;
			case 8: size[0] = '8'; size[1] = '\0'; break;
			case 9: size[0] = '9'; size[1] = '\0'; break;
			case 10: size[0] = '1';size[1]='0'; size[2] = '\0'; break;
			case 11: size[0] = '1';size[1]='1'; size[2] = '\0'; break;
			case 12: size[0] = '1'; size[1]='2'; size[2] = '\0'; break;
			case 13: size[0] = '1'; size[1]='3'; size[2] = '\0'; break;
			case 14: size[0] = '1';size[1]='4'; size[2] = '\0';  break;
			case 15: size[0] = '1';size[1]='5'; size[2] = '\0';  break;
			default: size[0] = '0'; size[1] = '\0'; 
		}

		// print 1 result
		interrupt(0x21, 0, fileName, 0, 0);
		interrupt(0x10, 0xE*256+0x20,0,0,0);
		interrupt(0x21, 0, size, 0, 0);
		interrupt(0x10, 0xE*256+0xa,0,0,0);

	}
}


// create command
void create(char* command) {
	char fileName[7];
	char line[32];
	char file[13312];
	char fileIndex = 0;
	int sectors;
	int i;
	int number;
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

	sectors = DIV(fileIndex, 512) + 1;
	// write file
	interrupt(0x21, 8, fileName, file, sectors);
}


// delete command
void delete(char* command) {
	char fileName[7];
	getFileName(command, fileName);
	//interrupt(0x21, 0, fileName, 0, 0);
	interrupt(0x21,7,fileName,0,0);
}


// copy command
void copy(char* command) {
	char fileName[7];
	char fileNameCopy[7];
	char buffer1[13312];

	getFileName(command, fileName);
	getFileNameCopy(command, fileNameCopy);

	interrupt(0x21, 3, fileName, buffer1, 0);

	interrupt(0x21, 8, fileNameCopy, buffer1, 1);
}