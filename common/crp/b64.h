#include <stdio.h>
int b64encode( FILE *infile, FILE *outfile, int linesize );
int b64estr(const unsigned char* data, int bytes, char* str);
int b64decode( FILE *infile, FILE *outfile );
int b64dstr( const char *str, unsigned char* data, int maxlen);
int b64( char opt, char *infilename, char *outfilename, int linesize );
