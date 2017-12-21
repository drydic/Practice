#include<stdio.h>
void main(){
	int len = 0;
	printf("Enter a message: ");
	while(getchar() != '\n'){
		len++;
	}
	printf("Your message was %d character(s) long.\n", len);
}
