void printString(char*);
void readString(char*);
void readSector(char* ,int);
void readFile(char* ,char*);
int DIV(int, int);
int MOD(int, int);
void handleInterrupt21(int, int, int, int);
void executeProgram(char*, int);
void terminateProgram();


int main() {

	makeInterrupt21();
	interrupt(0x21,4,"shell\0", 0x2000,0);

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
		else{
			line[counter] = x;
			interrupt(0x10, 0xE*256+x, 0, 0, 0); //print char
			++counter;
		}
		
	}
	line[counter] = 0xa;
	++counter;
	line[counter] = 0x0;
	interrupt(0x10, 0xE*256+0xa, 0, 0, 0);

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
	char buffer1[512];
	char line1[32];
	int found = 0; //flag always 0 until eneterd file_name matches file name in entry
	int i = 0 ; //Counter for looping over the whole sector
	int j = 0 ; //Counter for looping over each 32 byte line
	int c = 0 ; //Counter for looping over the first 6 chars in an entry (file name)
	int a = 6;
	int counter = 0;
	readSector(buffer1,2);
	while ((i < 512) && (found == 0)){
		j = 0;
		//Read line for each entry
		while(j < 32){
			line1[j] = buffer1[i+j];
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
					counter += 512;

				}
				a++;
			}
		}

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
	else{
		printString("\nERROR\n\0");
	}
}
