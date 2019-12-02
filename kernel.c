/* kernel.c -program to serve as OS kernel for comp350
 *-----------------------------------------------------
 *Authors: Kyle Snow, Tom Foley, Shane Driskell
 *
 *last modified: 10/26/19 @ 11:32
 *
 */

void printChar(char c);
void printString(char* chars);
char* readString(char* line);
void readSector(char* buffer, int sector);
void handleInterrupt21(int ax, int bx, int cx, int dx);
void readFile(char* buffer, char* message, int &sectorRead);
int strCompare(char* str1, char* str2, int offset);
void executeProgram(char* name);
void terminate();
void writeSector(char* buffer, int sector);
void deleteFile(char* filename);
void writeFile(char* buffer, char* filename, int numberOfSectors);



int main(){
		
	makeInterrupt21();
	//interrupt(0x21,8,"this is a test message","testmg",3);
	interrupt(0x21, 4, "shell", 0, 0);
	while(1);/*boucle infini*/

}

void writeFile(char* buffer, char* filename, int numberOfSectors){

	char map[512];
	char dir[512];
	char bufferSegment[512];
	int fileEntry;
	int i; //positional item for dir 
	int j; //counter up to numberOfSectors
	int k; //iterator of map
	int l; //filler of buffer segment & 
	int remainingBytes;


	//load map and directory into arrays
	interrupt(0x21,2,map,1,0);
	interrupt(0x21,2,dir,2,0);
	//find an open directory entry
	for(fileEntry=0;fileEntry<512;fileEntry+=32){ //iterate over directory in 32 byte increments
		if(dir[fileEntry]==0x0){  //if entry is empty
			//copy filename to first 6 of entry
			for(i=0;i<6;i++){dir[fileEntry+i]=filename[i];} //--FIX to allow for filename < 6
			break;  //leave loop.
		}else{continue;}//continue looping until an open entry is found	
	}
	//if fileEntry==512, unable to find empty slot. return.
	if(fileEntry==512){return;}

	//set i to 6 --> 
	i=6;	


	//for each sector to write:
	for(j=0;j<numberOfSectors;j++){

		//fill buffer segment that corresponds to current sector
		for(l=0;l<512;l++){
			bufferSegment[l] = buffer[(512*j)+l];
		}
				

		//find free sector in the map
		for(k=4;k<512;k++){
			
			if(map[k]==0x0){
				//set found sector to 0xFF
				map[k] = 0xFF;	
				//add that sector to the directory entry
				dir[fileEntry+i] = k;
			
				//write 512 byte block from buffer to that sector
				interrupt(0x21,6,bufferSegment,k,0);
				//fill remaining bytes in entry with 0x0's
				l=i; //i at this point == the current unfilled dir byte; l is free to use.
				remainingBytes = 32 - l;
				for(l;l<remainingBytes;l++){
					dir[fileEntry+l] = 0x0;
				}
			
				//write the Map and Directory sectors back to the disk 
				interrupt(0x21,6,map,1,0);
				interrupt(0x21,6,dir,2,0);//---------
			}else{continue;}
		}

		i++; //increment i for next assignment	

	}
	

}


void deleteFile(char* filename){

	char map[512];
	char dir[512];	
	char entryName[7];
	int fileEntry;
	int i;
	int tbd; //sector to be deleted

	//load the directory(sec2) and map(sec1) into corresponding arrays
	interrupt(0x21,2,map,1,0);
	interrupt(0x21,2,dir,2,0);

	//search directory for filename
	for(fileEntry=0;fileEntry<512;fileEntry+=32){ 
		//copy filename into array named entryName
		for(i=0;i<6;i++){
			entryName[i] = dir[fileEntry+i];
		}
		entryName[6]=0x0;

		// string compare  fileName vs entryName
		if(strCompare(filename, entryName,0) == 0){
			//if strings do NOT match
			continue;
		}else{
		//if strings match 
			//set first fileEntry to NULL
			dir[fileEntry]='\0';
			//step through listed sectors and delete the corresponding entry in map
			for(i=fileEntry+6;i<fileEntry+32;i++){
				tbd = dir[i];
				map[tbd]=0x0;
			}
			
			//write modified sectors back
			//bx= buffer to write, cx=sector to write to
			interrupt(0x21,6,map,1,0);
			interrupt(0x21,6,dir,2,0);


		}
	}


}


void terminate(){

	char shellname[6];

	shellname[0]='s'; shellname[1]='h';
	shellname[2]='e'; shellname[3]='l';
	shellname[4]='l'; shellname[5]='\0';


	executeProgram(shellname);
	//interrupt(0x21,4,shellname,0,0);
}


//takes name of file to execute and loads into memory
void executeProgram(char* name){

	char buffer[13312];
	int sectorsRead;
	int i;
	int offset = 0x0;
	
	//makeInterrupt21();
	interrupt(0x21,3,name,buffer, &sectorsRead);
	if(sectorsRead>0){
		for(i=0;i<13312;i++){
			
			//putInMemory(segment,address,char)
			putInMemory(0x2000, offset, buffer[i]);
			
			offset++;
		}
		launchProgram(0x2000);

	}else{
		interrupt(0x21,0,"program not found!\r\n",0,0);
	}


}

//takes buffer to write to,string file name, and address of sectors read
void readFile(char* buffer, char* filename, int* sectorsRead){

	char dir[512];
	int fileEntry;
	int i;
	char entryName[6];
	*sectorsRead = 0;
	//read sector 2 into a buffer	
	readSector(dir,2); 
	//iterate over every file entry in sector
	for(fileEntry=0;fileEntry<512;fileEntry+=32){ 
		//copy filename into array named entryName
		for(i=0;i<6;i++){
			entryName[i] = dir[fileEntry+i];
		}

		// string compare  fileName vs entryName
		if(strCompare(filename, entryName,0) == 0){
			*sectorsRead = 0;
			//printString("sectors Read set to 0\r\n");
			continue;
		}
		if(strCompare(filename, entryName, 0)==1){
			//load the file sector by sector into the buffer
			for(i=fileEntry+6;i<fileEntry+32;i++){
				readSector(buffer,dir[i]);
				buffer+=512;
				*sectorsRead = *sectorsRead+1;		
			}
			return;	
		}
	
	}
	return;
	

}
	


//compare string. takes: string1, string2, offset value; returns 0=F,1=T
int strCompare(char* str1, char* str2, int offset){  
	int i;
	for(i=0;i<6;i++){
		
		/*
		printChar(str1[i]);
		printChar(str2[i+offset]);
		printChar('\r');
		printChar('\n');
		*/

		if(str1[i]==0x0 && str2[i+offset]==0x0){break;}
		if(str1[i]=='\r' && str2[i+offset]==0x0){break;}
		if(str1[i] != str2[i+offset]){return 0;}
	}
	return 1;
}





void printChar(char c){
	interrupt(0x10,0xe*256+c,0,0,0);
}

void printString(char* chars){  

	while(*chars != 0x0){ //while element is NOT NULL, iterate.
		interrupt(0x10,0xe*256+*chars,0,0,0); //interrupt to print char
		chars++;
	}
 
}

char* readString(char* line){
	char new; //define a new character container
	do{

		new = interrupt(0x16,0,0,0,0); // interrupt to get character
		
		if(new == 0x08 && *line !=  0){//---------------------
			interrupt(0x10,0xe*256+0x08,0,0,0);//print bkspc
			line--; //step back in array
			interrupt(0x10,0xe*256+' ',0,0,0);//print space over bkspc 
			interrupt(0x10,0xe*256+0x08,0,0,0);//second backspace??
				
		}
		else{
		*line = new;
		interrupt(0x10,0xe*256+*line,0,0,0);
		line++;
		}

	}while(new != 0xd);


	*line = 0xa; //line return
	line++;
	*line = 0xd;
	line++;
	*line = 0x0; //append NULL
	interrupt(0x10,0xe*256+0xa,0,0,0);
	interrupt(0x10,0xe*256+0xd,0,0,0);

	return line;
}

// takes two parameters: predefined character array of >= 512 bytes to store result, and sector number to read	
void readSector(char* buffer, int sector){
	int AH = 2; //read
	int AL = 1; //number of sectors to read
	//pass buffer to BX
	int CH = 0;//trackNum
	int CL = sector + 1; //relative Sector
	int DH = 0; //head number
	int DL = 0x80; //device number

	interrupt(0x13,AH*256+AL,buffer,CH*256+CL,DH*256+DL);

}

// takes two parameters: predefined character array of >= 512 bytes to write, and sector number to write	to
void writeSector(char* buffer, int sector){
	int AH = 3; //read
	int AL = 1; //number of sectors to read
	//pass buffer to BX
	int CH = 0;//trackNum
	int CL = sector + 1; //relative Sector
	int DH = 0; //head number
	int DL = 0x80; //device number

	interrupt(0x13,AH*256+AL,buffer,CH*256+CL,DH*256+DL);

}


void handleInterrupt21(int ax, int bx, int cx, int dx){
	if(ax == 0){ //print string
		printString(bx);
	}else if(ax == 1){ //read string
		readString(bx);
	}else if(ax == 2){ //read sector
		readSector(bx,cx);
	}else if(ax==3){
		//filename,buffer,sectors as opposed to buffer, filename,sectors (as written)
		readFile(cx,bx,dx);
	}else if(ax==4){
		executeProgram(bx);	
	}else if(ax==5){
		terminate();
	}else if(ax==6){
		writeSector(bx,cx);
	}else if(ax==7){
		deleteFile(bx);
	}else if(ax==8){
		writeFile(bx,cx,dx);
	}else{printString("ERROR");}
}






