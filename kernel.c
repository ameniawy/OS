void printString(char*);
void readString(char*);
void readSector(char*, int);
void writeSector(char*, int);
void readFile(char*, char*);
int DIV(int, int);
int MOD(int, int);
void handleInterrupt21(int, int, int, int);
void executeProgram(char*, int);
void terminateProgram();
void deleteFile(char*);
void writeFile(char* , char* , int);


int main() {
/*	int i=0;
	char buffer1[13312];
	char buffer2[13312];
	buffer2[0]='h'; buffer2[1]='e'; buffer2[2]='l'; buffer2[3]='l';
	buffer2[4]='o';
	for(i=5; i<13312; i++) buffer2[i]=0x00;
	makeInterrupt21();
	interrupt(0x21,8, "testW\0", buffer2, 1); //write file testW
	interrupt(0x21,3, "testW\0", buffer1, 0); //read file testW
	interrupt(0x21,0, buffer1, 0, 0); // print out contents of testW*/

/*	char buffer[13312];
	makeInterrupt21();
	interrupt(0x21, 7, "messag\0", 0, 0); //delete messag
	interrupt(0x21, 3, "messag\0", buffer, 0); // try to read messag
	interrupt(0x21, 0, buffer, 0, 0); //print out the contents of buffer*/

	makeInterrupt21();
	interrupt(0x21,4,"shell\0", 0x2000,0);

	while(1);

}

void printString(char* line) {
	int i = 0;
	char c = line[i];
	while (c != '\0')
	{
	    interrupt(0x10, 0xE*256+c, 0, 0, 0);
	    i++;
	    c = line[i];
	}
}


void readString(char* line)
{	
	int counter = 0;
	char x = 0x0;

	while(x != 0xd){
		x = interrupt(0x16, 0, 0, 0, 0);
		if(x == 0x8){
			if(counter != 0){
				interrupt(0x10, 0xE*256+x, 0, 0, 0);
				interrupt(0x10, 0xE*256+0x20, 0, 0, 0);
				interrupt(0x10, 0xE*256+x, 0, 0, 0);
				--counter;
			}
		}
		else if(x != 0xd) {
			line[counter] = x;
			interrupt(0x10, 0xE*256+x, 0, 0, 0); //print char
			counter++;
		}
		
	}

	if(counter != 0) {
		line[counter] = 0xa;
		counter++;
		line[counter] = '\0';
	} else {
		line[counter] = '\0';
	}
	interrupt(0x10, 0xE*256+0xa, 0, 0, 0);
	interrupt(0x10, 0xE*256+0x0, 0, 0, 0);
	interrupt(0x10, 0xE*256+0xd, 0, 0, 0);


}


void readSector(char* buffer, int sector) {
	int AX, BX, CX, DX;
	int rel_sec = MOD(sector, 18) + 1;
	int head = MOD(DIV(sector, 18), 2);
	int track = DIV(sector, 36);
	AX = 2*256+1;
	BX = buffer;
	CX = track*256+rel_sec;
	DX = head*256+0;
	interrupt(0x13, AX, BX, CX, DX);
}


void writeSector(char* buffer, int sector) {
	int AX, BX, CX, DX;
	int rel_sec = MOD(sector, 18) + 1;
	int head = MOD(DIV(sector, 18), 2);
	int track = DIV(sector, 36);
	AX = 3*256+1;
	BX = buffer;
	CX = track*256+rel_sec;
	DX = head*256+0;
	interrupt(0x13, AX, BX, CX, DX);
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


void readFile(char* file_name, char* out){
	char directory[512];
	char line1[32];
	int found = 0; //flag always 0 until eneterd file_name matches file name in entry
	int i = 0 ; //Counter for looping over the whole sector
	int j = 0 ; //Counter for looping over each 32 byte line
	int c = 0 ; //Counter for looping over the first 6 chars in an entry (file name)
	int a = 6;
	int counter = 0;
	readSector(directory,2);
	//interrupt(0x21, 2, directory, 2,0);
	while ((i < 512) && (found == 0)){
		j = 0;
		//Read line for each entry
		while(j < 32){
			line1[j] = directory[i+j];
			j = j + 1;
		}

		// Check if file match entered file name ,if yes exit the loop
		c = 0;
		while((line1[c] == file_name[c]) && (c < 6)){
			c++;
		}
		if(c == 6){
			found = 1;
		}
		i = i + 32;

		if(found==1){
			while(a<32){
				// Add every sector in a space of 512 bytes in the variable out which is 13312 bytes
				if((line1[a]>0)&&(line1[a]!='\n')){
					readSector(out + counter,line1[a]); // passing a pointer to the location we want to start adding that sector
					//interrupt(0x21, 2, out + counter, line1[a],0);
					counter += 512;

				}
				a++;
			}
			break;

		}
	}


}


void deleteFile(char* file_name) {
	char directory[512];
	char map[512];
	char line1[32];
	int found = 0; //flag always 0 until eneterd file_name matches file name in entry
	int i = 0 ; //Counter for looping over the whole sector
	int j = 0 ; //Counter for looping over each 32 byte line
	int c = 0 ; //Counter for looping over the first 6 chars in an entry (file name)
	int a = 6;
	int counter = 0;
	
	interrupt(0x21, 2, directory, 2,0);
	interrupt(0x21, 2, map, 1,0);
	//readSector(directory, 2);
	//readSector(map, 1);
	while ((i < 512) && (found == 0)){
		j = 0;
		//Read line for each entry
		while(j < 32){
			line1[j] = directory[i+j];
			j = j + 1;
		}

		// Check if file match entered file name, if yes exit the loop
		c = 0;
		while((line1[c] == file_name[c]) && (c < 6)){
			c++;
		}
		if(c == 6){
			found = 1;
		}
		a = 6; // skip the first 6 characters

			printString("peace\0");
		if(found==1){
			// i represents the index of the first char in the entry
			// a is the index of the n th char in the entry

			directory[i] = 0x00; // first char in found word overwritten by 0x00;

			while(a<32) {
				if((line1[a] > 0)&&(line1[a] != '\n')) {
					directory[i + a] = 0x00; // overwrite sector number in directory
					map[line1[a]] = 0x08; // overwrite occupied sector mark in map
				}
				a++;
			}
		}

		i = i + 32; // skip to next entry
	}
	//writeSector(directory, 2);
	//writeSector(map, 1);
	interrupt(0x21, 6, directory, 2,0); // write new directory in sector 2
	interrupt(0x21, 6, map, 1,0); // write new map in sector 1
}


void writeFile(char* file_name, char* buffer, int secNum) {
	char directory[512];
	char map[512];
	int firstChar = 0;
	int j = 0;
	int k;
	int mapIndex;
	int directoryPointer;
	int filledSectors = 0;
	int bufferIndex = 0;

	makeInterrupt21();
	interrupt(0x21,2,map,1,0);
	interrupt(0x21,2,directory,2,0);

	// finding first char of empty directory entry
	for(firstChar = 0; firstChar < 512; firstChar += 32) {
		if(directory[firstChar] == 0x00)
			break;
	}

	// fill name with 0x00 as default
	for(j = 0; j < 6; j++) {
		directory[firstChar + j] = 0x00;
	}	

	// add file name in directory
	for(j = 0; j < 6; j++) {
		if(file_name[j] != '\0') {
			directory[firstChar + j] = file_name[j];
		}
	}

	directoryPointer = 6;
	// add sector numbers

	for(mapIndex = 3; mapIndex < 512 && filledSectors < secNum; mapIndex++) {
		if(map[mapIndex] == 0x00) {
			map[mapIndex] = 0xFF;
			directory[firstChar + directoryPointer] = mapIndex; // add the sector number to the directory;
			directoryPointer++;
			filledSectors++;

			// write 512 of buffer into the empty sector
			interrupt(0x21,6,buffer,mapIndex,0);

			buffer += 512;

		}

	}

	// fill the remaining empty spaces in the directory to 0x00
	for(k = directoryPointer; k < 32; k++) {
		directory[k] = 0x00;
	}


	if(filledSectors == secNum) {
		interrupt(0x21,6,map,1,0);
		interrupt(0x21,6,directory,2,0);
	} else {
		printString("NO SPACE FOUND \0");
	}

}


void executeProgram(char* file_name, int segment) {
	char out[13312];
	int i = 0;
	readFile(file_name, out);


	for(i = 0; i < 13312; i++){
		putInMemory(segment, i, out[i]);
	}

	launchProgram(segment);

}

void terminateProgram(){
	char word[6];
	word[0] = 's';
	word[1] = 'h';
	word[2] = 'e';
	word[3] = 'l';
	word[4] = 'l';
	word[5] = '\0';

	interrupt(0x21,4,word, 0x2000,0);

}


void handleInterrupt21(int ax, int bx, int cx, int dx) {
	if(ax == 0) {
		printString(bx);
	}
	else if(ax == 1) {
		readString(bx);
	}
	else if(ax == 2) {
		readSector(bx, cx);
	}
	else if(ax == 3) {
		readFile(bx, cx);
	}
	else if(ax == 4) {
		executeProgram(bx, cx);
	}
	else if(ax == 5) {
		terminateProgram();
	}
	else if(ax == 6) {
		writeSector(bx, cx);
	}
	else if(ax == 7) {
		deleteFile(bx);
	}
	else if(ax == 8) {
		writeFile(bx, cx, dx);
	}
	else {
		printString("\nERROR\n\0");
	}
}
