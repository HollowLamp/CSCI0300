#include <stdio.h>
#include <unistd.h>
int main(){
	char writeinto[1000];
	read(0, writeinto, 1000);
	if(strcmp(writeinto, "\n") == 0){
		printf("%s", "1");
	}
}



