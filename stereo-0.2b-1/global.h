#ifndef __CW_GLOBAL_H
#define __CW_GLOBAL_H

/* quad_t should be four bytes */
/* #define quad_t unsigned long */
/* #define quad_t unsigned int */
#define word unsigned short
#define byte unsigned char

/* These may not work on some systems: adjust as needed */

#define MAKE_COMMAND "make"
/*   #define MAKE_ARGS "-k", "-i"        */
/*   #define MAKE_ARGS "-k"              */
#define MAKE_ARGS 0

#define MAN_COMMAND "man"
#define MAN_ARGS "-a"

#define TEMPDIR "/home/terry/stereo-0.2b/temp"

#define DATADIR "/home/terry/stereo-0.2b/images"

#endif				/* __CW_GLOBAL_H */

