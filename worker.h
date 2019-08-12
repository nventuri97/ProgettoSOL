#ifndef WORKER_H
#define WORKER_H

#define _POSIX_C_SOURCE 200112L
#include<util.h>

void *Worker(int client_fd);

#endif