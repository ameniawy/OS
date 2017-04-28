void printString(char*);
void handleInput(char*);
int compareString(char*, char*);
void getFileName(char*, char*);
void getFileNameCopy(char*, char*);
void viewFile(char*);
void execute(char*);
void kill(char*);
void dir();
int DIV(int, int);
int MOD(int, int);
void create(char*);
void delete(char*);
int getNumberOfSectors(char*);
void copy(char*);
void printInt(int );


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
	else if(compareString(command, "kill\0") == 1) {
		kill(command);
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


int MOD(int x, int y) {
	int c = DIV(x, y);
	int i = 0;
	while(i < c) {
		x = x - y;
		i++;
	}
	return x;
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


// prints an Integer 
void printInt(int num)
{
	int i;
	int n;
	int mask;
	do{
		i = num;
		mask = 1;

		while(i > 9){
			i = DIV(i, 10);
			mask = mask * 10;
		}

		n = DIV(num, mask);
		num = MOD(num, mask);
		interrupt(0x10, 0xE*256+n+48, 0, 0, 0);

	} while(num != 0);
}


// handling view command
void viewFile(char* command) {
	char fileName[7];
	char fileContent[13312];

	getFileName(command, fileName);
	interrupt(0x21, 3, fileName, fileContent, 0);
	interrupt(0x21, 0, fileContent, 0, 0);

}


// handling kill command
void kill(char* command) {
	int processNum = command[5] -0x30;
	interrupt(0x21, 9, processNum , 0, 0);

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
		}
		fileName[7] = '\0';

		numberOfSectors = 0;

		// get number of sectors used
		for(k = 6; k < 32; k++) {
			if(directory[k + i] != 0x00)
				numberOfSectors++;
		}

		// print 1 result
		interrupt(0x21, 0, fileName, 0, 0);
		interrupt(0x10, 0xE*256+0x20,0,0,0);
		printInt(numberOfSectors);
		interrupt(0x10, 0xE*256+0x20,0,0,0);

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
	int sectors;

	getFileName(command, fileName);
	getFileNameCopy(command, fileNameCopy);

	interrupt(0x21, 3, fileName, buffer1, 0);

	sectors = getNumberOfSectors(buffer1);

	interrupt(0x21, 8, fileNameCopy, buffer1, sectors);
}


// get the number of sectors needed to save this buffer
int getNumberOfSectors(char* buffer){
	int i = 0 ;
	while(buffer[i] != '\0') {
		i++;
	}
	return DIV(i,512) + 1;
}