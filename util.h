#ifndef UTIL_H
#define UTIL_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/un.h>
#include<sys/stat.h>
#include<errno.h>
#include<unistd.h>

#define SOCKNAME "./objstore.sock"
#define True 1
#define False 0
#define MAXNAME 50
#define MAXBUFSIZE 180
#define UNIX_PATH_MAX 108

#define CHECKSOCK(x,f,m) \
    if((x=f)==-1){   \
        perror(m);   \
        exit(errno); \
    }

#define CHECK(x,f,m) \
    if((x=f)==-1){   \
        perror(m);   \
    }

/*Questa parte del programma è stata presa dal file conn.h della soluzione dell'assegnamento 11 */
/*--------------------------------------------------------------------------------------------------------------- */
static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=read((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;   // gestione chiusura socket
        left    -= r;
	bufptr  += r;
    }
    return size;
}

static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;  
        left    -= r;
	bufptr  += r;
    }
    return 1;
}
/*--------------------------------------------------------------------------------------------------------------- */

static inline int read_to_new(long fd, void *buf, size_t size) {
    size_t left = size;
    int r, find=0;
    char *bufptr = (char*)buf;
    while(left>0 && find==0) {
        errno=0;
        if ((r=read((int)fd ,bufptr,left)) == -1) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if(strchr(bufptr,'\n'))
            find=1;
        if (r == 0)
            return 0;   // gestione chiusura socket
        left-=r;
        bufptr+=r;
    }
    return size;
}

#endif  /* UTIL_H */