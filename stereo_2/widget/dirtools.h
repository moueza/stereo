#ifndef DIR_TOOLS_H
#define DIR_TOOLS_H

/*Returns a \n seperated list of directories or files from the directory *directory.
   The files are sorted alphabetacally */
/*The list is malloc'ed and must be free'd */
/*if f is '/' then only directories are returned. If f is '*' all files
   are returned. If f is 'f', only files are returned */
char *getfilelist (const char *directory, char f);

#endif

