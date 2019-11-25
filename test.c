#include <stdio.h>

int strCompare(char* str1, char* str2, int offset);


int main(){

		
	char* name = "mes";
	char dir[8];
	dir[0]='m';
	dir[1]='e';
	dir[2]='s';
	dir[3]='\0';

	if(strCompare(name, dir,0)==1){
		printf("same\n");
	}
	else{printf("Different\n");}


}

//compare string. takes: string1, string2, offset value; returns 0=F,1=T
int strCompare(char* str1, char* str2, int offset){  
	int i;
	for(i=0;i<6;i++){
		if(str1[i]==0x0 && str2[i+offset]==0x0){break;}
		if(str1[i]=='\r' && str2[i+offset]==0x0){break;}
		if(str1[i] != str2[i+offset]){return 0;}
	}
	return 1;
}


