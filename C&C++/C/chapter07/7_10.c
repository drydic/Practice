#include<stdio.h>
#include<ctype.h>

void main(){
	int len = 0;
	char ch;
	printf("Enter a sentence:");
	while((ch = tolower(getchar())) != '\n'){
		if(ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u'){
			len++;
		}
	}
	printf("Your sentence contains %d vowels.\n", len);
}
