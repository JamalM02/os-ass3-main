#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>

typedef struct {
    // State variables
    int numEntered; // used for synchronization barrier
    int numLeaving; // used for synchronization barrier
    int numThreads; // number of threads (number of files) used for synchronization barrier
    int topCount;   // total number of words to output
    char **words;   // the words from all of the files, an array of strings (2d array of char pointers)
    int *wordCount; // the number of words per length
    // lock and condition variables
    pthread_mutex_t lock; // mutual exclusion lock
    pthread_cond_t allCheckedIn; // condition variable used for synchronization barrier
    pthread_cond_t allLeaving;   // condition variable used for synchronization barrier
} monitor_t;

extern monitor_t *theMonitor; // the actual monitor instance
#endif
