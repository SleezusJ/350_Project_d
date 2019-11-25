
void type(char* filename);
int strCompare(char* str1, char* str2);
void getcmdName(char* cmdbuffer, char* cmdName, int* spaceIndex);
void getfileName(char* cmdbuffer, char* filename, int spaceIndex);
void dir();

int main(){

char cmdBuffer[20];
char* cmdName;  //[4]
char filename[7];  //6 chars + 1 for NULL long
int spi; //index of SPACE in command buffer(used to differentiate between cmd & filename)
int i;


	while(1){

		//print prompt:
		syscall(0,"KSH_> ");  //printString
		//read command
		syscall(1,cmdBuffer); //readString

		//parse out command name
		getcmdName(cmdBuffer, cmdName, &spi);
		
		//compare cmd
		if(strCompare(cmdName, "type") == 1){ //if command is type.
			
			//parse out filename
			for(i=0;i<6;i++){filename[i] = cmdBuffer[i+5];}	
			filename[6]=0x0;
			 
			//print contents of file designated by 6 char long filename
			type(filename);
	
		}else if (strCompare("exec", cmdName)==1){ //if command is exec.

			//parse out filename
			for(i=0;i<6;i++){filename[i] = cmdBuffer[i+5];}	
			filename[6]=0x0;
		
			//try to execute program designated by 6 char long filename
			syscall(4, filename);

		}else if (strCompare("dir", cmdName)==1){
			//list files
			dir();

		}else if(strCompare("del", cmdName)==1){
			
			//parse out filename
			for(i=0;i<6;i++){filename[i] = cmdBuffer[i+4];}	
			filename[6]=0x0;
			//delete file
			syscall(7,filename);

		}else{
			syscall(0, "Command not found!\r\n");
			
		}

	}
}



void dir(){

char dir[512]; //buffer to hold sector 2
char entryName[7];
int dirEntry;
int i;
	
	//read sector into buffer
	syscall(2,dir,2);		
	//iterate over all 16 entries
	for(dirEntry=0;dirEntry<512;dirEntry+=32){
		
		//if element @ dirEntry==0x0, DO NOT LIST
		if(dir[dirEntry]=='\0'){
			continue;
		}else{  //else, populate entry name...
			for(i=0;i<6;i++){entryName[i] = dir[dirEntry+i];}
			entryName[6] = '\0';
			//...and list it.
			syscall(0,entryName);
			syscall(0,"\r\n");
		}

		
	}
}


void type(char* filename){

	char buffer[13312]; //max file size
	int readSectors;
	int i;
	
	syscall(3,filename,buffer,&readSectors); //read file into buffer
	if(readSectors>0){
		syscall(0,buffer); //print buffer
		syscall(0,"\r\n");//-----------
		return;
	}else{syscall(0,"File not found!\r\n"); return;}


}




//compare string. takes: string1, string2; returns 0=F,1=T
int strCompare(char* str1, char* str2){  
	int i;
	for(i=0;i<6;i++){

		if(str1[i]==0x0 && str2[i]==0x0){break;}
		if(str1[i]=='\r' && str2[i]==0x0){break;}
		if(str1[i] != str2[i]){return 0;}
	}
	return 1;
}

//takes incoming command buffer, empty buffer to write command to, and int address to write index of SPACE to.
//
void getcmdName(char* cmdbuffer, char* cmdName, int* spaceIndex){

	int i = 0;
	while(cmdbuffer[i]!=0x20 && cmdbuffer[i]!='\r'){
		*cmdName = cmdbuffer[i];
		i++;
		cmdName++;
	}
	*cmdName = 0x0; //NULL terminate string
	*spaceIndex = *spaceIndex+i; //reference to location of SPACE in cmdbuffer
	return;
}

void getfileName(char* cmdbuffer, char* filename, int* spaceIndex){
	
	int i = *spaceIndex + 1; //location of first element after space
	while(cmdbuffer[i] != 0xa){ //
		*filename = cmdbuffer[i];
		i++;			
		filename++;	
	}
	*filename = 0x0; //null terminate string
	return;
}







