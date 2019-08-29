#ifndef SIGNAL_T
#define SIGNAL_T

#include"util.h"
#include<signal.h>

extern pthread_t create_signal_t();

extern void *signaller();

#endif /*SIGNAL_T*/