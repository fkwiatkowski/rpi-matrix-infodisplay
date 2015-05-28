#ifndef H_STRINGTOPIXELS
#define H_STRINGTOPIXELS

#ifdef __cplusplus
extern "C" {
#endif

#define	MATRIXHEIGHT	16
#define BYTE			8
#define MATRIXCHARWIDTH	10

char* convertStringToBits(char *, char* dst);

#ifdef __cplusplus
}
#endif

#endif /* H_STRINGTOPIXELS */