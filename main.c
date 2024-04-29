/*
  Operating Systems
  Semester 1 5784
  Kinneret College on the Sea of Galilee
  Michael J. May
  
  Assignment 3:
  Threads and Mutual Exclusion: Long Words

  Students:
  Jamal Majadle TZ-207513722
  Nagham Mousa TZ-208260018
  
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "monitor.h"
#include "file-processor.h"
#include "checkin.h"


/*Declaration of the printLongestWords function for use in main which defined in file-processor.c (for debug warning)*/
void printLongestWords();

struct thread_info {
    pthread_t thread_id;
    int thread_num;
    char *argv_string;
};


monitor_t *theMonitor;
/* The maximum length of each word in a file */
#define MAX_LINE_LEN 2000


/* monitor_init
* Initializes the monitor structure with necessary parameters and allocates memory for tracking words and their counts.
* Input: monitor - pointer to the monitor structure to be initialized,
*        topCount - the maximum number of words to be printed,
*        numFiles - the number of valid files to be processed.
* Output: Returns 0 on success or -1 on failure (e.g., memory allocation failure).
*/
int monitor_init(monitor_t *monitor, int topCount, int numFiles) {
    // TODO: Initialize the monitor with the parameters needed
    if (!monitor)
        return -1; 

    monitor->numEntered = 0;
    monitor->numLeaving = 0;
    monitor->numThreads = numFiles; 
    monitor->topCount = topCount;   

    monitor->words = malloc((MAX_LINE_LEN + 1) * sizeof(char*)); 

    if (!monitor->words) {
    return -1; 
    }

    for (int i = 0; i <= MAX_LINE_LEN; i++) {
        monitor->words[i] = NULL;
    }

    monitor->wordCount = calloc(MAX_LINE_LEN, sizeof(int));
    if (!monitor->wordCount) {
        perror("Memory allocation failed");
        for (int i = 0; i < MAX_LINE_LEN; i++) {
            free(monitor->words[i]);
        }
        free(monitor->words);
        return -1;
    }

    if (pthread_mutex_init(&monitor->lock, NULL) != 0) {
        perror("Mutex initialization failed");
        for (int i = 0; i < MAX_LINE_LEN; i++) {
            free(monitor->words[i]);
        }
        free(monitor->words);
        free(monitor->wordCount);
        return -1;
    }

    if (pthread_cond_init(&monitor->allCheckedIn, NULL) != 0 || pthread_cond_init(&monitor->allLeaving, NULL) != 0) {
        perror("Condition variable initialization failed");
        pthread_mutex_destroy(&monitor->lock);
        for (int i = 0; i < MAX_LINE_LEN; i++) {
            free(monitor->words[i]);
        }
        free(monitor->words);
        free(monitor->wordCount);
        return -1;
    }

    return 0; 
}


/* showUsage
* Prints the usage instructions for the program if missing arguments (no files, no length).
* Input: None.
* Output: invalid stdin message.
*/
void showUsage() {
    // TODO: Print the usage message
    printf("Usage: longest-words topCount file1 [...]\n");
}


/* main
* The main entry point for the program. It processes command line arguments, initializes the monitor, creates threads for file processing, and calls to output the longest words.
* Input: argc - the number of command line arguments,
*        args[] - array of command line arguments.
* Output: Returns 0 on successful execution or 1 on failure.
*/
int main(int argc, char *args[]) {

    // TODO: Read the parameters
    if (argc < 3||atoi(args[1])<1) {
        showUsage(); 
        return 0;
    }
    int topCount = atoi(args[1]);
    int validFileCount = 0;
    int totalFiles = argc - 2;
    struct thread_info *tinfo = malloc(totalFiles * sizeof(struct thread_info));
    if (!tinfo) {
        perror("Error allocating memory for thread info");
        return 1;
    }
    for (int i = 2; i < argc; i++) {
        FILE *inFile = fopen(args[i], "r");
        if (!inFile) {
            perror("Error opening file");
            continue; 
        }
        tinfo[validFileCount].thread_num = validFileCount;
        tinfo[validFileCount].argv_string = args[i];
        validFileCount++;
    }


    // TODO: Initialize the monitor using monitor_init
    theMonitor = malloc(sizeof(monitor_t));
    if (!theMonitor) {
        perror("Error allocating memory for monitor");
        free(tinfo);
        return 1;
    }    
    if (monitor_init(theMonitor, topCount, validFileCount) != 0) {
    perror("Error initializing monitor");
    free(theMonitor);
    free(tinfo);
    return 1;
    }


    // TODO: Create a thread for each file -  use processFile() function
    for (int i = 0; i < validFileCount; i++){
        pthread_create(&tinfo[i].thread_id, NULL, processFile, tinfo[i].argv_string);

    }


    // TODO: Wait for the threads to complete
    for (int i = 0; i < validFileCount; i++) {
        pthread_join(tinfo[i].thread_id, NULL);
    }


    // TODO: Output the final results - longest words requested, each word length alphabetized
    printLongestWords();


    // TODO: Free everything that was malloc-ed in the setup
    pthread_mutex_lock(&theMonitor->lock);
    for (int i = 0; i <= MAX_LINE_LEN; i++) {
     if (theMonitor->words[i] != NULL) {
        free(theMonitor->words[i]);
     }
    }
    free(theMonitor->words);
    free(theMonitor->wordCount);
    pthread_mutex_unlock(&theMonitor->lock);
    pthread_mutex_destroy(&theMonitor->lock);
    pthread_cond_destroy(&theMonitor->allCheckedIn);
    pthread_cond_destroy(&theMonitor->allLeaving);
    free(theMonitor);
    free(tinfo);
    return 0;
}


