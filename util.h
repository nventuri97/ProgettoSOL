#ifndef UTIL_H
#define UTIL_H

#include<sys/types.h>
#include<sys/un.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define SOCKNAME "./objectstore.sock"
#define True 1
#define False 0

#define CHECK(x,f,m) \
    if((x=f)==-1){   \
        perror(m);   \
        exit(errno); \
    }

#define ISNULL(x,f,m) \
    if((x=f)==NULL){  \
        perror(m);    \
        exit(errno);  \
    }






















#endif  /* UTIL_H */