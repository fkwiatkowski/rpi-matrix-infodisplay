#include <stdio.h>
#include <stdlib.h>

#include "string.h"

#include "convertstringtopixel.h"

#include "font-9x16.c"
//#include "font-12x16.c"

//#define NBPARAM	3



char* convertStringToBits(char* string, char* matrixpixzel) {
//convert label:letexte -depth 8 -geometry x16 letext.ppm
	int i = 0;
	int j = 0;

	int b = 0;
	int k = 0;

	int len;
	int lineoffset=0;
	int coloffset=0;

	//char * matrixpixzel = NULL;
	
	len = strlen(string);

	matrixpixzel = (char*)malloc(len * MATRIXHEIGHT * 2 * MATRIXCHARWIDTH);
	memset(matrixpixzel,0,len * MATRIXHEIGHT * 2 * MATRIXCHARWIDTH);
	

	for(i = 0;i<len;i++) {	//Caractére par caractére
	// printf("%c\n",argv[1][i]);
		k = 0;
		for(j=0;j<MATRIXHEIGHT*2;j=j+2,k++) { //LE LIGNE
			lineoffset=MATRIXCHARWIDTH*len*k;
			coloffset=i*MATRIXCHARWIDTH;
			// printf("0x%02X|0x%02X : %03d:%03d    : %03d %03d   ",console_font_9x16[argv[1][i]*32+j],console_font_9x16[argv[1][i]*32+j+1],i,k,lineoffset,coloffset);
			
			for(b=0;b<BYTE;b++) {
				if((console_font_9x16[string[i]*32+j]>>b)&0x1) {
					matrixpixzel[8-b + lineoffset + coloffset] = 1;
					// printf("# %03d | ",8-b + lineoffset + coloffset);
				}
				else {
					// printf("  %03d | ",8-b + lineoffset + coloffset);
				}			
			}
			for(b=BYTE;b<(MATRIXCHARWIDTH);b++) {
				if((console_font_9x16[string[i]*32+j+1]>>b)&0x1) {
					matrixpixzel[8-b + lineoffset + coloffset] = 1;
					// printf("# %03d | ",8-b + lineoffset + coloffset);
				}
				else {
					// printf("  %03d | ",8-b + lineoffset + coloffset);
				}
					
				//printf("%d",matrixpixzel[b+BYTE]);
			}
			// printf("\n");
		}
		// printf("\n");
	}

		
	for(j=0;j<MATRIXHEIGHT;j++) {
		for(b=0;b<MATRIXCHARWIDTH*len-1;b++) {
			
			if(matrixpixzel[b+j*MATRIXCHARWIDTH*len] && !matrixpixzel[b+j*MATRIXCHARWIDTH*len+1]) {
				printf("~");
			}
			else if(matrixpixzel[b+j*MATRIXCHARWIDTH*len] && matrixpixzel[b+j*MATRIXCHARWIDTH*len+1]) {
				printf("%c",0xFF);
			}
			else {
				printf(" ");
			}
		}
		printf("\n");
	}

	return matrixpixzel;
}

// int main(int argc, char *argv[]) {
	// printf("%s : %s\n",argv[1], argv[2]);
	// convertStringToBits(argv[1]);
	// return 0;
// }
